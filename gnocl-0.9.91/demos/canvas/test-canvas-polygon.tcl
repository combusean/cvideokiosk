#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-polygon.tcl,v 1.4 2004/09/23 19:50:04 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

# an example with canvas lines
set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

set canvas [gnocl::canvas -antialiased 1 -background white]

proc assert { opt val } {
   set val2 [$::canvas itemCget $::line $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

# puts "----- automatic tests done "------------"

$canvas create polygon -width 10 -fill green -outline orange \
      -coords {300 250 350 100 400 400 450 90 500 260} -tags polygon2

$canvas create polygon -width 6 -fill red -outline blue \
      -coords {50 250 200 200 250 50 300 200 450 250 300 300 250 450 200 300} \
      -tags polygon1 -dash {16 4}


proc config { opt val } {
   global canvas
   $canvas itemConfigure polygon1 $opt $val
}

set right [gnocl::box -orientation vertical]
# $right add [gnocl::box -borderWidth 0 -children [list \
#       [gnocl::label -text "capStyle" -align left -widthGroup labelWidth]  \
#       [gnocl::optionMenu -items {notLast butt round projecting} \
#             -onChanged "config -capStyle %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "fill" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu \
            -items {red blue "blue 0xD000" "blue 0x8000" "blue 0x2000" \
                  lightgreen} \
            -onChanged "config -fill %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "outline" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu \
            -items {red "red 0xD000" "red 0x8000" "red 0x2000" blue \
                  lightgreen} \
            -onChanged "config -outline %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "joinStyle" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu -items {miter round bevel} \
            -onChanged "config -joinStyle %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "width" -align left -widthGroup labelWidth]  \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -width %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "pixelWidth" -align left -widthGroup labelWidth]  \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -pixelWidth %v"]]] -expand 0


set mainBox [gnocl::box -orientation horizontal -homogeneous 0 \
      -children $canvas -fill 1 -expand 1]
$mainBox add $right -expand 0

set win [gnocl::window -title "Canvas Polygon Test" -child $mainBox \
      -defaultWidth 500 -defaultHeight 500 -onDestroy exit]

gnocl::mainLoop

