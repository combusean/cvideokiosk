/*
 * $Id: gnocl.c,v 1.44 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements a Tcl interface to GTK+ and Gnome 
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2006-02-27: version 0.9.91
   2005-02-27: version 0.9.90
               updates for GTK 2.4:
                  - fileChooser
                  - expander
                  - colorButton
                  - fontButton
                  - comboBox and comboEntry
   2004-12-07: version 0.5.18
               gnome VFS
               tag expressions in canvas
   2004-07-10: version 0.5.17
               many canvas improvements
   2004-04-21: version 0.5.16
               many small enhancements, most prominent: itemCget command
               for canvas
   2004-02-11: version 0.5.15
   2003-12-22: version 0.5.14
               - added {width,height,size}Group
               - added panel widget
               - added gnome session
   2003-11-10: version 0.5.13
   2003-09-29: version 0.5.12
               - cget
               - make install, pkgIndex.tcl
   2003-08-25: version 0.5.11
               - new library for gconf
               - added image to tree and list
   2003-07-21: version 0.5.10
               - renamed messageDialog to dialog
               - new widget separator
               - canvas has own library gnoclCanvas
   2003-06-16: version 0.5.9
               - replaced subcommand tooltip by configure
               - added -icon to window
   2003-04-16: version 0.5.8
               - added callback, some work on event loops
               - added subcommands collapse, expand, scrollToPosition, 
                 setSelection and option -single to subcommand add of 
                 tree and list widget
               - added scrollToPosition to text widget
               - image widget: renamed -size to -stockSize, added -size
               - new option -hasFocus for many widgets
               - added clipboard
   2003-03-03: version 0.5.7
               - new image widget
               - updated {file,color,font}Selection, 
               - removed gnocl::dnd, dnd works now via 
                  -{drag,drop}Targets and -on{Drag,Drop}Data
               - added "info allStockItems"
               - added -icon to button
               - messageDialog closes on return code break
               - removed GnoclWidgetOptions and related functions 
   2003-02-11: version 0.5.6
               - code cleanup for statusBar, eventBox, plug and socket
               - dialog is replaced by messageDialog
               - now three flavors of gnocl: pure gtk+, gtk+ and canvas
                 and gtk+, canvas and gnome
   2003-01-15: version 0.5.5
               - code cleanup for scale, scrolledWindow, toolBar
               - new options -value and -onChanged for entry widget 
               - more common functions for all check items (button, menu and
                 toolBar) and for all radio items
   2002-12-16: version 0.5.4 aka "The Return Of The Canvas"
               - canvas, appBar work with gnome 2.0
               - new canvas item bPath and richText
               - renamed -justification to -justify
               - code cleanup for text
               - removed gnocl::bind, binding works now via -on{Button,Key,...}
   2002-11-10: version 0.5.3
               - split menuItem in menu(Item|Sparator|CheckItem|RadioItem)
               - code cleanup for combo, paned, progressBar, optionMenu
               - renamed (menu|tool|status)bar to \1Bar
               - changed some (char *) to (const char *) (update for Tcl 8.4)
   2002-10-03: version 0.5.2
               - switched from GnoclWidgetOptions to GnoclOption
               - many cleanups, e.g. button has no more malloc'ed para
               - renamed -command to -onClicked, -onToggled resp.
               - new default value for -expand and -fill for box
               - docs in docbook format
   2002-07-30: version 0.5.1
               - list widget based on GtkListStore and GtkTreeView
               - tree widget
               - colorSelection and fontSelection
               - fileSelection now works
   2002-05-21: version 0.5.0
               - port to gtk 2.0
               - new GtkTextView and GtkTextBuffer are supported
   2002-02-19: version 0.0.10
               - "invoke" works now as in Tk (besides the return value)
               - bugfixes for all widgets with "-command" and 
                 "-variable" option
               - new widget scrolledWindow
   2002-01-21: version 0.0.9
               - added alignment to table and box
               - much work on table widget
               - new command "invoke" for most widgets with "-command" option
               - new widget eventBox
               - drag and drop support
   2001-12-21: version 0.0.8
               - remote activation via goad
               - list widget: columns can be sorted via a tcl callback 
                     function and cell text retrived
               - bind: detail for key and button event,
                     more percent substitution
               - parameter default height and width for window and app
               - toolbar icon loadable from file
               - pixmap widget
               - table is now a combination of gtk table and gtk frame
               - parameter [xy]Align and [xy]Pad for label
               - parameter icon for menuItem and optionMenu
   2001-11-18: version 0.0.7
               - scale, optionMenu, socket and plug widget
               - menu hints in appBar
               - scrollbars for canvas widget
               - accelerators for page change in notebook now work
               - new option -layout for box of boxType
   2001-10-17: version 0.0.6
               - spinButton
               - checkButton in menu
               - underlined accelerators for menu items
               - new command setValue, getValue for checkButton
   2001-09-07: version 0.0.5
               - gnome color and font picker
               - new std option -sensitive
               - %-substitution in command of checkButton
               - command addToolbar for app
               - entry widget now combination of gtk entry, gnome entry,
                 file, number, pixmap, and icon entry
   2001-07-23: version 0.0.4
               - mainLoop 
               - keyboard accelerators
               - list, text and paned widget
               - templated documentation with parsed manpages.txt
   2001-06-07: version 0.0.3
               - notebook, dialog and messageBox widget
               - rectangle, line, text, widget canvas item
               - %-identification for strings
               - tooltips
   2001-05-06: version 0.0.2
               - info command
               - radio button widget
               - gnome about dialog
               - bind on canvas items
               - much work on canvas
               - box is now a combination of gtk box and gtk frame
               - start of documentation
               - connect to signal "destroy" for cleanups
   2001-04-03: first official version (0.0.1) 
               announced in comp.lang.tcl.announce
   2001-03:    Begin of developement
 */

#include "gnocl.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <assert.h>

static GHashTable *name2widgetList;

static const char idPrefix[] = "::gnocl::_WID";

static void simpleDestroyFunc( GtkWidget *widget, gpointer data )
{
   const char *name = gnoclGetNameFromWidget( widget ); 
   gnoclForgetWidgetFromName( name );
   Tcl_DeleteCommand( (Tcl_Interp *)data, (char *)name );
   g_free( (char *)name );
}

int gnoclRegisterWidget( Tcl_Interp *interp, GtkWidget *widget, 
      Tcl_ObjCmdProc *proc )
{
   const char *name = gnoclGetAutoWidgetId();
   gnoclMemNameAndWidget( name, widget );

   g_signal_connect_after( G_OBJECT( widget ), "destroy", 
         G_CALLBACK( simpleDestroyFunc ), interp );

   if( proc != NULL )
      Tcl_CreateObjCommand( interp, (char *)name, proc, widget, NULL );
   Tcl_SetObjResult( interp, Tcl_NewStringObj( name, -1 ) );

   return TCL_OK;
}

GtkWidget *gnoclChildNotPacked( const char *name, Tcl_Interp *interp )
{
   GtkWidget *child = gnoclGetWidgetFromName( name, interp );

   if( child == NULL )
      return NULL;

   if( gnoclAssertNotPacked( child, interp, name ) )
      return NULL;

   return child;
}

int gnoclAssertNotPacked( GtkWidget *child, Tcl_Interp *interp, 
      const char *name )
{
   if( gtk_widget_get_parent( child ) != NULL )
   {
      if( name && interp )
         Tcl_AppendResult( interp, "window \"", name,
               "\" has already a parent.", (char *)NULL );
      return 1;
   }

   return 0;
}

/* 
   "char *" and not "const char *" because of a not very strict
   handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/
char *gnoclGetAutoWidgetId( void )
{
   static int no = 0;
   /*
   static char buffer[30];
   */

   char *buffer = g_new( char, sizeof( idPrefix ) + 15 );
   strcpy( buffer, idPrefix );

   /* with namespace, since the Id is also the widget command */
   sprintf( buffer + sizeof( idPrefix ) - 1, "%d", ++no );

   return buffer;
}

/* -----------------
   handle widget <-> name mapping
-------------------- */
GtkWidget *gnoclGetWidgetFromName( const char *id, Tcl_Interp *interp )
{
   GtkWidget *widget = NULL;
   int       n;

   if( strncmp( id, idPrefix, sizeof( idPrefix ) - 1 ) == 0 
         && (n = atoi( id + sizeof( idPrefix ) - 1 ) ) > 0 )
   {
      widget = g_hash_table_lookup( name2widgetList, GINT_TO_POINTER( n ) );
   }
   if( widget == NULL && interp != NULL )
      Tcl_AppendResult( interp, "Unknown widget \"", id, "\".", (char *)NULL );

   return widget;
}

const char *gnoclGetNameFromWidget( GtkWidget *widget )
{
   const char *name = g_object_get_data( G_OBJECT( widget ), "gnocl::name" );
   if( name == NULL && GTK_IS_TREE_VIEW( widget ) )
      name = gnoclGetNameFromWidget( gtk_widget_get_parent( widget ) );

   return name;
}

int gnoclMemNameAndWidget( const char *path, GtkWidget *widget )
{
   int n = atoi( path + sizeof( idPrefix ) - 1 );

   assert( n > 0 );
   assert( g_hash_table_lookup( name2widgetList, GINT_TO_POINTER( n ) ) 
         == NULL );
   assert( strncmp( path, idPrefix, sizeof( idPrefix ) - 1 ) == 0 );

   /* memorize the name of the widget in the widget */
   g_object_set_data( G_OBJECT( widget ), "gnocl::name", (char *)path );
   g_hash_table_insert( name2widgetList, GINT_TO_POINTER( n ), widget );

   return 0;
}
int gnoclForgetWidgetFromName( const char *path )
{
   int n = atoi( path + sizeof( idPrefix ) - 1 );
   assert( gnoclGetWidgetFromName( path, NULL ) );
   assert( strncmp( path, idPrefix, sizeof( idPrefix ) - 1 ) == 0 );
   assert( n > 0 );

   g_hash_table_remove( name2widgetList, GINT_TO_POINTER( n ) );

   return 0;
}

int gnoclDelete( Tcl_Interp *interp, GtkWidget *widget,
      int objc, Tcl_Obj * const objv[] )
{
   if( objc != 2 )
   {
      /* widget delete */
      Tcl_WrongNumArgs( interp, 1, objv, "" );
      return TCL_ERROR;
   }
   /* TODO: all children*/
   gtk_widget_destroy( widget );
   return TCL_OK;
}

/*
   Prefix Explanation
   %%     % plus rest of string as is
   %!     rest of string as is
   %#     stock text/icon/accelerator/... first '_' is underline character
   %_     text with underline

   else:  use defaultStringPrefix
   
   FIXME: what about translation?
*/

GnoclStringType gnoclGetStringType( Tcl_Obj *obj )
{
   char *name = Tcl_GetString( obj );
   if( *name == 0 )
      return GNOCL_STR_EMPTY;

   assert( GNOCL_STOCK_PREFIX[0] == '%' && GNOCL_STOCK_PREFIX[1] == '#' );

   if( *name == '%' )
   {
      /* if something is changed here, also change gnoclGetStringFromObj! */
      switch( name[1] )
      {
         case '!': 
         case '%': return GNOCL_STR_STR;

         case '#': return GNOCL_STR_STOCK | GNOCL_STR_UNDERLINE;
         case '/': return GNOCL_STR_FILE;
         case '_': return GNOCL_STR_UNDERLINE;
         case '<': return GNOCL_STR_MARKUP | GNOCL_STR_UNDERLINE;
      }
   }

   return GNOCL_STR_STR;
}

static char *getEscStringFromObj( Tcl_Obj *op, int *len, int useEscape )
{
   /* FIXME: how to use UTF-8? */
   char *ret;
   if( op == NULL )
      return NULL;

   ret = Tcl_GetStringFromObj( op, len );
   /* FIXME: ugly, but Tcl_GetByteArrayFromObj seems to be the right
      function for UTF-8 
   ret = (const char *)Tcl_GetByteArrayFromObj( op, len ); 
   ret = (const char *)Tcl_SetByteArrayLength( op, *len + 1 );
   ((char *)ret)[*len] = '\0';
   */

   if( useEscape && *ret == '%' )
   {
   /* printf( "stringEscape: %s", ret ); */
      /* if something is changed here, also change gnoclGetStringType! */
      switch( ret[1] )
      {
         case '/':                        /* GNOCL_STR_FILE */
         case '#':                        /* GNOCL_STR_STOCK */
         case '_':                        /* GNOCL_STR_UNDERLINE */
         case '<':                        /* GNOCL_STR_MARKUP */
         case '!':   ret += 2;            /* GNOCL_STR_STR */ 
                     if( len )   *len -= 2; 
                     break; 
         case '%':   ret += 1;            /* escaped % */
                     if( len )   *len -= 1; 
                     break; 
         /* translate
         case ???:   ret = _( ret + 2 );
                     if( len )   *len = strlen( ret );
                     break;
         */
      }
   /* printf( " -> %s\n", ret ); */
   }
   return ret;
}
char *gnoclGetStringFromObj( Tcl_Obj *op, int *len )
{
   /* TODO: global flag "gnoclNoStringEscape"? */
   return getEscStringFromObj( op, len, 1 );
}

char *gnoclGetString( Tcl_Obj *op )
{
   return gnoclGetStringFromObj( op, NULL );
}

char *gnoclStringDup( Tcl_Obj *op )
{
   int txtLen;
   const char *txt;
   if( op == NULL )
      return NULL;

   txt = gnoclGetStringFromObj( op, &txtLen );
   return g_memdup( txt, txtLen + 1 );
}

Tcl_Obj *gnoclGtkToStockName( const char *gtk )
{
   Tcl_Obj *ret;
   GString *name;

   assert( strncmp( gtk, "gtk-", 4 ) == 0 );

   name = g_string_new( NULL );
   for( gtk += 3; *gtk; ++gtk )
   {
      if( *gtk == '-' )
      {
         ++gtk;
         g_string_append_c( name, toupper( *gtk ) );
      }
      else
         g_string_append_c( name, *gtk );
   }

   ret = Tcl_NewStringObj( name->str, -1 );
   g_string_free( name, 1 );
   return ret;
}

static GString *createStockName( const char *init, Tcl_Obj *obj )
{
   int len;
   const char *name = getEscStringFromObj( obj, &len, 1 );   
   GString *gtkName = g_string_new( init );
   int   isFirst = 1;

   /* we can use isupper and tolower since the IDs use only ASCII */ 
   /* "AbcDef" is changed to "init-abc-def" */
   for( ; *name; ++name )
   {
      if( isupper( *name ) 
            || (isdigit( *name ) && (isFirst || !isdigit( name[-1] )) ) )
      {
         g_string_append_c( gtkName, '-' );
         g_string_append_c( gtkName, tolower( *name ) );
      }
      else
         g_string_append_c( gtkName, *name );
      isFirst = 0;
   }
   g_string_append_c( gtkName, 0 );

   return gtkName;
}

int gnoclGetStockItem( Tcl_Obj *obj, Tcl_Interp *interp, GtkStockItem *sp )
{
   GString *gtkName;

   gtkName = createStockName( "gtk", obj );

   /* printf( "%s -> %s\n", Tcl_GetString( obj ), gtkName->str ); */

   if( gtk_stock_lookup( gtkName->str, sp ) == 0 )
   {
      g_string_free( gtkName, 1 );
      gtkName = createStockName( "gnome-stock", obj );
      if( gtk_stock_lookup( gtkName->str, sp ) == 0 )
      {
         /* as last chance use the original string */
         int len;
         const char *name;   
         g_string_free( gtkName, 1 );

         name = getEscStringFromObj( obj, &len, 1 );   
         if( gtk_stock_lookup( name, sp ) == 0 )
         {
            /* TODO? test the original string? */
            if( interp != NULL )
               Tcl_AppendResult( interp, "unknown stock item \"", 
                     Tcl_GetString( obj ), "\"", (char *)NULL );
            return TCL_ERROR;
         }
      }
   }

   g_string_free( gtkName, 1 );

   return TCL_OK;
}

int gnoclGetImage( Tcl_Interp *interp, 
      Tcl_Obj *obj, GtkIconSize size, GtkWidget **widget )
{
   GnoclStringType type = gnoclGetStringType( obj );
   if( type == GNOCL_STR_EMPTY )
   {
      *widget = NULL;
   }
   else if( type & GNOCL_STR_STOCK )
   {
   #if 0
      int     len;
      char    *txt = gnoclGetStringFromObj( obj, &len );
      Tcl_Obj *obj2 = Tcl_NewStringObj( txt, len );
      int     idx;
      int     ret = Tcl_GetIndexFromObj( interp, obj2, opts, "option", 
                        TCL_EXACT, &idx );
      Tcl_DecrRefCount( obj2 );
      if( ret != TCL_OK )
         return ret;

      *widget = gnome_stock_pixmap_widget( window, opts[idx] );
   #endif
      GtkStockItem sp;
      if( gnoclGetStockItem( obj, interp, &sp ) != TCL_OK )
         return TCL_ERROR;
      *widget = gtk_image_new_from_stock( sp.stock_id, size );
      if( *widget == NULL )
      {
         Tcl_AppendResult( interp, "Unknown stock pixmap \"", 
               sp.stock_id, "\".", (char *)NULL );
         return TCL_ERROR;
      }
   }
   else if( type == GNOCL_STR_FILE )
   {
      char   *txt = gnoclGetStringFromObj( obj, NULL );
#ifdef GNOCL_USE_GNOME
      fprintf( stderr,
            "gtk_image_new_from_file seems to hang with gnome support.\n"
            "Please don't use menu icons if you need gnome support or\n"
            "comment the line USE_GNOME in the makefile to \n"
            "disable gnome support.\n"
            "If this works for you *with* USE_GNOME please mail your\n"
            "GTK+ and Gnome versions to peter@dr-baum.net.\n"
            "Thank you!\n" );
#endif
      *widget = gtk_image_new_from_file( txt );
   }
   else
   {
      Tcl_AppendResult( interp, "invalid pixmap type for \"", 
            Tcl_GetString( obj ), "\"", (char *)NULL );
      return TCL_ERROR;
   }

   return TCL_OK;
}

typedef struct 
{
   char *name;
   Tcl_ObjCmdProc *proc;
} GnoclCmd;

static GnoclCmd commands[] = {
   { "debug",           gnoclDebugCmd },
   { "callback",        gnoclCallbackCmd },
   { "clipboard",       gnoclClipboardCmd },
   { "configure",       gnoclConfigureCmd },
   { "info",            gnoclInfoCmd },
   { "mainLoop",        gnoclMainLoop },
   { "update",          gnoclUpdateCmd },

   /* Gtk Widgets */
   { "button",          gnoclButtonCmd },
   { "box",             gnoclBoxCmd }, 
   { "checkButton",     gnoclCheckButtonCmd },
   { "colorSelection",  gnoclColorSelectionCmd },
   { "combo",           gnoclComboCmd },
   { "dialog",          gnoclDialogCmd },
   { "entry",           gnoclEntryCmd },
   { "eventBox",        gnoclEventBoxCmd },
   { "fileSelection",   gnoclFileSelectionCmd },
   { "fontSelection",   gnoclFontSelectionCmd },
   { "image",           gnoclImageCmd },
   { "label",           gnoclLabelCmd },
   { "list",            gnoclListCmd },
   { "menu",            gnoclMenuCmd },
   { "menuBar",         gnoclMenuBarCmd },
   { "menuItem",        gnoclMenuItemCmd },
   { "menuCheckItem",   gnoclMenuCheckItemCmd },
   { "menuRadioItem",   gnoclMenuRadioItemCmd },
   { "menuSeparator",   gnoclMenuSeparatorCmd },
   { "notebook",        gnoclNotebookCmd },
   { "optionMenu",      gnoclOptionMenuCmd },
   { "paned",           gnoclPanedCmd },
   { "plug",            gnoclPlugCmd },
   { "progressBar",     gnoclProgressBarCmd },
   { "radioButton",     gnoclRadioButtonCmd },
   { "scale",           gnoclScaleCmd },
   { "scrolledWindow",  gnoclScrolledWindowCmd },
   { "separator",       gnoclSeparatorCmd },
   { "socket",          gnoclSocketCmd },
   { "spinButton",      gnoclSpinButtonCmd },
   { "statusBar",       gnoclStatusBarCmd },
   { "table",           gnoclTableCmd },
   { "text",            gnoclTextCmd },
   { "toolBar",         gnoclToolBarCmd },
   { "tree",            gnoclTreeCmd },
   { "window",          gnoclWindowCmd },
#if GTK_CHECK_VERSION(2,4,0)
   { "action",          gnoclActionCmd },
   { "colorButton",     gnoclColorButtonCmd },
   { "comboBox",        gnoclComboBoxCmd },
   { "comboEntry",      gnoclComboEntryCmd },
   { "expander",        gnoclExpanderCmd },
   { "fileChooser",     gnoclFileChooserCmd },
   { "fontButton",      gnoclFontButtonCmd },
#endif
#if GTK_CHECK_VERSION(2,6,0)
   { "aboutDialog",      gnoclAboutDialogCmd },
#endif

#ifdef GNOCL_USE_GNOME
   { "app",             gnoclAppCmd },
   { "appBar",          gnoclAppBarCmd },
#endif
   { NULL,           NULL },
};

/*
 *----------------------------------------------------------------------
 *
 * eventSetupProc --
 *
 *      This procedure implements the setup part of the gtk 
 *      event source.  It is invoked by Tcl_DoOneEvent before entering
 *      the notifier to check for events on all displays.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      ??? TODO
 *      the maximum
 *      block time will be set to 0 to ensure that the notifier returns
 *      control to Tcl even if there is no more data on the X connection.
 *
 *----------------------------------------------------------------------
 */
static void eventSetupProc( ClientData clientData, int flags)
{
   /* 
      TODO: make blockTime configurable 
            via Tcl_GetVar( "::gnocl::blockTime" )
   */
   Tcl_Time blockTime = { 0, 10000 };

   if( !(flags & TCL_WINDOW_EVENTS) )
   {
      /* under which circumstances do we get here? */
      return;
   }

   /* 
      is this the correct way? 
      polling is bad, but how to do better?
   */
   Tcl_SetMaxBlockTime( &blockTime ); 

   return;
}
/*
 *----------------------------------------------------------------------
 *
 * eventCheckProc --
 *
 *      This procedure checks for events sitting in the gtk event
 *      queue.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Moves queued events onto the Tcl event queue.
 *
 *----------------------------------------------------------------------
 */
 
typedef struct 
{
   Tcl_Event header;      /* Standard information for all events. */
   GdkEvent event;
} GnoclEvent;

/*
 *----------------------------------------------------------------------
 *
 * eventProc --
 *
 *      This procedure is called by Tcl_DoOneEvent when a window event
 *      reaches the front of the event queue.  This procedure is responsible
 *      for actually handling the event.
 *
 * Results:
 *      Returns 1 if the event was handled, meaning it should be removed
 *      from the queue.  Returns 0 if the event was not handled, meaning
 *      it should stay on the queue.  The event isn't handled if the
 *      TCL_WINDOW_EVENTS bit isn't set in flags, if a restrict proc
 *      prevents the event from being handled.
 *
 * Side effects:
 *      Whatever the event handlers for the event do.
 *
 *----------------------------------------------------------------------
 */


static int eventProc( Tcl_Event *evPtr, int flags )
{
   /* 
      with while loop more responsive than 
      with Tcl_SetMaxBlockTime( 0 ); 
      is this the right way?
   */
   /* assert( gtk_events_pending() ); 
      This is not true, since between eventCheckProc and eventProc
      the event can have been handled by the gtk event loop if we
      are in mainLoop 
   */
   gtk_main_iteration_do ( 0 );
   return 1;
}

static void eventCheckProc( ClientData clientData, int flags)
{
   /* FIXME: what to do with flags? */
   if( gtk_events_pending() )
   {
      /* Tcl_Time blockTime = { 0, 0 }; */
      GnoclEvent *gp = (GnoclEvent *)ckalloc( sizeof( GnoclEvent ) );
      gp->header.proc = eventProc;
      Tcl_QueueEvent( (Tcl_Event *)gp, TCL_QUEUE_TAIL );

      /* Tcl_SetMaxBlockTime( &blockTime ); */

      /* gp->event = *gtk_get_current_event( ); */
   }

   return;
}

#ifdef GNOCL_USE_GNOME
static GnomeAppBar *gnoclAppBar = NULL;
int gnoclRegisterHintAppBar( GtkWidget *widget, GnomeAppBar *appBar )
{
   gnoclAppBar = appBar;
   return 0;
}

GnomeAppBar *gnoclGetHintAppBar( GtkWidget *widget )
{
   if( gnoclAppBar != NULL )
   {
      GtkWidget *top = gtk_widget_get_toplevel( GTK_WIDGET( gnoclAppBar ) );
      GtkWidget *wTop = widget->parent;

      /*
      if( top == wTop )
         return gnoclAppBar;
      else 
      */
      while( wTop != NULL && GTK_CHECK_TYPE( wTop, GTK_TYPE_MENU ) )
      {
         wTop = gtk_menu_get_attach_widget( GTK_MENU( wTop ) );
         /* this happens for popup menus */
         if( wTop == NULL )
            break;
         if( gtk_widget_get_toplevel( wTop ) == top )
            return gnoclAppBar;
         wTop = wTop->parent;
      }
      printf( "DEBUG: gnoclAppBar not found\n" );
   }

   return NULL;
}

static void putHintInAppBar( GtkWidget* menuitem, gpointer data )
{
   /*
   MenuLabelParams *para = (MenuLabelParams *)data;
   const char *txt = gtk_object_get_data( GTK_OBJECT(menuitem),
         "gnocl_tooltip" );
   */
   const char *txt = (const char *)data;
   GnomeAppBar *appBar = gnoclGetHintAppBar( GTK_WIDGET( menuitem ) );
   if( appBar )
      gnome_appbar_set_status( appBar, txt );
}

static void removeHintFromAppBar( GtkWidget *menuitem, gpointer data )
{
   GnomeAppBar *appBar = gnoclGetHintAppBar( GTK_WIDGET( data ) );
   if( appBar )
      gnome_appbar_refresh( appBar );
}
#endif

GtkTooltips *gnoclGetTooltips( )
{
   static GtkTooltips *tooltips = NULL;
   if( tooltips == NULL )
      tooltips = gtk_tooltips_new( );
   return tooltips;
}

GtkAccelGroup *gnoclGetAccelGroup( )
{
   static GtkAccelGroup *accelGroup = NULL;
   if( accelGroup == NULL )
      accelGroup = gtk_accel_group_new();
   /* return gtk_accel_group_get_default( ); */
   return accelGroup;
}

static gint tclTimerFunc( gpointer data )
{
   /* Tcl_Interp *interp = (Tcl_Interp *)data; */
   while( Tcl_DoOneEvent( TCL_DONT_WAIT ) )
   {
      /* printf( "t" ); fflush( stdout ); */
      if( gtk_events_pending() )
         break;
   }

   /* TODO: or Tcl_ServiceAll(); ? */
   return 1;
}

int gnoclMainLoop( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   guint32 timeout = 100;

   /* TODO? flag for not looping in tclTimerFunc? */
   if( ( objc != 1 && objc != 3 ) || 
         ( objc == 3 && strcmp( Tcl_GetString( objv[1] ), "-timeout" ) ) )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "?-timeout val?" );
      return TCL_ERROR;
   }

   if( objc == 3 )
   {
      long    val;
      if( Tcl_GetLongFromObj( interp, objv[2], &val ) != TCL_OK )
         return TCL_ERROR;
      /* beware of problems with casting signed -> unsigned! */
      if( val < 0 )
      {
         Tcl_SetResult( interp, 
            "Timeout value must be greater or equal zero.", TCL_STATIC );
         return TCL_ERROR;
      }
      timeout = (guint32)val;
   }

   /* TODO
      this prevents the rekursive calls of tcl and gtk event loops 
      but then we can't use "vwait var" if var is set in a gnocl::callback
   Tcl_DeleteEventSource( eventSetupProc, eventCheckProc, interp );
   */

   /* if timeout == 0 don't call tcl's event loop */
   if( timeout > 0 )    
      g_timeout_add( timeout, tclTimerFunc, NULL );

   /* TODO? use gtk_idle_add( tclTimerFunc, NULL ); ? */

   gtk_main();

   return 0;
}

char **gnoclGetArgv( Tcl_Interp *interp, int *pargc )
{
   int  narg;
   char **argv;
   int k;

   typedef char *Pchar;

   Tcl_Obj *obj = Tcl_ObjGetVar2( interp, Tcl_NewStringObj( "::argv", -1 ), 
         NULL, 0 );
   if( obj == NULL 
         || Tcl_ListObjLength( NULL, obj, &narg ) != TCL_OK )

   {
      narg = 0;
   }

   argv = g_new( Pchar, narg + 2 );
   argv[0] = (char *)gnoclGetAppName( interp );
   for( k = 0; k < narg; ++k )
   {
      Tcl_Obj *child;
      if( Tcl_ListObjIndex( NULL, obj, k, &child ) == TCL_OK )
         argv[k+1] = Tcl_GetString( child );
      else
         argv[k+1] = "";
   }
   argv[narg+1] = NULL;

   *pargc = narg + 1;

   return argv;
}

const char *gnoclGetAppName( Tcl_Interp *interp )
{
   const char *ret = Tcl_GetVar( interp, "gnocl::appName", TCL_GLOBAL_ONLY );
   if( ret == NULL )
      ret = Tcl_GetVar( interp, "argv0", TCL_GLOBAL_ONLY );
   if( ret == NULL )
      ret = "gnocl";

   return ret;
}

const char *gnoclGetAppVersion( Tcl_Interp *interp )
{
   const char *ret = Tcl_GetVar( interp, "gnocl::appVersion", 
         TCL_GLOBAL_ONLY );
   if( ret == NULL )
      ret = VERSION;

   return ret;
}

int Gnocl_Init( Tcl_Interp *interp )
{
   char cmdBuf[128] = "gnocl::";
   int k;
   int argc;
   char **argv;
   char **argvp;

   /* printf( "Initializing gnocl version %s\n", VERSION ); */

   if( Tcl_InitStubs( interp, "8.3", 0 ) == NULL )
      return TCL_ERROR;

   /* TODO? set locale before every jump to Tcl? */
   Tcl_PutEnv( "LC_NUMERIC=C" );
   argvp = argv = gnoclGetArgv( interp, &argc );
   if( !gtk_init_check( &argc, &argvp ) )
   {
      Tcl_SetResult( interp, "could not initialize gtk", TCL_STATIC );
      return TCL_ERROR;
   }
   /* TODO: change argv in Tcl */
   g_free( argv );

   Tcl_CreateEventSource( eventSetupProc, eventCheckProc, interp );
   /* Tcl_CreateExitHandler( exitHandler, NULL ); */
   if( Tcl_PkgProvide( interp, "Gnocl", VERSION ) != TCL_OK )
      return TCL_ERROR;
   
   for( k = 0; commands[k].name; ++k )
   {
      strcpy( cmdBuf + 7, commands[k].name );
      Tcl_CreateObjCommand( interp, cmdBuf, commands[k]. proc, NULL, NULL );
   }
   
   name2widgetList = g_hash_table_new( g_direct_hash, g_direct_equal );


   /* FIXME: is there a more elegant way? */
   /*        use gtk_idle_add( tclTimerFunc, NULL ); ? */
   g_timeout_add( 100, tclTimerFunc, NULL );
   Tcl_SetMainLoop( gtk_main );

   return TCL_OK;
}

