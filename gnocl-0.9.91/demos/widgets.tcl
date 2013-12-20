#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: widgets.tcl,v 1.9 2004/08/25 19:26:27 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

# this example shows different widgets

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set top [gnocl::window -title "Widget Test Window" -onDestroy exit]
set box [gnocl::box -orientation vertical -homogeneous 0]
$top configure -child $box

set menu [gnocl::menu \
      -children [gnocl::menuItem -text "%#Quit" -onClicked exit]]

set menubar [gnocl::menuBar -children \
      [gnocl::menuItem -text "%__Edit" -submenu $menu]]

set hBox [gnocl::box -orientation horizontal -homogeneous 0]
$hBox add [gnocl::button -text "%_Bu_tton" -onClicked {puts "do something"} \
      -tooltip "This is a button which does something when pressed" ]
$hBox add [gnocl::spinButton]
set om [gnocl::optionMenu -onChanged {puts "option %v"}]
$om add "gtk+" -value GTK
$om add "gnome" -value GNOME
$om add "kde" -value KDE
$om add "other" -value OTHER
$hBox add $om

set notebook [gnocl::notebook -enablePopup 1 -children [list \
      [gnocl::button -text "%_b_utton" -onClicked "puts hello"] \
      "%_boo_k1" "menu1"] -onSwitchPage {puts "switched to page %p"}]

$box add [list $menubar $hBox $notebook]

set check1 0
set check2 1
set check3 1
set entry1 "hello Entry"
set combo2 qqq

set widgets [list [gnocl::checkButton -text "%_Check _1" \
            -tooltip "Testcheck 1" -variable check1 \
            -onToggled {puts "check1: $check1"}] \
      [gnocl::checkButton -text "%_Check _2" -tooltip "Testcheck 2" \
            -variable check2 -onToggled {puts "check2: $check2"}] \
      [gnocl::checkButton -text "%_Check _3" -tooltip "Testcheck 3" \
            -variable check3 -onToggled {puts "check3: $check3"}]]
set boxFrame1 [gnocl::box -orientation vertical -shadow etchedIn \
      -label "Checkbuttons" -children $widgets]
$notebook addPage $boxFrame1 "%__check" "checkMenu"

#lappend widgets [gnocl::entry -variable entry1]
#$box addBegin $widgets
set boxHor [gnocl::box -orientation horizontal -homogeneous 0]
set entryWidget [gnocl::entry -variable entry1 \
      -tooltip "this is an entry widget"]

$boxHor addEnd [list $entryWidget \
      [gnocl::label -text "%_e_ntry " -mnemonicWidget $entryWidget]]
$box addBegin $boxHor

set boxHor [gnocl::box -label "Combo" -shadow etchedIn \
      -orientation horizontal -homogeneous 0]
$box addBegin $boxHor
$boxHor addBegin [gnocl::label -text "editable: " -justify left]
$boxHor addBegin [gnocl::combo -variable combo2 \
      -items {111 222 333 444} -tooltip "editable combobox"]
$boxHor addBegin [gnocl::label -text "not editable: " -justify fill]
#$boxHor addBegin [gnocl::combo -variable combo3 \
#      -items {aaa bbb ccc ddd} -editable 0 -tooltip "non editable combobox"]

set boxHor [gnocl::box -orientation horizontal]
$box add $boxHor

# first radio group
set box1 [gnocl::box -orientation vertical -shadow etchedIn \
      -label "Group 1"]
# $boxHor add $box1

set group1 1
$box1 add [gnocl::radioButton -text "%_G1 _radio 1" -variable group1 \
   -onValue 1 -tooltip "Button 1" -onToggled {puts "R1: group1 = $group1 = %v"}]
$box1 add [gnocl::radioButton -text "%_G1 r_adio 2" -variable group1 \
   -onValue 2 -tooltip "Button 2" -onToggled {puts "R2: group1 = $group1 = %v"}]
$box1 add [gnocl::radioButton -text "%_G1 ra_dio 3" -variable group1 \
   -onValue 3 -tooltip "Button 3" -onToggled {puts "R3: group1 = $group1 = %v"}]
$box1 add [gnocl::radioButton -text "%_G1 rad_io 4" -variable group1 \
   -onValue 4 -tooltip "Button 4" -onToggled {puts "R4: group1 = $group1 = %v"}]

# second radio group

set group2 3
set widgets [gnocl::radioButton -text "%__G2 radio 1" -variable group2 \
      -tooltip "Radio 1" -onValue 1 -onToggled {puts "R1: group2 = $group2"}]
lappend widgets [gnocl::radioButton -text "%_G2 radio 2_x" -tooltip "Radio 2" \
      -variable group2 -onValue 2 -onToggled {puts "R2: group2 = $group2"}]
lappend widgets [gnocl::radioButton -text "G2 radio 3" -tooltip "Radio 3" \
      -variable group2 -onValue 3 -onToggled {puts "R3: group2 = $group2"}]
lappend widgets [gnocl::radioButton -text "G2 radio 4" -tooltip "Radio 4" \
      -variable group2 -onValue 4 -onToggled {puts "R4: group2 = $group2"}]

set box2 [gnocl::box -orientation vertical -shadow etchedIn \
      -label "Group 2" -children $widgets]

set paned [gnocl::paned -orientation horizontal -children [list $box1 $box2]]
$boxHor add $paned -fill 1 -expand 1

$box add [gnocl::scale -orientation horizontal]

set txt [gnocl::text -editable 1]
$box add $txt -fill 1 -expand 1

set childs ""
for { set k 1} { $k < 5} { incr k } {
   lappend childs [list $k "label $k"]
}

proc listPrint { widget path } {
   puts [format "path: %s =\"%s\"" $path [$widget get $path 0]]
}
set list [gnocl::list -titles {"No." "Text"} \
      -children $childs -onSelectionChanged {listPrint %w %p}]
      # -justifications "right left" -height 200 -selectionMode extended 
$box add $list -fill 1 -expand 1

set children [list [gnocl::button -text %#Ok -onClicked "$top delete"]]
      #[gnocl::button -text %#Cancel -command "$top delete"] \
      #[gnocl::button -text %#Help]
set boxHor [gnocl::box -orientation horizontal -buttonType 1 -expand 1 \
      -fill 1 -padding 10 -children $children]
$box add $boxHor -expand 0
gnocl::mainLoop

