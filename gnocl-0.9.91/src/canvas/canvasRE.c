/*
 * $Id: canvasRE.c,v 1.8 2004/09/23 19:49:33 baum Exp $
 *
 * This file implements the rectangle and ellipse item of the canvas widget
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
   2001-05: added rectangle and renamed file canvasEllipse.c to canvasRE.c 
   2001-04: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>


static GnoclOption reOptions[] = 
{
   { "-coords", GNOCL_LIST, NULL },
   { "-centerRadius", GNOCL_BOOL, NULL },
   { "-capStyle", GNOCL_OBJ, "cap_style", gnoclOptCapStyle },
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
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   { "-width", GNOCL_DOUBLE, "width_units" },
   { NULL }
};

static const int coordsIdx       = 0;
static const int centerRadiusIdx = 1;

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( reOptions );

   if( gnoclSetOptions( interp, reOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   if( reOptions[coordsIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *coordsObj = reOptions[coordsIdx].val.obj;
      int centerRadius = 
            GTK_CHECK_TYPE( info->item, GNOME_TYPE_CANVAS_RECT ) ? 0 : 1;
      double coords[4];
      int   noCoords;
      int   n;
      
      if( reOptions[centerRadiusIdx].status == GNOCL_STATUS_CHANGED )
         centerRadius = reOptions[centerRadiusIdx].val.b;

      if( Tcl_ListObjLength( interp, coordsObj, &noCoords ) != TCL_OK
         || !( noCoords == 4 || ( centerRadius && noCoords == 3 ) ) )
      {
         Tcl_SetResult( interp, 
               "list-of-coordinates must contain "
               "3 (if centerRadius) or 4 elements", TCL_STATIC );
         return TCL_ERROR;
      }

      for( n = 0; n < noCoords; ++n )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, coordsObj, n, &tp ) != TCL_OK 
               || Tcl_GetDoubleFromObj( interp, tp, &coords[n] ) != TCL_OK )
         {
            return TCL_ERROR;
         }
      }
      if( centerRadius )
      {
         double x = coords[0];
         double y = coords[1];
         double rx = coords[2];
         double ry = (noCoords == 3) ? rx : coords[3];
         coords[0] = x - rx;  /* FIXME: offset .5 ? */
         coords[1] = y - ry;
         coords[2] = x + rx;
         coords[3] = y + ry;
      }
      gnome_canvas_item_set( info->item, "x1", coords[0], "y1", coords[1],
            "x2", coords[2], "y2", coords[3], NULL );
   }

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   if( idx == coordsIdx )
   {
      double coords[4];
      int k = 4;
      g_object_get( G_OBJECT( info->item ), "x1", &coords[0], 
            "x2", &coords[1], "y1", &coords[2], "y2", &coords[3], NULL );
         
      ret = Tcl_NewListObj( 0, NULL );
      for( k = 0; k < 4; ++k )
         Tcl_ListObjAppendElement( NULL, ret, Tcl_NewDoubleObj( coords[k] ) );
   }
   
   return ret;
}

static Gnocl_CanvasItemInfo *createRE( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group, 
      int isRectangle )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = reOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, 
         isRectangle ? gnome_canvas_rect_get_type() : 
               gnome_canvas_ellipse_get_type(),
         "fill_color", "black", NULL );

   return info;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateEllipse( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   return createRE( interp, objc, objv, group, 0 );
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateRectangle( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   return createRE( interp, objc, objv, group, 1 );
}
