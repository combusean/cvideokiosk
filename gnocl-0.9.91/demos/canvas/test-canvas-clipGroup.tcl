#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-clipGroup.tcl,v 1.4 2005/02/26 23:01:32 baum Exp $

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

proc plotCoords { coords } {
   global canvas

   set cc ""
   foreach el $coords {
      if { [string is double $el] } {
         lappend cc $el
         if { [llength $cc] == 2 } {
            lappend cc 4
            $canvas create ellipse -coords $cc -fill darkgreen 
            set cc ""
         }
      }
   }
}
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

set clipId [$canvas create clipGroup -path $coords -tags clip1]
$canvas create bPath -coords [$canvas itemCget clip1 -path] -width 1 -fill ""
# plotCoords $coords
$canvas create line -coords "0 10 450 450" -parent clip1 -width 10 \
      -fill blue -tags line

set parentId [$canvas itemCget line -parent]
puts "$clipId == $parentId" 
if { $clipId != $parentId } {
   error "itemCget -parent failed ($clipId != $parentId)"
}

$canvas create rectangle -coords "350 100 400 400" -parent clip1 \
      -fill red -outline darkgreen -width 5 -tags rectangle
$canvas create ellipse -coords "150 200 50" -parent clip1 -fill "green 0.5"
set dir [file dirname [info script]]
$canvas create image -coords {240 90} -image "%/$dir/../floppybuddy.gif" \
      -parent clip1
$canvas create text -coords "60 300" -text "Text can be clipped" -parent clip1
# richText is not implemented for anti-aliased canvas
# and clipGroup obviously not for not anti-aliased canvas...
# $canvas create richText -coords "290 270" -text "richText" -parent clip1
$canvas create widget -coords "500 240" -parent clip1 \
      -widget [gnocl::button -text "Is not\nclipped" -onClicked "puts Hello"]

# $canvas configure -onMotion {puts "%x %y"}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

proc configXY { x y } {
   global canvas

   foreach {xOld yOld} [$canvas itemCget clip1 -coords] break
   if { [string length $x] } {
      set xOld $x
   } else {
      set yOld $y
   }
   $canvas itemConfigure clip1 -coords [list $xOld $yOld]
   puts [$canvas itemCget clip1 -coords]
}

## $right add [gnocl::box -borderWidth 0 -children [list \
##       [gnocl::label -text "x" -align left -widthGroup labelWidth]  \
##       [gnocl::spinButton -digits 0 -upper 300 \
##             -onValueChanged "configXY %v {}"]]] -expand 0
## $right add [gnocl::box -borderWidth 0 -children [list \
##       [gnocl::label -text "y" -align left -widthGroup labelWidth]  \
##       [gnocl::spinButton -digits 0 -upper 300 \
##             -onValueChanged "configXY {} %v"]]] -expand 0

set win [gnocl::window -child $mainBox -onDestroy exit \
      -defaultHeight 500 -defaultWidth 700]

gnocl::mainLoop

