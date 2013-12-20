#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

# $Id: test-vfs.tcl,v 1.6 2004/12/02 20:56:29 baum Exp $

# ensure that "." is the decimal point
unset -nocomplain env(LC_ALL)
set env(LC_NUMERIC) "C"

set auto_path [linsert $auto_path 0 \
      [file join [file dirname [info script]] ../../src]]
package require GnoclVFS

puts [gnocl::info version]
set fileName [file normalize [info script]]
set files $fileName
lappend files [file normalize [file join [file dirname $fileName] ../c.png]]

proc assert { t1 t2 } {
   if { [string equal $t1 $t2] } {
      puts "$t1 == $t2"
   } else {
      puts "ERROR! $t1 != $t2"
   }
}


if 0 {
foreach file $files {
   puts [format "\n%s:" $file]
   foreach el {name fileType mimeType isLocal isSymlink size symlinkName} {
      
      gnocl::file info "file:$file" -$el $el
      assert $el [gnocl::file info "file:$file" -$el]
      if { [catch {
         puts [format "  %s: %s" $el [gnocl::file info "file:$file" -$el]]
      } erg] } {
         puts [format "  %s" $erg]
      }
   }
   set mimeType [gnocl::file info "file://$file" -mimeType]
   puts "  $mimeType:"
   foreach el {canBeExecutable defaultAction defaultApplication \
         description icon } {
      puts [format "     %s: %s" $el \
            [gnocl::mime getMimeInfo $mimeType -$el]]
   }
   puts "     Application list:"
   foreach el [gnocl::mime getApplicationList $mimeType -all 1] {
      puts "         $el"
   }
}
}


if 0 {
foreach app [gnocl::mime getApplicationList "text/x-sh" -all 1] {
   puts "\n$app: "
   foreach el {canOpenMultipleFiles command expectURIs name requiresTerminal \
         URISchemes } {
      puts [format "  %s: %s" $el [gnocl::mime getApplicationInfo $app -$el]]
   }
}
}

if 0 {
# set curDir [file dirname [exec pwd]]
proc printFileInfo { name type isLocal isSymlink symlinkName mimeType size } {
   foreach el { name type isLocal isSymlink symlinkName mimeType size } {
      puts [format "%s = %s" $el [set $el]]
   }
   puts ""
}

foreach curDir { "~" "." } {
   puts "Listing in $curDir"
   puts "all"
   puts [gnocl::file listDir $curDir]
   puts "\nonly tcl"
   puts [gnocl::file listDir $curDir -matchName {.*tcl$}]
   puts "\nonly start with q-t"
   puts [gnocl::file listDir $curDir -matchName {^[q-t].*}]
   puts "\nstart with q-t with callback"
   puts [gnocl::file listDir $curDir -matchName {^[q-t].*} \
         -onMatch "printFileInfo %n %t %l %k %K %m %s"]
}
}

# set fp [gnocl::file open file:///home/baum/devel/gnocl/demos/vfs/qqq]
# set fp [gnocl::file open ~/devel/gnocl/demos/vfs/qqq]

set txtFp [gnocl::file create /tmp/VFS_test.txt -force 1]
$txtFp write "Gnocl with Gnome VFS"
$txtFp close
exec zip -j /tmp/qqq.zip /tmp/VFS_test.txt
puts [gnocl::file listDir /tmp/qqq.zip#zip:]
set zipFp [gnocl::file open /tmp/qqq.zip#zip:VFS_test.txt]
$zipFp read 1024 ll
$zipFp close
puts $ll  ;# will print "Gnome with Gnome VFS" 


set txt "abcdefgh"
set fp [gnocl::file create ./qqq -force 1]
$fp write $txt
$fp close
set fp [gnocl::file open ./qqq]
puts [$fp read 100 ll]
$fp close
assert $ll $txt

set fp [gnocl::file open ./qqq -mode rw]
set txt "01234567$txt"
$fp write $txt
assert [$fp tell] [string length $txt]
$fp seek 0
assert [$fp tell] 0
$fp read 1024 ll
$fp close
assert $ll $txt

exec zip qqq.zip qqq
set ls [gnocl::file listDir ./qqq.zip#zip:]
puts $ls
assert $ls qqq
set fp [gnocl::file open ./qqq.zip#zip:qqq]
# set fp [gnocl::file open ssh://janus/home/baum/gnomecard]
$fp read 1024 ll
$fp close
assert $ll $txt

# gnocl::file open file:///home/baum/devel/gnocl/demos/vfs/qqq.zip#zip:/vfs/qqq
 gnocl::file listDir  file:///home/baum/devel/gnocl/demos/vfs/qqq.zip#zip:

# gnocl::file listDir ~/devel/gnocl/demos/vfs/q.zip#zip:


# gnocl::mime launch gedit [list ~/qqq file:///qqq  http:///qqq]
puts "\n\n ************** done **************"


