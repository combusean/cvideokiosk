#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-menu2.tcl,v 1.10 2005/02/26 23:01:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set menuBar [gnocl::menuBar]

set menu [gnocl::menu]
set check1Win [gnocl::menuCheckItem -text "%__Check" \
      -accelerator "<Alt>h" \
      -variable check1 -onToggled "check %w %v ::check1"]
set onOffWin [gnocl::menuCheckItem -text "%__On Off" \
      -variable onOff -onValue on -offValue off \
      -accelerator "<control>M" -onToggled "check %w %v ::onOff"]
$menu add $check1Win
$menu add $onOffWin
$menu add [gnocl::menuSeparator]
$menu add [gnocl::menuItem -text "%_C_heck on" -tooltip "check on"\
      -onClicked "$check1Win configure -value 1" -accelerator "<control>h"]
$menu add [gnocl::menuItem -text "%_Ch_eck off" -tooltip "check off"\
      -onClicked "$check1Win configure -value 0" \
      -accelerator "<shift><control>h"]
$menu add [gnocl::menuItem -text "%_O_nOff on" -tooltip "onOff on"\
      -onClicked "$onOffWin configure -value on"]
$menu add [gnocl::menuItem -text "%_OnO_ff off" -tooltip "onOff off"\
      -onClicked "$onOffWin configure -value off"]

$menuBar add [gnocl::menuItem -text "%__First" -submenu $menu]

set subMenu [gnocl::menu]
foreach el {1 2 3} {
   $subMenu add [gnocl::menuItem -text "%_Label _$el" \
         -tooltip "label $el" -onClicked "puts label$el"]
}

set popup [gnocl::menu -tearoff 0]
foreach el {1 2 3} {
   $popup add [gnocl::menuItem -text "%_Pop _$el" -tooltip "pop $el" \
         -onClicked "puts pop$el"]
}

set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%_S_ubmenu" -submenu $subMenu]
$menu add [gnocl::menuCheckItem -text "%__Check2" \
      -onToggled "puts check2"]
$menu add [gnocl::menuItem -text "%__Popup" -accelerator "<control>P" \
      -onClicked "$popup popup"]
$menuBar add [gnocl::menuItem -text "%__Second" -submenu $menu]

set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%_With _png" \
      -icon "%/./one.png" -onClicked {puts "with png"}]
$menu add [gnocl::menuItem -text "%_With _jpg" \
      -icon "%/./two.jpg" -onClicked {puts "with jpg"}]
$menu add [gnocl::menuItem -text "%__No icon" -icon "" \
      -onClicked {puts "no icon"}]
$menu add [gnocl::menuItem -text "%__Stock icon" -icon "%#Save" \
      -onClicked {puts "stock icon"}]
$menuBar add [gnocl::menuItem -text "%__Third" -submenu $menu]

proc check { win val var } {
   puts -nonewline "value is now \"$val\" == \""
   puts -nonewline [$win cget -value]
   if { [string length $var] > 0 } {
      puts -nonewline "\" == \""
      puts -nonewline [set $var]
   }
   puts "\""
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $menuBar \
         -onDestroy exit]
} else {
   # gnocl::app MenuTest -title "Menu Tests" -menuBar $menuBar \
   #       -statusBar [gnocl::appBar]
   gnocl::window -title "Menu Tests" -child $menuBar -onDestroy exit
}

gnocl::mainLoop
