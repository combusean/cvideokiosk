/*
 * $Id: canvasTags.c,v 1.2 2004/08/25 19:29:04 baum Exp $
 *
 * This file implements the tag handling for the canvas widget
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2001-08 begin of developement
 */

#include "canvas.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

/*
   one hash table per canvas: 
      tag -> ptr_array of items
   one ptr_array per item: tags

   The tag name is g_new'ed for the canvas hash table. The ptr_array
   of the items contain only a pointer to this value. No need to g_free
   it there.
*/

typedef struct
{
   const char *str;
   const char *p;
   char type;
   char *name;
} Token;

enum { TOKEN_TAG = 'a' };

static int lex( Tcl_Interp *interp, Token *token );
static int orExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret );

static GPtrArray *copyArray( GPtrArray *in )
{
   GPtrArray *ret = NULL;
   if( in && in->len > 0 )
   {
      int k;
      ret = g_ptr_array_sized_new( in->len );
      for( k = 0; k < in->len; ++k )
         g_ptr_array_add( ret, g_ptr_array_index( in, k ) );
   }

   return ret; 
}

static int primExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret )
{
   *ret = NULL;
   switch( token->type )
   {
      case TOKEN_TAG:
               {
                  /* just copy items */
                  GPtrArray *items = g_hash_table_lookup( tagToItems, 
                        token->name );
                  g_free( token->name );
                  *ret = copyArray( items );
                  if( lex( interp, token ) != TCL_OK )
                     return TCL_ERROR;
               }
               break;

      case '(':
               {
                  if( lex( interp, token ) != TCL_OK )
                     return TCL_ERROR;
                  if( orExpr( interp, tagToItems, token, ret ) != TCL_OK )
                     return TCL_ERROR;
                  if( token->type != ')' )
                  {
                     Tcl_SetResult( interp, "Missing closing brace", 
                           TCL_STATIC );
                     return TCL_ERROR;
                  }
                  if( lex( interp, token ) != TCL_OK )
                     return TCL_ERROR;
               }
               break;

      default:
               Tcl_SetResult( interp, "Expected primary expression", 
                     TCL_STATIC );
               return TCL_ERROR;
   }
   return TCL_OK;
}

static int notExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret )
{
   if( token->type == '!' )
   {
      GPtrArray *op2;

      if( lex( interp, token ) != TCL_OK )
         return TCL_ERROR;
      if( notExpr( interp, tagToItems, token, &op2 ) != TCL_OK )
         return TCL_ERROR;

      *ret = copyArray( g_hash_table_lookup( tagToItems, "all" ) );

      if( op2 != NULL )
      {
         int k;
         for( k = 0; k < op2->len; ++k )
         {
            gpointer data = g_ptr_array_index( op2, k ); 
            g_ptr_array_remove_fast( *ret, data );
         }
      }
      if( op2 )
         g_ptr_array_free( op2, 0 );
   }
   else 
      return primExpr( interp, tagToItems, token, ret );
      
   return TCL_OK;
}

static int andExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret )
{
   if( notExpr( interp, tagToItems, token, ret ) != TCL_OK )
      return TCL_ERROR;
   while( token->type == '&' )
   {
      GPtrArray *op2;
      if( lex( interp, token ) != TCL_OK )
         return TCL_ERROR;
      if( notExpr( interp, tagToItems, token, &op2 ) != TCL_OK )
         return TCL_ERROR;
      if( *ret == NULL || op2 == NULL )
      {
         if( *ret )
            g_ptr_array_free( *ret, 0 );
         if( op2 )
            g_ptr_array_free( op2, 0 );
         *ret = NULL;
      }
      else
      {
         /* calculate the intersection of both arrays */
         int k = 0;
         while( k < (*ret)->len )
         {
            int m;
            gpointer data = g_ptr_array_index( *ret, k ); 
            for( m = 0; m < op2->len; ++m )
               if( g_ptr_array_index( op2, m ) == data )
                  break;

            if( m == op2->len )
               g_ptr_array_remove_index_fast( *ret, k );
            else
               ++k;
         }
         g_ptr_array_free( op2, 0 );
      }

   }
   return TCL_OK;
}

static int xorExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret )
{
   if( andExpr( interp, tagToItems, token, ret ) != TCL_OK )
      return TCL_ERROR;
   while( token->type == '^' )
   {
      GPtrArray *op2;
      if( lex( interp, token ) != TCL_OK )
         return TCL_ERROR;
      if( andExpr( interp, tagToItems, token, &op2 ) != TCL_OK )
         return TCL_ERROR;
      if( *ret == NULL )
         *ret = op2;
      else if( op2 != NULL )
      {
         /* remove the intersection of both arrays */
         int k;
         int retLen = (*ret)->len;
         for( k = 0; k < op2->len; ++k )
         {
            int m;
            gpointer data = g_ptr_array_index( op2, k ); 
            for( m = 0; m < retLen; ++m )
            {
               if( g_ptr_array_index( *ret, m ) == data )
               {
                  g_ptr_array_remove_index_fast( *ret, m );
                  ++m; /* make sure m != retLen */
                  --retLen;
                  break;
               }
            }
            if( m == retLen )
               g_ptr_array_add( *ret, data );
         }
         g_ptr_array_free( op2, 0 );
      }

   }
   return TCL_OK;
}

static int orExpr( Tcl_Interp *interp, GHashTable *tagToItems, 
      Token *token, GPtrArray **ret )
{
   if( xorExpr( interp, tagToItems, token, ret ) != TCL_OK )
      return TCL_ERROR;
   while( token->type == '|' )
   {
      GPtrArray *op2;
      if( lex( interp, token ) != TCL_OK )
         return TCL_ERROR;
      if( xorExpr( interp, tagToItems, token, &op2 ) != TCL_OK )
         return TCL_ERROR;
      if( *ret == NULL )
         *ret = op2;
      else if( op2 != NULL )
      {
         /* calculate the union of both arrays */
         int k;
         const int retLen = (*ret)->len;
         for( k = 0; k < op2->len; ++k )
         {
            int m;
            gpointer data = g_ptr_array_index( op2, k ); 
            for( m = 0; m < retLen; ++m )
               if( g_ptr_array_index( *ret, m ) == data )
                  break;
            if( m == retLen )
               g_ptr_array_add( *ret, data );
         }
         g_ptr_array_free( op2, 0 );
      }

   }
   return TCL_OK;
}

static int lex( Tcl_Interp *interp, Token *token )
{
   /* TODO?
      Tcl_UtfNext
      Tcl_UniCharIsSpace
      Tcl_UniCharIsAlnum
   */
   while( isspace( *token->p ) )
      ++token->p;

   switch( *token->p )
   {
      case 0:
      case '^':
      case '(':
      case ')':
      case '!':
                  token->type = *token->p;
                  ++token->p;
                  break;
      case '|':
      case '&':
                  /* allow || and && as well as | and & */
                  if( token->p[1] == token->p[0] )
                     ++token->p;
                  token->type = *token->p;
                  ++token->p;
                  break;
                  
      default:  
               {
                  const char *p;
                  if( !isalnum( *token->p ) )
                  {
                     Tcl_SetResult( interp, "Invalid character", TCL_STATIC );
                     return TCL_ERROR;
                  }
                  for( p = token->p + 1; isalnum( *p ) || *p == '_'; ++p )
                     ;
                  token->type = TOKEN_TAG;
                  token->name = g_strndup( token->p, p - token->p );
                  token->p = p;
               }
               break;

   }
   return TCL_OK;
}

int gnoclCanvasItemsFromTagOrId( Tcl_Interp *interp, CanvasParams *para, 
      const char *tag, GPtrArray **ret )
{
   Token token;
   token.str = tag;
   token.p = token.str;

   if( lex( interp, &token ) != TCL_OK )
      return TCL_ERROR;
   if( orExpr( interp, para->tagToItems, &token, ret ) != TCL_OK )
      return TCL_ERROR;
   if( token.type != 0 )
   {
      Tcl_SetResult( interp, "Unknown trailing characters", TCL_STATIC );
      return TCL_ERROR;
   }
   return TCL_OK;
}

GPtrArray *gnoclCanvasAllItems( CanvasParams *para )
{
   return g_hash_table_lookup( para->tagToItems, "all" );
}

int gnoclCanvasAddTag( Tcl_Interp *interp, CanvasParams *para, 
      Gnocl_CanvasItemInfo *item, const char *tag )
{
   char *origTag;
   GPtrArray *itemArray;
   if( !g_hash_table_lookup_extended( para->tagToItems, tag,
         (gpointer *)&origTag, (gpointer *)&itemArray ) )
   {
      origTag = g_strdup( tag );
      itemArray = g_ptr_array_new( );
      g_hash_table_insert( para->tagToItems, (gpointer)origTag, itemArray );
      /* printf( "new array %p for tag %s\n", itemArray, origTag ); */
   }
   g_ptr_array_add( itemArray, item );
   g_ptr_array_add( item->tags, origTag );
   /* printf( "adding %p to tag %s\n", item, tag ); */

   return TCL_OK;
}

int gnoclCanvasDelNthTag( Gnocl_CanvasItemInfo *item, int n )
{
   CanvasParams *para = item->canvasParams;
   const char *tag = (char *)g_ptr_array_index( item->tags, n );
   GPtrArray *itemArray = g_hash_table_lookup( para->tagToItems, tag );
   int rm = g_ptr_array_remove_fast( itemArray, item );

   assert( rm );
   assert( n >= 0 && n < item->tags->len ); 
   /* printf( "removing item %p from %s\n", item, tag ); */

   if( itemArray->len == 0 )
   {
      rm = g_hash_table_remove( para->tagToItems, tag );
      /* printf( "removed array %s\n", tag ); */
      assert( rm );
   }

   g_ptr_array_remove_index_fast( item->tags, n );
   return TCL_OK;
}

int gnoclOptCanvasTags( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, 
      Tcl_Obj **ret )
{
   Gnocl_CanvasItemInfo *info = g_object_get_data( obj, "gnocl::info" );
   assert( info->item == GNOME_CANVAS_ITEM( obj ) );

   if( ret == NULL ) /* set value */
   {
      int no, k;

      /* delete old tags apart from ID and "all" */
      assert( info->tags->len >= 2 );
      assert( strcmp( (const char *)g_ptr_array_index( info->tags, 1 ), 
            "all" ) == 0 );
      while( info->tags->len > 2 )
         gnoclCanvasDelNthTag( info, info->tags->len - 1 );

      /* add new tags */
      if( Tcl_ListObjLength( interp, opt->val.obj, &no ) != TCL_OK  )
         return TCL_ERROR;
      for( k = 0; k < no; ++k )
      {
         Tcl_Obj    *tp;
         const char *txt;
         const char *p = NULL;
         if( Tcl_ListObjIndex( interp, opt->val.obj, k, &tp ) != TCL_OK )
            return TCL_ERROR;
         /* TODO? tag must not start with a digit and must not contain 
                  any of "&|^!()" */
         txt = Tcl_GetString( tp );
         if( isalpha( *txt ) )
            for( p = txt + 1; isalnum( *p ); ++p )
               ; /* nothing to do */

         if( p == NULL || *p != 0 )
         {
            Tcl_SetResult( interp, 
                  "Tag must be a nonempty string, starting with a alphabetic "
                  "character followed by alphabetic or numeric characters",
                  TCL_STATIC );
            return TCL_ERROR;
         }
         gnoclCanvasAddTag( interp, info->canvasParams, info, txt );
      }
   }
   else /* get value */
   {
      int k;
      *ret = Tcl_NewListObj( 0, NULL );
      for( k = 0; k < info->tags->len; ++k )
      {
         const char *tag = (const char *)g_ptr_array_index( info->tags, k );
         Tcl_ListObjAppendElement( interp, *ret, Tcl_NewStringObj( tag, -1 ) );
      }
   }

   return TCL_OK;
}

