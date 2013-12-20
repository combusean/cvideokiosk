/*
 * $Id: applet.c,v 1.8 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements a Tcl interface to the GNOME applet API
 *
 * Copyright (c) 2001 - 2003 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * History:
        12: cleanups, new options and adaptations for gnoclGnome
            added menu
 * 2003-11-30:	Initial version by Akos Polster
 */

#include "../gnocl.h"
#include <assert.h>
#include <string.h>
#include <panel-applet.h>

static GnoclOption appletOptions[] =
{
   { "-child", GNOCL_STRING, NULL },            /* 0 */
   { "-onChangeOrientation", GNOCL_OBJ, NULL }, /* 1 */
   { "-onChangeSize", GNOCL_OBJ, NULL },        /* 2 */
   { "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
   { "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
   { "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
   { "-name", GNOCL_STRING, "name" },
   { "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
   { "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand }, 
   { "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
   { "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
   { "-onMotion", GNOCL_OBJ, "", gnoclOptOnMotion },
   { "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand }, 
   { "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { NULL }
};

static const int childIdx          = 0;
static const int onChangeOrientIdx = 1;
static const int onChangeSizeIdx   = 2;

/*
static void logFile( const char *txt ) 
{
   FILE *fp = fopen( "/tmp/applet.log", "a" );
   fputs( txt, fp );
   fclose( fp );
}
*/

static void sigFunc( PanelApplet *applet, gint arg1, gpointer data )
{
   GnoclPercSubst ps[] = {
      { 'w', GNOCL_STRING },	/* widget */
      { 0}
   };

   GnoclCommandData *cs = (GnoclCommandData *)data;

   ps[0].val.str = gnoclGetNameFromWidget( GTK_WIDGET( applet ) );

   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
}

static int configure( Tcl_Interp *interp, PanelApplet *applet, 
      GnoclOption options[] )
{
   if( options[childIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( gtk_bin_get_child( GTK_BIN( applet ) ) != NULL )
         gtk_container_remove( GTK_CONTAINER( applet ),
               gtk_bin_get_child( GTK_BIN( applet ) ) );
      if( *options[childIdx].val.str != 0 )
      {
         GtkWidget *childWidget = gnoclChildNotPacked( 
               options[childIdx].val.str, interp );
         if( childWidget == NULL )
            return TCL_ERROR;

         gtk_container_add( GTK_CONTAINER( applet ), childWidget );
      }
   }

   if( gnoclConnectOptCmd( interp, G_OBJECT( applet ), "change-orient",
         G_CALLBACK( sigFunc ), &options[onChangeOrientIdx], 
         NULL, NULL ) != TCL_OK )
      return TCL_ERROR;

   if( gnoclConnectOptCmd( interp, G_OBJECT( applet ), "change-size",
         G_CALLBACK( sigFunc ), &options[onChangeSizeIdx], 
         NULL, NULL ) != TCL_OK )
      return TCL_ERROR;

   return TCL_OK;
}

static int addMenuSeparator( PanelApplet *applet, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   BonoboUIComponent *popup = panel_applet_get_popup_component( applet );

   if( objc != 2 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, NULL );
      return TCL_ERROR;
   }

   bonobo_ui_component_set_translate( popup, "/popups/button3", 
         "<separator/>", NULL );

   return TCL_OK;
}

static void menuCallback( BonoboUIComponent *popup, gpointer data, 
      const char *cname )
{
   GnoclCommandData *cs = (GnoclCommandData *)data;
   GnoclPercSubst ps[] = {
      { 0}
   };

   if( cs->command )
      gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
}

static int addMenuItem( PanelApplet *applet, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-text", GNOCL_OBJ, NULL },
      { "-onClicked", GNOCL_OBJ, NULL },
      /* TODO { "-icon", GNOCL_OBJ, NULL }, */
      { NULL }
   };
   static const int textIdx      = 0;
   static const int onClickedIdx = 1;
   /* static const int iconIdx      = 2; */

   GnoclCommandData *cs;
   char buffer[1024];
   static int id = 0;
   char *label = NULL;
   char *pixtype = NULL;
   char *pixname = NULL;
   char *stockName = NULL;

   BonoboUIComponent *popup = panel_applet_get_popup_component( applet );

   if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
         options, NULL ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[textIdx].status == GNOCL_STATUS_CHANGED )
   {
      Tcl_Obj *obj = options[textIdx].val.obj;

      GnoclStringType type = gnoclGetStringType( obj );
      if( type & GNOCL_STR_STOCK )
      {
         GtkStockItem  stockItem;
         if( gnoclGetStockItem( obj, interp, &stockItem ) != TCL_OK )
            return TCL_ERROR;
         label = stockItem.label; 
         pixtype = "stock";
         pixname = stockItem.stock_id;
      }
      else
         label = gnoclGetString( obj );
   }

   if( label && *label)
      label = g_markup_escape_text( label, -1 );
   else
      label = "";
   if( pixtype && *pixtype )
      pixtype = g_markup_escape_text( pixtype, -1 );
   else
      pixtype = "";
   if( pixname && *pixname )
      pixname = g_markup_escape_text( pixname, -1 );
   else
      pixname = "";

   ++id;
   g_snprintf( buffer, sizeof( buffer ), 
         "<menuitem name=\"Item%d\" verb=\"Verb%d\" " 
         "_label=\"%s\" pixtype=\"%s\" pixname=\"%s\"/>", 
         id, id, label, pixtype, pixname );

   bonobo_ui_component_set_translate( popup, "/popups/button3", buffer, NULL );

   if( *label )
      g_free( label );
   if( *pixtype )
      g_free( pixtype );
   if( *pixname )
      g_free( pixname );

   cs = g_new( GnoclCommandData, 1 );
   cs->interp = interp;
   cs->command = NULL;
   if( options[onClickedIdx].status == GNOCL_STATUS_CHANGED )
      cs->command = g_strdup( Tcl_GetString( 
            options[onClickedIdx].val.obj ) );

   g_snprintf( buffer, sizeof( buffer ), "Verb%d", id );
   bonobo_ui_component_add_verb( popup, buffer, menuCallback, cs );

   if( stockName )
      g_free( stockName );
   gnoclClearOptions( options );

   return TCL_OK;
}

static int appletFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", 
         "addMenuSeparator", "addMenuItem",
         "getSize", "getOrientation", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, 
         AddMenuSeparatorIdx, AddMenuItemIdx,
         GetSizeIdx, GetOrientationIdx };
   PanelApplet *applet = PANEL_APPLET( data );
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
            return gnoclDelete( interp, GTK_WIDGET( applet ), objc, objv );

      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     appletOptions, G_OBJECT( applet ) ) == TCL_OK )
               {
                  ret = configure( interp, applet, appletOptions );
               }
               gnoclClearOptions( appletOptions );
               return ret;
            }
            break;
      case AddMenuItemIdx:
            return addMenuItem( applet, interp, objc, objv );

      case AddMenuSeparatorIdx:
            return addMenuSeparator( applet, interp, objc, objv );

      case GetSizeIdx:
            {
               int size;
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, NULL );
                  return TCL_ERROR;
               }
               size = panel_applet_get_size( applet );
               Tcl_SetObjResult( interp, Tcl_NewIntObj( size ) );
               return TCL_OK;
            }
      case GetOrientationIdx:
            {
               PanelAppletOrient orient;
               char *txt;
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, NULL );
                  return TCL_ERROR;
               }
               orient = panel_applet_get_orient( applet );
               switch( orient )
               {
                  case PANEL_APPLET_ORIENT_UP:    txt = "up"; break;
                  case PANEL_APPLET_ORIENT_DOWN:  txt = "down"; break;
                  case PANEL_APPLET_ORIENT_LEFT:  txt = "left"; break;
                  case PANEL_APPLET_ORIENT_RIGHT: txt = "right"; break;
               }
               Tcl_SetObjResult( interp, Tcl_NewStringObj( txt, -1 ) );
               return TCL_OK;
            }
   }

   return TCL_OK;
}

static void destroy( PanelApplet *applet, gpointer data )
{
   gtk_main_quit();
   Tcl_Exit( 0 );
}

static gboolean appletCallback( PanelApplet *applet, const gchar *iid, 
      gpointer data )
{
   GnoclPercSubst ps[] = {
      { 'w', GNOCL_STRING },	/* widget */
      { 'i', GNOCL_STRING },	/* application ID */
      { 0}
   };

   GnoclCommandData *cs = (GnoclCommandData *)data;
   int ret = TCL_OK;

   gtk_widget_show_all( GTK_WIDGET( applet ) );
   gnoclRegisterWidget( cs->interp, GTK_WIDGET( applet ), appletFunc );
   g_signal_connect_after( G_OBJECT( applet ), "destroy", 
         G_CALLBACK( destroy ), NULL );

   ps[0].val.str = gnoclGetNameFromWidget( GTK_WIDGET( applet ) );
   ps[1].val.str = iid;

   ret = gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
   g_free( cs->command );
   g_free( cs );

   if( ret == TCL_OK )
   {
      BonoboUIComponent *popup;
      popup = panel_applet_get_popup_component( applet );
      bonobo_ui_component_set( popup, "/", "<popups/>", NULL );
   }

   return ret == TCL_OK;
}

int gnoclAppletFactoryCmd(ClientData data, Tcl_Interp *interp, 
			  int objc, Tcl_Obj * const objv[])
{
   GnoclCommandData *cs;

   if( objc != 3 )
   {
      Tcl_WrongNumArgs(interp, 1, objv, "iid callback");
      return TCL_ERROR;
   }

   cs = g_new( GnoclCommandData, 1 );
   cs->command = g_strdup( Tcl_GetString( objv[2] ) );
   cs->interp = interp;

   panel_applet_factory_main( Tcl_GetString( objv[1] ), PANEL_TYPE_APPLET, 
         appletCallback, cs );

   return 0;
}
