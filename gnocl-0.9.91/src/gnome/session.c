/*
 * $Id: session.c,v 1.3 2003/12/22 18:46:38 baum Exp $
 *
 * This file implements the session command (gnome client)
 *
 * Copyright (c) 2001 - 2003 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2003-11: Begin of developement
 */

#include "gnoclGnome.h"
#include <libgnomeui/gnome-client.h>
#include <assert.h>

static GnoclOption sessionOptions[] =
{
   { "-onConnect", GNOCL_OBJ, NULL },           /* 0 */
   { "-onDie", GNOCL_OBJ, NULL },               /* 1 */
   { "-onDisconnect", GNOCL_OBJ, NULL },        /* 2 */
   { "-onSaveComplete", GNOCL_OBJ, NULL },      /* 3 */
   { "-onSaveYourself", GNOCL_OBJ, NULL },      /* 4 */
   { "-onShutdownCancelled", GNOCL_OBJ, NULL }, /* 5 */
   { "-restartCommand", GNOCL_STRING, NULL },   /* 6 */
   { "-discardCommand", GNOCL_STRING, NULL },   /* 7 */
   { "-currentDirectory", GNOCL_STRING, NULL }, /* 8 */
   { "-environment", GNOCL_STRING, NULL },      /* 9 */
   { "-saveDialog", GNOCL_STRING, NULL },      /* 10 */
   { "-saveErrorDialog", GNOCL_STRING, NULL }, /* 11 */
   { NULL },
};

static const int onConnectIdx           = 0;
static const int onDieIdx               = 1;
static const int onDisconnectIdx        = 2;
static const int onSaveCompleteIdx      = 3;
static const int onSaveYourselfIdx      = 4;
static const int onShutdownCancelledIdx = 5;
static const int restartCommandIdx      = 6;
static const int discardCommandIdx      = 7;
static const int currentDirectoryIdx    = 8;
static const int environmentIdx         = 9;
static const int saveDialogIdx         = 10;
static const int saveErrorDialogIdx    = 11;

void onConnect( GnomeClient *client, gboolean arg1, gpointer data )
{
   GnoclCommandData *cs = (GnoclCommandData *)data;
   GnoclPercSubst ps[] = {
      { 'r', GNOCL_BOOL },  /* whether the app has been restarted */
      { 0 }
   };

   ps[0].val.b = arg1;
   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
}

void onOneArg( GnomeClient *client, gpointer data )
{
   GnoclCommandData *cs = (GnoclCommandData *)data;
   GnoclPercSubst ps[] = {
      { 0 }
   };

   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
}

gboolean onSaveYourself( GnomeClient *client, gint phase,
      GnomeSaveStyle dataType, gboolean shutdown, GnomeInteractStyle interact,
      gboolean fast, gpointer data )
{
   GnoclCommandData *cs = (GnoclCommandData *)data;
   GnoclPercSubst ps[] = {
      { 'p', GNOCL_INT },   /* phase */
      { 'd', GNOCL_STRING },/* data */
      { 's', GNOCL_BOOL },  /* shutdown */
      { 'i', GNOCL_BOOL },  /* interact */
      { 'f', GNOCL_BOOL },  /* fast */
      { 0 }
   };

   ps[0].val.i = phase;
   ps[1].val.str = ""; /* FIXME */
   ps[2].val.b = shutdown;
   ps[3].val.b = 0;     /* FIXME */
   ps[4].val.b = fast;

   gnoclPercentSubstAndEval( cs->interp, ps, cs->command, 1 );
   return 0;
}

static int configure( Tcl_Interp *interp, GnomeClient *client, 
      GnoclOption options[] )
{
   gnoclConnectOptCmd( interp, G_OBJECT( client ), "connect",
         G_CALLBACK( onConnect ), options + onConnectIdx, NULL, NULL );
   gnoclConnectOptCmd( interp, G_OBJECT( client ), "die",
         G_CALLBACK( onOneArg ), options + onDieIdx, NULL, NULL );
   gnoclConnectOptCmd( interp, G_OBJECT( client ), "disconnect",
         G_CALLBACK( onOneArg ), options + onDisconnectIdx, NULL, NULL );
   gnoclConnectOptCmd( interp, G_OBJECT( client ), "save-complete",
         G_CALLBACK( onOneArg ), options + onSaveCompleteIdx, NULL, NULL );
   gnoclConnectOptCmd( interp, G_OBJECT( client ), "save-yourself",
         G_CALLBACK( onSaveYourself ), 
         options + onSaveYourselfIdx, NULL, NULL );

   if( options[currentDirectoryIdx].status == GNOCL_STATUS_CHANGED )
      gnome_client_set_current_directory( client, 
            options[currentDirectoryIdx].val.str );

   return TCL_OK;
}

int gnoclSessionCmd( ClientData data, Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[] )
{
   static const char *cmd[] = { "configure", NULL };
   enum optIdx { ConfigureIdx };
   int idx;

   static GnomeClient *client = NULL;
   if( client == NULL )
      client = gnome_master_client();

   if( client == NULL )
   {
      Tcl_SetResult( interp, "Unable to initialize session.", TCL_STATIC );
      return TCL_ERROR;
   }

   if( objc < 2 )
   {
      Tcl_WrongNumArgs( interp, 1, objv, "command" );
      return TCL_ERROR;
   }
   if( Tcl_GetIndexFromObj( interp, objv[1], cmd, "command", TCL_EXACT,
         &idx ) != TCL_OK )
      return TCL_ERROR;
   switch( idx )
   {
      case ConfigureIdx:
            {
               int ret = TCL_ERROR;
               if( gnoclParseAndSetOptions( interp, objc - 1, objv + 1, 
                     sessionOptions, G_OBJECT( client ) ) == TCL_OK )
               {
                  ret = configure( interp, client, sessionOptions );
               }
               gnoclClearOptions( sessionOptions );
               return ret;
            }
            break;
   }
   return TCL_OK;
}

