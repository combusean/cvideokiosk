/*
 * $Id: app.c,v 1.1 2003/11/23 17:42:27 baum Exp $
 *
 * This file implements the toplevel container gnomeApp
 *
 * Copyright (c) 2001 - 2003 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2002-11: Updates for gnome-2.0 and 
            switched from GnoclWidgetOptions to GnoclOption
        07: cleanups, addToolbar
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption appOptions[] =
{
   { "-contents", GNOCL_STRING, NULL },
   { "-menuBar", GNOCL_STRING, NULL },
   { "-statusBar", GNOCL_STRING, NULL },
   { "-toolBar", GNOCL_STRING, NULL },
   /* from here on it's the same as windowOptions */
   { "-title", GNOCL_STRING, "title" },
   { "-allowShrink", GNOCL_BOOL, "allow-shrink" },
   { "-allowGrow", GNOCL_BOOL, "allow-grow" },
   { "-resizable", GNOCL_BOOL, "resizable" },
   { "-modal", GNOCL_BOOL, "modal" },
   { "-defaultWidth", GNOCL_INT, "default-width" },
   { "-defaultHeight", GNOCL_INT, "default-height" },
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand }, 
   { NULL }
};

static const int contentsIdx  = 0;
static const int menuBarIdx   = 1;
static const int statusBarIdx = 2;
static const int toolBarIdx   = 3;

typedef struct
{
   Gnocl_StringBool name;
   Gnocl_ObjBool    behavior;
   Gnocl_ObjBool    placement;
   Gnocl_IntBool    bandNum;
   Gnocl_IntBool    bandPos;
   Gnocl_IntBool    offset;
} AddToolBarOpts;

static GnoclWidgetOptions addToolBarOptions[] = 
{
   { "-name", GNOCL_STRING, GNOCL_OFFSET( AddToolBarOpts, name ) },
   { "-behavior", GNOCL_OBJ, GNOCL_OFFSET( AddToolBarOpts, behavior ) },
   { "-placement", GNOCL_OBJ, GNOCL_OFFSET( AddToolBarOpts, placement ) },
   { "-bandNum", GNOCL_INT, GNOCL_OFFSET( AddToolBarOpts, bandNum ) },
   { "-bandPos", GNOCL_INT, GNOCL_OFFSET( AddToolBarOpts, bandPos ) },
   { "-offset", GNOCL_INT, GNOCL_OFFSET( AddToolBarOpts, offset ) },
   { NULL, 0, 0 }
};

static void freeToolBarOpts( AddToolBarOpts *p )
{
   GNOCL_FREE_MEM_OPT( p->name );
   GNOCL_FREE_TCL_OPT( p->behavior );
   GNOCL_FREE_TCL_OPT( p->placement );
}


static int gnoclGetDockItemBehavior( Tcl_Interp *interp, Tcl_Obj *obj, 
      BonoboDockItemBehavior *pb )
{
   const char *txt[] = { "normal", "exclusive", "neverFloating", 
         "neverVertical", "neverHorizontal", "locked", NULL };
   BonoboDockItemBehavior behavs[] = { 
         BONOBO_DOCK_ITEM_BEH_NORMAL,
         BONOBO_DOCK_ITEM_BEH_EXCLUSIVE,
         BONOBO_DOCK_ITEM_BEH_NEVER_FLOATING,
         BONOBO_DOCK_ITEM_BEH_NEVER_VERTICAL,
         BONOBO_DOCK_ITEM_BEH_NEVER_HORIZONTAL,
         BONOBO_DOCK_ITEM_BEH_LOCKED };
   int k, no;

   *pb = BONOBO_DOCK_ITEM_BEH_NORMAL;

   if( Tcl_ListObjLength( interp, obj, &no ) != TCL_OK )
   {
      Tcl_SetResult( interp, "behavior must be a list of flags",
            TCL_STATIC );
      return TCL_ERROR;
   }

   for( k = 0; k < no; ++k )
   {
      int idx;
      Tcl_Obj *tp;

      if( Tcl_ListObjIndex( interp, obj, k, &tp ) != TCL_OK )
         return TCL_ERROR;

      if( Tcl_GetIndexFromObj( interp, tp, txt, "dock item behavior",
            TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;
      *pb |= behavs[idx];
   }

   return TCL_OK;
}

static int gnoclGetDockPlacement( Tcl_Interp *interp, Tcl_Obj *obj, 
      BonoboDockPlacement *place )
{
   const char *txt[] = { "top", "right", "bottom", "left", "floating", NULL };
   BonoboDockPlacement placements[] = { 
         BONOBO_DOCK_TOP, BONOBO_DOCK_RIGHT, BONOBO_DOCK_BOTTOM,
         BONOBO_DOCK_LEFT, BONOBO_DOCK_FLOATING };

   int idx;
   if( Tcl_GetIndexFromObj( interp, obj, txt, "scrollBar policy",
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;
   *place = placements[idx];

   return TCL_OK;
}


static int addToolBar( GnomeApp *app, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   AddToolBarOpts params;
   GtkWidget *toolBar;
   const gchar *name = NULL;
   BonoboDockItemBehavior behavior = BONOBO_DOCK_ITEM_BEH_NORMAL;
   BonoboDockPlacement placement = BONOBO_DOCK_TOP;

   if( objc < 3 )
   {
      /* app addToolBar toolBar */
      Tcl_WrongNumArgs( interp, 2, objv, "toolBar-ID ?option val ...?" );
            return TCL_ERROR;
   }

   toolBar = gnoclGetWidgetFromName( Tcl_GetString( objv[2] ), interp );
   if( toolBar == NULL )
      return TCL_ERROR;

   if( !GTK_IS_TOOLBAR( toolBar ) )
   {
      Tcl_AppendResult( interp, "window \"",  Tcl_GetString( objv[2] ), 
            "\" is not a toolBar.", (char *)NULL );
      return TCL_ERROR;
   }

   
   memset( &params, 0, sizeof( params ) );
   params.bandNum.val = 1;
   params.bandPos.val = 1;
   params.offset.val = 0;

   if( gnoclParseAllWidgetOpts( interp, objc, objv,  2,
         addToolBarOptions, (char *)&params ) != TCL_OK )
   {
      freeToolBarOpts( &params );
      return TCL_ERROR;
   }
   
   if( params.name.changed )
      name = params.name.val;
   else
      name = "";

   if( params.behavior.changed )
   {
      if( gnoclGetDockItemBehavior( interp, params.behavior.val, 
            &behavior ) != TCL_OK )
      {
         freeToolBarOpts( &params );
         return TCL_ERROR;
      }
   }

   if( params.placement.changed )
   {
      if( gnoclGetDockPlacement( interp, params.placement.val, 
            &placement ) != TCL_OK )
      {
         freeToolBarOpts( &params );
         return TCL_ERROR;
      }
   }

   gnome_app_add_toolbar( app, GTK_TOOLBAR( toolBar ), 
         name, behavior, placement, 
         params.bandNum.val, params.bandPos.val, params.offset.val );


   freeToolBarOpts( &params );

   return TCL_OK;
}

static int configure( Tcl_Interp *interp, GnomeApp *app, 
      GnoclOption options[] )
{
   /*
      FIXME: delete old child
      if( gtk_container_children( GTK_CONTAINER( parent ) ) != NULL )
   */
   if( options[contentsIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkWidget *childWidget = gnoclChildNotPacked( 
            options[contentsIdx].val.str, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      gnome_app_set_contents( app, childWidget );
   }

   if( options[menuBarIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkWidget *childWidget = gnoclGetWidgetFromName( 
            options[menuBarIdx].val.str, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      if( !GTK_CHECK_TYPE( childWidget, GTK_TYPE_MENU_BAR ) )
      {
         Tcl_AppendResult( interp, "window \"", 
               options[menuBarIdx].val.str, 
               "\" is not a menu Bar.", (char *)NULL );
         return TCL_ERROR;
      }
      gnome_app_set_menus( app, GTK_MENU_BAR( childWidget ) );

/*
printf( "menuBar: %p %p\n", childWidget, gtk_widget_get_toplevel( childWidget ) );
*/
   }
   if( options[statusBarIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkWidget *childWidget = gnoclGetWidgetFromName( 
            options[statusBarIdx].val.str, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      /* if( !GTK_IS_STATUSBAR( childWidget ) ) */
      if( !GNOME_IS_APPBAR( childWidget ) )
      {
         Tcl_AppendResult( interp, "window \"", 
               options[statusBarIdx].val.str, 
               "\" is not a app Bar.", (char *)NULL );
         return TCL_ERROR;
      }
      gnome_app_set_statusbar( app, childWidget );
      gnoclRegisterHintAppBar( NULL, GNOME_APPBAR( childWidget ) );

/*
printf( "statusBar: %p %p\n", childWidget, gtk_widget_get_toplevel( childWidget ) );
*/
   }
   if( options[toolBarIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkWidget *childWidget = gnoclGetWidgetFromName( 
            options[toolBarIdx].val.str, interp );
      if( childWidget == NULL )
         return TCL_ERROR;

      if( !GTK_IS_TOOLBAR( childWidget ) )
      {
         Tcl_AppendResult( interp, "window \"", 
               options[toolBarIdx].val.str, 
               "\" is not a toolBar.", (char *)NULL );
         return TCL_ERROR;
      }
      gnome_app_set_toolbar( app, GTK_TOOLBAR( childWidget ) );
   }

   /* gtk_widget_show_all( GTK_WIDGET( para->window ) ); */

   return TCL_OK;
}

static int appFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   const char *cmds[] = { GNOCL_STD_CMD_NAMES,
         "configure", "addToolBar",
         "message", "flash", "error", "warning", "question",
         "okCancel", "request", "progress",
         NULL };
   enum cmdIdx { GNOCL_STD_CMD_ENUMS,
         ConfigureIdx, AddToolBarIdx,
         MessageIdx, FlashIdx, ErrorIdx, WarningIdx, QuestionIdx,
         OkCancelIdx, RequestIdx, ProgressIdx };
   GnomeApp *app = GNOME_APP( data );
   int idx;

   if( Tcl_GetIndexFromObj( interp, objv[1], cmds, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case DeleteIdx:
            return gnoclDelete( interp, GTK_WIDGET( app ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     appOptions, G_OBJECT( app ) ) == TCL_OK )
               {
                  ret = configure( interp, app, appOptions );
               }
               gnoclClearOptions( appOptions );
               return ret;
            }
            break;
      case AddToolBarIdx:
               return addToolBar( app, interp, objc, objv );

      case MessageIdx:
      case FlashIdx:
      case ErrorIdx:
      case WarningIdx:
            {
               const char *txt;
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "text" );
                  return TCL_ERROR;
               }
               txt = Tcl_GetString( objv[2] );
               switch( idx )
               {
                  case MessageIdx:  gnome_app_message( app, txt ); 
                                    break;
                  case FlashIdx:    gnome_app_flash( app, txt );
                                    break;
                  case ErrorIdx:    gnome_app_error( app, txt );
                                    break;
                  case WarningIdx:  gnome_app_warning( app, txt );
                                    break;
                  default:          assert( 0 );
               }
            }
            break;
#if 0   /* TODO */
      case QuestionIdx:
      case OkCancelIdx:
      case RequestIdx:
            /* TODO:  */
            break;
      case ProgressIdx:
            {
               GtkProgress *progress;
               double perc;

               /* FIXME: is there a documented way? */
               if( app->statusBar == NULL )
               {
                  Tcl_SetResult( interp, "app has no status Bar", 
                        TCL_STATIC );
                  return TCL_ERROR; 
               }
               progress = gnome_appbar_get_progress( 
                     GNOME_APPBAR( app->statusBar ) );
               if( progress == NULL )
               {
                  Tcl_SetResult( interp, "status bar has no progress bar", 
                        TCL_STATIC );
                  return TCL_ERROR; 
               }
               if( objc != 3 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, "percent" );
                  return TCL_ERROR;
               }
               if( Tcl_GetDoubleFromObj( interp, objv[2], &perc ) != TCL_OK )
                  return TCL_ERROR;
               perc /= 100.;
               if( perc > 1. )
                  perc = 1.;
               else if( perc < 0 )
                  perc = 0;
               gtk_progress_set_percentage( progress, perc );
            }
#endif
   }

   return TCL_OK;
}


int gnoclAppCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int        ret;
   GnomeApp   *app;
   
   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "appName ?option val ...?" );
      return TCL_ERROR;
   }

   if( gnoclParseOptions( interp, objc - 1, objv + 1, appOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( appOptions );
      return TCL_ERROR;
   }

   app = GNOME_APP( gnome_app_new( Tcl_GetString( objv[1] ), NULL ) );

   /* FIXME: each window own accel_group */
   gtk_window_add_accel_group( GTK_WINDOW( app ), gnoclGetAccelGroup() );

   ret = gnoclSetOptions( interp, appOptions, G_OBJECT( app ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, app, appOptions );
   gnoclClearOptions( appOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( app ) );
      return TCL_ERROR;
   }

   /* TODO: if not -visible == 0 */
   gtk_widget_show( GTK_WIDGET( app ) );

   return gnoclRegisterWidget( interp, GTK_WIDGET( app ), appFunc );
}

