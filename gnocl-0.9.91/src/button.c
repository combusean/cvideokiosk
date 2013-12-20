/*
 * $Id: button.c,v 1.17 2005/02/25 21:33:38 baum Exp $
 *
 * This file implements the button widget
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
        09: added cget
            renamed subcommand "invoke" to "onClicked"
   2003-03: added -icon
        08: switched from GnoclWidgetOptions to GnoclOption
            many cleanups, e.g. no more associated parameters
            renamed -command to -onClicked
        04: update for gtk 2.0
   2002-01: new command "invoke"
        09: underlined accelerators
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption buttonOptions[] =
{
   { "-text", GNOCL_OBJ, NULL },        /* 0 */
   { "-icon", GNOCL_OBJ, NULL },        /* 1 */
   { "-activeBackgroundColor", GNOCL_OBJ, "active", gnoclOptGdkColorBg }, 
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-name", GNOCL_STRING, "name" },
   { "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg }, 
   { "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
   { "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
   { "-onClicked", GNOCL_OBJ, "clicked", gnoclOptCommand },
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg }, 
   { "-relief", GNOCL_OBJ, "relief", gnoclOptRelief }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   /* TODO?: for all widgets, 
      or only for box/frame: {min,max}{Width,Height}, resizeInc, ...
   { "-width", GNOCL_INT, "width-request" }, 
   { "-height", GNOCL_INT, "height-request" }, 
   */
   { NULL },
};

static const int textIdx = 0;
static const int iconIdx = 1;

static int configure( Tcl_Interp *interp, GtkButton *button, 
      GnoclOption options[] )
{
   if( options[textIdx].status == GNOCL_STATUS_CHANGED 
         && gnoclConfigButtonText( interp, button, options[textIdx].val.obj )
            != TCL_OK )
   {
      return TCL_ERROR;
   }
      
   if( options[iconIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnoclStringType type = gnoclGetStringType( options[iconIdx].val.obj );
      GtkWidget *label = gnoclFindChild( GTK_WIDGET( button ), GTK_TYPE_LABEL );

      if( type == GNOCL_STR_EMPTY )
      {
         /* remove all children apart from label */
         GtkWidget *child = gtk_bin_get_child( GTK_BIN( button ) );
         if( child && (child != label) )
         {
            gtk_widget_ref( label );
            gtk_container_remove( GTK_CONTAINER( button ), child );
            gtk_container_add( GTK_CONTAINER( button ), label );
            gtk_widget_unref( label );
            gtk_widget_show( label );
         }
      }
      else
      {
         GtkWidget *image = gnoclFindChild( GTK_WIDGET( button ), 
               GTK_TYPE_IMAGE );
         if( label == NULL )
         {
            gtk_button_set_label( button, "" );
            label = gnoclFindChild( GTK_WIDGET( button ), GTK_TYPE_LABEL );
         }
         else if( (type & (GNOCL_STR_STOCK|GNOCL_STR_FILE)) == 0 )
         {
            Tcl_AppendResult( interp, "Unknown type for \"", 
                  Tcl_GetString( options[iconIdx].val.obj ),
                  "\" must be of type FILE (%/) or STOCK (%#)", NULL );
            return TCL_ERROR;
 
         }
            
         if( image == NULL )
         {
            /* this should match gtkbutton.c */
            GtkWidget *hbox = gtk_hbox_new( 0, 2 );
            GtkWidget *align = gtk_alignment_new( 0.5, 0.5, 0.0, 0.0 );
            image = gtk_image_new( );
         
            gtk_box_pack_start( GTK_BOX( hbox ), image, 0, 0, 0);

            gtk_widget_ref( label );
            gtk_container_remove( GTK_CONTAINER( button ), label ); 
            gtk_box_pack_end( GTK_BOX( hbox ), label, 0, 0, 0);
            gtk_widget_unref( label );
         
            gtk_container_add( GTK_CONTAINER( button ), align );
            gtk_container_add( GTK_CONTAINER( align ), hbox );
            gtk_widget_show_all( align );
         }

         if( type & GNOCL_STR_STOCK )
         {
            GtkStockItem item;
            if( gnoclGetStockItem( options[iconIdx].val.obj, interp, &item )
                  != TCL_OK )
               return TCL_ERROR;
            gtk_image_set_from_stock( GTK_IMAGE( image ), item.stock_id, 
                  GTK_ICON_SIZE_BUTTON );
         }
         else if( type & GNOCL_STR_FILE )
         {
            GdkPixbuf *pix = gnoclPixbufFromObj( interp, options + iconIdx );
            if( pix == NULL )
               return TCL_ERROR;
            gtk_image_set_from_pixbuf( GTK_IMAGE( image ), pix );
         }
      }
   }

   return TCL_OK;
}

int gnoclConfigButtonText( Tcl_Interp *interp, GtkButton *button,
      Tcl_Obj *txtObj )
{
   GnoclStringType type = gnoclGetStringType( txtObj );
   if( type & GNOCL_STR_STOCK )
   {
      GtkStockItem sp;
      if( gnoclGetStockItem( txtObj, interp, &sp ) != TCL_OK )
         return TCL_ERROR;

      gtk_button_set_label( button, sp.stock_id );
      gtk_button_set_use_stock( button, 1 );
   }
   else 
   {
      GtkLabel *label;
      char *txt = gnoclGetString( txtObj );

      gtk_button_set_label( button, txt );
      gtk_button_set_use_stock( button, 0 );

      label = GTK_LABEL( gnoclFindChild( GTK_WIDGET( button ), 
            GTK_TYPE_LABEL ) );
      assert( label );
      /* TODO? pango_parse_markup for error message */
      gtk_label_set_use_markup( label, (type & GNOCL_STR_MARKUP) != 0 );
      gtk_label_set_use_underline( label, 
            (type & GNOCL_STR_UNDERLINE) != 0 ); 
   }
   return TCL_OK;
}

Tcl_Obj *gnoclCgetButtonText( Tcl_Interp *interp, GtkButton *button )
{
   Tcl_Obj *obj = NULL;

   if( gtk_button_get_use_stock( button ) )
   {
      const char *st = gtk_button_get_label( button );

      if( st == NULL )
         obj = Tcl_NewStringObj( "", 0 );
      else
      {
         obj = Tcl_NewStringObj( "%#", 2 );
         Tcl_AppendObjToObj( obj, gnoclGtkToStockName( st ) );
      }
   }
   else 
   {
      GtkLabel *label = GTK_LABEL( gnoclFindChild( GTK_WIDGET( button ), 
            GTK_TYPE_LABEL ) );
      Tcl_Obj *old = Tcl_NewStringObj( gtk_label_get_label( label ), -1 );
      assert( label );

      if( gtk_label_get_use_markup( label ) )
      {
         obj = Tcl_NewStringObj( "%<", 2 );
         Tcl_AppendObjToObj( obj, old );
      }
      else if( gtk_label_get_use_underline( label ) )
      {
         obj = Tcl_NewStringObj( "%_", 2 );
         Tcl_AppendObjToObj( obj, old );
      }
      else
         obj = old;
   }

   return obj;
}

static int cget( Tcl_Interp *interp, GtkButton *button, 
      GnoclOption options[], int idx )
{
   Tcl_Obj *obj = NULL;
   if( idx == textIdx )
      obj = gnoclCgetButtonText( interp, button );
   else if( idx == iconIdx )
   {
      GtkWidget *image = gnoclFindChild( GTK_WIDGET( button ), 
            GTK_TYPE_IMAGE );

      if( image == NULL )
         obj = Tcl_NewStringObj( "", 0 );
      else
      {
         gchar   *st;
         g_object_get( G_OBJECT( image ), "stock", &st, NULL );
         if( st )
         {
            obj = Tcl_NewStringObj( "%#", 2 );
            Tcl_AppendObjToObj( obj, gnoclGtkToStockName( st ) );
            g_free( st );
         }
         else
         {
            Tcl_SetResult( interp, "Could not determine icon type.",
                  TCL_STATIC );
            return TCL_ERROR;
         }
      }
   }

   if( obj != NULL )
   {
      Tcl_SetObjResult( interp, obj );
      return TCL_OK;
   }

   return gnoclCgetNotImplemented( interp, options + idx );
}

static int buttonFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", "cget", "onClicked", 
         NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx };
   GtkButton *button = GTK_BUTTON( data );
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
                  ret = configure( interp, button, buttonOptions );
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
               gtk_button_clicked( button ); 
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
                           return cget( interp, button, buttonOptions, idx );
               }
            }
   }

   return TCL_OK;
}

int gnoclButtonCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int       ret;
   GtkButton *button;
   
   if( gnoclParseOptions( interp, objc, objv, buttonOptions ) != TCL_OK )
   {
      gnoclClearOptions( buttonOptions );
      return TCL_ERROR;
   }

   button = GTK_BUTTON( gtk_button_new( ) );
   gtk_widget_show( GTK_WIDGET( button ) );

   ret = gnoclSetOptions( interp, buttonOptions, G_OBJECT( button ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, button, buttonOptions );
   gnoclClearOptions( buttonOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( button ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( button ), buttonFunc );
}

