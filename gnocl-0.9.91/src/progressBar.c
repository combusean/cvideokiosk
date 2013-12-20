/*
 * $Id: progressBar.c,v 1.7 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the progressBar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-10: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static int optProgressBarOrientation( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   const char *txt[] = { "leftToRight", "rightToLeft", 
         "bottomToTop", "topToBottom", NULL };
   int types[] = { GTK_PROGRESS_LEFT_TO_RIGHT,
         GTK_PROGRESS_RIGHT_TO_LEFT, GTK_PROGRESS_BOTTOM_TO_TOP,
         GTK_PROGRESS_TOP_TO_BOTTOM };

   assert( sizeof( GTK_PROGRESS_LEFT_TO_RIGHT ) == sizeof( int ) );

   return gnoclOptGeneric( interp, opt, obj, "progressBar style", txt, 
         types, ret );
}

static GnoclOption progressBarOptions[] =
{
   { "-activityMode", GNOCL_BOOL, "activity-mode" },
   { "-fraction", GNOCL_DOUBLE, "fraction" },
   { "-pulseStep", GNOCL_DOUBLE, "pulse-step" },
   { "-orientation", GNOCL_OBJ, "orientation", optProgressBarOrientation },
   { "-text", GNOCL_STRING, "text" },
   { "-textAlign", GNOCL_OBJ, "text-?align", gnoclOptBothAlign },
   { "-showText", GNOCL_BOOL, "show-text" },
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { NULL }
};

/*
static int configure( Tcl_Interp *interp, GtkProgressBar *progressBar, 
      GnoclOption options[] )
{
   if( options[textIdx].status == GNOCL_STATUS_CHANGED )
   {
   }

   return TCL_OK;
}
*/

static int progressBarFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   static const char *cmds[] = { "delete", "configure", "pulse", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, PulseIdx };
   int idx;
   GtkProgressBar *progressBar = (GtkProgressBar *)data;

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
            return gnoclDelete( interp, GTK_WIDGET( progressBar ), objc, objv );

      case ConfigureIdx:
            {
               int ret = gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     progressBarOptions, G_OBJECT( progressBar ) );
               gnoclClearOptions( progressBarOptions );
               return ret;
            }
            break;
      case PulseIdx:
            gtk_progress_bar_pulse( progressBar );
            break;
   }

   return TCL_OK;
}

int gnoclProgressBarCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GtkProgressBar *progressBar;
   int            ret;
   
   if( gnoclParseOptions( interp, objc, objv, progressBarOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( progressBarOptions );
      return TCL_ERROR;
   }

   progressBar = GTK_PROGRESS_BAR( gtk_progress_bar_new( ) );

   ret = gnoclSetOptions( interp, progressBarOptions, 
         G_OBJECT( progressBar ), -1 );
   /*
   if( ret == TCL_OK )
      ret = configure( interp, progressBar, progressBarOptions );
   */
   gnoclClearOptions( progressBarOptions );

   gtk_widget_show( GTK_WIDGET( progressBar ) );

   return gnoclRegisterWidget( interp, GTK_WIDGET( progressBar ), 
         progressBarFunc );
}

