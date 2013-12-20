#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-spinButton.tcl,v 1.10 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

proc assert { opt val } {
   set val2 [$::spinButton cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set spinButton [gnocl::spinButton]

foreach opt {-data -onValueChanged -variable -onShowHelp -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" "" } {
      $spinButton configure $opt $el
      assert $opt $el
   }
}
foreach opt { -lower -upper -stepInc -pageInc  } {
   foreach el { 1. 5. -1 3. } {
      $spinButton configure $opt $el
      assert $opt $el
   }
}

foreach opt {-editable -visible -sensitive -wrap } {
   foreach val {0 1 0 1} {
      $spinButton configure $opt $val
      assert $opt $val
   }
}

foreach val { 0 1 5 } {
   $spinButton configure -digits $val
      assert -digits $val
}

$spinButton delete

puts "----- automatic tests done ------------"

set noUpdates 0

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
$mainBox addEnd $right

set no 0
set valLabel [gnocl::label]
set spinButton [gnocl::spinButton -variable var \
      -onValueChanged "setVal %w %v var $valLabel no" \
      -tooltip "This is a spinButton" -onShowHelp {puts "%w onShowHelp %h"}]

$left add [gnocl::box -orientation horizontal -padding 4 \
      -children [list \
            [gnocl::label -text "%__Spinbutton" -mnemonicWidget $spinButton] \
            $spinButton]]
$left add $valLabel

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "$spinButton configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$spinButton configure -visible %v"]
$right add [gnocl::checkButton -text "%_E_ditable" -active 1 \
      -onToggled "$spinButton configure -editable %v"]
$right add [gnocl::checkButton -text "numeric" -active 1 \
      -onToggled "$spinButton configure -numeric %v"]
$right add [gnocl::checkButton -text "wrap" -active 0 \
      -onToggled "$spinButton configure -wrap %v"]
set spin [gnocl::spinButton -lower 1 -upper 20 -digits 0 -value 20 \
      -onValueChanged "$spinButton configure -maxLength %v"]
$right add [gnocl::label -text "%_ma_xLength" -mnemonicWidget $spin]
$right add $spin

$right add [gnocl::label -text "digits"]
$right add [gnocl::spinButton -lower 0 -upper 5 -digits 0 -value 1 \
      -onValueChanged "$spinButton configure -digits %v"]
$right add [gnocl::label -text "lower"]
set menu [gnocl::optionMenu -onChanged "$spinButton configure -lower %v"]
foreach el {"-10" "-5.5" "0" "5.5"} {
   $menu add $el -value $el
}
$right add $menu
$right add [gnocl::label -text "upper"]
set menu [gnocl::optionMenu -onChanged "$spinButton configure -upper %v"]
foreach el {"0" "100"} {
   $menu add $el -value $el
}
$right add $menu
$right add [gnocl::label -text "stepInc"]
set menu [gnocl::optionMenu -onChanged "$spinButton configure -stepInc %v"]
foreach el {"0.5" "1"} {
   $menu add $el -value $el
}
$right add $menu

proc setVal { widget val var label no } {
   incr ::$no
   $label configure -text [format "#%3d %s = %s = %s" \
         [set ::$no] $val [$widget cget -value] [set ::$var]]
}

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

proc assertValue { val } {
   if { [$::spinButton cget -value] != $val || $::var != $val } {
      error [format "%s != %s != %s" $val [$::spinButton cget -value] $::var]
   }
}
# set value via -value
$spinButton configure -value 40
assertValue 40
if { $no != 0 } { error "configure should not call callback command" }
# set value via configure -value
$spinButton configure -value 20
assertValue 20
if { $no != 0 } { error "configure -value should not call callback command" }
# set value via variable
set var 30
assertValue 30
if { $no != 1 } { error "changing variable should call callback command" }
puts "Automatic tests done."

gnocl::mainLoop

