#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Tclx
package require mysqltcl

set ::m [mysqlconnect -user root -password cr3sm3t -db cvk]

set ::queue [list]

proc fputs {channel data} \
 {	puts $channel $data
	flush $channel
 }

proc parseStatus {ffhandle} \
 {	global m
	set status [catch { gets $ffhandle line } result]
	if { $result >= 0} \
         {      # sometimes dvgrab dumps out wide whitelines, maybe have to check buffering, but they dont contain data so dump them
		
		#frame= 3812 q=2.0 Lsize=   12377kB time=127.1 bitrate= 797.9kbits/s
		#frame=  280 q=2.0 Lsize=     686kB time=9.2 bitrate= 609.9kbits/s
		#puts "Wtf2"
		#puts $line

		fputs $::fflog $line

		#puts "ffmpeg says $line"

		if {[string range $line 2 9] == "Duration"} \
		 {	set line [string trim $line]
			set ::filetime [lindex [lindex [split $line ","] 0] 1]
			return
		 }

		if {[string range $line 0 5] == "video:"} \
		 {	
			fputs $::fflog "making /var/www/video/${::tapedate}_${::barcode}"
			catch {exec mkdir /var/www/video/${::tapedate}_${::barcode}}

			puts "${::barcode}_scene${::scene} 1.0"

			fputs $::fflog "moving $::output to /var/www/video/${::tapedate}_${::barcode}/${::tapedate}_${::barcode}_scene${::scene}.mp4"
			catch {exec mv $::output /var/www/video/${::tapedate}_${::barcode}/${::tapedate}_${::barcode}_scene${::scene}.mp4}

			fputs $::fflog "rm'ing $::filename"
	
			catch {exec rm $::filename} 

			fputs $::fflog "updating $::video_id"

			::mysql::exec $m "UPDATE video SET encode_time = NOW() WHERE video_id = '$::video_id'"
			set ::turn [expr $::turn - 1]
			set ::done 1
			fputs $::fflog "finished"
			catch {close $ffhandle}
			return

		 }

                if {[string range $line 0 4] != "frame"} \
                 {      #puts f$line
			return
                 }


		set time [lindex [split [string range $line [expr [string first "time=" $line] + 5] end] " "] 0]
	
		#puts $time

		#puts "scanning $::filetime"

		if {[scan $::filetime "%d:%d:%f" hours minutes seconds] == 3} \
                 {      set outPct [expr $time / ($seconds + ($hours * 3600) + ($minutes * 60) )]
			puts "${::barcode}_scene${::scene} $outPct"

			fputs $::fflog "duration:  $::filetime ${::barcode}_scene${::scene} op:  $outPct"
		 }
			
	 } elseif { [eof $ffhandle] } {
		catch {close $ffhandle}
                # End of file on the channel
		#puts "closed"
		#
		
		#

		#	


        } elseif { [fblocked $ffhandle] } {
                #puts "blocked!"
                # Read blocked.  Just return
        } else {
                # Something else
                #puts "can't happen"
        }


 }

proc step_queue {} \
 {	#puts "not busy, stepping the queue"
	set ::done 0

	set fileline [lvarpop ::queue]
	fputs $::fflog "got a $fileline"

	catch {close $::ffhandle}
	set ::ffhandle [open "/tmp/cvk_encoder_ff" {RDONLY NONBLOCK}]
	fileevent $::ffhandle readable "parseStatus $::ffhandle"

	set fileline [split $fileline " "]
	set ::filename [lindex $fileline 0]
	set ::video_id [lindex $fileline 1]


	set ::tapedate [lindex [split $::filename "_"] 2]
	set ::barcode [lindex [split $::filename "_"] 3]
	set ::scene [string map {"scene" "" ".avi" ""} [lindex [split $::filename "_"] 4]]
			
	#set ::title  [string map {".avi" "" "/tmp/cvk_dv_" ""} $::filename]
			
	set ::output [string map {".avi" ".mp4"} $::filename]
		
	fputs $::fflog "starting ffmpeg with $::filename (video id : $::video_id) writing to $::output"
	exec /usr/bin/ffmpeg -i $::filename -acodec aac -ab 128 -vcodec mpeg4 -r 15 -mbd 2 -flags +4mv+trell -aic 2 -cmp 2 -subcmp 2 -s 320x240 $::output 2> /tmp/cvk_encoder_ff &
 }

proc enqueuer {} \
 {	set status [catch { gets stdin line } result]
        if {$result >= 0} \
	 {	set mynumber $::turn
		incr ::turn

		fputs $::fflog "got $line"
		if {[llength [split $line " "]] == 2} \
		 {	lappend ::queue $line
			fputs $::fflog "queue is now $::queue"
			if {$::done == 0} \
			 {	fputs $::fflog "my number is $mynumber"
				while {$::turn != $mynumber} \
				 {	fputs $::fflog "::turn for $mynumber is $::turn"
					vwait ::done
				 }
				step_queue
			 } else \
			 {	step_queue	

			 }	
		 }
	 }
 } 

catch {exec rm /tmp/cvk_encoder_ff}

exec mkfifo --mode=+rwxrwxrwx /tmp/cvk_encoder_ff

set ::queue {}
set ::done 1
set ::turn 0

set fflog [open "/tmp/cvk_ffmpeg_log" "w"]

fileevent stdin readable enqueuer

vwait forever
