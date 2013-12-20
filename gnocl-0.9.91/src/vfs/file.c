/*
 * $Id: file.c,v 1.3 2004/12/02 20:56:30 baum Exp $
 *
 * This file implements a Tcl interface to the virtual file system of Gnome
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-06:    Begin of developement
 */

#include "gnoclVFS.h"
#include <string.h>

#include <assert.h>

#define GNOCL_SET_VAR_OR_RESULT(idx) \
   do \
   { \
      if( options[idx].val.str ) \
         Tcl_SetVar2Ex( interp, options[idx].val.str, NULL,  obj, 0 ); \
      else \
         Tcl_SetObjResult( interp, obj ); \
   } while( 0 )


typedef struct
{
   GnomeVFSHandle *handle;
   char           *name;
} FileInfo;

static int tstError( Tcl_Interp *interp, GnomeVFSResult err )
{
   if( err == GNOME_VFS_OK )
      return 0;

   Tcl_SetObjResult( interp, Tcl_NewStringObj( 
         gnome_vfs_result_to_string( err ), -1 ) );
   return 1;
}

static GnomeVFSURI *getURI( Tcl_Interp *interp, Tcl_Obj *obj )
{
   GnomeVFSURI *uri = gnome_vfs_uri_new( Tcl_GetString( obj ) );
   if( uri == NULL )
      Tcl_SetResult( interp, "Invalid URI", TCL_STATIC );

   return uri;
}

static const char *getFileType( GnomeVFSFileInfo *file )
{
   if( ( file->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_TYPE ) == 0 ) 
      return NULL;

   switch( file->type )
   {
      case GNOME_VFS_FILE_TYPE_UNKNOWN:          return "unknown"; 
      case GNOME_VFS_FILE_TYPE_REGULAR:          return "regular"; 
      case GNOME_VFS_FILE_TYPE_DIRECTORY:        return "directory"; 
      case GNOME_VFS_FILE_TYPE_FIFO:             return "fifo"; 
      case GNOME_VFS_FILE_TYPE_SOCKET:           return "socket"; 
      case GNOME_VFS_FILE_TYPE_CHARACTER_DEVICE: return "charDevice"; 
      case GNOME_VFS_FILE_TYPE_BLOCK_DEVICE:     return "blockDevice"; 
      case GNOME_VFS_FILE_TYPE_SYMBOLIC_LINK:    return "symbolicLink"; 
   }
   return NULL;
}

static Tcl_Obj *getFileSize( GnomeVFSFileInfo *file )
{
   if( ( file->valid_fields & GNOME_VFS_FILE_INFO_FIELDS_SIZE ) == 0 ) 
      return NULL;
   return Tcl_NewWideIntObj( file->size );
}

static int getPermission( Tcl_Interp *interp, Tcl_Obj *obj, guint *perm )
{
   int val;
   /* TODO: parse rwx string */
   if( Tcl_GetIntFromObj( interp, obj, &val ) == TCL_ERROR )
      return TCL_ERROR;
   if( val > 0777 || val < 0 )
   {
      Tcl_SetResult( interp, 
            "Permissions must be greater 0 and lower octal 777", 
            TCL_STATIC );
      return TCL_ERROR;
   }
   *perm = val;
   return TCL_OK;
}


static char *getFileCmdName( )
{
   static int n = 0;
   char buf[256];
   sprintf( buf, "file%d", ++n );
   return g_strdup( buf );
}

static int readCmd( FileInfo *info, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   Tcl_Obj          *obj;
   int              len;
   GnomeVFSResult   res;
   GnomeVFSFileSize bytesRead;

   if( objc != 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "bytes variable" );
      return TCL_ERROR;
   }
   if( Tcl_GetIntFromObj( interp, objv[2], &len ) != TCL_OK )
      return TCL_ERROR;
   if( len < 0 )
   {
      Tcl_SetResult( interp, "\"bytes\" must be greater zero.", TCL_STATIC );
      return TCL_ERROR;
   }

   obj = Tcl_NewStringObj( NULL, 0 );
   if( Tcl_AttemptSetObjLength( obj, len ) == 0 )
   {
      Tcl_SetResult( interp, "Could not allocate enough memory.", TCL_STATIC );
      goto errorExit;
   }
   
   res = gnome_vfs_read( info->handle, obj->bytes, len, &bytesRead );
   if( res == GNOME_VFS_ERROR_EOF )
      Tcl_SetObjResult( interp, Tcl_NewIntObj( -1 ) );
   else if( tstError( interp, res ) )
      goto errorExit;
   else
   {
      Tcl_SetObjResult( interp, Tcl_NewIntObj( (int)bytesRead ) );
      /* this cast is OK since bytesRead < len and len is int */
      Tcl_SetObjLength( obj, (int)bytesRead );
      Tcl_ObjSetVar2( interp, objv[3], NULL, obj, 0 );
   }

   return TCL_OK;

errorExit:
   Tcl_DecrRefCount( obj );
   return TCL_ERROR;
}

static int writeCmd( FileInfo *info, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnomeVFSResult   res;
   GnomeVFSFileSize written;

   if( objc != 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "string" );
      return TCL_ERROR;
   }

   res = gnome_vfs_write( info->handle, objv[2]->bytes, objv[2]->length, 
         &written );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   /* this cast is OK since written <= lenght and len is int */
   Tcl_SetObjResult( interp, Tcl_NewIntObj( (int)written ) );

   return TCL_OK;
}

static int seekCmd( FileInfo *info, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-origin", GNOCL_OBJ, NULL },
      { NULL }
   };
   enum { OriginIdx };

   GnomeVFSResult       res;
   GnomeVFSSeekPosition origin = GNOME_VFS_SEEK_START;
   GnomeVFSFileOffset   offset;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "offset" );
      return TCL_ERROR;
   }
   if( Tcl_GetWideIntFromObj( interp, objv[2], &offset ) != TCL_OK )
      return TCL_ERROR;

   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[OriginIdx].status == GNOCL_STATUS_CHANGED )
   {
      const char *txt[] = { "start", "current", "end", NULL };
      GnomeVFSSeekPosition pos[] = { GNOME_VFS_SEEK_START, 
            GNOME_VFS_SEEK_CURRENT, GNOME_VFS_SEEK_END };

      int idx;
      if( Tcl_GetIndexFromObj( interp, options[OriginIdx].val.obj, txt, 
            "offset", TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;
      origin = pos[idx];
   }

   res = gnome_vfs_seek( info->handle, origin, offset );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   return TCL_OK;
}

static int tellCmd( FileInfo *info, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnomeVFSResult   res;
   GnomeVFSFileSize offset;
   Tcl_Obj          *obj;

   if( objc != 2 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, NULL );
      return TCL_ERROR;
   }

   res = gnome_vfs_tell( info->handle, &offset );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   obj = Tcl_NewWideIntObj( offset );
   Tcl_SetObjResult( interp, Tcl_NewWideIntObj( offset ) );

   return TCL_OK;
}

static void cmdDelete( ClientData data )
{
   FileInfo *info = (FileInfo *)data;
   if( info->handle )
      gnome_vfs_close( info->handle );
   g_free( info->name );
   g_free( info );
}

static int fileCallback( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "read", "write", "seek", "tell", "close", 
         NULL };
   enum cmdIdx { ReadIdx, WriteIdx, SeekIdx, TellIdx, CloseIdx };

   FileInfo *info = (FileInfo *)data;
   int idx;

   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "command" );
      return TCL_ERROR;
   }

   if( Tcl_GetIndexFromObj( interp, objv[1], cmds, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case ReadIdx:  return readCmd( info, interp, objc, objv );
      case WriteIdx: return writeCmd( info, interp, objc, objv );
      case SeekIdx:  return seekCmd( info, interp, objc, objv );
      case TellIdx:  return tellCmd( info, interp, objc, objv );
      case CloseIdx: 
                     {
                        GnomeVFSResult res;
                        if( objc != 2 )
                        {
                           Tcl_WrongNumArgs( interp, 2, objv, NULL );
                           return TCL_ERROR;
                        }
                        res = gnome_vfs_close( info->handle );
                        info->handle = NULL;
                        Tcl_DeleteCommand( interp, info->name );
                        if( tstError( interp, res ) )
                           return TCL_ERROR;
                     }
                     return TCL_OK;
      default:       assert( 0 );
   }

   return TCL_ERROR;
}

static int getOpenMode( Tcl_Interp *interp, Tcl_Obj *obj, 
      GnomeVFSOpenMode *mode ) 
{
   const char *txt[] = { "read", "r", "write", "w", 
         "readWrite", "rw", NULL };
   GnomeVFSOpenMode modes[] = { GNOME_VFS_OPEN_READ, GNOME_VFS_OPEN_READ, 
         GNOME_VFS_OPEN_WRITE, GNOME_VFS_OPEN_WRITE,
         GNOME_VFS_OPEN_READ|GNOME_VFS_OPEN_WRITE, 
         GNOME_VFS_OPEN_READ|GNOME_VFS_OPEN_WRITE };

   int idx;
   if( Tcl_GetIndexFromObj( interp, obj, txt, "mode", TCL_EXACT, 
         &idx ) != TCL_OK )
      return TCL_ERROR;
   *mode = modes[idx];
   return TCL_OK;
}

static int openCmd( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-random", GNOCL_BOOL, NULL },
      { "-mode", GNOCL_OBJ, NULL },
      { NULL }
   };
   enum { RandomIdx, ModeIdx };

   GnomeVFSResult  res;
   GnomeVFSHandle  *handle;
   int             random = 0;
   FileInfo        *info;
   GnomeVFSOpenMode mode = GNOME_VFS_OPEN_READ;
   char            *uri;
   
   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "URI" );
      return TCL_ERROR;
   }

   /* file open fileName -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[ModeIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( getOpenMode( interp, options[ModeIdx].val.obj, &mode ) != TCL_OK )
         return TCL_ERROR;
   }

   if( mode == (GNOME_VFS_OPEN_READ|GNOME_VFS_OPEN_WRITE) )
      random = 1;

   if( options[RandomIdx].status == GNOCL_STATUS_CHANGED )
      random = options[RandomIdx].val.b;

   if( random )
      mode |= GNOME_VFS_OPEN_RANDOM;

   uri = gnoclMakeURI( interp, objv[2] );
   if( uri == NULL )
      return TCL_ERROR;
   res = gnome_vfs_open( &handle, uri,  mode );
   g_free( uri );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   info = g_new( FileInfo, 1 );
   info->name = getFileCmdName();
   info->handle = handle;
   Tcl_CreateObjCommand( interp, info->name, fileCallback, info, cmdDelete );
   Tcl_SetObjResult( interp, Tcl_NewStringObj( info->name, -1 ) );

   return TCL_OK;
}

static int createCmd( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-random", GNOCL_BOOL, NULL },
      { "-permission", GNOCL_OBJ, NULL },
      { "-force", GNOCL_BOOL, NULL },
      { NULL }
   };
   enum { RandomIdx, PermissionIdx, ForceIdx };

   GnomeVFSResult res;
   GnomeVFSHandle *handle;
   int            random = 0;
   gboolean       exclusive = 1;
   guint          perm = 0755;
   FileInfo       *info;
   char           *uri;
   
   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "URI" );
      return TCL_ERROR;
   }

   /* file create fileName -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[RandomIdx].status == GNOCL_STATUS_CHANGED )
      random = options[RandomIdx].val.b;

   if( options[ForceIdx].status == GNOCL_STATUS_CHANGED )
      exclusive = !options[ForceIdx].val.b;

   if( options[PermissionIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( getPermission( interp, options[PermissionIdx].val.obj, &perm )
            != TCL_OK )
         return TCL_ERROR;
   }

   uri = gnoclMakeURI( interp, objv[2] );
   if( uri == NULL )
      return TCL_ERROR;
   res = gnome_vfs_create( &handle, uri,  
         GNOME_VFS_OPEN_WRITE | (random ? GNOME_VFS_OPEN_RANDOM : 0),
         exclusive, perm );
   g_free( uri );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   info = g_new( FileInfo, 1 );
   info->name = getFileCmdName();
   info->handle = handle;
   Tcl_CreateObjCommand( interp, info->name, fileCallback, info, cmdDelete );
   Tcl_SetObjResult( interp, Tcl_NewStringObj( info->name, -1 ) );

   return TCL_OK;
}

static int fileInfo( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-fileType", GNOCL_STRING, NULL },
      { "-isLocal", GNOCL_STRING, NULL },
      { "-isSymlink", GNOCL_STRING, NULL },
      { "-mimeType", GNOCL_STRING, NULL },
      { "-name", GNOCL_STRING, NULL },
      { "-size", GNOCL_STRING, NULL },
      { "-symlinkName", GNOCL_STRING, NULL },
      { NULL }
   };
   enum { fileTypeIdx, isLocalIdx, isSymlinkIdx, mimeTypeIdx, nameIdx, 
          sizeIdx, symlinkNameIdx };
   /* TODO: atime, mtime, ctime, permissions, uid, gid */

   GnomeVFSResult          res;
   GnomeVFSFileInfo        info;
   GnomeVFSFileInfoOptions infoOpt = GNOME_VFS_FILE_INFO_DEFAULT;
   int                     ret = TCL_ERROR;
   char                    *uri;
   
   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "file-name" );
      return TCL_ERROR;
   }

   if( objc == 4 )
   {
      int idx;
      /* file info fileName -option */
      if( gnoclGetIndexFromObjStruct( interp, objv[3], 
            (char **)&options[0].optName, sizeof( GnoclOption ), 
            "option", TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;
      options[idx].status = GNOCL_STATUS_CHANGED;
      assert( options[idx].type == GNOCL_STRING );
      options[idx].val.str = NULL;
   } 
   else
   {
      /* file info fileName -option ... */
      if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
      {
         gnoclClearOptions( options );
         return TCL_ERROR;
      }
   }

   if( options[mimeTypeIdx].status == GNOCL_STATUS_CHANGED )
      infoOpt |= GNOME_VFS_FILE_INFO_GET_MIME_TYPE;

   uri = gnoclMakeURI( interp, objv[2] );
   if( uri == NULL )
      return TCL_ERROR;
   res = gnome_vfs_get_file_info( uri, &info, infoOpt );
   g_free( uri );
   if( tstError( interp, res ) )
      goto cleanExit;

#define CHECK_VALID( name, flag ) \
      if( (info.valid_fields & flag ) == 0 ) \
      { \
         Tcl_SetResult( interp, "Could not retrieve " name , TCL_STATIC ); \
         goto cleanExit; \
      } 

   if( options[fileTypeIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "file type", GNOME_VFS_FILE_INFO_FIELDS_TYPE );
      obj = Tcl_NewStringObj( getFileType( &info ), -1 );
      GNOCL_SET_VAR_OR_RESULT( fileTypeIdx );
   }

   if( options[isLocalIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "isLocal", GNOME_VFS_FILE_INFO_FIELDS_FLAGS );
      obj = Tcl_NewBooleanObj( GNOME_VFS_FILE_INFO_LOCAL( &info ) );
      GNOCL_SET_VAR_OR_RESULT( isLocalIdx );
   }

   if( options[isSymlinkIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "isSymlink", GNOME_VFS_FILE_INFO_FIELDS_FLAGS );
      obj = Tcl_NewBooleanObj( GNOME_VFS_FILE_INFO_SYMLINK( &info ) );
      GNOCL_SET_VAR_OR_RESULT( isSymlinkIdx );
   }

   if( options[mimeTypeIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "mime type", GNOME_VFS_FILE_INFO_FIELDS_MIME_TYPE );
      obj = Tcl_NewStringObj( gnome_vfs_file_info_get_mime_type( &info ), -1 );
      GNOCL_SET_VAR_OR_RESULT( mimeTypeIdx );
   }

   if( options[nameIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewStringObj( info.name , -1 );
      GNOCL_SET_VAR_OR_RESULT( nameIdx );
   }

   if( options[sizeIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "size", GNOME_VFS_FILE_INFO_FIELDS_SIZE );
      obj = getFileSize( &info );
      GNOCL_SET_VAR_OR_RESULT(sizeIdx);
   }

   if( options[symlinkNameIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj;
      CHECK_VALID( "symlink name", GNOME_VFS_FILE_INFO_FIELDS_SYMLINK_NAME );
      obj = Tcl_NewStringObj( info.symlink_name, -1 );
      GNOCL_SET_VAR_OR_RESULT( symlinkNameIdx );
   }

#undef CHECK_VALID

   ret = TCL_OK;

cleanExit:
   gnoclClearOptions( options );
   return ret;
}

static int makeDir( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-permission", GNOCL_OBJ, NULL },
      { NULL }
   };
   enum { PermissionIdx };

   GnomeVFSResult  res;
   guint           perm = 0755;
   char            *uri;
   
   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "directory-name" );
      return TCL_ERROR;
   }

   /* file makeDir fileName -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[PermissionIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( getPermission( interp, options[PermissionIdx].val.obj, &perm )
            != TCL_OK )
         return TCL_ERROR;
   }

   uri = gnoclMakeURI( interp, objv[2] );
   if( uri == NULL )
      return TCL_ERROR;
   res = gnome_vfs_make_directory( uri, perm );
   g_free( uri );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   return TCL_OK;
}

static int listDir( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-matchName", GNOCL_OBJ, NULL },
      { "-onMatch", GNOCL_STRING, NULL },
      { NULL }
   };
   /* -matchName, -matchMime, -matchType,... anything else? */
   enum { MatchNameIdx, OnMatchIdx };

   int                     ret = TCL_ERROR;
   GnomeVFSResult          res;
   GnomeVFSDirectoryHandle *handle;
   GnomeVFSFileInfo        *info;
   Tcl_Obj *obj;
   Tcl_Obj *namePat = NULL;
   char    *cmd = NULL;
   char    *uri;
   
   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "directory-name" );
      return TCL_ERROR;
   }

   /* file listDir fileName -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[MatchNameIdx].status == GNOCL_STATUS_CHANGED )
      namePat = options[MatchNameIdx].val.obj;
   if( options[OnMatchIdx].status == GNOCL_STATUS_CHANGED )
      cmd = options[OnMatchIdx].val.str;

   uri = gnoclMakeURI( interp, objv[2] );
printf( "gnoclMakeURI %s\n", Tcl_GetString( objv[2] ) );
   if( uri == NULL )
      return TCL_ERROR;
   res = gnome_vfs_directory_open( &handle, uri, GNOME_VFS_FILE_INFO_DEFAULT );
printf( "gnome_vfs_directory_open\n" );
   g_free( uri );
   if( tstError( interp, res ) )
      return TCL_ERROR;

   if( cmd == NULL )
      obj = Tcl_NewListObj( 0, NULL );
   info = gnome_vfs_file_info_new(); /* this clears all members */
printf( "gnome_vfs_file_info_new\n" );
   while( (res = gnome_vfs_directory_read_next( handle, info ) ) 
         == GNOME_VFS_OK )
   {
      Tcl_Obj *name = Tcl_NewStringObj( info->name, -1 );
      int match = namePat ? Tcl_RegExpMatchObj( interp, name, namePat ) : 1;
printf( "gnome_vfs_directory_read_next\n" );

printf( "info: %s\n", info->name );
      if( match  )
      {
         if( cmd )
         {
            int eRet;
            GnoclPercSubst ps[] = {
               { 'n', GNOCL_STRING },  /* 0 name */
               { 't', GNOCL_STRING },  /* 1 type */
               { 'l', GNOCL_BOOL },    /* 2 isLocal */
               { 'k', GNOCL_BOOL },    /* 3 isSymlink */
               { 'K', GNOCL_STRING },  /* 4 symlink name */
               { 'm', GNOCL_STRING },  /* 5 mimeType */
               { 's', GNOCL_OBJ },     /* 6 size */
               { 0 }
            };
printf( "in command\n" );
            ps[0].val.str = info->name;
            ps[1].val.str = getFileType( info );
            ps[2].val.b = GNOME_VFS_FILE_INFO_LOCAL( info );
            ps[3].val.b = GNOME_VFS_FILE_INFO_SYMLINK( info );
            ps[4].val.str = info->symlink_name;
            ps[5].val.str = NULL;
            ps[6].val.obj = getFileSize( info );

            eRet = gnoclPercentSubstAndEval( interp, ps, cmd, 0 );
            Tcl_DecrRefCount( name );
            if( ps[6].val.obj ) Tcl_DecrRefCount( ps[6].val.obj );

            if( eRet != TCL_OK )
               goto cleanExit;
         }
         else
            Tcl_ListObjAppendElement( NULL, obj, name );
      }
      else /* no match */
         Tcl_DecrRefCount( name );
      if( match < 0 )
         goto cleanExit;
   }

   ret = TCL_OK;

cleanExit:
   if( res != GNOME_VFS_OK && res != GNOME_VFS_ERROR_EOF )
   {
      ret = TCL_ERROR;
      tstError( interp, res );
   }

   gnoclClearOptions( options );
   gnome_vfs_file_info_unref( info );
   gnome_vfs_directory_close( handle );

   if( ret == TCL_OK )
   {
      /* if onMatch is given, return the empty string */
      if( cmd == NULL )
         Tcl_SetObjResult( interp, obj ); 
   }
   else
      Tcl_DecrRefCount( obj );

   return ret;
}

int gnoclFileCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmd[] = { "info", "exists", "makeDir", "removeDir", 
         "listDir", "remove", "create", "open", NULL };
   enum optIdx { InfoIdx, ExistsIdx, MakeDirIdx, RemoveDirIdx, 
         ListDirIdx, RemoveIdx, CreateIdx, OpenIdx };

   int  idx;
   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "subCommand" );
      return TCL_ERROR;
   }

   if( Tcl_GetIndexFromObj( interp, objv[1], cmd, "subCommand", TCL_EXACT,
         &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case InfoIdx: 
            return fileInfo( interp, objc, objv );
      case ExistsIdx:
            {
               GnomeVFSURI *uri;
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "URI" );
                  return TCL_ERROR;
               }
               uri = getURI( interp, objv[2] );
               if( uri == NULL )
                  return TCL_ERROR;
               Tcl_SetObjResult( interp, 
                     Tcl_NewBooleanObj( gnome_vfs_uri_exists( uri ) ) ); 
               gnome_vfs_uri_unref( uri );
            }
            return TCL_OK;
      case MakeDirIdx:
            return makeDir( interp, objc, objv );
      case RemoveDirIdx:
            {
               GnomeVFSResult res;
               char           *uri;
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "directory-name" );
                  return TCL_ERROR;
               }
               uri = gnoclMakeURI( interp, objv[2] );
               if( uri == NULL )
                  return TCL_ERROR;
               res = gnome_vfs_remove_directory( uri ); 
               g_free( uri );
               if( tstError( interp, res ) )
                  return TCL_ERROR;
            }
            return TCL_OK;
      case RemoveIdx:
            {
               GnomeVFSResult res;
               char           *uri;
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "file-name" );
                  return TCL_ERROR;
               }
               uri = gnoclMakeURI( interp, objv[2] );
               if( uri == NULL )
                  return TCL_ERROR;
               res = gnome_vfs_unlink( uri ); 
               g_free( uri );
               if( tstError( interp, res ) )
                  return TCL_ERROR;
            }
            return TCL_OK;
      case ListDirIdx:
            return listDir( interp, objc, objv );
      case CreateIdx:
            return createCmd( interp, objc, objv );
      case OpenIdx:
            return openCmd( interp, objc, objv );
   }

   return TCL_OK;
}

#undef GNOCL_SET_VAR_OR_RESULT


