/*
 * $Id: socket.c,v 1.6 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the socket widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-02: switched from GnoclWidgetOptions to GnoclOption
   2001-11: Begin of developement
 */

#include "gnocl.h"
#include <gdk/gdkx.h>
#include <string.h>
#include <assert.h>


static GnoclOption socketOptions[] =
{
   { "-plugID", GNOCL_OBJ, NULL },      /* 0 */
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-onPlugAdded", GNOCL_OBJ, "plug-added", gnoclOptCommand },
   { "-onPlugRemoved", GNOCL_OBJ, "plug-removed", gnoclOptCommand },
   { NULL },
};

static const int plugIDIdx;

static int configure( Tcl_Interp *interp, GtkSocket *socket, 
      GnoclOption options[] )
{
   if( options[plugIDIdx].status == GNOCL_STATUS_CHANGED )
   {
      long xid;
      if( Tcl_GetLongFromObj( interp, options[plugIDIdx].val.obj, &xid ) 
            != TCL_OK )
         return TCL_ERROR;
      gtk_socket_add_id( socket, xid );
   }
 
   return TCL_OK;
}

static int socketFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   const char *cmds[] = { "delete", "configure", "getID", "getPlugID", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, GetIDIdx, GetPlugIDIdx  };
   int idx;
   GtkSocket *socket = GTK_SOCKET( data );

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
            return gnoclDelete( interp, GTK_WIDGET( socket ), objc, objv );
      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     socketOptions, G_OBJECT( socket ) ) == TCL_OK )
               {
                  ret = configure( interp, socket, socketOptions );
               }
               gnoclClearOptions( socketOptions );
               return ret;
            }
            break;
      case GetIDIdx:
            {
               long xid;
               Tcl_Obj *val;
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, NULL );
                  return TCL_ERROR;
               }
               xid = GDK_WINDOW_XWINDOW( GTK_WIDGET( socket)->window );
               val = Tcl_NewLongObj( xid );
               Tcl_SetObjResult( interp, val );
            }
            break;
      case GetPlugIDIdx:
            {
               long     xid = 0;
               Tcl_Obj *val;
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, NULL );
                  return TCL_ERROR;
               }
               if( socket->plug_window )
                  xid = GDK_WINDOW_XWINDOW( socket->plug_window );
               val = Tcl_NewLongObj( xid ); 
               Tcl_SetObjResult( interp, val );
            }
            break;
   }

   return TCL_OK;
}

int gnoclSocketCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int       ret;
   GtkSocket *socket;
   
   if( gnoclParseOptions( interp, objc, objv, socketOptions ) != TCL_OK )
   {
      gnoclClearOptions( socketOptions );
      return TCL_ERROR;
   }

   socket = GTK_SOCKET( gtk_socket_new() );
   gtk_widget_show( GTK_WIDGET( socket ) );

   ret = gnoclSetOptions( interp, socketOptions, G_OBJECT( socket ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, socket, socketOptions );
   gnoclClearOptions( socketOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( socket ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( socket ), socketFunc );
}

