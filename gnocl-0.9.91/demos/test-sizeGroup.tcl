#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-sizeGroup.tcl,v 1.3 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set left [gnocl::box -orientation vertical -shadow in]
set right [gnocl::box -orientation vertical -homogeneous 0]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -expand 1 -fill 1 -homogeneous 0]
$mainBox add $right -fill 0 -expand 0

set box [gnocl::box -orientation horizontal -label "WidthGroup"]
set butH1 [gnocl::button -text "Short"]
set butH2 [gnocl::button -text "Long Description"]
$box add [list $butH1 $butH2]
$left add $box

set box [gnocl::box -orientation vertical -label "HeightGroup"]
set butV1 [gnocl::button -text "Short"]
set butV2 [gnocl::button -text "Double line\ndescription"]
$box add [list $butV1 $butV2]
$left add $box

set box [gnocl::table -label "SizeGroup"]
set butS1 [gnocl::button -text "Short"]
set butS2 [gnocl::button -text "Long and double line\ndescription"]
$box addRow [list $butS1 ""]
$box addRow [list "" $butS2]
$left add $box

$right add [gnocl::checkButton -text "Use width group" -active 0 \
      -onToggled "setWidthGroup %v"] -expand 0
$right add [gnocl::checkButton -text "Use height group" -active 0 \
      -onToggled "setHeightGroup %v"] -expand 0
$right add [gnocl::checkButton -text "Use size group" -active 0 \
      -onToggled "setSizeGroup %v"] -expand 0

proc setWidthGroup { on } {
   if { $on } {
      set group width
   } else {
      set group ""
   }
   
   foreach el {butH1 butH2} {
      global $el
      [set $el] configure -widthGroup $group
   }
}

proc setHeightGroup { on } {
   if { $on } {
      set group height
   } else {
      set group ""
   }
   
   foreach el {butV1 butV2} {
      global $el
      [set $el] configure -heightGroup $group
   }
}

proc setSizeGroup { on } {
   if { $on } {
      set group size
   } else {
      set group ""
   }
   
   foreach el {butS1 butS2} {
      global $el
      [set $el] configure -sizeGroup $group
   }
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit \
         -defaultHeight 500 -defaultWidth 500]
}

gnocl::mainLoop

