/*
 * $Id: canvasWidget.c,v 1.6 2004/12/21 21:38:07 baum Exp $
 *
 * This file implements the widget item of the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-11: switched from GnoclWidgetOptions to GnoclOption
   2001-05: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static GnoclOption widgetOptions[] = 
{
   { "-widget", GNOCL_OBJ, "widget", gnoclOptWidget },
   { "-anchor", GNOCL_OBJ, "anchor", gnoclOptAnchor }, 
   { "-coords", GNOCL_LIST, "?", gnoclOptXY },
   { "-height", GNOCL_DOUBLE, "height", NULL }, 
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { "-width", GNOCL_DOUBLE, "width", NULL }, 
   { NULL }
};

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( widgetOptions );

   if( gnoclSetOptions( interp, widgetOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   return ret;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateWidget( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = widgetOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_widget_get_type( ), 
         "width", 100., "height", 100., NULL );

   return info;
}

