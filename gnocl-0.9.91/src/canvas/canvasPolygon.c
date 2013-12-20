/*
 * $Id: canvasPolygon.c,v 1.4 2004/09/23 19:49:32 baum Exp $
 *
 * This file implements the polygon canvas item
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-06: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>


static GnoclOption polyOptions[] = 
{
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   /* not used? { "-capStyle", GNOCL_OBJ, "cap_style", gnoclOptCapStyle }, */
   { "-coords", GNOCL_LIST, "points", gnoclOptPoints },
   { "-dash", GNOCL_OBJ, "dash", gnoclOptDash },
   { "-fill", GNOCL_OBJ, "fill-color-rgba", gnoclOptRGBAColor },
   { "-joinStyle", GNOCL_OBJ, "join_style", gnoclOptJoinStyle },
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-outline", GNOCL_OBJ, "outline-color-rgba", gnoclOptRGBAColor },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { "-pixelWidth", GNOCL_INT, "width_pixels" },
   { "-width", GNOCL_DOUBLE, "width_units" },
   { NULL }
};

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( polyOptions );

   if( gnoclSetOptions( interp, polyOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   return ret;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreatePolygon( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = polyOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, gnome_canvas_polygon_get_type(),
         "fill_color", "black", NULL );

   return info;
}

