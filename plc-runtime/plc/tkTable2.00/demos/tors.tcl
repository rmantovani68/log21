#!/home/bonazza/tcltk/tclext/tkTable2.00/tablewish

array set table {
    table	.table
    rows	13
    cols	10
}

array set ColumnLabel {
    1	X1
    2	Y1
    3	Y2
    4	Y3
    5	Y4
    6	Z1
    7	""
    8	F
    9	COMANDI
}

set ColonnaSeparatrice 7

proc colorize num {
    if {$num == 7} { 
      return colored 
    } else {
      return anchorleft 
    } 
}

proc fill_headers {w {r 13} {c 9}} {
  global ColumnLabel
    for {set i 1} {$i <= $r} {incr i} {
	$w set $i,0 "$i"
    }
    for {set j 1} {$j <= $c} {incr j} {
	    $w set 0,$j $ColumnLabel($j)
    }
}

proc table_validate {w idx} {

global ColonnaSeparatrice
  set colonna [lindex [split $idx ","] 1]
  set riga [lindex [split $idx ","] 0]
  set NewRow [$w index active row]
  set NewCol [$w index active col]
  if { $NewCol == $ColonnaSeparatrice } {
    if { $riga == $NewRow && $colonna < $NewCol } { 
      incr NewCol 
      $w activate $riga,$NewCol 
    } elseif { $riga == $NewRow && $colonna > $NewCol } {
      incr NewCol -1
      $w activate $riga,$NewCol 
    }
  }
}

proc validate {c val} {
global table

    if {$c==[expr $table(cols) -1]} {
	  ## Alphanum
	  set expr {^[A-Za-z0-9 ]*$}
    } elseif {$c} {
	  ## Real
	  set expr {^[-+]?[0-9]*\.?[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
    }
    if {[regexp $expr $val]} {
	  return 1
    } else {
	  bell
	  return 0
    }
}

label .example -text "Esempio di molla a torsione"

set t $table(table)
table $t \
	-rows $table(rows) \
	-cols $table(cols) \
    -autoclear true \
	-cache 1 \
	-titlerows 1 \
	-titlecols 1 \
	-yscrollcommand { .tsy set } \
	-xscrollcommand { .tsx set } \
	-width 15 -height 13 \
	-coltagcommand colorize \
	-flashmode on \
	-selectmode extended \
	-colstretch unset \
	-rowstretch unset \
	-batchmode 1 \
    -browsecommand {table_validate %W %s} \
	-validate yes \
	-vcmd {if {![%W tag includes title %C]} { validate %c %S } }

fill_headers $t
$t tag config colored -bg lightblue -state disabled
$t tag config anchorleft -anchor w 
$t tag config title -fg red -anchor c
$t width 0 3
$t width $ColonnaSeparatrice 3
$t width [expr $ColonnaSeparatrice + 2] 15

scrollbar .tsy -command [list $t yview]
scrollbar .tsx -command [list $t xview] -orient horizontal
grid .example -     -sticky ew
grid $t       .tsy   -sticky news
grid .tsx            -sticky ew
grid columnconfig . 0 -weight 1
grid rowconfig . 1 -weight 1
