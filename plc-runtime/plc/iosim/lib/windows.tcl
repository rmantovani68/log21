
#               #################################
#               #                               #
#               #       SwitchHost		#
#               #                               #
#               #################################
proc    SwitchHost { padre } {

wm withdraw $padre
global FileLingua font_usato font_fisso tcl_platform Host okHost

set w [toplevel .host]
wm title $w "[CaricaLingua $FileLingua SelezionaHost]"

frame $w.top -border 1 -relief raised

set Host localhost
tixLabelEntry $w.top.a -label "Host: " \
	-options {
	    entry.width 30
	    entry.textVariable Host
	    label.anchor e
	    entry.anchor o
	}
bind [$w.top.a subwidget entry]  <Return>  "[format "okcmd %s" $w]"

pack $w.top.a -side top -fill x -expand yes
tixButtonBox $w.box -orientation horizontal
$w.box add ok -text Ok -underline 0 -command "[format "okcmd %s" $w]" -width 6
$w.box add exit     -text Exit     -underline 0 -command "exit" -width 6

pack $w.box -side bottom -fill x
pack $w.top -side top -fill both -expand yes

focus $w.top.a

tkwait variable okHost
destroy $w
update idletasks
wm deiconify $padre
return "OK"
}

proc okcmd { w } {
global	okHost Host FileLingua
if { [VerificaEsistenzaPlc] == 0 } {
	set okHost yes
	} else {
	MessageBox $w  "$Host: [CaricaLingua $FileLingua PlcAssente]"  yes
	}
}



#               #################################
#               #                               #
#               #       windows			#
#               #                               #
#               #################################
proc    windows { w } {

global FileLingua font_usato Matrice font_fisso SAVEPATH
global tcl_platform listbox BottoneStart BottoneStop BottoneReload BottoneAdd
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome

#### ICONE ####
global HOME FrecciaDestra IconaExit IconaStart IconaStop IconaReload

#
#  Creazione della MENUBAR principale
#
frame $w.top
frame $w.top.menubar -relief raised -bd 0

menubutton $w.top.menubar.mb1 -text [CaricaLingua $FileLingua File]\
		-menu $w.top.menubar.mb1.file -font $font_usato -underline 0 

set file [menu $w.top.menubar.mb1.file -tearoff 0 ]

$file add command -label [CaricaLingua $FileLingua Apri]\
		-command [format {Apri %s} $w] -font $font_usato\
		-underline 0
set BottoneApri(widget) $file
set BottoneApri(index)  0
$file add command -label [CaricaLingua $FileLingua Salva]\
		-command [format {Salva %s} $w] -font $font_usato\
		-underline 0
set BottoneSalva(widget) $file
set BottoneSalva(index)  1
$file add command -label [CaricaLingua $FileLingua Salvacome]\
		-command [format {SaveCome %s} $w] -font $font_usato\
		-underline 0
set BottoneSalvaCome(widget) $file
set BottoneSalvaCome(index)  2
$file add separator
$file add command -label [CaricaLingua $FileLingua Uscita]\
 		-command [format {Uscita %s} $w] -font $font_usato\
		-underline 2

menubutton $w.top.menubar.mb2 -text [CaricaLingua $FileLingua Edit]\
		-menu $w.top.menubar.mb2.edit -font $font_usato -underline 0

set edit [menu $w.top.menubar.mb2.edit -tearoff 0 ]
$edit add command -label "Cancella tutto"\
 		-command [format {Cancella %s} $w] -font $font_usato\
		-underline 2
set BottoneCancella(widget) $edit
set BottoneCancella(index)  0

pack $w.top.menubar -in $w.top -side left -fill x -expand true -anchor n
pack $w.top.menubar.mb1 -side left -fill x
pack $w.top.menubar.mb2 -side left -fill x
pack $w.top -side top -fill x 



#########################
# Disegno la status bar #
#########################
label $w.statusbar
pack $w.statusbar -side bottom -fill x -padx 3 -pady 1
tixBalloon $w.ballon -statusbar $w.statusbar

###################################################
# Disegno Label con il nome configurazione ATTIVA #
###################################################
set Matrice(NomeFile) "$SAVEPATH/NONAME.SIM"
set BoxNomeConf [frame $w.frame2 -bd 1 -relief ridge]
label $BoxNomeConf.lab -anchor w \
	 -text "[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)"
set Matrice(WidgetNomeFile) $BoxNomeConf.lab
pack $BoxNomeConf -side top -fill x -padx 2 -pady 2
pack $BoxNomeConf.lab -fill x -side top -padx 2 -pady 2

##################################
# Disegno TOOLBOX con i pulsanti #
##################################
set ToolBox [frame $w.frame1 -bd 1 -relief ridge]

set BottoneReload [button $ToolBox.iconareload -image $IconaReload\
				-command [format {Reload %s} $w] ]
pack $ToolBox.iconareload -in $ToolBox -side left

set BottoneAdd [button $ToolBox.frecciadestra -image $FrecciaDestra\
				-command [format {Aggiungi %s} $w]]
pack $ToolBox.frecciadestra -in $ToolBox -side left

set BottoneStart [button $ToolBox.iconastart -image $IconaStart\
				-command [format {StartAggiornamento %s} $w] ]
pack $ToolBox.iconastart -in $ToolBox -side left

set BottoneStop [button $ToolBox.iconastop -image $IconaStop\
		-command [format {Stop %s} $w] -state disabled ]
pack $ToolBox.iconastop -in $ToolBox -side left

pack $ToolBox -fill x -side top -padx 2 -pady 2



###################################
# Disegno la finestra splittabile #
###################################

set pane [tixPanedWindow $w.pane -orient horizontal]
pack $pane -expand yes -fill both -side right
set f1 [$pane add 1 -min 100 -expand 2]
set f2 [$pane add 2 -min 100 -expand 1]
$f1 config -relief flat
$f2 config -relief flat

set ListaWindows [frame $f1.frame -border 1 -relief raised]
set DebugWindow [frame $f2.frame -border 1 -relief raised]



###########################################
# Disegno la LISTBOX con gli Input/Output #
###########################################
set ListaVariabili [frame $ListaWindows.frame]
set listboxm [tixScrolledTList $ListaVariabili.lb -width 370\
		-height 130 -options {\
		tlist.orient horizontal\
		tlist.selectMode extended\
		} ]
set listbox $listboxm.tlist
pack $ListaVariabili -side top -fill both -expand yes
pack $listboxm -side left -fill both -expand yes
pack $ListaWindows -fill both -expand yes

bind $listbox <Double-1> [format {Aggiungi %s} $w]


################################################
# Disegno la MATRICE per le variabili in esame #
################################################

set Matrice(page) AA
set Matrice(copypage) A 
set Matrice(selezione) ""

set FrameDebug [frame $DebugWindow.frame]
set Matrice(widget) [table $FrameDebug.grid \
		-autoclear 1 \
		-variable $Matrice(page)\
		-titlerows 1 \
		-titlecols 0\
		-colstretch unset\
		-drawmode fast\
		-cols 2\
		-rows 1\
                -validate yes \
                -validatecommand { validate %c %S } \
                -browsecmd { browse %s %S } \
		-width 13 -height 10 ]
set Matrice(linee) 0
set Matrice(browse) 1
$Matrice(widget) activate 1,1
set Matrice(browse) 0
$Matrice(widget) tag config destro-ro -anchor e -state disabled
$Matrice(widget) tag config sinistro-ro -anchor w -state disabled
$Matrice(widget) tag config centro-ro -anchor c -state disabled
$Matrice(widget) tag configure INP-OFF -fg white -bg DarkGreen -relief sunken
$Matrice(widget) tag configure INP-ON  -fg black -bg green -relief raised
$Matrice(widget) tag configure OUT-OFF -fg white -bg red4 -relief sunken
$Matrice(widget) tag configure OUT-ON  -fg white -bg red -relief raised
$Matrice(widget) tag configure NOPLC  -bg gray -relief sunken

$Matrice(widget) tag row centro-ro 0
$Matrice(widget) tag col sinistro-ro 0 
$Matrice(widget) tag col destro-ro 1
$Matrice(widget) width 1 6

upvar \#0 $Matrice(page) ary
set ary(0,0) [CaricaLingua $FileLingua LabelIO]
set ary(0,1) [CaricaLingua $FileLingua Valore]

scrollbar $FrameDebug.sy -command [list $Matrice(widget) yview]
scrollbar $FrameDebug.sx -command [list $Matrice(widget) xview]\
					 -orient horizontal
$Matrice(widget) configure  -yscrollcommand [format { %s set } $FrameDebug.sy ]\
                 -xscrollcommand  [format { %s set } $FrameDebug.sx ] 

bind $FrameDebug.sy <FocusIn> "focus $Matrice(widget)"
bind $FrameDebug.sx <FocusIn> "focus $Matrice(widget)"

pack $FrameDebug.sx -in $FrameDebug -side bottom -fill x
pack $FrameDebug.sy -in $FrameDebug -side right -fill y
pack $Matrice(widget) -in $FrameDebug -side left -expand yes -fill both
pack $FrameDebug -side left -expand yes -fill both  -padx 3 -pady 3
pack $DebugWindow -fill both -expand yes

bind $Matrice(widget) <Alt-f> { }
bind $Matrice(widget) <Alt-F> { }

bind $Matrice(widget) <ButtonPress-3> {
    	set Matrice(selezione) [%W get @%x,%y]
	}
bind $Matrice(widget) <ButtonRelease-2> {
	break
	}
bind $Matrice(widget) <1> {
    ClickSuMatrice %W %x %y
    break
}
bind $Matrice(widget) <Motion> {
    break
}

bind $Matrice(widget) <Key-Up> {
	break
	}
bind $Matrice(widget) <Key-Down> {
	break
	}
bind $Matrice(widget) <Key-Left> {
	break
	}
bind $Matrice(widget) <Key-Right> {
	break
	}

PopUpTkTable $Matrice(widget)

set Matrice(toplevel) $w



#################################################
#						#
#	Definizione stringhe per HELP giallo	#
#						#
#################################################

$w.ballon bind $ToolBox.iconareload\
	-balloonmsg "Ricarica"\
	-statusmsg "Ricarica la lista variabili dal PLC" 
$w.ballon bind $ToolBox.frecciadestra -balloonmsg "Aggiungi" \
	-statusmsg "Aggiunge la variabile dalla lista variabili"
# $w.ballon bind $ToolBox.remove -balloonmsg "Cancella" \
#	-statusmsg "Cancella la variabile dalla lista variabili"
$w.ballon bind $ToolBox.iconastart -balloonmsg "Start" \
	-statusmsg "Avvia l'aggiornamento delle variabili"
$w.ballon bind $ToolBox.iconastop -balloonmsg "Stop" \
	-statusmsg "Ferma l'aggiornamento delle variabili"

}

