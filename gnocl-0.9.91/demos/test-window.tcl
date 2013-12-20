#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-window.tcl,v 1.13 2005/02/26 23:01:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

proc assert { opt val } {
   set val2 [$::win cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

set win [gnocl::window]

set child [gnocl::label -text "Hello"]
$win configure -child $child
assert -child $child

foreach opt {-defaultWidth -defaultHeight} {
   foreach val {100 200 300 100} {
      $win configure $opt $val
      assert $opt $val
   }
}

foreach opt {-allowGrow -allowShrink -modal -resizable -sensitive} {
   foreach val {0 1 0 1} {
      $win configure $opt $val
      assert $opt $val
   }
}

# FIXME -dragTargets does not work
foreach opt {-data -dropTargets \
      -onDelete -onDestroy -onDragData -onDropData -onKeyPress \
      -onKeyRelease -onPopupMenu -onShowHelp -title -tooltip} {
   foreach val {"aaa" "bbb ccc" "" "" "ccc" "" "ddd" ""} {
      $win configure $opt $val
      assert $opt $val
   }
}

foreach opt { -x -y -width -height } {
   foreach val {100 200 300 100} {
      $win configure $opt $val
      gnocl::update
      after 200
      gnocl::update
      assert $opt $val
   }
}

$win delete

puts "----- automatic tests done ------------"

gnocl::configure -defaultIcon "%/./one.png"
set noUpdates 0

proc onDelete { win } {
   set ret [gnocl::dialog -type question -text "Really close window?" \
         -buttons "%#Cancel %#Close"]
   if { [string compare $ret Cancel] == 0 } {
      return 0
   }
   return 1
}

set box [gnocl::box -orientation vertical]
set testWindow [gnocl::window -child $box -title "Test Window" \
      -onShowHelp {puts "show help %w %h"} \
      -onPopupMenu {puts "%w popup menu"} -tooltip "tooltip" \
      -onKeyPress {puts [list KeyPress: %w %k %K %a]} \
      -onKeyRelease {puts [list KeyRelease: %w %k %K %a]} \
      -onDestroy { puts "destroying %w" } \
      -onDelete { onDelete %w } \
      -icon "%/./c.png" \
      -defaultWidth 300 -defaultHeight 200 ]

$box add [gnocl::checkButton -text "allowShrink"  \
      -onToggled "$testWindow configure -allowShrink %v"]
$box add [gnocl::checkButton -text "allowGrow"  \
      -onToggled "$testWindow configure -allowGrow %v"]
$box add [gnocl::checkButton -text "resizable" -active 1 \
      -onToggled "$testWindow configure -resizable %v"]
$box add [gnocl::checkButton -text "modal" \
      -onToggled "$testWindow configure -modal %v"]

set box [gnocl::box -orientation vertical]
$box add [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$testWindow configure -visible %v"]
$box add [gnocl::checkButton -text "iconify" -active 0 \
      -onToggled "$testWindow iconify %v"]
$box add [gnocl::checkButton -text "sensitive" -active 1 \
      -onToggled "$testWindow configure -sensitive %v"]
$box add [gnocl::box -orientation horizontal -borderWidth 0 -children [list \
      [gnocl::label -text "x" -widthGroup labelGroup] \
      [gnocl::spinButton -lower 0 -upper 400 -stepInc 10 -digits 0 \
            -value 10 \
            -onValueChanged "$testWindow configure -x %v"]]]
$box add [gnocl::box -orientation horizontal -borderWidth 0 -children [list \
      [gnocl::label -text "y" -widthGroup labelGroup] \
      [gnocl::spinButton -lower 0 -upper 400 -stepInc 10 -digits 0 \
            -value 10 \
            -onValueChanged "$testWindow configure -y %v"]]]
$box add [gnocl::box -orientation horizontal -borderWidth 0 -children [list \
      [gnocl::label -text "width" -widthGroup labelGroup] \
      [gnocl::spinButton -lower 10 -upper 400 -stepInc 10 -digits 0 \
            -value 200 \
            -onValueChanged "$testWindow configure -width %v"]]]
$box add [gnocl::box -orientation horizontal -borderWidth 0 -children [list \
      [gnocl::label -text "height" -widthGroup labelGroup] \
      [gnocl::spinButton -lower 10 -upper 400 -stepInc 10 -digits 0 \
            -value 200 \
            -onValueChanged "$testWindow configure -height %v"]]]

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $box -onDestroy exit]
} else {
   set win [gnocl::window -child $box -onDestroy exit]
}

gnocl::mainLoop

