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

# set action [gnocl::action -text "%#Copy"]
set menu [gnocl::menu]

set action [gnocl::action -text "qqq" -accelerator "<control>i" -onActivate "puts activated"]
$menu add [$action createMenuItem]
#set action [gnocl::action -text "%#Quit" -onActivate "puts stock"]
#$menu add [$action createMenuItem]

$menuBar add [gnocl::menuItem -text "%__Action Menu" -submenu $menu]

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $menuBar \
         -onDestroy exit]
} else {
   # gnocl::app MenuTest -title "Menu Tests" -menuBar $menuBar \
   #       -statusBar [gnocl::appBar]
   gnocl::window -title "Menu Tests" -child $menuBar -onDestroy exit
}

gnocl::mainLoop
