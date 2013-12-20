/*
 * $Id: paned.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the paned widget which is a combination of the
 * gtk paned widget (TODO? and the gtk frame widget?)
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-10: switched from GnoclWidgetOptions to GnoclOption
   2001-06: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption panedOptions[] =
{
   { "-orientation", GNOCL_OBJ, NULL },
   { "-resize", GNOCL_OBJ, NULL },
   { "-shrink", GNOCL_OBJ, NULL },
   { "-children", GNOCL_OBJ, NULL },
   { "-position", GNOCL_INT, "position" },
   /* What is this for? GNOCL_BOOL, "position-set" */
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
 
   { NULL },
};

static const int orientIdx = 0;
static const int resizeIdx = 1;
static const int shrinkIdx = 2;
static const int childrenIdx = 3;

static int addChildren( GtkPaned *paned, Tcl_Interp *interp, Tcl_Obj *children, 
      Tcl_Obj *resizeObj, Tcl_Obj *shrinkObj )
{
   int n, noChilds;
   int shrink[2] = { 1, 1};
   int resize[2] = { 1, 1};

   if( Tcl_ListObjLength( interp, children, &noChilds ) != TCL_OK 
         || noChilds != 2 )
   {
      Tcl_SetResult( interp, "widget-list must contain 2 elements", 
            TCL_STATIC );
      return TCL_ERROR;
   }

   if( shrinkObj )
   {
      if( gnoclGet2Boolean( interp, shrinkObj, shrink, shrink + 1 ) != TCL_OK )
         return TCL_ERROR;
   }
   if( resizeObj )
   {
      if( gnoclGet2Boolean( interp, resizeObj, resize, resize + 1 ) != TCL_OK )
         return TCL_ERROR;
   }

   for( n = 0; n < 2; ++n )
   {
      GtkWidget *childWidget;
      Tcl_Obj   *tp;
      const char *name;

      if( Tcl_ListObjIndex( interp, children, n, &tp ) != TCL_OK )
         return TCL_ERROR;

      name = Tcl_GetString( tp );
      if( *name == 0 )
         continue;

      childWidget = gnoclChildNotPacked( name, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      if( n == 0 )
         gtk_paned_pack1( paned, childWidget, resize[0], shrink[0] );
      else
         gtk_paned_pack2( paned, childWidget, resize[1], shrink[1] );
   }

   return TCL_OK;
}

static int configure( Tcl_Interp *interp, GtkPaned *paned, 
      GnoclOption options[] )
{
   /* FIXME?: create or delete frame if necessary 
              then also change: gnoclMemNameAndWidget( name, widget );
   */
#define VAL_IF_SET(idx) \
      options[idx].status == GNOCL_STATUS_CHANGED ? options[idx].val.obj : NULL 
   if( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( addChildren( paned, interp, options[childrenIdx].val.obj, 
            VAL_IF_SET(resizeIdx), VAL_IF_SET( shrinkIdx ) ) != TCL_OK )
         return TCL_ERROR;
   }
#undef VAL_IF_SET

   return TCL_OK;
}

static int panedFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = {  "delete", "configure", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx };
   GtkPaned *paned = GTK_PANED( data );
   int idx;

   if( Tcl_GetIndexFromObj( interp, objv[1], cmds, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case DeleteIdx:
            return gnoclDelete( interp, GTK_WIDGET( paned ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     panedOptions, G_OBJECT( paned ) ) == TCL_OK )
               {
                  ret = configure( interp, paned, panedOptions );
               }
               gnoclClearOptions( panedOptions );
               return ret;
            }
            break;
   }

   return TCL_OK;
}

int gnoclPanedCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int        ret;
   GtkPaned  *paned;
   GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
   
   if( gnoclParseOptions( interp, objc, objv, panedOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( panedOptions );
      return TCL_ERROR;
   }

   if( panedOptions[orientIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( gnoclGetOrientationType( interp, panedOptions[orientIdx].val.obj,
            &orient ) != TCL_OK )
      {
         gnoclClearOptions( panedOptions );
         return TCL_ERROR;
      }
   }

   if( orient == GTK_ORIENTATION_HORIZONTAL )
      paned = GTK_PANED( gtk_hpaned_new() );
   else
      paned = GTK_PANED( gtk_vpaned_new() );

   ret = gnoclSetOptions( interp, panedOptions, G_OBJECT( paned ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, paned, panedOptions );
   gnoclClearOptions( panedOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( paned ) );
      return TCL_ERROR;
   }

   /* TODO: if not -visible == 0 */
   gtk_widget_show( GTK_WIDGET( paned ) );

   return gnoclRegisterWidget( interp, GTK_WIDGET( paned ), panedFunc );
}

