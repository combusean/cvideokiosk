#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

#fconfigure $handle -blocking 0 -buffering line -translation auto

proc ReadOutput {dvhandle} \
 {	set status [catch { gets $dvhandle line } result]
    if { $status != 0 } {
        # Error on the channel
        puts "error reading $f: $result"
        set ::DONE 2
    } elseif { $result >= 0 } {
        # Successfully read the channel
        puts "got: $line"
    } elseif { [eof $f] } {
        # End of file on the channel
        puts "end of file"
        set ::DONE 1
    } elseif { [fblocked $f] } {
	puts "blocked!"
        # Read blocked.  Just return
    } else {
        # Something else
        puts "can't happen"
        set ::DONE 3
    }


 }	

set handle [open "|dvgrab -i --guid  0x00808803019035a8 2>@ stdout" "r+"]
fconfigure $handle -blocking 0


#fileevent $handle readable [list ReadOutput $handle]

puts "I'm rewinding the tape now."
puts $handle "a"
flush $handle



vwait ::DONE
