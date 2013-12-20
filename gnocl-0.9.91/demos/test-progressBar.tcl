#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-progressBar.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set noUpdates 0

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$left $right"]

# set pulse [gnocl::progressBar -activityMode 1 -text "Activity Mode" -showText 1]
set fract 0
set pulse [gnocl::progressBar -activityMode 1]
$left add $pulse 
set bar [gnocl::progressBar]
$left add $bar 

$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$bar configure -visible %v"]
$right add [gnocl::checkButton -text "showText" -active 1 \
      -onToggled "$bar configure -showText %v"]

set opt [gnocl::optionMenu -onChanged "$bar configure -orientation %v" \
      -items {leftToRight rightToLeft bottomToTop topToBottom}]
$right add $opt
$right add [gnocl::label -text "pulseStep"]
$right add [gnocl::spinButton -lower 0.1 -upper 0.6 -stepInc 0.1 -digits 1 \
      -value 0.2 -onValueChanged "$pulse configure -pulseStep %v"]

$right add [gnocl::label -text "textAlign"]
set alignMenu [gnocl::optionMenu -onChanged "$bar configure -textAlign %v" \
      -items {topLeft top topRight left center right bottomLeft \
      bottom bottomRight "0.2 0.2" "0.2 0.8" "0.8 0.8" }]
$right add $alignMenu


proc doPulse { } {
   $::pulse pulse
   after 10 doPulse
}
proc doFract { } {
   global fract
   set fract [expr {$fract+0.05}]
   if { $fract > 1 } {
      set fract 0
   }
   $::bar configure -fraction $fract -text \
         [format "%.0f %%" [expr {$fract*100.}]]
   after 10 doFract
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

doPulse
doFract

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

