/*
 * $Id: gnoclGconf.c,v 1.4 2004/07/17 17:00:37 baum Exp $
 *
 * This file implements a Tcl interface to GConf
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2003-07:    Begin of developement
 */

#include "../gnocl.h"
#include <gconf/gconf.h>
#include <gconf/gconf-client.h>
#include <string.h>

#include <assert.h>

static const char *type2string( GConfValueType type )
{
   switch( type )
   {
      case GCONF_VALUE_STRING:  return "string";
      case GCONF_VALUE_INT:     return "integer";
      case GCONF_VALUE_FLOAT:   return "float";
      case GCONF_VALUE_BOOL:    return "boolean";
      case GCONF_VALUE_LIST:    return "list";  /* TODO list type */
      case GCONF_VALUE_PAIR:    return "pair";  /* TODO pair type */
      case GCONF_VALUE_SCHEMA:  return "schema";
      case GCONF_VALUE_INVALID: return "invalid";
      default:
            assert( 0 );
   }
   return NULL;
}

static const char *entry2Type( GConfClient *client, GConfEntry *entry )
{
   GConfValue *val = gconf_entry_get_value( entry );
   if( val == NULL )
      return "notSet";

   return type2string( val->type );
}

static Tcl_Obj *value2Tcl( GConfValue *val )
{
   if( val == NULL )
      return NULL;

   switch( val->type )
   {
      case GCONF_VALUE_STRING:  
            return Tcl_NewStringObj( gconf_value_get_string( val ), -1 );
      case GCONF_VALUE_INT:  
            return Tcl_NewIntObj( gconf_value_get_int( val ) );
      case GCONF_VALUE_FLOAT:  
            return Tcl_NewDoubleObj( gconf_value_get_float( val ) );
      case GCONF_VALUE_BOOL:  
            return Tcl_NewBooleanObj( gconf_value_get_bool( val ) );
      case GCONF_VALUE_LIST:
            {
               GSList *lst = gconf_value_get_list( val );
               Tcl_Obj *res = Tcl_NewListObj( 0, NULL );
               GSList *p;
               for( p = lst; p != NULL; p = p->next )
               {
                  GConfValue *val = p->data;
                  Tcl_Obj *v = value2Tcl( val );
                  if( v != NULL ) 
                     Tcl_ListObjAppendElement( NULL, res, v );
               }
               g_slist_free( lst );
               return res;
            }
      case GCONF_VALUE_PAIR:
            {
               Tcl_Obj *res;
               Tcl_Obj *cdr = NULL;
               Tcl_Obj *car = value2Tcl( gconf_value_get_car( val ) );
               if( car )
                  cdr = value2Tcl( gconf_value_get_cdr( val ) );
               if( cdr == NULL )
                  return NULL;
               res = Tcl_NewListObj( 0, NULL );
               Tcl_ListObjAppendElement( NULL, res, car );
               Tcl_ListObjAppendElement( NULL, res, cdr );
               
               return res;
            }

      case GCONF_VALUE_SCHEMA:
            {
               GConfSchema *schema = gconf_value_get_schema( val );
               Tcl_Obj *res = Tcl_NewListObj( 0, NULL );

               Tcl_Obj *obj;
               /* type */
               obj = Tcl_NewStringObj( type2string( 
                     gconf_schema_get_type( schema ) ), -1 );
               Tcl_ListObjAppendElement( NULL, res, obj );
               /* default value */
               obj = value2Tcl( gconf_schema_get_default_value( schema ) ); 
               if( obj == NULL )
                  obj = Tcl_NewStringObj( "", 0 );
               Tcl_ListObjAppendElement( NULL, res, obj );
               /* short descr */
               obj = Tcl_NewStringObj( gconf_schema_get_short_desc( schema ), 
                     -1 );
               Tcl_ListObjAppendElement( NULL, res, obj );
               /* long descr */
               obj = Tcl_NewStringObj( gconf_schema_get_long_desc( schema ), 
                     -1 );
               Tcl_ListObjAppendElement( NULL, res, obj );
               /* owner */
               obj = Tcl_NewStringObj( gconf_schema_get_owner( schema ), -1 );
               Tcl_ListObjAppendElement( NULL, res, obj );
               /* locale */
               obj = Tcl_NewStringObj( gconf_schema_get_locale( schema ), -1 );
               Tcl_ListObjAppendElement( NULL, res, obj );

               return res;
            }
            break;
      case GCONF_VALUE_INVALID: 
            break;
   }
   return NULL;
}

/* returns a list of
   - key
   - value
   - type
*/
static Tcl_Obj *entry2Tcl( Tcl_Interp *interp, GConfClient *client,
      GConfEntry *entry )
{
   GConfValue *value = gconf_entry_get_value( entry );
   Tcl_Obj    *res = Tcl_NewListObj( 0, NULL );

   const char *key = gconf_entry_get_key( entry );
   Tcl_Obj    *val = value2Tcl( value );

   Tcl_ListObjAppendElement( interp, res, Tcl_NewStringObj( key , -1 ) );
   if( val != NULL ) 
      Tcl_ListObjAppendElement( interp, res, val );
   else
      Tcl_ListObjAppendElement( interp, res, Tcl_NewStringObj( "" , 0 ) );
   Tcl_ListObjAppendElement( interp, res, 
         Tcl_NewStringObj( entry2Type( client, entry ) , -1 ) );

   return res;
}

static int getEntry( Tcl_Interp *interp, GConfClient *client, 
      const char *key )
{
   const gchar *locale = NULL;         /* TODO */
   gboolean    use_schema_default = 1; /* TODO */

   int        isDir;
   Tcl_Obj    *res;
   GError     *error = NULL;
   GConfEntry *entry = gconf_client_get_entry( client, key, locale,
                              use_schema_default, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   res = entry2Tcl( interp, client, entry );
   isDir = gconf_client_dir_exists( client, key, NULL );
   Tcl_ListObjAppendElement( interp, res, Tcl_NewBooleanObj( isDir ) );

   Tcl_SetObjResult( interp, res );

   return TCL_OK;
}

static int get( Tcl_Interp *interp, GConfClient *client, 
      const char *key, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-mode", GNOCL_OBJ, NULL },
      { NULL }
   };
   const int modeIdx = 0;

   const char *txt[] = { "value", "default", "noDefault", NULL };
   enum optIdx { ValueIdx, DefaultIdx, NoDefaultIdx };

   int idx = ValueIdx;

   Tcl_Obj    *res;
   GError     *error = NULL;
   GConfValue *value;

   /* gconf get key -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
   {
      gnoclClearOptions( options );
      return TCL_ERROR;
   }

   if( options[modeIdx].status == GNOCL_STATUS_CHANGED )
   {
      if( Tcl_GetIndexFromObj( interp, options[modeIdx].val.obj, txt, 
            "mode", TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;
   }
   gnoclClearOptions( options );

   switch( idx )
   {
      case ValueIdx:       value = gconf_client_get( client, key, &error );
                           break;
      case DefaultIdx:     value = gconf_client_get_default_from_schema( 
                                 client, key, &error );
                           break;
      case NoDefaultIdx:   value = gconf_client_get_without_default( 
                                 client, key, &error );
                           break;
   }

   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   res = value2Tcl( value );
   if( res == NULL )
      res = Tcl_NewStringObj( "" , 0 );

   Tcl_SetObjResult( interp, res );

   return TCL_OK;
}

static int tcl2value( Tcl_Interp *interp, Tcl_Obj *type, Tcl_Obj *tclValue, 
      int allowSchema, GConfValue **ret, GConfValueType *pType )
{
   static const char *txt[] = { 
         "schema",              /* schema must come first! */
         "list", "pair",  
         "string",  "float", "boolean", 
         "integer", 
         NULL };
   GConfValueType    types[] = { 
         GCONF_VALUE_SCHEMA, 
         GCONF_VALUE_LIST, GCONF_VALUE_PAIR,
         GCONF_VALUE_STRING, GCONF_VALUE_FLOAT, GCONF_VALUE_BOOL, 
         GCONF_VALUE_INT };

   int  gType = GCONF_VALUE_STRING;

   if( type != NULL )
   {
      int idx;

      if( Tcl_GetIndexFromObj( interp, type, allowSchema ? txt : txt + 1, 
            "type", TCL_EXACT, &idx ) != TCL_OK )
         return TCL_ERROR;

      gType = types[allowSchema ? idx : idx + 1];
   }

   if( pType != NULL )
      *pType = gType;

   switch( gType )
   {
      case GCONF_VALUE_SCHEMA:
               {
                  int     no;
                  Tcl_Obj *obj;
                  GConfSchema *sc;

                  if( Tcl_ListObjLength( interp, tclValue, &no ) != TCL_OK 
                        || (no != 5 && no != 6) )
                  {
                     Tcl_SetResult( interp, 
                           "Value must be a list of 5 or 6 elements.", 
                           TCL_STATIC );
                     return TCL_ERROR;
                  }
                  sc = gconf_schema_new ( );

                  /* type and default value */
                  {  
                     Tcl_Obj        *type, *value;
                     GConfValueType gType;
                     GConfValue     *gValue;

                     if( Tcl_ListObjIndex( interp, tclValue, 0, &type ) 
                           != TCL_OK 
                           || Tcl_ListObjIndex( interp, tclValue, 1, &value ) 
                           != TCL_OK )
                        goto schemaExit;

                     if( tcl2value( interp,  type, value, 0, &gValue, &gType ) 
                           != TCL_OK )
                        goto schemaExit;

                     gconf_schema_set_type( sc, gType );
                     if( gType == GCONF_VALUE_LIST )
                        gconf_schema_set_list_type( sc, GCONF_VALUE_STRING );
                     else if( gType == GCONF_VALUE_PAIR )
                     {
                        gconf_schema_set_car_type( sc, GCONF_VALUE_STRING );
                        gconf_schema_set_cdr_type( sc, GCONF_VALUE_STRING );
                     }

                     gconf_schema_set_default_value( sc, gValue );
                  }

                  /* short description */
                  if( Tcl_ListObjIndex( interp, tclValue, 2, &obj ) != TCL_OK )
                     goto schemaExit;
                  gconf_schema_set_short_desc( sc, Tcl_GetString( obj ) );

                  /* long description */
                  if( Tcl_ListObjIndex( interp, tclValue, 3, &obj ) != TCL_OK )
                     goto schemaExit;
                  gconf_schema_set_long_desc( sc, Tcl_GetString( obj ) );

                  /* owner */
                  if( Tcl_ListObjIndex( interp, tclValue, 4, &obj ) != TCL_OK )
                     goto schemaExit;
                  gconf_schema_set_long_desc( sc, Tcl_GetString( obj ) );

                  /* locale */
                  if( no == 6 )
                  {
                     if( Tcl_ListObjIndex( interp, tclValue, 5, &obj ) 
                           != TCL_OK )
                        goto schemaExit;
                     gconf_schema_set_locale( sc, Tcl_GetString( obj ) );
                  }
                  else
                     gconf_schema_set_locale( sc, "C" );

                  *ret = gconf_value_new( GCONF_VALUE_SCHEMA );
                  gconf_value_set_schema_nocopy( *ret, sc );

                  break;

               schemaExit:
                  gconf_schema_free( sc );
                  return TCL_ERROR;
               }

      case GCONF_VALUE_LIST:     /* TODO: type of list */
               {
                  GSList *lst = NULL;
                  int    k, no;
                  if( Tcl_ListObjLength( interp, tclValue, &no ) != TCL_OK )
                  {
                     Tcl_SetResult( interp, "value must be proper list", 
                           TCL_STATIC );
                     return TCL_ERROR;
                  }
                  for( k = 0; k < no; ++k )
                  {
                     Tcl_Obj    *tp;
                     GConfValue *val;
                     if( Tcl_ListObjIndex( interp, tclValue, k, &tp ) 
                           != TCL_OK )
                     {
                        g_slist_free( lst );
                        return TCL_ERROR;
                     }
                     val = gconf_value_new( GCONF_VALUE_STRING );
                     gconf_value_set_string( val, Tcl_GetString( tp ) );
                     lst = g_slist_append( lst, val );
                  }
                  *ret = gconf_value_new( GCONF_VALUE_LIST );
                  gconf_value_set_list_type( *ret, GCONF_VALUE_STRING );
                  gconf_value_set_list_nocopy( *ret, lst );
               }
               break;
      case GCONF_VALUE_PAIR:     /* TODO: type of pair */
               {
                  int        no;
                  Tcl_Obj    *car, *cdr;
                  GConfValue *gCar, *gCdr;
                  if( Tcl_ListObjLength( interp, tclValue, &no ) != TCL_OK 
                        || no != 2 )
                  {
                     Tcl_SetResult( interp, 
                           "Value must be a list of two elements", 
                           TCL_STATIC );
                     return TCL_ERROR;
                  }
                  if( Tcl_ListObjIndex( interp, tclValue, 0, &car ) != TCL_OK 
                        || Tcl_ListObjIndex( interp, tclValue, 1, &cdr ) 
                              != TCL_OK )
                  {
                     return TCL_ERROR;
                  }
                  gCar = gconf_value_new( GCONF_VALUE_STRING );
                  gconf_value_set_string( gCar, Tcl_GetString( car ) );
                  gCdr = gconf_value_new( GCONF_VALUE_STRING );
                  gconf_value_set_string( gCdr, Tcl_GetString( cdr ) );

                  *ret = gconf_value_new( GCONF_VALUE_PAIR );
                  gconf_value_set_car_nocopy( *ret, gCar );
                  gconf_value_set_cdr_nocopy( *ret, gCdr );
               }
               break;
      case GCONF_VALUE_STRING:   
               {
                  *ret = gconf_value_new( GCONF_VALUE_STRING );
                  gconf_value_set_string( *ret, Tcl_GetString( tclValue ) );
               }
               break;
      case GCONF_VALUE_FLOAT:    
               {
                  double d;
                  if( Tcl_GetDoubleFromObj( interp, tclValue, &d ) != TCL_OK )
                     return TCL_ERROR;
                  *ret = gconf_value_new( GCONF_VALUE_FLOAT );
                  gconf_value_set_float( *ret, d );
               }
               break;
      case GCONF_VALUE_BOOL:
               {
                  int i;
                  if( Tcl_GetBooleanFromObj( interp, tclValue, &i ) != TCL_OK )
                     return TCL_ERROR;
                  *ret = gconf_value_new( GCONF_VALUE_BOOL );
                  gconf_value_set_bool( *ret, i );
               }
               break;
      case GCONF_VALUE_INT:
               {
                  int i;
                  if( Tcl_GetIntFromObj( interp, tclValue, &i ) != TCL_OK )
                     return TCL_ERROR;
                  *ret = gconf_value_new( GCONF_VALUE_INT );
                  gconf_value_set_int( *ret, i );
               }
               break;
   }

   return TCL_OK;
}

static int set( Tcl_Interp *interp, GConfClient *client, 
      const char *key, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-type", GNOCL_OBJ, NULL },
      { NULL }
   };
   const int typeIdx = 0;

   GError     *error = NULL;
   Tcl_Obj    *type = NULL;
   GConfValue *value;

   if( objc < 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "key value" );
      return TCL_ERROR;
   }

   /* gconf set key value -option ... */
   if( gnoclParseOptions( interp, objc - 3, objv + 3, options ) != TCL_OK )
      goto cleanExit;

   if( options[typeIdx].status == GNOCL_STATUS_CHANGED )
      type = options[typeIdx].val.obj;

   if( tcl2value( interp, type, objv[3], 1, &value, NULL ) != TCL_OK )
      goto cleanExit;

   gconf_client_set( client, key, value, &error );

   gnoclClearOptions( options );

   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   return TCL_OK;

cleanExit:
   gnoclClearOptions( options );
   return TCL_ERROR;
}

static int unsetRecursive( Tcl_Interp *interp, GConfClient *client, 
      const char *key )
{
   GError *error = NULL;

   /* unset all directories */
   GSList *lst = gconf_client_all_dirs( client, key,  &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   if( lst != NULL )
   {
      GSList *p = lst;
      for( p = lst; p != NULL; p = p->next )
      {
         const char *val = p->data;
         if( unsetRecursive( interp, client, val ) != TCL_OK )
         {
            g_slist_free( lst );
            return TCL_ERROR;
         }
      }
      g_slist_free( lst );
   }

   /* unset all files */
   lst = gconf_client_all_entries( client, key,  &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   if( lst != NULL )
   {
      GSList *p = lst;
      for( ; p != NULL; p = p->next )
      {
         GConfEntry *entry = p->data;
         gconf_client_unset( client, gconf_entry_get_key( entry ), &error );
         if( error != NULL )
         {
            Tcl_SetResult( interp, error->message, TCL_VOLATILE );
            g_error_free( error );
            g_slist_free( lst );
            return TCL_ERROR;
         }
      }
      g_slist_free( lst );
   }

   /* unset the file itself */
   gconf_client_unset( client, key, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   return TCL_OK;
}

static int unset( Tcl_Interp *interp, GConfClient *client, 
      const char *key, int objc, Tcl_Obj * const objv[] )
{
   GnoclOption options[] =
   {
      { "-recursive", GNOCL_OBJ, NULL },
      { NULL }
   };
   const int recursiveIdx = 0;

   GError *error = NULL;
   int    recursive = 0;

   if( objc < 3 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "key" );
      return TCL_ERROR;
   }

   /* gconf unset key -option ... */
   if( gnoclParseOptions( interp, objc - 2, objv + 2, options ) != TCL_OK )
      return TCL_ERROR;

   if( options[recursiveIdx].status == GNOCL_STATUS_CHANGED )
      recursive = options[recursiveIdx].val.b;

   if( recursive )
      return unsetRecursive( interp, client, key );

   gconf_client_unset( client, key, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   return TCL_OK;
}

static int sync( Tcl_Interp *interp, GConfClient *client )
{
   GError     *error = NULL;

   gconf_client_suggest_sync( client, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   return TCL_OK;
}

static void notifyFunc( GConfClient *client, guint cnxn_id, GConfEntry *entry,
   gpointer user_data )
{
   GnoclCommandData *cs = (GnoclCommandData *)user_data;

   GnoclPercSubst ps[] = {
      { 'k', GNOCL_STRING },  /* key */
      { 't', GNOCL_STRING },  /* type */
      { 'v', GNOCL_OBJ },     /* value */
      { 0 }
   };
   ps[0].val.str = gconf_entry_get_key( entry );
   ps[1].val.str = entry2Type( client, entry );
   ps[2].val.obj = value2Tcl( gconf_entry_get_value( entry ) );
   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
}

static int onChanged( Tcl_Interp *interp, GConfClient *client, 
      const char *key, int objc, Tcl_Obj * const objv[] )
{
   GnoclCommandData *cs;
   GError     *error = NULL;
   static char *dirRemembered = NULL;

   /* gconf onChanged key command */
   if( objc != 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "key command" );
      return TCL_ERROR;
   }

   cs = g_new( GnoclCommandData, 1 );
   cs->interp = interp;
   cs->command = g_strdup( Tcl_GetString( objv[3] ) );

   /*
      the notify works only for directories added.
      At the momement we add only one directory.
      TODO: make this a list and add/remove dirs as necessary
   */
   if( dirRemembered == NULL )
   {
      dirRemembered = g_strdup( key );
      /* make sure, that dirRemembered is really a dir */
      if( gconf_client_dir_exists( client, dirRemembered, NULL ) == 0 )
      {
         char *p = strrchr( dirRemembered, '/' );
         if( p )
            *p = '\0';
      }
   }
   else
   {
      gconf_client_remove_dir( client, dirRemembered, &error );
      if( error != NULL )
      {
         Tcl_SetResult( interp, error->message, TCL_VOLATILE );
         g_error_free( error );
         return TCL_ERROR;
      }
      while( !gconf_key_is_below( dirRemembered, key ) )
      {
         char *p = strrchr( dirRemembered, '/' );
         if( p == dirRemembered )
         {
            p[1] = '\0';
            break;
         }
         *p = '\0';
      }
   }

   /* DEBUG */
   {
      char *txt;
      if( !gconf_valid_key( dirRemembered, &txt ) )
         printf( "gconf: key not valid: %s\n", txt );
      printf( "gconf add_dir: %s for key %s\n", dirRemembered, key );
   }

   gconf_client_add_dir( client, dirRemembered, 
         GCONF_CLIENT_PRELOAD_NONE, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   gconf_client_notify_add( client, key, notifyFunc, cs, NULL, &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }

   return TCL_OK;
}

static int associateSchema( Tcl_Interp *interp, GConfClient *client, 
      const char *key, int objc, Tcl_Obj * const objv[] )
{
   GError      *error = NULL;
   GConfEngine *engine;
   const char  *sKey;

   if( objc != 4 )
   {
      Tcl_WrongNumArgs( interp, 2, objv, "key schema-key" );
      return TCL_ERROR;
   }

   engine = gconf_engine_get_default();
   sKey = Tcl_GetString( objv[3] );

   gconf_engine_associate_schema( engine, key, sKey, &error );
   gconf_engine_unref( engine );

   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   return TCL_OK;
}

static int getEntries( Tcl_Interp *interp, GConfClient *client, 
      const char *key )
{
   GError *error = NULL;
   GSList *lst = gconf_client_all_entries( client, key,  &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   if( lst != NULL )
   {
      Tcl_Obj *res = Tcl_NewListObj( 0, NULL );
      GSList *p = lst;
      for( ; p != NULL; p = p->next )
      {
         GConfEntry *entry = p->data;
         Tcl_Obj *v = entry2Tcl( interp, client, entry );
         Tcl_ListObjAppendElement( interp, res, v );
      }
      g_slist_free( lst );
      Tcl_SetObjResult( interp, res );
   }
   return TCL_OK;
}

static int getDirs( Tcl_Interp *interp, GConfClient *client, 
      const char *key )
{
   GError *error = NULL;
   GSList *lst = gconf_client_all_dirs( client, key,  &error );
   if( error != NULL )
   {
      Tcl_SetResult( interp, error->message, TCL_VOLATILE );
      g_error_free( error );
      return TCL_ERROR;
   }
   if( lst != NULL )
   {
      Tcl_Obj *res = Tcl_NewListObj( 0, NULL );
      GSList *p = lst;
      for( p = lst; p != NULL; p = p->next )
      {
         char *val = p->data;
         Tcl_ListObjAppendElement( interp, res, Tcl_NewStringObj( val, -1 ) );
      }
      g_slist_free( lst );
      Tcl_SetObjResult( interp, res );
   }
   return TCL_OK;
}

static int gconfCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmd[] = { "version", "sync", 
         "getDirs", "getEntries",  "getEntry", 
         "get", "set", "unset", 
         "onChanged", "associateSchema",
         NULL };
   enum optIdx { VersionIdx, SyncIdx,
         GetDirsIdx, GetEntriesIdx, GetEntryIdx, 
         GetIdx, SetIdx, UnsetIdx, 
         OnChangedIdx, AssociateSchemaIdx };
   int  idx;

   GConfClient *client;
   char        *key = NULL;
   int         ret = TCL_ERROR;

   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "subCommand" );
      return TCL_ERROR;
   }
   if( Tcl_GetIndexFromObj( interp, objv[1], cmd, "subCommand", TCL_EXACT,
         &idx ) != TCL_OK )
      return TCL_ERROR;

   if( idx == VersionIdx || idx == SyncIdx )
   {
      if( objc != 2 )
      {
         Tcl_WrongNumArgs( interp, 2, objv, NULL );
         return TCL_ERROR;
      }
   }
   else
   {
      char *p;
      if( objc < 3 )
      {
         Tcl_WrongNumArgs( interp, 2, objv, "key" );
         return TCL_ERROR;
      }
      /* keys may not end with an "/", exception: key is "/" */
      key = g_strdup( Tcl_GetString( objv[2] ) );
      p = strrchr( key, '/' );
      if( p && (p[1] == '\0') && (p != key) )
         *p = '\0';
   }

   client = gconf_client_get_default( );

   switch( idx )
   {
      case VersionIdx: 
            Tcl_SetObjResult( interp, Tcl_NewStringObj( VERSION, -1 ) );
            ret = TCL_OK;
            break;
      case SyncIdx:
            ret = sync( interp, client );
            break;
      case GetDirsIdx:
            if( objc != 3 )
               Tcl_WrongNumArgs( interp, 2, objv, "key" );
            else
               ret = getDirs( interp, client, key );
            break;
      case GetEntriesIdx:
            if( objc != 3 )
               Tcl_WrongNumArgs( interp, 2, objv, "key" );
            else
               ret = getEntries( interp, client, key );
            break;
      case GetEntryIdx:
            if( objc != 3 )
               Tcl_WrongNumArgs( interp, 2, objv, "key" );
            else
               ret = getEntry( interp, client, key );
            break;
      case GetIdx:
            ret = get( interp, client, key, objc, objv );
            break;
      case SetIdx:
            ret = set( interp, client, key, objc, objv );
            break;
      case UnsetIdx:
            ret = unset( interp, client, key, objc, objv );
            break;
      case OnChangedIdx:
            ret = onChanged( interp, client, key, objc, objv );
            break;
      case AssociateSchemaIdx:
            ret = associateSchema( interp, client, key, objc, objv );
            break;
   }

   g_object_unref( client );

   if( key )
      g_free( key );
      
   return ret;
}

int Gnoclgconf_Init( Tcl_Interp *interp )
{

   /* printf( "Initializing gnocl gconf version %s\n", VERSION ); */

   if( Tcl_InitStubs( interp, "8.3", 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgRequire( interp, "Gnocl", VERSION, 0 ) == NULL )
      return TCL_ERROR;

   if( Tcl_PkgProvide( interp, "GnoclGconf", VERSION ) != TCL_OK )
      return TCL_ERROR;

   Tcl_CreateObjCommand( interp, "gnocl::gconf", gconfCmd, NULL, NULL );
   
   return TCL_OK;
}

