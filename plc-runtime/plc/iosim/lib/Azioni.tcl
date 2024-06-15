#
#
#
#               #######################
#               #                     #
#               #       Reload        #
#               #                     #
#               #######################
#
# Interroga nuovamente il PLC per ottenere la lista delle variabili I/O
#
#  INPUTS:	w		-> toplevel
#
proc    Reload { w } {
global Matrice font_usato listbox Nomi TipoIO Lunghezza
global FileLingua IconaInput IconaOutput Congela
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome
global BottoneStart BottoneStop BottoneReload BottoneAdd

if {$Congela == "True"} {
	return
	}

$BottoneStart configure -state disabled
$BottoneStop configure -state disabled
$BottoneAdd configure -state disabled
$BottoneReload configure -state disabled
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled
$BottoneSalvaCome(widget) entryconfigure\
			$BottoneSalvaCome(index) -state disabled


$w config -cursor watch
update idletasks

if [array exists Nomi] {
	unset Nomi 
	}
GetLista

$listbox delete 0 end
update idletasks

foreach var [lsort [array names Nomi]] {
	if { $var != "" && [info exist TipoIO($var) ] } {
		if { $TipoIO($var) == 1 } {
			# INPUT
			$listbox insert end -itemtype imagetext -text "$var" -image $IconaInput
			} elseif { $TipoIO($var) == 2 } {
			# OUTPUT
			$listbox insert end -itemtype imagetext -text "$var" -image $IconaOutput
			}
		}
	}
$w config -cursor arrow
$BottoneStart configure -state active
$BottoneStop configure -state disabled
$BottoneAdd configure -state active
$BottoneReload configure -state active
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state active
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state active
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state active
$BottoneSalvaCome(widget) entryconfigure\
			$BottoneSalvaCome(index) -state active
}
#
#
#
#               #######################
#               #                     #
#               #       Aggiungi      #
#               #                     #
#               #######################
#
# Gestisce l'inserimento della variabile selezionata correntemente nella 
# listbox ..
#
#  INPUTS:	w		-> toplevel
#
proc    Aggiungi { w } {
global Matrice font_usato listbox
global FileLingua Congela

if {$Congela == "True"} {
	return
	}

if { [ catch { set selezionato [$listbox info selection ] } out ] != 0 } {
	return
	}
foreach linea [split $selezionato " "] {
	if { $linea != "" } {
     		InserisceVar [$listbox entrycget $linea -text] toggle
		}
	}
}


#
#
#               ####################
#               #                  #
#               #       Stop       #
#               #                  #
#               ####################
#
# Ferma l'aggiornamento a video dello stato delle variabili di I/O
#
#  INPUTS:	w		-> toplevel
#
proc    Stop { w } {
global Matrice font_usato listbox BottoneAdd BottoneReload
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome
global FileLingua Start BottoneStart BottoneStop IconaStart

set Start 0
$BottoneStart configure -state active
$BottoneStop configure -state disabled
$BottoneAdd configure -state active
$BottoneReload configure -state active
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state active
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state active
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state active
$BottoneSalvaCome(widget) entryconfigure $BottoneSalvaCome(index)\
								 -state active
}
#
#
#
#
#
#
#
#
#
#
#
#               #################################
#               #                               #
#               #       InserisceVar            #
#               #                               #
#               #################################
#
# Inserisce nella TKtable una nuova linea con la variabile I/O 
#
#  INPUTS:	NomeVariabile 	-> Nome della variabile
#
proc    InserisceVar { NomeVariabile tred } {

global Matrice font_usato listbox TipoIO Lunghezza
global FileLingua ArrayDebugLinee
global HOME

if [info exist ArrayDebugLinee($NomeVariabile) ] {
	return
	}

upvar \#0 $Matrice(page) ary
upvar \#0 $Matrice(copypage) copyary

# linee reali nella matrice
set linee [expr [lindex [$Matrice(widget) configure -rows] 4] -1]

set ArrayDebugLinee($NomeVariabile) [expr $Matrice(linee) + 1]
if { $linee <= $Matrice(linee) } {
	$Matrice(widget) insert rows $Matrice(linee) 1
	update idletasks
	}

set ary([expr $Matrice(linee) +1],0) $NomeVariabile
set copyary([expr $Matrice(linee) +1],0) $NomeVariabile
set copyary([expr $Matrice(linee) +1],1) ""
InserisceValore [expr $Matrice(linee) +1] $tred

incr Matrice(linee)
$Matrice(widget) see $Matrice(linee),0
update idletasks
# $Matrice(widget) tag row centro-ro 0
$Matrice(widget) tag col sinistro-ro 0
$Matrice(widget) tag col centro-ro 1
}

#
#
#
#               #################################
#               #                               #
#               #       InserisceVareVal        #
#               #                               #
#               #################################
#
# Inserisce nella TKtable una nuova linea con la variabile I/O 
#
#  INPUTS:	NomeVariabile 	-> Nome della variabile
#
proc    InserisceVareVal { NomeVariabile Valore tred } {

global Matrice font_usato listbox TipoIO Lunghezza
global FileLingua ArrayDebugLinee
global HOME

if [info exist ArrayDebugLinee($NomeVariabile) ] {
	return
	}

upvar \#0 $Matrice(page) ary
upvar \#0 $Matrice(copypage) copyary

# linee reali nella matrice
set linee [expr [lindex [$Matrice(widget) configure -rows] 4] -1]

set ArrayDebugLinee($NomeVariabile) [expr $Matrice(linee) + 1]
if { $linee <= $Matrice(linee) } {
	$Matrice(widget) insert rows $Matrice(linee) 1
	update idletasks
	}

set copyary([expr $Matrice(linee) +1],0) $NomeVariabile
set copyary([expr $Matrice(linee) +1],1) ""
set ary([expr $Matrice(linee) +1],0) $NomeVariabile
SetMatrice $NomeVariabile $TipoIO($NomeVariabile) $Lunghezza($NomeVariabile)\
		[expr $Matrice(linee) +1] $Valore

incr Matrice(linee)
$Matrice(widget) see $Matrice(linee),0
update idletasks
$Matrice(widget) tag col sinistro-ro 0
$Matrice(widget) tag col centro-ro 1
}


#
#
#
#
#               #################################
#               #                               #
#               #       InserisceValore         #
#               #                               #
#               #################################
#
# Aggiorna nella TKtable il valore corrente della variabile nella linea
#
#  INPUTS:	linea da aggiornare
#
proc    InserisceValore { linea tred } {

global Matrice font_usato listbox TipoIO Lunghezza
global FileLingua
global HOME


upvar \#0 $Matrice(page) ary

set NomeVariabile $ary($linea,0)

if { $TipoIO($NomeVariabile) != 1 && $TipoIO($NomeVariabile) != 2 } {
	MessageBox $Matrice(toplevel)\
	 "[CaricaLingua $FileLingua ErroreTipoSconosciuto] $NomeVariabile" yes
	return
	}
SetMatrice $NomeVariabile $TipoIO($NomeVariabile) $Lunghezza($NomeVariabile)\
	$linea [Getstate $NomeVariabile $tred ]

}




#
#
#
#               #################################
#               #                               #
#               #       StartAggiornamento      #
#               #                               #
#               #################################
#
# Lancia l'aggiornamento nella TKtable il valore corrente delle variabili
#
#
proc    StartAggiornamento { w } {

global Matrice font_usato listbox TipoIO Lunghezza
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome
global FileLingua BottoneStart BottoneStop BottoneAdd BottoneReload
global HOME Start IconaStop Congela

if {$Congela == "True"} {
	return
	}

$BottoneStart configure -state disabled
$BottoneStop configure -state active
$BottoneAdd configure -state disabled
$BottoneReload configure -state disabled
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled
$BottoneSalvaCome(widget) entryconfigure $BottoneSalvaCome(index)\
								 -state disabled


for {set i 1} { $i <= $Matrice(linee) } { incr i } {
	InserisceValore $i delay
	}
set Start 1
Aggiornamento
}



#
#
#
#               #################################
#               #                               #
#               #       Aggiornamento           #
#               #                               #
#               #################################
#
# Aggiorna nella TKtable il valore corrente delle variabili
#
#
proc    Aggiornamento {} {

global Matrice font_usato listbox TipoIO Lunghezza
global FileLingua
global HOME Start
set y1 \
   [expr round([lindex [$Matrice(widget) yview] 0] * $Matrice(linee) + 1) ] 
set y2 \
   [expr round([lindex [$Matrice(widget) yview] 1] * $Matrice(linee) ) ] 

if { $Start == 1 } {
	for {set i $y1} { $i <= $y2  } { incr i } {
		InserisceValore $i delay
		}
	update idletasks
	after 500 Aggiornamento
	}
}




#
#
#
#               #################################
#               #                               #
#               #       ToggleValore            #
#               #                               #
#               #################################
#
# Aggiorna nella TKtable il valore corrente della variabile nella linea
#
#  INPUTS:	linea da aggiornare
#
proc    ToggleValore { line } {

global Matrice TipoIO Lunghezza
global HOME FileLingua

upvar \#0 $Matrice(page) ary
set NomeVariabile $ary($line,0)

if { [Getstate $NomeVariabile toggle] == 0 } {
	# e' zero ... lo setto setin <key> <val>
	if { [Setstate $NomeVariabile 1 toggle] != "OK" } {
		MessageBox $Matrice(toplevel)\
		 "[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile"  yes
		return
		}
	while { [Getstate $NomeVariabile toggle] != 1 } {
		}
   } else {	
	# e' uno ... lo resetto
	if { [Setstate $NomeVariabile 0 toggle] != "OK" } {
		MessageBox $Matrice(toplevel)\
		 "[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile"  yes
		return
		}
	while { [Getstate $NomeVariabile toggle] != 0 } {
		}
   }
InserisceValore $line toggle
}


#
#               #################################
#               #                               #
#               #          DeleteVar            #
#               #                               #
#               #################################
#
# Cancella dalla TKtable una linea con la variabile I/O 
#
#  INPUTS:	NomeVariabile 	-> Nome della variabile
#
proc    DeleteVar { NomeVariabile } {

global Matrice font_usato listbox TipoIO Lunghezza
global FileLingua ArrayDebugLinee
global HOME

upvar \#0 $Matrice(page) ary


if { ! [info exist ArrayDebugLinee($NomeVariabile) ] } {
	return
	}

$Matrice(widget) delete rows $ArrayDebugLinee($NomeVariabile) 1
set Matrice(linee) [expr $Matrice(linee) - 1]

update idletasks
$Matrice(widget) flush
update idletasks

for {set i [expr $ArrayDebugLinee($NomeVariabile) ]} \
		 		{$i <= $Matrice(linee)} { incr i } {
	set ArrayDebugLinee($ary($i,0)) $i
	}

unset ArrayDebugLinee($NomeVariabile)
return
}

#
#
#
#               #######################
#               #                     #
#               #       Cancella      #
#               #                     #
#               #######################
#
# Gestisce la cancellazione della matrice di debug
#
#  INPUTS:	w		-> toplevel
#
proc    Cancella { w } {
global Matrice font_usato
global FileLingua ArrayDebugLinee

upvar \#0 $Matrice(page) ary

for {set i 1} {$i <= $Matrice(linee)} { incr i } {
	unset ArrayDebugLinee($ary($i,0))
        }

$Matrice(widget) delete rows 1 $Matrice(linee)
set Matrice(linee) [expr $Matrice(linee) - 1]

set Matrice(linee) 0
$Matrice(widget) flush
update idletasks
}

