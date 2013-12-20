#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-label.tcl,v 1.10 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set label [gnocl::label]

proc assert { opt val } {
   set val2 [$::label cget $opt]
   if { $val != $val2 } {
      error "$opt: $val != $val2"
   }
   puts "$opt: $val == $val2"
}

foreach yPad { 1 4 } {
   foreach xPad { 2 5 } {
      $label configure -xPad $xPad -yPad $yPad
      assert -xPad $xPad
      assert -yPad $yPad
   }
}

foreach opt {-wrap -selectable -visible -sensitive} {
   foreach val {1 0 1} {
      $label configure $opt $val
      assert $opt $val
   }
}

foreach opt {-data -name -widthGroup -heightGroup -sizeGroup} {
   foreach val {aaa bbb "" "" ccc "" ddd ""} {
      $label configure $opt $val
      assert $opt $val
   }
}

foreach el {left center right fill} {
   $label configure -justify $el
   assert -justify $el
}

foreach el {topLeft top topRight left center right bottomLeft \
      bottom bottomRight "0.2 0.2" "0.2 0.8" "0.8 0.8" } {
   $label configure -align $el
   if { [llength $el] == 1 } {
      assert -align $el
   } else {
      set val2 [$::label cget -align]
      foreach a $el b $val2 {
         if { abs( $a - $b ) > 0.001 } {
            error "$-align: $el != $val2"
         }
      }
      
   }
}

$label configure -name "label1"
assert -name "label1" 

foreach el { "Simple<big> _St</big>ring" "%_Under_line" "%<<b>Pango</b>"} {
   $label configure -text $el
   assert -text $el
}

set widg [gnocl::label]
$label configure -mnemonicWidget $widg
assert -mnemonicWidget $widg
$widg delete

$label delete

puts "----- automatic tests done ------------"

set left [gnocl::box -orientation vertical -shadow in]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children $left]
$mainBox add $right -expand 0

set label [gnocl::label -text \
      "%<normal <sub>subscript</sub> <sup>superscript</sup> <tt>monospace</tt>
normal <b>bold</b> <i>italic</i> <big>big</big> <small>small</small>"]
$left add $label


# label with entry as mnemonic widget
set entry [gnocl::entry]
set box [gnocl::box -orientation horizontal -children [list \
      [gnocl::label -text "%_E_ntry: " -mnemonicWidget $entry] $entry]]
$left add $box -expand 0

# label with option menu as mnemonic widget
set optionMenu [gnocl::optionMenu]
foreach el {foo bar boom} {
   $optionMenu add $el
}
set box [gnocl::box -orientation horizontal -children [list \
      [gnocl::label -text "%_O_ption menu: " -mnemonicWidget $optionMenu] \
      $optionMenu]]
$left add $box -expand 0

set wrapLabel [gnocl::label -wrap 1 -text "wrap test: blah blub blub blah\
      foo bar blub blah blah blub blah blub foo bar foo bar"]
$left add $wrapLabel  -expand 0

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "$label configure -sensitive %v"] -expand 0
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$label configure -visible %v"] -expand 0
$right add [gnocl::checkButton -text "%_S_electable" -active 0 \
      -onToggled "$label configure -selectable %v"] -expand 0
$right add [gnocl::label -text "justify"] -expand 0
set justifyMenu [gnocl::optionMenu -onChanged "$label configure -justify %v"]
foreach el {left center right fill} {
   $justifyMenu add $el -value $el
}
$right add $justifyMenu -expand 0

$right add [gnocl::label -text "align"] -expand 0
set alignMenu [gnocl::optionMenu -onChanged "$label configure -align %v"]
foreach el {topLeft top topRight left center right bottomLeft \
      bottom bottomRight "0.2 0.2" "0.2 0.8" "0.8 0.8" } {
   $alignMenu add $el -value $el
}
$alignMenu configure -value center
$right add $alignMenu -expand 0

# wrap cannot be changed on the fly: bug in GTK+ 2.0.2 ?
$right add [gnocl::checkButton -text "%__Wrap" -active 1 -sensitive 0 \
      -onToggled "$wrapLabel configure -wrap %v"] -expand 0

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}
if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

