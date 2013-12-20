/*
 * $Id: text.c,v 1.7 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the text widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2003-03: added scrollToPosition
        11: switched from GnoclWidgetOptions to GnoclOption
   2002-05: transition to gtk 2.0: move from gtkText to gtkTextView
   2001-06: Begin of developement

   TODO: marks
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>

/* as long as the options for the GtkScrolledWindow are not set 
   automatically, we don't need any special handling in gnoclSetOptions.
*/
static GnoclOption textOptions[] = 
{
   { "-scrollbar", GNOCL_OBJ, NULL },   
   { "-pixelsInsideWrap", GNOCL_INT, "pixels_inside_wrap" },
   { "-pixelsBelowLines", GNOCL_INT, "pixels_below_lines" },
   { "-pixelsAboveLines", GNOCL_INT, "pixels_above_lines" },
   { "-editable", GNOCL_BOOL, "editable" },
   { "-wrapMode", GNOCL_OBJ, "wrap_mode", gnoclOptWrapmode },
   { "-justify", GNOCL_OBJ, "justification", gnoclOptJustification },
   { "-leftMargin", GNOCL_INT, "left_margin" },
   { "-rightMargin", GNOCL_INT, "right_margin" },
   { "-indent", GNOCL_INT, "indent" },
   { "-cursorVisible", GNOCL_BOOL, "cursor_visible" },

   { "-hasFocus", GNOCL_BOOL, "has-focus" },
   { "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
   { "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp }, 
   { "-name", GNOCL_STRING, "name" },
   { "-visible", GNOCL_BOOL, "visible" }, 
   { "-sensitive", GNOCL_BOOL, "sensitive" }, 
   { NULL }
};

static const int scrollBarIdx = 0;

static int posToIter( Tcl_Interp *interp, Tcl_Obj *obj, 
      GtkTextBuffer *buffer, GtkTextIter *iter )
{
   char errMsg[] = "Position must be eiter a list of row and column "
            "or a keyword plus offset";
   char errEndOffset[] = "offset to \"end\" must be negativ";

   int len;
   if( Tcl_ListObjLength( interp, obj, &len ) != TCL_OK || len < 1 || len > 2 )
   {
      Tcl_SetResult( interp, errMsg, TCL_STATIC );
      return TCL_ERROR;
   }

   if( len == 2 )
   {
      int idx[2];
      int isEnd[2] = { 0, 0 };
      int k;
      for( k = 0; k < 2; ++k )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, k, &tp ) != TCL_OK )
         {
            Tcl_SetResult( interp, errMsg, TCL_STATIC );
            return TCL_ERROR;
         }
         if( Tcl_GetIntFromObj( NULL, tp, idx + k ) != TCL_OK )
         {
            char *txt = Tcl_GetString( tp );
            if( strncmp( txt, "end", 3 ) == 0 )
            {
               if( gnoclPosOffset( interp, txt + 3, idx + k ) != TCL_OK )
                  return TCL_ERROR;
               if( idx[k] > 0 )
               {
                  Tcl_SetResult( interp, errEndOffset, TCL_STATIC );
                  return TCL_ERROR;
               }
               isEnd[k] = 1;
            }
            else
            {
               Tcl_AppendResult( interp, "unknown row or column index \"", 
                     txt, "\" must be integer or end plus offset" );
               return TCL_ERROR;
            }
               
         }
      }

      gtk_text_buffer_get_start_iter( buffer, iter );
      if( isEnd[0] )
      {
         gtk_text_iter_set_line( iter, -1 );
         gtk_text_iter_backward_lines( iter, -idx[0] );
      }
      else
         gtk_text_iter_set_line( iter, idx[0] );

      if( isEnd[0] )
      {
         gtk_text_iter_forward_to_line_end( iter );
         gtk_text_iter_backward_chars( iter, -idx[1] );
      }
      else
         gtk_text_iter_forward_chars( iter, idx[1] );
   }
   else if( Tcl_GetIntFromObj( NULL, obj, &len ) == TCL_OK )
   {
      if( len < 0 )
      {
         Tcl_SetResult( interp, "character offset must be greater zero.", 
               TCL_STATIC );
         return TCL_ERROR;
      }
      gtk_text_buffer_get_iter_at_offset( buffer, iter, len );
   }
   else
   {
      const char *txt = Tcl_GetString( obj );
      const char *last;
      int offset;

      if( strncmp( txt, "end", 3 ) == 0 )
      {
         gtk_text_buffer_get_end_iter( buffer, iter );
         last = txt + 3;
      }
      else if( strncmp( txt, "cursor", 6 ) == 0 )
      {
         last = txt + 6;
         gtk_text_buffer_get_iter_at_mark( buffer, iter, 
                     gtk_text_buffer_get_insert( buffer ) );
      }
      else if( strncmp( txt, "selectionStart", 14 ) == 0 )
      {
         GtkTextIter end;
         gtk_text_buffer_get_selection_bounds( buffer, iter, &end );
         last = txt + 14;
      }
      else if( strncmp( txt, "selectionEnd", 12 ) == 0 )
      {
         GtkTextIter start;
         gtk_text_buffer_get_selection_bounds( buffer, &start, iter );
         last = txt + 12;
      }
      else
      {
         Tcl_AppendResult( interp, "unknown index \"", txt, 
               "\", must be a list of row and column, "
               "an integer as character offset, "
               "or one of end, cursor, selectionStart, or selectionEnd", 
               NULL );
         return TCL_ERROR;
      }
      if( gnoclPosOffset( interp, last, &offset ) != TCL_OK )
         return TCL_ERROR;
      if( offset > 0 )
         gtk_text_iter_forward_chars( iter, offset );
      else if( offset < 0 )
         gtk_text_iter_backward_chars( iter, -offset );
   }

   return TCL_OK;
}

static int tagCmd( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], int cmdNo )
{
   const char *cmds[] = { "create", "configure", "add", "delete", 
         NULL };
   enum cmdIdx { CreateIdx, ConfigureIdx, AddIdx, DeleteIdx };

   /* see also list.c */
   GnoclOption tagOptions[] = {
      { "-background", GNOCL_OBJ, "background-gdk", gnoclOptGdkColor },
      { "-foreground", GNOCL_OBJ, "foreground-gdk", gnoclOptGdkColor },
      { "-font", GNOCL_STRING, "font" }, 
      { "-fontFamily", GNOCL_STRING, "family" },
      { "-fontStyle", GNOCL_OBJ, "style", gnoclOptPangoStyle },
      { "-fontVariant", GNOCL_OBJ, "variant", gnoclOptPangoVariant },
      { "-fontWeight", GNOCL_OBJ, "weight", gnoclOptPangoWeight },
      { "-fontRise", GNOCL_OBJ, "rise", gnoclOptPangoScaledInt },
      { "-fontStretch", GNOCL_OBJ, "stretch", gnoclOptPangoStretch },
      { "-fontSize", GNOCL_OBJ, "size", gnoclOptPangoScaledInt },
      { "-fontScale", GNOCL_OBJ, "scale", gnoclOptScale },

      { "-wrapMode", GNOCL_OBJ, "wrap-mode", gnoclOptWrapmode },
      { "-justification", GNOCL_OBJ, "justification",gnoclOptJustification}, 
      { "-strikethrough", GNOCL_BOOL, "strikethrough" },
      { "-underline", GNOCL_OBJ, "underline", gnoclOptUnderline },
      { "-invisible", GNOCL_BOOL, "invisible" },
      { "-editable", GNOCL_BOOL, "editable" },
      { NULL }
   };

   int idx;
   if( objc < cmdNo + 1 )
   {
      Tcl_WrongNumArgs( interp, cmdNo, objv, "subcommand ?option val ...?" );
      return TCL_ERROR;
   }

   if( Tcl_GetIndexFromObj( interp, objv[cmdNo], cmds, "subcommand", 
         TCL_EXACT, &idx ) != TCL_OK )
      return TCL_ERROR;

   switch( idx )
   {
      case CreateIdx:
            {
               int ret;
               GtkTextTag *tag; 
               /* win tag create name */
               if( objc < cmdNo + 2 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo + 1, objv, 
                        "tag-name ?option val ...?" );
                  return TCL_ERROR;
               }
               /* tag = gtk_text_tag_new( Tcl_GetString( objv[3] ) ); */
               tag = gtk_text_buffer_create_tag( buffer, 
                     Tcl_GetString( objv[cmdNo+1] ), NULL );

               ret = gnoclParseAndSetOptions( interp, objc-cmdNo-1, 
                     objv+cmdNo+1, tagOptions, G_OBJECT( tag ) );
               gnoclClearOptions( tagOptions );

               return ret;
            }
      case ConfigureIdx:
            {
               int        ret;
               GtkTextTag *tag; 

               /* win tag create name */
               if( objc < cmdNo + 2 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo + 1, objv, 
                        "tag-name ?option val ...?" );
                  return TCL_ERROR;
               }

               /* TODO? first reset options */
               tag = gtk_text_tag_table_lookup( 
                     gtk_text_buffer_get_tag_table( buffer ),
                     Tcl_GetString( objv[cmdNo+1] ) );
               if( tag == NULL )
               {
                  Tcl_AppendResult( interp, "Unknown tag \"", 
                        Tcl_GetString( objv[cmdNo+1] ), "\"", NULL );
                  return TCL_ERROR;
               }
               ret = gnoclParseAndSetOptions( interp, objc-cmdNo-1, 
                     objv+cmdNo+1, tagOptions, G_OBJECT( tag ) );
               gnoclClearOptions( tagOptions );

               return ret;
            }
      default:
            assert( 0 );
            return TCL_ERROR;
   }

   return TCL_OK; 
}

static int scrollToPos( GtkTextView *view, GtkTextBuffer *buffer,
      Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-margin", GNOCL_DOUBLE, NULL },    /* 0 */
      { "-align", GNOCL_OBJ, NULL },        /* 1 */
      { NULL }
   };
   const int marginIdx = 0;
   const int alignIdx  = 1;

   int   ret = TCL_ERROR;

   double      margin = .0;
   int         useAlign = 0;
   gfloat      xAlign = 0.5,
               yAlign = 0.5;
   GtkTextIter iter;
   GtkTextMark *mark;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "index ?-option val ...?" );
      return TCL_ERROR;
   }

   if( posToIter( interp, objv[2], buffer, &iter ) != TCL_OK )
      return TCL_ERROR;

   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
      goto clearExit;

   if( options[alignIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( gnoclGetBothAlign( interp, options[alignIdx].val.obj, 
            &xAlign, &yAlign ) != TCL_OK )
         goto clearExit;
      useAlign = 1;
   }
   if( options[marginIdx].status == GNOCL_STATUS_CHANGED )
   {
      margin = options[marginIdx].val.d;
      if( margin < .0 || margin >= 0.5 )
      {
         Tcl_SetResult( interp, "-margin must be between 0 and 0.5",
               TCL_STATIC );
         goto clearExit;
      }
   }

   mark = gtk_text_buffer_create_mark( buffer, "__gnoclScrollMark__",
         &iter, 0 );
   gtk_text_view_scroll_to_mark( view, mark, margin, useAlign, 
         xAlign, yAlign );
   gtk_text_buffer_delete_mark( buffer, mark );

   ret = TCL_OK;

clearExit:
   gnoclClearOptions( options );
   return ret;
}

static int textInsert( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], int cmdNo )
{
   GnoclOption insertOptions[] =
   {
      { "-tags", GNOCL_LIST, NULL },
      { NULL }
   };
   const int tagsIdx = 0;
   gint      startOffset;
   int       ret = TCL_ERROR;

   GtkTextIter   iter;

   if( objc < cmdNo + 2 )
   {
      Tcl_WrongNumArgs( interp, cmdNo, objv, 
            "position text ?-option val ...?" );
      return TCL_ERROR;
   }

   if( posToIter( interp, objv[cmdNo], buffer, &iter ) != TCL_OK )
      return TCL_ERROR;

   if( gnoclParseOptions( interp, objc - cmdNo-1, objv + cmdNo+1, 
         insertOptions ) != TCL_OK )
      goto clearExit;

   startOffset = gtk_text_iter_get_offset( &iter );

   gtk_text_buffer_insert( buffer, &iter, gnoclGetString( objv[cmdNo+1] ), -1 );
   if( insertOptions[tagsIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkTextIter start;
      int k, no;
      Tcl_Obj *obj = insertOptions[tagsIdx].val.obj;

      gtk_text_buffer_get_iter_at_offset( buffer, &start, startOffset );
      if( Tcl_ListObjLength( interp, obj, &no ) != TCL_OK )
         goto clearExit;

      for( k = 0; k < no; ++k )
      {
         Tcl_Obj *tp;
         if( Tcl_ListObjIndex( interp, obj, k, &tp ) != TCL_OK )
         {
            Tcl_SetResult( interp, "Could not read tag list", TCL_STATIC );
            goto clearExit;
         }
         gtk_text_buffer_apply_tag_by_name( buffer, Tcl_GetString( tp ), 
               &start, &iter );
      }
   }

   ret = TCL_OK;

clearExit:
   gnoclClearOptions( insertOptions );

   return ret;
}

static int configure( Tcl_Interp *interp, GtkScrolledWindow *scrolled,
      GtkTextView *text, GnoclOption options[] )
{
   if( options[scrollBarIdx].status == GNOCL_STATUS_CHANGED )
   {
      GtkPolicyType hor, vert;
      if( gnoclGetScrollbarPolicy( interp, options[scrollBarIdx].val.obj, 
            &hor, &vert ) != TCL_OK )
         return TCL_ERROR;

      gtk_scrolled_window_set_policy( scrolled, hor, vert );
   }

   return TCL_OK;
}

/* 
   ->   0: Ok
        1: delete chosen
        2: configure chosen
        3: scrollToPosition chosen
      < 0: ERROR
*/
int gnoclTextCommand( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc,
      Tcl_Obj * const objv[], int cmdNo, int isTextWidget )
{
   const char *cmds[] = { "delete", "configure", "scrollToPosition",
         "erase", "select", "get", "cut", "copy", "paste", 
         "getLength", "setCursor", "getCursor", "insert", "tag", 
         NULL };
   enum cmdIdx { DeleteIdx, ConfigureIdx, ScrollToPosIdx,
         EraseIdx, SelectIdx, GetIdx, CutIdx, CopyIdx, PasteIdx, 
         GetLengthIdx, SetCursorIdx, GetCursorIdx, InsertIdx, TagIdx };
   int   idx;

   if( objc < cmdNo + 1 )
   {
      Tcl_WrongNumArgs( interp, cmdNo, objv, "command" );
      return -1;
   }

   if( Tcl_GetIndexFromObj( interp, objv[cmdNo], 
         isTextWidget ? cmds : cmds + 3, "command", 
         TCL_EXACT, &idx ) != TCL_OK )
      return -1;

   if( !isTextWidget )
      idx += 3;

   switch( idx )
   {
      case DeleteIdx:           return 1; 
      case ConfigureIdx:        return 2;
      case ScrollToPosIdx:      return 3;

      case EraseIdx:
      case SelectIdx:
      case GetIdx:
            {
               GtkTextIter startIter, endIter;
               /* text erase/select/getChars startIndex ?endIndex? */
               if( objc < cmdNo + 2 || objc > cmdNo + 3 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo + 1, objv, 
                        "startIndex ?endIndex?" );
                  return TCL_ERROR; 
               }
               if( posToIter( interp, objv[cmdNo+1], buffer, 
                     &startIter ) != TCL_OK )
                  return TCL_ERROR;

               if( objc >= 4 )
               {
                  if( posToIter( interp, objv[cmdNo+2], buffer, 
                        &endIter ) != TCL_OK )
                     return TCL_ERROR;
               }
               else
               {
                  endIter = startIter;
                  gtk_text_iter_backward_char( &endIter );
               }
               
               switch( idx )
               {
                  case EraseIdx:
                           gtk_text_buffer_delete( buffer, 
                                 &startIter, &endIter );
                           break;
                  case SelectIdx:
                           gtk_text_buffer_place_cursor( buffer, &startIter );
                           gtk_text_buffer_move_mark_by_name( buffer, 
                                 "selection_bound", &endIter );
                           break;
                  case GetIdx:
                           {
                              /* TODO: include_hidden_chars */
                              char *txt = gtk_text_buffer_get_text(
                                    buffer, &startIter, &endIter, 1 );
                              Tcl_SetObjResult( interp, 
                                    Tcl_NewStringObj( txt, -1 ) );
                           }
                           break;
               }
            }
            break;
      case CutIdx:
      case CopyIdx:
      case PasteIdx:
            {
               /* TODO: option which clipboard */
               GtkClipboard *clipboard = gtk_clipboard_get( GDK_NONE );
               if( objc != cmdNo+1 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo+1, objv, NULL );
                  return TCL_ERROR; 
               }
               switch( idx )
               {
                  case CutIdx:
                     gtk_text_buffer_cut_clipboard( buffer, clipboard, 1 );
                     break;
                  case CopyIdx:
                     gtk_text_buffer_copy_clipboard( buffer, clipboard ); 
                     break;
                  case PasteIdx:
                     gtk_text_buffer_paste_clipboard( buffer, clipboard,
                           NULL, 1 ); 
                     break;
               }
            }
            break;
      case GetLengthIdx: /* TODO getByteCount */
            {
               /* editable getLength */
               if( objc != cmdNo+1 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo+1, objv, NULL );
                  return -1; 
               }

               Tcl_SetObjResult( interp, Tcl_NewIntObj( 
                     gtk_text_buffer_get_char_count( buffer ) ) );
            }
            break;
      case SetCursorIdx:
            {
               GtkTextIter iter;

               /* text setCursor index */
               if( objc != cmdNo+2 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo+1, objv, "index" );
                  return -1; 
               }
               if( posToIter( interp, objv[cmdNo+1], buffer, &iter ) != TCL_OK )
                  return -1;

               gtk_text_buffer_place_cursor( buffer, &iter );
            }
            break;
      case GetCursorIdx:
            {
               GtkTextIter   iter;
               int           row, col;
               Tcl_Obj       *resList;

               /* text getCursor */
               if( objc != cmdNo+1 )
               {
                  Tcl_WrongNumArgs( interp, cmdNo+1, objv, NULL );
                  return -1; 
               }
               gtk_text_buffer_get_iter_at_mark( buffer, &iter,
                     gtk_text_buffer_get_insert( buffer ) );

               row = gtk_text_iter_get_line( &iter );
               col = gtk_text_iter_get_line_offset( &iter );

               resList = Tcl_NewListObj( 0, NULL );
               Tcl_ListObjAppendElement( interp, resList,
                     Tcl_NewIntObj( row ) );
               Tcl_ListObjAppendElement( interp, resList,
                     Tcl_NewIntObj( col ) );
               Tcl_SetObjResult( interp, resList );
            }
            break;
      case InsertIdx:
            if( textInsert( buffer, interp, objc, objv, cmdNo+1 ) != TCL_OK )
               return -1;
            break;
      case TagIdx:
            if( tagCmd( buffer, interp, objc, objv, cmdNo+1 ) != TCL_OK )
               return -1;
            break;
            
      default:
            assert( 0 );
            return -1;
   }

   return 0;
}

static int textFunc( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   GtkScrolledWindow  *scrolled = GTK_SCROLLED_WINDOW( data );
   GtkTextView        *text = GTK_TEXT_VIEW( 
                           gtk_bin_get_child( GTK_BIN( scrolled ) ) );
   GtkTextBuffer *buffer = gtk_text_view_get_buffer( text );

   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "command" );
      return TCL_ERROR;
   }

   switch( gnoclTextCommand( buffer, interp, objc, objv, 1, 1 ) )
   {
      case 0:
            break;      /* return TCL_OK */
      case 1:   /* delete */
            return gnoclDelete( interp, GTK_WIDGET( scrolled ), objc, objv );

      case 2:   /* configure */
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     textOptions, G_OBJECT( text ) ) == TCL_OK )
               {
                  ret = configure( interp, scrolled, text, textOptions );
               }
               gnoclClearOptions( textOptions );
               return ret;
            }
            break;
      case 3: /* scrollToPosition */
            return scrollToPos( text, buffer, interp, objc, objv );
      default:
            return TCL_ERROR;
   }

   return TCL_OK;
}

int gnoclTextCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   int               ret;
   GtkTextView       *text;
   GtkScrolledWindow *scrolled;

   if( gnoclParseOptions( interp, objc, objv, textOptions ) != TCL_OK )
   {
      gnoclClearOptions( textOptions );
      return TCL_ERROR;
   }

   text = GTK_TEXT_VIEW( gtk_text_view_new( ) );
   scrolled =  GTK_SCROLLED_WINDOW( gtk_scrolled_window_new( NULL, NULL ) );
   gtk_scrolled_window_set_policy( scrolled, 
         GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
   gtk_container_add( GTK_CONTAINER( scrolled ), GTK_WIDGET( text ) );
   gtk_widget_show_all( GTK_WIDGET( scrolled) );
   
   ret = gnoclSetOptions( interp, textOptions, G_OBJECT( text ), -1 );
   if( ret == TCL_OK )
      ret = configure( interp, scrolled, text, textOptions );

   gnoclClearOptions( textOptions );

   if( ret != TCL_OK )
   {
      gtk_widget_destroy( GTK_WIDGET( scrolled ) );
      return TCL_ERROR;
   }

   return gnoclRegisterWidget( interp, GTK_WIDGET( scrolled ), textFunc );
}

