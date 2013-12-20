/*
 * $Id: gnoclVFS.c,v 1.4 2004/12/02 20:56:31 baum Exp $
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

char *gnoclMakeURI( Tcl_Interp *interp, Tcl_Obj *obj )
{
   const char *txt = Tcl_GetString( obj );

   if( *txt == '~' )
      return gnome_vfs_expand_initial_tilde( txt );
   if( *txt == '.' )
   {
      char *d1 = g_get_current_dir();
      char *d2 = g_strconcat( d1, "/", txt, NULL );
      g_free( d1 );
      return d2;
   }

   return g_strdup( txt );


   /* TODO: nothing of this is necessary?!?
   gnome_vfs_get_uri_scheme( txt ); 
   if( *txt == '.' )
   {
      char *ret = g_get_current_dir();
      char *dummy = g_strconcat( "file://", ret, "/", txt, NULL );
      g_free( ret );
      ret = gnome_vfs_get_uri_from_local_path( dummy );
      g_free( dummy );
printf( "%s\n", ret );
      return ret;
   }
      
printf( "txt %s %s\n", txt, gnome_vfs_get_uri_from_local_path( txt ) );
   if( *txt != '/' )
   {
      Tcl_SetResult( interp, 
         "Path must be either absolut or begin with '~' or '.'", 
         TCL_STATIC );
      return NULL;
   }

   return gnome_vfs_get_uri_from_local_path( txt );
   */
}

int Gnoclvfs_Init( Tcl_Interp *interp )
{

   if( Tcl_InitStubs( interp, "8.3", 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgRequire( interp, "Gnocl", VERSION, 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgProvide( interp, "GnoclVFS", VERSION ) != TCL_OK )
      return TCL_ERROR;

   if( gnome_vfs_init( ) == 0 )
   {
      Tcl_SetResult( interp, "Could not initialize gnome vfs", TCL_STATIC );
      return TCL_ERROR;
   }

   Tcl_CreateObjCommand( interp, "gnocl::file", gnoclFileCmd, NULL, NULL );
   Tcl_CreateObjCommand( interp, "gnocl::mime", gnoclMimeCmd, NULL, NULL );
   
   return TCL_OK;
}

#undef GNOCL_SET_VAR_OR_RESULT


