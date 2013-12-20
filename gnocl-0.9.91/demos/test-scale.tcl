#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-scale.tcl,v 1.11 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

proc assert { opt val } {
   set val2 [$::scale cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set scale [gnocl::scale -orientation vertical]
assert -orientation vertical
$scale delete
set scale [gnocl::scale -orientation horizontal]
assert -orientation horizontal

foreach opt {-data -onValueChanged -variable -onShowHelp -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $scale configure $opt $el
      assert $opt $el
   }
}
foreach opt { -lower -upper -stepInc -pageInc  } {
   foreach el { 1. 5. -1 3. } {
      $scale configure $opt $el
      assert $opt $el
   }
}

foreach opt {-visible -sensitive -drawValue -inverted } {
   foreach val {0 1 0 1} {
      $scale configure $opt $val
      assert $opt $val
   }
}

foreach val { 0 1 5 } {
   $scale configure -digits $val
      assert -digits $val
}

foreach val { continuous discontinuous delayed } {
   $scale configure -updatePolicy $val
      assert -updatePolicy $val
}

$scale delete

puts "----- automatic tests done ------------"

set noUpdates 0

set left [gnocl::table -homogeneous 0]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left \
      -fill 1 -expand 1]
$mainBox addEnd $right

set horNo 0
set verNo 0
set horLabel [gnocl::label]
set verLabel [gnocl::label]
set scaleHor [gnocl::scale -orientation horizontal -variable varHor \
      -onValueChanged "setVal %w %v varHor $horLabel horNo" \
      -tooltip "This is a horizontal scale" \
      -onShowHelp {puts "%w onShowHelp %h"}]
set scaleVer [gnocl::scale -variable varVer \
      -onValueChanged "setVal %w %v varVer $verLabel verNo" \
      -tooltip "This is a horizontal scale" \
      -onShowHelp {puts "%w onShowHelp %h"}]

# [gnocl::label -text "%__Spinbutton" -mnemonicWidget $spinButton] 

$left add $scaleHor 0 0 -fill 1 -columnSpan 2
$left add $scaleVer 1 1 -fill 1
$left addRow $horLabel -columnSpan 2 -expand 0 -fill 0
$left addRow $verLabel -columnSpan 2 -expand 0 -fill 0

proc config { option value } {
   $::scaleHor configure $option $value
   $::scaleVer configure $option $value
}
proc setVal { widget val var label no } {
   incr ::$no
   $label configure -text [format "%3d: %s = %s = %s" [set ::$no] \
         $val [set ::$var] [$widget cget -value]]
}

$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "config -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "config -visible %v"]
$right add [gnocl::checkButton -text "inverted" -active 0 \
      -onToggled "config -inverted %v"]
$right add [gnocl::checkButton -text "drawValue" -active 1 \
      -onToggled "config -drawValue %v"]

$right add [gnocl::label -text "digits"]
$right add [gnocl::spinButton -lower 0 -upper 5 -digits 0 -value 1 \
      -onValueChanged "config -digits %v"]

$right add [gnocl::label -text "valuePos"]
$right add [gnocl::optionMenu -items {top bottom left right} \
      -onChanged "config -valuePos %v"]

$right add [gnocl::label -text "updatePolicy"]
$right add [gnocl::optionMenu -items {continuous discontinuous delayed} \
      -onChanged "config -updatePolicy %v"]

$right add [gnocl::label -text "lower"]
$right add [gnocl::optionMenu -items {-10 -5.5 0 5.5} -value 0 \
      -onChanged "config -lower %v"]

$right add [gnocl::label -text "upper"]
$right add [gnocl::optionMenu -items {0 100} -value 100 \
      -onChanged "config -upper %v"]

$right add [gnocl::label -text "stepInc"]
$right add [gnocl::optionMenu -items {0.5 1} -value 1 \
      -onChanged "config -stepInc %v"]

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
   if { [$::scaleHor cget -value] != $val || $::varHor != $val } {
      error [format "%s != %s != %s" $val [$::scaleHor cget -value] $::varHor]
   }
}
# set value via -value
$scaleHor configure -value 40
assertValue 40
if { $horNo != 0 } { error "configure should not call callback command" }
# set value via variable
set varHor 30
assertValue 30
if { $horNo != 1 } { error "changing variable should call callback command" }
# test onChanged
$scaleHor onValueChanged
if { $horNo != 2 } { error "onValueChanged did not call callback command" }

puts "Automatic tests done."

gnocl::mainLoop

