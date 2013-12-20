/*
 * $Id: fontButton.c,v 1.2 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the fontButton widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-12: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption buttonOptions[] =
{
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-font", GNOCL_STRING, "font-name" }, 
   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-name", GNOCL_STRING, "name" },
   { "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
   { "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
   { "-onClicked", GNOCL_OBJ, "clicked", gnoclOptCommand },
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-relief", GNOCL_OBJ, "relief", gnoclOptRelief }, 
   { "-showSize", GNOCL_BOOL, "show-size" }, 
   { "-showStyle", GNOCL_BOOL, "show-style" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { "-title", GNOCL_STRING, "title" }, 
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-useFont", GNOCL_BOOL, "use-font" }, 
   { "-useSize", GNOCL_BOOL, "use-size" }, 
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   { NULL },
};

static int buttonFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", "cget", "onClicked", 
         NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx };
   GtkFontButton *button = GTK_FONT_BUTTON( data );
   int idx;

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
            return gnoclDelete( interp, GTK_WIDGET( button ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     buttonOptions, G_OBJECT( button ) ) == TCL_OK )
               {
                  ;
               }
               gnoclClearOptions( buttonOptions );
               return ret;
            }
            break;
      case OnClickedIdx:
            if( objc != 2 )
            {
               Tcl_WrongNumArgs( interp, 2, objv, NULL );
               return TCL_ERROR;
            }
            if( GTK_WIDGET_IS_SENSITIVE( GTK_WIDGET( button ) ) )
               gtk_button_clicked( GTK_BUTTON( button ) ); 
            break;
      case CgetIdx:
            {
               int     idx;

               switch( gnoclCget( interp, objc, objv, G_OBJECT( button ), 
                     buttonOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           return gnoclCgetNotImplemented( interp, 
                                 buttonOptions + idx );
               }
            }
   }

   return TCL_OK;
}

int gnoclFontButtonCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int       ret;
   GtkFontButton *button;
   
   if( gnoclParseOptions( interp, objc, objv, buttonOptions ) != TCL_OK )
   {
      gnoclClearOptions( buttonOptions );
      return TCL_ERROR;
   }

   button = GTK_FONT_BUTTON( gtk_font_button_new( ) );
   gtk_widget_show( GTK_WIDGET( button ) );

   ret = gnoclSetOptions( interp, buttonOptions, G_OBJECT( button ), -1 );
   gnoclClearOptions( buttonOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( button ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( button ), buttonFunc );
}

