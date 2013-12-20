#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-box3.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical]

set boxList ""
foreach expand {1 0} {
   foreach fill {1 0} {
      set bb [gnocl::box -children [gnocl::button -text "-1-"] \
            -label "expand $expand fill $fill" \
            -expand $expand -fill $fill] 
      $bb addEnd [list [gnocl::button -text "5"] \
            [gnocl::button -text "4"]] \
            -expand $expand -fill $fill
      $bb addBegin [list [gnocl::button -text "2"] \
            [gnocl::button -text "3"]] \
            -expand $expand -fill $fill
      $mainBox add $bb
      lappend boxList $bb
   }
}

foreach el {add addBegin addEnd} {
   set bb [gnocl::box -label $el]
   $mainBox add $bb
   $bb $el [gnocl::button -text "1"]
   $bb $el [gnocl::button -text "2"]
   $bb $el [gnocl::button -text "3"]
   lappend boxList $bb
}

$mainBox add [gnocl::checkButton -text "homogeneous" \
   -onToggled "config -homogeneous %v"]

proc config { opt val } {
   foreach el $::boxList {
      $el configure $opt $val
   }
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 400 -defaultHeight 400 -onDestroy exit]
}

gnocl::mainLoop

