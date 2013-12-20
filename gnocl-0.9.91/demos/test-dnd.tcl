#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-dnd.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set targetTxt [gnocl::label]
set targetCol [gnocl::eventBox -child $targetTxt]
set targetBox [gnocl::box -label "Drag'n Drop Target" -orientation vertical \
      -dropTargets {STRING text/plain text/uri-list application/x-color} \
      -onDropData {dropFunc %T %t %l %d}]
$targetBox add $targetCol
set colButton [gnocl::button -text "Start ColorSelector" \
      -onClicked \
      {gnocl::colorSelection -modal 1 -title "Use for Drop Color" -opacity 0}]

proc binaryColorToList { col } {
   # use only rgb value as unsigned 16 bit integer
   set res ""
   binary scan $col s3 colList
   foreach el $colList {
      lappend res [expr {($el + 0x10000) % 0x10000}]
   }
   puts $res
   return $res
}

proc dropFunc { type time length data } {
   puts "drop type: $type"
   puts "time: $time"
   puts [format "length: %d" [string length $data]]
   puts [format "data length: %d" [string bytelength $data]]
   puts $data
   switch -glob -- $type {
      application/x-color { 
            $::targetCol configure -background [binaryColorToList $data]
            }
      STRING -
      text/* { $::targetTxt configure -text $data }
   }
   puts ""
}

set source [gnocl::box -label "Drag'n Drop Source" -orientation vertical]
set box [gnocl::box -orientation vertical]
$box add [list $targetBox $source $colButton] -expand 1 -fill 1

set lab [gnocl::eventBox -child [gnocl::label -text "STRING: \"some text\""] \
      -dragTargets {STRING} -onDragData {dragFunc %T "some text" }]
$source add $lab

set lab [gnocl::eventBox \
      -child [gnocl::label -text "text/plain: \"other text\""] \
      -dragTargets {text/plain} -onDragData {dragFunc %T "other text"}]
$source add $lab

set val "\[]}{$; "
# escape the "$", otherwise $val would be evaluated when executing
# the callback function
set lab [gnocl::eventBox -child [gnocl::label -text $val] \
      -dragTargets {STRING text/plain} -onDragData "dragFunc %T \$val"]
$source add $lab

$source add [gnocl::eventBox \
      -child [gnocl::label -text "application/x-color: red"] \
      -dragTargets {application/x-color} \
      -onDragData {dragFunc %T [binary format s4 {65000 0 0 0}]}]
$source add [gnocl::eventBox \
      -child [gnocl::label -text "application/x-color: green"] \
      -dragTargets {application/x-color} \
      -onDragData {dragFunc %T [binary format s4 {0 65000 0 0}]}]

proc dragFunc { targets val } {
   set ::dragValue $val
   # puts "drag target: $targets\n"
   return $val
}


if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $box -onDestroy exit]
} else {
   set win [gnocl::window -child $box -defaultWidth 200 -defaultHeight 200 \
         -onDestroy exit]
}

set box [gnocl::box -homogeneous 1 -orientation vertical]
$box add [gnocl::box -dropTargets STRING -onDropData {puts "box got %d"}] \
   -expand 1 -fill 1
$box add [gnocl::table -dropTargets STRING -onDropData {puts "table got %d"}] \
   -expand 1 -fill 1
$box add [gnocl::label] -expand 1 -fill 1
gnocl::window -child $box -title "Box and Table Drop STRING targets" \
   -dropTargets STRING -onDropData {puts "window got %d"}
 
gnocl::mainLoop

