#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-clipboard.tcl,v 1.11 2005/02/25 22:14:40 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

foreach el {0 1} {
   if { $el } {
      puts "Testing PRIMARY"
   } else {
      puts "Testing standard clipboard"
   }
   gnocl::clipboard clear -primary $el
   if { [gnocl::clipboard hasText -primary $el] } {
      puts "Error: clipboard should be empty"
   }

   set txt "abcdäöüß$el"
   gnocl::clipboard setText $txt -primary $el
   set ret [gnocl::clipboard getText -primary $el]
   if { [string length $ret] != [string length $txt] \
         || [string compare $ret $txt] != 0 } {
      error "$ret != $txt"
   }
   puts [format "length: %d == %d; %s == %s" [string length $txt] \
         [string length $ret] $txt $ret]

   gnocl::clipboard clear -primary $el
   if { [gnocl::clipboard hasText -primary $el] } {
      error "clipboard should be empty"
   }
}

set main [gnocl::label -text "This space intentionally left blank"]
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $main -onDestroy exit]
} else {
   set win [gnocl::window -child $main -onDestroy exit]
}

gnocl::mainLoop


