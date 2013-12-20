#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-list.tcl,v 1.18 2005/02/26 23:01:32 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set mainBox [gnocl::box -orientation horizontal -homogeneous 0]
set right [gnocl::box -orientation vertical]

set list [gnocl::list -titles {bool "bool (default callback)" \
      image string integer "float (editable)"} \
      -scrollbar never -selectionMode extended \
      -types {boolean boolean image string integer float} \
      -onSelectionChanged "selChangedFunc %w %p" \
      -children {{1 0 %/c.png "box" -345 4.546} \
            {0 1 %/two.jpg "text" 123 5.79}}]


$list add {{0 1 %#Delete "list" -1045 10.45} \
      {1 0 %/three.png "window" 3350 9.58}}
$list add {{0 1 %#DialogQuestion "blub" 1095 1.45} \
      {1 0 %/one.png "blah" 257 -8.58}}

$list columnConfigure 0 -clickable 0 -onToggled "clickCmd %w %p %c"
$list columnConfigure 3 -editable 1 -width 200
# $list columnConfigure 1 -fontScale x-large -fontWeight bold -background red
$list columnConfigure 4 -align right -background lightyellow \
      -onCellData "onCellData %v" 
$list columnConfigure 5 -titleAlign center -align center \
      -fontFamily fixed -foreground blue -background lightblue \
      -editable 1
# $list cellConfigure $el 1 -foreground green
foreach el {4 5} {
   $list columnConfigure $el -reorderable 1 -onEdited "editCmd %w %p %c %v"
}

# $list addEnd {{row7 20 "abc 7"} {row8 15 "abc 12"}}
# $list addBegin {{row3 22 "abc 60"} {row2 5 "abc 22"}}
# $list add {{row9 a32 "abc 70"} {row10 b7 "abc 22"}}
# $list addBegin {{row1 22 "abc 60"}}

$mainBox add $list -fill 1 -expand 1
$mainBox add $right -expand 0

$right add [gnocl::button -text "erase all" -onClicked "$list erase 0 end"]
$right add [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$list configure -visible %v"]
$right add [gnocl::label -text "scrollbar"]
$right add [gnocl::optionMenu  -onChanged "$list configure -scrollbar %v" \
      -items "never always automatic"]

proc onCellData { val } {
   if { $val > 0 } {
      return [list -foreground blue -value $val]
   }
   return [list -foreground red -value "($val)"]
}
proc clickCmd { widget path col } {
   set val [$widget get $path $col]
   $widget cellConfigure $path $col -value [expr {!$val}]
}

proc editCmd { widget path col txt } {
   if { [catch { $widget cellConfigure $path $col -value $txt } erg] } {
      puts "ERROR in edit: $erg"
   }
}

proc selChangedFunc { widg path } {
   puts -nonewline "sel changed: widget $widg, path $path, values: "
   foreach el {0 1 2 3} {
      puts -nonewline [$widg get $path $el]
      puts -nonewline " "
   }
   puts ""
   # if { [string compare $path "3"] == 0 } {
   #    puts "moving selection to path 2"
   #    $widg setSelection 2 
   # }
}

$list setSelection 1 
$list onSelectionChanged
$list setSelection {}

proc assertSel { ll } {
   gnocl::update
   set sel [$::list getSelection]
   if { [string compare $sel $ll] } {
      error "selection differ: $sel != $ll"
   } 
   puts "selection equal: $sel == $ll"
}
$list setSelection 1
assertSel 1
$list setSelection 4 -add 1
assertSel {1 4}
$list setSelection 5 -add 1 
assertSel {1 4 5}
$list setSelection all
assertSel {0 1 2 3 4 5}
$list setSelection {2 4 5} -add 1 -unselect 1
assertSel {0 1 3}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 300 -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

