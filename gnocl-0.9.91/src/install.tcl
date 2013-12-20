#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"
 
# $Id: install.tcl,v 1.5 2003/12/08 20:18:21 baum Exp $

if { $argc != 2 } {
   set name [file tail $argv0]
   error "Wrong number of args.\nUsage: $name version install/uninstall"
}

foreach {version what} $argv break

if { [info exist env(DESTDIR)] } {
   set dir $env(DESTDIR)
} else {
   set dir [info library]
}

set destDir [file join $dir gnocl-$version]
switch -- $what {
   "install"   {
                  if { [file exists $destDir] } {
                     puts "$destDir exists already. Aborting installation."
                     exit -1
                  }
                  puts "Creating $destDir"
                  file mkdir $destDir
                  set files [glob *.so]
                  lappend files pkgIndex.tcl
                  foreach file $files {
                     puts "Copying $file"
                     file copy $file $destDir
                  }
               }
   "test"      {
                  if { ![info exist env(DISPLAY)] } {
                     puts "Since DISPLAY is not set,\
                           package require cannot be tested."
                     exit 0
                  }
                  puts "Testing package require for $destDir"
                  if { [catch {package require Gnocl} err] } {
                     puts "package require Gnocl failed.\nError was:"
                     puts $err
                     puts "\nIf there is an error which says something like\
                          \n\"Xlib: connection to \":0.0\" refused by server\"\
                          \nthe error does not necessarily mean, that the\
                          installation failed.\
                          \nTry next time a \"xhost +localhost\""
                     exit 0
                  }
                  if { [catch {gnocl::info version} ver] } {
                     puts "gnocl::info failed"
                     exit -1
                  }
                  if { [string compare $ver $version] } {
                     puts "Version $ver != Version $version"
                     puts "The correct Gnocl version could not be loaded"
                     exit -1
                  }
                  puts "All tests passed"
                  puts "\nInsert a line \"package require Gnocl\"\
                        \nin your scripts to use gnocl commands."
               }
   "uninstall" {
                  puts "Deleting $destDir"
                  file delete -force $destDir
               }
   default     {
                  error "unknown type \"$what\" must be install, test or uninstall"
               }
}
   
