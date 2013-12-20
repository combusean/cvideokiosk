#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-canvas-richText.tcl,v 1.12 2005/02/26 23:01:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclCanvas

# in libgnomecanvas 2.6.0 richText is implemented 
# only for non-antialiased canvas
set right [gnocl::box -orientation vertical -homogeneous 0]
set left [gnocl::box -orientation vertical -homogeneous 1]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -expand 1 -fill 1 -homogeneous 0]
$mainBox add $right -expand 0

set canvas [gnocl::canvas -background white -antialiased 0 \
      -onKeyPress {puts [list KeyPress: %w %k %K %a]} \
      -onButtonPress "%w configure -hasFocus 1"]

proc assert { opt val } {
   set val2 [$::canvas itemCget $::text $opt]
   if { $val != $val2 } {
      puts "$opt: $val != $val2"
      return
   }
   puts "$opt: $val == $val2"
}

set text [$canvas create richText -coords {10 10}]

foreach opt {-onButtonPress -onButtonRelease -onEnter -onLeave -onMotion} {
   foreach val {qqq "" "as asd" "asdf" "" ""} {
      $canvas itemConfigure $text $opt $val
      assert $opt $val
   }
}


# with libgnomecanvas 2.6.0 -pixelsAboveLines gives an error
foreach opt {-indent -leftMargin -pixelsBelowLines 
      -pixelsInsideWrap -rightMargin } {
   foreach val {1 5 10 1 1} {
      $canvas itemConfigure $text $opt $val
      gnocl::update
      assert $opt $val
   }
}

# with libgnomecanvas 2.6.0 -visible gives an error
foreach opt {-cursorBlink -cursorVisible -editable -growHeight} {
   foreach val {1 0 0 1 1 0} {
      $canvas itemConfigure $text $opt $val
      assert $opt $val
   }
}

$canvas itemDelete $text

puts ""
puts "With libgnomecanvas-2.2.1 this gives error, since gnome lib is buggy"
puts "----- automatic tests done ------------"

set text [gnocl::text]
$left add [list $canvas $text] -fill 1 -expand 1
$canvas create ellipse -coords {150 80 2} -fill blue
$canvas create richText -coords {150 80} -height 70 -tags text -anchor NW \
      -text "Hello World!\nThis is the second line.\nAnd this the third"
$canvas create richText -coords {10 170} -width 200 -tags tagText 
# libgnomecanvas 2.6.0 does not allow to rotate richText
# $canvas rotate text "100 100 1.3"

$text tag create tag1 -foreground blue
$text tag create tag2
$canvas itemCommand tagText tag create tag1 -foreground blue
$canvas itemCommand tagText tag create tag2

$text insert end "This text has no tag\n"
$text insert end "This text has tag #1.\n" -tags tag1
$text insert end "This text has tag #2.\n" -tags tag2
$text insert end "NoTag" 
$text insert end "Tag#1"  -tags tag1
$text insert end "NoTag"
$text insert end "Tag#2"  -tags tag2
$text insert end "Tag#1"  -tags tag1

$canvas itemCommand tagText insert end "This text has no tag\n"
$canvas itemCommand tagText insert end "This text has tag #1\n" -tags tag1
$canvas itemCommand tagText insert end "This text has tag #2\n" -tags tag2
$canvas itemCommand tagText insert end "NoTag"
$canvas itemCommand tagText insert end "Tag#1" -tags tag1
$canvas itemCommand tagText insert end "NoTag"
$canvas itemCommand tagText insert end "Tag#2" -tags tag2
$canvas itemCommand tagText insert end "Tag#1" -tags tag1

proc setTags { tag val } {
   eval $::text tag configure $tag -$val
   eval $::canvas itemCommand tagText tag configure $tag -$val
}

set items {"fontStyle oblique" "fontWeight bold" "fontFamily courier" \
      "fontFamily helvetica" "foreground blue" "foreground red" \
      "background blue" "background white" "underline single" "underline none" } 
$right add [gnocl::label -text "tag1:"] -expand 0
$right add [gnocl::optionMenu -items $items \
      -onChanged "setTags tag1 %v"]
$right add [gnocl::label -text "tag2:"] -expand 0
$right add [gnocl::optionMenu -items $items \
      -onChanged "setTags tag2 %v"]

$right add [gnocl::checkButton -text "editable" -active 1 \
      -onToggled "$canvas itemConfigure text -editable %v"] -expand 0
$right add [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$canvas itemConfigure text -visible %v"] -expand 0
$right add [gnocl::checkButton -text "cursorVisible" -active 1 \
      -onToggled "$canvas itemConfigure text -cursorVisible %v"] -expand 0
$right add [gnocl::checkButton -text "cursorBlink" -active 1 \
      -onToggled "$canvas itemConfigure text -cursorBlink %v"] -expand 0
$right add [gnocl::checkButton -text "growHeight" -active 0 \
      -onToggled "$canvas itemConfigure text -growHeight %v"] -expand 0

proc addRight { txt widget } {
   $::right add [gnocl::box -borderWidth 0 -children [list \
         [gnocl::label -align left -text $txt -widthGroup labelGroup] \
         $widget] -expand 1 -fill 1]
}

addRight "wrapMode:" [gnocl::optionMenu -items {word char none} \
      -onChanged "$canvas itemConfigure text -wrapMode %v"]
addRight "justify:" [gnocl::optionMenu -items {left right center} \
      -onChanged "$canvas itemConfigure text -justify %v"]
addRight "anchor:" [gnocl::optionMenu \
      -items {center N NW NE S SW SE W E} \
      -onChanged "$canvas itemConfigure text -anchor %v" -value NW]
addRight "pixelsAboveLines:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged \
      "$canvas itemConfigure text -pixelsAboveLines %v"]
addRight "pixelsBelowLines:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged \
      "$canvas itemConfigure text -pixelsBelowLines %v"]
addRight "pixelsInsideWrap:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged \
      "$canvas itemConfigure text -pixelsInsideWrap %v"]
addRight "leftMargin:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged "$canvas itemConfigure text -leftMargin %v"]
addRight "rightMargin:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged "$canvas itemConfigure text -rightMargin %v"]
addRight "indent:" [gnocl::spinButton -digits 0 -upper 30 \
      -onValueChanged "$canvas itemConfigure text -indent %v"]

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

set win [gnocl::window -child $mainBox -onDestroy exit \
         -defaultHeight 500 -defaultWidth 500]

gnocl::mainLoop

