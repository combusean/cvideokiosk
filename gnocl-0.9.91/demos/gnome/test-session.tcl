#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
 
# $Id: test-session.tcl,v 1.1 2003/12/22 18:46:38 baum Exp $
 
# ensure that "." is the decimal point
set ::env(LC_NUMERIC) "C"
 
set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclGnome

gnocl::window -child [gnocl::button -onClicked "exit" -text "%#Quit"]

# onDisconnect
foreach el {onConnect onDie onSaveComplete onSaveYourself 
      onShutdownCancelled} {
   gnocl::session configure -$el "puts $el" 
}
gnocl::session configure -onSaveYourself \
      {puts "onSaveYourself phase %p data: %d shutdown: %s fast: %f" }

if 0 {
gnocl::session configure \
   -onDie "exit"
}

