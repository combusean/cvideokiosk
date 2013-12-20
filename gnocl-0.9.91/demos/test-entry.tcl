#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-entry.tcl,v 1.13 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl



set entry [gnocl::entry]

proc assert { opt val } {
   set val2 [$::entry cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

foreach opt {-editable -textVisible -visible -sensitive } {
   foreach val {0 1 0 1} {
      $entry configure $opt $val
      assert $opt $val
   }
}

foreach opt {-maxLength -widthChars } {
   foreach val { 10 30 0 0 20 } {
      $entry configure $opt $val
      assert $opt $val
   }
}

foreach opt {-data -value -onChanged -onActivate -variable -name -tooltip \
      -onShowHelp -widthGroup -heightGroup -sizeGroup} {
   foreach val {"qqq" "bbb" "" "" "ddd" "" "eee" } {
      $entry configure $opt $val
      assert $opt $val
   }
}

$entry delete

puts "----- automatic tests done ------------"


set no 0

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
$mainBox addEnd $right

set label [gnocl::label]
set entry [gnocl::entry -variable var -tooltip "This is an entry widget" \
      -onActivate {puts "%w activated" } \
      -onShowHelp {puts "%w onShowHelp %h"} \
      -onChanged "onChanged %w %v $label var" ]

$left add [gnocl::label -text "%__Entry" -mnemonicWidget $entry] 
$left add $entry 
$left add $label 

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "$entry configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$entry configure -visible %v"]
$right add [gnocl::checkButton -text "%_E_ditable" -active 1 \
      -onToggled "$entry configure -editable %v"]
$right add [gnocl::checkButton -text "%__TextVisible" -active 1 \
      -onToggled "$entry configure -textVisible %v"]
set spin [gnocl::spinButton -lower 1 -upper 20 -digits 0 -value 20 \
      -onValueChanged "$entry configure -maxLength %v"]
$right add [gnocl::label -text "%_ma_xLength" -mnemonicWidget $spin]
$right add $spin
set spin [gnocl::spinButton -lower 1 -upper 20 -digits 0 -value 20 \
      -onValueChanged "$entry configure -widthChars %v"]
$right add [gnocl::label -text "%__widthChars" -mnemonicWidget $spin]
$right add $spin

proc onChanged {widget val label var} {
   incr ::no
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
   if { [$::entry cget -value] != $val || $::var != $val } {
      error [format "%s != %s != %s" $val [$::entry cget -value] $::var]
   }
}

# set value via -value
$entry configure -value "set via -value"
assertValue "set via -value"
if { $no != 0 } { error "configure should not call callback command" }
# set value via variable
set var "set via variable"
assertValue "set via variable"
if { $no != 1 } { error "changing variable should call callback command" }
# test onChanged
$entry onChanged
if { $no != 2 } { error "onChanged did not call callback command" }
# tests without -onChanged
set en2 [gnocl::entry -variable var2]
if { [string length $var2] != 0 } {
   error "variable should be set to empty string"
}
$en2 delete

puts "Automatic tests done."

gnocl::mainLoop

