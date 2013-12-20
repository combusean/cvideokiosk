#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-box2.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

proc config { } {
   $::hBox configure -children $::hChildren -fill [list $::xFill $::yFill] \
         -expand $::expand -align $::align
   $::vBox configure -children $::vChildren -fill [list $::xFill $::yFill] \
         -expand $::expand -align $::align
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

set hChildren [list [gnocl::button -text "%#Ok"] \
      [gnocl::button -text "%#Cancel"] [gnocl::button -text "%#Help"]]
set vChildren [list [gnocl::button -text "%#Ok"] \
      [gnocl::button -text "%#Cancel"] [gnocl::button -text "%#Help"]]

set hBox [gnocl::box -orientation horizontal -children $hChildren \
      -shadow etchedIn]
set vBox [gnocl::box -orientation vertical -children $vChildren \
      -shadow etchedIn]

set left [gnocl::box -orientation vertical -children "$hBox $vBox" \
      -homogeneous 1 -expand 1 -fill 1 -shadow etchedIn -label "left" ]

set right [gnocl::box -orientation vertical -homogeneous 0 -shadow etchedIn]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -fill 1 -expand 1 -homogeneous 0]
$mainBox add $right -expand 0

$right add [gnocl::checkButton -text "%__homogeneous" -active 0 \
      -onToggled \
      "$hBox configure -homogeneous %v; $vBox configure -homogeneous %v"] \
      -expand 0
$right add [gnocl::checkButton -text "%__Expand" -active 1 \
      -onToggled "config" -variable expand] -expand 0
$right add [gnocl::label -text "x fill:"] -expand 0
$right add [gnocl::spinButton -onValueChanged "config" \
      -lower 0 -upper 1 -stepInc 0.1 -variable xFill -value 1] -expand 0
$right add [gnocl::label -text "y fill:"] -expand 0
$right add [gnocl::spinButton -onValueChanged config \
      -lower 0 -upper 1 -stepInc 0.1 -variable yFill -value 1] -expand 0
$right add [gnocl::label -text "align"] -expand 0
set alignMenu [gnocl::optionMenu -onChanged "config" -variable align]
foreach el {topLeft top topRight left center right bottomLeft \
      bottom bottomRight "0.2 0.2" "0.2 0.8" "0.8 0.8" } {
   $alignMenu add $el -value $el
}
$alignMenu configure -value center
$right add $alignMenu -expand 0

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultHeight 800 -defaultWidth 500 -onDestroy exit]
}

gnocl::mainLoop

