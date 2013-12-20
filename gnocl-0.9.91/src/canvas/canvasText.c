/*
 * $Id: canvasText.c,v 1.7 2004/08/15 15:44:57 baum Exp $
 *
 * This file implements the text item of the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-11: Update for gnome 2.0: moved to Pango text
            switched from GnoclWidgetOptions to GnoclOption
   2001-05: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static GnoclOption textOptions[] = 
{
   { "-text", GNOCL_OBJ, NULL },
   { "-tags", GNOCL_OBJ, "", gnoclOptCanvasTags },
   { "-anchor", GNOCL_OBJ, "anchor", gnoclOptAnchor }, 
   { "-coords", GNOCL_LIST, "?", gnoclOptXY },
   { "-fill", GNOCL_OBJ, "fill-color-rgba", gnoclOptRGBAColor },
   { "-font", GNOCL_STRING, "font", NULL },
   { "-fontScale", GNOCL_OBJ, "scale", gnoclOptScale },
   { "-fontSize", GNOCL_OBJ, "size", gnoclOptPangoScaledInt },
   { "-fontStretch", GNOCL_OBJ, "stretch", gnoclOptPangoStretch },
   { "-fontStyle", GNOCL_OBJ, "style", gnoclOptPangoStyle },
   { "-fontVariant", GNOCL_OBJ, "variant", gnoclOptPangoVariant },
   { "-fontWeight", GNOCL_OBJ, "weight", gnoclOptPangoWeight },
   { "-justify", GNOCL_OBJ, "justification", gnoclOptJustification }, 
   { "-offset", GNOCL_LIST, "?_offset", gnoclOptXY },
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { NULL }
};

static const int textIdx   = 0;

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( textOptions );

   if( textOptions[textIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnoclStringType type = gnoclGetStringType( textOptions[textIdx].val.obj );
      char *txt = gnoclGetString( textOptions[textIdx].val.obj );

      gnome_canvas_item_set( info->item, 
            type & GNOCL_STR_MARKUP ? "markup" : "text", txt, NULL );
   }
 
   if( gnoclSetOptions( interp, textOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static Tcl_Obj *cget( Tcl_Interp *interp, int idx, Gnocl_CanvasItemInfo *info )
{
   Tcl_Obj *ret = NULL;

   if( idx == textIdx )
   {
      char *txt;
      g_object_get( G_OBJECT( info->item ), "markup", &txt, NULL );
      if( txt )
      {
         ret = Tcl_NewStringObj( "%#", 2 );
         Tcl_AppendToObj( ret, txt, -1 );
      }
      else
      {
         g_object_get( G_OBJECT( info->item ), "text", &txt, NULL );
         if( txt )
            ret = Tcl_NewStringObj( txt, -1 );
         else
            ret = Tcl_NewStringObj( NULL, 0 );
      }
   }
   
   return ret;
}

Gnocl_CanvasItemInfo *gnoclCanvasCreateText( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = textOptions;
   info->setOptions = &setOptions;
   info->getOption = &cget;
   info->command = NULL;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_text_get_type( ), 
         "fill_color", "black", "font", "Sans 10", NULL );
   return info;
}

