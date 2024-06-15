#########
# salva #
#########

proc Salva { w } {
global	Matrice
if [info exist Matrice(NomeFile) ] {
	Save
	} else {
	SaveCome $w
	}
}

########
# Save #
########

proc Save { } {

global Matrice
global font_usato FileLingua

upvar \#0 $Matrice(page) ary

if {[catch "set fId [open $Matrice(NomeFile) w]" out] == 0} {
	fconfigure $fId -translation binary
	puts -nonewline $fId "Firmoni\n"
	puts -nonewline $fId "#    FILE DI CONFIGURAZIONE\n"
	puts -nonewline $fId "#  SIMULATORE DI I/O PER QPLC\n"
	for {set i 1} {$i <= $Matrice(linee)} { incr i } {
		puts -nonewline $fId "Riga$i=$ary($i,0)=$ary($i,1)\n"
		}
	puts -nonewline $fId "inomriF\n"
	close $fId
   	} else {
	MessageBox $w\
	      "Problemi nell'apertura \n$Matrice(NomeFile) per salvataggio" yes
	return 0
	}
return 1
}
##############
#  Save Come #
##############


proc SaveCome { w } {
global Matrice FileLingua NomeFile ProgPath SAVEPATH
set ProgPath $SAVEPATH
FileSelection $w [CaricaLingua $FileLingua Salva]
if { $NomeFile != "" } {
	set Matrice(OldNomeFile) $Matrice(NomeFile) 
	if { [string first ".SIM" $NomeFile ] == "-1" && \
			[string first ".sim" $NomeFile ] == "-1" } {
		set Matrice(NomeFile)\
			 "/home/d_xnc/plc/[string toupper $NomeFile.SIM]"
		} else {
		set Matrice(NomeFile)\
			 "/home/d_xnc/plc/[string toupper $NomeFile]"
		}
	if [ Save ] {
		$Matrice(WidgetNomeFile) configure\
	      -text "[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)"
		} else {
		set Matrice(NomeFile) $Matrice(OldNomeFile)
		}
		
	}
}


########
# Apri #
########

proc Apri { w } {
global Matrice FileLingua NomeFile ProgPath SAVEPATH
set ProgPath $SAVEPATH
FileSelection $w [CaricaLingua $FileLingua Apri]
if { $NomeFile != "" } {
	set Matrice(OldNomeFile) $Matrice(NomeFile) 
	set Matrice(NomeFile) "/home/d_xnc/plc/$NomeFile"
	if [ ApriPure ] {
		$Matrice(WidgetNomeFile) configure\
	      -text "[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)"
		} else {
		set Matrice(NomeFile) $Matrice(OldNomeFile)
		}
	}
}


proc ApriPure { } {

global Matrice
global font_usato FileLingua

upvar \#0 $Matrice(page) ary

if {[catch "set fId [open $Matrice(NomeFile) r]" out] != 0} {
	MessageBox $w\
	      "Problemi nell'apertura \n$Matrice(NomeFile) per lettura" yes
	return 0
	}

set inputfile [split [read $fId] \n]
close $fId

Cancella $Matrice(toplevel)

update idletasks
foreach line $inputfile {
	if { $line != "Firmoni" && $line != "inomriF" && $line != ""  && \
				 [string index $line 0] != "#"  } {
		InserisceVareVal [lindex [split $line =] 1]\
					[lindex [split $line =] 2] toggle
		}
	}
RefreshInputs
return 1
}

