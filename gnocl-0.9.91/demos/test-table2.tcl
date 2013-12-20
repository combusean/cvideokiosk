#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-table2.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation horizontal]

set table [gnocl::table -homogeneous 0 -label "Align default (center)"]
$mainBox add $table -fill 1 -expand 1
$table addRow [list [gnocl::button -text "expand 0\n"] \
      [gnocl::button -text "fill 0"]] -expand 0 -fill 0
$table addRow [list [gnocl::button -text "expand 1\n"] \
      [gnocl::button -text "fill 0"]] -expand 1 -fill 0
$table addRow [list [gnocl::button -text "expand 1\n"] \
      [gnocl::button -text "fill 1"]] -expand 1 -fill 1
$table addRow [list [gnocl::button -text "default\n"] \
      [gnocl::button -text "default"]]

foreach align {top bottom} {
   set table [gnocl::table -homogeneous 0 -label "Align $align"]
   $mainBox add $table
   $table addRow [list [gnocl::button -text "expand 0\n"] \
         [gnocl::button -text "fill 0"]] -expand 0 -fill 0 -align $align
   $table addRow [list [gnocl::button -text "expand 1\n"] \
         [gnocl::button -text "fill 0"]] -expand 1 -fill 0 -align $align
   $table addRow [list [gnocl::button -text "expand 1\n"] \
         [gnocl::button -text "fill 1"]] -expand 1 -fill 1 -align $align
   $table addRow [list [gnocl::button -text "default\n"] \
         [gnocl::button -text "default"]]
}



if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -defaultHeight 500 -defaultWidth 500 -onDestroy exit]
}

gnocl::mainLoop

