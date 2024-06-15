proc WaitRiconnetti { } {
  global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome
  global BottoneStart BottoneStop BottoneReload BottoneAdd

$BottoneStart configure -state disabled
$BottoneStop configure -state disabled
$BottoneAdd configure -state disabled
$BottoneReload configure -state disabled
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled
$BottoneSalvaCome(widget) entryconfigure\
				$BottoneSalvaCome(index) -state disabled
}

proc OkRiconnetti { } {
  global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome
  global BottoneStart BottoneStop BottoneReload BottoneAdd Congela Matrice

set Congela False
Reload $Matrice(toplevel)
RefreshInputs
MessageBox $Matrice(toplevel) "CONNECTED !!!!" yes
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



##############################################################################
# Procedura che verifica l'esistenza del processo qplc
# Ritorna:
#	0	Il PLC e' presente
#	1 	Il PLC non e' presente
##############################################################################
proc Riconnetti { } {

  global sId_delay sId_toggle sId_listp
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
  global SocketPlc Host Congela Matrice

if { $Congela == "True" } {
	WaitRiconnetti
	}
if { [VerificaEsistenzaPlc] == 0 } {
	OkRiconnetti
	return
	}
# ci riprovo dopo 3 secondi
after 3000 Riconnetti
}

##############################################################################
# Procedura che verifica l'esistenza del processo qplc
# Ritorna:
#	0	Il PLC e' presente
#	1 	Il PLC non e' presente
##############################################################################
proc VerificaEsistenzaPlc { } {

  global sId_delay sId_toggle sId_listp
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
  global SocketPlc Host Congela

if [catch {set sId_delay [socket $Host $SocketPlc]} Result] {
	return 1
	}
fileevent $sId_delay readable {AspettaRispostaPLC delay}
vwait RispostaPLC_delay
if [catch {set sId_toggle [socket $Host $SocketPlc]} Result] {
	return 1
	}
fileevent $sId_toggle readable {AspettaRispostaPLC toggle}
vwait RispostaPLC_toggle
if [catch {set sId_listp [socket $Host $SocketPlc]} Result] {
	return 1
	}
fileevent $sId_listp readable {AspettaRispostaPLC listp}
vwait RispostaPLC_listp
set Congela False
return 0
}


##############################################################################
# Procedura che ritorna il valore della variabile se il PLC e' attivo
# Ritorna: valore
#	
##############################################################################
proc Getstate { NomeVar tred } {
  global TcpHostPlcRemoto SocketPlc AddressBase TipoIO Nomi Lunghezza
  global sId_delay sId_toggle sId_listp 
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
  global	Congela

if { $Congela == "True" } {
	return
	}

switch -- $tred {
	"delay" {
		set sId	 $sId_delay
		set RispostaPLC RispostaPLC_delay
		}
	"toggle" {
		set sId	 $sId_toggle
		set RispostaPLC RispostaPLC_toggle
		}
	"listp" {
		set sId	 $sId_listp
		set RispostaPLC RispostaPLC_listp
		}
	}

switch -- $TipoIO($NomeVar) {
	"1" {
		puts $sId "mem [format %x $Nomi($NomeVar) ]\
			 			[Nbytes $Lunghezza($NomeVar)]"
		flush $sId
		vwait $RispostaPLC
		append risp $ $RispostaPLC
		eval "set rispo $risp"
		return [format %d 0x[InvertiByte [lindex $rispo 1] ] ] }
	"2" {
		puts $sId "mem [format %x $Nomi($NomeVar) ]\
						[Nbytes $Lunghezza($NomeVar)]"
		flush $sId
		vwait $RispostaPLC
		append risp $ $RispostaPLC
		eval "set rispo $risp"
		return [format %d 0x[InvertiByte [lindex $rispo 1] ] ] }
	}
}



##############################################################################
# Procedura che setta il valore della variabile se il PLC e' attivo
#
#  ritorna OK se e' andato a buon fine
#	
##############################################################################
proc Setstate { NomeVar Valore tred } {
  global Lunghezza Key
  global sId_delay sId_toggle sId_listp 
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
  global Congela

if { $Congela == "True" } {
	return
	}

switch -- $tred {
	"delay" {
		set sId	 $sId_delay
		set RispostaPLC RispostaPLC_delay
		}
	"toggle" {
		set sId	 $sId_toggle
		set RispostaPLC RispostaPLC_toggle
		}
	"listp" {
		set sId	 $sId_listp
		set RispostaPLC RispostaPLC_listp
		}
	}

puts $sId "setin $Key($NomeVar) [format %x $Valore]"
flush $sId
vwait $RispostaPLC
append risp $ $RispostaPLC
eval "set rispo $risp"
return $rispo
}



##############################################################################
# Procedura che attende la risposta dal PLC
# Ritorna:
#       Risposta del PLC
##############################################################################
proc AspettaRispostaPLC { tred } {

global Variabili
global sId_delay sId_toggle sId_listp 
global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
global	Congela

switch -- $tred {
	"delay" {
		set sId	 $sId_delay
		set RispostaPLC RispostaPLC_delay
		}
	"toggle" {
		set sId	 $sId_toggle
		set RispostaPLC RispostaPLC_toggle
		}
	"listp" {
		set sId	 $sId_listp
		set RispostaPLC RispostaPLC_listp
		}
	}

set linea [GetLine $sId] 

set Variabili [list ""]

#testo se si tratta di OK

if {[string range $linea 0 2] == "OK"} {
	set $RispostaPLC $linea
	return
	}

# se il secondo carattere e' - significa che segue un'altra linea
while { [string index $linea 1] == "-" } {
	set var [string range $linea 2 end]
	set Variabili [lappend Variabili "$var"]
	set linea [GetLine $sId]
	}
set $RispostaPLC $linea
}



##############################################################################
# Procedura che richiede al PLC lo stato di una variabile
# Ritorna:
#	0	OK
#	1 	ERRORE
##############################################################################
proc GetOutput { Variabile tred } {

global sId_delay sId_toggle sId_listp 
global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp
global	Congela

if { $Congela == "True" } {
	return
	}

switch -- $tred {
	"delay" {
		set sId	 $sId_delay
		set RispostaPLC RispostaPLC_delay
		}
	"toggle" {
		set sId	 $sId_toggle
		set RispostaPLC RispostaPLC_toggle
		}
	"listp" {
		set sId	 $sId_listp
		set RispostaPLC RispostaPLC_listp
		}
	}


puts $sId "wreset"
flush $sId
vwait $RispostaPLC

if { [lindex $$RispostaPLC 0] == "OK" } {
	return 0
    	}
return 1
}


##############################################################################
# Procedura che richiede al PLC la lista variabili di I/O
# Ritorna:
#	0	OK
#	1 	ERRORE
##############################################################################
proc GetLista { } {
global	Congela
global sId_listp RispostaPLC_listp FileLingua
global Variabili Nomi TipoIO Lunghezza Key AddressBase

if { $Congela == "True" } {
	return
	}

puts $sId_listp "mod U_CODE"
flush $sId_listp
vwait RispostaPLC_listp
if { [lindex $RispostaPLC_listp 0] == "E" } {
	MessageBox $w  "[CaricaLingua $FileLingua ErroreLista]"  yes
	} elseif { [lindex $RispostaPLC_listp 0] == "D" } {
		set AddressBase 0x[lindex $RispostaPLC_listp 1]
	}

puts $sId_listp "list p"
flush $sId_listp
vwait RispostaPLC_listp
foreach var $Variabili {
    if { [lindex $var 2] != "" } {
	puts $sId_listp "sym [lindex $var 3]"
	flush $sId_listp
	vwait RispostaPLC_listp
	if { [lindex $RispostaPLC_listp 1] != "S" } {
		set mnemonico [lindex $RispostaPLC_listp 2]
		} else {
		set mnemonico "[lindex $RispostaPLC_listp 2]\[[format "%d" 0x[lindex $RispostaPLC_listp 3]]\]"
		}
	set address [lindex $var 3]
	set key [lindex $var 4]
	set inpoout [lindex $var 1]
	set lunghezza [format "%d" "0x[lindex $var 2]" ]
	set Nomi($mnemonico) "0x$address"
	set TipoIO($mnemonico) $inpoout
	set Lunghezza($mnemonico) $lunghezza
	set Key($mnemonico) $key
	}
     }
}


##############################################################################
# Procedura che scarica una linea dal socket con il PLC
# Ritorna:
#	0	OK
#	1 	ERRORE
##############################################################################
proc GetLine { sId } {
global	Congela Matrice FileLingua
set carattere [read $sId 1]
append Risultato $carattere
if { $carattere == "\0" } {
# Caso di PLC killato
	Stop $Matrice(toplevel)
	set Congela True
	close $sId
	set risp [MessageBox $Matrice(toplevel) "RECONNECT ?" yesno]
	if { $risp == "no" } {
		exit
		}
	if { $risp == "busy" } {
		return
		}
	if { $risp == "yes" } {
		Riconnetti
		return
		}
	}
while { $carattere != "\n" && $carattere != "\0" } {
	set carattere [read $sId 1]
	if { $carattere != "\n" && $carattere != "\0" } {
  		append Risultato $carattere
		}
	}
if { $Risultato == "A 1" } {
	if { $Congela == "True" } {
		return
		}
	set Congela True
        Stop $Matrice(toplevel)
	WaitRiconnetti
	return
	}
if { $Risultato == "A 2" } {
	if { $Congela == "False" } {
		return
		}
	set Congela False
	OkRiconnetti
	return
	}
if { $carattere == "\0" } {
# Caso di PLC killato
        set Congela True
	Stop $Matrice(toplevel)
	close $sId
	if { [MessageBox $Matrice(toplevel) "RECONNECT ?" yesno] == "no" } {
		exit
		} else {
		Riconnetti
		return
		}
        }
return $Risultato
}





##############################################################################
# Procedura che rinfresca gli inputs dopo un KILL
##############################################################################
proc RefreshInputs { } {

global Congela Matrice TipoIO FileLingua

$Matrice(widget) flush
upvar \#0 $Matrice(page) ary

for {set line 1} { $line <= $Matrice(linee) } { incr line } {
   # Loop su tutti gli inputs sorvegliati
   set NomeVariabile $ary($line,0)
   if { $TipoIO($NomeVariabile) == 1 } {

	if { $ary($line,1) == "ON" } {
# Caso di variabile a 1 bit
	    if { [Setstate $NomeVariabile 1 toggle] != "OK" } {
		MessageBox $Matrice(toplevel)\
		 "[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile"  yes
		continue
		}
	     while { [Getstate $NomeVariabile toggle] != 1 } {
                }
	     }
        } else {
# Caso di variabile a 16 bits
	if { [Setstate $NomeVariabile $ary($line,1) toggle] != "OK" } {
                MessageBox $Matrice(toplevel)\
                 "[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile"  yes
                continue
                }
	}
    }
}
