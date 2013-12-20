#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-gconf.tcl,v 1.6 2004/03/07 09:27:18 baum Exp $

# ensure that "." is the decimal point
set ::env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclGconf

# find the tree path for a given key
proc gconfToTreePath { tree key } {
   set path ""
   foreach el [lrange [split $key /] 1 end] {
      for { set n 0} { 1 } { incr n } {
         set p $path
         lappend p $n
         set name [$tree get $p 0]
         if { [string compare $name $el] == 0 } {
            set path $p
            break
         }
      }
   }
   return $path
}

proc treeToGconfPath { tree path } {
   set gconf "/"
   
   set no [llength $path]
   for { set k 0} {$k < $no} { incr k } {
      set p [lrange $path 0 $k]
      append gconf  [$tree get $p 0]
      append gconf "/"
   }
   return $gconf
}

# this works only if the entry already exists
proc onChanged { tree key val } {
   if { [catch {set path [gconfToTreePath $tree $key]}] == 0 } {
      # set val [gnocl::gconf get $key]
      $tree cellConfigure $path 1 -value $val
   }
}

# fill tree recursively
proc fillTreeRec { tree confPath treePath } {
   set p ""
   foreach el [lsort [gnocl::gconf getDirs $confPath]] {
      set n [string last "/" $el]
      set name [string range $el [expr {$n+1}] end]
      set p [$tree add $treePath $name -singleRow 1]
      # to speed things up we don't look at schemas
      # if { [string compare $el "/schemas"] } { }
         fillTreeRec $tree $el $p
      # break
   }
   foreach el [lsort [gnocl::gconf getEntries $confPath]] {
      foreach {key val type} $el break
      set n [string last "/" $key]
      set name [string range $key [expr {$n+1}] end]
      # puts [list $name $val]
      if { [string compare $type "schema"] == 0 } {
         # continue
         foreach {sType default shortDesc longDescr owner locale} $val break
         # foreach el {sType locale owner shortDesc longDescr default} {
         #    puts [format "%10s: %s" $el [set $el]]
         # }
         set val "$shortDesc ($sType, default: $default)"
      }
      $tree add $treePath [list $name $val $type] -singleRow 1

      # set val [gnocl::gconf get $key] 
      # set defVal [gnocl::gconf get $key -type noDefault] 
      # if { [string compare $val $defVal] } {
      #    puts "$key: $val != $defVal"
      # }
   }
}

proc fillTree { tree } {
   fillTreeRec $tree / {}
}

proc expandRow { tree treePath } {
   set confPath [treeToGconfPath $tree $treePath]
   set last [$tree getNumChildren $treePath]
   foreach el [lsort [gnocl::gconf getDirs $confPath]] {
      set n [string last "/" $el]
      set name [string range $el [expr {$n+1}] end]
      set p [$tree add $treePath $name -singleRow 1]
      $tree add $p "" -singleRow 1
   }
   foreach el [lsort [gnocl::gconf getEntries $confPath]] {
      foreach {key val type} $el break
      set n [string last "/" $key]
      set name [string range $key [expr {$n+1}] end]
      # puts [list $name $val]
      if { [string compare $type "schema"] == 0 } {
         # continue
         foreach {sType default shortDesc longDescr owner locale} $val break
         # foreach el {sType locale owner shortDesc longDescr default} {
         #    puts [format "%10s: %s" $el [set $el]]
         # }
         set val "$shortDesc ($sType, default: $default)"
      }
      $tree add $treePath [list $name $val $type] -singleRow 1
   }
   if { $last > 0 } {
      incr last -1
      $tree erase "$treePath 0" "$treePath $last"
   }
}

proc makeKey { widget path } {
   set key "/"
   for { set k 0} { $k < [llength $path] - 1 } { incr k } {
      append key [$widget get [lrange $path 0 $k] 0]
      append key "/"
   }
   append key [$widget get $path 0]
   return $key
}

proc changeVal { widget path col value } {
   # $widget cellConfigure $path $col -value $value 
   set type [$widget get $path [expr {$col+1}]]
   set key [makeKey $widget $path]
   gnocl::gconf set $key $value -type $type 
}

proc printInfo { widget pathList } {
   if { [llength $pathList] == 0 } {
      return 
   }

   set key [format "/schemas%s" [makeKey $widget [lindex $pathList 0]]]
   set val [gnocl::gconf get $key]
   if { [llength $val] == 0 } {
      set val {"" "" "" "" "" ""}
   } 

   foreach {type default shortDesc longDescr owner locale} $val break
   $::typeLabel configure -text $type
   $::defaultLabel configure -text $default
   $::shortLabel configure -text $shortDesc
   $::longLabel configure -text $longDescr
   $::ownerLabel configure -text $owner
   $::localeLabel configure -text $locale
}

proc bgerror { err } {
   puts "in bgerror: $err"
   puts $::errorInfo
   # puts "errorCode: $::errorCode"
}

set mainBox [gnocl::box -orientation vertical -homogeneous 0]

set typeLabel [gnocl::label -align left]
set defaultLabel [gnocl::label -align left]
set shortLabel [gnocl::label -align left]
set longLabel [gnocl::label -wrap 1 -align left]
set ownerLabel [gnocl::label -align left]
set localeLabel [gnocl::label -align left]
set table [gnocl::table -homogeneous 0]
$table addColumn [list \
      [gnocl::label -text "Type:"] \
      [gnocl::label -text "Default:"] \
      [gnocl::label -text "Short Descr.:"] \
      [gnocl::label -text "Long Descr.:"] \
      [gnocl::label -text "Owner:"] \
      [gnocl::label -text "Locale:"] ] -align left -expand 0 -fill 0

$table addColumn [list $typeLabel $defaultLabel $shortLabel $longLabel \
      $ownerLabel $localeLabel] -fill {1 0} -expand 1 

set tree [gnocl::tree -titles { "" Value Type} \
      -onRowExpanded "expandRow %w %p" \
      -onSelectionChanged "printInfo %w %p"]
$tree columnConfigure 1 -editable 1 -onEdited "changeVal %w %p %c %v"
$mainBox add $tree -fill 1 -expand 1
$mainBox add $table 


# gnocl::gconf set /apps/gnome-volume-control/show-icons 0 -type boolean
gnocl::gconf set /extra/test/gnocl 0 -type boolean
gnocl::gconf set /extra/test/gnocl2 0 -type boolean
gnocl::gconf onChanged /extra/test/gnocl {puts "changed gnocl: %k"}
gnocl::gconf onChanged /extra/test/gnocl2 {puts "changed gnocl2: %k"}
gnocl::gconf set /extra/test/gnocl 1 -type boolean
gnocl::gconf onChanged /extra {puts "changed extra: %k"}
gnocl::gconf set /extra/test2/gnocl 0 -type boolean
puts [gnocl::gconf getEntry /extra/test]
puts [gnocl::gconf getEntry /extra/test/gnocl]
puts [gnocl::gconf getEntry /extra/test/gnocl2]

gnocl::gconf sync
gnocl::update

gnocl::gconf unset /extra/test/gnocl
gnocl::gconf unset /extra/test/gnocl2
#gnocl::gconf unset /extra/test
puts "\n\n"
#puts [gnocl::gconf getEntry /extra/test]
#puts [gnocl::gconf getEntry /extra/test/gnocl]
#puts [gnocl::gconf getEntry /extra/test/gnocl2]

puts "--------------"
proc assert { key val isDir } {
   # puts [gnocl::gconf getEntry $key] 
   foreach {nkey nval ntype nisDir} [gnocl::gconf getEntry $key] break
   if { $key != $nkey } {
      error "key not equal $key != $nkey"
   }
   if { $isDir != $nisDir } {
      puts "isDir not equal $isDir != $nisDir"
   }
   if { [string compare "notSet" $ntype] } {
      foreach {val type} $val break
      if { [string compare $type $ntype] } {
         error "type not equal $type != $ntype"
      }
      if { [string compare $val $nval] } {
         error "val not equal $val != $nval"
      }
   }
}

set dir "/extra/test"
gnocl::gconf unset "$dir" -recursive 1
assert "$dir/dir/val" notSet 0
assert "$dir/dir"     notSet 0
gnocl::gconf set "$dir/dir" 1 -type boolean
assert "$dir/dir/val" notSet 0
assert "$dir/dir"     {1 boolean} 0
gnocl::gconf set "$dir/dir/val" 0.5 -type float
assert "$dir/dir/val" {0.5 float} 0
assert "$dir/dir"     {1 boolean} 1
gnocl::gconf unset "$dir/dir/val" 
assert "$dir/dir/val" notSet 0
assert "$dir/dir"     {1 boolean} 0
gnocl::gconf set "$dir/dir/val" -0.5 
assert "$dir/dir/val" {-0.5 string} 0
assert "$dir/dir"     {1 boolean} 1
gnocl::gconf unset "$dir/dir" 
assert "$dir/dir/val" {-0.5 string} 0
assert "$dir/dir"     notSet 1
gnocl::gconf unset "$dir/dir/val" 
assert "$dir/dir/val" notSet 0
assert "$dir/dir"     notSet 0
gnocl::gconf set "$dir/dir/val" -11 -type integer
gnocl::gconf set "$dir/dir/val2" -0.5 -type float
gnocl::gconf set "$dir/dir/val3" "aaa 7" -type pair
gnocl::gconf set "$dir/dir/val4" "1 2 3 4 7" -type list
gnocl::gconf set "$dir/dir/dir1/val1" "xxx"
gnocl::gconf set "$dir/dir/dir1/val2" "yyy"
gnocl::gconf set "$dir/dir/dir1/dir1/val1" "zzz"
assert "$dir/dir/val" {-11 integer} 0
assert "$dir/dir/val2" {-0.5 float} 0
assert "$dir/dir/val3" {"aaa 7" pair} 0
assert "$dir/dir/val4" {"1 2 3 4 7" list} 0
assert "$dir/dir"     notSet 1
gnocl::gconf unset "$dir/dir" -recursive 1
assert "$dir/dir/val" notSet 0
assert "$dir/dir"     notSet 0

proc assert {a b} {
   if { [string compare $a $b] } {
      error "\"$a\" != \"$b\""
   }
}
gnocl::gconf set "/schemas$dir/val1" \
      {boolean 1 "short descr" "long descr" "gnocl"} -type schema
gnocl::gconf associateSchema $dir/val1 /schemas$dir/val1
# val does not yet exist
assert [gnocl::gconf get "$dir/val1"] 1
assert [gnocl::gconf get "$dir/val1" -mode default] 1
assert [gnocl::gconf get "$dir/val1" -mode noDefault] ""

gnocl::gconf set "$dir/val1" 0 -type boolean
assert [gnocl::gconf get "$dir/val1"] 0
assert [gnocl::gconf get "$dir/val1" -mode default] 1
assert [gnocl::gconf get "$dir/val1" -mode noDefault] 0

gnocl::gconf unset "$dir/val1"
assert [gnocl::gconf get "$dir/val1"] 1
assert [gnocl::gconf get "$dir/val1" -mode default] 1
assert [gnocl::gconf get "$dir/val1" -mode noDefault] ""

puts "---------- Automatic Test Done -------------"

#proc printDelTest { } {
#   puts [gnocl::gconf getEntry /extra/test3/gnocl]
#   puts [gnocl::gconf getEntry /extra/test3/gnocl/val]
#}
#proc testDel { args } {
#   puts "-- $args"
#   uplevel #0 $args
#}
#testDel gnocl::gconf set /extra/test3/gnocl 0 -type boolean
#printDelTest
#testDel gnocl::gconf set /extra/test3/gnocl/val 0 -type boolean
#printDelTest
#testDel gnocl::gconf unset /extra/test3/gnocl
#printDelTest
#testDel gnocl::gconf unset /extra/test3/gnocl/val
#printDelTest

# puts "get"
# puts [gnocl::gconf get /apps/battstat-applet/prefs/show_percent]
# puts "getDefault"
# puts [gnocl::gconf get /apps/battstat-applet/prefs/show_percent -type default]
# puts [gnocl::gconf get /schemas/apps/battstat-applet/prefs/show_percent]


# fillTree $tree 
expandRow $tree {}
gnocl::gconf onChanged / "onChanged $tree %k %v"

if { $argc == 1 } {
   set win [gnocl::plug -socketID [lindex $argv 0] -child $mainBox \
         -onDestroy exit]
} else {
   set win [gnocl::window -child $mainBox \
         -defaultWidth 300 -defaultHeight 200 -onDestroy exit]
}

gnocl::mainLoop

