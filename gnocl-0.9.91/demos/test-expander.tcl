#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-expander.tcl,v 1.2 2005/02/25 22:14:40 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../src]]
package require Gnocl

set right [gnocl::box -orientation vertical]
set left [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -expand 1 -fill 1 -homogeneous 0]
$mainBox add $right -expand 0
set expander [gnocl::expander -label Gnocl \
      -child [gnocl::label -text "Something"]]
$left add $expander -fill 1 -expand 1

proc addRight { txt widget } {
   $::right add [gnocl::box -borderWidth 0 -children [list \
         [gnocl::label -align left -text $txt -widthGroup labelGroup] \
         $widget] -expand 1 -fill 1]
}

proc configure { opt val } {
   $::expander configure $opt $val
}
addRight label [gnocl::comboBox \
      -items {"Gnocl" "%_G_nocl" "%<Bold: <b>bold</b>" ""} \
      -onChanged "configure -label %v"]
$right add [gnocl::checkButton -text "Expand" \
      -onToggled "configure -expand %v"]

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

