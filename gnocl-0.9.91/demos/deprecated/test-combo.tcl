#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-combo.tcl,v 1.1 2005/02/22 23:16:19 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../../src]]
package require Gnocl

set combo [gnocl::combo]

proc assert { opt val } {
   set val2 [$::combo cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

foreach opt {-editable -mustMatch -allowEmpty -caseSensitive -enableArrowKeys \
      -enableArrowsAlways -visible -sensitive } {
   foreach val {0 1 0 1} {
      $combo configure $opt $val
      assert $opt $val
   }
}

foreach opt {-data -value -onChanged -variable -name -tooltip -onShowHelp } {
   foreach val {"qqq" "bbb" "" "ddd" "" } {
      $combo configure $opt $val
      assert $opt $val
   }
}

foreach val { "a b c d e" "" "1 2 3"  "1" } {
      $combo configure -items $val
      assert -items $val
}

$combo delete

puts "----- automatic tests done ------------"



set noUpdates 0

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$left $right"]
set str {"Cow" "Dog" "Bird" "Parrot" "Cat" "Lion" "White Shark"}

set label [gnocl::label]
set combo [gnocl::combo -variable comboVar -tooltip "This is a combo widget" \
      -onChanged "onChanged %w %v $label comboVar" \
      -onShowHelp {puts "%w onShowHelp %h"} -items $str]

$left add [gnocl::label -text "%__Combo" -mnemonicWidget $combo]
$left add $combo 
$left add $label 

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "$combo configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$combo configure -visible %v"]
$right add [gnocl::checkButton -text "%__Editable" -active 1 \
      -onToggled "$combo configure -editable %v"]
$right add [gnocl::checkButton -text "mustMatch" -active 0 \
      -onToggled "$combo configure -mustMatch %v"]
$right add [gnocl::checkButton -text "allowEmpty" -active 0 \
      -onToggled "$combo configure -allowEmpty %v"]
$right add [gnocl::checkButton -text "caseSensitive" -active 0 \
      -onToggled "$combo configure -caseSensitive %v"]
$right add [gnocl::checkButton -text "enableArrowKeys" -active 1 \
      -onToggled "$combo configure -enableArrowKeys %v"]
$right add [gnocl::checkButton -text "enableArrowsAlways" -active 0 \
      -onToggled "$combo configure -enableArrowsAlways %v"]

proc onChanged {widget val label var} {
   incr ::no
   puts [format "%3d: %s = %s" $::no $val [set ::$var]]
   $label configure -text [format "%3d: %s = %s" $::no $val [set ::$var]]
   # [$widget cget -value]
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

proc assertValue { val } {
   if { [$::combo cget -value] != $val || $::comboVar != $val } {
      error [format "%s != %s != %s" $val [$::combo cget -value] $::comboVar]
   }
}

set no 0
# set value via -value
$combo configure -value Bird
assertValue Bird
if { $no != 0 } { error "configure should not call callback command" }
# set value via variable
set comboVar "set via variable"
assertValue "set via variable"
if { $no != 1 } { error "changing variable should call callback command" }
# test onChanged
$combo onChanged
if { $no != 2 } { error "onChanged did not call callback command" }
# tests without -onChanged
set en2 [gnocl::combo -variable var2]
if { [string length $var2] != 0 } {
   error "variable should be set to empty string"
}
$en2 delete

puts "Automatic tests done."


gnocl::mainLoop

