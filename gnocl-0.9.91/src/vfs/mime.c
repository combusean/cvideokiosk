/*
 * $Id: mime.c,v 1.3 2005/04/12 19:21:46 baum Exp $
 *
 * This file implements the mime command
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
#include <libgnomevfs/gnome-vfs-application-registry.h>
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


static int tstError( Tcl_Interp *interp, GnomeVFSResult err )
{
   if( err == GNOME_VFS_OK )
      return 0;

   Tcl_SetObjResult( interp, Tcl_NewStringObj( 
         gnome_vfs_result_to_string( err ), -1 ) );
   return 1;
}

static GnomeVFSMimeApplication *getMimeApp( Tcl_Interp *interp, 
      Tcl_Obj *obj )
{
   GnomeVFSMimeApplication *app =
         gnome_vfs_mime_application_new_from_id( Tcl_GetString( obj ) );
   if( app == NULL )
   {
      Tcl_AppendResult( interp, "Unknown application-id \"", 
            Tcl_GetString( obj ), "\"", NULL );
      return NULL;
   }

   return app;
}

static int getApplicationInfo( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-canOpenMultipleFiles", GNOCL_STRING, NULL },
      { "-command", GNOCL_STRING, NULL },
      { "-expectURIs", GNOCL_STRING, NULL },
      { "-name", GNOCL_STRING, NULL },
      { "-requiresTerminal", GNOCL_STRING, NULL },
      { "-URISchemes", GNOCL_STRING, NULL },
      { NULL }
   };

   enum { canOpenMultipleFilesIdx, commandIdx, expectURIsIdx,
         nameIdx, requiresTerminalIdx, URISchemesIdx };

   int ret = TCL_ERROR;

   GnomeVFSMimeApplication *app;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "application-id" );
      return TCL_ERROR;
   }
   
   app = getMimeApp( interp, objv[2] );
   if( app == NULL )
      return TCL_ERROR;

   if( objc == 4 )
   {
      int idx;
      /* file getApplicationInfo mime-type -option */
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
      /* file getApplicationInfo mime-type -option ... */
      if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
         goto cleanExit;
   }

   if( options[canOpenMultipleFilesIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewBooleanObj( app->can_open_multiple_files );
      GNOCL_SET_VAR_OR_RESULT( canOpenMultipleFilesIdx );
   }

   if( options[commandIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewStringObj( app->command, -1 );
      GNOCL_SET_VAR_OR_RESULT( commandIdx );
   }

   if( options[expectURIsIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj    *obj;
      const char *txt;
      switch( app->expects_uris )
      {
         case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS: 
                                    txt = "URIs"; break;
         case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_PATHS:
                                    txt = "Paths"; break;
         case GNOME_VFS_MIME_APPLICATION_ARGUMENT_TYPE_URIS_FOR_NON_FILES: 
                                    txt = "URIsForNonFiles"; break;
      }
      obj = Tcl_NewStringObj( txt, -1 );
      GNOCL_SET_VAR_OR_RESULT( expectURIsIdx );
   }

   if( options[nameIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewStringObj( app->name, -1 );
      GNOCL_SET_VAR_OR_RESULT( nameIdx );
   }

   if( options[requiresTerminalIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewBooleanObj( app->requires_terminal );
      GNOCL_SET_VAR_OR_RESULT( requiresTerminalIdx );
   }

   if( options[requiresTerminalIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewBooleanObj( app->requires_terminal );
      GNOCL_SET_VAR_OR_RESULT( requiresTerminalIdx );
   }

   if( options[URISchemesIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewListObj( 0, NULL );
      GList *p = app->supported_uri_schemes;
      for( ; p != NULL; p = p->next )
      {
         Tcl_ListObjAppendElement( NULL, obj, 
               Tcl_NewStringObj( (char *)p->data, -1 ) );
      }
      GNOCL_SET_VAR_OR_RESULT( URISchemesIdx );
   }

   ret = TCL_OK;

cleanExit:
   gnome_vfs_mime_application_free( app );
   gnoclClearOptions( options );
   return ret;
}

static int getMimeInfo( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-canBeExecutable", GNOCL_STRING, NULL },
      { "-defaultAction", GNOCL_STRING, NULL },
      { "-defaultApplication", GNOCL_STRING, NULL },
      { "-description", GNOCL_STRING, NULL },
      { "-icon", GNOCL_STRING, NULL },
      { NULL }
   };
   /* TODO { "-defaultComponent", GNOCL_STRING, NULL }, */

   enum { canBeExecutableIdx, defaultActionIdx, defaultApplicationIdx,
         descriptionIdx, iconIdx };

   const char *mimeType;
   int   ret = TCL_ERROR;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "mime-type" );
      return TCL_ERROR;
   }

   mimeType = Tcl_GetString( objv[2] );

   if( objc == 4 )
   {
      int idx;
      /* file mimeInfo mime-type -option */
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
      /* file mimeInfo mime-type -option ... */
      if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
         goto cleanExit;
   }

   if( options[canBeExecutableIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = Tcl_NewBooleanObj( 
            gnome_vfs_mime_can_be_executable( mimeType ) );
      GNOCL_SET_VAR_OR_RESULT(canBeExecutableIdx);
   }

   if( options[defaultActionIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj    *obj;
      const char *txt;
      switch( gnome_vfs_mime_get_default_action_type( mimeType ) )
      {
         case GNOME_VFS_MIME_ACTION_TYPE_APPLICATION: 
                                    txt = "Application"; break;
         case GNOME_VFS_MIME_ACTION_TYPE_COMPONENT:
                                    txt = "Component"; break;

         default:                   assert( 0 );
         case GNOME_VFS_MIME_ACTION_TYPE_NONE: 
                                    txt = "None"; break;
      }
      obj = Tcl_NewStringObj( txt, -1 );
      GNOCL_SET_VAR_OR_RESULT(defaultActionIdx);
   }

   if( options[defaultApplicationIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnomeVFSMimeApplication *app = gnome_vfs_mime_get_default_application(
            mimeType );
      Tcl_Obj *obj = Tcl_NewStringObj( app ? app->id : "", -1 );
      GNOCL_SET_VAR_OR_RESULT(defaultApplicationIdx);
   }

   /* TODO 
   if( options[defaultComponentIdx].status == GNOCL_STATUS_CHANGED )
   {
      Bonobo_ServerInfo *bs = gnome_vfs_mime_get_default_component( mimeType );
      Tcl_Obj *obj = Tcl_NewStringObj( "", -1 );
      GNOCL_SET_VAR_OR_RESULT(defaultComponentIdx);
   }
   */

   if( options[descriptionIdx].status == GNOCL_STATUS_CHANGED )
   {
      const char *descr = gnome_vfs_mime_get_description( mimeType );
      Tcl_Obj *obj = Tcl_NewStringObj( descr ? descr : "", -1 ); 
      GNOCL_SET_VAR_OR_RESULT( descriptionIdx );
   }

   if( options[iconIdx].status == GNOCL_STATUS_CHANGED )
   {
      const char *icon = gnome_vfs_mime_get_icon( mimeType );
      Tcl_Obj *obj = Tcl_NewStringObj( icon ? icon : "", -1 );
      GNOCL_SET_VAR_OR_RESULT( iconIdx );
   }

   ret = TCL_OK;

cleanExit:
   gnoclClearOptions( options );
   return ret;
}

static int getApplicationList( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-all", GNOCL_BOOL, NULL },
      { NULL }
   };
   enum { allIdx };

   int     all = 0;
   GList   *glist;
   Tcl_Obj *ret;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "mime-type" );
      return TCL_ERROR;
   }

   /* mime getApplications mime-type -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
      return TCL_ERROR;

   if( options[allIdx].status == GNOCL_STATUS_CHANGED )
      all = options[allIdx].val.b;

   gnoclClearOptions( options );

   if( all )
      glist = gnome_vfs_mime_get_all_applications( 
            Tcl_GetString( objv[2] ) );
   else
      glist = gnome_vfs_mime_get_short_list_applications( 
            Tcl_GetString( objv[2] ) );

   ret = Tcl_NewListObj( 0, NULL );
   for( ; glist != NULL; glist = glist->next )
   {
      GnomeVFSMimeApplication *app = (GnomeVFSMimeApplication *)glist->data;
      Tcl_ListObjAppendElement( NULL, ret, Tcl_NewStringObj( app->id, -1 ) );
   }

   Tcl_SetObjResult( interp, ret );

   return TCL_OK;
}

static int launch( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[] )
{
   /* TODO 
   GnoclOption options[] =
   {
      { "-env", GNOCL_LIST, NULL },
      { NULL }
   };
   enum { URIsIdx, envIdx };
   */

   GnomeVFSMimeApplication *app;
   GnomeVFSResult          res;
   int                     k, noURIs;
   GList                   *uris = NULL;

   if( objc < 4 )
   {
      Tcl_WrongNumArgs( interp, 3, objv, "application-id URI-list" );
      return TCL_ERROR;
   }

   if( Tcl_ListObjLength( interp, objv[3], &noURIs ) != TCL_OK )
      return TCL_ERROR;

   if( noURIs < 1 )
   {
      Tcl_SetResult( interp, "URI-list must contain at least one element",
            TCL_STATIC );
      return TCL_ERROR;
   }

   for( k = 0; k < noURIs; ++k )
   {
      Tcl_Obj *tp;
      char *txt;
      if( Tcl_ListObjIndex( interp, objv[3], k, &tp ) != TCL_OK )
         return TCL_ERROR;
      txt = gnoclMakeURI( interp, tp );
      if( txt == NULL )
      {
         gnome_vfs_list_deep_free( uris );
         return TCL_ERROR;
      }
      uris = g_list_append( uris, txt );
   }

   app = getMimeApp( interp, objv[2] );
   if( app == NULL )
   {
      gnome_vfs_list_deep_free( uris );
      return TCL_ERROR;
   }

   res = gnome_vfs_mime_application_launch( app, uris );
   /* TODO  
   gnome_vfs_mime_application_launch_with_env( app, uris, char **envp); 
   */

   gnome_vfs_mime_application_free( app );
   gnome_vfs_list_deep_free( uris );

   if( tstError( interp, res ) )
      return TCL_ERROR;

   return TCL_OK;
}

int gnoclMimeCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmd[] = { "getApplicationList", "getMimeInfo", 
         "getApplicationInfo", "launch", NULL };
   enum optIdx { getApplicationsIdx, getMimeInfoIdx, 
         getApplicationInfoIdx, launchIdx };

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
      case getApplicationsIdx: 
            return getApplicationList( interp, objc, objv );
      case getMimeInfoIdx: 
            return getMimeInfo( interp, objc, objv );
      case getApplicationInfoIdx: 
            return getApplicationInfo( interp, objc, objv );
      case launchIdx: 
            return launch( interp, objc, objv );
   }

   return TCL_OK;
}

#undef GNOCL_SET_VAR_OR_RESULT


