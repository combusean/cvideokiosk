#!/bin/sh
# the next line restarts using tclsh \
exec tclsh "$0" "$@"


# 
#  this is a quick and dirty script to convert a text description of
#  the gnocl commands to html man pages
# 
#  Copyright (c) 2001 - 2003 P.G. Baum
# 
#  See the file "license.terms" for information on usage and redistribution
#  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
# 

#  History
#  2002-08: Move from html pages to DocBook refentrys
#  2001-06: Begin of developement
# 

proc getLine { fp {skipEmpty 0} } {
   # skip comments with "#" as first character
   while { [set ret [gets $fp ::lineNT]] >= 0 \
         && ( [string index $::lineNT 0] == "#" \
            || ( $skipEmpty && \
                  [string length [string trim $::lineNT]] == 0 ) ) } {
      ; # do nothing
   }

   set ::line [string trim $::lineNT]

   # puts "in getLine: $::line"
   return $ret
}

proc handleShortDescr { fp command } {
   upvar $command cmd
   # .SHORTDESCR:
   set cmd(shortDescr) [string trim [string range $::line 13 end]]
   return [getLine $fp 1]
}

proc handleSynopsis { fp command } {
   upvar $command cmd
   # .SYNOPSIS:
   set cmd(synopsis) [string trim [string range $::line 10 end]]
   return [getLine $fp 1]
}

proc subOptions { fp } {
   # optionName; type; default
   # description\n...
   #
   # optionName; type; default
   # description\n...
   #
   # .QQQ:
   set opts ""

   # loop over all options
   while { [set ret [getLine $fp 1]] > 0 \
         && [string index $::line 0] != "." } {
      foreach {optName type default descr} {"" "" "" ""} break
      foreach {optName type default} [split $::line ";"] break

      # loop over description
      while { [set ret [getLine $fp 0]] > 0 \
            && [string index $::line 0] != "." } {
         append descr $::line "\n"
      }
      lappend opts [list [string trim $optName] [string trim $type] \
            [string trim $default] [string trim $descr]]
   }
   puts "last: $::line"
   return $opts
}

proc handleOptions { fp command } {
   ## upvar $ll line
   upvar $command cmd

   set cmd(options) [subOptions $fp]
   ## # new section start with an "."
   ## while { [set ret [getLine $fp 1]] > 0 \
   ##       && [string index $::line 0] != "." } {
   ##    set opt $::line
   ##    # after the options comes the description til the first empty line
   ##    set descr ""
   ##    while { [set ret [getLine $fp 0]] > 0 && \
   ##          [string length $::line] > 0 } {
   ##       append descr $::line "\n"
   ##    }
   ##    # options are separated by "; "
   ##    set q [split [string map {"; " "\n"} $opt] "\n"] 
   ##    lappend q $descr
   ##    lappend cmd(options) $q
   ## }
   ## return $ret
   return 1
}

proc handleDescription { fp command } {
   upvar $command cmd

   # .DESCRIPTION:
   set cmd(descr) [string trim [string range $::line 14 end]]
   while { [set ret [getLine $fp 0]] > 0 \
         && [string length $::line] > 0 } {
      append cmd(descr) "\n" $::line
   }
   if { $ret >= 0 } {
      # skip last empty line
      set ret [getLine $fp 0]
   }
   return $ret
}
proc handleCommands { fp command } {
   upvar $command cmd

   # .CMD:
   set syn [string trim [string range $::line 6 end]]
   set descr {}
   set opts {}

   set ret [getLine $fp 0]
   if { $ret > 0 && [string equal -length 10 $::line ".CMDDESCR:"] } {
      set descr [string range $::line 11 end]
      while { [set ret [getLine $fp 1]] > 0 \
            && [string index $::line 0] != "." } {
         append descr "\n" $::line
      }
   }

   if { $ret > 0 && [string equal -length 12 $::line ".CMDOPTIONS:"] } {
      set opts [subOptions $fp]
   }

   lappend cmd(cmds) [list $syn $descr $opts]

   return $ret
}
proc handleExample { fp command } {
   upvar $command cmd
   set nn ""
   while { [getLine $fp 0] > 0 \
         && [string length $::lineNT] \
         && [string index $::lineNT 0] != "." } {
      append cmd(example) $nn $::lineNT
      set nn "\n"
   }

   # set efp [open qqq.tcl w]
   # puts $efp "package require GnoclCanvas"
   # puts $efp "package require GnoclGnome"
   # puts $efp $cmd(example)
   # puts $efp "gnocl::mainLoop"
   # close $efp
   # catch {exec tclsh qqq.tcl &}

   return [getLine $fp 1]
}

proc handleSeeAlso { fp command } {
   upvar $command cmd
   # .SEEALSO: 
   set cmd(seeAlso) [string range $::line 9 end]
   return [getLine $fp 1]
}

proc handleKeywords { fp command } {
   upvar $command cmd
   return [getLine $fp 1]
}

proc printOption { fp name type default descr spaces } {
   set sp [string repeat " " $spaces]
      
   puts $fp "$sp<varlistentry>"
   puts $fp "$sp   <term>$name</term>"
   puts $fp "$sp   <listitem>"
   if { [string length $type] } {
      switch -exact -- $type {
         PERCENTSTRING  { set txt "<link linkend='percentString'><type>percent-string</type></link>" }
         COLOR          { set txt "<link linkend='color'><type>color</type></link>" }
         COLORA         { set txt "<link linkend='color'><type>color with transparancy</type></link>" }
         default        { set txt "<type>$type</type>" }
      }
      puts -nonewline $fp "$sp      <para>type: $txt"

      if { [string length $default] } {
         puts $fp " (default: $default)"
      } else {
         puts $fp ""
      }
      puts $fp "$sp      </para>"
   }
   if { [string length $descr] } {
      if { [regexp {(.*)DISCLAIMER_COLOR\s} $descr qqq txt] } {
         puts $fp "$sp      <para>$txt</para>"
         puts $fp "$sp      <warning><para>Setting colors of widgets is 
               dangerous. If another theme is used the widgets can become 
               unreadable!</para></warning>"
      } else {
         puts $fp "$sp      <para>$descr</para>"
      }
   }
   puts $fp "$sp   </listitem>"
   puts $fp "$sp</varlistentry>"
}

proc printSynopsis { fp arguments spaces } {
   set sp [string repeat " " $spaces]

   foreach el $arguments {
      if { [string equal $el .VAROPTS.] } {
         puts $fp "$sp<arg rep=\"repeat\" choice=\"opt\"><replaceable>-option</replaceable> <replaceable>value</replaceable></arg>"
      } else {
         puts $fp "$sp<arg choice=\"plain\"><replaceable>$el</replaceable></arg>"
      }
   }
}

proc getCmdName { orig } {
   if { [regexp canvas(.*) $orig {} item] } {
      set name "canvas "
      append name [string tolower $item 0 0]
   } elseif { [regexp toolBar(.*) $orig {} item] } {
      set name "toolBar "
      append name [string tolower $item 0 0]
   } else {
      set name $orig
   }
   return $name
}

proc printCmd { command } {
   upvar $command cmd

   set fileName [string tolower $cmd(name)].xml 
   set graphFile "../pics/$cmd(name).png"
   set isCanvas [string match "canvas?*" $cmd(name)]
   set isToolBar [string match "toolBar?*" $cmd(name)]

   set name [getCmdName $cmd(name)]
   set fp [open $fileName w]
   puts $fp "
<refentry id=\"$cmd(name)\">
<refmeta> 
   <refentrytitle>$name</refentrytitle>
</refmeta>

<refnamediv>
   <refname>$name</refname>
   <refpurpose>$cmd(shortDescr)</refpurpose>
</refnamediv>\n"

   puts $fp "<refsynopsisdiv>\n   <cmdsynopsis>"
   set el [lindex $cmd(synopsis) 0] 
   if { $isCanvas || $isToolBar } {
      puts -nonewline $fp "      <command><replaceable>"
      if { $isCanvas } {
         puts -nonewline $fp canvasId
      } else {
         puts -nonewline $fp toolBarId
      }
      puts $fp "</replaceable></command> <arg choice=\"plain\">"
      if { $isCanvas } {
         puts -nonewline $fp create
      } else {
         puts -nonewline $fp add
      }
      puts $fp "</arg> <arg choice=\"plain\">$el</arg>"
   } else {
      puts $fp "      <command>$el</command>"
   }
   printSynopsis $fp [lrange $cmd(synopsis) 1 end] 6
   puts $fp "   </cmdsynopsis>\n</refsynopsisdiv>\n"

   if { [file exist $graphFile] } {
      puts $fp "<refsect1><title>Screenshot</title>"
      puts $fp "   <screenshot><graphic fileref=\"$graphFile\"></graphic></screenshot>"
      puts $fp "</refsect1>"
   }

   if { [llength $cmd(options)] } {
      puts $fp "<refsect1><title>Options</title>\n   <variablelist>"
      foreach el $cmd(options) {
         foreach {optName type default descr} {"" "" "" ""} break
         foreach {optName type default descr} $el break
         printOption $fp $optName $type $default $descr 6
      }
      puts $fp "   </variablelist>\n</refsect1>\n"
   }


   if { [string length $cmd(descr)] } {
      puts $fp "<refsect1><title>Description</title>"
      puts $fp "<para>"
      puts $fp [string map {NEWPARA "</para>\n<para>"} $cmd(descr)]
      puts $fp "</para>\n</refsect1>\n"

   }

   if { [string length $cmd(cmds)] } {
      puts $fp "<refsect1><title>Commands</title>\n   <variablelist>"
      foreach el $cmd(cmds) {
         puts $fp "      <varlistentry>\n         <term><cmdsynopsis>"
         foreach {syn descr opts} $el break
         if { $isCanvas } {
            puts $fp "            <command><replaceable>canvasId</replaceable></command> <arg choice=\"plain\">itemCommand</arg> <arg choice=\"plain\"><replaceable>tagOrId</replaceable></arg>"
         } else {
            puts $fp "            <command><replaceable>id</replaceable></command>"
         }
         set subCmd [lindex $syn 0]
         puts $fp "            <arg choice=\"plain\">$subCmd</arg>"
         printSynopsis $fp [lrange $syn 1 end] 12 
         puts $fp "            </cmdsynopsis></term>\n         <listitem>"
         puts $fp "         <para>$descr</para>"
         if { [llength $opts] } {
            puts $fp "         <variablelist><title>Options</title>"
            foreach elOpt $opts {
               # first set all vars to ""
               foreach {optName type default descr} {"" "" "" ""} break
               foreach {optName type default descr} $elOpt break
               printOption $fp $optName $type $default $descr 12
            }
            puts $fp "         </variablelist>"
         }
         puts $fp "         </listitem>\n      </varlistentry>"
      }
      puts $fp "   </variablelist>\n</refsect1>\n"
   }

   if { [string length $cmd(example)] } {
      puts $fp "<refsect1><title>Example</title>\n<para>"
      puts $fp "<programlisting><!\[CDATA\["
      puts $fp $cmd(example)
      puts $fp "]]></programlisting>\n</para>"
      if { [file exist $graphFile] } {
         puts $fp "<para>results in</para>"
         puts $fp "<screenshot><graphic fileref=\"$graphFile\"></graphic></screenshot>"
      }
      puts $fp "</refsect1>\n"

   }

   if { [string length $cmd(seeAlso)] } {
      puts $fp "<refsect1><title>See also</title>\n<para>"
      set notFirst 0 
      foreach el [split $cmd(seeAlso) ","] {
         set el [string trim $el]
         if { $notFirst } {
            puts $fp "$txt,"
         }
         set notFirst 1
         if { [string first "Gtk" $el] == 0 \
               || [string first "Gnome" $el] == 0 } {
            set txt "   <function>$el</function>"
         } else {
            set elName [getCmdName $el]
            set txt "   <link linkend=\"$el\"><command>$elName</command></link>"
         }
      }
      puts $fp $txt
      puts $fp "</para>\n</refsect1>\n"
   }

   puts $fp "</refentry>\n"

   close $fp
}

proc main { fp } {
   array set cmd {name ""}

   while { ![eof $fp] } {
      set idx [string first ":" $::line]
      if { $idx < 0 || [string index $::line 0] != "." } {
         error "handleCommand: unknown sequence: \"$::line\""
      }
      # puts [format "string: \"%s\"" [string range $::line 1 $idx] ]

      switch -exact -- [string range $::line 1 $idx] {
         NAME:          { 
                           if { [string length $cmd(name)] } {
                              # parray cmd
                              printCmd cmd
                           }
                           array set cmd {name "" shortDescr "" synopsis "" 
                                 options "" descr "" cmds "" example "" 
                                 seeAlso ""}
                           set cmd(name) [string trim \
                              [string range $::line 6 end]] 
                           set ret [getLine $fp 1]
                        }
         SHORTDESCR:    { set ret [handleShortDescr $fp cmd] }
         SYNOPSIS:      { set ret [handleSynopsis $fp cmd] }
         OPTIONS:       { set ret [handleOptions $fp cmd] }
         DESCRIPTION:   { set ret [handleDescription $fp cmd] }
         CMD:           { set ret [handleCommands $fp cmd] }
         EXAMPLE:       { set ret [handleExample $fp cmd] }
         SEEALSO:       { set ret [handleSeeAlso $fp cmd] }
         default        { error "unknown sequence \"$::line\"" }
      }
         # KEYWORDS:      { set ret [handleKeywords $fp line cmd] }
   }

   # parray cmd
   printCmd cmd

   return $ret
}

# skip empty lines
# set fp [open manpages.txt]
set fp stdin
set line ""
set ret [getLine $fp 1]

main $fp 

## while { $ret >= 0 } {
##    set idx [string first ":" $::line]
##    if { $idx < 0 || [string index $::line 0] != "." } {
##       error "handleCommand: unknown sequence: \"$::line\""
##    }
##    switch -exact -- [string range $::line 1 $idx] {
##       NAME:    { set ret [handleCommand $fp line] }
##       default  { error "unknown sequence: \"$::line\"" }
##    }
## }

close $fp

