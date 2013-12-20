/*
 * $Id: helperFuncs.c,v 1.11 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements some helper functions
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
        05: added gnoclPosOffset from text.c
   2003-04: added gnoclPixbufFromObj
   2002-12: removed gnoclEventToString
            joined with convert.c 
        09: gnoclFindLabel, gnoclLabelParseULine
   2001-07: Begin of developement
 */

#include "gnocl.h"
#include <ctype.h>
#include <string.h>
#include <assert.h>

typedef struct 
{
   GtkWidget *widget;
   GtkType   type;
} FindWidgetStruct;

static void findChildIntern( GtkWidget *widget, gpointer data )
{
   FindWidgetStruct *fw = (FindWidgetStruct *)data;
   if( fw->widget == NULL )
   {
      if( GTK_CHECK_TYPE( widget, fw->type ) )
         fw->widget = widget;
      else if( GTK_IS_CONTAINER( widget ) )
         gtk_container_foreach( GTK_CONTAINER( widget ), 
               findChildIntern, data );
   }
}

GtkWidget *gnoclFindChild( GtkWidget *widget, GtkType type )
{
   FindWidgetStruct fw;
   fw.widget = NULL;
   fw.type = type;
   findChildIntern( widget, &fw );
   return fw.widget;
}

int gnoclPosOffset( Tcl_Interp *interp, const char *txt, int *offset )
{
   *offset = 0;
   /* pos[+-]offset */
   if( *txt == '+' || *txt == '-' )
   {
      if( sscanf( txt + 1, "%d", offset ) != 1 )
      {
         Tcl_AppendResult( interp, "invalid offset \"", txt + 1, 
               "\"", NULL );
         return TCL_ERROR;
      }
      if( *txt == '-' )
         *offset *= -1;
      for( ++txt; isdigit( *txt ); ++txt )
         ;
   }
   if( *txt )
   {
      Tcl_AppendResult( interp, "invalid appendix \"", txt, 
            "\"", NULL );
      return TCL_ERROR;
   }
   return TCL_OK;
}

int gnoclPercentSubstAndEval( Tcl_Interp *interp, GnoclPercSubst *ps, 
      const char *orig_script, int background )
{
   int        len = strlen( orig_script );
   const char *old_perc = orig_script;
   const char *perc;
   GString    *script = g_string_sized_new( len + 20 );
   int        ret;

   for( ; ( perc = strchr( old_perc, '%' ) ) != NULL;
         old_perc = perc + 2 )
   {
      g_string_sprintfa( script, "%.*s", perc - old_perc, old_perc );
      /* printf( "script: \"%s\"\n", script->str ); */
      if( perc[1] == '%' )
         g_string_append_c( script, '%' ); 
      else
      {
         int k = 0;
         while( ps[k].c && ps[k].c != perc[1] )
            ++k;
         if( ps[k].c == 0 )
         {
            printf( "DEBUG: unknown percent substitution %c\n", perc[1] );
            /*
            Tcl_AppendResult( interp, "unknown percent substitution" , 
                  (char *)NULL );
            g_string_free( script, 1 );
            return TCL_ERROR;
            */
            g_string_append_c( script, '%' ); 
            g_string_append_c( script, perc[1] ); 
         }
         else
         {
            switch( ps[k].type )
            {
               case GNOCL_STRING: 
                     /* FIXME: escape special characters: ' ', '\n', '\\' ...
                        or use Tcl_EvalObj? */
                     if( ps[k].val.str != NULL )
                     {
                        /* handle special characters correctly 
                           TODO: would it be better to escape 
                                 special characters? 
                        */
                        char *txt = Tcl_Merge( 1, &ps[k].val.str );
                        g_string_append( script, txt );
                        /*
                        printf( "percent string: \"%s\" -> \"%s\"\n", 
                              ps[k].val.str, txt );
                        */
                        Tcl_Free( txt );
                     }
                     else
                        g_string_append( script, "{}" );
                     break;
               case GNOCL_OBJ: 
                     if( ps[k].val.obj != NULL )
                     {
                        /* embedden 0s should be UTF encoded. Should
                           therefor also work. */
                        /*
                        g_string_sprintfa( script, "%s", 
                              Tcl_GetString( ps[k].val.obj ) );
                        */
                        const char *argv[2] = { NULL, NULL };
                        char *txt;
                        argv[0] = Tcl_GetString( ps[k].val.obj );
                        txt = Tcl_Merge( 1, argv );
                        g_string_append( script, txt );
                        Tcl_Free( txt );
                     }
                     else
                        g_string_append( script, "{}" );
                     break;
               case GNOCL_INT:
                     g_string_sprintfa( script, "%d", ps[k].val.i );
                     break;
               case GNOCL_BOOL:
                     g_string_sprintfa( script, "%d", ps[k].val.b != 0 );
                     break;
               case GNOCL_DOUBLE:
                     g_string_sprintfa( script, "%f", ps[k].val.d );
                     break;
               default:
                     assert( 0 );
                     break;
            }
         }
      }
   }
   g_string_append( script, old_perc );

   /* Tcl_EvalObj would be faster and more elegant, but incompatible: eg. 
   two consecutive percent substitutions without space */
   ret = Tcl_EvalEx( interp, script->str, -1, TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT );
   /*
   printf( "DEBUG: script in percEval: %s -> %d %s\n", script->str,
         ret, Tcl_GetString( Tcl_GetObjResult( interp ) ) );
   */
   g_string_free( script, 1 );
   if( background && ret != TCL_OK )
      Tcl_BackgroundError( interp );

   return ret;
}

int gnoclAttacheVariable( GnoclOption *newVar, char **oldVar, 
      const char *signal, GObject *obj, GCallback gtkFunc, 
      Tcl_Interp *interp, Tcl_VarTraceProc tclFunc, 
      gpointer data )
{
   if( *oldVar && ( newVar == NULL || newVar->status == GNOCL_STATUS_CHANGED ) )
      Tcl_UntraceVar( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, 
            tclFunc, data );

   if( newVar == NULL || newVar->status != GNOCL_STATUS_CHANGED 
         || newVar->val.str[0] == 0 )
   {
      /* no new variable -> delete all */
      if( *oldVar )
      {
         g_signal_handlers_disconnect_matched( obj,
               G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer *)gtkFunc, NULL );
         g_free( *oldVar );
         *oldVar = NULL;
      }
   }
   else
   {
      if( *oldVar == NULL )     /* new variable but old didn't exist */
         g_signal_connect( obj, signal, gtkFunc, data );
      else                      /* new variable and old did exist */
         g_free( *oldVar );

      *oldVar = newVar->val.str;    /* transfer ownership */
      newVar->val.str = NULL;

      Tcl_TraceVar( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, 
            tclFunc, data );
   }

   return TCL_OK;
}

int gnoclAttacheOptCmdAndVar( GnoclOption *newCmd, char **oldCmd, 
      GnoclOption *newVar, char **oldVar, 
      const char *signal, 
      GObject *obj, GCallback gtkFunc, 
      Tcl_Interp *interp, Tcl_VarTraceProc tclFunc, 
      gpointer data )
{
   const int wasConnected = *oldVar != NULL || *oldCmd != NULL;

   /* handle variable */
   if( newVar == NULL || newVar->status == GNOCL_STATUS_CHANGED )
   {
      if( *oldVar )
      {
         Tcl_UntraceVar( interp, *oldVar, 
               TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, tclFunc, data );
         g_free( *oldVar );
         *oldVar = NULL;
      }
   }
   if( newVar && newVar->status == GNOCL_STATUS_CHANGED 
         && *newVar->val.str != '\0' )
   {
      *oldVar = g_strdup( newVar->val.str );
      Tcl_TraceVar( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, 
            tclFunc, data );
   }

   /* handle command */
   if( newCmd == NULL || newCmd->status == GNOCL_STATUS_CHANGED )
   {
      if( *oldCmd )
      {
         g_free( *oldCmd );
         *oldCmd = NULL;
      }
   }
   if( newCmd && newCmd->status == GNOCL_STATUS_CHANGED 
         && *newCmd->val.str != '\0' )
   {
      *oldCmd = g_strdup( newCmd->val.str );
   }

   /* if cmd or var is set, we need the gtkFunc */
   if( *oldVar || *oldCmd )
   {
      if( wasConnected == 0 )
         g_signal_connect( G_OBJECT( obj ), signal, gtkFunc, data );
   }
   else if( wasConnected )
      g_signal_handlers_disconnect_matched( G_OBJECT( obj ),
            G_SIGNAL_MATCH_FUNC, 0, 0, NULL, (gpointer *)gtkFunc, NULL );

   return TCL_OK;
}


int gnoclGet2Boolean( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 )
{
   int no;
   if( Tcl_ListObjLength( interp, obj, &no ) == TCL_OK 
         && ( no == 2 || no == 1 ) )
   {
      if( no == 1 )
      {
         if( Tcl_GetBooleanFromObj( interp, obj, b1 ) != TCL_OK )
            return TCL_ERROR;
         *b2 = *b1;
         return TCL_OK;
      }
      else
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, 0, &tp ) == TCL_OK )
         {
            if( Tcl_GetBooleanFromObj( interp, tp, b1 ) != TCL_OK )
               return TCL_ERROR;
            if( Tcl_ListObjIndex( interp, obj, 1, &tp ) == TCL_OK )
            {
               if( Tcl_GetBooleanFromObj( interp, tp, b2 ) != TCL_OK )
                  return TCL_ERROR;
            }
            return TCL_OK;
         }
      }
   }
   Tcl_AppendResult( interp, "Expected boolean value or list of "
         "two boolean values but got \"", Tcl_GetString( obj ), "\"", NULL );

   return TCL_ERROR;
}

int gnoclGet2Int( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 )
{
   int no;
   if( Tcl_ListObjLength( interp, obj, &no ) == TCL_OK 
         && ( no == 2 || no == 1 ) )
   {
      if( no == 1 )
      {
         if( Tcl_GetIntFromObj( interp, obj, b1 ) != TCL_OK )
            return TCL_ERROR;
         *b2 = *b1;
         return TCL_OK;
      }
      else
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, 0, &tp ) == TCL_OK )
         {
            if( Tcl_GetIntFromObj( interp, tp, b1 ) != TCL_OK )
               return TCL_ERROR;
            if( Tcl_ListObjIndex( interp, obj, 1, &tp ) == TCL_OK )
            {
               if( Tcl_GetIntFromObj( interp, tp, b2 ) != TCL_OK )
                  return TCL_ERROR;
            }
            return TCL_OK;
         }
      }
   }
   Tcl_AppendResult( interp, "Expected integer value or list of "
         "two integer values but got \"", Tcl_GetString( obj ), "\"", NULL );

   return TCL_ERROR;
}

int gnoclGet2Double( Tcl_Interp *interp, Tcl_Obj *obj, double *b1, double *b2 )
{
   int no;
   if( Tcl_ListObjLength( interp, obj, &no ) == TCL_OK 
         && ( no == 2 || no == 1 ) )
   {
      if( no == 1 )
      {
         if( Tcl_GetDoubleFromObj( interp, obj, b1 ) != TCL_OK )
            return TCL_ERROR;
         *b2 = *b1;
         return TCL_OK;
      }
      else
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, 0, &tp ) == TCL_OK )
         {
            if( Tcl_GetDoubleFromObj( interp, tp, b1 ) != TCL_OK )
               return TCL_ERROR;
            if( Tcl_ListObjIndex( interp, obj, 1, &tp ) == TCL_OK )
            {
               if( Tcl_GetDoubleFromObj( interp, tp, b2 ) != TCL_OK )
                  return TCL_ERROR;
            }
            return TCL_OK;
         }
      }
   }
   Tcl_AppendResult( interp, "Expected float value or list of "
         "two float values but got \"", Tcl_GetString( obj ), "\"", NULL );

   return TCL_ERROR;
}

static int getScrollbarPolicy( Tcl_Interp *interp, Tcl_Obj *obj, 
      GtkPolicyType *pol )
{
   const char *txt[] = { "always", "never", "automatic", NULL };
   GtkPolicyType policies[] = { 
         GTK_POLICY_ALWAYS, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC };
   int on;

   if( Tcl_GetBooleanFromObj( NULL, obj, &on ) == TCL_OK )
      *pol = on ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER;
   else
   {
      int idx;
      if( Tcl_GetIndexFromObj( interp, obj, txt, "scrollbar policy",
            TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;
      *pol = policies[idx];
   }
   return TCL_OK;
}

int gnoclGetScrollbarPolicy( Tcl_Interp *interp, Tcl_Obj *obj, 
      GtkPolicyType *hor, GtkPolicyType *vert )
{
   int no;

   if( Tcl_ListObjLength( interp, obj, &no ) != TCL_OK  || no > 2 )
   {
      Tcl_SetResult( interp, "policy must be either a single value "
            "or a list with two elements.", TCL_STATIC );
      return TCL_ERROR;
   }

   if( no == 1 )
   {
      if( getScrollbarPolicy( interp, obj, hor ) != TCL_OK )
         return TCL_ERROR;
      *vert = *hor;
   }
   else
   {
      Tcl_Obj *tp;

      if( Tcl_ListObjIndex( interp, obj, 0, &tp ) != TCL_OK )
         return TCL_ERROR;
      if( getScrollbarPolicy( interp, tp, hor ) != TCL_OK )
         return TCL_ERROR;
      if( Tcl_ListObjIndex( interp, obj, 1, &tp ) != TCL_OK )
         return TCL_ERROR;
      if( getScrollbarPolicy( interp, tp, vert ) != TCL_OK )
         return TCL_ERROR;
   }

   return TCL_OK;
}

int gnoclGetSelectionMode( Tcl_Interp *interp, Tcl_Obj *obj,
      GtkSelectionMode *selection )
{
   const char *txt[] = { "single", "browse", "multiple", "extended", NULL };
   GtkSelectionMode modes[] = { GTK_SELECTION_SINGLE,
         GTK_SELECTION_BROWSE, GTK_SELECTION_MULTIPLE,
         GTK_SELECTION_EXTENDED };

   int idx;
   if( Tcl_GetIndexFromObj( interp, obj, txt, "selection modes",
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   *selection = modes[idx];

   return TCL_OK;
}

int gnoclGetOrientationType( Tcl_Interp *interp, Tcl_Obj *obj,
      GtkOrientation *orient )
{
   const char *txt[] = { "horizontal", "vertical", NULL };
   GtkOrientation types[] = { GTK_ORIENTATION_HORIZONTAL, 
         GTK_ORIENTATION_VERTICAL };

   int idx;
   if( Tcl_GetIndexFromObj( interp, obj, txt, "orientation",
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   *orient = types[idx];

   return TCL_OK;
}

GdkPixbuf *gnoclPixbufFromObj( Tcl_Interp *interp, GnoclOption *opt )
{
   char *txt = gnoclGetString( opt->val.obj );
   GError *error = NULL;
   GdkPixbuf *pix = gdk_pixbuf_new_from_file( txt, &error );

   assert( gnoclGetStringType( opt->val.obj ) == GNOCL_STR_FILE );

   if( pix == NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return NULL;
   }

   return pix;
}

