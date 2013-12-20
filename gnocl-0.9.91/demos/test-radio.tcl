#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-radio.tcl,v 1.11 2005/02/25 21:57:32 baum Exp $

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
foreach item {radioButton menuRadioItem toolBarRadio} {
   puts "\nTesting $item"
   switch $item {
      radioButton   { set createTxt "gnocl::radioButton" }
      radioButton   { set createTxt "gnocl::menuRadioItem" }
      toolBarRadio  { set createTxt "$toolBar add radioItem" }
   }

   puts -nonewline "Testing variable not set, active not set: "
   catch { unset var }
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   if { [string compare $var 1] } { puts "\n"; error "test failed" }
   puts -nonewline " $var"
   foreach el {1 2 3} {
      set vget [[set radio$el] cget -value]
      puts -nonewline [format " == %s" $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts " --- Ok"

   puts -nonewline "Testing variable not set, active     set: "
   catch { unset var }
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -active [expr {$el==2}] -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   if { [string compare $var 2] } { puts "\n"; error "test failed" }
   puts -nonewline " $var"
   foreach el {1 2 3} {
      set vget [[set radio$el] cget -value]
      puts -nonewline [format " == %s" $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts " --- Ok"

   puts -nonewline "Testing variable     set, active     set: "
   set var 3
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -active [expr {$el==2}] -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   if { [string compare $var 2] } { puts "\n"; error "test failed" }
   puts -nonewline " $var"
   foreach el {1 2 3} {
      set vget [[set radio$el] cget -value]
      puts -nonewline [format " == %s" $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts " --- Ok"

   puts -nonewline "Testing variable     set, active not set: "
   set var 2
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   if { [string compare $var 2] } { puts "\n"; error "test failed" }
   puts -nonewline " $var"
   foreach el {1 2 3} {
      set vget [[set radio$el] cget -value]
      puts -nonewline [format " == %s" $vget]
      if { [string compare $vget $var] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts " --- Ok"

   puts -nonewline "Testing callback on var: "
   set var 1
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   foreach el {2 3 1} {
      set var $el
      set vget [$radio3 cget -value]
      puts -nonewline [format "%s == %s " $el $vget]
      if { [string compare $el $vget] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts "--- Ok"

   puts -nonewline "Testing configure -value:        "
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -onToggled {error "test failed"}]
      set radio$el [eval $createTxt $opts]
   }
   foreach el {2 3 1} {
      $radio3 configure -value $el
      set vget [$radio1 cget -value]
      puts -nonewline [format "%s == %s " $el $vget]
      if { [string compare $el $vget] } { puts "\n"; error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts "--- Ok"

   puts -nonewline "Testing onToggled: "
   proc callback {val1 val2 widget} {
      set vget [$widget cget -value]
      puts -nonewline [format "%s == %s == %s " $val1 $val2 $vget]
      if { [string compare $val1 $val2] || [string compare $val1 $vget] } { 
         puts "\n"; error "test failed" 
      }
      incr ::noCalled
   }
   set var 1
   foreach el {1 2 3} {
      set opts [list -text "Radio $el" -onValue $el -variable var \
            -onToggled "callback $el %v %w"]
      set radio$el [eval $createTxt $opts]
   }
   foreach el {2 3 1} {
      set noCalled 0
      [set radio$el] configure -active 1
      if { $noCalled != 0 } { error "test failed" }
      $radio3 onToggled
      if { $noCalled != 1 } { error "test failed" }
   }
   foreach el {1 2 3} { [set radio$el] delete }
   puts "--- Ok"
   # set mainBox [gnocl::box -orientation vertical \
   #       -children [list $radio1 $radio2 $radio3]]
   # set win [gnocl::window -child $mainBox -onDestroy exit]
   # gnocl::mainLoop
}
puts "Automatic test done\n\n"

set main [gnocl::label -text "This space intentionally left blank"]
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $main -onDestroy exit]
} else {
   set win [gnocl::window -child $main -onDestroy exit]
}

gnocl::mainLoop



