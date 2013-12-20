/*
 * $Id: gnoclGnome.c,v 1.5 2004/07/17 17:00:37 baum Exp $
 *
 * This file implements a Tcl interface to Gnome
 *
 * Copyright (c) 2001 - 2003 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
        12: added appletFactory
   2003-11: Begin of developement
 */

#include "../gnocl.h"
#include <gconf/gconf.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <string.h>
#include <assert.h>

typedef struct 
{
   char *name;
   Tcl_ObjCmdProc *proc;
} GnoclCmd;

Tcl_ObjCmdProc gnoclSessionCmd;
Tcl_ObjCmdProc gnoclAppletFactoryCmd;

static GnoclCmd commands[] = {
   { "session",       gnoclSessionCmd },
   { "appletFactory", gnoclAppletFactoryCmd },
   { NULL }
};

int Gnoclgnome_Init( Tcl_Interp *interp )
{
   GnomeProgram *prg;
   int argc;
   char **argv;
   int k;
   char cmdBuf[128] = "gnocl::";

   printf( "Initializing gnocl gnome version %s\n", VERSION );

   if( Tcl_InitStubs( interp, "8.3", 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgRequire( interp, "Gnocl", VERSION, 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgProvide( interp, "GnoclGnome", VERSION ) != TCL_OK )
      return TCL_ERROR;

   argv = gnoclGetArgv( interp, &argc );
   prg = gnome_program_init( gconf_escape_key( gnoclGetAppName( interp ), -1 ),
         gnoclGetAppVersion( interp ), LIBGNOMEUI_MODULE, argc, argv, 
         GNOME_CLIENT_PARAM_SM_CONNECT, FALSE, GNOME_PARAM_NONE );
   g_free( argv );
   if( prg == NULL )
   {
      Tcl_SetResult( interp, "Could not initialize gnome.", TCL_STATIC );
      return TCL_ERROR;
   }

   for( k = 0; commands[k].name; ++k )
   {
      strcpy( cmdBuf + 7, commands[k].name );
      Tcl_CreateObjCommand( interp, cmdBuf, commands[k]. proc, NULL, NULL );
   }

   return TCL_OK;
}

