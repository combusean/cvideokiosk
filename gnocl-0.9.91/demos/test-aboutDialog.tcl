#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-aboutDialog.tcl,v 1.1 2005/08/30 20:19:18 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

gnocl::aboutDialog -artists {artist1 artist2} -authors {author1 author2} \
      -name "Gnocl" -comments "This is a comment. Blub, blah, blah" \
      -copyright "Copyright by qqq" -documenters {documenter1 documenter2} \
      -license "This is a license" -version "Version 1.2.3" \
      -website "http://www.dr-baum.net/gnocl/" -logo "%/./two.jpg"
gnocl::mainLoop


