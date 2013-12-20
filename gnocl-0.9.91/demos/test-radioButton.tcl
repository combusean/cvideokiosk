#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-radioButton.tcl,v 1.12 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl


proc assert { opt val } {
   set val2 [$::radio cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set radio [gnocl::radioButton -variable qqq -onValue 0]
set radio2 [gnocl::radioButton -variable qqq -onValue 2]
assert -variable qqq

foreach opt {-data -onValue -onToggled -onShowHelp -onPopupMenu \
      -onButtonRelease -onButtonPress -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $radio configure $opt $el
      assert $opt $el
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "%<<b>Pango</b>"
      "%#Quit" } {
   $radio configure -text $el
   assert -text $el
}

foreach val { "normal" "half" "none" "normal" } {
   $radio configure -relief $val
   assert -relief $val
}

foreach opt {-visible -sensitive -indicatorOn } {
   foreach val {0 1 0 1} {
      $radio configure $opt $val
      assert $opt $val
   }
}

$radio configure -active 1
assert -active 1
$radio2 configure -active 1
assert -active 0
$radio configure -active 1
assert -active 1

$radio delete
$radio2 delete

puts "----- automatic tests done ------------"



set left [gnocl::box -orientation vertical -shadow in]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$left $right"]

set label [gnocl::label]
set radio [gnocl::radioButton -text "%__Cats" -onValue cats -variable animal \
      -tooltip "Tooltip Cats" -onToggled "configLabel $label %w %v" \
      -onButtonRelease {puts "button release %b"} -onButtonPress {puts "button pressed %b"} \
      -onShowHelp {puts "%w show help %h"} -onPopupMenu {puts "%w popup menu"}]

$left add $radio
set radio2 [gnocl::radioButton -text "%__Dogs" -onValue dogs -variable animal \
      -tooltip "Tooltip Dogs" -onToggled "configLabel $label %w %v"]
$left add $radio2
set radio2 [gnocl::radioButton -text "%__Birds" -onValue birds \
      -variable animal -tooltip "Tooltip Birds" \
      -onToggled "configLabel $label %w %v"]
$left add $radio2
$left add $label

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "$radio configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$radio configure -visible %v"]
$right add [gnocl::checkButton -text "%__Indicator" -active 1 \
      -onToggled "$radio configure -indicatorOn %v"]
$right add [gnocl::button -text "%_S_et variable to \"cats\"" \
      -onClicked "set animal cats"]
$right add [gnocl::button -text "%_Se_t variable to \"dogs\"" \
      -onClicked "set animal dogs"]
$right add [gnocl::button -text "%_Set v_ariable to \"birds\"" \
      -onClicked "set animal birds"]


proc configLabel { label widget value } {
   $label configure -text "$value == $::animal == [$widget cget -value]"
}

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

