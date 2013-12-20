#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: canvas.tcl,v 1.2 2003/06/24 19:19:20 baum Exp $

# ensure that "." is the decimal point
set ::env(LC_NUMERIC) "C"

# This example is a bit more complex application with many different
# widgets. It shows especially the use of the canvas widget.
# It's only ment as demonstration of the different widgets, not as
# example of good, ergonomic GUI design :-)

load ../../src/gnocl.so

if { ![gnocl::info hasGnomeSupport] } {
   gnocl::messageDialog -type error -text \
         "Sorry, this demo needs gnome support but the gnocl library\
         used is compiled without it."
   exit -1
}

# current colors as global variables 
set fillColor blue
set outlineColor blue

# make menus
# File menu
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "%#New"]
$menu add [gnocl::menuSeparator]
$menu add [gnocl::menuItem -text "%#Quit" -onClicked exit]
set file [gnocl::menuItem -text "%__File" -submenu $menu]

# Edit menu
set menu [gnocl::menu]
foreach {el acc} {flash "<control>F" message "<alt>M" 
      warning "<shift><control>A" error "" } {
   $menu add [gnocl::menuItem -text "%__$el" -onClicked \
         "\$app $el \"$el text\"" -accelerator $acc]
}
$menu add [gnocl::menuItem -text %__progess -onClicked \
      {
         for { set k 0 } { $k < 100 } { set k [expr {$k+1}]} {
            $app progress $k; gnocl::update; after 10 
         }
         $app progress 0
      }]

set edit [gnocl::menuItem -text "%__Edit" -submenu $menu]

# Help menu
set menu [gnocl::menu]
$menu add [gnocl::menuItem -text "About" -onClicked {
      gnocl::about -title "Gnocl Gnome Test Application" \
      -authors {{P.G. Baum (peter@dr-baum.net)}} \
      -version [gnocl::info version] \
      -copyright "(c) 2001 - 2002 P.G. Baum" \
      -comments "GTK+ version: [gnocl::info gtkVersion]" }]
set help [gnocl::menuItem -text "%__Help" -submenu $menu]


# add all menus to main menu Bar
set menuBar [gnocl::menuBar]
$menuBar add [list $file $edit $help]

set statusBar [gnocl::appBar]

# main toolBar
set toolBar [gnocl::toolBar -style both]
$toolBar add item -text "Clear" -icon "%#Clear" -tooltip "Clear canvas" \
      -onClicked {$canvas itemDelete all}
$toolBar addBegin space
$toolBar addBegin item -text "%#Quit" \
      -tooltip "Exit application" -onClicked exit

# $toolBar add space
# $toolBar add item -text "Save" -icon "%#Save" -tooltip "Tooltip Save" \
#             -tooltip_private "Tooltip_private Save"

set box [gnocl::box -orientation vertical -homogeneous 0]
set app [gnocl::app testApp -title "Gnocl Test Application" \
      -menuBar $menuBar -statusBar $statusBar -toolBar $toolBar \
      -contents $box -onDestroy exit -defaultHeight 400 -defaultWidth 500]

# second toolBar
proc fillTransparent { cpwidget transp } {
   $cpwidget configure -sensitive [expr {$transp?0:1}]
   set alpha [expr {$transp?0:65535}]
   set ::fillColor [lreplace [$cpwidget getColor] 3 3 $alpha]
}

set toolBar [gnocl::toolBar]
# set colorSelection [gnocl::colorPicker -variable fillColor \
#       -title "Select the fill color"]
# $toolBar add widget [gnocl::label -text "%_f_ill color " \
#       -bindUnderline $colorPicker]
# $toolBar add widget [gnocl::checkButton -text "%_t_ransparent" \
#       -command "fillTransparent $colorPicker %v"]
# $toolBar add widget $colorPicker
# $toolBar add space
# set colorPicker [gnocl::colorPicker -variable outlineColor \
#       -title "Select the outline color"]
# $toolBar add widget [gnocl::label -text "%__outline color " \
#       -bindUnderline $colorPicker]
# $toolBar add widget $colorPicker
# $toolBar add space
# # font is automatically set to default font
# set fontPicker [gnocl::fontPicker -variable font]
# $toolBar add widget [gnocl::label -text "%_fo_nt " -bindUnderline $fontPicker]
# $toolBar add widget $fontPicker
$app addToolBar $toolBar -name "Color and Font" -bandNum 2 

set hbox [gnocl::box -orientation horizontal -homogeneous 0]
$box add [list $hbox \
      [gnocl::label -text "Please click in the canvas below"]] -expand 0
set ::b2Action Move
set ::drawItem Line
set itemsWidget [gnocl::optionMenu -variable drawItem \
      -items {Line Rectangle Ellipse Text}]
set actionWidget [gnocl::optionMenu -variable b2Action \
      -items {Move Scale Rotate}]
$hbox add [list \
      [gnocl::label -text "%_ B_1: " -mnemonicWidget $itemsWidget] \
      $itemsWidget \
      [gnocl::label -text "%_ B_2: " -mnemonicWidget $actionWidget] \
      $actionWidget] -expand 0 -fill 0

set canvas [gnocl::canvas -antialiased 1 -background white]
$box add $canvas -fill 1 -expand 1

proc defaultCanvasBindings { } {
   $::canvas configure -onButtonPress [list canvasButton %w %x %y %b] \
         -onButtonRelease "" -onMotion ""
}

defaultCanvasBindings

# different procs which are called if mouse point is over one item
# or mouse button is pressed
proc enterItem { item x y } {
   puts "enter: $item"
   $::canvas move $item {10 10}
   # $::canvas itemConfigure $item -fill red
}
proc leaveItem { item x y } {
   puts "leave $item"
   $::canvas itemConfigure $item -fill $::fillColor
}

proc configLine {win x y item } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   $win itemConfigure $item \
         -coords [lreplace $::lineCoords end-1 end $x $y]
}
proc lineAddPoint {win x y but item } {
   if { $but == 1 } {
      foreach {x y} [$win windowToWorld [list $x $y]] break
      set ::lineCoords [lreplace $::lineCoords end-1 end $x $y $x $y]
      $win itemConfigure $item -coords $::lineCoords
   } 
}
proc lineEndPoint {win but item } {
   if { $but != 1 } {
      if { [llength $::lineCoords] == 4 } {
         $win itemDelete $item
      } else {
         $win itemConfigure $item -coords [lrange $::lineCoords 0 end-2]
      }
      defaultCanvasBindings
   }
}

proc configRect {win x y item x0 y0 } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   set r [expr {sqrt(pow($x-$x0,2)+pow($y-$y0,2))}]
   $win itemConfigure $item -centerRadius 1 -coords \
         [list $x0 $y0 [expr {abs($x-$x0)}] [expr {abs($y-$y0)}]]
}
proc configEllipse {win x y item x0 y0 } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   set r [expr {sqrt(pow($x-$x0,2)+pow($y-$y0,2))}]
   $win itemConfigure $item -centerRadius 1 -coords [list $x0 $y0 $r]
}
proc moveItem {win x y item x0 y0 } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   $win move $item [list [expr {$x-$x0}] [expr {$y-$y0}]]
   $win configure -onMotion "moveItem $win %x %y $item $x $y"
}
proc scaleItem {win x y item x0 y0 {lastX 1.} {lastY 1.} } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   set sensitivity 10.
   set xscale [expr {exp(($x-$x0)/$sensitivity)}]
   set yscale [expr {exp(($y-$y0)/$sensitivity)}]
   $win scale $item \
         [list $x0 $y0 [expr {$xscale/$lastX}] [expr {$yscale/$lastY}]]
   $win configure -onMotion \
         "scaleItem $win %x %y $item $x0 $y0 $xscale $yscale"
}
proc printCoords { win item } {
   foreach {x1 y1 x2 y2} [$win getBounds $item] break
   set xm [expr {($x2+$x1)/2.}]
   set ym [expr {($y2+$y1)/2.}]
   puts "x1: $x1 y1: $y1 x2: $x2 y2: $y2 xm: $xm ym $ym"
}
proc rotateItem {win x y item x0 y0 {last 0} } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   foreach {x1 y1 x2 y2} [$win getBounds $item] break
   set xm [expr {($x2+$x1)/2.}]
   set ym [expr {($y2+$y1)/2.}]
   set w0 [expr {atan2($x0-$xm,$y0-$ym)}]
   set w [expr {atan2($x-$xm,$y-$ym)-$w0}]
   set ws [expr {-$w+$last}]
   $win rotate $item [list $xm $ym $ws]
   $win configure -onMotion "rotateItem $win %x %y $item $x0 $y0 $w"
}

#   rotate with affine 
#   $win move $item [list [expr {-$xm}] [expr {-$ym}]]
#   set af [list [expr {cos($ws)}] [expr {sin($ws)}] \
#               [expr {-sin($ws)}] [expr {cos($ws)}] \
#               [expr {0}] [expr {0}]]
#   $win affine $item $af
#   $win move $item [list $xm $ym]

proc canvasButton {win x y but } {
   foreach {x y} [$win windowToWorld [list $x $y]] break
   if { $but == 1 } {
      switch -exact $::drawItem {
         Line    {  set ::lineCoords [list $x $y $x $y]
                    set item [$win create line -coords $::lineCoords \
                          -fill $::fillColor -width 3 -arrow both \
                          -arrowShape {5 10 5} \
                          -capStyle butt -joinStyle round]
                    $win configure \
                          -onMotion "configLine %w %x %y $item" \
                          -onButtonPress "lineAddPoint %w %x %y %b $item" \
                          -onButtonRelease "lineEndPoint %w %b $item"
                 }
         Ellipse     {  set item [$win create ellipse -coords [list $x $y 1] \
                              -centerRadius 1 -fill $::fillColor \
                              -outline $::outlineColor -width 3]
                        $win configure \
                              -onMotion "configEllipse $win %x %y $item $x $y" \
                              -onButtonRelease defaultCanvasBindings
                     }
         Rectangle   {  set item [$win create rectangle -coords [list $x $y 1] \
                              -centerRadius 1 -fill $::fillColor \
                              -outline $::outlineColor -width 3]
                        $win configure \
                              -onMotion "configRect $win %x %y $item $x $y" \
                              -onButtonRelease defaultCanvasBindings
                     }
         Text        {  set item [$win create text -coords [list $x $y] \
                              -label "gnocl" -fill $::fillColor -font $::font]
                     }
      }
      # puts "new $item"
      $win itemConfigure $item \
            -onEnter [list $::app flash "entered $::drawItem $item @(%x,%y)"]

   } elseif { $but == 2 } {
      set item [$win findItemAt $x $y]
      if { $item != "" } {
         switch -exact $::b2Action {
            Move  { $win configure -onMotion "moveItem %w %x %y $item $x $y" }
            Scale { $win configure -onMotion \
                           "scaleItem $win %x %y $item $x $y" }
            Rotate { $win configure -onMotion \
                           "rotateItem $win %x %y $item $x $y" }
         }
         $win configure -onButtonRelease defaultCanvasBindings
      }
   }
}

proc createEllipse {win x y but } {
   if { [catch {
      foreach {x y} [$win windowToWorld [list $x $y]] break
      switch -exact $but {
         1  { set item [$win create ellipse [list $x $y 10] \
                     -centerRadius 1 -fill $::fillColor \
                     -outline $::outlineColor] }
         2  { set item [$win create ellipse [list $x $y 25] \
                     -centerRadius 1 -fill $::fillColor \
                     -outline $::outlineColor -width 3] }
         3  { set item [$win create ellipse [list $x $y \
                     [expr {$x+5.}] [expr {$y+5.}]] ] }
      }
   } erg] } {
      $::app error "in createEllipse:\n$erg"
   }
}
if { [info exists tk_patchLevel] } {
   wm withdraw .
} else {
   gnocl::mainLoop
}

