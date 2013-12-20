#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-toolBar.tcl,v 1.12 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set bar [gnocl::toolBar]

puts "testing cget for checkItem"

proc assert { opt val } {
   set val2 [$::check cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   } else {
      puts "$opt: $val == $val2"
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" } {
   set check [$bar add checkItem -text $el]
   assert -text $el
   $check delete
}

set check [$bar add checkItem]

foreach opt {-data -onToggled -variable -onShowHelp -onPopupMenu \
      -onButtonRelease -onButtonPress -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $check configure $opt $el
      assert $opt $el
   }
}

foreach el { "on" 1 "On" } {
   $check configure -onValue $el
   assert -onValue $el
}

foreach el { "off" 0 "Off" } {
   $check configure -offValue $el
   assert -offValue $el
}

foreach opt {-visible -sensitive -active } {
   foreach val {0 1 0 1} {
      $check configure $opt $val
      assert $opt $val
   }
}

$check delete

puts "testing cget for radioItem"
proc assert { opt val } {
   set val2 [$::radio1 cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   } else {
      puts "$opt: $val == $val2"
   }
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" } {
   set radio1 [$bar add radioItem -text $el -variable qqq -onValue 0]
   assert -text $el
   $radio1 delete
}

foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
   set radio1 [$bar add radioItem -onValue $el -variable qqq]
   assert -onValue $el
   $radio1 delete
}

set radio1 [$bar add radioItem -variable qqq -onValue 0]
set radio2 [$bar add radioItem -variable qqq -onValue 2]
assert -variable qqq

foreach opt {-data -onToggled -onShowHelp -onPopupMenu \
      -onButtonRelease -onButtonPress -name -tooltip } {
   foreach el { "aaa" "bbbb" "" "ccc" "" "ddd" } {
      $radio1 configure $opt $el
      assert $opt $el
   }
}

foreach opt {-visible -sensitive } {
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
$radio2 delete

puts "----- automatic tests done ------------"

set items ""
set checkItems ""
set main [gnocl::table -homogeneous 0 -columns 3 -borderWidth 0]

set labelCheck [gnocl::label]
set labelRadio [gnocl::label]

lappend items [$bar add item -text "%#Quit" \
      -tooltip "Exits application" -onClicked exit \
      -onButtonRelease {puts "item release %b"} \
      -onButtonPress {puts "item pressed %b"}]
$bar add space
lappend items [$bar add item -text "%__NoIcon" -tooltip "No Icon" \
      -onClicked {puts "No Icon"}]
lappend items [$bar add item -text "Png" -icon "%/./one.png" \
      -tooltip "png image" -onClicked {puts "png image"}]
$bar add space
proc checkProc { widget value var } {
   $::labelCheck configure -text [format "%s: %s == %s == %s" \
      $var $value [$widget cget -value] [set ::$var]]
}
lappend checkItems [$bar add checkItem -text "Check1" -variable check1 \
      -tooltip "Check item without image" \
         -onToggled "checkProc %w %v check1" \
      -onButtonRelease {puts "check release %b"} \
      -onButtonPress {puts "check pressed %b"}]
lappend checkItems [$bar add checkItem -text "%#Underline" -variable check2 \
      -tooltip "Check item without image" \
         -onToggled "checkProc %w %v check2" ]
lappend checkItems [$bar add checkItem -text "Check3" -variable check3 \
      -icon "%/c.png" -tooltip "Check item with png image" \
      -onToggled "checkProc %w %v check3" ]
$bar add space

proc radioProc { name widget value } {
   $::labelRadio configure -text [format "radio %s: %s == %s == %s" \
      $name $value [$widget cget -value] $::radio]
}
lappend radioItems [$bar add radioItem -text "%__Radio1" -variable radio \
      -onValue 1 -tooltip "Radio item without image" \
      -onToggled "radioProc 1 %w %v" \
      -onButtonRelease {puts "radio release %b"} \
      -onButtonPress {puts "radio pressed %b"}]
lappend radioItems [$bar add radioItem -text "%#JustifyLeft" -variable radio \
      -onValue 2 -tooltip "Radio item with stock item" \
      -onToggled "radioProc 2 %w %v"]
lappend radioItems [$bar add radioItem -text "Radio3" -variable radio \
      -onValue 3 -icon "%/three.png" -tooltip "Radio item with png image" \
      -onToggled "radioProc 3 %w %v"]
$bar add space
$bar add widget [gnocl::entry -tooltip "Entry Widget" -value "Standard Widget"]

proc itemsConfig { opt val } {
   foreach el [concat $::items $::checkItems $::radioItems] {
      $el configure $opt $val
   }
}

$main addRow $bar -columnSpan 3 -fill 1
$main addRow $labelCheck -columnSpan 3 -fill 1
$main addRow $labelRadio -columnSpan 3 -fill 1
$main addRow [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$bar configure -visible %v"] -columnSpan 2 -fill 1
$main addRow [gnocl::checkButton -text "sensitive" -active 1 \
      -onToggled "$bar configure -sensitive %v"] -columnSpan 2 -fill 1
$main addRow [list [gnocl::label -text "style"] \
      [gnocl::optionMenu -items {icons text both} -value both \
      -onChanged "$bar configure -style %v"]] -fill 1
$main addRow [list [gnocl::label -text "orientation"] \
      [gnocl::optionMenu -items {horizontal vertical} -value horizontal \
      -onChanged "$bar configure -orientation %v"]] -fill 1
$main addRow [gnocl::label -text "Configure items"] -columnSpan 2 -fill 1
$main addRow [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "itemsConfig -visible %v"] -columnSpan 2 -fill 1
$main addRow [gnocl::checkButton -text "sensitive" -active 1 \
      -onToggled "itemsConfig -sensitive %v"] -columnSpan 2 -fill 1

[lindex $checkItems 0] onToggled
[lindex $radioItems 0] onToggled

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $main -onDestroy exit]
} else {
   set win [gnocl::window -child $main \
         -defaultWidth 200 -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

