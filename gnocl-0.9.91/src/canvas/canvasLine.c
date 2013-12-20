/*
 * $Id: canvasLine.c,v 1.8 2004/09/21 20:23:38 baum Exp $
 *
 * This file implements the line item of the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
        11: switched from GnoclWidgetOptions to GnoclOption
   2002-01: removed "smooth" and "splineSteps" options since they are not
            implemented in gnome-canvas-line.c
   2001-05: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static int optArrow( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret );
static int optArrowShape( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret );

static GnoclOption lineOptions[] = 
{
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   { "-arrow", GNOCL_OBJ, "", optArrow },
   { "-arrowShape", GNOCL_LIST, "", optArrowShape },
   { "-capStyle", GNOCL_OBJ, "cap-style", gnoclOptCapStyle },
   { "-coords", GNOCL_LIST, "points", gnoclOptPoints },
   { "-fill", GNOCL_OBJ, "fill-color-rgba", gnoclOptRGBAColor },
   { "-joinStyle", GNOCL_OBJ, "join-style", gnoclOptJoinStyle },
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { "-pixelWidth", GNOCL_INT, "width-pixels" },
   { "-width", GNOCL_DOUBLE, "width-units" },
   { NULL }
};

static int optArrowShape( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   if( ret == NULL ) /* set value */
   {
      double vals[3];
      int k;

      if( Tcl_ListObjLength( interp, opt->val.obj, &k ) != TCL_OK
            || k != 3 )
      {
         Tcl_SetResult( interp, 
               "arrow shape must be a list of 3 double values", TCL_STATIC );
         return TCL_ERROR;
      }
      for( k = 0; k < 3; ++k )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, opt->val.obj, k, &tp ) != TCL_OK 
            || Tcl_GetDoubleFromObj( interp, tp, &vals[k] ) != TCL_OK )
         {
            return TCL_ERROR;
         }
      }
      g_object_set( obj, "arrow_shape_a", vals[0], "arrow_shape_b", vals[1], 
            "arrow_shape_c", vals[2], NULL );
   }
   else /* get value */
   {
      double vals[3];
      int k;
      g_object_get( obj, "arrow_shape_a", &vals[0], "arrow_shape_b", &vals[1], 
            "arrow_shape_c", &vals[2], NULL );
      *ret = Tcl_NewListObj( 0, NULL );
      for( k = 0; k < 3; ++k )
         Tcl_ListObjAppendElement( NULL, *ret, Tcl_NewDoubleObj( vals[k] ) );
   }
   return TCL_OK;
}

static int optArrow( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   const char *txt[] = { "none", "first", "last", "both", NULL };
   if( ret == NULL ) /* set value */
   {
      int idx;
      if( Tcl_GetIndexFromObj( interp, opt->val.obj, txt, "arrow",
            TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;

      g_object_set( obj, 
               "first_arrowhead", (gboolean)(idx & 1), 
               "last_arrowhead", (gboolean)((idx & 2) != 0), NULL );
   }
   else /* get value */
   {
      int idx = 0;
      gboolean first, last;
      g_object_get( obj, "first_arrowhead", &first, 
               "last_arrowhead", &last, NULL );
      if( first )
         idx |= 1;
      if( last )
         idx |= 2;
      *ret = Tcl_NewStringObj( txt[idx], -1 );
   }
   return TCL_OK;
}

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( lineOptions );

   if( gnoclSetOptions( interp, lineOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   return ret;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateLine( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = lineOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_line_get_type( ), 
         "fill_color", "black", 
         "arrow_shape_a", 5., 
         "arrow_shape_b", 10., 
         "arrow_shape_c", 5.,
         NULL );

   return info;
}

