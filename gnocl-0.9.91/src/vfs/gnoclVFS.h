#ifndef GNOCLVFS_H_INCLUDED
#define GNOCLVFS_H_INCLUDED

/*
 * $Id: gnoclVFS.h,v 1.1 2004/08/04 18:57:05 baum Exp $
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
#include <libgnomevfs/gnome-vfs.h>

Tcl_ObjCmdProc gnoclFileCmd;
Tcl_ObjCmdProc gnoclMimeCmd;

char *gnoclMakeURI( Tcl_Interp *interp, Tcl_Obj *obj );

#endif

