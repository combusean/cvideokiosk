#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-box.tcl,v 1.10 2004/08/12 08:17:29 baum Exp $

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

set hButBox [gnocl::box -orientation horizontal -buttonType 1 \
      -label "button type with frame" \
      -tooltip "button type with frame" \
      -onShowHelp {puts "button type with frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]
set vButBox [gnocl::box -orientation vertical -buttonType 1 \
      -onShowHelp {puts "button type without frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]
set hBox [gnocl::box -orientation horizontal \
      -onShowHelp {puts "normal box without frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]
set vBox [gnocl::box -orientation vertical -label "normal box with frame" \
      -onShowHelp {puts "normal box with frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]

foreach el [list $hButBox $vButBox $hBox $vBox] {
   $el add [gnocl::button -text "%#Ok"]
   $el add [gnocl::button -text "%#Cancel"]
   $el add [gnocl::button -text "%#Help"] -padding 30
}

set vert [gnocl::box -orientation horizontal -children "$vBox $vButBox" \
      -expand 1 -fill 1]
$left add [list $hButBox $vert $hBox] -expand 1 -fill 1

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "config -sensitive %v"] -expand 0
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "config -visible %v"] -expand 0
$right add [gnocl::label -text "Frame shadow:"] -expand 0
set opt [gnocl::optionMenu -onChanged "configFrame -shadow %v"]
foreach el {none in out etchedIn etchedOut} {
   $opt add $el -value $el
}
$right add $opt -expand 0
$right add [gnocl::label -text "Button layout:"] -expand 0
set opt [gnocl::optionMenu -onChanged "configBox -layout %v"]
foreach el {default spread edge start end} {
   $opt add $el -value $el
}
$right add $opt -expand 0
$right add [gnocl::label -text "borderWidth:"] -expand 0
$right add [gnocl::spinButton -onValueChanged "config -borderWidth %v" \
      -digits 0 -upper 30] -expand 0
$right add [gnocl::optionMenu -items "small normal big" -value normal\
      -onChanged "config -borderWidth %v"]
$right add [gnocl::label -text "spacing:"] -expand 0
$right add [gnocl::spinButton -onValueChanged "config -spacing %v" \
      -digits 0 -upper 30] -expand 0
$right add [gnocl::optionMenu -items "small normal big" -value normal\
      -onChanged "config -spacing %v"]
$right add [gnocl::label -text "labelAlign:"] -expand 0
$right add [gnocl::spinButton -onValueChanged "configFrame -labelAlign %v" \
      -lower 0 -upper 1 -stepInc 0.1] -expand 0

proc config { option value } {
   foreach el [list $::hButBox $::vButBox $::hBox $::vBox] {
      $el configure $option $value
   }
}
proc configFrame { option value } {
   foreach el [list $::hButBox $::vBox] {
      $el configure $option $value
   }
   # for boxes without frame this must give an error (but no segfault!)
   foreach el [list $::vButBox $::hBox] {
      catch {$el configure $option $value} erg
      #puts "Ok: $erg"
   }
}
proc configBox { option value } {
   foreach el [list $::vButBox $::hButBox] {
      $el configure $option $value
   }
   # for no button types this must give an error (but no segfault!)
   foreach el [list $::hBox $::vBox] {
      catch {$el configure $option $value} erg
      #puts "Ok: $erg"
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

