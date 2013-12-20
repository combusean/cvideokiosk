/*
 * $Id: canvasBPath.c,v 1.9 2004/09/23 19:49:32 baum Exp $
 *
 * This file implements the bPath item of the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-06: added path with curveTo, lineTo, moveTo ...
   2002-11: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static GnoclOption bPathOptions[] = 
{
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   { "-capStyle", GNOCL_OBJ, "cap_style", gnoclOptCapStyle },
   { "-coords", GNOCL_LIST, "bpath", gnoclOptPath },
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
   /* what's that for? 
   { "-wind", GNOCL_INT, "wind" },
   { "-miterLimit", GNOCL_DOUBLE, "miterlimit" },
   */
   /* TODO: 
   { "-dash", GNOCL_OBJ, "dash" }, 
   { "-fillStipple", GNOCL_OBJ, "fill_stipple" }, 
   */
};

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( bPathOptions );

   if( gnoclSetOptions( interp, bPathOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   return ret;
}

static int command( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
         Gnocl_CanvasItemInfo *info )
{
   const char *cmds[] = { "appendCoords", NULL };
   enum cmdIdx { AppendCoordsIdx };
   int   idx;

   /* canvas itemCommand tag cmd ?-option value? */

   if( Tcl_GetIndexFromObj( interp, objv[3], cmds, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case AppendCoordsIdx:
               {
                  GnomeCanvasPathDef *path;
                  if( objc != 5 )
                  {
                     Tcl_WrongNumArgs( interp, 4, objv, "coords-list" );
                     return TCL_ERROR;
                  }
                  g_object_get( G_OBJECT( info->item), "bpath", &path, NULL );
                  if( gnoclCanvasAppendPath( interp, objv[4], 0, path ) 
                        != TCL_OK )
                     return TCL_ERROR;
                  g_object_set( G_OBJECT( info->item), "bpath", path, NULL );
               }
               break;
            
      default:
            assert( 0 );
            return TCL_ERROR;
   }

   return TCL_OK;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateBPath( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = bPathOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = command;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_bpath_get_type( ), 
         "fill_color", "black", "outline_color", "black", NULL );

   return info;
}

