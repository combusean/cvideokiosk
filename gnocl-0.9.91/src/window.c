/*
 * $Id: window.c,v 1.16 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements the window widget
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
        11: added cget, -x, -y, -width and -height
        10: added onKey{Press,Release}
        04: added -icon 
        02: added drag and drop options
   2003-01: small fixes, added options "-name"
        09: switched from GnoclWidgetOptions to GnoclOption
            more updates, more and some renamed options
   2002-04: update for gtk 2.0
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption windowOptions[] =
{
   { "-defaultWidth", GNOCL_INT, "default-width" },     /* these must be */
   { "-defaultHeight", GNOCL_INT, "default-height" },   /* before -visible! */
   { "-decorated", GNOCL_BOOL, "decorated" },           /* 2 */
   { "-visible", GNOCL_BOOL, "visible" },               /* 3 */
   { "-x", GNOCL_INT, NULL },                           /* 4 */        
   { "-y", GNOCL_INT, NULL },                           /* 5 */        
   { "-width", GNOCL_INT, NULL },                       /* 6 */        
   { "-height", GNOCL_INT, NULL },                      /* 7 */        
   { "-allowGrow", GNOCL_BOOL, "allow-grow" },
   { "-allowShrink", GNOCL_BOOL, "allow-shrink" },
   { "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding }, 
   { "-child", GNOCL_OBJ, "", gnoclOptChild },
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
   { "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
   { "-heightRequest", GNOCL_INT, "height-request" },
   { "-icon", GNOCL_OBJ, "", gnoclOptIcon },
   { "-modal", GNOCL_BOOL, "modal" },
   { "-name", GNOCL_STRING, "name" },
   { "-onDelete", GNOCL_OBJ, "", gnoclOptOnDelete },
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand }, 
   { "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
   { "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
   { "-onKeyPress", GNOCL_OBJ, "", gnoclOptOnKeyPress },
   { "-onKeyRelease", GNOCL_OBJ, "", gnoclOptOnKeyRelease },
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand }, 
   { "-onResize", GNOCL_OBJ, "", gnoclOptOnConfigure },
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-resizable", GNOCL_BOOL, "resizable" },
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-title", GNOCL_STRING, "title" },
   { "-widthRequest", GNOCL_INT, "width-request" },
   { "-typeHint", GNOCL_OBJ, "", gnoclOptWindowTypeHint },
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { NULL }
};

static const int decoratedIdx   = 2;
static const int visibleIdx     = 3;
static const int xIdx           = 4;
static const int yIdx           = 5;
static const int widthIdx       = 6;
static const int heightIdx      = 7;

static gboolean doOnConfigure( GtkWidget *widget, GdkEventConfigure *event,
      gpointer data )
{
   GnoclCommandData *cs = (GnoclCommandData *)data;

   GnoclPercSubst ps[] = {
      { 'w', GNOCL_STRING },  /* widget */
      { 'x', GNOCL_INT },     /*  */
      { 'y', GNOCL_INT },     /*  */
      { 'W', GNOCL_INT },     /*  */
      { 'H', GNOCL_INT },     /*  */
      { 0 }
   };

   ps[0].val.str = gnoclGetNameFromWidget( widget );
   ps[1].val.i = event->x;
   ps[2].val.i = event->y;
   ps[3].val.i = event->width;
   ps[4].val.i = event->height;
   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );

   return FALSE;
}

int gnoclOptOnConfigure( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   assert( strcmp( opt->optName, "-onResize" ) == 0 );
   return gnoclConnectOptCmd( interp, obj, "configure-event",
         G_CALLBACK( doOnConfigure ), opt, NULL, ret );
}

int gnoclOptWindowTypeHint( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   const char *txt[] = { "normal", "dialog", "menu", "toolbar",
            "splashscreen", "utility", "dock", "desktop", NULL };
   GdkWindowTypeHint types[] = { GDK_WINDOW_TYPE_HINT_NORMAL,
         GDK_WINDOW_TYPE_HINT_DIALOG, GDK_WINDOW_TYPE_HINT_MENU,
         GDK_WINDOW_TYPE_HINT_TOOLBAR, GDK_WINDOW_TYPE_HINT_SPLASHSCREEN,
         GDK_WINDOW_TYPE_HINT_UTILITY, GDK_WINDOW_TYPE_HINT_DOCK,
         GDK_WINDOW_TYPE_HINT_DESKTOP };

   if( ret == NULL ) /* set value */
   {
      int idx;
      if( Tcl_GetIndexFromObj( interp, opt->val.obj, txt, "type hint",
            TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;

       gtk_window_set_type_hint( GTK_WINDOW( obj ), types[idx] );
   }
   else /* get value */
   {
      GdkWindowTypeHint val = gtk_window_get_type_hint( GTK_WINDOW( obj ) );
      int k;
      for( k = 0; txt[k]; ++k )
      {
         if( types[k] == val )
         {
            *ret = Tcl_NewStringObj( txt[k], -1 );
            return TCL_OK;
         }
      }
      Tcl_SetResult( interp, "Unknown setting for parameter", TCL_STATIC );
      return TCL_ERROR;
   }
   return TCL_OK;
}

static int configure( Tcl_Interp *interp, GtkWindow *window, 
      GnoclOption options[] )
{
   /* make only one move if x and y are set */
   if( options[xIdx].status == GNOCL_STATUS_CHANGED 
         && options[xIdx].status == GNOCL_STATUS_CHANGED )
   {
      gtk_window_move( window, options[xIdx].val.i, options[yIdx].val.i );
   } 
   else if( options[xIdx].status == GNOCL_STATUS_CHANGED 
         || options[yIdx].status == GNOCL_STATUS_CHANGED )
   {
      int x, y;
      gtk_window_get_position( window, &x, &y );
      if( options[xIdx].status == GNOCL_STATUS_CHANGED )
         x = options[xIdx].val.i;
      else
         y = options[yIdx].val.i;
      gtk_window_move( window, x, y );
   } 

   /* get_size does not return size set by resize if the resize event
      is not yet handled, we therefor use get_size only if really necessary */
   if( options[widthIdx].status == GNOCL_STATUS_CHANGED 
         && options[heightIdx].status == GNOCL_STATUS_CHANGED )
   {
      gtk_window_resize( window, options[widthIdx].val.i, 
            options[heightIdx].val.i );
   } 
   else if( options[widthIdx].status == GNOCL_STATUS_CHANGED 
         || options[heightIdx].status == GNOCL_STATUS_CHANGED )
   {
      int width, height;
      gtk_window_get_size( window, &width, &height );
      if( options[widthIdx].status == GNOCL_STATUS_CHANGED )
         width = options[widthIdx].val.i;
      else
         height = options[heightIdx].val.i;
      gtk_window_resize( window, width, height );
   } 

   return TCL_OK;
}

static int cget( Tcl_Interp *interp, GtkWindow *window, 
      GnoclOption options[], int idx )
{
   Tcl_Obj *obj = NULL;
   if( idx == xIdx )
   {
      int x, y;
      gtk_window_get_position( window, &x, &y );
      obj = Tcl_NewIntObj( x );
   }
   else if( idx == yIdx )
   {
      int x, y;
      gtk_window_get_position( window, &x, &y );
      obj = Tcl_NewIntObj( y );
   }
   else if( idx == widthIdx )
   {
      int width, height;
      gtk_window_get_size( window, &width, &height );
      obj = Tcl_NewIntObj( width );
   }
   else if( idx == heightIdx )
   {
      int width, height;
      gtk_window_get_size( window, &width, &height );
      obj = Tcl_NewIntObj( height );
   }

   if( obj != NULL )
   {
      Tcl_SetObjResult( interp, obj );
      return TCL_OK;
   }

   return gnoclCgetNotImplemented( interp, options + idx );
}

static int windowFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", "cget", "iconify",
         NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, IconifyIdx };
   GtkWindow *window = GTK_WINDOW( data );
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
            return gnoclDelete( interp, GTK_WIDGET( window ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     windowOptions, G_OBJECT( window ) ) == TCL_OK )
               {
                  ret = configure( interp, window, windowOptions );
               }
               gnoclClearOptions( windowOptions );
               return ret;
            }
            break;
      case CgetIdx:
            {
               int     idx;

               switch( gnoclCget( interp, objc, objv, G_OBJECT( window ), 
                     windowOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           return cget( interp, window, windowOptions, idx );
               }
               assert( 0 );
            }
      case IconifyIdx:
            {
               int iconify = 1;
               if( objc == 3 )
               {
                  if( Tcl_GetBooleanFromObj( interp, objv[2], &iconify )
                        != TCL_OK )
                     return TCL_ERROR;
               }
               else if( objc > 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "?state?" );
                  return TCL_ERROR;
               }
               if( iconify )
                  gtk_window_iconify( window );
               else
                  gtk_window_deiconify( window );
            }
   }

   return TCL_OK;
}

int gnoclWindowCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int        ret;
   GtkWindow  *window;
   
   assert( strcmp( windowOptions[visibleIdx].optName, "-visible" ) == 0 );

   if( gnoclParseOptions( interp, objc, objv, windowOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( windowOptions );
      return TCL_ERROR;
   }

   window = GTK_WINDOW( gtk_window_new( GTK_WINDOW_TOPLEVEL ) );

   /* FIXME: each window own accel_group */
   gtk_window_add_accel_group( window, gnoclGetAccelGroup() );

   ret = gnoclSetOptions( interp, windowOptions, G_OBJECT( window ), -1 );
   if( ret == TCL_OK )
   {
      /* This must be after setting "default-{width,height}".
         If it is after setting the child widget, we get problems if
         the child contains a combo widget. Bizarre!
      */
      if( windowOptions[visibleIdx].status == 0 )
         gtk_widget_show( GTK_WIDGET( window ) );
      ret = configure( interp, window, windowOptions );
   }
   gnoclClearOptions( windowOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( window ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( window ), windowFunc );
}

