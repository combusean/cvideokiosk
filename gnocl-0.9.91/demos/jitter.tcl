#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: jitter.tcl,v 1.7 2004/08/25 19:26:26 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set done 0
set last0 0
set last1 0
set bgVal 0.1
set exit 0

proc background { } {
   global bgVal
   set bgVal [expr {sin( $bgVal )}]
   after 1 background
}
   
proc callFunc { interval isTcl } {
   set numCalls 10

   set clock$isTcl [clock clicks -milliseconds]

   global last$isTcl values$isTcl done
   if { [set last$isTcl] != 0 } {
      lappend values$isTcl [expr {[set clock$isTcl]-[set last$isTcl]}]
      incr ::no$isTcl
   } else {
      set ::no$isTcl 0
      set values$isTcl {}
   }

   if { [set ::no$isTcl] == $numCalls } {
      set mean 0
      set jitter 0
      foreach val [set values$isTcl] {
         incr mean $val
      }
      set mean [expr {$mean/double($numCalls)}]
      set max 0
      foreach el [set values$isTcl] {
         set val [expr {abs($el-$mean)}]
         if { $val > $max } {
            set max  $val
         }
         set jitter [expr {$jitter+$val}]
      }
      set jitter [expr {$jitter/double($numCalls)}]

      if { $isTcl } {
         set name  Tcl
      } else {
         set name  Gnocl
      }
      puts [format "%6s %3d mean: %5.1f diff: %4.1f jitter: %3.1f max: %4.1f" \
            $name $interval $mean [expr {$mean-$interval}] $jitter $max]
      set last$isTcl 0
      incr done
      if { $isTcl == 0 } {
         return -code break
      }
   } else {
      set last$isTcl [clock clicks -milliseconds]
      if { $isTcl } {
         after $interval callFunc $interval 1
      }
   }
}

proc doIt { } {
   # foreach el {0 1 5 9 10 11 20 25 50 100} 
   foreach el {11 21} {
      callFunc $el 1
      if { [catch {package present Gnocl}] == 0 } {
         gnocl::callback create "callFunc $el 0" -interval $el
         vwait ::done
      }
      vwait ::done
   }

   if { $::exit } {
      exit
   }
}

# background

puts "pure Tcl"
doIt

puts "\npure Tcl with gnocl loaded"
set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl
set id [gnocl::callback create {puts -nonewline ""} -interval idle ]
after 100 "gnocl::callback delete $id"

doIt


#gnocl::callback create {puts -nonewline "A"; flush stdout} -interval 200
#gnocl::callback create {puts -nonewline "B"; flush stdout} -interval 300

puts "\ngnocl::mainLoop"
after 500 doIt
set exit 1
gnocl::mainLoop -timeout 10



