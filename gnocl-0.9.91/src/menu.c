/*
 * $Id: menu.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the menu widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
        10: switched from GnoclWidgetOptions to GnoclOption
   2002-04: updates for gtk 2.0
        09: accelerator for menuItems
   2001-03: Begin of developement
 */

/*
   TODO?: menu factory? But as tcl function!
*/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption menuOptions[] =
{
   { "-children", GNOCL_LIST, NULL },
   { "-tearoff", GNOCL_BOOL, NULL },
   { "-title", GNOCL_STRING, "tearoff-title" },
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { NULL }
};

static const int childrenIdx = 0;
static const int tearoffIdx  = 1;

int gnoclMenuShellAddChildren( Tcl_Interp *interp, GtkMenuShell *shell, 
      Tcl_Obj *children, int atEnd )
{
   int n, noChilds;

   if( Tcl_ListObjLength( interp, children, &noChilds ) != TCL_OK 
         || noChilds < 1 )
   {
      Tcl_SetResult( interp, 
            "Widget-list must be a list with at least one element", 
            TCL_STATIC );
      return TCL_ERROR;
   }
   for( n = 0; n < noChilds; ++n )
   {
      GtkWidget   *childWidget;
      Tcl_Obj     *tp;
      const char  *childName;

      if( Tcl_ListObjIndex( interp, children, n, &tp ) != TCL_OK )
         return TCL_ERROR;
      childName = Tcl_GetString( tp );

      childWidget = gnoclChildNotPacked( childName, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      if( !GTK_CHECK_TYPE( childWidget, GTK_TYPE_MENU_ITEM ) )
      {
         Tcl_AppendResult( interp, "child window \"", 
               childName, "\" is not a menu item.", (char *)NULL );
         return TCL_ERROR;
      }

      if( atEnd )
         gtk_menu_shell_append( shell, childWidget );
      else
         gtk_menu_shell_prepend( shell, childWidget );
   }

   return TCL_OK;
}

static int configure( Tcl_Interp *interp, GtkMenu *menu, 
      GnoclOption options[] )
{
   if( options[tearoffIdx].status == GNOCL_STATUS_CHANGED )
   {
      /* the tearoff widget is created on creation of the menu.
         Here it is only hidden or shown */
      GtkWidget *widget = GTK_WIDGET( GTK_MENU_SHELL( menu )->children->data ); 
      if( options[tearoffIdx].val.b )
         gtk_widget_show( widget );
      else
         gtk_widget_hide( widget );
   }

   if( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
   {
      return gnoclMenuShellAddChildren( interp, GTK_MENU_SHELL( menu ), 
            options[childrenIdx].val.obj, 1 );
   }
      
   return TCL_OK;
}

static int menuFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", 
         "add", "addBegin", "addEnd", "popup", "popdown", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, 
         AddIdx, BeginIdx, EndIdx, PopupIdx, PopdownIdx };

   GtkMenu *menu = GTK_MENU( data );
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
            return gnoclDelete( interp, GTK_WIDGET( menu ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     menuOptions, G_OBJECT( menu ) ) == TCL_OK )
               {
                  ret = configure( interp, menu, menuOptions );
               }
               gnoclClearOptions( menuOptions );
               return ret;
            }
            break;

      case AddIdx:      /* AddIdx and EndIdx is the same */
      case BeginIdx:
      case EndIdx:
            {
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "widget-list" );
                  return TCL_ERROR;
               }
               return gnoclMenuShellAddChildren( interp, 
                     GTK_MENU_SHELL( menu ), objv[2], idx != BeginIdx );
            }
      case PopupIdx:
            gtk_menu_popup( menu, NULL, NULL, NULL, NULL, 0, 0 );
            break;
      case PopdownIdx:
            gtk_menu_popdown( menu );
            break; 
   }
   return TCL_OK;
}

int gnoclMenuCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int       ret;
   GtkMenu   *menu;
   GtkWidget *tearoff;
   
   if( gnoclParseOptions( interp, objc, objv, menuOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( menuOptions );
      return TCL_ERROR;
   }

   /* set tearoff to "on" as default */
   menu = GTK_MENU( gtk_menu_new() );
   tearoff = gtk_tearoff_menu_item_new();
   gtk_menu_shell_append( GTK_MENU_SHELL( menu ), tearoff );
   gtk_widget_show( tearoff );
   gtk_widget_show( GTK_WIDGET( menu ) );

   ret = gnoclSetOptions( interp, menuOptions, G_OBJECT( menu ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, menu, menuOptions );
   gnoclClearOptions( menuOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( menu ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( menu ), menuFunc );
}

