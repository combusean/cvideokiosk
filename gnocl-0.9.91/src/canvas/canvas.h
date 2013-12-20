#ifndef GNOCLCANVAS_H_INCLUDED
#define GNOCLCANVAS_H_INCLUDED

/*
 * $Id: canvas.h,v 1.13 2004/09/23 19:49:32 baum Exp $
 *
 * This file 
 *
 * Copyright (c) 2001 - 2004 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "../gnocl.h"
#include <libgnomecanvas/libgnomecanvas.h>

struct CanvasParams_;
struct Gnocl_CanvasItemInfo_;

typedef int (*GnoclItemConfigFunc)( Tcl_Interp *interp, int objc, 
      Tcl_Obj * const objv[], struct Gnocl_CanvasItemInfo_ *typeInfo );
typedef int (*GnoclItemDeleteFunc)( Tcl_Interp *interp, 
      struct Gnocl_CanvasItemInfo_ *typeInfo );


enum { GNOCL_CANVAS_ON_MOTION, 
       GNOCL_CANVAS_ON_ENTER, GNOCL_CANVAS_ON_LEAVE,
       GNOCL_CANVAS_ON_BUTTON_PRESS, GNOCL_CANVAS_ON_BUTTON_RELEASE,
       GNOCL_CANVAS_ON_KEY_PRESS, GNOCL_CANVAS_ON_KEY_RELEASE, 
       GNOCL_CANVAS_ON_COUNT };

struct Gnocl_CanvasItemInfo_
{
   /* set by the item creation function */
   GnomeCanvasItem  *item;
   GnoclOption *options;
   int (*setOptions)( Tcl_Interp *interp, 
         struct Gnocl_CanvasItemInfo_ *typeInfo );
   Tcl_Obj *(*getOption)( Tcl_Interp *interp, int idx, 
         struct Gnocl_CanvasItemInfo_ *typeInfo );
   int (*command)( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], 
         struct Gnocl_CanvasItemInfo_ *typeInfo );

   /* set in canvas.c */
   gint       id;
   GPtrArray  *tags;
   char       *scripts[GNOCL_CANVAS_ON_COUNT];
   struct CanvasParams_  *canvasParams;
};

typedef struct CanvasParams_
{
   char         *name;
   Tcl_Interp   *interp;
   GnomeCanvas  *canvas;
   GHashTable   *tagToItems;
} CanvasParams;


typedef struct Gnocl_CanvasItemInfo_ Gnocl_CanvasItemInfo;

typedef Gnocl_CanvasItemInfo *(GnoclItemCreateFunc)( Tcl_Interp *interp,
      int objc, Tcl_Obj * const objv[], GnomeCanvasGroup *group );

GnoclItemCreateFunc gnoclCanvasCreateBPath;
GnoclItemCreateFunc gnoclCanvasCreateClipGroup;
GnoclItemCreateFunc gnoclCanvasCreateEllipse;
GnoclItemCreateFunc gnoclCanvasCreateImage;
GnoclItemCreateFunc gnoclCanvasCreateLine;
GnoclItemCreateFunc gnoclCanvasCreatePolygon;
GnoclItemCreateFunc gnoclCanvasCreateRectangle;
GnoclItemCreateFunc gnoclCanvasCreateRichText;
GnoclItemCreateFunc gnoclCanvasCreateText;
GnoclItemCreateFunc gnoclCanvasCreateWidget;

GPtrArray *gnoclCanvasAllItems( CanvasParams *para );
int gnoclCanvasItemsFromTagOrId( Tcl_Interp *interp, CanvasParams *para, 
      const char *tag, GPtrArray **ret );
Gnocl_CanvasItemInfo *gnoclInfoFromCanvasItem( CanvasParams *param, 
      GnomeCanvasItem *item );
int gnoclCanvasAddTag( Tcl_Interp *interp, CanvasParams *para, 
      Gnocl_CanvasItemInfo *item, const char *tag );
int gnoclCanvasDelNthTag( Gnocl_CanvasItemInfo *item, int n );
int gnoclCanvasHandleTags( Tcl_Interp *interp, GnoclOption *opt, 
      Gnocl_CanvasItemInfo *info );
int gnoclCanvasAppendPath( Tcl_Interp *interp, Tcl_Obj *obj, int k, 
      GnomeCanvasPathDef *path );

gnoclOptFunc gnoclOptCanvasTags;
gnoclOptFunc gnoclOptJoinStyle;
gnoclOptFunc gnoclOptCapStyle;
gnoclOptFunc gnoclOptDash;
gnoclOptFunc gnoclOptXY;
gnoclOptFunc gnoclOptPoints;
gnoclOptFunc gnoclOptPath;
gnoclOptFunc gnoclOptParent;
gnoclOptFunc gnoclItemOptOnFunc;

#endif

