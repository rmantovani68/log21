
###########################
# gestione controllo dato #
###########################

proc validate {colonna val} {

# Gestione test dato editato
switch $colonna {
	1 {
	  }
	2 {
	  ## Nome file Alfanumerico SOLO maiuscolo Lunghezza 64
	  set expr {^[a-zA-Z0-9_./]*$}
	  set len 64
	  }
	3 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	4 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	5 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	6 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	7 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	8 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	9 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	10 {
	  ## contapezzi SOLO numerico Lunghezza 4
	  set expr {^[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 4
	  }
	11 {
	  ## larghezza lamiera SOLO numerico Lunghezza 8
	  set expr {^[0-9]*\.?[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	  set len 7
	  }
	default { return 0 }
	}
 if {[regexp $expr $val]} {
	if { [string length $val] <= $len } {
#		SetModificata
		return 1
		} else {
		bell
  		return 0
		}
	} else {
	bell
  	return 0
	}
}


#######################
# gestione enter cell #
#######################

proc browse { prev next } {
    global Matrice
    global FileLingua

if { $Matrice(browse) == 1 } { return }	

set Matrice(browse) 1

# Controllo che il cursore non superi il numero righe definito
if { [lindex [split $next ","] 0] == "0" || \
			[lindex [split $next ","] 0] > $Matrice(linee) } {
	$Matrice(widget) activate $prev
	set next $prev
	}
set Matrice(browse) 0
}




#####################################
# Uscita
#####################################
proc Uscita { w } {
global FileLingua 

exit
}





#####################################
# ClickSuMatrice
#####################################
#
# tasto sinistro del mouse premuto sulla matrice
#

proc ClickSuMatrice { w r c } {
global FileLingua Matrice Nomi TipoIO Lunghezza ArrayDebugLinee Congela

if {$Congela == "True"} {
        return
        }

set label [$w get @0,$c]

if { $label == "" } {
	return
	}

if { $label == [$w get @$r,$c] } {
	# si tratta della prima colonna (LABELS)
	return
	}
upvar \#0 $Matrice(page) ary
if { $ary(0,1) == [$w get @$r,$c] } {
	# si tratta della prima riga 
	return
	}

set y $ArrayDebugLinee($label)

# Si tratta della seconda colonna (VALORI)
if { ! [info exist ary($y,0) ] } {
	return
	}

if { ! [info exist Nomi($ary($y,0)) ] } {
	return
	}

if { $TipoIO($ary($y,0)) != 1 } {
	# posso forzare solo gli inputs
	return
	}

if { $Lunghezza($ary($y,0)) == 1 } {
	# trattasi  di variabile a 1 bit
	ToggleValore $y
	} else {
	set ret [AdcInput $Matrice(toplevel) $ary($y,0) $ary($y,1) 1024 -1023]
	if { $ret != "" } {
		Setstate $ary($y,0) $ret toggle
		set ary($y,1) $ret
		}
	}
}

