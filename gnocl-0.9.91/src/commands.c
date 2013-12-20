/*
 * $Id: commands.c,v 1.9 2005/02/22 23:16:10 baum Exp $
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
   2003-03: split from gnocl.c
*/

#include "gnocl.h"
#include <string.h>
#include <ctype.h>

int gnoclInfoCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmd[] = { "version", "gtkVersion", 
         "hasGnomeSupport", "allStockItems", "breakpoint", NULL };
   enum optIdx { VersionIdx, GtkVersionIdx, 
         HasGnomeIdx, AllStockItems, BreakpointIdx };
   int idx;

   if( objc != 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "option" );
      return TCL_ERROR;
   }
   if( Tcl_GetIndexFromObj( interp, objv[1], cmd, "option", TCL_EXACT,
         &idx ) != TCL_OK )
      return TCL_ERROR;
   switch( idx )
   {
      case VersionIdx:     
            Tcl_SetObjResult( interp, Tcl_NewStringObj( VERSION, -1 ) );
            break;
      case GtkVersionIdx:
            {
               char buffer[128];
               sprintf( buffer, "%d.%d.%d", gtk_major_version,
                     gtk_minor_version, gtk_micro_version );
               Tcl_SetObjResult( interp, Tcl_NewStringObj( buffer, -1 ) );
            }
            break;
      case HasGnomeIdx:
            Tcl_SetObjResult( interp, Tcl_NewBooleanObj(
                  #ifdef GNOCL_USE_GNOME
                     1
                  #else
                     0
                  #endif
                  ) );
            break;
      case AllStockItems:
            {
               Tcl_Obj *res = Tcl_NewListObj( 0, NULL );
               GSList *ids = gtk_stock_list_ids();
               GSList *p;
               for( p = ids; p != NULL; p = p->next )
               {
                  char *txt = p->data;
                  int skip = 0;
                  /* FIXME: gtk-missing-image, gtk-dnd-multiple and gtk-dnd
                            fail lookup, why?
                  {
                     GtkStockItem sp;
                     printf( "%s lookup: %d\n", txt, 
                           gtk_stock_lookup( txt, &sp ) );
                  }
                  */

                  /* see createStockName and gnoclGetStockName */
                  if( strncmp( txt, "gtk", 3 ) == 0 )
                     skip = 3;
#ifdef GNOCL_USE_GNOME
                  else if( strncmp( txt, "gnome-stock", 11 ) == 0 )
                     skip = 11;
#endif
                  if( skip > 0 )
                  {
                     GString *name = g_string_new( NULL );
                     char *tp = txt + skip;
                     for( ; *tp; ++tp )
                     {
                        if( *tp == '-' )
                        {
                           ++tp;
                           g_string_append_c( name, toupper( *tp ) );
                        }
                        else
                           g_string_append_c( name, *tp );
                     }
                     Tcl_ListObjAppendElement( interp, res, 
                           Tcl_NewStringObj( name->str, -1 ) );
                     /* printf( "%s -> %s\n", (char *)p->data, name->str ); */
                     g_string_free( name, 1 );
                  }
                  else
                     Tcl_ListObjAppendElement( interp, res, 
                           Tcl_NewStringObj( txt, -1 ) );
                  g_free( p->data ); 
               }
               g_slist_free( ids );
               Tcl_SetObjResult( interp, res );
            }
            break;
      case BreakpointIdx:
            /* this is only for debugging */
            G_BREAKPOINT();
            break;
   }
   return TCL_OK;
}

int gnoclUpdateCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int nMax = 500;
   int n;

   if( objc != 1 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, NULL );
      return TCL_ERROR;
   }

   for( n = 0; n < nMax && gtk_events_pending(); ++n )
      gtk_main_iteration_do ( 0 );

   Tcl_SetObjResult( interp, Tcl_NewIntObj( n ) );

   return TCL_OK;
}

int gnoclConfigureCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-tooltip", GNOCL_BOOL, NULL },
      { "-defaultIcon", GNOCL_OBJ, NULL },
      { NULL }
   };
   const int tooltipIdx     = 0;
   const int defaultIconIdx = 1;

   int ret = TCL_ERROR;

   if( gnoclParseOptions( interp, objc, objv, options ) != TCL_OK )
      goto cleanExit;

   if( options[defaultIconIdx].status == GNOCL_STATUS_CHANGED )
   {
      GnoclStringType type = gnoclGetStringType( 
            options[defaultIconIdx].val.obj );
      switch( type )
      {
         case GNOCL_STR_EMPTY: 
               gtk_window_set_default_icon_list( NULL ); 
               break;
         case GNOCL_STR_FILE:
               {
                  GdkPixbuf *pix = gnoclPixbufFromObj( interp, 
                        options + defaultIconIdx );
                  GList *list = NULL;
                  if( pix == NULL )
                     goto cleanExit;
                  list = g_list_append( list, pix );
                  gtk_window_set_default_icon_list( list ); 
                  g_list_free( list );
               }
               break;
         default:
               Tcl_AppendResult( interp, "Unknown type for \"", 
                     Tcl_GetString( options[defaultIconIdx].val.obj ),
                     "\" must be of type FILE (%/) or empty", NULL );
               goto cleanExit;
      }
   }

   if( options[tooltipIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( options[tooltipIdx].val.b )
         gtk_tooltips_enable( gnoclGetTooltips() );
      else
         gtk_tooltips_disable( gnoclGetTooltips() );
   }
   
   ret = TCL_OK;

cleanExit:
   gnoclClearOptions( options );
   return ret;
}

int gnoclClipboardCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-primary", GNOCL_BOOL, NULL },
      { NULL }
   };
   const int usePrimaryIdx = 0;

   static const char *cmd[] = { "hasText", "setText", "getText", "clear",
         NULL };
   enum optIdx { HasTextIdx, SetTextIdx, GetTextIdx, ClearIdx };
   int idx;
   int optNum;
   GtkClipboard *clip;
   int usePrimary = 0;

   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "option" );
      return TCL_ERROR;
   }
   if( Tcl_GetIndexFromObj( interp, objv[1], cmd, "option", TCL_EXACT,
         &idx ) != TCL_OK )
      return TCL_ERROR;
   if( idx == SetTextIdx )
   {
      optNum = 2;
      if( objc < 3 )
      {
         Tcl_WrongNumArgs( interp, 1, objv, "text ?option value?" );
         return TCL_ERROR;
      }
   }
   else
   {
      optNum = 1;
      if( objc < 2 )
      {
         Tcl_WrongNumArgs( interp, 1, objv, NULL );
         return TCL_ERROR;
      }
   }

   if( gnoclParseOptions( interp, objc - optNum, objv + optNum, options ) 
         != TCL_OK )
      return TCL_ERROR;

   if( options[usePrimaryIdx].status == GNOCL_STATUS_CHANGED )
      usePrimary = options[usePrimaryIdx].val.b;
      
   clip = gtk_clipboard_get( usePrimary ? gdk_atom_intern( "PRIMARY", 1 ) 
         : GDK_NONE );

   switch( idx )
   {
      case HasTextIdx:     
               {
                  int ret = gtk_clipboard_wait_is_text_available( clip );
                  Tcl_SetObjResult( interp, Tcl_NewBooleanObj( ret ) );
               }
               break;
      case SetTextIdx:
               gtk_clipboard_set_text( clip, Tcl_GetString( objv[2] ), -1 );
               break;
      case GetTextIdx:
               {
                  char *txt = gtk_clipboard_wait_for_text( clip );
                  if( txt )
                  {
                     Tcl_SetObjResult( interp, Tcl_NewStringObj( txt, -1 ) );
                     g_free( txt );
                  }
                  /* FIXME? else error? */
               }
               break;
      case ClearIdx:
               gtk_clipboard_clear( clip );
               break;
   }
   return TCL_OK;
}

