#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-statusBar.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

if { $argc != 1 } {
   puts "usage: $argv0 socket-XID"
}

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set main [gnocl::table -homogeneous 0 -columns 2 -borderWidth 0]
set bar [gnocl::statusBar]
$bar add [gnocl::button -text "add"]
$bar addBegin [gnocl::button -text "addBegin"]
$bar addEnd [gnocl::button -text "addEnd"]

$main addRow [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$bar configure -visible %v"] -columnSpan 2 -fill 1
$main addRow [gnocl::checkButton -text "resizeGrip" -active 1 \
      -onToggled "$bar configure -resizeGrip %v"] -columnSpan 2 -fill 1
$main addRow [gnocl::checkButton -text "homogeneous" -active 0 \
      -onToggled "$bar configure -homogeneous %v"] -columnSpan 2 -fill 1
$main addRow [list [gnocl::label -text "spacing"] \
      [gnocl::optionMenu -items "small normal big" -value normal \
            -onChanged "$bar configure -spacing %v"]] -fill 1

proc push { } {
   if { $::useContext } {
      set ret [$::bar push $::pushText -context $::context]
   } else {
      set ret [$::bar push $::pushText]
   }
   $::idLabel configure -text "Message ID was $ret"
}

proc pop { } {
   if { $::useContext } {
      $::bar pop -context $::context
   } else {
      $::bar pop 
   }
}
set contEntry [gnocl::entry -variable context -sensitive 0]
$main addRow [list $contEntry \
      [gnocl::checkButton -text "Use context ID" -variable useContext \
      -onToggled "$contEntry configure -sensitive %v"]] -fill 1

$main addRow [list [gnocl::entry -variable pushText] \
      [gnocl::button -text "push" -onClicked push]] -fill 1
set idLabel [gnocl::label]
$main addRow $idLabel -fill 1
$main addRow [gnocl::button -text "pop" -onClicked pop] -columnSpan 2 -fill 1



$main addRow $bar -columnSpan 3 -fill 1
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $main -onDestroy exit]
} else {
   set win [gnocl::window -child $main \
         -defaultWidth 200 -defaultHeight 200 -onDestroy exit]
}

set no1 [$bar push "aaa"]
set no2 [$bar push "bbb" -context 1]
$bar push "ccc"
$bar push "ddd"
$bar push "eee" -context 1
$bar push "fff"
$bar push "ggg"

set cmdNo 0
set cmdList [list \
   "$bar pop" "fff is visible" \
   "$bar pop" "eee is visible" \
   "$bar pop" "eee is visible" \
   "$bar pop -context 1" "ccc is visible" \
   "$bar remove $no2 -context 1" "ccc is visible" \
   "$bar pop" "aaa is visible" \
   "$bar remove $no1" "empty"]

proc nextCmd { } {
   global cmdList cmdNo label win
   set cmd [lindex $cmdList [expr {$cmdNo*2}]]
   set txt [lindex $cmdList [expr {$cmdNo*2+1}]]

   eval $cmd
   $label configure -text $txt
   incr cmdNo
   if { 2*$cmdNo == [llength $cmdList] } {
      $win delete
   }
}

set label [gnocl::label]
set win [gnocl::window -child [gnocl::box -orientation horizontal \
      -children [list [gnocl::button -text "%#GoForward" -onClicked nextCmd] \
         $label]]]

gnocl::mainLoop

