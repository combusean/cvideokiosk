#!/bin/sh
# the next line restarts using tclsh \
exec wish "$0" "$@"

# $Id: embed.tcl,v 1.6 2004/08/12 08:17:28 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

puts "\n\nThis does not work with GTK+ 2.0 any more."
puts "Any explanation why, anyone?\n"

# embed gtk in tk
# FIXME: why does this not work?
button .but -text "Hello tk" -command {puts "Hello tk!"}
frame .frame -container 1
pack .but .frame
# this update is essential!
update
gnocl::plug -socketID [format %d [winfo id .frame]] -child \
      [gnocl::button -text "Hello gnocl" -onClicked {puts "Hello gnocl!"}]

# embed tk in gtk window
# gtk top level window
set menubar [gnocl::menuBar]
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit]
$menubar add [gnocl::menuItem -text "%__File" -submenu $menu]

set cont [gnocl::box -orientation vertical -children $menubar]
gnocl::window -title "Test Application" -child $cont
set socket [gnocl::socket]
$cont add [gnocl::button -text "Hello gnocl" -onClicked {puts "Hello gnocl!"}]
$cont add $socket

gnocl::update
toplevel .embed -use [format "0x%x" [$socket getID]]
button .embed.but -text "Hello tk" -command {puts "Hello tk!"}
pack .embed.but

