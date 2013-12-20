#!/bin/sh
# the next line restarts using tclsh \
exec wish "$0" "$@"

# $Id: socket-man.tcl,v 1.7 2004/08/25 19:26:27 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl
wm state . withdrawn    ;# hide tk main window

set box [gnocl::box -orientation vertical]
gnocl::window -title "Socket" -child $box  -onDestroy exit

set menubar [gnocl::menuBar]
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit]
$menubar add [gnocl::menuItem -text "%__File" -submenu $menu]
$box add $menubar -expand 0

set socket [gnocl::socket]
$box add $socket
gnocl::update

# embed a tk button in the gnocl window
toplevel .embed -use [format "0x%x" [$socket getID]]
button .embed.but -text "tk in gnocl" -command {puts "Hello tk!"}
pack .embed.but
# why is this necessary? How to do better?
update; wm minsize .embed [winfo reqwidth .embed] [winfo reqheight .embed]

