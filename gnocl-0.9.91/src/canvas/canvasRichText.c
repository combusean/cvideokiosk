/*
 * $Id: canvasRichText.c,v 1.6 2004/08/15 15:44:57 baum Exp $
 *
 * This file implements the rich text item of the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-11: Begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <assert.h>

static GnoclOption textOptions[] = 
{
   { "-tags", GNOCL_OBJ, "", gnoclOptCanvasTags },
   { "-anchor", GNOCL_OBJ, "anchor", gnoclOptAnchor }, 
   { "-coords", GNOCL_LIST, "?", gnoclOptXY },
   { "-cursorBlink", GNOCL_BOOL, "cursor_blink" },
   { "-cursorVisible", GNOCL_BOOL, "cursor_visible" },
   /* { "-direction", GNOCL_OBJ, "direction", ??? }, */
   { "-editable", GNOCL_BOOL, "editable" },
   { "-growHeight", GNOCL_BOOL, "grow_height" },
   { "-height", GNOCL_DOUBLE, "height" },
   { "-indent", GNOCL_INT, "indent" },
   { "-justify", GNOCL_OBJ, "justification", gnoclOptJustification },
   { "-leftMargin", GNOCL_INT, "left_margin" },
   { "-onButtonPress", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onButtonRelease", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onEnter", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onLeave", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-onMotion", GNOCL_OBJ, "", gnoclItemOptOnFunc },
   { "-parent", GNOCL_OBJ, "", gnoclOptParent },
   { "-pixelsAboveLines", GNOCL_INT, "pixels_above_lines" },
   { "-pixelsBelowLines", GNOCL_INT, "pixels_below_lines" },
   { "-pixelsInsideWrap", GNOCL_INT, "pixels_inside_wrap" },
   { "-rightMargin", GNOCL_INT, "right_margin" },
   { "-text", GNOCL_STRING, "text" },
   { "-visible", GNOCL_BOOL, "visible" },
   { "-width", GNOCL_DOUBLE, "width" },
   { "-wrapMode", GNOCL_OBJ, "wrap_mode", gnoclOptWrapmode },
   { NULL }
};

static int setOptions( Tcl_Interp *interp, Gnocl_CanvasItemInfo *info )
{
   gnoclResetSetOptions( textOptions );

   if( gnoclSetOptions( interp, textOptions, G_OBJECT( info->item ), -1 ) 
         != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}


static int command( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
         Gnocl_CanvasItemInfo *info )
{
   GtkTextBuffer *buffer = gnome_canvas_rich_text_get_buffer(
         GNOME_CANVAS_RICH_TEXT( info->item ) );

   /* canvas itemCommand tag cmd ?-option value? */
   int ret = gnoclTextCommand( buffer, interp, objc, objv, 3, 0 );
   return ret == 0 ? TCL_OK : TCL_ERROR;
}

#if 0
/* FIXME: this does not work, why?  */
static void setFocus( GObject *item, GdkEventButton *event,
      gpointer data )
{
   if( event->type == GDK_BUTTON_PRESS )
   {
      printf( "in setFocus\n" );
      g_object_set( G_OBJECT( GNOME_CANVAS_ITEM( item )->canvas ), 
            "has-focus", (gboolean) 1, NULL );
      /*
      gnome_canvas_item_grab_focus( GNOME_CANVAS_ITEM( item ) ); 
      */
   }
}
#endif

Gnocl_CanvasItemInfo *gnoclCanvasCreateRichText( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group )
{
   Gnocl_CanvasItemInfo *info = g_new( Gnocl_CanvasItemInfo, 1 );
   info->options = textOptions;
   info->setOptions = &setOptions;
   info->getOption = NULL;
   info->command = &command;
   info->item = gnome_canvas_item_new( group, 
         gnome_canvas_rich_text_get_type( ), 
         "width", 100.0, "height", 100.0, NULL );

   /* FIXME: this does not work, why?
   g_signal_connect( G_OBJECT( info->item ), "event",
         G_CALLBACK( setFocus ), NULL );
   */

   return info;
}

