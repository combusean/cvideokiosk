/*
 * $Id: statusBar.c,v 1.5 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the statusbar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2003-01: switched from GnoclWidgetOptions to GnoclOption
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption barOptions[] =
{
   { "-resizeGrip", GNOCL_BOOL, NULL },        /* 0 */
   { "-homogeneous", GNOCL_BOOL, "homogeneous" },
   { "-spacing", GNOCL_OBJ, "spacing", gnoclOptPadding },
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { NULL }
};

static const int resizeGripIdx = 0;

static int configure( Tcl_Interp *interp, GtkStatusbar *bar, 
      GnoclOption options[] )
{
   if( options[resizeGripIdx].status == GNOCL_STATUS_CHANGED )
      gtk_statusbar_set_has_resize_grip( bar, options[resizeGripIdx].val.b );

   return TCL_OK;
}

static int addChildren( GtkBox *box, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], int begin )
{
   GnoclOption packOptions[] = 
   {
      { "-expand", GNOCL_BOOL, NULL },    /* 0 */
      { "-fill", GNOCL_BOOL, NULL },      /* 1 */
      { "-padding", GNOCL_OBJ, NULL },    /* 2 */
      { NULL, 0, 0 }
   };
   const int expandIdx  = 0;
   const int fillIdx    = 1;
   const int paddingIdx = 2;

   int k, noChildren;
   int expand  = 1;
   int fill    = 1;
   int padding = 0;
   int ret = TCL_ERROR;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "widget-list ?option val ...?" );
      return TCL_ERROR;
   }

   if( Tcl_ListObjLength( interp, objv[2], &noChildren ) != TCL_OK )
   {
      Tcl_SetResult( interp, "widget-list must be proper list", 
            TCL_STATIC );
      return TCL_ERROR;
   }

   if( gnoclParseOptions( interp, objc - 2, objv + 2, packOptions ) != TCL_OK )
      goto cleanExit;
   
   if( packOptions[expandIdx].status == GNOCL_STATUS_CHANGED )
      expand = packOptions[expandIdx].val.b;
   if( packOptions[fillIdx].status == GNOCL_STATUS_CHANGED )
      fill = packOptions[fillIdx].val.b;
   if( packOptions[paddingIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( gnoclGetPadding( interp, packOptions[paddingIdx].val.obj, 
            &padding ) != TCL_OK )
         goto cleanExit;
   }

   for( k = 0; k < noChildren; ++k )
   {
      GtkWidget *childWidget;
      Tcl_Obj *tp;

      if( Tcl_ListObjIndex( interp, objv[2], k, &tp ) != TCL_OK )
         goto cleanExit;

      childWidget = gnoclChildNotPacked( Tcl_GetString( tp ), interp );
      if( childWidget == NULL )
         goto cleanExit;

      if( begin )
         gtk_box_pack_start( box, childWidget, expand, fill, padding );
      else
         gtk_box_pack_end( box, childWidget, expand, fill, padding );
   }

   ret = TCL_OK;

cleanExit:
   gnoclClearOptions( packOptions );

   return ret;
}

static int barFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnoclOption opts[] = { 
      { "-context", GNOCL_INT, NULL },
      { NULL }
   };

   static const char *cmds[] = { "delete", "configure", 
         "push", "pop", "remove", 
         "add", "addBegin", "addEnd", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, 
         PushIdx, PopIdx, RemoveIdx, 
         AddIdx, BeginIdx, EndIdx };
   GtkStatusbar *bar = GTK_STATUSBAR( data );
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
            return gnoclDelete( interp, GTK_WIDGET( bar ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     barOptions, G_OBJECT( bar ) ) == TCL_OK )
               {
                  ret = configure( interp, bar, barOptions );
               }
               gnoclClearOptions( barOptions );
               return ret;
            }
            break;
     case PushIdx:
            {
               int ret;
               int contextId = 0;
               if( objc < 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "text ?-context id?" );
                  return TCL_ERROR;
               }
               if( gnoclParseOptions( interp, objc - 2, objv + 2, opts ) 
                     != TCL_OK )
                  return TCL_ERROR;

               if( opts[0].status == GNOCL_STATUS_CHANGED )
                  contextId = opts[0].val.i;
               /*
               else
                  contextId = gtk_statusbar_get_context_id( bar,
                     Tcl_GetString( objv[2] ) );
               */
               ret = gtk_statusbar_push( bar, contextId, 
                     Tcl_GetString( objv[2] ) );

               Tcl_SetObjResult( interp, Tcl_NewIntObj( ret ) );
            }
            break;
      case PopIdx:
            {
               int contextId = 0;
               if( gnoclParseOptions( interp, objc - 1, objv + 1, opts ) 
                     != TCL_OK )
                  return TCL_ERROR;

               if( opts[0].status == GNOCL_STATUS_CHANGED )
                  contextId = opts[0].val.i;
               gtk_statusbar_pop( bar, contextId );
            }
            break;
      case RemoveIdx:
            {
               int contextId = 0;
               int no;

               if( objc < 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "no ?-context id?" );
                  return TCL_ERROR;
               }
               if( gnoclParseOptions( interp, objc - 2, objv + 2, opts ) 
                     != TCL_OK )
                  return TCL_ERROR;

               if( opts[0].status == GNOCL_STATUS_CHANGED )
                  contextId = opts[0].val.i;

               if( Tcl_GetIntFromObj( interp, objv[2], &no ) != TCL_OK )
                  return TCL_ERROR;
               gtk_statusbar_remove( bar, contextId, no );
            }
            break;
      case AddIdx:
      case BeginIdx:
      case EndIdx:
               return addChildren( GTK_BOX( bar ), interp, objc, objv,
                     idx != EndIdx );
            break;
   }

   return TCL_OK;
}

int gnoclStatusBarCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GtkStatusbar *bar;
   int          ret;
   
   if( gnoclParseOptions( interp, objc, objv, barOptions ) != TCL_OK )
   {
      gnoclClearOptions( barOptions );
      return TCL_ERROR;
   }

   bar = GTK_STATUSBAR( gtk_statusbar_new( ) );
   gtk_widget_show( GTK_WIDGET( bar ) );
   gtk_box_set_spacing( GTK_BOX( bar ), GNOCL_PAD );

   ret = gnoclSetOptions( interp, barOptions, G_OBJECT( bar ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, bar, barOptions );
   gnoclClearOptions( barOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( bar ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( bar ), barFunc );
}

