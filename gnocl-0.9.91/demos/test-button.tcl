#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-button.tcl,v 1.14 2005/02/25 21:51:53 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set button [gnocl::button]

proc assert { opt val } {
   set val2 [$::button cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "%<<b>Pango</b>"
      "%#Quit" } {
   $button configure -text $el
   assert -text $el
}

foreach val { "%#Save" "" "%/./one.png" } {
   $button configure -icon $val
   if { [catch {assert -icon $val} erg] } {
      puts $erg
   }
}

foreach val { "normal" "half" "none" "normal" } {
   $button configure -relief $val
   assert -relief $val
}

foreach opt {-text -icon } {
   foreach el [lsort [gnocl::info allStockItems]] {
      if { [catch {$button configure $opt "%#$el"} erg] } {
         puts $erg
      } else {
         assert $opt "%#$el"
      }
   }
}

foreach opt {-name -data -tooltip -widthGroup -heightGroup -sizeGroup} {
   foreach val {"qqq" "bbb" "" "" "ddd" "" } {
      $button configure $opt $val
      assert $opt $val
   }
}

foreach opt {-visible -sensitive} {
   foreach val {0 1 0 1} {
      $button configure $opt $val
      assert $opt $val
   }
}


foreach opt {-onClicked -onShowHelp -onPopupMenu -onButtonPress \
      -onButtonRelease } {
   foreach val {"puts hallo" "" "puts qqq"} {
      $button configure $opt $val
      assert $opt $val
   }
}
$button configure -onShowHelp "puts hallo"
assert -onShowHelp "puts hallo"
assert -onClicked "puts qqq"

foreach opt {-normalBackgroundColor -activeBackgroundColor \
      -prelightBackgroundColor} {
   foreach val {"65535 0 0" "0 65535 0" "0 0 65535"} {
      $button configure $opt $val
      assert $opt $val
   }
}

set but2 [gnocl::button]
set box [gnocl::box -children [list $button $but2]]
set win [gnocl::window -child $box]

foreach el {1 2} {
   $but2 configure -hasFocus 1
   after 200
   puts [gnocl::update]
   assert -hasFocus 0

   $button configure -hasFocus 1
   after 200
   puts [gnocl::update]
   assert -hasFocus 1
}

$win delete

puts "----- automatic tests done ------------"


set left [gnocl::box -orientation vertical]
# set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
# $mainBox addEnd $right

set relief normal
set txtNo 0
set commandList { "puts Hallo" "" {puts "Hallo 2"}}
set command ""

$left add [gnocl::button -onClicked {puts "markup test"} -text \
      "%<normal <b>_bold</b> <big>big</big> <small>small</small>" \
      -onButtonRelease {puts "button release %w %b"} \
      -onButtonPress {puts "button pressed %w %b"}]
$left add [gnocl::button -text "relief half" -relief $relief \
      -activeBackgroundColor red -normalBackgroundColor blue \
      -prelightBackgroundColor green -onClicked {configRelief %w}]

set but [gnocl::button -text "%__Command changed" \
      -onClicked "puts {Hello World!}"]
$left add [gnocl::button -text "Command change" \
      -onClicked "configCommand $but"]
$left add $but
$left add [gnocl::button -text "" -onClicked {configText %w}]
$left add [gnocl::button -text "%_Un_derline" \
      -onClicked {puts "%w underline"} -onShowHelp {puts "%w showHelp %h"} \
      -onPopupMenu {puts "%w popup menu"} -tooltip "This is a tooltip"]

$left add [gnocl::button -text "%_H_elp" -icon "%#Save"]

$but onClicked

proc configCommand { widg } {
   set ll [lsearch -exact $::commandList $::command]
   incr ll
   if { $ll >= [llength $::commandList] } {
      set ll 0
   }
   set ::command [lindex $::commandList $ll]
   $widg configure -onClicked $::command
}
proc configRelief { widg } {
   switch $::relief {
   normal { set ::relief half }
   half   { set ::relief none }
   none   { set ::relief normal }
   }
   $widg configure -relief $::relief -text "relief $::relief"
}

proc configText { widg } {
   switch $::txtNo {
      0 { $widg configure -text %#Yes }
      1 { $widg configure -text %_ec_ho }
      2 { $widg configure -text %#No }
      3 { $widg configure -text "%_h_allo Icon" -icon "%#Save" }
      4 { $widg configure -icon "" }
      5 { $widg configure -text "%_hal_lo Icon" -icon "%#Save" }
      6 { $widg configure -icon "%#SaveAs" }
      7 { $widg configure -text "%__File" -icon "%/./one.png" }
   }
   incr ::txtNo
   if { $::txtNo == 8 } {
      set ::txtNo 1
   }
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

