#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-paned.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set paned ""
set lShrink 1
set rShrink 1
set lResize 1
set rResize 1

proc makePaned { } {
   if { [string length $::paned] } {
      $::paned delete
   }
   set lChild [gnocl::button -text "left child"]
   set rChild [gnocl::button -text "right child"]
   set ::paned [gnocl::paned -children [list $lChild $rChild] \
         -shrink [list $::lShrink $::rShrink] \
         -resize [list $::lResize $::rResize]] 
   $::left add $::paned
}

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal]
$mainBox add $left -fill 1 -expand 1
$mainBox addEnd $right

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled {$paned configure -sensitive %v}]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled {$paned configure -visible %v}]
$right add [gnocl::label -text "position"]
$right add [gnocl::spinButton -lower 20 -upper 500 -digits 0 -value 50 \
      -onValueChanged {$paned configure -position %v}]
$right add [gnocl::label -text "resize"]
$right add [gnocl::box -orientation horizontal -children [list \
    [gnocl::checkButton -text "left" -variable lResize -onToggled makePaned] \
    [gnocl::checkButton -text "right" -variable rResize -onToggled makePaned]]]
$right add [gnocl::label -text "shrink"]
$right add [gnocl::box -orientation horizontal -children [list \
     [gnocl::checkButton -text "left" -variable lShrink -onToggled makePaned] \
     [gnocl::checkButton -text "right" -variable rShrink -onToggled makePaned]]]

makePaned

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

