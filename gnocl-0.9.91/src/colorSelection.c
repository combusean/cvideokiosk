/*
 * $Id: colorSelection.c,v 1.5 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the file selection dialog
 *
 * Copyright (c) 2002 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:

   2003-02: cleanups with GnoclOption
   2002-07: start of developement
*/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption options[] = {
   { "-onClicked", GNOCL_STRING, NULL },         /* 0 */
   { "-modal", GNOCL_BOOL, "modal" },            /* 1 */
   { "-visible", GNOCL_BOOL, "visible" },        /* 2 */
   { "-name", GNOCL_STRING, "name" },            /* 3 */
   { "-title", GNOCL_STRING, "title" },          /* 4 */
   { "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand }, /* 5 */
   { "-color", GNOCL_OBJ, "current-color", gnoclOptGdkColor },  /* 6 */
   { "-alpha", GNOCL_INT, "current-alpha" },
   { "-palette", GNOCL_BOOL, "has-palette" },
   { "-opacity", GNOCL_BOOL, "has-opacity-control" },
   { NULL }
};

static const int commandIdx   = 0;
static const int modalIdx     = 1;
static const int colSelectIdx = 6;  /* first index which relates to the
                                    GtkColorSelection and not to the dialog */

typedef struct
{
   char             *name;
   Tcl_Interp       *interp;
   char             *onClicked;
   GtkColorSelectionDialog *colorSel;
} ColorSelParams;


static void onButtonFunc( ColorSelParams *para, int isOk )
{
   if( para->onClicked )
   {
     GnoclPercSubst ps[] = {
         { 'w', GNOCL_STRING },  /* widget */
         { 'x', GNOCL_STRING },  /* exit command */
         { 'r', GNOCL_INT },     /* red */
         { 'g', GNOCL_INT },     /* green */
         { 'b', GNOCL_INT },     /* blue */
         { 'a', GNOCL_INT },     /* alpha */
         { 0 }
      };
      GdkColor color;

      gtk_color_selection_get_current_color( 
            GTK_COLOR_SELECTION( para->colorSel->colorsel ), &color);

      ps[0].val.str = para->name;
      ps[1].val.str = isOk ? "OK" : "CANCEL";
      ps[2].val.i = color.red;
      ps[3].val.i = color.green;
      ps[4].val.i = color.blue;
      ps[5].val.i = gtk_color_selection_get_current_alpha( 
            GTK_COLOR_SELECTION( para->colorSel->colorsel ) );

      gnoclPercentSubstAndEval( para->interp, ps, para->onClicked, 1 );
   }
}

static void onOkFunc( GtkWidget *widget, gpointer data )
{
   onButtonFunc( (ColorSelParams *)data, 1 );
}

static void onCancelFunc( GtkWidget *widget, gpointer data )
{
   onButtonFunc( (ColorSelParams *)data, 0 );
}

static int colorSelFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx };
   ColorSelParams *para = (ColorSelParams *)data;
   GtkWidget *widget = GTK_WIDGET( para->colorSel );
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
            if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) 
                  == TCL_OK )
            {
               /* TODO? error if modalIdx has changed? */
               if( options[commandIdx].status == GNOCL_STATUS_CHANGED )
               {
                  para->onClicked = options[commandIdx].val.str;  
                  options[commandIdx].val.str = NULL;   /* avoid double free */
               }
               return TCL_OK;
            }
            return TCL_ERROR;
   }

   return TCL_OK;
}

int gnoclColorSelectionCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   ColorSelParams *para = NULL;
   int           ret = TCL_ERROR;
   int           isModal = 1;           /* default: is modal */

   assert( strcmp( options[commandIdx].optName, "-onClicked" ) == 0 );
   assert( strcmp( options[modalIdx].optName, "-modal" ) == 0 );
   assert( strcmp( options[colSelectIdx].optName, "-color" ) == 0 );

   if( gnoclParseOptions( interp, objc, objv, options )  != TCL_OK )
      goto cleanExit;

   para = g_new( ColorSelParams, 1 );
   para->colorSel = GTK_COLOR_SELECTION_DIALOG( 
         gtk_color_selection_dialog_new( "" ) );
   para->interp = interp;
   para->name = NULL;
   if( options[commandIdx].status == GNOCL_STATUS_CHANGED )
   {
      para->onClicked = options[commandIdx].val.str;  
      options[commandIdx].val.str = NULL;   /* avoid double free */

   }
   else
      para->onClicked = NULL;
   
   /* set default values */
   gtk_color_selection_set_has_palette(
            GTK_COLOR_SELECTION( para->colorSel->colorsel ), 1 );

   if( gnoclSetOptions( interp, options, G_OBJECT( para->colorSel ), 
         colSelectIdx ) != TCL_OK )
      goto cleanExit;
   if( gnoclSetOptions( interp, options + colSelectIdx, 
         G_OBJECT( para->colorSel->colorsel ), -1 ) != TCL_OK )
      goto cleanExit;

   if( options[modalIdx].status == GNOCL_STATUS_SET )
      isModal = options[modalIdx].val.b;
   else
      gtk_window_set_modal( GTK_WINDOW( para->colorSel ), isModal );

   g_signal_connect( GTK_OBJECT( para->colorSel->ok_button ),
         "clicked", G_CALLBACK( onOkFunc ), para );

   g_signal_connect(GTK_OBJECT( para->colorSel->cancel_button ),
         "clicked", G_CALLBACK( onCancelFunc ), para );

   gtk_widget_show( GTK_WIDGET( para->colorSel ) );

   if( isModal )
   {
      gint res = gtk_dialog_run( GTK_DIALOG( para->colorSel ) );
      if( res == GTK_RESPONSE_OK )
      {
         GdkColor color;
         Tcl_Obj *obj = Tcl_NewListObj( 0, NULL );

         gtk_color_selection_get_current_color( 
               GTK_COLOR_SELECTION( para->colorSel->colorsel ), &color);

         Tcl_ListObjAppendElement( interp, obj, Tcl_NewIntObj( color.red ) );
         Tcl_ListObjAppendElement( interp, obj, Tcl_NewIntObj( color.green ) );
         Tcl_ListObjAppendElement( interp, obj, Tcl_NewIntObj( color.blue ) );
         Tcl_ListObjAppendElement( interp, obj, Tcl_NewIntObj( 
               gtk_color_selection_get_current_alpha( 
               GTK_COLOR_SELECTION( para->colorSel->colorsel ) ) ) );
         Tcl_SetObjResult( interp, obj );
      }
      gtk_widget_destroy( GTK_WIDGET( para->colorSel ) );
   }
   else
   {
      para->name = gnoclGetAutoWidgetId();
      gnoclMemNameAndWidget( para->name, GTK_WIDGET( para->colorSel ) );
      gtk_widget_show( GTK_WIDGET( para->colorSel ) );

      Tcl_CreateObjCommand( interp, para->name, colorSelFunc, para, NULL );
      Tcl_SetObjResult( interp, Tcl_NewStringObj( para->name, -1 ) );
   }

   ret = TCL_OK;
cleanExit:
   /* TODO:
   if( ret != TCL_OK )
      freeParams
   */
      
   gnoclClearOptions( options );
   
   return ret;
}

