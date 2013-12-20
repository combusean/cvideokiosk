/*
 * $Id: expander.c,v 1.2 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the expander widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2005-01: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption expanderOptions[] =
{
   { "-child", GNOCL_OBJ, "", gnoclOptChild }, 
   { "-expand", GNOCL_BOOL, "expanded" },
   { "-label", GNOCL_OBJ, "label", gnoclOptLabelFull }, 
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },  
   { "-visible", GNOCL_BOOL, "visible" },
   { NULL },
};

static int configure( Tcl_Interp *interp, GtkExpander *expander, 
      GnoclOption options[] )
{
   return TCL_OK;
}

static int expanderFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{

   const char *cmds[] = { "delete", "configure", "cget", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx };
   int idx;
   GtkExpander *expander = GTK_EXPANDER( data );

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
            return gnoclDelete( interp, GTK_WIDGET( expander ), objc, objv );
      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     expanderOptions, G_OBJECT( expander ) ) == TCL_OK )
               {
                  ret = configure( interp, expander, expanderOptions );
               }
               gnoclClearOptions( expanderOptions );
               return ret;
            }
            break;
      case CgetIdx:
            {
               int     idx;
               switch( gnoclCget( interp, objc, objv, G_OBJECT( expander ), 
                     expanderOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           assert( 0 );
               }
               assert( 0 );
            }
   }

   return TCL_OK;
}

int gnoclExpanderCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int         ret;
   GtkExpander *expander;
   
   if( gnoclParseOptions( interp, objc, objv, expanderOptions ) != TCL_OK )
   {
      gnoclClearOptions( expanderOptions );
      return TCL_ERROR;
   }

   expander = GTK_EXPANDER( gtk_expander_new( "" ) );
   gtk_widget_show( GTK_WIDGET( expander ) ); 

   ret = gnoclSetOptions( interp, expanderOptions, G_OBJECT( expander ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, expander, expanderOptions );
   gnoclClearOptions( expanderOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( expander ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( expander ), expanderFunc );
}

