#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-check.tcl,v 1.10 2005/02/25 21:57:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set callbackNo 0
set callbackVar ""
proc callback { widget value } {
   incr ::callbackNo
   set ::callbackVar $value
}

set toolBar [gnocl::toolBar]
foreach item {checkButton menuCheckItem toolBarCheck} {
   puts "\nTesting $item"
   switch $item {
      checkButton   { set createTxt "gnocl::checkButton" }
      menuCheckItem { set createTxt "gnocl::menuCheckItem" }
      toolBarCheck  { set createTxt "$toolBar add checkItem" }
   }
   puts -nonewline "Testing variable not set, active not set: "
   catch { unset var }
   set opts [list -text Check -onValue on -offValue off -variable var \
         -onToggled {error "test failed"}]
   set check [eval $createTxt $opts]
   set vget [$check cget -value]
   puts -nonewline [format "off == %s " $vget]
   if { [string compare $vget off] } { puts "\n"; error "test failed" }
   $check delete
   puts "--- Ok"

   puts -nonewline "Testing variable not set, active     set: "
   foreach el {0 1} {
      catch { unset var }
      set opts [list -text Check -onValue on -offValue off \
            -variable var -active $el -onToggled {error "test failed"}]
      set check [eval $createTxt $opts]
      set vget [$check cget -value]
      puts -nonewline [format "%s == %s " $var $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
      $check delete
   }
   puts "--- Ok"

   puts -nonewline "Testing variable     set, active     set: "
   foreach el {0 1} elTxt {off on} {
      set var on
      set opts [list -text Check -onValue on -offValue off \
            -variable var -active $el -onToggled {error "test failed"}]
      set check [eval $createTxt $opts]
      set vget [$check cget -value]
      puts -nonewline [format "%s == %s " $var $vget]
      if { [string compare $var $elTxt] } { error "\n1: $var != $elTxt" }
      if { [string compare $vget $var] } { error "\n2: $vget != $var" }
      $check delete
   }
   puts "--- Ok"

   puts -nonewline "Testing variable     set, active not set: "
   foreach el {off on} {
      set var $el
      set opts [list -text Check -onValue on -offValue off \
            -variable var -onToggled {error "test failed"}]
      set check [eval $createTxt $opts]
      set vget [$check cget -value]
      puts -nonewline [format "%s == %s " $var $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
      $check delete
   }
   puts "--- Ok"

   puts -nonewline "Testing callback on var: "
   set var 0
   set opts [list -text Check -variable var -onToggled {error "test failed"}]
   set check [eval $createTxt $opts]
   foreach el {0 1} {
      set callbackNo 0
      set var $el
      set vget [$check cget -value]
      puts -nonewline [format "%s == %s " $el $vget]
      if { [string compare $el $vget] } { 
            puts "\n"; error "test failed" 
      }
   }
   $check delete
   puts "--- Ok"

   puts -nonewline "Testing onToggled: "
   set opts [list -text Check -onToggled {callback %w %v}]
   set check [eval $createTxt $opts]
   foreach el {0 1} {
      set callbackNo 0
      $check configure -active $el
      $check onToggled
      puts -nonewline [format "%s == %s " $el $callbackVar]
      if { [string compare $el $callbackVar] || $callbackNo != 1 } { 
            puts "\n"; error "test failed" 
      }
   }
   $check delete
   puts "--- Ok"
}
puts "Automatic test done\n\n"

set main [gnocl::label -text "This space intentionally left blank"]
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $main -onDestroy exit]
} else {
   set win [gnocl::window -child $main -onDestroy exit]
}

gnocl::mainLoop

