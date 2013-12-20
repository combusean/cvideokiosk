#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-bpath.tcl,v 1.11 2004/09/23 19:50:04 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

set canvas [gnocl::canvas -background white -antialiased 1]
set right [gnocl::box -orientation vertical -homogeneous 0]
set mainBox [gnocl::box -orientation horizontal \
      -children $canvas -expand 1 -fill 1 -homogeneous 0]
$mainBox add $right -expand 0

proc plotCoords { coords {tag ""} } {
   global canvas

   set cc ""
   foreach el $coords {
      if { [string is double $el] } {
         lappend cc $el
         if { [llength $cc] == 2 } {
            lappend cc 2
            $canvas create ellipse -coords $cc -fill darkgreen -tags $tag
            set cc ""
         }
      }
   }
}
set x0 10
set y0 120
set coords [list $x0 $y0]
set dy 100
foreach x {0 10 20 30 40} {
   lappend coords curveTo [expr {$x0+$x}] [expr {$y0+$dy}] \
         [expr {$x0+100-$x}] [expr {$y0+$dy}] \
         [expr {$x0+100}] $y0 
   set dy [expr {$dy*-1}]
   incr x0 100
}
$canvas create bPath -coords $coords -outline blue -tags "bpath bpath1" \
      -width 10 -dash {32 4 4 8 8}
plotCoords $coords

$canvas create bPath -coords [$canvas itemCget bpath1 -coords] -fill ""

set x0 50
set y0 250
set coords [list $x0 $y0]
set dy 100
foreach dy {100 -80 60 -40 20} {
   lappend coords curveTo [expr {$x0+20}] [expr {$y0+$dy}] \
         [expr {$x0+80}] [expr {$y0+$dy}] \
         [expr {$x0+100}] $y0
   incr x0 100
}
lappend coords lineTo 300 100
lappend coords close
plotCoords $coords rotateItems

$canvas create bPath -coords $coords -fill red -outline blue \
      -width 10 -tags "bpath bpath2 rotateItems"

$canvas create bPath -coords [$canvas itemCget bpath2 -coords] -fill ""

$canvas rotate rotateItems "300 340 0.2"

set coords [list \
      moveTo 100 400 lineTo 10 400 lineTo 10 450 lineTo 100 450 closeCurrent \
      moveTo 150 400 lineTo 200 400 lineTo 200 450 lineTo 150 450 close]

$canvas create bPath -coords $coords -fill orange -outline blue \
      -width 10 -tags bPath3 -dash {10 10}

$canvas create bPath -coords [$canvas itemCget bPath3 -coords] -fill ""

proc config { opt val } {
   global canvas
   $canvas itemConfigure bpath $opt $val
}

proc config2 { opt val } {
   global canvas
   $canvas itemConfigure bpath2 $opt $val
}

proc onButton { canvas x y but } {
   global current 
   if { [string length $current] } {
      if { $but == 1 } {
         $canvas itemCommand $current appendCoords [list lineTo $x $y]
      } else {
         set current ""
      }
   } else {
      set current [$canvas create bPath -width 3 -outline orange \
            -coords [list moveTo $x $y]]
   }
}

proc onMotion { canvas x y } {
   global current 
   if { [string length $current] } {
      $canvas itemCommand $current appendCoords [list lineToMoving $x $y]
   } 
}

set current ""
$canvas configure -onMotion "onMotion %w %x %y" \
      -onButtonPress "onButton %w %x %y %b"

$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "fill" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu \
            -items {red blue "blue 0xD000" "blue 0.3" "" darkgreen } \
            -onChanged "config2 -fill %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "outline" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu \
            -items {red blue "blue 0xD000" "blue 0.3" darkgreen } \
            -onChanged "config2 -outline %v"]]] -expand 0

$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "joinStyle" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu -items {miter round bevel} \
            -onChanged "config -joinStyle %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "capStyle" -align left -widthGroup labelWidth]  \
      [gnocl::optionMenu -items {notLast butt round projecting} \
            -onChanged "config -capStyle %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "width" -align left -widthGroup labelWidth]  \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -width %v"]]] -expand 0
$right add [gnocl::box -borderWidth 0 -children [list \
      [gnocl::label -text "pixelWidth" -align left -widthGroup labelWidth]  \
      [gnocl::spinButton -digits 0 -upper 30 \
            -onValueChanged "config -pixelWidth %v"]]] -expand 0
$right add [gnocl::label -text "Click in the canvas to draw a bPath"]

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

set win [gnocl::window -child $mainBox -onDestroy exit \
      -defaultHeight 500 -defaultWidth 700]

gnocl::mainLoop

