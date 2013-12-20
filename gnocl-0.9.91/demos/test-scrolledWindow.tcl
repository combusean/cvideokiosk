#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-scrolledWindow.tcl,v 1.4 2004/12/02 20:56:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation horizontal -homogeneous 0]
set right [gnocl::box -orientation vertical]

set box [gnocl::box -orientation vertical -label "Box" ]
set table [gnocl::table -label "Table"] 
foreach el {1 2 3 4 5 6 7 8 9 10} {
   $box add [gnocl::button -text "Button $el"]
   $table addRow [list [gnocl::button -text "Button $el,0"] \
         [gnocl::button -text "Button $el,1"]]
}

set scrollWin1 [gnocl::scrolledWindow -child $box -onChanged "printInfo %w"]
set scrollWin2 [gnocl::scrolledWindow -child $table]

$mainBox add $scrollWin1 -fill 1 -expand 1
$mainBox add $scrollWin2 -fill 1 -expand 1
$mainBox add $right -expand 0

proc printInfo { w } {
   foreach el {xLower xPageSize xUpper xValue} {
      puts [format "%s %s" $el [$w cget -$el]]
   }
}

proc configure { opt val } {
   $::scrollWin1 configure $opt $val
   $::scrollWin2 configure $opt $val
}

$right add [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "configure -visible %v"]
$right add [gnocl::label -text "scrollbar"]
$right add [gnocl::optionMenu  -onChanged "configure -scrollbar %v" \
      -items "never always automatic"]
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "borderWidth:" -align left -widthGroup label] \
      [gnocl::spinButton -onValueChanged "configure -borderWidth %v" \
            -digits 0 -upper 30]] -expand 0]
foreach el {xValue yValue} {
   $right add [gnocl::box -borderWidth 0 -children [list \
         [gnocl::label -text "$el:" -align left -widthGroup label] \
         [gnocl::spinButton -onValueChanged "configure -$el %v" \
               -digits 0 -upper 50]] -expand 0]
}


if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox \
         -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 300 -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

