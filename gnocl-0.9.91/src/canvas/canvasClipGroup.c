/*
 * $Id: canvasClipGroup.c,v 1.2 2004/08/15 15:44:57 baum Exp $
 *
 * This file implements the clip group of the canvas widget
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
#include <libgnomecanvas/gnome-canvas-clipgroup.h>
#include <string.h>
#include <assert.h>

static GnoclOption clipOptions[] = 
{
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   /* does not work in libgnomecanvas 2.4.0 
   { "-coords", GNOCL_LIST, "?", gnoclOptXY }, */
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-path", GNOCL_LIST, "path", gnoclOptPath },
   { NULL }
};

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( clipOptions );

   if( gnoclSetOptions( interp, clipOptions, G_OBJECT( info->item ), -1 ) 
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
   const char *cmds[] = { "appendPath", NULL };
   enum cmdIdx { AppendPathIdx };
   int   idx;

   /* canvas itemCommand tag cmd ?-option value? */

   if( Tcl_GetIndexFromObj( interp, objv[3], cmds, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case AppendPathIdx:
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

Gnocl_CanvasItemInfo *gnoclCanvasCreateClipGroup( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = clipOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = command;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_clipgroup_get_type( ), NULL );

   return info;
}

