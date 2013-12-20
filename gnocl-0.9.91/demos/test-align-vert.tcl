#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-align-vert.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical]

set box [gnocl::box -orientation horizontal -label "With box"]
$mainBox add $box
$box add [gnocl::label -text "1\n2\n3\n4\n5"]
$box add [gnocl::label -text "Top"] -align 0 -fill 0
$box add [gnocl::label -text "Top 20%"] -align 0.2 -fill 0
$box add [gnocl::separator -orientation vertical] -fill 1
$box add [gnocl::label -text "Center (Default)"] -fill 0
$box add [gnocl::separator -orientation vertical] -fill 1
$box add [gnocl::label -text "Bottom 80%"] -align 0.8 -fill 0
$box add [gnocl::label -text "Bottom"] -align 1 -fill 0

set table [gnocl::table -homogeneous 0 -label "With table"]
$mainBox add $table
$table add [gnocl::label -text "1\n2\n3\n4\n5"] 0 0
$table add [gnocl::label -text "Top"] 1 0 -align 0 -fill 0
$table add [gnocl::label -text "Top 20%"] 2 0 -align 0.2 -fill 0
$table add [gnocl::label -text "Center (Default)"] 3 0 -fill 0
$table add [gnocl::label -text "Bottom 80%"] 4 0 -align 0.8 -fill 0
$table add [gnocl::label -text "Bottom"] 5 0 -align 1 -fill 0

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

