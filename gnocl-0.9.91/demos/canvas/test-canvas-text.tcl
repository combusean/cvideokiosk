#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-text.tcl,v 1.9 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

set canvas [gnocl::canvas -antialiased 1 -width 500 -height 500\
      -background white -scrollRegion {0 0 500 500}]


proc assert { opt val } {
   set val2 [$::canvas itemCget $::text $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}


set text [$canvas create text -coords {10 10}]

foreach opt {-onButtonPress -onButtonRelease -onEnter -onLeave -onMotion} {
   foreach val {qqq "" "as asd" "asdf" "" ""} {
      $canvas itemConfigure $text $opt $val
      assert $opt $val
   }
}

$canvas itemDelete $text
puts "----- automatic tests done ------------"


# anchor
set x 100
set y 50
foreach el {center N NW NE S SW SE W E} {
   $canvas create text -coords [list $x $y] -text "anchor $el" -anchor $el
   $canvas create ellipse -coords [list $x $y 2] -centerRadius 1 -fill red
   incr y 30
}

#fill
set x 250
set y 100
foreach el {black blue red darkgreen} {
   $canvas create text -coords [list $x $y] -text $el -fill $el -anchor W
   incr y 20
}

set x 250
set y 200
foreach {col txt} {blue rod green plau red krün} {
   $canvas create text -coords [list $x $y] -text $txt -font "Sans 15" \
         -fill $col -anchor W
   incr y 30
}

# Umlaute
$canvas create text -coords {250 300} -text "äöüÄÖÜß" -fill $col -anchor center

#offset
set x 100
set y 350
foreach {dx dy} {-20 -10 0 0 20 10 } {
   $canvas create ellipse -coords [list $x $y 2] -centerRadius 1 -fill red
   $canvas create text -coords [list $x $y] -text "offset $dx $dy" \
         -anchor W -fill blue -offset [list $dx $dy]
   incr y 30
}

#markup
set x 50
set y 450
set txt "normal <sub>subscript</sub> <sup>superscript</sup> <tt>monospace</tt> normal <b>bold</b> <i>italic</i> <big>big</big> <small>small</small>"
$canvas create text -coords [list $x $y] -text "no markup: $txt" -fill blue -anchor W
incr y 20
$canvas create text -coords [list $x $y] -text "%<markup: $txt" -fill blue -anchor W

set win [gnocl::window -child $canvas -defaultWidth 500 -defaultHeight 500 \
      -onDestroy exit]

gnocl::mainLoop

