#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-text.tcl,v 1.7 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

# this is an example for the text widget
# TODO: font, foreground and background

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

# create submenu for menu "File" 
# with standard items "New" and "Quit" (text and icon)
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#New" -onClicked new]
$menu add [gnocl::menuItem -text "%#Print" \
      -onClicked {puts [$::txtID getChars 0 end]}]
$menu add [gnocl::menuSeparator]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit]

# create menu "File"
set file [gnocl::menuItem -text "%__File" -submenu $menu]

# create menu "Edit"
set menu [gnocl::menu -children [list \
      [gnocl::menuItem -text "%#Cut" -onClicked cut] \
      [gnocl::menuItem -text "%#Copy" -onClicked copy] \
      [gnocl::menuItem -text "%#Paste" -onClicked paste]]]

set edit [gnocl::menuItem -text "%__Edit" -submenu $menu]

# create menu "Help" with submenu "About" 
set help [gnocl::menuItem -text "%__Help" -submenu \
   [gnocl::menu -children \
   [gnocl::menuItem -text "%__About" -onClicked about]]]

# create toolBar with icons "Quit" and "New"
set toolBar [gnocl::toolBar -style both]
$toolBar add item -text "%#Quit" -tooltip "Tooltip Quit" \
            -onClicked exit
$toolBar add space
$toolBar add item -text "%#Cut" -tooltip "Cut Selection" \
      -onClicked cut
$toolBar add item -text "%#Copy" -tooltip "Copy Selection" \
      -onClicked copy
$toolBar add item -text "%#Paste" -tooltip "Paste Selection" \
      -onClicked paste
$toolBar add space
$toolBar add item -text "%#GotoFirst" -onClicked {$txtID scrollToPosition 0}
$toolBar add item -text "%#GotoLast" -onClicked {$txtID scrollToPosition end}
set menu  [gnocl::menu -children [list \
   [gnocl::menuItem -text "To Cursor" -onClicked {$txtID scrollToPosition cursor}] \
   [gnocl::menuItem -text "Cursor top" -onClicked {$txtID scrollToPosition cursor -align top}] \
   [gnocl::menuItem -text "Cursor center" -onClicked {$txtID scrollToPosition cursor -align center}] \
   [gnocl::menuItem -text "Cursor bottomLeft" -onClicked {$txtID scrollToPosition cursor -align bottomLeft}] \
   [gnocl::menuItem -text "End margin 0.1" -onClicked {$txtID scrollToPosition end -margin 0.1}]\
   [gnocl::menuItem -text "End margin 0.3" -onClicked {$txtID scrollToPosition end -margin 0.3}]\
   ]]
$toolBar add item -text "Goto" -onClicked "$menu popup"

# create text widget
set txtID [gnocl::text -editable 1]
set box [gnocl::box -orientation vertical -borderWidth 0 -spacing 0]
$box add [gnocl::menuBar -children [list $file $edit $help]] 
$box add $toolBar 
$box add $txtID -fill 1 -expand 1

gnocl::window -title "Test of Text Widget" -child $box \
      -defaultWidth 600 -defaultHeight 600 -onDestroy exit
## # create gnome application
## gnocl::app testApp -title "Test of Text Widget" \
##       -menuBar [gnocl::menuBar -children [list $file $edit $help]] \
##       -statusBar [gnocl::appBar] \
##       -toolBar $toolBar \
##       -contents $txtID

proc copy {}   { $::txtID copy }
proc cut {}    { $::txtID cut }
proc paste {}  { $::txtID paste }
proc new {}    { $::txtID remove 0 end }

proc about {} {
   puts "Text Test Application (c) 2001 - 2002 P.G. Baum"
#   gnocl::about -title "Text Test Application" -authors {{P.G. Baum}} \
#         -version [gnocl::info version] -copyright "(c) 2001 P.G. Baum"
}

set txt "The quick brown fox\njumps over\nthe lazy dog." 
$txtID insert 0 $txt
      # -foreground blue

set res [$txtID get 0 end]
if { [string compare $txt $res] } {
   puts "get all failed: \"$txt\" != \"$res\""
}
set res [$txtID get {1 6} {end-1 end}]
if { [string compare "over" $res] } {
   puts "get word failed: \"over\" != \"$res\""
}
$txtID erase {2 4} {end end-4}
set txt "The quick brown fox\njumps over\nthe dog." 
set res [$txtID get 0 end]
if { [string compare $txt $res] } {
   puts "erase failed: \"$txt\" != \"$res\""
}

$txtID insert {end end-4} "big "
set txt "The quick brown fox\njumps over\nthe big dog." 
set res [$txtID get 0 end]
if { [string compare $txt $res] } {
   puts "insert failed: \"$txt\" != \"$res\""
}

$txtID tag create monospace -fontFamily monospace
$txtID insert end "\n\nCharacter Table"
set txt ""
set noRows 64
for { set k 0 } { $k < 15 } { incr k } { 
   append txt [format "\n%3x: " [expr {$k*$noRows}]]
   for { set m 0 } { $m < $noRows } { incr m } {
      set val [expr {$k*$noRows+$m}]
      if { $val < 32 } {
         append txt " "
      } else {
         eval append txt [format {\u%x} $val] 
      }
   }
}
$txtID insert end $txt -tags monospace
$txtID insert end "\n"

# justification needs wrapMode != none
$txtID insert end "\n"
foreach el {left right center} {
   $txtID tag create justify$el -justification $el -wrapMode char
   $txtID insert end "Justification $el\n" -tags justify$el
}
$txtID insert end "\n"

set cursorPos [$txtID getCursor]

# style
foreach el { normal oblique italic } {
   $txtID tag create style$el -fontStyle $el 
   $txtID insert end "Style $el   " -tags style$el
}
$txtID insert end "\n"

# variant
foreach el { normal smallCaps } {
   $txtID tag create variant$el -fontVariant $el 
   $txtID insert end "Variant $el   " -tags variant$el
}
$txtID insert end "\n"

# weight
foreach el { ultralight light normal bold ultrabold heavy } {
   $txtID tag create weight$el -fontWeight $el 
   $txtID insert end "Weight $el   " -tags weight$el
}
$txtID insert end "\n"

# stretch
foreach el { ultraCondensed extraCondensed condensed semiCondensed \
      normal semiExpanded expanded extraExpanded ultraExpanded } {
   $txtID tag create stretch$el -fontStretch $el -wrapMode word
   $txtID insert end "Stretch $el   " -tags stretch$el
}
$txtID insert end "\n"

# scale
foreach val {xx-small x-small small medium large x-large xx-large } {
   set tag "tag-scale-$val"
   $txtID tag create $tag -fontScale $val -wrapMode word
   $txtID insert end "scale $val " -tags $tag
}
$txtID insert end "\n"

# family
foreach el {"helvetica" "new century schoolbook" "courier" \
      "lucida" "lucidatypewriter" "times" } {
   $txtID tag create family$el -fontFamily $el -wrapMode word
   $txtID insert end "Family $el  " -tags [list family$el]
}
$txtID insert end "\n\n"


# misc
$txtID tag create superscript -fontSize 8 -fontRise 10
$txtID tag create subscript -fontSize 8 -fontRise -10
$txtID insert end "This is"
$txtID insert end "superscript" -tags superscript
$txtID insert end "and this"
$txtID insert end "subscript" -tags subscript
$txtID insert end "\n"

foreach {el val} {foreground blue background red font "Courier 15"
      editable no underline single underline double strikethrough on } {
   # remove spaces from tag
   set tag [string map {{ } {}} "tag-$el-$val"]
   $txtID tag create $tag -$el $val
   $txtID insert end "$el $val\n" -tags $tag
}

foreach {el val} {wrapMode word wrapMode char wrapMode none } {
   # remove spaces from tag
   set tag [string map {{ } {}} "tag-$el-$val"]
   $txtID tag create $tag -$el $val
   $txtID insert end "\n$el $val: this is a test for the wrap mode.\
         What happens to lines, which do not fit on the screen anymore?" \
         -tags $tag
}

$txtID setCursor $cursorPos
$txtID setCursor cursor-10
$txtID configure -hasFocus 1
puts "Automatic tests done."


if { [info exists tk_patchLevel] } {
   wm withdraw .
} else {
   gnocl::mainLoop
}
