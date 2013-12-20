/*
 * $Id: gnoclCanvas.c,v 1.13 2004/09/23 19:49:33 baum Exp $
 *
 * This file implements a Tcl interface to the Gnome canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-06: added complete art path
   2003-07:Split from gnocl as separate library
 */

#include "canvas.h"
#include <string.h>
#include <ctype.h>
#include <libart_lgpl/art_vpath_dash.h>

#include <assert.h>

Tcl_ObjCmdProc gnoclCanvasCmd;

int gnoclOptItemVisible( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   if( ret == NULL ) /* set value */
   {
      int visible;
      if( Tcl_GetBooleanFromObj( interp, opt->val.obj, &visible ) != TCL_OK )
         return TCL_ERROR;

      if( visible )
         gnome_canvas_item_show( GNOME_CANVAS_ITEM( obj ) );
      else
         gnome_canvas_item_hide( GNOME_CANVAS_ITEM( obj ) );
   }
   else /* get value */
   {
      /* TODO how? */
   }

   return TCL_OK;
}

int gnoclOptParent( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   Gnocl_CanvasItemInfo *info = g_object_get_data( obj, "gnocl::info" );
   assert( info->item == GNOME_CANVAS_ITEM( obj ) );

   if( ret == NULL ) /* set value */
   {
      Gnocl_CanvasItemInfo *parent;
      GPtrArray *items;

      if( gnoclCanvasItemsFromTagOrId( interp, info->canvasParams, 
                     Tcl_GetString( opt->val.obj ), &items ) != TCL_OK )
         return TCL_ERROR;

      if( items->len > 1 )
      {
         g_ptr_array_free( items, 0 );
         Tcl_SetResult( interp, "This command works only fo a single item.", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      parent = (Gnocl_CanvasItemInfo *)g_ptr_array_index( items, 0 );
      if( !GNOME_IS_CANVAS_GROUP( parent->item ) )
      {
         g_ptr_array_free( items, 0 );
         Tcl_SetResult( interp, "Parent must be a group or a clipGroup.", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      gnome_canvas_item_reparent( info->item, 
            GNOME_CANVAS_GROUP( parent->item ) );

      g_ptr_array_free( items, 0 );
   }
   else /* get value */
   {
      GnomeCanvasItem *parent;
      Gnocl_CanvasItemInfo *parentInfo;

      g_object_get( obj, "parent", &parent, NULL );
      parentInfo = gnoclInfoFromCanvasItem( info->canvasParams, parent );
      if( parentInfo )
         *ret = Tcl_NewIntObj( parentInfo->id );
   }

   return TCL_OK;
}

int gnoclOptJoinStyle( Tcl_Interp *interp, GnoclOption *opt,
      GObject *obj, Tcl_Obj **ret )
{
   const char *txt[] = { "miter", "round", "bevel", NULL };
   const int types[] = { GDK_JOIN_MITER, GDK_JOIN_ROUND, GDK_JOIN_BEVEL };

   assert( sizeof( GDK_JOIN_MITER ) == sizeof( int ) );

   return gnoclOptGeneric( interp, opt, obj, "join style", txt, types, ret );
}

int gnoclOptCapStyle( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   const char *txt[] = { "notLast", "butt", "round", "projecting", NULL };
   const int types[] = { GDK_CAP_NOT_LAST, GDK_CAP_BUTT,
          GDK_CAP_ROUND, GDK_CAP_PROJECTING };

   assert( sizeof( GDK_CAP_NOT_LAST ) == sizeof( int ) );

   return gnoclOptGeneric( interp, opt, obj, "cap style", txt, types, ret );
}

int gnoclOptDash( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   if( ret == NULL ) /* set value */
   {
      int no;
      if( Tcl_ListObjLength( interp, opt->val.obj, &no ) != TCL_OK )
         return TCL_ERROR;
      if( no == 1 )
      {
         Tcl_SetResult( interp, "length of dash pattern list must be > 1",
               TCL_STATIC );
         return TCL_ERROR;
      }

      if( no == 0 )
      {
         g_object_set( obj, opt->propName, NULL, NULL );
         return TCL_OK;
      }
      else
      {
         int retVal = TCL_ERROR;
         int          k, kStart;
         ArtVpathDash dd;
         Tcl_Obj      *tp;

         if( no % 2 )
         {
            kStart = 1;
            dd.n_dash = no - 1;
            if( Tcl_ListObjIndex( interp, opt->val.obj, 0, &tp ) != TCL_OK 
                  || Tcl_GetDoubleFromObj( interp, tp, &dd.offset ) != TCL_OK )
            {
               return TCL_ERROR;
            }
         }
         else
         {
            kStart = 0;
            dd.n_dash = no;
            dd.offset = 0;
         }

         dd.dash = g_new( double, dd.n_dash );
         for( k = kStart; k < no; ++k )
         {
            if( Tcl_ListObjIndex( interp, opt->val.obj, k, &tp ) != TCL_OK 
                  || Tcl_GetDoubleFromObj( interp, tp, &dd.dash[k-kStart] ) 
                        != TCL_OK )
            {
               goto cleanExit;
            }
         }

         g_object_set( obj, opt->propName, &dd, NULL );
         retVal = TCL_OK;
cleanExit:
         g_free( dd.dash );
         return retVal;
      }
   }
   else /* get value */
   {
      Tcl_SetResult( interp, 
            "libgnomecanvas-2.4.0 has a bug which prevents this from working",
            TCL_STATIC );
      return TCL_ERROR;
      /*
      ArtVpathDash *dd;
      g_object_get( obj, opt->propName, &dd, NULL );
      *ret = Tcl_NewListObj( 0, NULL );
      if( dd->dash && dd->n_dash > 0 )
      {
         int k;
         Tcl_ListObjAppendElement( NULL, *ret, Tcl_NewDoubleObj( dd->offset ) );
         for( k = 0; k < dd->n_dash; ++k )
            Tcl_ListObjAppendElement( NULL, *ret, 
                  Tcl_NewDoubleObj( dd->dash[k] ) );
      }
      */
   }
   return TCL_OK;
}

static void itemEventFunc( GtkWidget *widget, GdkEvent *event, 
      gpointer data)
{
   Gnocl_CanvasItemInfo *info = (Gnocl_CanvasItemInfo *)data;
   int ns;

   switch( event->type )
   {
      case GDK_MOTION_NOTIFY:  ns = GNOCL_CANVAS_ON_MOTION; break;
      case GDK_ENTER_NOTIFY:   ns = GNOCL_CANVAS_ON_ENTER; break;
      case GDK_LEAVE_NOTIFY:   ns = GNOCL_CANVAS_ON_LEAVE; break;
      case GDK_BUTTON_PRESS:   
      case GDK_2BUTTON_PRESS:   
      case GDK_3BUTTON_PRESS:   
                               ns = GNOCL_CANVAS_ON_BUTTON_PRESS; break;
      case GDK_BUTTON_RELEASE: ns = GNOCL_CANVAS_ON_BUTTON_RELEASE; break;
      default:  return;
   }

   if( info->scripts[ns] == NULL )
      return;

   switch( event->type )
   {
      case GDK_MOTION_NOTIFY:  
               {
                  GnoclPercSubst ps[] = {
                     { 'w', GNOCL_STRING },
                     { 'i', GNOCL_INT },
                     { 'x', GNOCL_DOUBLE },
                     { 'y', GNOCL_DOUBLE },
                     { 's', GNOCL_INT },
                     { 0 }
                  };
                  ps[0].val.str = info->canvasParams->name;
                  ps[1].val.i = info->id;
                  ps[2].val.d = event->motion.x;
                  ps[3].val.d = event->motion.y;
                  ps[4].val.i = event->motion.state;  
                  gnoclPercentSubstAndEval( info->canvasParams->interp, 
                        ps, info->scripts[ns], 1 );
               }
               return;
      case GDK_ENTER_NOTIFY:  
      case GDK_LEAVE_NOTIFY:
               {
                  GnoclPercSubst ps[] = {
                     { 'w', GNOCL_STRING },
                     { 'i', GNOCL_INT },
                     { 'x', GNOCL_DOUBLE },
                     { 'y', GNOCL_DOUBLE },
                     { 's', GNOCL_INT },
                     { 0 }
                  };
                  ps[0].val.str = info->canvasParams->name;
                  ps[1].val.i = info->id;
                  ps[2].val.d = event->crossing.x;
                  ps[3].val.d = event->crossing.y;
                  ps[4].val.i = event->crossing.state;  
                  gnoclPercentSubstAndEval( info->canvasParams->interp, 
                        ps, info->scripts[ns], 1 );
               }
               return;

       case GDK_BUTTON_PRESS:
       case GDK_2BUTTON_PRESS:
       case GDK_3BUTTON_PRESS:
       case GDK_BUTTON_RELEASE:
               {
                  GnoclPercSubst ps[] = {
                     { 'w', GNOCL_STRING },
                     { 'i', GNOCL_INT },
                     { 'x', GNOCL_DOUBLE },
                     { 'y', GNOCL_DOUBLE },
                     { 's', GNOCL_INT },
                     { 'b', GNOCL_INT },
                     { 't', GNOCL_STRING },
                     { 0 }
                  };
                  ps[0].val.str = info->canvasParams->name;
                  ps[1].val.i = info->id;
                  ps[2].val.d = event->button.x;
                  ps[3].val.d = event->button.y;
                  ps[4].val.i = event->button.state;  
                  ps[5].val.i = event->button.button;  
                  switch( event->type )
                  {
                     case GDK_BUTTON_PRESS:  ps[6].val.str = "buttonPress"; 
                                             break;
                     case GDK_2BUTTON_PRESS: ps[6].val.str = "button2Press"; 
                                             break;
                     case GDK_3BUTTON_PRESS: ps[6].val.str = "button3Press"; 
                                             break;
                     case GDK_BUTTON_RELEASE: ps[6].val.str = "buttonRelease"; 
                                              break;
                     default:  assert( 0 ); break;
                  }
                  gnoclPercentSubstAndEval( info->canvasParams->interp, 
                        ps, info->scripts[ns], 1 );
               }
               return;
      default:  break;
   }
}

int gnoclItemOptOnFunc( Tcl_Interp *interp, GnoclOption *opt, 
      GObject *obj, Tcl_Obj **ret )
{
   Gnocl_CanvasItemInfo *info = g_object_get_data( obj, "gnocl::info" );
   int sig;

   assert( info->item == GNOME_CANVAS_ITEM( obj ) );
   switch( opt->optName[3] )
   {
      case 'M':   sig = GNOCL_CANVAS_ON_MOTION;  break;
      case 'E':   sig = GNOCL_CANVAS_ON_ENTER;   break;
      case 'L':   sig = GNOCL_CANVAS_ON_LEAVE;   break;
      case 'B':   if( opt->optName[9] == 'P' )
                     sig = GNOCL_CANVAS_ON_BUTTON_PRESS;
                  else
                     sig = GNOCL_CANVAS_ON_BUTTON_RELEASE;
   }

   if( ret == NULL ) /* set value */
   {
      const char *cmd = Tcl_GetString( opt->val.obj );

      g_free( info->scripts[sig] );

      if( cmd && *cmd )
      {
         int k;

         for( k = 0; k < GNOCL_CANVAS_ON_COUNT; ++k )
            if( info->scripts[k] != NULL )
               break;

         info->scripts[sig] = g_strdup( cmd );

         if( k == GNOCL_CANVAS_ON_COUNT )
            g_signal_connect_data( obj, "event", (GCallback)itemEventFunc,
                  info, NULL, (GConnectFlags)0 );
      }
      else
      {
         int k;

         info->scripts[sig] = NULL;
         
         for( k = 0; k < GNOCL_CANVAS_ON_COUNT; ++k )
            if( info->scripts[k] != NULL )
               break;

         if( k == GNOCL_CANVAS_ON_COUNT )
            g_signal_handlers_disconnect_matched( obj, G_SIGNAL_MATCH_FUNC,
                   0, 0, NULL, (gpointer *)itemEventFunc, NULL );
      }
   }
   else /* get value */
   {
      if( info->scripts[sig] )
         *ret = Tcl_NewStringObj( info->scripts[sig], -1 );
      else
         *ret = Tcl_NewStringObj( NULL, 0 );
   }

   return TCL_OK;
}

int gnoclOptXY( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   double coords[2];
   char bufX[32];
   char bufY[32];
   strcpy( bufX, opt->propName );
   *strchr( bufX, '?' ) = 'x';
   strcpy( bufY, opt->propName );
   *strchr( bufY, '?' ) = 'y';

   if( ret == NULL ) /* set value */
   {
      int    k;
      if( Tcl_ListObjLength( interp, opt->val.obj, &k ) != TCL_OK || k != 2 )
      {
         Tcl_SetResult( interp, "list must contain exactly 2 elements", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      for( k = 0; k < 2; ++k )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, opt->val.obj, k, &tp ) != TCL_OK 
               || Tcl_GetDoubleFromObj( interp, tp, &coords[k] ) != TCL_OK )
         {
            return TCL_ERROR;
         }
      }

      g_object_set( obj, bufX, coords[0], bufY, coords[1], NULL );
   }
   else /* get value */
   {
      g_object_get( obj, bufX, &coords[0], bufY, &coords[1], NULL );
      *ret = Tcl_NewListObj( 0, NULL );
      Tcl_ListObjAppendElement( NULL, *ret, Tcl_NewDoubleObj( coords[0] ) );
      Tcl_ListObjAppendElement( NULL, *ret, Tcl_NewDoubleObj( coords[1] ) );
   }

   return TCL_OK;
}

static int getCoords( Tcl_Interp *interp, Tcl_Obj *obj, int idx, int n, 
      double *coords )
{
   int k;
   for( k = 0; k < n; ++k )
   {
      Tcl_Obj *tp;
      if( Tcl_ListObjIndex( interp, obj, idx + k, &tp ) != TCL_OK 
            || Tcl_GetDoubleFromObj( interp, tp, &coords[k] ) != TCL_OK )
      {
         return TCL_ERROR;
      }
   }
   return TCL_OK;
}

int gnoclCanvasAppendPath( Tcl_Interp *interp, Tcl_Obj *obj, int k, 
      GnomeCanvasPathDef *path )
{
   double coords[6];
   int no;
   if( Tcl_ListObjLength( interp, obj, &no ) != TCL_OK )
      return TCL_ERROR;
   while( k < no )
   {
      char    *txt;
      Tcl_Obj *tp;
      if( Tcl_ListObjIndex( interp, obj, k, &tp ) != TCL_OK )
         return TCL_ERROR;

      txt = Tcl_GetString( tp );
      switch( *txt )
      {
         case 'l':   if( strcmp( txt, "lineTo" ) == 0 )
                     {
                        if( getCoords( interp, obj, k + 1, 2, coords ) 
                              != TCL_OK )
                           return TCL_ERROR;
                        gnome_canvas_path_def_lineto( path, 
                              coords[0], coords[1] );
                        k += 3;
                        break;
                     }
                     else if( strcmp( txt, "lineToMoving" ) == 0 )
                     {
                        if( getCoords( interp, obj, k + 1, 2, coords ) 
                              != TCL_OK )
                           return TCL_ERROR;
                        gnome_canvas_path_def_lineto_moving( path, 
                              coords[0], coords[1] );
                        k += 3;
                        break;
                     }
                     /* fall through */
         case 'm':   if( strcmp( txt, "moveTo" ) == 0 )
                     {
                        if( getCoords( interp, obj, k + 1, 2, coords ) 
                              != TCL_OK )
                           return TCL_ERROR;
                        gnome_canvas_path_def_moveto( path, 
                              coords[0], coords[1] );
                        k += 3;
                        break;
                     }
                     /* fall through */
         case 'c':   if( strcmp( txt, "curveTo" ) == 0 )
                     {
                        if( getCoords( interp, obj, k + 1, 6, coords ) 
                              != TCL_OK )
                           return TCL_ERROR;
                        gnome_canvas_path_def_curveto( path, 
                              coords[0], coords[1], coords[2], coords[3], 
                              coords[4], coords[5] );
                        k += 7;
                        break;
                     }
                     else if( strcmp( txt, "close" ) == 0 )
                     {
                        gnome_canvas_path_def_closepath( path );
                        k += 1;
                        break;
                     }
                     else if( strcmp( txt, "closeCurrent" ) == 0 )
                     {
                        gnome_canvas_path_def_closepath_current( path );
                        k += 1;
                        break;
                     }
                     /* fall through */
         default:    Tcl_AppendResult( interp, "Unknown path type \"",
                           txt, "\"", NULL );
                     return TCL_ERROR;
      }
   }
   return TCL_OK;
}

int gnoclOptPath( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   int res = TCL_ERROR;

   if( ret == NULL ) /* set value */
   {
      double coords[2];
      GnomeCanvasPathDef *path;
      int no, start;

      if( Tcl_ListObjLength( interp, opt->val.obj, &no ) != TCL_OK )
         return TCL_ERROR;
      if( no == 0 )
      {
         g_object_set( obj, opt->propName, NULL, NULL );
         return TCL_OK;
      }
      if( no < 2 )
      {
         Tcl_SetResult( interp, 
               "list-of-coordinates must contain at least two elements", 
               TCL_STATIC );
         return TCL_ERROR;
      }

      path = gnome_canvas_path_def_new();

      /* first must be either "moveTo" or coords */
      if( getCoords( interp, opt->val.obj, 0, 2, coords ) == TCL_OK )
      {
         gnome_canvas_path_def_moveto( path, coords[0], coords[1] );
         start = 2;
      }
      else
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, opt->val.obj, 0, &tp ) != TCL_OK )
            goto cleanExit;
         if( strcmp( Tcl_GetString( tp ), "moveTo" ) != 0 )
         {
            Tcl_SetResult( interp, 
                  "path must start with either two coordinates or "
                  "with \"lineTo\"", TCL_STATIC );
            goto cleanExit;
         }
         start = 0;
      }
      if( gnoclCanvasAppendPath( interp, opt->val.obj, start, path ) != TCL_OK )
         goto cleanExit;

      g_object_set( obj, opt->propName, path, NULL );
      res = TCL_OK;
cleanExit:
      gnome_canvas_path_def_unref( path );
   }
   else /* get value */
   {
      GnomeCanvasPathDef *path;
      ArtBpath *art = NULL;
      g_object_get( obj, opt->propName, &path, NULL );
      *ret = Tcl_NewListObj( 0, NULL );
      if( path )
         art =  gnome_canvas_path_def_bpath( path );
      if( art )
      {
         int isClosed = 0;
         int k;
         for( k = 0; art[k].code != ART_END; ++k )
         {
            const char *cmd;

            if( isClosed )
            {
               switch( art[k].code )
               {
                  case ART_LINETO:
                           switch( art[k+1].code )
                           {
                              case ART_MOVETO:
                              case ART_MOVETO_OPEN:
                              case ART_END:
                                       isClosed = 0;
                                       Tcl_ListObjAppendElement( NULL, *ret, 
                                             Tcl_NewStringObj( "close", -1 ) );
                                       continue;
                              default: ;
                           }
                           break;
                  case ART_MOVETO:
                  case ART_MOVETO_OPEN:
                           assert( art[k-1].code == ART_CURVETO );
                           isClosed = 0;
                           Tcl_ListObjAppendElement( NULL, *ret, 
                                 Tcl_NewStringObj( "closeCurrent", -1 ) );
                           break;
                  default: ;
               }
            }

            switch( art[k].code )
            {
               case ART_MOVETO:       cmd = "moveTo"; isClosed = 1; break;
               case ART_MOVETO_OPEN:  cmd = "moveTo"; break;
               case ART_LINETO:       cmd = "lineTo"; break;
               case ART_CURVETO:      cmd = "curveTo"; break;
               case ART_END:          assert( 0 );
            }
            Tcl_ListObjAppendElement( NULL, *ret, 
                  Tcl_NewStringObj( cmd, -1 ) );
            if( art[k].code == ART_CURVETO )
            {
               Tcl_ListObjAppendElement( NULL, *ret, 
                     Tcl_NewDoubleObj( art[k].x1 ) );
               Tcl_ListObjAppendElement( NULL, *ret, 
                     Tcl_NewDoubleObj( art[k].y1 ) );
               Tcl_ListObjAppendElement( NULL, *ret, 
                     Tcl_NewDoubleObj( art[k].x2 ) );
               Tcl_ListObjAppendElement( NULL, *ret, 
                     Tcl_NewDoubleObj( art[k].y2 ) );
            }
            Tcl_ListObjAppendElement( NULL, *ret, 
                  Tcl_NewDoubleObj( art[k].x3 ) );
            Tcl_ListObjAppendElement( NULL, *ret, 
                  Tcl_NewDoubleObj( art[k].y3 ) );
         }
      }
      res = TCL_OK;
   }

   return res;
}

int gnoclOptPoints( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   if( ret == NULL ) /* set value */
   {
      GnomeCanvasPoints *points;
      int   noCoords;
      int   n;
      Tcl_Obj *coords = opt->val.obj;

      if( Tcl_ListObjLength( interp, coords, &noCoords ) != TCL_OK
            || noCoords < 4 || ( noCoords % 2 != 0 ) )
      {
         Tcl_SetResult( interp, "list-of-coordinates must contain an "
               "even numer of elements (at least four).", TCL_STATIC );
         return TCL_ERROR;
      }
      points = gnome_canvas_points_new( noCoords / 2 );
      for( n = 0; n < noCoords; ++n )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, coords, n, &tp ) != TCL_OK 
               || Tcl_GetDoubleFromObj( interp, tp, &points->coords[n] ) 
               != TCL_OK )
         {
            gnome_canvas_points_free( points );
            return TCL_ERROR;
         }
      }
      g_object_set( obj, opt->propName, points, NULL );
      gnome_canvas_points_free( points );
   }
   else /* get value */
   {
      GnomeCanvasPoints *points;
      g_object_get( obj, opt->propName, &points, NULL );
      *ret = Tcl_NewListObj( 0, NULL );
      if( points )
      {
         int k;
         for( k = 0; k < 2*points->num_points; ++k )
         {
            Tcl_ListObjAppendElement( NULL, *ret, 
                  Tcl_NewDoubleObj( points->coords[k] ) );
         }
         gnome_canvas_points_free( points );
      }
   }

   return TCL_OK;
}

int Gnoclcanvas_Init( Tcl_Interp *interp )
{

   /* printf( "Initializing gnocl canvas version %s\n", VERSION ); */

   if( Tcl_InitStubs( interp, "8.3", 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgRequire( interp, "Gnocl", VERSION, 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgProvide( interp, "GnoclCanvas", VERSION ) != TCL_OK )
      return TCL_ERROR;

   Tcl_CreateObjCommand( interp, "gnocl::canvas", gnoclCanvasCmd, NULL, NULL );
   
   return TCL_OK;
}

