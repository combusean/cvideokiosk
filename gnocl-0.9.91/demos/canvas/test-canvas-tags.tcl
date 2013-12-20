#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-tags.tcl,v 1.11 2004/08/28 18:46:13 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]

package require GnoclCanvas

set pi2 [expr {-2.*atan(1)}]

set canvas [gnocl::canvas -antialiased 1 -background white]
      # -scrollRegion {0 0 500 500} -scrollbar never]

proc createItems { canv tags x y } {
   $canv create text -coords [list $x $y] -text $tags -tags $tags
   $canv create line -width 2 -tags $tags -coords \
         [list [expr {$x-10}] [expr {$y-10}] [expr {$x+10}] [expr {$y+10}]] 
   $canv create line -coords "-10 -10 10 10" -width 2
}

proc assertTags { canv item tags } {
   set orig [lsort [$canv itemCget $item -tags]]
   if { [string compare [lsort $tags] $orig] != 0 } {
      puts "ERROR: $orig != $tags"
   } else {
      # puts "$orig == $tags"
   }
}
set id [$canvas create line -coords {0 0 10 10}]
assertTags $canvas $id [list $id all]
$canvas itemConfigure $id -tags "t1 t3"
assertTags $canvas $id [list $id all t1 t3]
$canvas itemConfigure $id -tags t4
assertTags $canvas $id [list $id all t4]
$canvas itemConfigure $id -tags ""
assertTags $canvas $id [list $id all]
$canvas itemDelete $id

proc assertTagExpr { canv exp tags } {
   set orig [lsort [$canv findWithTag $exp]]
   if { [string compare [lsort $tags] $orig] != 0 } {
      puts "ERROR: $exp $orig != $tags"
   } else {
      puts [format " %-10s: \{%s\} == \{%s\}" $exp $orig $tags]
   }
}

for { set k 1 } { $k < 8 } { incr k } {
   set tags ""
   for { set m 0 } { $m < 3 } {incr m } {
      lappend tags [format "t%d" [expr {$k+$m}]]
   }
   set id$k [$canvas create line -coords {0 0 10 10} -tags $tags]
}

assertTagExpr $canvas "t3" [list $id1 $id2 $id3]
assertTagExpr $canvas "t4" [list $id2 $id3 $id4]
assertTagExpr $canvas "t5" [list $id3 $id4 $id5]
assertTagExpr $canvas "t8" [list $id6 $id7]
assertTagExpr $canvas "t3|t8" [list $id1 $id2 $id3 $id6 $id7]
assertTagExpr $canvas "t8|t3" [$canvas findWithTag "t3|t8"]
assertTagExpr $canvas "t3^t5" [list $id1 $id2 $id4 $id5]
assertTagExpr $canvas "t5^t3" [$canvas findWithTag "t3^t5"]
assertTagExpr $canvas "t3&t4" [list $id2 $id3]
assertTagExpr $canvas "t4&t3" [$canvas findWithTag "t3&t4"]
assertTagExpr $canvas "!t5" [list $id1 $id2 $id6 $id7]
assertTagExpr $canvas "(t3|t8)&t4" [list $id2 $id3]
assertTagExpr $canvas "t3&!t5" [list $id1 $id2]

assertTagExpr $canvas "t3|qqq" [$canvas findWithTag "t3"]
assertTagExpr $canvas "qqq|t3" [$canvas findWithTag "t3"]
assertTagExpr $canvas "t3^qqq" [$canvas findWithTag "t3"]
assertTagExpr $canvas "qqq^t3" [$canvas findWithTag "t3"]
assertTagExpr $canvas "t3&qqq" ""
assertTagExpr $canvas "qqq&t3" ""
assertTagExpr $canvas "!qqq" [$canvas findWithTag "all"]
assertTagExpr $canvas "(t2&!t5) | (t5&!t2)" [$canvas findWithTag "t2^t5"]

foreach el {"t2^t5)" "t2^(t5" "t2+t5" "t2&t5-"} {
   if { [catch {$canvas findWithTag $el} err] } {
      puts "Ok, error was: $err"
   } else {
      error "\"$el\" should be an error"
   }
}
createItems $canvas {top left} 100 100
createItems $canvas {top right} 300 100
createItems $canvas {bottom left} 100 300
createItems $canvas {bottom right} 300 300

$canvas itemConfigure right -fill blue
$canvas itemDelete bottom
$canvas create rectangle -coords [$canvas getBounds top] -fill {} \
      -outline yellow


set win [gnocl::window -title "Canvas Line Test" -child $canvas \
      -defaultWidth 500 -defaultHeight 500 -onDestroy exit]

gnocl::mainLoop

