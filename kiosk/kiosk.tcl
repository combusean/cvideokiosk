#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"

package require Gnocl 
package require mysqltcl 
package require csv 
package require Tclx
package require control
control::control assert enabled 1

# Setting plugs w/ setctl-p2p.sh
exec /usr/local/kiosk/setctl-p2p.sh &

set ::m [mysqlconnect -user root -password ###### -db cvk]

set deckguid(1) 0x0000850001399440

set deckguid(2) 0x00008500013caa0c

set kiosk_home "/usr/local/kiosk"

# don't change the next line
set dvgraboptions " -i --format dv2 --opendml --size 0"

 namespace eval control {
    namespace export waitForAny
    variable waitForAnyKey 0

    # new "vwait" that takes multiple variables and/or optional timeout
    # usage:  waitForAny ?timeout? variable ?variable ...?
    proc waitForAny {args} {
       variable waitForAnyArray
       variable waitForAnyKey

	set timeout 0

       # if first arg is a number, then that is max wait time
       #if {[string is int [lindex $args 0]]} {
       #   set timeout [lindex $args 0]
       #   set args [lrange $args 1 end]
       #}

       # create trigger script that will cause vwait to fall thru
       # (trailing comment is to eat appended args in trace command)
       set index "Key[incr waitForAnyKey]"
       set trigger "[namespace code [list set waitForAnyArray($index) 1]] ;#"

       # create trace to trip trigger
       foreach var $args {
          uplevel \#0 [list trace variable $var w $trigger]
      }

       # set timer is user requested one
       #if {$timeout > 0} {
       #   set timerId [after $timeout $trigger]
       #}
       vwait [namespace which -variable waitForAnyArray]($index)

       # remove all traces
       foreach var $args {
          uplevel \#0 [list trace vdelete $var w $trigger]
       }

       # cancel timer
       #if {$timeout > 0} {
       #   after cancel $timerId
       #}
       # cleanup
       unset waitForAnyArray($index)
    }
 }
namespace import control::waitForAny 


proc loadBox {listvariable sql} \
 {	global m
	set result [::mysql::query $m $sql]
	if {[::mysql::result $result rows] > 0} \
	 {	set newList {{Select -1}}
		while {[set row [::mysql::fetch $result]]!=""} \
		 {	lappend {newList} $row
		 }
		::mysql::endquery $result
		$listvariable configure -itemList $newList
	 }
 }

proc fputs {f str} \
 {	puts $f $str
	flush $f
 }

proc readEncoderOutput {encHandle} \
 {	set status [catch { gets $encHandle line } result]
	if {$result >= 0} \
	 {	puts "the encoder says $line"
		set ecline [split $line " "]
		if {[llength $ecline] == 2} \
		 {	set efilename [lindex $ecline 0]
			set etime [lindex $ecline 1]
			if {$etime >= 1} \
			 {	set etext "Finished encoding $efilename"
				set etime 0.0
			 } else \
			 {	set etext "Encoding $efilename"
			 }		

			$::eprogress configure -text $etext -fraction $etime
		 }
	 }
 }

proc close_deck {deckID} \
 {	deckCommand $deckID "q" 1 0
	catch {close [set ::deckcontrol($deckID)]}
	catch {close [set ::deckstatus($deckID)]}
	$::ccbutton($deckID) configure -sensitive 0
	#$::fbutton($deckID) configure -sensitive 0
	$::deckicon($deckID) configure -image "%/$::kiosk_home/pixmaps/no_tape.png"
	$::deckprogress($deckID) configure -text "Offline" -fraction 0.0
	set ::deckdw($deckID) "Offline"
	puts "deck_closed"
	return
 }

proc readDvOutput {deckID dvhandle} \
 {	set status [catch { gets $dvhandle line } result]
	if { $status != 0 } \
	 {	# Error on the channel
		puts "error result: $result"
	 } elseif { $result >= 0} \
	 {	if {$::terminated($deckID) == 1} \
		 {	return
		 }

		# sometimes dvgrab dumps out wide whitelines, maybe have to check buffering, but they dont contain data so dump them		
		if {$line == "Winding Stopped"} \
		 {	set ::ws($deckID) 1
			return
		 }

		if {[string length $line] == 80} \
		 {	return
		 }
		# Successfully read the channel
		set deckLine [csv::split $line " "]
			
		#puts $deckLine

		puts $line

		set doingWhat [lindex $deckLine 0]

		set deckTime [lindex $deckLine 1]

		# trim out "centiseconds" ...
		set deckTime [string range $deckTime 0 7]
		
		set deckPct 0.0
	
		if {[scan $deckTime "%d:%d:%d" hours minutes seconds] == 3} \
		 {	set deckPct [expr (($hours * 3600) + ($minutes * 60) + $seconds) / 5040.0]
		 } 
		
		if {[string first ".avi" $doingWhat] > 0} \
		 {	incr ::scenecount($deckID)
			if {$::scenecount($deckID) > 20} \
			 {	set ::terminated($deckID) 1
				close_deck $deckID
				set diag [gnocl::dialog -type error -text "Too many scene changes detected on Deck $deckID"]

				return
	
			 }
		 }	

		puts "deck $deckID is $doingWhat at time $deckTime"
		set icon "%/$::kiosk_home/pixmaps/unknown.png"

		switch $doingWhat \
		 {	"Playing Paused"	{	set icon "%/$::kiosk_home/pixmaps/playing_paused.png"
							set ::playingpaused($deckID) 1
						}
			"Playing" 		{	set icon "%/$::kiosk_home/pixmaps/play.png"
							set ::playing($deckID) 1
							if {$deckTime == "ff:ff:ff:ff" && $::playing($deckID) == 1} \
							 {	set ::playingdone($deckID) 1
							 }
						}
			"Winding stopped"	{	set icon "%/$::kiosk_home/pixmaps/stopped.png"
						}
			"Winding forward"	{	set icon "%/$::kiosk_home/pixmaps/fast_forward.png"
							set ::ff($deckID) 1
						}
			"Playing Fast Forward"	{	set icon "%/$::kiosk_home/pixmaps/fast_forward.png"
							set ::ff($deckID) 1
						}
			"Winding reverse"	{	set icon "%/$::kiosk_home/pixmaps/rewind.png"
						}
			"Winding" 		{	set icon "%/$::kiosk_home/pixmaps/winding.png"
						}
		 }

		$::deckprogress($deckID) configure -text "[set ::deckfile($deckID)]: $deckTime" -fraction $deckPct
		$::deckicon($deckID) configure -image $icon
		set ::deckdw($deckID) $doingWhat

	 } elseif {[eof $dvhandle]} \
	 {	close_deck $deckID
	 }
 }



proc VideoSearch {whatToReturn year month day grant_id project_id event_id group_id session_number position tape_number} \
 {	global m

	set wSQL "1"

	if {$year > -1} \
	 { set wSQL "$wSQL AND DATE_FORMAT(video.tape_date, '%Y') = '$year' "
	 } 

	if {$month > -1} \
	 {	set wSQL "$wSQL AND DATE_FORMAT(video.tape_date, '%m') = '$month' "
	 }
	
	if {$day > -1} \
	 {	set wSQL "$wSQL AND DATE_FORMAT(video.tape_date, '%d') = '$day' "
 	 }

	if {$grant_id > 0} \
	 { 	set wSQL "$wSQL AND video.grant_id = '$grant_id' "
	 }
	
	if {$project_id > 0} \
	 {	set wSQL "$wSQL AND video.project_id = '$project_id' "
 	 }

	if {$event_id > 0} \
	 {	set wSQL "$wSQL AND video.event_id = '$event_id' "
	 }
	
	if {$group_id > 0} \
	 {	set wSQL "$wSQL AND video.group_id = '$group_id' "
	 }

	if {$session_number > -1} \
	 {	set wSQL "$wSQL AND session_number = '$session_number' "
	 }

	if {$position != -1} \
	 {	set wSQL "$wSQL AND position = '$position' "
 	 }
	
	if {$tape_number != -1} \
	 {	set tape_number "$wSQL AND tape_number = '$tape_number' "
	 }

	set svSQL "SELECT grant.grant_name AS grant_name, project.project_name AS project_name, event.event_name AS event_name, group.group_name AS group_name,
							video.sequence_number AS sequence_number, video.tape_date AS tape_date,
							video.session_number AS session_number, video.position AS position, video.tape_number AS tape_number,
							video.notes AS notes, video.encode_time AS encode_time, video.creation_time AS creation_time
						FROM video
						LEFT JOIN `grant` ON (`grant`.grant_id = video.grant_id)
						LEFT JOIN project ON (project.project_id = video.project_id)
						LEFT JOIN event ON (event.event_id = video.event_id)
						LEFT JOIN `group` ON (group.group_id = video.group_id)
						WHERE $wSQL"

	set result [mysql::query $m $svSQL]
	set numRows [::mysql::result $result rows]
	if {$whatToReturn == "count"} \
	 {	return $numRows
	 } else \
	 {	::mysql::endquery $result
		set sList [mysql::sel $m $svSQL -list]
                set searchList [gnocl::list -titles {"Grant" "Project" "Event" "Group" "Sequence" "Date" "Session" "Position" "Tape Num" "Notes" "Creation Time" "Encode Time"} \
                                                -types {string string string string integer string integer string integer string string string}]

		$searchList add $sList
		return $searchList

	 }
 }

proc printLabel {video_id} \
 {	global m
	set svlSQL "SELECT grant_bc_name AS grant_bc_name, project.project_bc_name AS project_bc_name, event.event_bc_name AS event_bc_name,
			group.group_bc_name AS group_bc_name, video.sequence_number AS sequence_number,
			DATE_FORMAT(video.tape_date, '%y%m%d') AS tape_date,
			video.session_number AS session_number, video.position AS position, video.tape_number AS tape_number, video.notes AS notes
			FROM video
			LEFT JOIN `grant` ON (`grant`.grant_id = video.grant_id)
			LEFT JOIN project ON (project.project_id = video.project_id)
			LEFT JOIN event ON (event.event_id = video.event_id)
			LEFT JOIN `group` ON (`group`.group_id = video.group_id)
			WHERE video.video_id = '$video_id'";
	set result [mysql::query $m $svlSQL]
	set row [mysql::fetch $result]
	set barcode ""
	if {[string length [lindex $row 0]] > 0} \
	 {	set barcode [concat $barcode[lindex $row 0].]
		if {[string length [lindex $row 1]] > 0} \
		 {	set barcode [concat $barcode[lindex $row 1].]
			if {[string length [lindex $row 2]] > 0} \
			 {	set barcode [concat $barcode[lindex $row 2].]
				if {[string length [lindex $row 3]] > 0} \
				 {	set barcode [concat $barcode[lindex $row 3].]
				 }
			 } 
	 	 }
	 }
	set barcode [concat $barcode[lindex $row 4]]
	set tape_date [lindex $row 5]
	set session_number [lindex $row 6]
	set position [lindex $row 7]
	set tape_number [lindex $row 8]
	set notes [string map {" " "\ "} [lindex $row 9]]
	exec /usr/local/kiosk/barcode_generator/cvkbcgen.php barcode=$barcode tape_date=$tape_date session_number=$session_number tape_number=$tape_number position=$position notes=$notes
	exec lp -o landscape -o scaling=100 /tmp/cvk_tp_$barcode.png
	exec rm /tmp/cvk_tp_$barcode.png
	exec rm /tmp/cvk_bc_$barcode.png
 }

proc get_single_id {year month day grant_id project_id event_id group_id session_number position tape_number} \
 {	global m
	return [lindex [mysql::sel $m "SELECT video_id
					FROM video
					WHERE tape_date = '$year-$month-$day' AND
						grant_id = '$grant_id' AND
						project_id = '$project_id' AND
						event_id = '$event_id' AND
						group_id = '$group_id' AND
						session_number = '$session_number' AND
						position = '$position' AND
						tape_number = '$tape_number'" -flatlist] 0]
			
 }


proc updateSaveLabel {initial_video_id year month day grant_id project_id event_id group_id session_number position tape_number notes} \
 {	global m

	if { $year == -1 || $month == -1 || $day == -1 || $day == -1 || $grant_id == -1 || $project_id == -1 || $event_id == -1 || $group_id == -1 || $session_number == -1 || $position == -1 || $tape_number == -1} \
	 {	set diag [gnocl::dialog -type error -text "All dropdowns must be filled."]
		return -1
	 }
	set newSequence false

	set wSQL ""
	
	puts "A-2" 
	puts $initial_video_id
			

	if {$initial_video_id == 0} \
	 {	set current_video_id [get_single_id $year $month $day $grant_id $project_id $event_id $group_id $session_number $position $tape_number]
	
		puts "A-1" 
		puts $current_video_id

		if {$current_video_id < 1} \
		 {	set iuSQL "INSERT INTO "
			set newSequence true
		 } else \
		 {	set diag [gnocl::dialog -type error -text "A label already exists with the parameters you have specified."]
			return -1
		 }
	 } else \
	 {	set iuSQL "UPDATE"
		set wSQL "WHERE video_id = '$initial_video_id'";
		set newSequence false
	 }
	
	 mysql::query $m "$iuSQL video
				SET tape_date = '$year-$month-$day',
					grant_id = '$grant_id',
					project_id = '$project_id',
					event_id = '$event_id',
					group_id = '$group_id',
					session_number = '$session_number',
					position = '$position',
					tape_number = '$tape_number',
					creation_time = NOW(),
					notes = '$notes'
				$wSQL"

	if {$iuSQL == "INSERT INTO "} \
	 {	#puts "A"
		set new_video_id [mysql::insertid $m]

		#puts "B"
		puts $new_video_id
		if {$newSequence == true} \
		 { 	set sqcount [mysql::sel $m "SELECT *
							FROM video
							WHERE grant_id = '$grant_id' AND
								project_id = '$project_id' AND
								event_id = '$event_id' AND
								group_id = '$group_id'"]
			mysql::exec $m "UPDATE video
					SET sequence_number = '$sqcount'
					WHERE video_id = '$new_video_id'"
		 }
		return $new_video_id
	 } else \
	 {	return $initial_video_id
	 }
 }

proc deckCommand {deckID command aftertime dowevwait} \
 {	puts "deck $deckID is going to $command"

	if {$::deckdw($deckID) == "Offline" || $::terminated($deckID) == 1} \
	 {	if {$command != "q"} \
		 {	puts $::deckdw($deckID)
			puts $::terminated($deckID)
			puts "terminated, leaving"
			return
		 }
	 }

	if {$dowevwait == 1} \
	 {	if {$command == "z"} \
		 {	catch {unset ::ff($deckID)}
		 } elseif {$command == "c"} \
		 {	catch {unset ::playingdone($deckID)}
		 } else \
		 {	catch {unset ::done($deckID)}
		 }
	 }

	if {$aftertime == 1} \
	 {	catch {after 500 [fputs [set ::deckcontrol($deckID)] $command]}
		puts "waited 500"
	 } else \
	 {	catch {fputs $::deckcontrol($deckID) $command}
	 }
	
	if {$dowevwait == 1} \
	 {	if {$command == "z"} \
		 {	control::waitForAny ::terminated($deckID) ::ff($deckID)
		 } elseif {$command == "c" || $command == "p"} \
		 {	control::waitForAny ::terminated($deckID) ::playingdone($deckID) 
		 } else \
		 {	control::waitForAny ::terminated($deckID) ::ws($deckID)
		 }
	 }
 }

proc captureVideo {} \
 {	global m
	puts "beep beep"
	set diag "Retry"
	set ::barcode ""
	set pauser 0
	set adeck 0

	while {$diag == "Retry"} \
	 {	set adeck 0
		foreach deckID [array names ::deckguid] \
		 {	if {$::deckdw($deckID) == "Offline"} \
			 {	set adeck $deckID
				set diag "OK"
				break
			 }	
		 }
		if {$adeck == 0} \
		 {	set diag [gnocl::dialog -type error -text "Decks are busy" -buttons "%#Retry %#Cancel"]
			if {$diag == "Cancel"} \
			 {	return
			 }

		 }
	 }

	set cvTable [gnocl::table]
	set cvWindow [gnocl::window -visible 0 -x 600 -y 500 -child $cvTable -title "Barcode Entry"]
	
	set ::cvdiag "wtf"

	$cvTable add [gnocl::label -text "Please scan the tape's barcode"] 0 0 -columnSpan 2
	$cvTable add [gnocl::entry -variable barcode -widthChars 20 -onActivate {puts "really, wtf"; set ::cvdiag "Ok"; set pauser 1}] 0 1 -columnSpan 2
	$cvTable add [gnocl::button -text "%#Cancel" -onClicked {set ::cvdiag "Cancel"; set pauser 1}] 0 2
	$cvTable add [gnocl::button -text "%#Ok" -onClicked {set ::cvdiag "Ok"; set pauser 1}] 1 2
	$cvWindow configure -visible 1
	
	vwait pauser
	$cvWindow delete

	puts $::cvdiag

	set ::barcode [string toupper $::barcode]

	#M.CPCM3.INT.09
	set bc [split $::barcode "."]

	if {[llength $bc] > 1 && $::cvdiag == "Ok"} \
	 {	set diag [gnocl::dialog -type info -text "Please insert tape with barcode $::barcode into Deck $adeck and click OK." -buttons "%#Cancel %#Ok"]
		if {$diag == "Ok"} \
		 {	foreach file [glob -nocomplain /tmp/cvk_${::barcode}*] \
			 {	exec rm $file
				puts "Removing existing $file in tmp for $::barcode ... check to see if the previous session crashed."
			 }

			puts "home now"
			set jSQL ""
			set wSQL "WHERE 1 "
			set grant_bc_name ""
			set project_bc_name ""
			set event_bc_name ""
			set group_bc_name ""
		
			set numparts [llength $bc] 

			switch $numparts \
			 {	2	{	set grant_bc_name [lindex $bc 0]
						set sequence_number [lindex $bc 1]
					}	
				3	{	set grant_bc_name [lindex $bc 0]
						set project_bc_name [lindex $bc 1]
						set sequence_number [lindex $bc 2]
					}
				4	{	set grant_bc_name [lindex $bc 0]
						set project_bc_name [lindex $bc 1]
						set event_bc_name [lindex $bc 2]
						set sequence_number [lindex $bc 3]
					}
				5	{	set grant_bc_name [lindex $bc 0]
						set project_bc_name [lindex $bc 1]
						set event_bc_name [lindex $bc 2]
						set group_bc_name [lindex $bc 3]
						set sequence_number [lindex $bc 4]
					}
			
			 }
			if {[string length $grant_bc_name] > 0} \
			 {	set jSQL "$jSQL LEFT JOIN `grant` ON (`grant`.grant_id = video.grant_id) "
				set wSQL " $wSQL AND `grant`.grant_bc_name = '$grant_bc_name' "
			 }
			if {[string length $project_bc_name] > 0} \
			 {	set jSQL "$jSQL LEFT JOIN project ON (project.project_id = video.project_id) "
				set wSQL "$wSQL AND project.project_bc_name = '$project_bc_name' "
			 }
			if {[string length $event_bc_name] > 0} \
			 {	set jSQL "$jSQL LEFT JOIN event ON (event.event_id = video.event_id) "
				set wSQL "$wSQL AND event.event_bc_name = '$event_bc_name' "
			 }
			if {[string length $group_bc_name] > 0} \
			 {	set jSQL "$jSQL LEFT JOIN `group` ON (`group`.group_id = video.group_id) "
				set wSQL "$wSQL AND `group`.group_bc_name = '$group_bc_name' "
			 }
			set wSQL "$wSQL AND sequence_number = '$sequence_number' "

			set selSQL "SELECT video_id, DATE_FORMAT(video.tape_date, '%y%m%d') AS tape_date
					FROM video
					$jSQL
					$wSQL"

			set row [mysql::sel $m $selSQL -flatlist] 

			set video_id [lindex $row 0]
			set tapedate [lindex $row 1]

			if {$video_id < 1} \
			 {	set diag [gnocl::dialog -type error -text "Couldn't find $::barcode in database.  Checked the barcode displayed with that on the tape.  If it matches, please tell somebody, otherwise, you may rescan the tape."]
				return -1
			 }

			puts "Capturing $video_id"

			set ::terminated($adeck) 0
			set ::scenecount($adeck) 0
			set ::deckdw($adeck) "Online"

			catch {close $::deckcontrol($adeck)}
			catch {close $::deckstatus($adeck)}

			set ::deckstatus($adeck) [open /tmp/cvk_dvgrab_${adeck} {RDONLY NONBLOCK}]
			fileevent $::deckstatus($adeck) readable "readDvOutput $adeck $::deckstatus($deckID)"

			foreach file [glob -nocomplain /tmp/cvk_dv_deck${adeck}*.avi] \
			 {	exec rm $file
				puts "Removing existing Deck $adeck AVI $file for $::barcode ... check to see if the previous session crashed."
			 }
			$::ccbutton($adeck) configure -sensitive 1
			#$::fbutton($adeck) configure -sensitive 1

			set ::deckfile($adeck) $::barcode

			set ::deckcontrol($adeck) [open "|/usr/local/bin/dvgrab $::dvgraboptions --guid [set ::deckguid($adeck)]  /tmp/cvk_dv_deck${adeck} 2>/tmp/cvk_dvgrab_${adeck}" "w"]
			deckCommand $adeck "p" 1 0
			#deckCommand $adeck "z" 0 1
			deckCommand $adeck "\x1b" 0 0
			deckCommand $adeck "\x1b" 0 0
			deckCommand $adeck "a" 0 1
			deckCommand $adeck "c" 0 1
			deckCommand $adeck "\x1b" 0 0
			deckCommand $adeck "\x1b" 1 1
			deckCommand $adeck "a" 0 1
			close_deck $adeck

			if {$::terminated($adeck) == 0} \
			 {	set i 1
				foreach file [glob -nocomplain /tmp/cvk_dv_deck${adeck}*.avi] \
				 {	set targetfile /tmp/cvk_dv_${tapedate}_[set ::deckfile($adeck)]_scene${i}.avi
					exec mv $file $targetfile
					fputs $::encodercontrol "$targetfile $video_id" 
					incr i
				 }
			 }
		 }
 	 } 
 }

proc ui_dropdowns {} \
 {	global m

	set uiTable [gnocl::table]
	
	$uiTable add [gnocl::label -align left -text {%<<span size="large" weight="bold">Describe the video using the pulldowns below.</span>}] 0 0 -columnSpan 10 -align left -expand 0 -fill 0

	set j 0

	#		0 1 2 3 4 5 6 7 8 9
	foreach lbl {"Grant" "Project" "Event" "Group" "Year" "Month" "Day" "Session" "Position" "Tape Num"} \
	 {	$uiTable add [gnocl::label -text $lbl] $j 1
		incr j
	 }

	set grantList {{Select "-1"}}
	set defList {{"N/A" "0"}}
	#set defList {{Select "-1"}} set def2List [list {{Select "-1"}}]
	foreach grant [mysqlsel $m {SELECT grant_name, grant_id FROM `grant` ORDER BY grant_name} -list] \
	 {	lappend grantList $grant
	 }

	set uiGroupColumn [gnocl::comboBox -variable uiGroup -itemList $defList]

	set uiEventColumn [gnocl::comboBox -variable uiEvent -itemList $defList \
						-onChanged "$uiGroupColumn configure -itemList {$defList};
								loadBox $uiGroupColumn {SELECT group_name, group_id FROM `group` WHERE event_id = '%v'};"]

	set uiProjectColumn [gnocl::comboBox -variable uiProject -itemList $defList \
						 -onChanged "$uiEventColumn configure -itemList {$defList};
								$uiGroupColumn configure -itemList {$defList};
								loadBox $uiEventColumn {SELECT event_name, event_id FROM `event` WHERE project_id = '%v'};"]

	set uiGrantColumn [gnocl::comboBox -variable uiGrant -itemList $grantList \
				-onChanged "$uiGroupColumn configure -itemList {$defList};
						$uiEventColumn configure -itemList {$defList};
						$uiProjectColumn configure -itemList {$defList};
						loadBox $uiProjectColumn {SELECT project_name, project_id FROM project WHERE grant_id = '%v'};"]
	
	$uiTable add $uiGrantColumn 0 2 -fill 0

	$uiTable add $uiProjectColumn 1 2 -fill 0

	$uiTable add $uiEventColumn 2 2 -fill 0

	$uiTable add $uiGroupColumn 3 2 -fill 0

	$uiTable add [gnocl::comboBox -variable uiYear -itemList [list {"Select" "-1"} {2004 2004} {2005 2005} {2006 2006} {2007 2007}]] 4 2 -fill 0

	$uiTable add [gnocl::comboBox -variable uiMonth -itemList [list {"Select" "-1"} {"January" "01"} {"February" "02"} {"March" "03"} {"April" "04"} {"May" "05"} \
										{"June" "06"} {"July" "07"} {"August" "08"} {"September" "09"} {"October" "10"} \
										{"November" "11"} {"December" "12"}]] 5 2 -fill 0

	set uiDayColumn [gnocl::comboBox -variable uiDay ]

	lappend dayList [list "Select" ""]

	for {set i 1} {$i <= 31} {incr i} \
	 {	set iFormatted [format "%02d" $i]

		lappend dayList [list $i $iFormatted]
	 }									

	$uiDayColumn configure -itemList $dayList
	$uiTable add $uiDayColumn 6 2 -fill 0

	set uiSession 1
	set uiPosition F

	$uiTable add [gnocl::comboBox -variable uiSession -itemList [list {"Select" "-1"} {"N/A" "0"} {"1" "01"} {"2" "02"} {"3" "03"} {"4" "04"} {"5" "05"} {"6" "06"} {"7" "07"} {"8" "08"} {"9" "09"} {"10" "10"} {"11" "11"} {"12" "12"} {"13" "13"} {"14" "14"} {"15" "15"}]] 7 2 -fill 0

	$uiTable add [gnocl::comboBox -variable uiPosition -itemList [list {"Select" "-1"} {"Front" "F"} {"Rear" "R"} {"N/A" "-"}]] 8 2 -fill 0

	$uiTable add [gnocl::comboBox -variable uiTapeNum -itemList [list {"Select" "-1"} {1 1} {2 2} {3 3} {4 4}]] 9 2 -fill 0
 
	return $uiTable
 }


proc CreateNewLabel {} \
 {
	set cnlTable [gnocl::table -label "Video details"]

	$cnlTable add [ui_dropdowns] 0 0

	$cnlTable add [gnocl::label -text "\n"] 0 1

	$cnlTable add [gnocl::label -align left -text {%<<span size="large" weight="bold">Optionally, you may enter notes that further describe the video.</span>}] 0 2 -align left -expand 0 -fill 0
	
	$cnlTable add [gnocl::entry -variable uiNotes -widthChars 60] 0 3 -expand 0 -fill 0 -align left

	return $cnlTable
 }

proc SearchCollection {} \
 {	set scTable [gnocl::table -label "Video details"]
	$scTable add [ui_dropdowns] 0 0

	return $scTable
 }

proc lm_startOver {} \
 {	$::lmHeader configure -text {%<<span size="x-large" weight="bold">1.  Welcome to the Label Manager</span>}
	$::button1 configure -text "Create New Label" -visible 1
	$::button2 configure -text "Search Collection" -visible 1
	$::SObutton configure -visible 0

	$::lmTable delete
	set ::lmTable [gnocl::table]
	$::lmSuperTable add $::lmTable 0 2
 }

proc ui_createpath {} \
 {	set b1text [$::button1 cget -text]
	switch $b1text \
	 { 	"Begin Search" { 		$::lmHeader configure -text {%<<span size="x-large" weight="bold">3.  Search Results</span>}
						$::lmTable delete
						set searchCount [VideoSearch "count" $::uiYear $::uiMonth $::uiDay $::uiGrant $::uiProject $::uiEvent $::uiGroup $::uiSession $::uiPosition $::uiTapeNum]
						if {$searchCount > 0} \
						 {	set ::lmTable [VideoSearch "ui" $::uiYear $::uiMonth $::uiDay $::uiGrant $::uiProject $::uiEvent $::uiGroup $::uiSession $::uiPosition $::uiTapeNum]
							$::lmSuperTable add $::lmTable 0 2
							$::button1 configure -visible 0
						 } else \
						 {	set ::lmTable [gnocl::label -text "No results found."]
							$::lmSuperTable add $::lmTable 0 2
							$::button1 configure -visible 0
						 }
					}

		"Save and Print"	{	set new_id [updateSaveLabel 0 $::uiYear $::uiMonth $::uiDay $::uiGrant $::uiProject $::uiEvent $::uiGroup $::uiSession $::uiPosition $::uiTapeNum $::uiNotes]
						#puts "D" puts $new_id
						if {$new_id > 0} \
						 {	printLabel $new_id
							#lm_startOver
						 }
					}
		"Create New Label"	{	$::lmHeader configure -text {%<<span size="x-large" weight="bold">2. Label Creator</span>}
						$::lmTable delete
						set ::lmTable [CreateNewLabel]
						$::lmSuperTable add $::lmTable 0 2
						$::button1 configure -text "Save and Print"
						$::button2 configure -text "Save"
						$::SObutton configure -visible 1
					}
	 }

 }

proc ui_searchpath {} \
 {	set b2text [$::button2 cget -text]
	
	switch $b2text \
	 {	"Search Collection"	{	$::lmHeader configure -text {%<<span size="x-large" weight="bold">2.  Collection Searcher</span>}
						$::lmTable delete
						set ::lmTable [SearchCollection]
						$::lmSuperTable add $::lmTable 0 2
						$::button1 configure -text "Begin Search"
						$::button2 configure -visible 0
						$::SObutton configure -visible 1
					}
		"Save" 			{	set new_id [updateSaveLabel 0 $::uiYear $::uiMonth $::uiDay $::uiGrant $::uiProject $::uiEvent $::uiGroup $::uiSession $::uiPosition $::uiTapeNum $::uiNotes]
						if {$new_id > 0} \
						 {	lm_startOver
						 }
					}

	 }
 }


proc labelManager {} \
 {	global m

	set ::lmSuperTable [gnocl::table]

	set ::lmHeader [gnocl::label -align left -text {%<<span size="x-large" weight="bold">1.  Welcome to the Label Manager</span>}]

	set ::lmTable [gnocl::table]

	set buttonBox [gnocl::box -orientation horizontal]

	set ::button1 [gnocl::button -text "Create New Label" -onClicked "ui_createpath"]
	set ::button2 [gnocl::button -text "Search Collection" -onClicked "ui_searchpath"]
	set ::SObutton [gnocl::button -text "Start Over" -visible 0 -onClicked lm_startOver]

	$buttonBox add $::button1
	$buttonBox add $::button2
	$buttonBox add $::SObutton

	$::lmSuperTable add $::lmHeader 0 1
	$::lmSuperTable add $::lmTable 0 2
	$::lmSuperTable add $buttonBox 0 3

	#$plBox add [gnocl::button -text "Print Label" -onClicked {handleLabel "print" $::plYear $::plMonth $::plDay $::plGrant $::plProject $::plEvent $::plGroup 
	#$::plSession $::plPosition $::plTapeNum}] -expand 0 -fill 0 -align left $lmBox add [gnocl::button -text "Search Video" -onClicked {handleLabel "search" $::plYear 
	#$::uiMonth $::uiDay $::uiGrant $::uiProject $::uiEvent $::uiGroup $::uiSession $::uiPosition $::uiTapeNum}] -expand 0 -fill 0 -align left

	return $::lmSuperTable
 }

set mainBox [gnocl::box -orientation vertical]


set headerLabel [gnocl::label -align left -text {%<<span size="xx-large">CRESMET Video Kiosk</span>}]

set table [gnocl::table -homogeneous 0 -label "What do you want to do?" -labelAlign left]

set labelManagerButton [gnocl::button]

set labelBox [labelManager]

$labelBox configure -visible 0

$table add $labelBox 0 1

# programmed before I knew about the expander widget, but i kinda like it anyway
$labelManagerButton configure -text "Enter Label Manager" -onClicked \
 {	if {[$labelManagerButton cget -text] == "Enter Label Manager"} \
	 {	$labelBox configure -visible 1
		$labelManagerButton configure -text "Close Label Manager"
	 } else \
	 {	$labelBox configure -visible 0
		$labelManagerButton configure -text "Enter Label Manager"
	 }
 }

$table add $labelManagerButton 0 0 -expand 0 -fill 0 -align left

$table add [gnocl::button -text "Capture Video" -onClicked {captureVideo}] 0 2 -expand 0 -fill 0 -align left

set barbox [gnocl::box -orientation vertical]

proc killdeck deckID \
 {	puts "Terminated $deckID"
	set ::terminated($deckID) 1
	puts "Closing $deckID"
	close_deck $deckID
 }
set ::ccbutton(1) [gnocl::button -text "Cancel" -sensitive 0 -onClicked {killdeck 1}]
set ::ccbutton(2) [gnocl::button -text "Cancel" -sensitive 0 -onClicked {killdeck 2}]


#deckID command aftertime dowevwait
#set ::fbutton(1) [gnocl::button -text "Finish" -sensitive 0 -onClicked {deckCommand 1 "q" 0 0 }]
#set ::fbutton(2) [gnocl::button -text "Finish" -sensitive 0 -onClicked {deckCommand 2 "q" 0 0 }]

foreach deckID [array names ::deckguid] \
 {	set deckdw($deckID) "Offline"
	catch {exec rm /tmp/cvk_dvgrab_${deckID}}
	exec mkfifo --mode=+rwxrwxrwx /tmp/cvk_dvgrab_${deckID}

	set decktable($deckID) [gnocl::table -homogeneous 0 -label "Deck $deckID status"]

	set ::deckicon($deckID) [gnocl::image -image "%/$kiosk_home/pixmaps/no_tape.png"]
	set ::deckprogress($deckID) [gnocl::progressBar -activityMode 0 -text "Offline"]

	$decktable($deckID) add $::ccbutton($deckID) 0 0 -expand 0 -fill 0
	#$decktable($deckID) add $::fbutton($deckID) 1 0 -expand 0 -fill 0 
	$decktable($deckID) add $::deckicon($deckID) 2 0 -expand 0 -fill 1
	$decktable($deckID) add $::deckprogress($deckID) 3 0 -expand 1 -fill 1
	
	$barbox add $decktable($deckID)
	
 }	


set eprogress [gnocl::progressBar -activityMode 0]
catch {exec rm /tmp/cvk_encoder}
exec mkfifo --mode=+rwxrwxrwx /tmp/cvk_encoder
set encodestatus [open /tmp/cvk_encoder {RDONLY NONBLOCK}]
fileevent $encodestatus readable "readEncoderOutput $encodestatus"
set encodercontrol [open "|$kiosk_home/encode_control.sh >/tmp/cvk_encoder" "w"]
set estatustable [gnocl::table -homogeneous 0 -label "Encoder Status"]
$estatustable add $eprogress 1 0 -expand 1 -fill 1

$barbox add $estatustable

$mainBox add [list $headerLabel $table]
$mainBox add $barbox -align bottom -expand 1

gnocl::window -title "Video Kiosk" -widthRequest 1280 -heightRequest 1024 -child $mainBox -onDestroy exit

gnocl::mainLoop
