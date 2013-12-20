#!/bin/sh
# the next line restarts using tclsh \
exec wish "$0" "$@"

# $Id: embed-tk.tcl,v 1.8 2004/08/12 08:17:28 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

# embed gtk in tk
puts "embed gnocl into tk: still does not work"
frame .frame -container 1
pack .frame
scan [winfo id .frame] %x xid
toplevel .embedTk -use [format "0x%x" $xid]
button .embedTk.but -text "tk in tk" -command {puts "Hello tk!"}
pack .embedTk.but
# this update is essential!
update
gnocl::plug -socketID $xid -onDestroy {puts "plug destroyed"} \
      -defaultHeight 20 -defaultWidth 100 -child \
      [gnocl::button -text "gnocl in tk" -onClicked {puts "Hello gnocl!"}]

# embed tk in gtk window
# gtk top level window
puts "\n\nembed into gnocl"
set menubar [gnocl::menuBar]
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit]
$menubar add [gnocl::menuItem -text "%__File" -submenu $menu]

set cont [gnocl::box -orientation vertical]
gnocl::window -title "Test Application" -child $cont 
set socketGtk [gnocl::socket \
      -onPlugAdded {puts [format "gtk: plug in 0x%%x" [%w getPlugID]]} \
      -onPlugRemoved {puts "gtk: plug out"}]
set socketTk [gnocl::socket \
      -onPlugAdded {puts [format "tk:  plug in 0x%%x" [%w getPlugID]]} \
      -onPlugRemoved {puts "tk: plug out"}]
$cont add $menubar -expand 0
$cont add $socketGtk
$cont add $socketTk
puts [format "updates: %d" [gnocl::update]]

# add gnocl in gnocl
set but [gnocl::button -text "gnocl in gnocl" -onClicked {puts "Hello gnocl!"}]
# these two should be equivalent
if { 0 } {
   set plug [gnocl::plug -socketID [$socketGtk getID] -child $but]
} else {
   set plug [gnocl::plug -child $but]
   $socketGtk configure -plugID [$plug getID]
}
set xid [format "0x%x" [$socketTk getID]]
# add tk in gnocl
toplevel .embed -use $xid
# $socket addID [format %d [winfo id .embed]]
button .embed.but -text "tk in gnocl" -command {puts "Hello tk!"}
pack .embed.but
set t [clock clicks -milliseconds]
# why does this update take approx 2 sec with tk 8.3.3?
# has this something to do with this change in 8.4a4:
# "corrected 2 second 'raise' delay on some Unix window managers"?
puts [format "updates: %d" [gnocl::update]]
set t2 [clock clicks -milliseconds]
puts [format "update took %d ms" [expr {$t2-$t}]]
# why is this necessary? How to do better?
wm minsize .embed [winfo reqwidth .embed] [winfo reqheight .embed]

