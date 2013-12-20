#!/bin/sh
# restart \
exec tclsh "$0" "$@"

proc eleven {str} {
set a 0
for {set i 0} {$i < 11} {incr i} {
set a [expr {($a + [A [string index $str $i]])%11}]
}
set b 0
for {set i 0} {$i < 11} {incr i} {
set b [expr {($b + ($i+1)*[A [string index $str $i]])%11}]
}
return [list $a $b]
}

proc A {x} {
if {$x == "X"} {
return 10
} else {
return $x
}
}

proc B {x} {
if {$x == "10"} {
return "X"
} else {
return $x
}
}

proc inv {x} {
set r 0
for {set i 0} {$i < 11} {incr i} {
if {[expr {($x*$i)%11}]==1} {
set r $i
break
}
}
return $r
}

proc fix {str} {
set ch [eleven $str]
set loc [expr {([lindex $ch 1]*[inv [lindex $ch 0]])%11}]
if {$loc == 0} {
set loc 11
}
set n [B [expr {([A [string index $str [expr $loc - 1]]] - [lindex $ch 0])%11}]]
set out [string replace $str [expr $loc -1] [expr $loc -1] $n]
return $out
}

proc Fix {} {
global co cr 
set cr [fix $co]
}

proc Check {} {
global co ch
set ch [eleven $co]
}

frame .f1
frame .f2
button .f1.l1 -text "code:" -command "Check"
entry .f1.e1 -textvariable co -width 12
entry .f1.e2 -textvariable ch -width 5
pack .f1.l1 .f1.e1 .f1.e2 -side left
button .f2.l2 -text "correct:" -command "Fix"
entry .f2.e3 -textvariable cr -width 12
pack .f2.l2 .f2.e3 -side left
pack .f1 .f2 -side top
