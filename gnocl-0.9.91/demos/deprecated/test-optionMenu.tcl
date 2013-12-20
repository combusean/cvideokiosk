#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-optionMenu.tcl,v 1.1 2005/02/22 23:16:19 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../../src]]
package require Gnocl

proc assert { opt val } {
   set val2 [$::om cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set om [gnocl::optionMenu]

foreach opt {-data -onChanged -variable -onShowHelp -onPopupMenu \
      -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $om configure $opt $el
      assert $opt $el
   }
}

foreach opt {-visible -sensitive } {
   foreach val {0 1 0 1} {
      $om configure $opt $val
      assert $opt $val
   }
}

foreach val { "a b d e f" "" "d e g k" "" "" "l o p q" } {
   $om configure -items $val
   assert -items $val
}

foreach val { {{a 1} {b 2} {d 3} {f 4}} {} {{g 7} {e 3}} {} {} {{h 9} {k 3}} } {
   $om configure -itemValueList $val
assert -itemValueList $val
}

$om delete

set noCalled1 0
set noCalled2 0
set callOnChanged 0

proc assertValue { val } {
   if { [$::om cget -value] != $val || $::var != $val } {
      error [format "%s != %s != %s" $val [$::entry cget -value] $::var2]
   }
}

# use value from variable
set var 5
set om [gnocl::optionMenu -variable var -items "3 5 7"]
assertValue 5
set var 7
assertValue 7
$om delete

# if -value is set use this value
set var 5
set om [gnocl::optionMenu -variable var -items "3 5 7" -value 7]
assertValue 7
set var 5
assertValue 5
$om delete

# if nothing is set use the first value
unset var
set om [gnocl::optionMenu -variable var -items "3 5 7"]
assertValue 3
$om delete

# set value via -value
set om [gnocl::optionMenu -variable var -items "3 5 7" \
      -onChanged "incr noCalled2"]
$om configure -value 5
assertValue 5
if { $noCalled2 != 0 } { 
   error "configure should not call callback command" 
}
# set value via variable
set var 7
assertValue 7
if { $noCalled2 != 1 } { 
   error "changing variable should call callback command" 
}
# test onChanged
$om onChanged
if { $noCalled2 != 2 } { error "onChanged did not call callback command" }

$om delete

puts "----- automatic tests done ------------"



set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$left $right"]

set label [gnocl::label -align left]
set om [gnocl::optionMenu -variable var1 -tooltip "OptionMenu 1" \
      -onChanged "printValue $label noCalled1 %w %v ::var1" \
      -itemValueList {"one 1" "two.5 2.5" {"six teen" "sixteen"}}]
$left add "$om $label"

set label [gnocl::label -align left]
set om [gnocl::optionMenu -variable var2 -items {qqq1 qqq2 qqq3} \
      -tooltip "OptionMenu 2" -onShowHelp {puts "%w show help %h"} \
      -onChanged "printValue $label noCalled2 %w %v ::var2"] 
$om configure -items {hello1 hello2 hello3} 
$left add "$om $label"

$left add [gnocl::checkButton -text "Call onChanged" -variable callOnChanged]
set hbox1 [gnocl::box -orientation horizontal -label "set via variable"]
set hbox2 [gnocl::box -orientation horizontal -label "set via -value"]
foreach el {1 2 3} {
   set var "hello$el"
   $hbox1 add [gnocl::button -text "op = $var" \
         -onClicked "set ::var2 $var; onChangedFunc $om"]
   $hbox2 add [gnocl::button -text "op = $var" \
         -onClicked "$om configure -value $var; onChangedFunc $om"]
}
$left add "$hbox1 $hbox2"


$right add [gnocl::checkButton -text "%_Se_nsitive" -active 1 \
      -onToggled "$om configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$om configure -visible %v"]

proc onChangedFunc { win } {
   if { $::callOnChanged } {
      $win onChanged
   }
}
proc printValue { lab cnt win val var } {
   global $cnt

   incr $cnt
   set txt [format "%3d \"%s\" = \"%s\" = \"%s\"" [set $cnt] \
         $val [$win cget -value] [set $var]]
   $lab configure -text $txt
}



if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox \
         -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

