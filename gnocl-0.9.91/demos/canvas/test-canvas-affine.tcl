#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-affine.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

# an example with canvas lines
set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

proc mkItem { canvas x y color } {
   return [$canvas create line -coords \
         [list [expr {$x-10}] [expr {$y+10}] [expr {$x+10}] [expr {$y-10}]] \
         -fill $color -arrow last -arrowShape {5 5 5} -width 2] 
}
# text cannot be rotated?
# proc mkItem { canvas x y color } {
#    return [$canvas create text [list $x $y] -text "gnocl" -fill $color]
# }

set canvas [gnocl::canvas -background white]

# move
$canvas create text -coords "50 10" -text "move 20 10"
mkItem $canvas 40 30 blue
set item [mkItem $canvas 40 30 red]
$canvas move $item "20 10"

# scale
$canvas create text -coords "50 80" -text "scale 1.8"
mkItem $canvas 40 110 blue
set item [mkItem $canvas 40 110 red]
$canvas scale $item "40 110 1.8"

# rotate
$canvas create text -coords "50 150" -text "rotate 1.3"
mkItem $canvas 40 170 blue
set item [mkItem $canvas 40 170 red]
$canvas rotate $item "40 170 1.3"

set win [gnocl::window -child $canvas -defaultWidth 300 -defaultHeight 300 \
      -onDestroy exit]

gnocl::mainLoop

