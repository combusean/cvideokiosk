#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: mini.tcl,v 1.9 2004/08/25 19:26:27 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

# this example shows a minimum application with menubar,
# statusbar, toolBar, and main window

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

# create submenu for menu "File" with standard items "New" and 
# "Quit" (text, icon and accelerator). 'N', 'Q' respectively, 
# are underlined and are used as mnemonics.
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#New" -tooltip "Make new" \
      -onClicked {puts "That's new"}]
$menu add [gnocl::menuSeparator]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit \
      -tooltip "Quit program"]

# create menu "File", 'F' is underlined and used as mnemonic
set file [gnocl::menuItem -text "%__File" -submenu $menu]

# create menu "Help" with item "About" 
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%__About" \
      -accelerator "<Ctrl><Shift>a" \
      -tooltip "Show about dialog" \
      -onClicked {puts "Mini example (c) 2001 P.G. Baum"}]
set help [gnocl::menuItem -text "%__Help" -submenu $menu]

# create toolbar with standard items "Quit" and "New"
set toolBar [gnocl::toolBar -style both]
$toolBar add item -text "%#Quit" -tooltip "Tooltip Quit" \
      -onClicked exit
$toolBar add space
$toolBar add item -text "%#New" -tooltip "Tooltip new" \
      -onClicked {puts "That's new"}

# create GTK+ application with menu, toolBar, statusbar 
# and a label as main widget
set box [gnocl::box -orientation vertical -borderWidth 0 -spacing 0]
set win [gnocl::window -child $box -title "Test Application" -onDestroy exit]
$box add [gnocl::menuBar -children [list $file $help]]
$box add $toolBar 
$box add [gnocl::label -text \
      {%<<span foreground="blue" size="large">Hello</span>\
      <span foreground="red" size="large">World</span>}] -expand 1
$box add [gnocl::statusBar] 

# this is the gnome version
###gnocl::app testApp -title "Test Application" -width 250 \
###      -menubar [gnocl::menuBar -children [list $file $help]] \
###      -statusbar [gnocl::appBar] -toolbar $toolBar \
###      -contents [gnocl::label -text "here comes the main widget"]

gnocl::mainLoop

