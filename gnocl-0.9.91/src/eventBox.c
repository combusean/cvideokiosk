/*
 * $Id: eventBox.c,v 1.9 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the eventBox widget
 *
 * Copyright (c) 2002 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2003-02: added drag and drop options
   2003-01: switched from GnoclWidgetOptions to GnoclOption
   2002-01: Begin of developement
 */
/*
   TODO? handle invisible for user? Or only for gnocl::box?
*/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption boxOptions[] =
{
   { "-child", GNOCL_OBJ, "", gnoclOptChild },
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand }, 
   { "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
   { "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
   { "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
   { "-onMotion", GNOCL_OBJ, "", gnoclOptOnMotion },
   { "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
   { "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
   { "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
   { "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { NULL }
};

static int eventBoxFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", "cget", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx };
   GtkEventBox *box = GTK_EVENT_BOX( data );
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
            return gnoclDelete( interp, GTK_WIDGET( box ), objc, objv );

      case ConfigureIdx:
            {
               int ret = gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     boxOptions, G_OBJECT( box ) );
               gnoclClearOptions( boxOptions );
               return ret;
            }
            break;
      case CgetIdx:
            {
               int     idx;
               switch( gnoclCget( interp, objc, objv, G_OBJECT( box ), 
                     boxOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           assert( 0 );
               }
               assert( 0 );
            }
   }

   return TCL_OK;
}

int gnoclEventBoxCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int         ret;
   GtkEventBox *box;
   
   if( gnoclParseOptions( interp, objc, objv, boxOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( boxOptions );
      return TCL_ERROR;
   }

   box = GTK_EVENT_BOX( gtk_event_box_new( ) );
   gtk_widget_show( GTK_WIDGET( box ) );

   ret = gnoclSetOptions( interp, boxOptions, G_OBJECT( box ), -1 );
   gnoclClearOptions( boxOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( box ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( box ), eventBoxFunc );
}

