#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-widget.tcl,v 1.9 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

set canvas [gnocl::canvas -antialiased 1 -background white \
      -width 500 -height 500]

proc assert { opt val } {
   set val2 [$::canvas itemCget $::widget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}


set widget [$canvas create widget -coords {10 10}]

foreach opt {-onButtonPress -onButtonRelease -onEnter -onLeave -onMotion} {
   foreach val {qqq "" "as asd" "asdf" "" ""} {
      $canvas itemConfigure $widget $opt $val
      assert $opt $val
   }
}

set opt -width
foreach val {1.1 5.1 0.7} {
   $canvas itemConfigure $widget $opt $val
   assert $opt $val
}

set opt -coords
foreach val {"1.0 1.0" "1.0 5.0" "3.0 2.0"} {
   $canvas itemConfigure $widget $opt $val
   assert $opt $val
}

set opt -widget
foreach val [list [gnocl::label] [gnocl::label]] {
   $canvas itemConfigure $widget $opt $val
   assert $opt $val
}

$canvas itemDelete $widget
puts "----- automatic tests done ------------"


# anchor
set x 250
set y 50
foreach el {center N NW NE S SW SE W E} {
   $canvas create widget -coords [list $x $y] -width 100 -height 25 \
         -widget [gnocl::button -text "anchor $el" \
         -onClicked "puts \"anchor $el\""] -anchor $el
   $canvas create ellipse -coords [list $x $y 3] -centerRadius 1 -fill red
   incr y 50
}

set win [gnocl::window -child $canvas -defaultWidth 500 -defaultHeight 500 \
         -onDestroy exit]

gnocl::mainLoop

