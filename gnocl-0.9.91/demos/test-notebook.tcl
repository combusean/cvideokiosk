#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-notebook.tcl,v 1.8 2004/08/12 08:17:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 [file join [file dirname [info script]] ../src]]
package require Gnocl

set notebook [gnocl::notebook -onShowHelp {puts "%w showHelp %h"}]
set right [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal -children "$notebook $right"]

puts [$notebook addPage [gnocl::label -text "Child Page 1"] "%__Page 1"]
puts [$notebook addPage [gnocl::label -text "Water\n(Page 2)"] "%<_H<sub>2</sub>0"]
puts [$notebook addPage [gnocl::label -text "Birds\n(Page 3)"] "%__Birds"]
puts [$notebook addPage [gnocl::label -text "Dogs\n(Page 4)"] "%__Dogs"]

$right add [gnocl::checkButton -text "%__Sensitive" -active 1 \
      -onToggled "$notebook configure -sensitive %v"]
$right add [gnocl::checkButton -text "%__Visible" -active 1 \
      -onToggled "$notebook configure -visible %v"]
$right add [gnocl::checkButton -text "%_Sh_ow tabs" -active 1 \
      -onToggled "$notebook configure -showTabs %v"]
$right add [gnocl::checkButton -text "Show border" -active 1 \
      -onToggled "$notebook configure -showBorder %v"]
$right add [gnocl::checkButton -text "%_S_crollable" \
      -onToggled "$notebook configure -scrollable %v"]
$right add [gnocl::checkButton -text "%_Pop_ups" \
      -onToggled "$notebook configure -enablePopup %v"]
$right add [gnocl::checkButton -text "%_Ho_mogeneous" \
      -onToggled "$notebook configure -homogeneous %v"]
$right add [gnocl::checkButton -text "%__Tooltip" \
      -onToggled {$notebook configure \
      -tooltip [expr {%v?"Notebook Tooltip":""}]}]
$right add [gnocl::checkButton -text "%_OnS_witchPage" \
      -onToggled "setSwitchFunc $notebook %v"]
$right add [gnocl::button -text "Next Page" -onClicked "$notebook nextPage"]
$right add [gnocl::button -text "Previous Page" \
      -onClicked "$notebook nextPage -1"]
$right add [gnocl::button -text "Remove Page" \
      -onClicked {$notebook removePage [$notebook currentPage]}]
$right add [gnocl::optionMenu -items {left right top bottom} \
      -onChanged "$notebook configure -tabPosition %v" -value top]
$right add [gnocl::box -orientation vertical -children [list \
      [gnocl::label -text "Border Width"] \
      [gnocl::spinButton -digits 0 -lower 0 -upper 5 \
            -onValueChanged "$notebook configure -borderWidth %v"]]]

set pageMenu [gnocl::optionMenu -items {0 1 2 3} \
      -onChanged "$notebook configure -page %v"]
$right add $pageMenu

proc setSwitchFunc { widget on } {
   set cmd ""
   if { $on } {
      set cmd {puts "switched to %p (last was: [%w currentPage])"}
   } 
   $widget configure -onSwitchPage $cmd
}

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox -onDestroy exit]
}

gnocl::mainLoop

