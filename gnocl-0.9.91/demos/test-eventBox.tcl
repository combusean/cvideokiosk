#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-eventBox.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set noUpdates 0

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left \
      -fill 1 -expand 1]
$mainBox addEnd $right

set onButtonNo 0
set onMotionNo 0
set buttonLabel [gnocl::label]
set motionLabel [gnocl::label]
set box [gnocl::eventBox -tooltip "eventBox" \
      -onButtonPress "onButton %t %x %y %b %s" \
      -onMotion "onMotion %x %y %s" \
      -onDestroy {puts "eventBox %w is destroyed"} \
      -child [gnocl::label -text "Label in a\neventBox"]]

# [gnocl::label -text "%__Spinbutton" -mnemonicWidget $spinButton] 

$left add [list $box $motionLabel $buttonLabel]

proc onMotion { x y state } {
   global onMotionNo

   $::motionLabel configure -text \
         "$onMotionNo: motion x: $x y: $y state: $state"
   incr onMotionNo
}

proc onButton { type x y but state } {
   global onButtonNo

   $::buttonLabel configure -text \
         "$onButtonNo: button $but $type x: $x y: $y state: $state"
   incr onButtonNo
}

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "$box configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$box configure -visible %v"]

$right add [gnocl::label -text "background"]
$right add [gnocl::optionMenu -items {red blue green grey} \
      -onChanged "$box configure -background %v"]

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -allowShrink 0 -onDestroy exit]
}
gnocl::mainLoop

