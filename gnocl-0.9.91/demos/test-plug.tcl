#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-plug.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

if { $argc != 1 } {
   puts "usage: $argv0 socket-XID"
   exit
}

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -label "Plug widget" -orientation vertical]
$mainBox add [gnocl::label -text "pid: [pid]"]

set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
gnocl::mainLoop

