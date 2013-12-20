#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-menu.tcl,v 1.11 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

puts "Testing menuCheckItem"
proc assert { opt val } {
   set val2 [$::check cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set check [gnocl::menuCheckItem]

foreach opt {-data -onToggled -variable -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $check configure $opt $el
      assert $opt $el
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "Quit" } {
   $check configure -text $el
   assert -text $el
}

foreach el { "on" 1 "On" } {
   $check configure -onValue $el
   assert -onValue $el
}
foreach el { "off" 0 "Off" } {
   $check configure -offValue $el
   assert -offValue $el
}

foreach opt {-visible -sensitive -active -inconsistent } {
   foreach val {0 1 0 1} {
      $check configure $opt $val
      assert $opt $val
   }
}

$check delete

puts "Testing menuRadioItem"

proc assert { opt val } {
   set val2 [$::radio1 cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set radio1 [gnocl::menuRadioItem -variable qqq -onValue 1]
set radio2 [gnocl::menuRadioItem -variable qqq -onValue 2]

foreach opt {-data -onToggled -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $radio1 configure $opt $el
      assert $opt $el
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "Quit" } {
   $radio1 configure -text $el
   assert -text $el
}

foreach el { "on" 1 "On" } {
   $radio1 configure -onValue $el
   assert -onValue $el
}

foreach opt {-visible -sensitive -inconsistent } {
   foreach val {0 1 0 1} {
      $radio1 configure $opt $val
      assert $opt $val
   }
}

$radio1 configure -active 1
assert -active 1
$radio2 configure -active 1
assert -active 0
$radio1 configure -active 1
assert -active 1

$radio1 delete

puts "cget Ok"

puts -nonewline "Testing variable not set, active not set: "
catch { unset var }
set check [gnocl::menuCheckItem -text "Check" -onValue on -offValue off \
      -variable var -onToggled {error "test failed"}]
set vget [$check cget -value]
puts -nonewline [format "off == %s " $vget]
if { [string compare $vget off] } { puts "\n"; error "test failed" }
$check delete

puts -nonewline "--- Ok\nTesting variable not set, active     set: "
foreach el {0 1} {
   catch { unset var }
   set check [gnocl::menuCheckItem -text "Check" -onValue on -offValue off \
         -variable var -active $el -onToggled {error "test failed"}]
   set vget [$check cget -value]
   puts -nonewline [format "%s == %s " $var $vget]
   if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   $check delete
}

puts -nonewline "--- Ok\nTesting variable     set, active     set: "
foreach el {0 1} elTxt {off on} {
   set var on
   set check [gnocl::menuCheckItem -text "Check" -onValue on -offValue off \
         -variable var -active $el -onToggled {error "test failed"}]
   set vget [$check cget -value]
   puts -nonewline [format "%s == %s " $var $vget]
   if { [string compare $var $elTxt] } { puts "\n"; error "test failed" }
   if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   $check delete
}
puts -nonewline "--- Ok\nTesting variable     set, active not set: "
foreach el {off on} {
   set var $el
   set check [gnocl::menuCheckItem -text "Check" -onValue on -offValue off \
         -variable var -onToggled {error "test failed"}]
   set vget [$check cget -value]
   puts -nonewline [format "%s == %s " $var $vget]
   if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   $check delete
}
puts "--- Ok\n"

puts "Testing menuRadioItem"
catch { unset var }
foreach el {1 2 3} {
   set radio$el [gnocl::menuRadioItem -text "radio $el" -onValue $el \
         -variable var -onToggled {error "test failed"}]
}
foreach type {active value setVariable} {
   puts -nonewline "Testing $type"
   foreach el {1 2 3} {
      puts -nonewline " $el"
      switch $type {
         active         { [set radio$el] configure -active 1 }
         value          { $radio2 configure -value $el }
         setVariable    { set var $el }
      }
      if { [string compare $var $el] } { puts "\n"; error "test failed" }
      foreach k {1 2 3} {
         if { [string compare [[set radio$k] cget -value] $el] } {
            puts [format "\n%d %s %s" $k [[set radio$k] cget -value] $el]
            error "test failed"
         }
      }
   }
   puts " --- Ok"
}
$radio1 delete
$radio2 delete
$radio3 delete

puts -nonewline "Testing var before creation"
foreach val {1 2 3} {
   puts -nonewline " $val"
   set var $val
   foreach el {1 2 3} {
      set radio$el [gnocl::menuRadioItem -text "radio $el" -onValue $el \
            -variable var -onToggled {error "test failed"}]
   }
   if { [string compare $var $val] } { puts "\n"; error "test failed" }
   foreach k {1 2 3} {
      if { [string compare [[set radio$k] cget -value] $val] } {
         puts [format "\n%d %s %s" $k [[set radio$k] cget -value] $val]
         error "test failed"
      }
   }
   foreach el {1 2 3} {
      [set radio$el] delete
   }
}
puts " --- Ok"

puts "\nAutomatic test done\n\n"

set sensitve 1
set visible 1
set noRadio 0
set noCheck 0
set noItem 0

proc itemProc { } {
   global noItem
   $::itemLabel configure -text [format "item: %d" $noItem]
   incr noItem
}

proc radioProc { widget val value } {
   global noRadio
   $::radioLabel configure -text [format "radio: %d: %s == %s == %s" \
         $noRadio $val $value [$widget cget -value]]
   incr noRadio
}
proc checkProc { widget value } {
   global noCheck
   $::checkLabel configure -text [format "check %d: %s == %s" \
         $noCheck $value [$widget cget -value]]
   incr noCheck
}

proc configureAll { option val } {
   global items
   foreach el $items {
      $el configure $option $val
   }
}

set items ""
set menu [gnocl::menu -tearoff 1]
lappend items [gnocl::menuItem -text "%__Label" -onClicked itemProc]
lappend items [gnocl::menuCheckItem -text "%__Check" \
      -variable checkVar -onToggled "checkProc %w %v"]
lappend items [gnocl::menuSeparator]
lappend items [gnocl::menuRadioItem -text "%_Radio _1" \
      -variable radioVar -onValue 1 -onToggled "radioProc %w 1 %v"]
lappend items [gnocl::menuRadioItem -text "%_Radio _2" \
      -variable radioVar -onValue 2 -onToggled "radioProc %w 2 %v"]
lappend items [gnocl::menuRadioItem -text "%_Radio _3" \
      -variable radioVar -onValue 3 -onToggled "radioProc %w 3 %v"]
$menu add $items

set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
$mainBox addEnd $right

$left add [gnocl::button -text "Create Menu" -onClicked "$menu popup"]
set itemLabel [gnocl::label]
$left add $itemLabel
set checkLabel [gnocl::label]
$left add $checkLabel
set radioLabel [gnocl::label]
$left add $radioLabel

set box [gnocl::box -orientation vertical -label "only if menu is detached!"]
$box add [gnocl::checkButton -text "sensitive" -active 1 \
      -variable sensitive -onToggled "configureAll -sensitive %v"]
$box add [gnocl::checkButton -text "visible" -active 1 \
      -variable visible -onToggled "configureAll -visible %v"]
$right add $box

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   # gnocl::app MenuTest -title "Menu Tests" -menuBar $menuBar \
   #       -statusBar [gnocl::appBar]
   gnocl::window -title "Menu Tests" -child $mainBox -onDestroy exit
}

gnocl::mainLoop

