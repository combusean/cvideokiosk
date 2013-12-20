/*
 * $Id: appBar.c,v 1.1 2003/11/23 17:42:27 baum Exp $
 *
 * This file implements the appBar widget
 *
 * Copyright (c) 2001 -2003 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-11: Updates for gnome-2.0 and 
            switched from GnoclWidgetOptions to GnoclOption
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption barOptions[] =
{
   /* TODO: interactivity */
   { "-hasStatus", GNOCL_BOOL, "has-status" }, 
   { "-hasProgress", GNOCL_BOOL, "has-progress" }, 
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { NULL }
};

static int configure( Tcl_Interp *interp, GnomeAppBar *button, 
      GnoclOption options[] )
{
   return TCL_OK;
}

static int barFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   const char *cmds[] = { "delete", "configure", 
         "push", "pop", "clear", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, 
         PushIdx, PopIdx, ClearIdx };
   GnomeAppBar *bar = GNOME_APPBAR( data );
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
      case DeleteIdx:
            return gnoclDelete( interp, GTK_WIDGET( bar ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     barOptions, G_OBJECT( bar ) ) == TCL_OK )
               {
                  ret = configure( interp, bar, barOptions );
               }
               gnoclClearOptions( barOptions );
               return ret;
            }
            break;
     case PushIdx:
            {
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "status" );
                  return TCL_ERROR;
               }
               gnome_appbar_push( bar, Tcl_GetString( objv[2] ) );
            }
            break;
      case PopIdx:
            {
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "t" );
                  return TCL_ERROR;
               }
               gnome_appbar_pop( bar );
            }
            break;
      case ClearIdx:
            {
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "t" );
                  return TCL_ERROR;
               }
               gnome_appbar_clear_stack( bar );
            }
            break;
   }

   return TCL_OK;
}

int gnoclAppBarCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnomePreferencesType interactivity = GNOME_PREFERENCES_USER;
   GnomeAppBar  *bar;
   int          ret;
   
   if( gnoclParseOptions( interp, objc, objv, barOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( barOptions );
      return TCL_ERROR;
   }

   bar = GNOME_APPBAR( gnome_appbar_new( 1, 1, interactivity ) );
   gtk_widget_show( GTK_WIDGET( bar ) );

   ret = gnoclSetOptions( interp, barOptions, G_OBJECT( bar ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, bar, barOptions );
   gnoclClearOptions( barOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( bar ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( bar ), barFunc );
}

