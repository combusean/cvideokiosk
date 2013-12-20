#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-fileChooser.tcl,v 1.2 2005/02/22 23:16:19 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../src]]
package require Gnocl

set right [gnocl::box -orientation vertical]
set left [gnocl::box -orientation vertical]
set mainBox [gnocl::box -orientation horizontal \
      -children $left -expand 1 -fill 1 -homogeneous 0]
$mainBox add $right -expand 0
$left add [gnocl::button -text "default file chooser" \
      -onClicked doItDefault]
$left add [gnocl::button -text "file chooser with options" \
      -onClicked doIt]
set list [gnocl::list -columns 1]
$left add $list -fill 1 -expand 1

proc doItDefault { } {
   $::list erase 0 end
   set ret [gnocl::fileChooser]
   $::list add $ret
}

proc doPreview { widget file } {
   set preview [$widget cget -previewWidget]
   if { [catch {$preview configure -image %/$file} erg] } {
      $preview configure -image %#Cancel
   }
   $widget configure -previewWidgetActive 1
}

proc doIt { } {
   $::list erase 0 end
   set extraWidget ""
   if { $::extraWidgetFlag } {
      set extraWidget [gnocl::checkButton -text "Gnocl Extra Widget" \
            -variable extra]
   }
   set previewWidget ""
   set previewProc ""
   if { $::previewWidgetFlag } {
      set previewWidget [gnocl::image]
      set previewProc "doPreview %w %f"
   }
   set ret [gnocl::fileChooser -action $::action -title $::title \
         -selectMultiple $::selectMultiple -getURIs $::getURIs \
         -filename $::filename -selectFilename $::selectFilename \
         -currentFolder $::currentFolder -currentName $::currentName \
         -localOnly $::localOnly \
         -extraWidget $extraWidget \
         -onUpdatePreview $previewProc -previewWidget $previewWidget]
   $::list add $ret
}

proc addRight { txt widget } {
   $::right add [gnocl::box -borderWidth 0 -children [list \
         [gnocl::label -align left -text $txt -widthGroup labelGroup] \
         $widget] -expand 1 -fill 1]
}

addRight title [gnocl::comboBox -items {"Gnocl Test 1" "Gnocl Test 2"} \
      -variable title]
addRight action [gnocl::comboBox -items {open save openFolder createFolder} \
      -variable action]
$right add [gnocl::checkButton -text "Select Multiple" -variable selectMultiple]
$right add [gnocl::checkButton -text "Get URIs" -variable getURIs]
$right add [gnocl::checkButton -text "Extra widget" -variable extraWidgetFlag]
$right add [gnocl::checkButton -text "Preview widget" \
      -variable previewWidgetFlag]
$right add [gnocl::checkButton -text "localOnly" -variable localOnly]
addRight "Current folder" [gnocl::comboBox -items {"" "/usr/share"} \
      -variable currentFolder]
addRight "Current name" [gnocl::comboBox -items {"" gnocl.qqq} \
      -variable currentName]
addRight "Filename" [gnocl::comboBox \
      -items [list "" [pwd]/gnocl.qqq] -variable filename]
addRight "SelectFilename" [gnocl::comboBox \
      -items [list "" [pwd]/test-fileChooser.tcl] -variable selectFilename]

set win [gnocl::window -child $mainBox -onDestroy exit \
         -defaultHeight 500 -defaultWidth 500]

gnocl::mainLoop

