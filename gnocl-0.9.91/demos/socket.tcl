#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: socket.tcl,v 1.7 2004/08/25 19:26:27 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical]
$mainBox add [gnocl::label -text "pid: [pid]"]
set sock [gnocl::socket]
$mainBox add [gnocl::box -label "Socket Widget" -children $sock]

set win [gnocl::window -title "Socket plug Tests" -child $mainBox \
      -onDestroy exit]
# wait that all widgets are shown
gnocl::update
puts [$sock getID]
set erg ""
catch {exec ./test-plug.tcl [$sock getID] &} erg
puts $erg

gnocl::mainLoop

