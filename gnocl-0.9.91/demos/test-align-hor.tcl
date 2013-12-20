#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-align-hor.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical -label "With box"]

$mainBox add [gnocl::label -text \
      "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20"]
$mainBox add [gnocl::label -text "Left"] -align 0 -fill 0
$mainBox add [gnocl::label -text "Left 20%"] -align 0.2 -fill 0
$mainBox add [gnocl::separator -orientation horizontal]  -fill 1
$mainBox add [gnocl::label -text "Center (Default)"]  -fill 0
$mainBox add [gnocl::separator -orientation horizontal]  -fill 1
$mainBox add [gnocl::label -text "Right 80%"] -align 0.8 -fill 0
$mainBox add [gnocl::label -text "Right"] -align 1  -fill 0

set table [gnocl::table -homogeneous 1 -label "With table"]
$mainBox add $table
$table add [gnocl::label -text \
      "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20"] 0 0
$table add [gnocl::label -text "Left"] 0 1 -align 0 -fill 0 
$table add [gnocl::label -text "Left 20%"] 0 2 -align 0.2 -fill 0 
$table add [gnocl::label -text "Center (Default)"]  0 3 -fill 0 
$table add [gnocl::label -text "Right 80%"] 0 4 -align 0.8 -fill 0 
$table add [gnocl::label -text "Right"] 0 5 -align 1 -fill 0 

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

