#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-tree.tcl,v 1.16 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl 


set tree [gnocl::tree -titles {1 2 3}]
set win [gnocl::window -child $tree]

proc assert { opt val } {
   set val2 [$::tree columnCget 1 $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

foreach opt {-clickable -reorderable -resizable -visible} {
   foreach val {0 1 0 1} {
      $tree columnConfigure 1 $opt $val
      assert $opt $val
   }
}

# $tree columnConfigure 1 -sizing fixed
foreach opt {-maxWidth -minWidth} {
   foreach val {10 20 30 50} {
      $tree columnConfigure 1 $opt $val
      assert $opt $val
   }
}

foreach val {growOnly autosize fixed autosize autosize} {
   $tree columnConfigure 1 -sizing $val
   assert -sizing $val
}

foreach val {left right center} {
   $tree columnConfigure 1 -titleAlign $val
   assert -titleAlign $val
}

$tree columnConfigure 1 -minWidth 10 -maxWidth 50
foreach val {10 20 30 50} {
   $tree columnConfigure 1 -width $val
   gnocl::update
   assert -width $val
}

$win delete

proc assert { x y } {
   if { [string compare $x $y] } {
      error "\"$x\" != \"$y\""
   }
   puts "$x == $y"
}

set tree [gnocl::tree \
      -titles {string bool markup "integer (editable)" "float"} \
      -scrollbar never \
      -types {string boolean markup integer float} -selectionMode multiple \
      -onButtonPress {onButtonPress %w %t %x %y %b %s} \
      -onRowExpanded { puts "expanded %w %p" } \
      -onRowCollapsed { puts "collapsed %w %p" } \
      -onSelectionChanged "selChangedFunc %w %p"]

# gnocl::debug breakpoint

$tree configure -children {{Perl 1 blob -44 -3.6} {Python 1 py -37 -1.6}}
assert [$tree getNumChildren] 2
$tree configure -children {}
assert [$tree getNumChildren] 0

$tree add {} {{language}}
foreach {script comp other} [$tree add 0 { Script Compiled Others }] { break }
set path [$tree add $script {{Tcl 1 <b>blub</b> 999 3.1} \
      {bash 0 <big>blah</big> 374 5.3}}]
set tclPath [lindex $path 0]
set tclRef [$tree getReference $tclPath]

assert $tclPath [$tree referenceToPath $tclRef] 

set path [$tree add $script {Perl 1 <i>blob</i> -44 -3.6} -singleRow 1]
set perlRef [$tree getReference $path]

assert [$tree get $perlRef 0] Perl
assert [$tree get $path 0] [$tree get $perlRef 0] 

set pyPath [$tree add $script {Python 1 py -37 -1.6} -singleRow 1]
set pyRef [$tree getReference $pyPath]

assert [$tree get $perlRef 0] Perl
assert [$tree get $path 0] [$tree get $perlRef 0] 

$tree addBegin $script {Ruby 1 ru -25 -0.6} -singleRow 1
assert [$tree get $perlRef 0] Perl
assert [$tree get $path 0]  bash
$tree deleteReference $perlRef

$tree add $comp {{C 0 <u>foo</u> -4 9.4} \
      {C++ 1 {<span foreground="blue">bar</span>} 3 64.7} \
      {Java 0 <s>done</s> -100 3.7}} 
set path [$tree add $other {{"Others level 2"}}]
set path3 [$tree add [lindex $path 0] {"Others level 3"} -singleRow 1]
set path [$tree add $path3 {"level4"} -singleRow 1]
set otherRef [$tree getReference $path]
set path [$tree add $otherRef {"level 5"} -singleRow 1]
assert [$tree get $otherRef 0] level4
# delete parent of otherRef
$tree erase $path3
assert [catch {$tree referenceToPath $otherRef}] 1


assert [$tree get $pyRef 0] Python
assert [$tree get $tclRef 0] Tcl

$tree erase $pyRef
assert [catch {$tree referenceToPath $pyRef}] 1
set start [$tree add $other "aaa" -singleRow 1]
$tree add $other "bbb" -singleRow 1
set end [$tree add $other "ccc" -singleRow 1]
$tree add $other "ddd" -singleRow 1
$tree erase $start $end
# ddd is now start
$tree erase $start


puts "-------------- automatic tests done ------------------"

#$tree columnConfigure 0 -background green
$tree columnConfigure 1 -width 100 -clickable 1 \
      -onToggled "toggledFunc %w %p %c"
$tree columnConfigure 2 -background MistyRose
$tree columnConfigure 3 -editable 1 -onEdited "editedFunc %w %p %c %v" \
      -align right -background yellow -onCellData "formatCell %v"
$tree columnConfigure 4 -background peachPuff

# $tree columnConfigure 1 -scale x-large -weight bold -background red
# $tree columnConfigure 2 -align right 
# $tree columnConfigure 3 -titleAlign center -align center \
#       -family fixed -foreground blue -background lightyellow \
#       -editable 1
# foreach el {1 2 3} {
# $tree columnConfigure $el -reorderable 1 -onEdited "editedFunc %w %p $el %t"
# }

# $tree addEnd {{row7 20 "abc 7"} {row8 15 "abc 12"}}
# $tree addBegin {{row3 22 "abc 60"} {row2 5 "abc 22"}}
# $tree add {{row9 a32 "abc 70"} {row10 b7 "abc 22"}}
# $tree addBegin {{row1 22 "abc 60"}}

set mainBox [gnocl::box -orientation horizontal -homogeneous 0 \
      -children $tree -fill 1 -expand 1]
set right [gnocl::box -orientation vertical]
$mainBox add $right -expand 0

$right add [gnocl::checkButton -text "visible" -active 1 \
      -onToggled "$tree configure -visible %v"]
$right add [gnocl::label -text "scrollbar"]
$right add [gnocl::optionMenu  -onChanged "$tree configure -scrollbar %v" \
      -items "never always automatic"]

$right add [gnocl::button -text "Erase" -tooltip \
      "Erase all between the first and last selected rows" \
      -onClicked "erase $tree"]

$right add [gnocl::button -text "Erase to end" -tooltip \
      "Erase all rows between the first selected to the last of this depth" \
      -onClicked "eraseEnd $tree"]

$right add [gnocl::button -text "Erase all" -tooltip \
      "Erase all rows" -onClicked "$tree erase 0 end"]
$right add [gnocl::button -text "Expand" -tooltip \
      "Expand the selected row" -onClicked "expandCollapse $tree expand"]
$right add [gnocl::button -text "Collapse" -tooltip \
      "Expand the selected row" -onClicked "expandCollapse $tree collapse"]

$right add [gnocl::button -text "Scroll To Selection" -tooltip \
      "Scroll To Selection" -onClicked "scroll $tree"]
$right add [gnocl::button -text "Scroll To End" -tooltip \
      "Scroll To Selection" -onClicked "$tree scrollToPosition"]

$right add [gnocl::button -text "Print Tcl Reference"  \
      -onClicked [list printRef $tree $tclRef $tclPath]]
      
proc printRef { tree ref path } {
   puts "reference: [$tree get $ref 0]"
   puts "path $path [$tree get $path 0]"
}

proc scroll { widget } {
   set sel [$widget getSelection]
   if { [llength $sel] >= 1 } {
      $widget scrollToPosition -path [lindex $sel 0] 
   } 
}

proc expandCollapse { widget cmd } {
   set sel [lindex [$widget getSelection] 0]
   $widget $cmd -path $sel -recursive 1
}

proc onButtonPress { win type x y but state } {
   puts "button press: $win $type $x $y $but $state"
   foreach {path col x y} [$win coordsToPath $x $y] break
   puts "  -> path: $path column $col cell coords: $x $y"
}

proc eraseEnd { widget } {
   set sel [$widget getSelection]
   if { [llength $sel] >= 1 } {
      $widget erase [lindex $sel 0] end
   } 
}

proc erase { widget } {
   set sel [$widget getSelection]
   if { [llength $sel] == 1 } {
   puts "erase 1"
      $widget erase [lindex $sel 0]
   } elseif { [llength $sel] >= 2 } {
   puts "erase >= 2"
      $widget erase [lindex $sel 0] [lindex $sel end]
   }
}
proc toggledFunc { widget path col } {
   set val [$widget get $path $col]
   $widget cellConfigure $path $col -value [expr {!$val}] 
}

proc editedFunc { widget path col txt } {
   puts [format "in onEdited, path: %s old: \"%s\" new: \"%s\""  \
         $path [$widget get $path $col] $txt]
   $widget cellConfigure $path $col -value $txt 
}

proc selChangedFunc { widg path } {
   puts "sel changed: widget $widg, path $path: "
   if { [string compare $path [$widg getSelection]] } {
      puts "ERROR: path different!"
   }
   foreach el $path {
      puts -nonewline "   $el: "
      foreach row {0 1 2 3} {
         puts -nonewline [$widg get $el $row]
         puts -nonewline " "
      }
      puts ""
   }
}

proc formatCell { val } {
   if { [string index $val 0] == "-" } {
      return "-foreground red"
   }
   return "-foreground black"
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

$tree expand 
$tree setSelection {{0 0 0} {0 0 2}}
$tree onSelectionChanged
$tree setSelection {0 1 1} -add 1 -single 1
# $tree setSelection {}

proc assert { x y } {
   if { $x != $y } {
      error "$x != $y"
   } else {
      puts "$x == $y"
   }
}
#assert [$tree getNumChildren] [$tree getNumChildren {}] 
assert [$tree getNumChildren] 1
assert [$tree getNumChildren {0 0}] 4

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 300 -defaultHeight 200 -onDestroy exit]
}

puts "updates: [gnocl::update]"
$tree setCursor {0 1 2} -column 3 -startEdit 1

gnocl::mainLoop

