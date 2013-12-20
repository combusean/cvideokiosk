/*
 * $Id: canvasImage.c,v 1.4 2004/08/15 15:44:57 baum Exp $
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
   2004-05: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static GnoclOption imageOptions[] = 
{
   { "-image", GNOCL_OBJ, NULL },
   { "-tags", GNOCL_LIST, "", gnoclOptCanvasTags },
   { "-anchor", GNOCL_OBJ, "anchor", gnoclOptAnchor }, 
   { "-coords", GNOCL_LIST, "?", gnoclOptXY },
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { NULL }
};

static const int imageIdx = 0;

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( imageOptions );

   if( gnoclSetOptions( interp, imageOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   if( imageOptions[imageIdx].status == GNOCL_STATUS_CHANGED )
   {
      GdkPixbuf *pb = gnoclPixbufFromObj( interp, imageOptions + imageIdx );
      if( pb == NULL )
         return TCL_ERROR;

      gnome_canvas_item_set( info->item, "pixbuf", pb, NULL );
      g_object_unref( pb );
   }

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   /*
   if( idx == imageIdx )
   {
       TODO 
   }
   */
   
   return ret;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateImage( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = imageOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_pixbuf_get_type( ), NULL );

   return info;
}

