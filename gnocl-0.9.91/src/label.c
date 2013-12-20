/*
 * $Id: label.c,v 1.7 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements the label widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-02: added -data
   2003-08: added cget
        08: switched from GnoclWidgetOptions to GnoclOption
        04: updates for gtk 2.0
   2002-01: _really_ set STD_OPTIONS
        12: removed {x,y}{Align,Pad}, use list parameters instead
        12: {x,y}{Align,Pad}
        09: underlined accelerators; bindUnderline widgetID
        07: Fixed reconfiguration, added std options
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

#include <signal.h>

static GnoclOption labelOptions[] =
{
   { "-text", GNOCL_OBJ, "label", gnoclOptLabelFull },                /* 0 */
   { "-mnemonicWidget", GNOCL_STRING, NULL },   /* 1 */
   { "-align", GNOCL_OBJ, "?align", gnoclOptBothAlign },
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-justify", GNOCL_OBJ, "justify", gnoclOptJustification },
#if GTK_CHECK_VERSION(2,6,0)
   { "-maxWidthChars", GNOCL_INT, "max-width-chars" },
#endif
   { "-name", GNOCL_STRING, "name" },
   { "-selectable", GNOCL_BOOL, "selectable" },
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { "-visible", GNOCL_BOOL, "visible" }, 
#if GTK_CHECK_VERSION(2,6,0)
   { "-widthChars", GNOCL_INT, "width-chars" },
#endif
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   { "-wrap", GNOCL_BOOL, "wrap" },
   { "-xPad", GNOCL_INT, "xpad" },
   { "-yPad", GNOCL_INT, "ypad" },
   { NULL }
};

static const int textIdx = 0;
static const int mnemonicWidgetIdx = 1;

static int configure( Tcl_Interp *interp, GtkLabel *label, 
      GnoclOption options[] )
{
   #if 0
   if( options[textIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnoclStringType type = gnoclGetStringType( options[textIdx].val.obj );
      char *txt = gnoclGetString( options[textIdx].val.obj );

      gtk_label_set_text( label, txt );
      /* TODO? pango_parse_markup for error message */
      gtk_label_set_use_markup( label, (type & GNOCL_STR_MARKUP) != 0 );
      gtk_label_set_use_underline( label, (type & GNOCL_STR_UNDERLINE) != 0 ); 
   }
   #endif
   if( options[mnemonicWidgetIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkWidget *widget = gnoclGetWidgetFromName( 
            options[mnemonicWidgetIdx].val.str, interp );
      if( widget == NULL )
         return TCL_ERROR;
      /* this is a bug in GTK 2.0.6 */
      if( GTK_IS_COMBO( widget ) )
         widget = GTK_COMBO( widget )->entry;

      gtk_label_set_mnemonic_widget( label, widget );
   }

   return TCL_OK;
}

static int cget( Tcl_Interp *interp, GtkLabel *label, 
      GnoclOption options[], int idx )
{
   Tcl_Obj *obj = NULL;

   #if 0
   if( idx == textIdx )
   {
      obj = Tcl_NewStringObj( gtk_label_get_label( label ), -1 );
      if( gtk_label_get_use_markup( label ) )
      {
         Tcl_Obj *old = obj;
         obj = Tcl_NewStringObj( "%<", 2 );
         Tcl_AppendObjToObj( obj, old );
      }
      else if( gtk_label_get_use_underline( label ) )
      {
         Tcl_Obj *old = obj;
         obj = Tcl_NewStringObj( "%_", 2 );
         Tcl_AppendObjToObj( obj, old );
      }
   }
   else 
   #endif
   if( idx == mnemonicWidgetIdx )
   {
      GtkWidget *widg = gtk_label_get_mnemonic_widget( label );
      if( widg == NULL )
         obj = Tcl_NewStringObj( "", 0 );
      else
         obj = Tcl_NewStringObj( gnoclGetNameFromWidget( widg ), -1 );
   }

   if( obj != NULL )
   {
      Tcl_SetObjResult( interp, obj );
      return TCL_OK;
   }

   return gnoclCgetNotImplemented( interp, options + idx );
}

static int labelFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   static const char *cmds[] = { "delete", "configure", "cget", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx };
   int idx;
   GtkLabel *label = (GtkLabel *)data;

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
            return gnoclDelete( interp, GTK_WIDGET( label ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     labelOptions, G_OBJECT( label ) ) == TCL_OK )
               {
                  ret = configure( interp, label, labelOptions );
               }
               gnoclClearOptions( labelOptions );
               return ret;
            }
            break;
      case CgetIdx:
            {
               int     idx;

               /* kill( 0, SIGINT ); */
               switch( gnoclCget( interp, objc, objv, G_OBJECT( label ), 
                     labelOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           return cget( interp, label, labelOptions, idx );
               }
            }
   }

   return TCL_OK;
}

int gnoclLabelCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GtkLabel *label;
   int      ret;
   
   if( gnoclParseOptions( interp, objc, objv, labelOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( labelOptions );
      return TCL_ERROR;
   }

   label = GTK_LABEL( gtk_label_new( NULL ) );

   ret = gnoclSetOptions( interp, labelOptions, G_OBJECT( label ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, label, labelOptions );
   gnoclClearOptions( labelOptions );
   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( label ) );
      return TCL_ERROR;
   }

   gtk_widget_show( GTK_WIDGET( label ) );

   return gnoclRegisterWidget( interp, GTK_WIDGET( label ), labelFunc );
}

