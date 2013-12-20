#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-table.tcl,v 1.10 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set left [gnocl::box -orientation horizontal -shadow in]
set right [gnocl::box -orientation vertical -homogeneous 0 -fill 0]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -homogeneous 0 -expand 1 -fill 1]
$mainBox add $right -expand 0

set expand 1
set xFill 1
set yFill 1
set shrink 1
set padding 0

proc populate { widget } {
   set child [list [gnocl::button -text "1\n11\n111"] "" \
         [gnocl::button -text "2"]]
   $widget addRow $child -expand $::expand \
         -fill [list $::xFill $::yFill] -shrink $::shrink -padding $::padding

   set w [list [gnocl::button -text "3"] "" [gnocl::button -text "4"]]
   eval lappend child $w
   $widget addColumn $w -expand $::expand \
         -fill [list $::xFill $::yFill] -shrink $::shrink -padding $::padding

   set w [gnocl::button -text "rowSpan 3"] 
   lappend child $w
   $widget add $w 1 0 -rowSpan 3 -expand $::expand \
         -fill [list $::xFill $::yFill] -shrink $::shrink -padding $::padding

   set w [gnocl::button -text "columnSpan 2"] 
   lappend child $w
   $widget add $w 2 1 -columnSpan 2 -expand $::expand \
         -fill [list $::xFill $::yFill] -shrink $::shrink -padding $::padding

   set w [gnocl::button -text "columnSpan 4"] 
   lappend child $w
   $widget addRow $w -columnSpan 4 -expand $::expand \
         -fill [list $::xFill $::yFill] -shrink $::shrink -padding $::padding

   return $child
}

proc packNew { } {
   foreach el $::child {
      if { [string length $el] } {
         $el delete
      }
   }
   foreach el $::fChild {
      if { [string length $el] } {
         $el delete
      }
   }
   set ::child [populate $::tab]
   set ::fChild [populate $::fTab]
   # gnocl::update
}


set tab [gnocl::table \
      -onShowHelp {puts "normal box without frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]
set fTab [gnocl::table -label "Table with frame" \
      -onShowHelp {puts "normal box without frame %w %h"} \
      -onPopupMenu {puts "%w popupMenu"}]

$left add [list $tab $fTab] -fill 1 -expand 1

set child [populate $tab]
set fChild [populate $fTab]

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "config -sensitive %v"] -expand 0
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "config -visible %v"] -expand 0
$right add [gnocl::checkButton -text "%__Homogeneous" -active 0 \
      -onToggled "config -homogeneous %v"] -expand 0
$right add [gnocl::label -text "Frame shadow:"] -expand 0
set opt [gnocl::optionMenu -onChanged "configFrame -shadow %v" \
      -items {none in out etchedIn etchedOut}]
$right add $opt -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "borderWidth:" -align left -widthGroup labelWidth]  \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -borderWidth %v"]]] -expand 0
$right add [gnocl::optionMenu -items "small normal big" -value normal\
      -onChanged "config -borderWidth %v"]
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "rowSpacing:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -rowSpacing %v"]]] -expand 0
$right add [gnocl::optionMenu -items "small normal big" -value normal\
      -onChanged "config -rowSpacing %v"]
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "columnSpacing:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -columnSpacing %v"]]] -expand 0
$right add [gnocl::optionMenu -items "small normal big" -value normal\
      -onChanged "config -columnSpacing %v"]
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "labelAlign:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -onValueChanged "configFrame -labelAlign %v" \
            -lower 0 -upper 1 -stepInc 0.1]]] -expand 0
$right add [gnocl::label -text "pack Options:"] -expand 0
$right add [gnocl::checkButton -text "expand" -active 1 \
      -variable expand -onToggled packNew] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "fill x:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -onValueChanged packNew -variable xFill \
               -lower 0 -upper 1 -digits 1 -stepInc 0.1]]]
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "fill y:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -onValueChanged packNew -variable yFill \
               -lower 0 -upper 1 -digits 1 -stepInc 0.1]]]
$right add [gnocl::checkButton -text "shrink" -active 1 \
      -variable shrink -onToggled packNew] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "padding:" -align left -widthGroup labelWidth] \
      [gnocl::spinButton -onValueChanged packNew -variable padding \
            -lower 0 -upper 30 -digits 0]]] -expand 0

proc config { option value } {
   $::tab configure $option $value
   $::fTab configure $option $value
}
proc configFrame { option value } {
   $::fTab configure $option $value
   # for tables without frame this must give an error (but no segfault!)
   catch {$tab configure $option $value} erg
   #puts "Ok: $erg"
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -defaultHeight 500 -defaultWidth 500 -onDestroy exit]
}

gnocl::mainLoop

