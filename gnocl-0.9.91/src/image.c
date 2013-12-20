/*
 * $Id: image.c,v 1.9 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements the image widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
        04: renamed -size to -stockSize, added -size
   2003-02: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption imageOptions[] =
{
   { "-image", GNOCL_OBJ, NULL },
   { "-stockSize", GNOCL_OBJ, NULL },
   { "-size", GNOCL_OBJ, NULL },
   { "-align", GNOCL_OBJ, "?align", gnoclOptBothAlign },
   { "-xPad", GNOCL_OBJ, "xpad", gnoclOptPadding },
   { "-yPad", GNOCL_OBJ, "ypad", gnoclOptPadding },
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { NULL }
};

static const int imageIdx = 0;
static const int stockSizeIdx  = 1;
static const int sizeIdx = 2;

static int getIconSize( Tcl_Interp *interp, Tcl_Obj *obj, GtkIconSize *size )
{
   const char *txt[] = { "menu", "smallToolBar", 
         "largeToolBar", "button", "dnd", "dialog", NULL };
   GtkIconSize modes[] = { GTK_ICON_SIZE_MENU, GTK_ICON_SIZE_SMALL_TOOLBAR,
         GTK_ICON_SIZE_LARGE_TOOLBAR, GTK_ICON_SIZE_BUTTON,
         GTK_ICON_SIZE_DND, GTK_ICON_SIZE_DIALOG };

   int idx;
   if( Tcl_GetIndexFromObj( interp, obj, txt, "icon size",
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   *size = modes[idx];

   return TCL_OK;
}

static int configure( Tcl_Interp *interp, GtkImage *image, 
      GnoclOption options[] )
{
/*
   TODO: 
   GdkPixbuf*  gdk_pixbuf_new_from_data        (const guchar *data,
                                             GDK_COLORSPACE_RGB,
                                             gboolean has_alpha,
                                             int bits_per_sample,
                                             int width,
                                             int height,
                                             int rowstride,
                                             GdkPixbufDestroyNotify destroy_fn,
                                             gpointer destroy_fn_data);
         %m25x10x8
         %i-RGB-25x10x10-
         %i-RGBA-25x10x16

GdkPixbuf*  gdk_pixbuf_new_from_xpm_data    (const char **data);


*/
   if( options[imageIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnoclStringType type = gnoclGetStringType( options[imageIdx].val.obj );
      switch( type & (GNOCL_STR_FILE|GNOCL_STR_STOCK) )
      {
      case GNOCL_STR_FILE:
            {
               GError *error = NULL;
               char *txt = gnoclGetString( options[imageIdx].val.obj );
               GdkPixbufAnimation *ani = gdk_pixbuf_animation_new_from_file( 
                     txt, &error );
               if( ani == NULL )
               {
                  Tcl_SetResult( interp, error->message, TCL_VOLATILE );
                  g_error_free( error );
                  return TCL_ERROR;
               }
               if( gdk_pixbuf_animation_is_static_image( ani ) )
               {
                  GdkPixbuf *pix = gdk_pixbuf_animation_get_static_image( ani );
                  gtk_image_set_from_pixbuf( image, pix );
               }
               else
                  gtk_image_set_from_animation( image, ani );
               g_object_unref( ani );
            }
            break;
      case GNOCL_STR_STOCK:
            {
               GtkIconSize size = GTK_ICON_SIZE_BUTTON;
               GtkStockItem item;
               if( gnoclGetStockItem( options[imageIdx].val.obj, interp, 
                     &item ) != TCL_OK )
                  return TCL_ERROR;
               if( options[stockSizeIdx].status == GNOCL_STATUS_CHANGED )
               {
                  if( getIconSize( interp, options[stockSizeIdx].val.obj,
                        &size ) != TCL_OK )
                  {
                     return TCL_ERROR;
                  }
               }
               else if( gtk_image_get_storage_type( image ) == GTK_IMAGE_STOCK )
                  gtk_image_get_stock( image, NULL, &size );
               gtk_image_set_from_stock( image, item.stock_id, size );
            }
            break;
      default:
            Tcl_AppendResult( interp, "Unknown type for \"", 
                  Tcl_GetString( options[imageIdx].val.obj ),
                  "\" must be of type FILE (%/) or STOCK (%#)", NULL );
            return TCL_ERROR;
      }
   } 
   else if( options[stockSizeIdx].status == GNOCL_STATUS_CHANGED )
   {
      char        *id;
      GtkIconSize size;
      if( gtk_image_get_storage_type( image ) != GTK_IMAGE_STOCK )
      {
         Tcl_SetResult( interp, "Size can only be changed for stock images.",
               TCL_STATIC );
         return TCL_ERROR;
      }
      gtk_image_get_stock( image, &id, &size );
      if( getIconSize( interp, options[stockSizeIdx].val.obj, &size ) 
            != TCL_OK )
         return TCL_ERROR;
      gtk_image_set_from_stock( image, id, size );
   }

   if( options[sizeIdx].status == GNOCL_STATUS_CHANGED )
   {
      GdkPixbuf *src, *dest;
      int       width, height;

      if( gtk_image_get_storage_type( image ) != GTK_IMAGE_PIXBUF )
      {
         Tcl_SetResult( interp, "Only pixbuf images can be sized.",
               TCL_STATIC );
         return TCL_ERROR;
      }

      if( gnoclGet2Int( interp, options[sizeIdx].val.obj, 
            &width, &height ) != TCL_OK )
         return TCL_ERROR;
      if( width <= 0  || height <= 0 )
      {
         Tcl_SetResult( interp, "Size must be greater zero.", TCL_STATIC );
         return TCL_ERROR;
      }

      src = gtk_image_get_pixbuf( image );
      dest = gdk_pixbuf_scale_simple( src, width, height, 
            GDK_INTERP_BILINEAR );
      if( dest == NULL )
      {
         Tcl_SetResult( interp, "Error in scaling. Not enough memory?", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      gtk_image_set_from_pixbuf( image, dest );
      g_object_unref( dest );
   }

   return TCL_OK;
}

static int imageFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   static const char *cmds[] = { "delete", "configure", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx };
   int idx;
   GtkImage *image = (GtkImage *)data;

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
            return gnoclDelete( interp, GTK_WIDGET( image ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     imageOptions, G_OBJECT( image ) ) == TCL_OK )
               {
                  ret = configure( interp, image, imageOptions );
               }
               gnoclClearOptions( imageOptions );
               return ret;
            }
            break;
   }

   return TCL_OK;
}

int gnoclImageCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GtkImage *image;
   int      ret;
   
   if( gnoclParseOptions( interp, objc, objv, imageOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( imageOptions );
      return TCL_ERROR;
   }

   image = GTK_IMAGE( gtk_image_new( ) );

   ret = gnoclSetOptions( interp, imageOptions, G_OBJECT( image ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, image, imageOptions );
   gnoclClearOptions( imageOptions );
   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( image ) );
      return TCL_ERROR;
   }

   gtk_widget_show( GTK_WIDGET( image ) );

   return gnoclRegisterWidget( interp, GTK_WIDGET( image ), imageFunc );
}

