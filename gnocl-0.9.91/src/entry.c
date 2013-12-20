/*
 * $Id: entry.c,v 1.11 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the entry widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2004-02: added -data
        09: added cget
            added -widthChars
            removed getValue
   2003-01: fixed uninitialized variables
        12: added -value and -onChanged
        08: switched from GnoclWidgetOptions to GnoclOption
            gtk+ 2.0 related cleanups
   2002-04: updates for gtk 2.0
        09: added GnomeEntry, GnomeFileEntry, GnomeNumberEntry, 
            GnomePixmapEntry and GnomeIconEntry
   2001-03: Begin of developement
*/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption entryOptions[] =
{
   { "-variable", GNOCL_STRING, NULL },      /* 0 */
   { "-onChanged", GNOCL_STRING, NULL },     /* 1 */
   { "-value", GNOCL_STRING, NULL },         /* 2 */
   { "-data", GNOCL_OBJ, "", gnoclOptData },
   { "-editable", GNOCL_BOOL, "editable" }, 
   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, 
   { "-maxLength", GNOCL_INT, "max-length" },
   { "-name", GNOCL_STRING, "name" },
   { "-onActivate", GNOCL_OBJ, "activate", gnoclOptCommand }, 
   { "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand }, 
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup }, 
   { "-textVisible", GNOCL_BOOL, "visibility" }, 
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-widthChars", GNOCL_INT, "width-chars" },
   { "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup }, 
   { NULL }
};

static const int variableIdx  = 0;
static const int onChangedIdx = 1;
static const int valueIdx     = 2;

typedef struct
{
   GtkEntry    *entry;
   Tcl_Interp  *interp;
   char        *name;
   char        *variable;
   char        *onChanged;
   int         inSetVar;
} EntryParams;

static int setVal( GtkEntry *entry, const char *txt );
static void changedFunc ( GtkWidget *widget, gpointer data );

static int doCommand( EntryParams *para, const char *val, int background )
{
   if( para->onChanged )
   {
      GnoclPercSubst ps[] = {
         { 'w', GNOCL_STRING },  /* widget */
         { 'v', GNOCL_STRING },  /* value */
         { 0 }
      };

      ps[0].val.str = para->name;
      ps[1].val.str = val;

      return gnoclPercentSubstAndEval( para->interp, ps, para->onChanged, 
            background );
   }
   return TCL_OK;
}

static int setVal( GtkEntry *entry, const char *txt )
{
   int blocked = g_signal_handlers_block_matched( 
         G_OBJECT( entry ), G_SIGNAL_MATCH_FUNC,
         0, 0, NULL, (gpointer *)changedFunc, NULL );
   gtk_entry_set_text( entry, txt );

   if( blocked )
      g_signal_handlers_unblock_matched( 
            G_OBJECT( entry ), G_SIGNAL_MATCH_FUNC, 
            0, 0, NULL, (gpointer *)changedFunc, NULL );
   return TCL_OK;
}

static char *traceFunc( ClientData data,
      Tcl_Interp *interp, const char *name1, const char *name2, int flags)
{
   EntryParams *para = (EntryParams *)data;

   if( para->inSetVar == 0 && name1 )
   {
      const char *txt = name1 ? Tcl_GetVar2( interp, name1, name2, 0 ): NULL;
      if( txt )
      {
         setVal( para->entry, txt );
         doCommand( para, txt, 1 );
      }
   }

   return NULL;
}

static int setVariable( EntryParams *para, const char *val )
{
   if( para->variable && para->inSetVar == 0 )
   {
      const char *ret;
      para->inSetVar = 1;
      ret = Tcl_SetVar( para->interp, para->variable, val, TCL_GLOBAL_ONLY );
      para->inSetVar = 0;
      return ret == NULL ? TCL_ERROR : TCL_OK;
   }

   return TCL_OK;
}

static void changedFunc ( GtkWidget *widget, gpointer data )
{
   EntryParams *para = (EntryParams *)data;
   const char *val = gtk_entry_get_text( para->entry ); 
   setVariable( para, val );
   doCommand( para, val, 1 );
}

static void destroyFunc( GtkWidget *widget, gpointer data )
{
   EntryParams *para = (EntryParams *)data;

   gnoclForgetWidgetFromName( para->name );
   Tcl_DeleteCommand( para->interp, para->name );

   gnoclAttacheOptCmdAndVar( 
         NULL, &para->onChanged,
         NULL, &para->variable,
         "changed", G_OBJECT( para->entry ), 
         G_CALLBACK( changedFunc ), para->interp, traceFunc, para );

   g_free( para->variable );
   g_free( para->name );
   g_free( para );
}

static int configure( Tcl_Interp *interp, EntryParams *para,
      GnoclOption options[] )
{
   gnoclAttacheOptCmdAndVar( 
         &options[onChangedIdx], &para->onChanged,
         &options[variableIdx], &para->variable,
         "changed", G_OBJECT( para->entry ), 
         G_CALLBACK( changedFunc ), interp, traceFunc, para );
   
   if( options[variableIdx].status == GNOCL_STATUS_CHANGED 
         && options[valueIdx].status == 0  /* value is handled below */ 
         && para->variable != NULL )
   {
      /* if variable does not exist -> set it, else set widget state */
      const char *val = Tcl_GetVar( interp, para->variable, TCL_GLOBAL_ONLY );
      if( val == NULL )
      {
         val = gtk_entry_get_text( para->entry ); 
         setVariable( para, val );
      }
      else
         setVal( para->entry, val );
   }

   if( options[valueIdx].status == GNOCL_STATUS_CHANGED )
   {
      char *str = options[valueIdx].val.str;
      setVal( para->entry, str );
      setVariable( para, str );
   }

   return TCL_OK;
}

static int cget( Tcl_Interp *interp, EntryParams *para, 
      GnoclOption options[], int idx )
{
   Tcl_Obj *obj = NULL;
   if( idx == variableIdx )
      obj = Tcl_NewStringObj( para->variable, -1 );
   else if( idx == onChangedIdx )
      obj = Tcl_NewStringObj( para->onChanged ? para->onChanged : "", -1 );
   else if( idx == valueIdx )
      obj = Tcl_NewStringObj( gtk_entry_get_text( para->entry ), -1 );

   if( obj != NULL )
   {
      Tcl_SetObjResult( interp, obj );
      return TCL_OK;
   }

   return gnoclCgetNotImplemented( interp, options + idx );
}

static int entryFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmds[] = { "delete", "configure", "cget",
         "onChanged", NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx,
         OnChangedIdx };

   EntryParams *para = (EntryParams *)data;
   GtkWidget *widget = GTK_WIDGET( para->entry );
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
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     entryOptions, G_OBJECT( widget ) ) == TCL_OK )
               {
                  ret = configure( interp, para, entryOptions );
               }
               gnoclClearOptions( entryOptions );
               return ret;
            }
            break;

      case CgetIdx:
            {
               int     idx;

               switch( gnoclCget( interp, objc, objv, G_OBJECT( para->entry ), 
                     entryOptions, &idx ) )
               {
                  case GNOCL_CGET_ERROR:  
                           return TCL_ERROR;
                  case GNOCL_CGET_HANDLED:
                           return TCL_OK;
                  case GNOCL_CGET_NOTHANDLED:
                           return cget( interp, para, entryOptions, idx );
               }
            }

      case OnChangedIdx:
            {
               const char *txt = gtk_entry_get_text( para->entry );
               if( objc != 2 )
               {
                  Tcl_WrongNumArgs( interp, 2, objv, NULL );
                  return TCL_ERROR;
               }
               return doCommand( para, txt, 0 );
            }
   }

   return TCL_OK;
}

int gnoclEntryCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   EntryParams *para;
   int ret;
   
   if( gnoclParseOptions( interp, objc, objv, entryOptions ) 
         != TCL_OK )
   {
      gnoclClearOptions( entryOptions );
      return TCL_ERROR;
   }

   para = g_new( EntryParams, 1 );
   para->entry = GTK_ENTRY( gtk_entry_new( ) );
   para->interp = interp;
   para->variable = NULL;
   para->onChanged = NULL;
   para->inSetVar = 0;
   gtk_widget_show( GTK_WIDGET( para->entry ) );

   ret = gnoclSetOptions( interp, entryOptions, 
         G_OBJECT( para->entry ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, para, entryOptions );
   gnoclClearOptions( entryOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( para->entry ) );
      g_free( para );
      return TCL_ERROR;
   }

   para->name = gnoclGetAutoWidgetId();
   g_signal_connect( G_OBJECT( para->entry ), "destroy", 
         G_CALLBACK( destroyFunc ), para );

   gnoclMemNameAndWidget( para->name, GTK_WIDGET( para->entry ) );

   Tcl_CreateObjCommand( interp, para->name, entryFunc, para, NULL );

   Tcl_SetObjResult( interp, Tcl_NewStringObj( para->name, -1 ) );

   return TCL_OK;
}

