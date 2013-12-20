/*
 * $Id: canvas.c,v 1.17 2004/08/25 19:29:04 baum Exp $
 *
 * This file implements the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-03: added itemCget 
            added options to itemInfo
            removed opt{Clear,Parse} functions from itemInfo
   2003-10: added onKey{Press,Release}
   2002-11: updates for gnome-2.0
            switched from GnoclWidgetOptions to GnoclOption
            added support for tags
        11: added scroll window
        05: added rectangle, line, text
   2001-03: Begin of developement
            first item is ellipse
 */

#include "canvas.h"
#include <string.h>
#include <math.h>
#include <assert.h>

#define GET_INFO(items,idx) \
      ((Gnocl_CanvasItemInfo *)g_ptr_array_index( items, idx ))

enum { Affine, Move, Scale, Rotate };

static int gnoclOptOnResize( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret );

static GnoclOption canvasOptions[] = 
{
   { "-antialiased", GNOCL_BOOL, NULL },
   { "-centerScroll", GNOCL_BOOL, NULL },
   { "-scrollRegion", GNOCL_LIST, NULL },
   { "-pixelPerUnit", GNOCL_DOUBLE, NULL },
   { "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-height", GNOCL_INT, "height" },
   { "-name", GNOCL_STRING, "name" },
   { "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
   { "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
   { "-onEnter", GNOCL_OBJ, "", gnoclOptOnEnterLeave },
   { "-onKeyPress", GNOCL_OBJ, "", gnoclOptOnKeyPress },
   { "-onKeyRelease", GNOCL_OBJ, "", gnoclOptOnKeyRelease },
   { "-onLeave", GNOCL_OBJ, "", gnoclOptOnEnterLeave },
   { "-onMap", GNOCL_OBJ, "map", gnoclOptCommand },
   { "-onMotion", GNOCL_OBJ, "", gnoclOptOnMotion },
   { "-onResize", GNOCL_OBJ, "", gnoclOptOnResize },
   { "-onUnmap", GNOCL_OBJ, "unmap", gnoclOptCommand },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-width", GNOCL_INT, "width" },
   { NULL }
};

static const int antialiasedIdx  = 0;
static const int centerScrollIdx = 1;
static const int scrollRegionIdx = 2;
static const int pixelPerUnitIdx = 3;

/* TODO?: { "-scrollbar" } */

Gnocl_CanvasItemInfo *gnoclInfoFromCanvasItem( CanvasParams *param, 
      GnomeCanvasItem *item )
{
   if( item != NULL )
   {
      GPtrArray *items = gnoclCanvasAllItems( param );
      if( items != NULL )
      {
         guint     n;
         for( n = 0; n < items->len; ++n )
         {
            Gnocl_CanvasItemInfo *info = GET_INFO( items, n );
            if( info->item == item )
               return info;
         }
      }
   }

   return NULL;
}

static void doOnResize( GtkWidget *widget, GtkAllocation *alc,
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
   ps[1].val.i = alc->x;
   ps[2].val.i = alc->y;
   ps[3].val.i = alc->width;
   ps[4].val.i = alc->height;
   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );

   return;
}

static int gnoclOptOnResize( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   assert( strcmp( opt->optName, "-onResize" ) == 0 );
   return gnoclConnectOptCmd( interp, obj, "size-allocate",
         G_CALLBACK( doOnResize ), opt, NULL, ret );
}

static void destroyItemFunc( GnomeCanvasItem *item, gpointer data )
{
   Gnocl_CanvasItemInfo *info = (Gnocl_CanvasItemInfo *)data;
   guint k;

   while( info->tags->len )
      gnoclCanvasDelNthTag( info, info->tags->len - 1 );

   for( k = 0; k < GNOCL_CANVAS_ON_COUNT; ++k )
      g_free( info->scripts[k] );

   g_free( info );
}

static void destroyFunc( GtkWidget *widget, gpointer data )
{
   CanvasParams *p = (CanvasParams *)data;
   GPtrArray    *items;

   gnoclForgetWidgetFromName( p->name );
   Tcl_DeleteCommand( p->interp, p->name );

   g_free( p->name );
   /*  TODO: the canvas receives the destroy signal before its items. But
             the items use the hash table on destruction.
             Shouldn't children receive the signal before their parents?
             Hmm, GTK widgets do it the same way.  
             gtk_container_foreach( GTK_CONTAINER( widget ), 
                   (GtkCallback)destroyItemFunc, NULL);  
             doesn't help either

   */
   items = gnoclCanvasAllItems( p );
   if( items != NULL )
   {
      int k;
      for( k = items->len - 1; k >= 0; --k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         gtk_object_destroy( GTK_OBJECT( info->item ) );
      }
   }
   g_hash_table_destroy( p->tagToItems ); 
}

static int cget( Tcl_Interp *interp, GnomeCanvas *canvas, 
      GnoclOption options[], int idx )
{
   Tcl_Obj *obj = NULL;

   if( obj != NULL )
   {
      Tcl_SetObjResult( interp, obj );
      return TCL_OK;
   }

   return gnoclCgetNotImplemented( interp, options + idx );
}

static int configure( Tcl_Interp *interp, CanvasParams *para,
      GnoclOption options[] )
{
   if( options[scrollRegionIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = options[scrollRegionIdx].val.obj;
      int     k, no;
      double  val[4];   /* x, y, w, h */

      if( Tcl_ListObjLength( interp, obj, &no ) != TCL_OK 
         || no != 4 )
      {
         Tcl_SetResult( interp, 
               "scrollRegion must be proper list with four members", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      for( k = 0; k < no; ++k )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, k, &tp ) != TCL_OK )
            return TCL_ERROR;
         if( Tcl_GetDoubleFromObj( interp, tp, &val[k] ) )
            return TCL_ERROR;
      }
      gnome_canvas_set_scroll_region( para->canvas, 
            val[0], val[1], val[0] + val[2], val[1] + val[3] ); 
   }

   if( options[pixelPerUnitIdx].status == GNOCL_STATUS_CHANGED )
   {
      gnome_canvas_set_pixels_per_unit( para->canvas, 
            options[pixelPerUnitIdx].val.d );
   }

   if( options[centerScrollIdx].status == GNOCL_STATUS_CHANGED )
   {
      gnome_canvas_set_center_scroll_region( para->canvas, 
            options[centerScrollIdx].val.b );
   }

#if 0
   if( popt->scrollbar.changed )
   {
      GtkPolicyType hor, vert;
      if( gnoclGetScrollbarPolicy( interp, popt->scrollbar.val, 
            &hor, &vert ) != TCL_OK )
         return TCL_ERROR;

      gtk_scrolled_window_set_policy( para->scrollWin, hor, vert );
   }
#endif

   return TCL_OK;
}

static int windowToCanvas( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], CanvasParams *params, int reverse )
{
   Tcl_Obj *resList;
   int     noCoords, n;
   if( objc != 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, 
            /* canvas windowToCanvas */
            "list-of-coordinates ?option val ...?" );
      return TCL_ERROR;
   }
   /* TODO  
         -only [xy]:          only x, y coordinates
         -pairs [true|false]: list of coordinate pairs (lists)
   */
   if( Tcl_ListObjLength( interp, objv[2], &noCoords ) != TCL_OK
         || ( noCoords % 2 ) )
   {
      Tcl_SetResult( interp, 
            "size of list-of-coordinates must be even", 
            TCL_STATIC );
      return TCL_ERROR;
   }
   resList = Tcl_NewListObj( 0, NULL );
   for( n = 0; n < noCoords; n += 2 )
   {
      Tcl_Obj *tp;
      double xw, yw, x, y;
      int ret = Tcl_ListObjIndex( interp, objv[2], n, &tp );
      if( ret == TCL_OK )
         ret = Tcl_GetDoubleFromObj( interp, tp, &xw );
      if( ret == TCL_OK )
         ret = Tcl_ListObjIndex( interp, objv[2], n + 1, &tp );
      if( ret == TCL_OK )
         ret = Tcl_GetDoubleFromObj( interp, tp, &yw );

      if( ret != TCL_OK )
      {
         Tcl_DecrRefCount( resList );  /* FIXME: is this correct? */
         return TCL_ERROR;
      }
      if( reverse )
         gnome_canvas_world_to_window( params->canvas, xw, yw, &x, &y );
      else
         gnome_canvas_window_to_world( params->canvas, xw, yw, &x, &y );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( x ) );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( y ) );
   }
   Tcl_SetObjResult( interp, resList );

   return TCL_OK;
}

static int findItemAt( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], CanvasParams *params )
{
   double   x, y;
   GnomeCanvasItem *item;
   Gnocl_CanvasItemInfo *info;

   if( objc != 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, /* canvas findItemAt */ "x y" );
      return TCL_ERROR;
   }

   if( Tcl_GetDoubleFromObj( interp, objv[2], &x ) != TCL_OK )
      return TCL_ERROR;
   if( Tcl_GetDoubleFromObj( interp, objv[3], &y ) != TCL_OK )
      return TCL_ERROR;
   
   item = gnome_canvas_get_item_at( params->canvas, x, y );
   /* printf( "item: %p %f %f\n", item, x, y ); */
   info = gnoclInfoFromCanvasItem( params, item );
   if( info )
      Tcl_SetObjResult( interp, Tcl_NewIntObj( info->id ) );

   return TCL_OK;
}

static int itemBounds( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items )
{
   if( objc != 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, NULL );
      return TCL_ERROR;
   }

   if( items != NULL && items->len > 0 )
   {
      Tcl_Obj *resList;
      double  xMin, yMin, xMax, yMax;
      guint   k;
      Gnocl_CanvasItemInfo *info = GET_INFO( items, 0 );
      gnome_canvas_item_get_bounds( info->item, &xMin, &yMin, &xMax, &yMax );

      for( k = 1; k < items->len; ++k )
      {
         double x1, y1, x2, y2;
         info = GET_INFO( items, k );
         gnome_canvas_item_get_bounds( info->item, &x1, &y1, &x2, &y2 );
         if( x1 < xMin )
            xMin = x1;
         if( y1 < yMin )
            yMin = y1;
         if( x2 > xMax )
            xMax = x2;
         if( y2 > yMax )
            yMax = y2;
      }

      resList = Tcl_NewListObj( 0, NULL );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( xMin ) );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( yMin ) );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( xMax ) );
      Tcl_ListObjAppendElement( interp, resList, Tcl_NewDoubleObj( yMax ) );
      Tcl_SetObjResult( interp, resList );
   }

   return TCL_OK;
}

static int getIDs( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items )
{
   if( objc != 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, NULL );
      return TCL_ERROR;
   }

   if( items != NULL && items->len > 0 )
   {
      int     k;
      Tcl_Obj *resList = Tcl_NewListObj( 0, NULL );
      for( k = 0; k < items->len; ++k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         Tcl_ListObjAppendElement( interp, resList, 
               Tcl_NewIntObj( info->id ) );
      }
      Tcl_SetObjResult( interp, resList );
   }
   return TCL_OK;
}


static int itemCommand( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], CanvasParams *param, GPtrArray *items )
{
   /* canvas itemCommand tag-or-id command ?option val ...? */
   guint k;
   if( objc < 4 )
   {
      Tcl_WrongNumArgs( interp, 3, objv, "command ?option val ...?" );
      return TCL_ERROR;
   }

   if( items != NULL )
   {
      for( k = 0; k < items->len; ++k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         if( info->command == NULL )
         {
            Tcl_SetResult( interp, "Canvas item does not have any command.", 
               TCL_STATIC );
            return TCL_ERROR;
         }
         if( (*info->command)( interp, objc, objv, info ) != TCL_OK )
            return TCL_ERROR;
      }
   }

   return TCL_OK;
}

static int itemCget( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], CanvasParams *param, GPtrArray *items )
{
   /* canvas itemCget tag-or-id option */
   Gnocl_CanvasItemInfo *info;
   int idx;

   if( objc != 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "tag-or-id option" );
      return TCL_ERROR;
   }

   if( items == NULL || items->len == 0 )
      return TCL_OK;

   if( items->len > 1 )
   {
      Tcl_SetResult( interp, "tag-or-id must specify a single item", 
            TCL_STATIC );
      return TCL_ERROR;
   }

   info = GET_INFO( items, 0 );

   switch( gnoclCgetOne( interp, objv[3], G_OBJECT( info->item ), 
         info->options, &idx ) )
   {
      case GNOCL_CGET_ERROR:  
               return TCL_ERROR;
      case GNOCL_CGET_HANDLED:
               return TCL_OK;
      case GNOCL_CGET_NOTHANDLED:
               {
                  Tcl_Obj *ret = NULL;
                  if( info->getOption )
                     ret = (*info->getOption)( interp, idx, info );
                  if( ret == NULL )
                     return gnoclCgetNotImplemented( interp, 
                           info->options + idx );
                  Tcl_SetObjResult( interp, ret );
                  return TCL_OK;
               }
   }

   return TCL_ERROR;
}

static int itemConfigure( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], CanvasParams *param, GPtrArray *items )
{
   /* canvas itemConfigure tag-or-id ?option val ...? */
   guint k;
   int   ret = TCL_ERROR;

   if( items == NULL )
      return TCL_OK;

   for( k = 0; k < items->len; ++k )
   {
      Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
      if( gnoclParseOptions( interp, objc - 2, objv + 2, info->options ) 
            != TCL_OK )
         goto cleanExit;
   }
   for( k = 0; k < items->len; ++k )
   {
      Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
      if( (*info->setOptions)( interp, info ) != TCL_OK )
         goto cleanExit;
   }

   ret = TCL_OK;

cleanExit:
   for( k = 0; k < items->len; ++k )
   {
      Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
      gnoclClearOptions( info->options );
   }

   return ret;
}

static int itemShow( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items )
{
   guint k;
   int   on = 1;

   if( objc > 4 )
   {
      Tcl_WrongNumArgs( interp, 3, objv, "?on?" );
      return TCL_ERROR;
   }
   if( objc == 4 )
   {
      if( Tcl_GetBooleanFromObj( interp, objv[3], &on ) != TCL_OK )
         return TCL_ERROR;
   }

   if( items != NULL )
   {
      for( k = 0; k < items->len; ++k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         if( on )
            gnome_canvas_item_show( info->item );
         else
            gnome_canvas_item_hide( info->item );
      }
   }

   return TCL_OK;
}

static int itemRaise( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items, int isRaise )
{
   int k;
   int position = 0;
   if( objc == 4 )
   {
      if( Tcl_GetIntFromObj( interp, objv[3], &position ) != TCL_OK )
         return TCL_ERROR;
      if( position == 0 )
         return TCL_OK;
      if( position < 0 )
      {
         position *= -1;
         isRaise = !isRaise;
      }
   }
   else if( objc > 4 )
   {
      Tcl_WrongNumArgs( interp, 3, objv, "?position?" );
      return TCL_ERROR;
   }

   if( items != NULL )
   {
      for( k = 0; k < items->len; ++k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         if( isRaise )
         {
         /* TODO: with libgnomecanvas-2.2.1 raise_to_top works exactly once */
            if( objc == 3 )
               gnome_canvas_item_raise_to_top( info->item ); 
            else
               gnome_canvas_item_raise( info->item, position );
         }
         else
         {
            if( objc == 3 )
               gnome_canvas_item_lower_to_bottom( info->item );
            else
               gnome_canvas_item_lower( info->item, position );
         }
      }
   }

   return TCL_OK;
}

static int itemDelete( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items )
{
   if( objc != 3 )
   {
      Tcl_WrongNumArgs( interp, 3, objv, NULL );
      return TCL_ERROR;
   }

   if( items != NULL )
   {
      int k;
      for( k = 0; k < items->len; ++k )
      {
         Gnocl_CanvasItemInfo *info = GET_INFO( items, k );
         gtk_object_destroy( GTK_OBJECT( info->item ) );
      }
   }

   return TCL_OK;
}

static int canvasCreateItem( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], CanvasParams *param )
{
   const char *items[] = { "line", "rectangle", "ellipse", "bPath",
         "text", "richText", "widget", "image", "polygon", "clipGroup", 
         NULL };
   enum itemIdx { LineIdx, RectangleIdx, EllipseIdx,  BPathIdx,
         TextIdx, RichTextIdx, WidgetIdx, ImageIdx, PolygonIdx, ClipGroupIdx };

   static int noItems = 0;

   Gnocl_CanvasItemInfo *info;
   GnoclItemCreateFunc *func;
   int  idx;
   int  k;
   char buffer[64];

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, 
            /* canvas create */ "type ?option val ...?" );
      return TCL_ERROR;
   }

   if( Tcl_GetIndexFromObj( interp, objv[2], items, "item type", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case LineIdx:        func = gnoclCanvasCreateLine; break;
      case RectangleIdx:   func = gnoclCanvasCreateRectangle; break;
      case EllipseIdx:     func = gnoclCanvasCreateEllipse; break;
      case BPathIdx:       func = gnoclCanvasCreateBPath; break;
      case TextIdx:        func = gnoclCanvasCreateText; break;
      case RichTextIdx:    func = gnoclCanvasCreateRichText; break;
      case WidgetIdx:      func = gnoclCanvasCreateWidget; break;
      case ImageIdx:       func = gnoclCanvasCreateImage; break;
      case PolygonIdx:     func = gnoclCanvasCreatePolygon; break;
      case ClipGroupIdx:   func = gnoclCanvasCreateClipGroup; break;
      default:             assert( 0 );
                           return TCL_ERROR; /* should never happen */
   }

   info = (*func)( interp, objc, objv, gnome_canvas_root( param->canvas ) );
   if( info == NULL )
      return TCL_ERROR;

   gnoclParseOptions( NULL, 0, NULL, info->options );

   g_object_set_data( G_OBJECT( info->item ), "gnocl::info", info );

   info->id = ++noItems;
   info->tags = g_ptr_array_new( );
   info->canvasParams = param;
   for( k = 0; k < GNOCL_CANVAS_ON_COUNT; ++k )
      info->scripts[k] = NULL;

   sprintf( buffer, "%d", noItems );
   gnoclCanvasAddTag( interp, param, info, buffer );
   gnoclCanvasAddTag( interp, param, info, "all" );

   gnoclClearOptions( info->options );
   if( gnoclParseOptions( interp, objc - 2, objv + 2, info->options ) )
      return TCL_ERROR; /* TODO: free all */

   if( (*info->setOptions)( interp, info ) != TCL_OK )
      return TCL_ERROR; /* TODO: free all */

   gnoclClearOptions( info->options );
   
   g_signal_connect( G_OBJECT( info->item ), "destroy", 
         G_CALLBACK( destroyItemFunc ), info );

   /* gnome_canvas_update_now( param->canvas ); */
   Tcl_SetObjResult( interp, Tcl_NewIntObj( noItems ) );

   return TCL_OK;
}

static int affine( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
      CanvasParams *param, GPtrArray *items, int type )
{
   GnoclOption options[] = {
      { "-absolute", GNOCL_BOOL, NULL },
      { "-reverseOrder", GNOCL_BOOL, NULL },
      { NULL }
   };
   static const int absoluteIdx     = 0;
   static const int reverseOrderIdx = 1;

   guint m; 
   int   noCoords;
   int   absolute = 0; 
   int   reverse = 0;

   if( objc < 4  )
   {
      /* canvas (affine|move|scale|rotate) tag-or-id  coords ?options ? */
      Tcl_WrongNumArgs( interp, 3, objv, 
            "list-of-coordinates ?option val ...?" );
      return TCL_ERROR;
   }
   if( gnoclParseOptions( interp, objc - 3, objv + 3, options ) != TCL_OK )
      return TCL_ERROR;

   if( options[absoluteIdx].status == GNOCL_STATUS_CHANGED )
      absolute = options[absoluteIdx].val.b;
   if( options[reverseOrderIdx].status == GNOCL_STATUS_CHANGED )
      reverse = options[reverseOrderIdx].val.b;
   gnoclClearOptions( options );

   if( absolute && reverse )
   {
      Tcl_SetResult( interp, "Options \"-reverseOrder\" is only valid "
            "for relative transformations.", TCL_STATIC );
      return TCL_ERROR;
   }

   if( Tcl_ListObjLength( interp, objv[3], &noCoords ) != TCL_OK )
      return TCL_ERROR;

   for( m = 0; m < items->len; ++m )
   {
      Gnocl_CanvasItemInfo *info = GET_INFO( items, m );
      int k;
      double af[6];
      Tcl_Obj *tp;

      for( k = 0; k < 6 && k < noCoords; ++k )
      {
         if( Tcl_ListObjIndex( interp, objv[3], k, &tp ) != TCL_OK )
            return TCL_ERROR;
         if( Tcl_GetDoubleFromObj( interp, tp, &af[k] ) != TCL_OK )
            return TCL_ERROR;
      }

      /*
         x' = af[0] * x + af[2] * y + af[4];
         y' = af[1] * x + af[3] * y + af[5];
      */
      switch( type )
      {
         case Affine:
               if( noCoords != 6 )
               {
                  Tcl_SetResult( interp, 
                        "size of list-of-coordinates must be 6", TCL_STATIC );
                  return TCL_ERROR;
               }
               break;
         case Move:
               if( noCoords != 2 )
               {
                  Tcl_SetResult( interp, "size of list-of-coordinates must "
                        "be 2 (delta-x and delta-y)", TCL_STATIC );
                  return TCL_ERROR;
               }
               af[4] = af[0];
               af[5] = af[1];

               af[0] = 1.; af[2] = 0.;
               af[1] = 0.; af[3] = 1.;
               break;
         case Scale:
               if( noCoords != 3 && noCoords != 4 )
               {
                  Tcl_SetResult( interp, "size of list-of-coordinates must "
                        "be 3 or 4 (center, x-scale and y-scale)", 
                        TCL_STATIC );
                  return TCL_ERROR;
               }
               else
               {
                  double x = af[0];
                  double y = af[1];
                  double scalex = af[2];
                  double scaley = (noCoords == 4) ? af[3] : scalex;
                  af[0] = scalex; af[2] = .0;     af[4] = x * (1. - scalex);
                  af[1] = .0;     af[3] = scaley; af[5] = y * (1. - scaley);
               }
               break;
         case Rotate:
               /* there seems to be some problems with the 
                  canvas without antialiasing and rotating 
                  (bizarre drawings)
               */
               if( noCoords != 3 )
               {
                  Tcl_SetResult( interp, "size of list-of-coordinates must "
                        "be 3 (center and angle)", TCL_STATIC );
                  return TCL_ERROR;
               }
               else
               {
                  double x = af[0];
                  double y = af[1];
                  double w = af[2];
                  const double cw = cos( w );
                  const double sw = sin( w );

                  af[0] = cw; af[2] = -sw; af[4] = x - x * cw + y * sw;
                  af[1] = sw; af[3] =  cw; af[5] = y - x * sw - y * cw;
               }
               break;
      }

      /*
      printf( "\nitem: %p absolute: %d\n", info->item, absolute );
      printf( "before:\n" );
      {
         int k;
         double af[6];
         gnome_canvas_item_i2c_affine( info->item, af );
         for( k = 0; k < 6; k += 2 )
            printf( "%d: %f ", k, af[k] );
         printf( "\n" );
         for( k = 1; k < 6; k += 2 )
            printf( "%d: %f ", k, af[k] );
         printf( "\n" );
      }
      printf( "affine:\n" );
      for( k = 0; k < 6; k += 2 )
         printf( "%d: %f ", k, af[k] );
      printf( "\n" );
      for( k = 1; k < 6; k += 2 )
         printf( "%d: %f ", k, af[k] );
      printf( "\n" );
      */

      if( absolute )
         gnome_canvas_item_affine_absolute( info->item, af );
      else
      {
         /* Well, *I* think this is a bug in gnome: 
            affine_relative should not be the reverse of what one
            would expect. Or do I have wrong expectations? */
         if( reverse )
            gnome_canvas_item_affine_relative( info->item, af );
         else
         {
            double bf[6];
            gnome_canvas_item_i2w_affine( info->item, bf );
            gnome_canvas_item_affine_absolute( info->item, af );
            gnome_canvas_item_affine_relative( info->item, bf );
         }
      }
   }

   return TCL_OK;
}

static int isMapped( Tcl_Interp *interp, GtkWidget *widget, 
      int objc, Tcl_Obj * const objv[] )
{
   if( objc != 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, NULL );
      return TCL_ERROR;
   }
   Tcl_SetObjResult( interp, 
         Tcl_NewBooleanObj( GTK_WIDGET_MAPPED( widget ) ) );
   return TCL_OK;
}

static int getCurSize( Tcl_Interp *interp, GtkWidget *widget, 
      int objc, Tcl_Obj * const objv[] )
{
   Tcl_Obj *res;
   if( objc != 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, NULL );
      return TCL_ERROR;
   }
   res = Tcl_NewListObj( 0, NULL );
   Tcl_ListObjAppendElement( interp, res, 
         Tcl_NewIntObj( widget->allocation.x ) );
   Tcl_ListObjAppendElement( interp, res, 
         Tcl_NewIntObj( widget->allocation.y ) );
   Tcl_ListObjAppendElement( interp, res, 
         Tcl_NewIntObj( widget->allocation.width ) );
   Tcl_ListObjAppendElement( interp, res, 
         Tcl_NewIntObj( widget->allocation.height ) );
   Tcl_SetObjResult( interp, res );
   return TCL_OK;
}

static int canvasFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   const char *cmds[] = { "delete", "configure", "cget", "isMapped",
         "getCurrentSize", "update", "raise", "lower", 
         "create", "itemDelete", "itemShow",
         "itemConfigure", "itemCget", "itemCommand", 
         "affine", "scale", "move", "rotate",
         "windowToCanvas", "canvasToWindow",
         "findItemAt", "getBounds", "findWithTag",
         NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, IsMappedIdx,
         GetCurSizeIdx, UpdateIdx, RaiseIdx, LowerIdx,
         CreateIdx, ItemDeleteIdx, ItemShowIdx,
         ItemConfigureIdx, ItemCgetIdx, ItemCommandIdx, 
         AffineIdx, ScaleIdx, MoveIdx, RotateIdx,
         WindowToCanvasIdx, CanvasToWindowIdx,
         FindItemAtIdx, GetBoundsIdx, GetIDsFromTagIdx };
   CanvasParams *para = (CanvasParams *)data;
   GtkWidget    *widget = GTK_WIDGET( para->canvas );
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
            return gnoclDelete( interp, widget, objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     canvasOptions, G_OBJECT( widget ) ) == TCL_OK )
               {

                  if( canvasOptions[antialiasedIdx].status 
                        == GNOCL_STATUS_CHANGED )
                  {
                     Tcl_SetResult( interp, "antialiasing cannot be changed "
                           "after creation", TCL_STATIC );
                  }
                  else
                     ret = configure( interp, para, canvasOptions );
               }
               gnoclClearOptions( canvasOptions );
               return ret;
            }
            break;
      case CgetIdx:
            {
               int     idx;
               switch( gnoclCget( interp, objc, objv, G_OBJECT( widget ), 
                     canvasOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           return cget( interp, para->canvas, 
                                 canvasOptions, idx );
               }
               break;
            }
      case IsMappedIdx:
            return isMapped( interp, widget, objc, objv );
      case GetCurSizeIdx:
            return getCurSize( interp, widget, objc, objv );
      case UpdateIdx:
            if( objc != 2 )
            {
               Tcl_WrongNumArgs( interp, 2, objv, NULL );
               return TCL_ERROR;
            }
            gnome_canvas_update_now( para->canvas );
            break;
      case CreateIdx:
            return canvasCreateItem( interp, objc, objv, para );
      case RaiseIdx:
      case LowerIdx:
      case ItemDeleteIdx:
      case ItemShowIdx:
      case ItemConfigureIdx:
      case ItemCgetIdx:
      case ItemCommandIdx:
      case AffineIdx:
      case ScaleIdx:
      case MoveIdx:
      case RotateIdx:
      case GetBoundsIdx:
      case GetIDsFromTagIdx:
            {
               GPtrArray *items;
               int       ret;

               if( objc < 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, 
                        "tag-or-id ?option val ...?" );
                  return TCL_ERROR;
               }
               if( gnoclCanvasItemsFromTagOrId( interp, para, 
                     Tcl_GetString( objv[2] ), &items ) != TCL_OK )
                  return TCL_ERROR;

               switch( idx )
               {
                  case RaiseIdx:
                  case LowerIdx:
                     ret = itemRaise( interp, objc, objv, para, items,
                           idx == RaiseIdx );
                     break;
                  case ItemDeleteIdx:
                     ret = itemDelete( interp, objc, objv, para, items );
                     break;
                  case ItemShowIdx:
                     ret = itemShow( interp, objc, objv, para, items );
                     break;
                  case ItemConfigureIdx:
                     ret = itemConfigure( interp, objc, objv, para, items );
                     break;
                  case ItemCgetIdx:
                     ret = itemCget( interp, objc, objv, para, items );
                     break;
                  case ItemCommandIdx:
                     ret = itemCommand( interp, objc, objv, para, items );
                     break;
                  case AffineIdx:
                     ret = affine( interp, objc, objv, para, items, Affine );
                     break;
                  case ScaleIdx:
                     ret = affine( interp, objc, objv, para, items, Scale );
                     break;
                  case MoveIdx:
                     ret = affine( interp, objc, objv, para, items, Move );
                     break;
                  case RotateIdx:
                     ret = affine( interp, objc, objv, para, items, Rotate );
                     break;
                  case GetBoundsIdx:
                     ret = itemBounds( interp, objc, objv, para, items );
                     break;
                  case GetIDsFromTagIdx:
                     ret = getIDs( interp, objc, objv, para, items );
                     break;
                  default:
                     assert( 0 );
               }
               if( items )
                  g_ptr_array_free( items, 0 );
               return ret;
            }
            break; 
      case WindowToCanvasIdx:
            return windowToCanvas( interp, objc, objv, para, 0 );
      case CanvasToWindowIdx:
            return windowToCanvas( interp, objc, objv, para, 1 );
      case FindItemAtIdx:
            return findItemAt( interp, objc, objv, para );
   }

   return TCL_OK;
}

static void ptrArrayFree( gpointer data )
{
   assert( ((GPtrArray *)data)->len == 0 );
   g_ptr_array_free( (GPtrArray *)data, 0 );
}

int gnoclCanvasCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   CanvasParams *para;
   int          ret;

   if( gnoclParseOptions( interp, objc, objv, canvasOptions ) != TCL_OK )
   {
      gnoclClearOptions( canvasOptions );
      return TCL_ERROR;
   }

   para = g_new( CanvasParams, 1 );

   /* what is that for? Found in canvas demos. */
   gtk_widget_push_colormap( gdk_rgb_get_cmap() );

   /* antialiased is default */
   if( canvasOptions[antialiasedIdx].status == GNOCL_STATUS_CHANGED &&
         canvasOptions[antialiasedIdx].val.b == 0 )
   {
      para->canvas = GNOME_CANVAS( gnome_canvas_new( ) );
   }
   else
      para->canvas = GNOME_CANVAS( gnome_canvas_new_aa( ) );

   gtk_widget_show( GTK_WIDGET( para->canvas ) );

   /*
   TODO: what is that for? Found in canvas demos. 
   gtk_widget_pop_colormap(); 
   */

   gnome_canvas_set_center_scroll_region( para->canvas, 0 );
   ret = gnoclSetOptions( interp, canvasOptions, 
         G_OBJECT( para->canvas ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, para, canvasOptions );
   gnoclClearOptions( canvasOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( para->canvas ) );
      g_free( para );
      return TCL_ERROR;
   }

   para->name = gnoclGetAutoWidgetId();
   gnoclMemNameAndWidget( para->name, GTK_WIDGET( para->canvas ) );

   /* TODO: g_hash_table_new_full */
   para->tagToItems = g_hash_table_new_full( g_str_hash, g_str_equal,
         g_free, ptrArrayFree );
   para->interp = interp;
   g_signal_connect_after( G_OBJECT( para->canvas ), "destroy", 
         G_CALLBACK( destroyFunc ), para );

   Tcl_CreateObjCommand( interp, para->name, canvasFunc, para, NULL );

   Tcl_SetObjResult( interp, Tcl_NewStringObj( para->name, -1 ) );

   return TCL_OK;
}

