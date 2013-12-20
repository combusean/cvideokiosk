#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: canvas-man.tcl,v 1.11 2004/09/23 19:50:04 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]

package require GnoclCanvas

set canv [gnocl::canvas -background white -antialiased 1]
set coords {10 100 30 60 50 20 110 20 150 60 190 100}
foreach {x y} $coords {
   $canv create ellipse -coords [list $x $y 3] -centerRadius 1 -tags dots
}
$canv create bPath -coords {30 60 curveTo 50 20 110 20 150 60} -outline red -width 2 -tags "path t2"
$canv create line -coords [lrange $coords 0 5] -fill blue
$canv create line -coords [lrange $coords 6 11] -fill blue
$canv create ellipse -coords {70 140 50} -centerRadius 1 -fill "" -outline mediumOrchid -width 3 -dash {16 4}
$canv create rectangle -coords {90 110 180 180} -fill "blue 0.2" -outline green -width 3

$canv create text -coords {105 80} -text "gnocl" -font "Utopia 14"

$canv itemConfigure dots -fill darkgreen 
$canv itemConfigure "dots|path" -onButtonPress "puts pressed" -onButtonRelease "puts released" 

gnocl::window -title "Canvas" -child $canv -onDestroy exit

gnocl::mainLoop

