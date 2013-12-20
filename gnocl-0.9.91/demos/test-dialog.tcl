#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-dialog.tcl,v 1.6 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical]
set infoLabel [gnocl::label]

proc simple { opt } {
   set val "Dialog with default icon (with -child no icon is displayed)\nand default button."
   if { [string compare $opt -child] == 0 } {
      set val [gnocl::label -text $val]
   }
   $::infoLabel configure -text ""
   gnocl::dialog $opt $val
}
proc question { opt } {
   set val "Dialog with question icon and default button."
   if { [string compare $opt -child] == 0 } {
      set val [gnocl::label -text $val]
   }
   $::infoLabel configure -text "" 
   gnocl::dialog $opt $val -type question
}


proc info { opt } {
   set val "Info dialog with text, %#Ok button and special window icon." 
   if { [string compare $opt -child] == 0 } {
      set val [gnocl::label -text $val]
   }
   $::infoLabel configure -text ""
   set ret [gnocl::dialog -type info \
         -buttons "{Pure text} {{Ok plus Icon} %#Ok}" \
         $opt $val -icon "%/./c.png" ] 
   $::infoLabel configure -text "return value was \"$ret\""
}

proc onResponse { label val } {
   $label configure -text "return value was \"$val\""
   if { "Ok" == $val } {
      return -code break
   }
}

proc warning { opt } {
   set val "Non modal Warning dialog with special button\nand %#Ok."
   if { [string compare $opt -child] == 0 } {
      set val [gnocl::label -text $val]
   }
   $::infoLabel configure -text ""
   set but [gnocl::button -text "%__Don't exit"]
   set ret [gnocl::dialog -modal 0 $opt $val \
         -type warning -buttons [list "$but DontExit" %#Ok] \
         -onResponse "onResponse $::infoLabel %v" ]
}

proc error { opt } {
   set val "Warning dialog with #Cancel and %#Ok.\nOk is default"
   if { [string compare $opt -child] == 0 } {
      set val [gnocl::label -text $val]
   }
   $::infoLabel configure -text ""
   gnocl::dialog -type error -buttons "%#Cancel %#Ok" \
         $opt $val -defaultButton 1 -onResponse \
         "$::infoLabel configure -text {return value was \"%v\"}; break" \
}

foreach el {-text -child} {
   set box [gnocl::box -orientation vertical -label $el]
   $box add [gnocl::button -text "Simple Dialog" -onClicked "simple $el"]
   $box add [gnocl::button -text "Question" -onClicked "question $el"]
   $box add [gnocl::button -text "Info" -onClicked "info $el"]
   $box add [gnocl::button -text "Warning" -onClicked "warning $el"]
   $box add [gnocl::button -text "Error" -onClicked "error $el"]
   $mainBox add $box
}
$mainBox add $infoLabel



foreach el {0 1 2} {
   set but [gnocl::button -text "%#Save"]
   set ret [gnocl::dialog -type info -buttons [list qqq %#Ok "$but Save"] \
         -defaultButton $el -text "Default Button is $el"]
   puts "return value: $ret"
}

foreach el {0 1 2} {
   set but [gnocl::button -text "%#Save"]
   set ret [gnocl::dialog -type info -buttons [list qqq %#Ok "$but Save"] \
         -defaultButton $el -child [gnocl::label -text "Default Button is $el"]]
   puts "return value: $ret"
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

# simple
# info
# warning
# error
gnocl::mainLoop

