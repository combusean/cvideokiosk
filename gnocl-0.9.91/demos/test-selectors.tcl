#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-selectors.tcl,v 1.9 2005/02/26 23:01:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation vertical -homogeneous 0]
# colorButton gives problems with GTK+ 2.4.9 and the plug window:
#Gtk-WARNING **: gtk_widget_size_allocate(): attempt to allocate widget with width -1 and height 23
# why???
$mainBox add [gnocl::colorButton]
$mainBox add [gnocl::fontButton]
$mainBox add [gnocl::button -text "file selection: modal, multi select" \
      -onClicked fileSelectionModalMulti]
$mainBox add [gnocl::button -text \
      "file selection: modal, single select, onClicked" \
      -onClicked fileSelectionModalCommand]
$mainBox add [gnocl::button -text "file selection: non modal, multi select" \
      -onClicked fileSelectionNonModalMulti]
$mainBox add [gnocl::button -text "color selection: modal" \
      -onClicked colorSelectionModal]
$mainBox add [gnocl::button -text "color selection: non modal" \
      -onClicked colorSelectionNonModal]
$mainBox add [gnocl::button -text "font selection: modal" \
      -onClicked fontSelectionModal]
$mainBox add [gnocl::button -text "font selection: non modal" \
      -onClicked fontSelectionNonModal]
proc bgerror msg {
   puts "in bgerror"
   puts $msg
}

proc fileCmd { widget but files } {
   puts "in file onClicked: "
   puts "   widget: $widget"
   puts "   button: $but"
   puts [format "   %d files: " [llength $files]]
   foreach el $files {
      puts "   $el"
   }
}
proc file2Cmd { widget but files } {
   fileCmd $widget $but $files
   puts "now deleting the widget"
   $widget delete
}

proc colorCmd { widget ret r g b a } {
   puts "in colorCmd: "
   puts "   widget: $widget"
   puts "   button: $ret"
   puts "   color: $r $g $b $a"
   puts "now deleting widget"
   $widget delete
}

proc fontCmd { widget ret font } {
   puts "in fontCmd: "
   puts "   widget: $widget"
   puts "   button: $ret"
   puts "   font: $font" 
   $widget delete
}

proc fontSelectionNonModal {} {
   set id [gnocl::fontSelection -title "Non Modal Serif Bold 14" -modal 0 \
         -font "Serif Bold 14" \
         -onClicked "fontCmd %w %x %f" -previewText "Umlaute work: ‰ˆ¸ƒ÷‹ﬂ"]
   puts "id: $id"
}

proc fontSelectionModal {} {
   set font [gnocl::fontSelection -title "Modal"]
   puts "font : $font"
}


proc colorSelectionNonModal {} {
   set id [gnocl::colorSelection -title "Non Modal, blue, opacity, no palette" \
         -modal 0 -opacity 1 -palette 0 -onDestroy {puts "destroying %w"} \
         -color blue -alpha 8000 -onClicked "colorCmd %w %x %r %g %b %a" ]
   puts "id: $id"
}

proc colorSelectionModal {} {
   set color [gnocl::colorSelection -title "Modal, red" \
         -onDestroy {puts "destroying %w"} -color red]
   puts "color : $color"
}

proc fileSelectionModalMulti {} {
   set files [gnocl::fileSelection -title "Modal, multi select" \
         -onDestroy {puts "destroying %w"} \
         -selectMultiple 1 -modal 1 -file "/usr/lib/"]
   puts [format "return from fileSelection: %d files" [llength $files]]
   foreach el $files {
      puts "   $el"
   }
}

proc fileSelectionModalCommand {} {
   set files [gnocl::fileSelection -title "Modal, single select" \
         -file "/etc/host.conf" -showFileOps 0 \
         -onDestroy {puts "destroying %w"} \
         -selectMultiple 0 -onClicked "fileCmd %w %x %f"]
   puts [format "return from fileSelection: %d files" [llength $files]]
   foreach el $files {
      puts "   $el"
   }
}

proc fileSelectionNonModalMulti {} {
   set ret [gnocl::fileSelection -title "Nonmodal, multi select" \
         -onDestroy {puts "destroying %w"} \
         -modal 0 -selectMultiple 1 -onClicked "file2Cmd %w %x %f"]
   puts "return from fileSelection: \"$ret\""
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 300 -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

