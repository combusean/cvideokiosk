#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-checkButton.tcl,v 1.14 2005/02/25 21:51:53 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

proc assert { opt val } {
   set val2 [$::check cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set check [gnocl::checkButton]

foreach opt { -data -onToggled -variable -onShowHelp -onPopupMenu \
      -onButtonRelease -onButtonPress -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $check configure $opt $el
      assert $opt $el
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "%<<b>Pango</b>"
      "%#Quit" } {
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

foreach val { "normal" "half" "none" "normal" } {
   $check configure -relief $val
   assert -relief $val
}

foreach opt {-visible -sensitive -active -indicatorOn -inconsistent } {
   foreach val {0 1 0 1} {
      $check configure $opt $val
      assert $opt $val
   }
}

foreach opt {-normalBackgroundColor -activeBackgroundColor \
      -prelightBackgroundColor} {
   foreach val {"65535 0 0" "0 65535 0" "0 0 65535"} {
      $check configure $opt $val
      assert $opt $val
   }
}

$check delete

puts "----- automatic tests done ------------"


puts -nonewline "\nTesting variable not set, active not set: "
catch { unset var }
set check [gnocl::checkButton -text "Check" -onValue on -offValue off \
      -variable var -onToggled {error "test failed"}]
set vget [$check cget -value]
puts -nonewline [format "off == %s " $vget]
if { [string compare $vget off] } { puts "\n"; error "test failed" }
$check delete

puts -nonewline "--- Ok\nTesting variable not set, active     set: "
foreach el {0 1} {
   catch { unset var }
   set check [gnocl::checkButton -text "Check" -onValue on -offValue off \
         -variable var -active $el -onToggled {error "test failed"}]
   set vget [$check cget -value]
   puts -nonewline [format "%s == %s " $var $vget]
   if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   $check delete
}

puts -nonewline "--- Ok\nTesting variable     set, active     set: "
foreach el {0 1} elTxt {off on} {
   set var on
   set check [gnocl::checkButton -text "Check" -onValue on -offValue off \
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
   set check [gnocl::checkButton -text "Check" -onValue on -offValue off \
         -variable var -onToggled {error "test failed"}]
   set vget [$check cget -value]
   puts -nonewline [format "%s == %s " $var $vget]
   if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   $check delete
}
puts "--- Ok\nAutomatic test done\n\n"


set noUpdates 0
set left [gnocl::box -orientation vertical]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$left $right"]


$left add [gnocl::checkButton -text \
      {%<different <span color="red">_Colors</span>} \
      -normalBackgroundColor blue \
      -activeBackgroundColor red -prelightBackgroundColor green \
      -onButtonRelease {puts "button release %b"} -onButtonPress {puts "button pressed %b"} \
      -tooltip "Tooltip Color"] -fill 0

set check [gnocl::checkButton -text "%__Status" -onValue "Yes" \
      -offValue "No" -variable status -onToggled update \
      -tooltip "Tooltip Status"]
set label [gnocl::label]
$left add "$check $label" -fill 0

$right add [gnocl::checkButton -text "%_S_ensitive" -active 1 \
      -onToggled "$check configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$check configure -visible %v"]
$right add [gnocl::checkButton -text "%__Indicator" -active 1 \
      -onToggled "$check configure -indicatorOn %v"]
$right add [gnocl::checkButton -text "%__Tooltip" -active 1 \
      -onToggled "gnocl::configure -tooltip %v"]
$right add [gnocl::button -text "Set variable \"Yes\"" \
      -onClicked "set status Yes"]
$right add [gnocl::button -text "Set variable \"No\"" \
      -onClicked "set status No"]
$right add [gnocl::button -text "Set button \"Yes\"" \
      -onClicked "$check configure -value Yes"]
$right add [gnocl::button -text "Set button \"No\"" \
      -onClicked "$check configure -value No"]

proc update { args } {
   $::label configure -text [format "%3d: %s == %s" \
         $::noUpdates $::status [$::check cget -value]]
   incr ::noUpdates
}

update

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

