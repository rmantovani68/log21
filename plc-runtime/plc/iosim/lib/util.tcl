#
#               #################################
#               #                               #
#               #       CaricaLingua            #
#               #                               #
#               #################################
#
proc CaricaLingua { FileLingua NomeEtichetta } {

  global Lingua

  if { $FileLingua != "" } { 
    set IndiceEtichetta [string first "$NomeEtichetta.$Lingua" $FileLingua]
    if { $IndiceEtichetta != -1 } {
      set FileLingua [string range $FileLingua $IndiceEtichetta end]
      set Linea [string range $FileLingua 0 [string first ";" $FileLingua]] 
      return [string range $Linea [expr [string first "=" $Linea] + 1] [expr [string length $Linea] -2]]
    } else {
      return $NomeEtichetta
    }
  } else {
    return $NomeEtichetta
  }
} 

######################################################################
# parse_args arglist - parse arglist in parent procedure
#   arglist is a list of option names (without leading "-");
# this proc puts their values (if any) into variables (named after
#   the option name) in d parent procedure
# any element of arglist can also be a list consisting of an option
#   name and a default value.
######################################################################
proc parse_args { arglist } {
  upvar args args
  
  foreach pair $arglist {
    set option [lindex $pair 0]
    set default [lindex $pair 1]                ;# will be null if not supplied
    set index [lsearch -exact $args "-$option"]
    if {$index != -1} {
      set index1 [expr {$index + 1}]
      set value [lindex $args $index1]
      uplevel 1 [list set $option $value]       ;# caller's variable "$option"
    } else {
      uplevel 1 [list set $option $default]     ;# caller's variable "$option"
    }
  }
  return 0
}


######################################################################
# button_entry w ?options? - bittuned entry for global variable
# options include
#   -label (default "Value:")
#   -variable (default value - not really optional)
#   -buttonwidth (default 15)
#   -entrywidth (default 8)
#   -command (default "")
######################################################################
proc button_entry {args} {
  parse_args {
    {label {Value:} }
    {variable value }
    {buttonwidth {15} }
    {entrywidth {8} }
    {command {} }
  }
  global $variable font_usato 
  
  set newframe [lindex $args 0]
  
  frame $newframe
  set b $newframe.b
  set e $newframe.e
  button $b -text $label -width $buttonwidth -anchor w -font $font_usato -padx 0 -pady 0 -bd 1  \
            -command "$command $newframe"
  entry $e -width $entrywidth -textvariable $variable 
  
  pack $b -in $newframe -side left -fill x 
  pack $e -in $newframe -side left -fill x -expand yes
  
  bind $e <FocusIn> {
      %W select range 0 end
  }
  bind $e <Return> [format { %s invoke } $b]
  return $newframe
}


##############################################################################
# Procedura che posiziona le finestre al centro della finestra principale    #
#############################################################################
proc posiziona:finestre {MainW w } {
    set top_default .lae
    if {$MainW  == "" } {
      set main $top_default
    } else {
      set main $MainW
    }
    # first, display off-screen:
    wm withdraw $w              ;# hide the window
    wm protocol $w WM_DELETE_WINDOW GestioneUscita
    update idletasks            ;# force geometry managers to run
    # calculate position:
    # il 7  ed il 25 sono relative ai bordi delle finestre
    if { [winfo exist $w ] == 0 } {
      return
    }

    set x [expr [winfo rootx $main] - 7 + [winfo width $main]/2 \
            - [winfo reqwidth $w]/2]
    set y [expr [winfo rooty $main] - 25 + [winfo height $main]/2 \
            - [winfo reqheight $w]/2]

    wm geom $w +$x+$y
    update idletasks
    wm deiconify $w
    update idletasks
    wm focus $w
}

proc PrmEsc {} {
  global NamePrompt
set NamePrompt {}
destroy .pr
}


######################################################################
# prompt 
######################################################################
proc prompt { CalWdg title } {
  global HOME font_usato FileLingua
  global NamePrompt

  set NamePrompt ""
  set w .pr
  if [winfo exists $w] {
    destroy $w 
  }

  toplevel $w

  wm title $w $title
  
  entry $w.field -width 40 -font $font_usato -textvariable NamePrompt

  tixButtonBox $w.box -orientation horizontal
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato  -command "destroy $w" 
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command "
                                                    set NamePrompt {}
                                                    destroy $w" 

  pack $w.field -side top -padx 10 -pady 10
  pack $w.box -side bottom -fill x

  $w.field delete 0 end
  $w.field insert end ""
  
  wm transient $w [winfo toplevel $CalWdg]

  posiziona:finestre [winfo toplevel $CalWdg] $w          

  focus $w.field

  bind $w.field  <KeyRelease> {
    set NamePrompt [string toupper $NamePrompt]
  }

  bind $w.field  <Return> "
    destroy $w
  "

  grab $w
  wm protocol $w WM_DELETE_WINDOW PrmEsc

  tkwait window $w
  return $NamePrompt
}


proc MsgEsc {} {
  global retMess
set retMess esc
destroy .message
}

#######################################
# MessageBox
#######################################
proc MessageBox { CalWdg Message type } {

  global FileLingua font_usato
  global retMess

  set w .message
  if [winfo exist $w] {
    return busy
  }


  toplevel $w
  
  label $w.top -padx 20 -pady 10 -border 1 -relief raised -anchor c\
		 -text $Message -font $font_usato 

  tixButtonBox $w.box -orientation horizontal

if { $type == "yesnoesc" } {
	$w.box add ok -text [CaricaLingua $FileLingua SI] -font $font_usato\
		 -command " 	set retMess yes
				destroy $w" 
	$w.box add close -text [CaricaLingua $FileLingua NO] -font $font_usato\
		 -command "	set retMess no
				destroy $w" 
	$w.box add esc -text [CaricaLingua $FileLingua ANNULLA]\
		 -font $font_usato -command "	set retMess esc
						destroy $w" 
	bind [$w.box subwidget ok] 	<Right> [format {
				 focus %s } [$w.box subwidget close] ]
	bind [$w.box subwidget close] 	<Right> [format {
				 focus %s } [$w.box subwidget esc] ]
	bind [$w.box subwidget esc] 	<Right> [format {
				 focus %s } [$w.box subwidget ok] ]

	bind [$w.box subwidget ok] 	<Left> [format {
				 focus %s } [$w.box subwidget esc] ]
	bind [$w.box subwidget close] 	<Left> [format {
				 focus %s } [$w.box subwidget ok] ]
	bind [$w.box subwidget esc] 	<Left> [format {
				 focus %s } [$w.box subwidget close] ]
}
if { $type == "yesno" } {
	$w.box add ok -text [CaricaLingua $FileLingua SI] -font $font_usato\
		-command " 	set retMess yes
				destroy $w" 
	$w.box add close -text [CaricaLingua $FileLingua NO] -font $font_usato\
		-command "	set retMess no
				destroy $w" 
	bind [$w.box subwidget ok] 	<Right> [format {
				 focus %s } [$w.box subwidget close] ]
	bind [$w.box subwidget close] 	<Right> [format {
				 focus %s } [$w.box subwidget ok] ]

	bind [$w.box subwidget ok] 	<Left> [format {
				 focus %s } [$w.box subwidget close] ]
	bind [$w.box subwidget close] 	<Left> [format {
				 focus %s } [$w.box subwidget ok] ]
}
if { $type == "yes" } {
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato \
	 -command " 	set retMess yes
			destroy $w" 
}
  pack $w.box -side bottom -fill x
  pack $w.top -side top -fill both -expand yes

  focus [$w.box subwidget ok] 

  wm transient $w $CalWdg 

  posiziona:finestre $CalWdg $w

  grab $w
  wm protocol $w WM_DELETE_WINDOW MsgEsc
  tkwait window $w
  return $retMess
}



#
#               #################################
#               #                               #
#               #       SetModificata           #
#               #                               #
#               #################################
#
# Setto la variabile "Modificata" che indica che la distinta corrente e' stata
# modificata
#
proc SetModificata { } {

  global Lingua IconaModificata nosaved_image editgrid

set editgrid(Modificata) 1
$IconaModificata configure -image $nosaved_image
}



#
#               #################################
#               #                               #
#               #       UnSetModificata         #
#               #                               #
#               #################################
#
# Resetto la variabile "Modificata"
#
proc UnSetModificata { } {

  global Lingua IconaModificata saved_image editgrid

if [info exist editgrid(NomeFile)] {
	if [info exist editgrid(Modificata) ] {
		unset editgrid(Modificata) 
		}
	}
$IconaModificata configure -image $saved_image
}





#
#               #################################
#               #                               #
#               #       SelezionaHost           #
#               #                               #
#               #################################
#
# Apro la finestra di edit macchina.
#
proc SelezionaHost { } {

  global Lingua IconaModificata saved_image FileLingua WSelectHost Host

#sporco trucco per il Doppio click su file
  catch {destroy .specchio}
  toplevel .specchio
  wm withdraw .specchio


set w .selezionahost
if [winfo exist $w] {
    return
}

toplevel $w
wm title $w [CaricaLingua $FileLingua SelezionaHost]


frame $w.top 
frame $w.top.fr -relief ridge -bd 2

button $w.top.ok -text [CaricaLingua $FileLingua OK]\
	 -command "okhost" -state disabled
button $w.top.annulla -text [CaricaLingua $FileLingua ANNULLA] \
	 -command "annullahost"

set WSelectHost(okbutton) $w.top.ok


tixComboBox $w.top.fr.a -label "[CaricaLingua $FileLingua Host]: "\
	-dropdown true -prunehistory true \
	-editable true -grab none -validatecmd CommandHost \
	-options {
	    listbox.height 6
	    label.width 10
	    label.anchor e
	}

if [info exist Host] {
	set WSelectHost(old) $Host
	} else {
	set WSelectHost(old) ""
	}
  
set WSelectHost(padre) $w.top.fr.a
set WSelectHost(entry) [$w.top.fr.a subwidget entry]
set WSelectHost(pulsante) [$w.top.fr.a subwidget arrow]
set WSelectHost(storia) [$w.top.fr.a subwidget listbox]

# bind $w.top.ok <FocusIn> [format {
#		if { [%s cget -value] == "" } { focus %s }
#		} $w.top.fr.a $WSelectHost(entry)]

pack $w.top
pack $w.top.fr -pady 2 -padx 2
pack $w.top.fr.a -pady 5 -padx 3
pack $w.top.ok -pady 5 -padx 3 -side left
pack $w.top.annulla -pady 5 -padx 3 -side right

foreach valore [ ReadHistory ]  {
	if { [string index $valore 0 ] != " " && [string length $valore] > 0 } {
		$WSelectHost(padre) addhistory $valore
		}
	}
posiziona:finestre [winfo toplevel .lae] $w
focus $WSelectHost(entry)

grab $w
wm protocol $w WM_DELETE_WINDOW annullahost
tkwait window .specchio
destroy $w

}


#
#               #################################
#               #                               #
#               #       CommandHost             #
#               #                               #
#               #################################
#
# Procedura di -command per selezione host
#
proc CommandHost { valore } {

  global Lingua IconaModificata saved_image FileLingua tcl_platform WSelectHost

if { $valore == "" } { return "" }
if { $tcl_platform(platform) == "unix" } {
	# UNIX
	if { [ catch { exec /bin/ping $valore 56 1 } out ] ==0 } {
		# l'host esiste
		$WSelectHost(padre) addhistory $valore
		$WSelectHost(okbutton) configure -state active
		update idletasks
		focus $WSelectHost(okbutton)
		return $valore
		} else {
		# l'host non esiste
		update idletasks
		focus $WSelectHost(entry)
		return ""
		}
	} else {
	# WINDOWS
	if { [file exist //$valore/shared ]} {
		$WSelectHost(padre) insert end $valore
		$WSelectHost(okbutton) configure -state active
		return $valore
		} else {
		# l'host non esiste
		update idletasks
		focus $WSelectHost(entry)
		return ""
		}
	}
}



#
#               #################################
#               #                               #
#               #           okhost              #
#               #                               #
#               #################################
#
# Procedura di command tasto OK finestra di selezione HOST
#
proc okhost { } {

  global FileLingua WSelectHost Host editgrid

set Host [$WSelectHost(padre) cget -value]
$editgrid(host) configure -text "[CaricaLingua $FileLingua Host]: $Host"
WriteHistory [$WSelectHost(storia) get 0 end]
destroy .specchio
}

#
#               #################################
#               #                               #
#               #       annullahost             #
#               #                               #
#               #################################
#
# Procedura di command tasto annulla finestra di selezione HOST
#
proc annullahost { } {

  global FileLingua WSelectHost Host

set Host $WSelectHost(old)
WriteHistory [$WSelectHost(storia) get 0 end]
destroy .specchio
}
#
#               #################################
#               #                               #
#               #       MostraVersione          #
#               #                               #
#               #################################
#
# Mostra la versione sul message box
#
proc MostraVersione {w} {
global FileLingua VERSIONE

MessageBox $w "[CaricaLingua $FileLingua Versione] $VERSIONE"  yes
}


#################################
#				#
#	ReadHistory		#
#				#
#################################

proc ReadHistory {} {
	global HOME
	global tcl_platform

set lista ""

if [file exist $HOME/history] {
	set fId [open $HOME/history r]
	set lista [split [read $fId] \n]
	close $fId
	}
return $lista
}



#################################
#				#
#	WriteHistory		#
#				#
#################################

proc WriteHistory { lista } {
	global HOME
	global tcl_platform

if {[catch { set fId [open $HOME/history w] } out] == 0} {	
	foreach linea $lista {
		puts $fId $linea
	}
	close $fId
	}
return
}




#################################
#				#
#	validate		#
#				#
#################################

proc validate { widget Parametro Carattere } {
	global	ValoreParametri
	global	TipoParametri
	global	RangeParametri

set fuorirange 0
set comando "catch {if {! ($ValoreParametri($Parametro) $RangeParametri($Parametro)) } {
		set fuorirange 1
		} } out "

if { $TipoParametri($Parametro) == "I" } {
	#INTERO .... solo numeri
	if { $ValoreParametri($Parametro) != "" } {
		eval $comando
	}
	if { $RangeParametri($Parametro) == ">=0" } {
		# INTERO SOLO POSITIVO
		set expr {^[1-9]?[0-9]*([0-9]?e[0-9]?[0-9]*)?$}
	} else {
		# INTERO ANCHE NEGATIVO
		set expr {^[-+]?[0-9]*([0-9]?e[-+]?[0-9]*)?$}
	}
}

if { $TipoParametri($Parametro) == "R" } {
	#REALE
	if { $ValoreParametri($Parametro) != "" } {
		eval $comando
	}
	if { $RangeParametri($Parametro) == ">=0" } {
		# REALE SOLO POSITIVO
		set expr {^[0-9]?[0-9]*\.?[0-9]*([0-9]\.?e[0-9]?[0-9]*)?$}
	} else {
		# REALE ANCHE NEGATIVO
		set expr {^[-+]?[0-9]*\.?[0-9]*([0-9]\.?e[-+]?[0-9]*)?$}
	}
}


if { ! [regexp $expr $ValoreParametri($Parametro) ] || $fuorirange } {
	set indice [string last $Carattere $ValoreParametri($Parametro)]
	set tmp1 [string range $ValoreParametri($Parametro) 0 [expr $indice -1]]
	append tmp1 [string range $ValoreParametri($Parametro)\
							 [expr $indice + 1] end]
	set ValoreParametri($Parametro) $tmp1
	$widget icursor $indice
	}
}


################################################################################
#
# Funzione InvertiByte
# Data una stringa in ingresso i cui elementi a due a due rappresentano 
# dei byte in esadecimale, inverte l'ordine dei byte.
# Input: 
#       stringa da invertire
# Ritorna :
#       stringa invertita
################################################################################
#
proc InvertiByte { Stringa } {

  set LunghezzaStringa [string length $Stringa]
  incr LunghezzaStringa -2
  
  for {set indice $LunghezzaStringa} { $indice >= 0 } {incr indice -2} {
    append StringaInvertita [string range $Stringa $indice [expr $indice + 1]]
  }
  return $StringaInvertita
}



################################################################################
#
# Funzione Nbytes
# Dato un numero di bits ritorna quanti bytes servono per rappresentarlo
# Input: 
#       bits
# Ritorna :
#       numero bytes
################################################################################
#
proc Nbytes { bits } {
  return [expr (($bits -1) / 8) +1]
}





################################################################################
#
# Funzione SetMatrice
# setta il segnale di I/O al valore dato in input tenendo conto della copia
# Input: 
#       NomeVariabile TipoIO Lunghezza linea valore
#
################################################################################
#
proc SetMatrice { NomeVariabile TipoIO Lunghezza linea valore } {
global Matrice

upvar \#0 $Matrice(page) ary
upvar \#0 $Matrice(copypage) copyary

if { ! [info exist copyary($linea,1)] } {
	set copyary($linea,1) ""
	} 

switch -- $TipoIO {
	"1" {
		if { $Lunghezza == 1 } {
			if { $valore == "1" } {
				set valore "ON"
				} elseif { $valore == "0" } {
					set valore "OFF"
					}
			if { $valore == "OFF" && $copyary($linea,1) != 0 } {
				set ary($linea,1) "OFF"
				set copyary($linea,1) 0
				$Matrice(widget) tag cell INP-OFF $linea,1
				}
			if { $valore == "ON" && $copyary($linea,1) != 1 } {
				set ary($linea,1) "ON"
				set copyary($linea,1) 1
				$Matrice(widget) tag cell INP-ON $linea,1
				}
			} else {
			set ary($linea,1) $valore
			set copyary($linea,1) $valore
			$Matrice(widget) tag cell INP-OFF $linea,1
			}
		}
	"2" {
		if { $Lunghezza == 1 } {
			if { $valore == "1" } {
				set valore "ON"
				} elseif { $valore == "0" } {
					set valore "OFF"
					}
			if { $valore == "OFF" && $copyary($linea,1) != 0 } {
				set ary($linea,1) "OFF"
				set copyary($linea,1) 0
				$Matrice(widget) tag cell OUT-OFF $linea,1
				}
			if { $valore == "ON" && $copyary($linea,1) != 1 } {
				set ary($linea,1) "ON"
				set copyary($linea,1) 1
				$Matrice(widget) tag cell OUT-ON $linea,1
				}
			} else {
			set ary($linea,1) $valore
			set copyary($linea,1) $valore
			$Matrice(widget) tag cell OUT-OFF $linea,1
			}
		}
	defaut {
		}
	}
}



################################################################################
#
# Funzione InterpretaOpzioni
# Input: 
# argv ==> lista con gli argomenti della linea di comando
# argc ==> numero elementi nella lista
#
################################################################################

proc InterpretaOpzioni { w argv argc } {
global	NomeFile AutoCaricamento SAVEPATH
set AutoCaricamento "False"

for { set i 0} {$i < [llength $argv] } {incr i} {
	switch -- [lindex $argv $i] {
		"-f" {
			if { $i < [ expr [llength $argv] -1] } {
				incr i
				puts [lindex $argv $i]
				set NomeFile "$SAVEPATH/[lindex $argv $i]"
				if [ file readable $NomeFile] {
					set AutoCaricamento "True"
					} else {
					MessageBox $w\
						"$NomeFile NOT FOUND" yes
					}
				} else {
				MessageBox $w\
				 "USO: Iosim \[-f <nomefile configurazione>\]"\
							 yes
				exit
				}
			}
		}
	}
}






######################################################################
# AdcInput : Apre la finestra di ENTRY per input analogico
######################################################################
proc AdcInput { CalWdg title valoreOld max min } {
  global font_usato FileLingua
  global value

  set NamePrompt ""
  set w .adcInput
  if [winfo exists $w] {
	return
  }

  toplevel $w

  wm title $w $title

  set value $valoreOld
  
  tixControl $w.field -variable value -min $min -max $max -value $value

  tixButtonBox $w.box -orientation horizontal
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato  -command "destroy $w" 
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command "
							set value $valoreOld
                                                    	destroy $w" 

  pack $w.field -side top -padx 10 -pady 10
  pack $w.box -side bottom -fill x

  wm transient $w [winfo toplevel $CalWdg]

  posiziona:finestre [winfo toplevel $CalWdg] $w          

  focus $w.field

  grab $w
  wm protocol $w WM_DELETE_WINDOW PrmEsc

  tkwait window $w
  return $value
}

