#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-image.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set left [gnocl::box -orientation vertical -shadow in]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
$mainBox add $right -expand 0

$left add [gnocl::label -text "animated gif"]
set fileImg [gnocl::image -image "%/./floppybuddy.gif"]
$left add $fileImg

$left add [gnocl::label -text "png image"]
set fileImg [gnocl::image -image "%/./one.png"]
$left add $fileImg

$left add [gnocl::label -text "stock image"]
set stockImg [gnocl::image -image "%#Quit"]
$left add $stockImg

$left add [gnocl::label -text "stock image dialog size"]
set img [gnocl::image -image "%#SaveAs" -stockSize dialog -align left]
$left add $img -expand 1 -fill 1

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "$img configure -sensitive %v"] -expand 0
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$img configure -visible %v"] -expand 0

$right add [gnocl::label -text "padding"] -expand 0
$right add [gnocl::spinButton -lower 0 -upper 30 -digits 0 \
      -onValueChanged "$img configure -xPad %v"]
$right add [gnocl::spinButton -lower 0 -upper 30 -digits 0 \
      -onValueChanged "$img configure -yPad %v" -tooltip toolTip1]

$right add [gnocl::label -text "Stock item"] -expand 0
$right add [gnocl::optionMenu -onChanged "$stockImg configure -image %%#%v" \
      -items [lsort [gnocl::info allStockItems]]]

$right add [gnocl::label -text "stockSize"] -expand 0
$right add [gnocl::optionMenu -onChanged "$stockImg configure -stockSize %v" \
      -value button -items "menu smallToolBar largeToolBar button dnd dialog"]

$right add [gnocl::label -text "align"] -expand 0
$right add [gnocl::optionMenu -onChanged "$img configure -align %v" \
      -items {topLeft top topRight left center right bottomLeft \
      bottom bottomRight "0.2 0.2" "0.2 0.8" "0.8 0.8"} -value left]

$right add [gnocl::label -text "size"] -expand 0
$right add [gnocl::optionMenu -onChanged "$fileImg configure -size %v" \
      -items {64 32 16 8 4 "32 16" "16 32"}]

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox \
         -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

