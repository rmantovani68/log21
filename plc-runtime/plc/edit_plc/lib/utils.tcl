
            ######################################
            #                                    #
            #     File di funzioni di carattere  #
            #     generale                       #
            #                                    #
            ######################################

#####################################################################
# Opzioni dei metawidget :
#
option add *Rule.relief sunken widgetDefault
option add *Rule.width 2 widgetDefault
option add *Rule.height 2 widgetDefault
option add *Rule.borderWidth 1 widgetDefault
option add *Filler.relief flat widgetDefault
option add *Filler.width 10 widgetDefault
option add *Filler.height 10 widgetDefault

#############################################################
# Procedura CreateImagePixmap
# Con tale funzione si creano i pixmap una volta sola
# Input: 
#   Nessuno
# Ritorna :
#   Niente
#############################################################
proc CreateImagePixmap { } {
global Home EditPrefs SiIoconf

  if [file exist $Home/icone/OK.png] {
    image create photo ok -file $Home/icone/OK.png
  }
  if [file exist $Home/icone/CANCEL.png] {
    image create photo cancel -file $Home/icone/CANCEL.png
  }
  if [file exist $Home/icone/info4.png] {
    image create photo info4 -file $Home/icone/info4.png
  }
  if [file exist $Home/icone/095.png] {
    image create photo confirm -file $Home/icone/095.png
  }
  if [file exist $Home/icone/ZIP.png] {
    image create photo zip -file $Home/icone/ZIP.png
  }
  if [file exist $Home/icone/xncPROG.png] {
    image create photo xncPROG -file $Home/icone/xncPROG.png
  }
  if [file exist $Home/icone/xncLOCAL.png] {
    image create photo xncLOCAL -file $Home/icone/xncLOCAL.png
  }
  if [file exist $Home/icone/ADDCOLL.png] {
    image create photo addcoll -file $Home/icone/ADDCOLL.png
  }
  if [file exist $Home/icone/ADDCOLR.png] {
    image create photo addcolr -file $Home/icone/ADDCOLR.png
  }
  if [file exist $Home/icone/ADDROW.png] {
    image create photo addrow -file $Home/icone/ADDROW.png
  }
  if [file exist $Home/icone/DELCONN.png] {
    image create photo delconn -file $Home/icone/DELCONN.png
  }
  if [file exist $Home/icone/wipeout.png] {
    image create photo wipeout -file $Home/icone/wipeout.png
  }
  if [file exist $Home/icone/debug.png] {
    image create photo debug -file $Home/icone/debug.png
  }
  if [file exist $Home/icone/filesys.png] {
    image create photo filesys -file $Home/icone/filesys.png
  }
  if [file exist $Home/icone/notepd.png] {
    image create photo notepd -file $Home/icone/notepd.png
  }
  if [file exist $Home/icone/magic.png] {
    image create photo magic -file $Home/icone/magic.png
  }
  if [file exist $Home/icone/convert3.png] {
    image create photo convert3 -file $Home/icone/convert3.png
  }
  if [file exist $Home/icone/delete.png] {
    image create photo delete -file $Home/icone/delete.png
  }
  if [file exist $Home/icone/search.png] {
    image create photo search -file $Home/icone/search.png
  }
  if [file exist $Home/icone/replace.png] {
    image create photo replace -file $Home/icone/replace.png
  }
  if [file exist $Home/icone/replall.png] {
    image create photo replaceall -file $Home/icone/replall.png
  }
  if [file exist $Home/icone/savevar.png] {
    image create photo save -file $Home/icone/savevar.png
  }
  if [file exist $Home/icone/parametr.png] {
    image create photo parametri -file $Home/icone/parametr.png
  }
  if [file exist $Home/icone/connect.png] {
    image create photo connect -file $Home/icone/connect.png
  }
  if [file exist $Home/icone/free.png] {
    image create photo free -file $Home/icone/free.png
  }
  if [file exist $Home/icone/freeall.png] {
    image create photo freeall -file $Home/icone/freeall.png
  }
  if [file exist $Home/icone/default.png] {
    image create photo default -file $Home/icone/default.png
  }
  if [file exist $Home/icone/OKALL.png] {
    image create photo okall -file $Home/icone/OKALL.png
  }
  if [file exist $Home/icone/OPEN.png] {
    image create photo loadfile -file $Home/icone/OPEN.png
  }
  if [file exist $Home/icone/BACKUP.png] {
    image create photo backup -file $Home/icone/BACKUP.png
  }
  if [file exist $Home/icone/PRINTER.png] {
    image create photo printer -file $Home/icone/PRINTER.png
  }
  if [file exist $Home/icone/OPENEXE.png] {
    image create photo openexe -file $Home/icone/OPENEXE.png
  }
  if [file exist $Home/icone/SAVE.png] {
    image create photo savefile -file $Home/icone/SAVE.png
  }
  if [file exist $Home/icone/WINDOWS.png] {
    image create photo windows -file $Home/icone/WINDOWS.png
  }
  if [file exist $Home/icone/DEBDEF.png] {
    image create photo debdef -file $Home/icone/DEBDEF.png
  }
  if [file exist $Home/icone/EDTSEG.png] {
    image create photo edtseg -file $Home/icone/EDTSEG.png
  }
  if [file exist $Home/icone/aggscamb.png] {
    image create photo aggscamb -file $Home/icone/aggscamb.png
  }
  if { $SiIoconf == "yes" } {
    if [file exist $Home/icone/activIO.png] {
      image create photo activio -file $Home/icone/activIO.png
    }
    if [file exist $Home/icone/deactIO.png] {
      image create photo deactio -file $Home/icone/deactIO.png
    }
  }
}

#################################################################
# Procedura InputArgv
# Con tale funzione si processano gli argomenti che si possono
# dare all' editor
# Input: 
#   argv
# Ritorna :
#   Niente
#################################################################
proc InputArgv {argv} {
global iconic SiIoconf IoconfModificato
global Demo VersioneEditor tcl_platform 
global predir nc1000

  set iconic "no"
  set SiIoconf "no"
  set IoconfModificato "no"
  set Demo "no"
  set predir cn
  set nc1000 0

  foreach argomenti $argv {
    if { $argomenti == "-iconic" } {
      set iconic "yes"
    } elseif { $argomenti == "-SiIoconf" } {
      set SiIoconf "yes"
    } elseif { $argomenti == "-Demo" } {
      set Demo "yes"
    } elseif { $argomenti == "-nc1000" } {
      set nc1000 1 
    } elseif { [string first "-predir=" $argomenti] != -1 } {
      set predir [string range $argomenti [string length "-predir="] end]
    } elseif { $argomenti == "-Help" || $argomenti == "--h"} {
      if {$tcl_platform(platform) == "unix"} {
        puts "Editor plc versione $VersioneEditor\n"
        puts "Versione file ctclcni [CtclcniVer]\n"
        puts "Elenco ozioni ammesse\n"
        puts "-iconic   attiva il processo iconizzato"
        puts "-SiIoconf attiva la possibilita' di definire il \
             file di configurazione io"
        puts "-nc1000   personalizza l'editor per nc1000"
        puts "-predir=<dir>  direttorio associato a nc1000"
        puts "-Help     visualizza questo help"
      } else {
        set Aiuto "Editor plc versione $VersioneEditor\n\n\
        Versione file ctclcni [CtclcniVer]\n\n\
        Elenco ozioni ammesse\n\n\
        -iconic   attiva il processo iconizzato\n\
        -SiIoconf attiva la possibilita' di definire il \
             file di configurazione io\n\
        -nc1000   personalizza l'editor per nc1000\n\
        -predir=<dir>  direttorio associato a nc1000\n\
        -Help     visualizza questo help"
        tk_messageBox -icon info -message $Aiuto -type ok 
      }
      exit
    } 
  }
}

####################################################################
# Procedura ParseArgs
# Con tale funzione si assegnano dei valori alle variabili nella
# procedura chiamante 
# Input: 
#   arglist  Lista delle opzioni da processare composta
#            da una opzione e seguita dal valore che vogliamo
#            fargli assumere
# Ritorna :
#   Niente
####################################################################
proc ParseArgs { arglist } {
upvar args args
  
  foreach pair $arglist {
    set option [lindex $pair 0]
    # Il valore di default e' nullo se non specificato
    set default [lindex $pair 1]		
    set index [lsearch -exact $args "-$option"]
    if {$index != -1} {
      set index1 [expr {$index + 1}]
      set value [lindex $args $index1]
      # Variabile del chiamante "$option"
      uplevel 1 [list set $option $value]	
      set args [lreplace $args $index $index1]
    } else {
      uplevel 1 [list set $option $default]	
    }
  }
  return 0
}

####################################################################
# Procedura DefaultButton 
# Con tale funzione si assegna la callback di un bottone a piu'
# widget dati in input
# Input: 
#   button   Nome del bottone la cui callback e' chiamata
#   args     Lista dei widget a cui associare il bind che
#            chiama la callback
# Ritorna :
#   Niente
####################################################################
proc DefaultButton { button args } {
  foreach w $args {
    bind $w <Return> [list catch [list $button invoke]]
    bind $w <Return> "+\nbreak"
  }
}

####################################################################
# Procedura SeparationLine
# Procedura di creazione di una linea di separazione tra widget
# Input: 
#   parent    Widget genitore
# Ritorna :
#   Nome del separatore
####################################################################
proc SeparationLine { parent } {
  global RuleNumber

  if {$parent == "."} {set parent ""}
  
  if {[info exists RuleNumber]} then {
    set RuleNumber [expr {$RuleNumber + 1}]
  } else {
    set RuleNumber 0
  }

  set rule "$parent.rule$RuleNumber"
  frame $rule -class Rule
  return $rule
}

####################################################################
# Procedura FillerFrame
# Procedura di creazione di un frame di riempimento
# Input: 
#   parent    Widget genitore
# Ritorna :
#   Nome del frame
####################################################################
proc FillerFrame { {parent {}} args} {
global FrameFiller

  if {$parent == "."} {set parent ""}
  
  if {[info exists FrameFiller]} then {
    set FrameFiller [expr {$FrameFiller + 1}]
  } else {
    set FrameFiller 0
  }

  set filler "$parent.filler$FrameFiller"
  frame $filler -class Filler
  if {$args != ""} {eval $filler configure $args}
  lower $filler
  return $filler
}

####################################################################
# Procedura PosizionaFinestre
# Procedura che posiziona le finestre al centro della finestra 
# principale
# Input: 
#   MainW    Nome della finestra principale
#   w        Nome della finestra da posizionare
#   place    flag per il posizionamento
# Ritorna :
#   Niente
####################################################################
proc PosizionaFinestre {MainW w place} {

  set top_default .jedit0

  if {$MainW  == "" } {
    set main $top_default 
  } else {
    set main $MainW
  }

  wm withdraw $w
  update idletasks

  # calculate position:
  # il 7  ed il 25 sono relative ai bordi delle finestre
  if { [winfo exist $w ] == 0 } {
    return
  }

  if { $place == "c" } {
    set x [expr [winfo rootx $main] - 7 + [winfo width $main]/2 - [winfo reqwidth $w]/2]
    set y [expr [winfo rooty $main] - 25 + [winfo height $main]/2 \
          - [winfo reqheight $w]/2]
  } elseif { $place == "u" } {
    set x [expr [winfo rootx $main] - 7 + [winfo width $main]/2 - [winfo reqwidth $w]/2]
    set y [winfo rooty $main]
  } else {
    set x [expr [winfo rootx $main] - 7 + [winfo width $main] - [winfo reqwidth $w]]
    set y [expr [winfo rooty $main] - 25] 
  }

  wm geom $w +$x+$y
  update idletasks  
  wm deiconify $w    
  update idletasks  
  wm focus $w      
}

####################################################################
# Procedura SetModiFile 
# Procedura che pone ModiFile=1 se ci sono le condizioni
# Input: 
#   t                Nome della finestra di testo
#   ResetEquazioni   Flag per il reset delle equazioni ladder
# Ritorna :
#   Niente
####################################################################
proc SetModiFile { t ResetEquazioni} {
global WindowType MainOfFunctionBlock ModiFile 

  set w [TextToTop $t]
  if { $w==".jedit0" || $WindowType($w)=="fb" && $MainOfFunctionBlock($w)==".jedit0" } {
    set ModiFile 1
  }
  if { $ResetEquazioni == 1 && [info exists ContactDef(linee_equazioni)]} {
    unset ContactDef(linee_equazioni)
    DisplayCurrentLine $t
  }
}

####################################################################
# Procedura SwapWindowsVariable 
# Procedura che fa il cambio delle varibili globali relative al 
# file caricato al cambio del focus
# Input: 
#   newwin           Nome della nuova finestra 
# Ritorna :
#   Niente
####################################################################
proc SwapWindowsVariable {newwin} {
global FocusWindow
global FunctionBlockList FunctionBlockList$FocusWindow FunctionBlockList$newwin
global VariableList VariableList$FocusWindow VariableList$newwin
global elenco_moduli elenco_moduli$FocusWindow elenco_moduli$newwin
global LineNumber LineNumberVect
global ProgramComponent ProgramComponent$FocusWindow ProgramComponent$newwin
global PredefinedFunctionBlock PredefinedFunctionBlock$FocusWindow PredefinedFunctionBlock$newwin
global ModiFile ModiFileVect
global OffsetLineaProg OffsetLineaProg1
global WindowType MainOfFunctionBlock


  # Salvo la configurazione di variabili della finestra che lascio
  if [winfo exists $FocusWindow] {
    # Prima delle variabili
    if {[info exists VariableList$FocusWindow] == 1 } {
      unset VariableList$FocusWindow
    }
    if {[info exists VariableList] == 1 } {
      array set VariableList$FocusWindow [array get VariableList]
    }
     
    # Poi dei function block
    if {[info exists FunctionBlockList$FocusWindow] == 1 } {
      unset FunctionBlockList$FocusWindow
    }
    if {[info exists FunctionBlockList] == 1 } {
      array set FunctionBlockList$FocusWindow [array get FunctionBlockList]
    }

    # Poi della definizione dei moduli
    if {[info exists elenco_moduli$FocusWindow] == 1 } {
      unset elenco_moduli$FocusWindow
    }
    if {[info exists elenco_moduli] == 1 } {
      array set elenco_moduli$FocusWindow [array get elenco_moduli]
    }
    if {[info exists LineNumberVect($FocusWindow)] == 1 } {
      unset LineNumberVect($FocusWindow)
    }
    if {[info exists LineNumber] == 1 } {
      set LineNumberVect($FocusWindow) $LineNumber
    }

    # per gli array ProgramComponent e PredefinedFunctionBlock non effettuo
    # uno swap se le finestre sono padre e figlio 
    set LeaveWindow 0
    if {$WindowType($newwin)=="main" && $WindowType($FocusWindow)=="main" } {
      set LeaveWindow $FocusWindow
    }
    if {$WindowType($newwin)=="fb" && $WindowType($FocusWindow)=="main" } {
      if {$FocusWindow != $MainOfFunctionBlock($newwin) } {
        set LeaveWindow $FocusWindow
      }
    }
    if {$WindowType($newwin)=="main" && $WindowType($FocusWindow)=="fb" } {
      if {$newwin != $MainOfFunctionBlock($FocusWindow) } {
        set LeaveWindow $MainOfFunctionBlock($FocusWindow)
      }
    }
    if {$WindowType($newwin)=="fb" && $WindowType($FocusWindow)=="fb" } {
      if {$MainOfFunctionBlock($FocusWindow) != $MainOfFunctionBlock($newwin) } {
        set LeaveWindow $MainOfFunctionBlock($FocusWindow)
      }
    }

    if {$LeaveWindow != 0 } {
      global ProgramComponent$LeaveWindow PredefinedFunctionBlock$LeaveWindow 
      # Poi della definizione dei Function Block utente
      if {[info exists ProgramComponent$LeaveWindow] == 1 } {
        unset ProgramComponent$LeaveWindow
      }
      if {[info exists ProgramComponent] == 1 } {
        array set ProgramComponent$LeaveWindow [array get ProgramComponent]
      }
      # Poi della definizione della lista Function Block utente + custom
      if {[info exists PredefinedFunctionBlock$LeaveWindow] == 1 } {
        unset PredefinedFunctionBlock$LeaveWindow
      }
      if {[info exists PredefinedFunctionBlock] == 1 } {
        array set PredefinedFunctionBlock$LeaveWindow [array get PredefinedFunctionBlock]
      }
    }

    # Poi dell'indicatore di file modificato
    if {[info exists ModiFile] == 1 } {
      set ModiFileVect($FocusWindow) $ModiFile
    } else {
      set ModiFileVect($FocusWindow) 0
    }

    # Poi della varibile che tiene conto dell'offset vale a dire delle linee
    # che non si vedono 
    if {[info exists OffsetLineaProg] == 1} {
      set OffsetLineaProg1($FocusWindow) $OffsetLineaProg
    }
  }

  # Aggiorno la situazione nella finestra in cui vado
  # Prima delle variabili
  if {[info exists VariableList] == 1 } {
    unset VariableList
  }
  if {[info exists VariableList[winfo toplevel $newwin]] == 1 } {
    array set VariableList [array get VariableList[winfo toplevel $newwin]]
  } 

  # Poi dei function block
  if {[info exists FunctionBlockList] == 1 } {
    unset FunctionBlockList
  }
  if {[info exists FunctionBlockList[winfo toplevel $newwin]] == 1 } {
    array set FunctionBlockList [array get FunctionBlockList[winfo toplevel $newwin]]
  }

  # per gli array ProgramComponent e PredefinedFunctionBlock non effettuo
  # uno swap se le finestre sono padre e figlio 
  set EnterWindow 0
  if {$WindowType($newwin)=="main" && $WindowType($FocusWindow)=="main" } {
    set EnterWindow $newwin
  }
  if {$WindowType($newwin)=="fb" && $WindowType($FocusWindow)=="main" } {
    if {$FocusWindow != $MainOfFunctionBlock($newwin) } {
      set EnterWindow $MainOfFunctionBlock($newwin)
    }
  }
  if {$WindowType($newwin)=="main" && $WindowType($FocusWindow)=="fb" } {
    if {$newwin != $MainOfFunctionBlock($FocusWindow) } {
      set EnterWindow $newwin
    }
  }
  if {$WindowType($newwin)=="fb" && $WindowType($FocusWindow)=="fb" } {
    if {$MainOfFunctionBlock($FocusWindow) != $MainOfFunctionBlock($newwin) } {
      set EnterWindow $MainOfFunctionBlock($newwin)
    }
  }

  if {$EnterWindow != 0} {
    global ProgramComponent$EnterWindow PredefinedFunctionBlock$EnterWindow 
    # Poi dei function block utente
    if {[info exists ProgramComponent] == 1 } {
      unset ProgramComponent
    }
    if {[info exists ProgramComponent[winfo toplevel $EnterWindow]] == 1 } {
      array set ProgramComponent [array get ProgramComponent[winfo toplevel $EnterWindow]]
    }
    # Poi della lista function block utente + custom
    if {[info exists PredefinedFunctionBlock] == 1 } {
      unset PredefinedFunctionBlock
    }
    if {[info exists PredefinedFunctionBlock[winfo toplevel $EnterWindow]] == 1 } {
      array set PredefinedFunctionBlock [array get \
        PredefinedFunctionBlock[winfo toplevel $EnterWindow]]
    } else {
      PredefinedFunctionBlockListPreset
    }
  }

  # Poi dell'indicatore di file modificato
  if {[info exists ModiFileVect([winfo toplevel $newwin])] == 1 } {
    set ModiFile $ModiFileVect([winfo toplevel $newwin])
  } else {
    set ModiFile 0
  }

  # Poi della definizione dei moduli
  if {[info exists elenco_moduli] == 1 } {
    unset elenco_moduli
  }
  if {[info exists elenco_moduli[winfo toplevel $newwin]] == 1 } {
    array set elenco_moduli [array get elenco_moduli[winfo toplevel $newwin]]
  } else {
    ResetBoardList
  }
  if {[info exists LineNumber] == 1 } {
    unset LineNumber
  }
  if {[info exists LineNumberVect([winfo toplevel $newwin])] == 1 } {
    set LineNumber $LineNumberVect([winfo toplevel $newwin])
  } else {
    set LineNumber 1
  }

  # Poi delle linee nascoste
  if {[info exists OffsetLineaProg1([winfo toplevel $newwin])] == 1 } {
    set OffsetLineaProg $OffsetLineaProg1([winfo toplevel $newwin])
  }

  set FocusWindow [winfo toplevel $newwin]
}

####################################################################
# Procedura WaitWindow
# Procedura che attiva il cursore in modalita' orologio
# e la disattiva
# Input: 
#   t    	Nome del widget di testo
#   FlagCursor  0 Ripristino il cursore 1 Cursore ad orologio
# Ritorna :
#   Niente
####################################################################
proc WaitWindow { t FlagCursor } {
global EditPrefs Home

  if { $FlagCursor == 1 } {
    #$t config -cursor "@$Home/icone/cless.png \
      $Home/icone/clessmsk.png midnightblue lemonchiffon"
    #[winfo toplevel $t] config -cursor \
      "@$Home/icone/cless.png $Home/icone/clessmsk.png \
      midnightblue lemonchiffon"
    $t config -cursor watch 
    [winfo toplevel $t] config -cursor watch
  } else {
    $t config -cursor left_ptr
    [winfo toplevel $t] config -cursor left_ptr
  }
}

####################################################################
# Procedura AbsoluteColumn
# Procedura che che calcola la colonna assoluto
# che fa da riferimento quando si scrolla in su e in giu'.
# Input: 
#   t    	Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc AbsoluteColumn { t } {

  global ColonnaAssoluta EditPrefs

  set insertindex [split [$t index insert] {.}]
  set column [lindex $insertindex 1]
  set ContenutoLinea [$t get "insert linestart" "insert lineend"]
  set ColonnaAssoluta 0
  set TmpIndice 0 
  # Calcolo il valore assoluto della lunghezza della linea risolvendo
  # i tab assumendoli di 8 caratteri
  for {set i 0 } { $i < $column } {incr i} {
    if { [string range $ContenutoLinea $i $i] == "\t" } {
      set ColonnaAssoluta [expr (($ColonnaAssoluta / $EditPrefs(tablength)) + 1) * $EditPrefs(tablength) ] 
    } else {
      incr ColonnaAssoluta
    }
  }
}

#################################################################################
# Procedura InvertiByte
# Data una stringa in ingresso i cui elementi a due a due rappresentano 
# dei byte in esadecimale, inverte l'ordine dei byte.
# Input: 
#     	stringa da invertire
# Ritorna :
# 	stringa invertita
#################################################################################
proc InvertiByte { Stringa } {

  set LunghezzaStringa [string length $Stringa]
  incr LunghezzaStringa -2
  
  for {set indice $LunghezzaStringa} { $indice >= 0 } {incr indice -2} {
    append StringaInvertita [string range $Stringa $indice [expr $indice + 1]]
  }
  return $StringaInvertita
}


#################################################################################
# Procedura Ktooltip_post
# Con tale funzione si attiva l'help giallo su un bottone
# Input: 
#     	w	oggetto per cui si vuole l'help
#     	message	messaggio che si vuole visulizzare
#     	font	font del messaggio
# Ritorna :
# 	Niente
#################################################################################
proc Ktooltip_post {w message font} {
global ToolTipStatus tcl_platform

   if {[info exists ToolTipStatus($w)]} {
      if {$ToolTipStatus($w) == 0} { return }
   }

   if { [winfo exists $w] == 0 } {
     return
   }

   set orig_x 0
   set orig_y 0
   set size_x 0
   set size_y 0

   set mytoplevel [winfo toplevel $w]
   set wp $w

   set pass	0

   while {$wp != $mytoplevel} {
      #
      # get the geometry of $wp and ``add'' it to orig_x, orig_y
      #
      # puts "wp = $wp"
      scan [winfo geometry $wp] "%dx%d+%d+%d" a b x y
      set orig_x [expr $orig_x + $x]
      set orig_y [expr $orig_y + $y]
      if {$pass == 0} {
         set size_x $a
         set size_y $b
      }
      incr pass
      #
      # now remove the trailing .xxx widget name from $wp
      #
      set dot [string last {.} $wp]

      if {$dot > 0} { set wp [string range $wp 0 [expr $dot - 1]] } \
      else          { set wp {.} }
   }

   #
   # and lastly we add-in the geometry of our toplevel
   #

   scan [winfo geometry $mytoplevel] "%dx%d+%d+%d" a b x y
   set orig_x [expr $orig_x + $x]
   set orig_y [expr $orig_y + $y]

   # puts "final geometry of $w is ${size_x}x${size_y}+${orig_x}+$orig_y"

   if {[catch {toplevel .tooltip}] == 0} {
      wm title .tooltip tooltip
      wm overrideredirect .tooltip true
      set tip_xorig [expr $orig_x + int( 0.5 * $size_x )]
      if {$tcl_platform(platform) == "unix"} {
        set tip_yorig [expr $orig_y + $size_y]
      } else {
        # Nel caso Windows se il mouse punta la parte 
        # bassa del bottone Tooltip e mouse si sovrappongono
        # e viene chiamata una Leave impropriamente
        set tip_yorig [expr $orig_y + $size_y + 20]
      }

      label .tooltip.l -text $message -bd 1 -relief solid -bg yellow -font $font 

      set LarghezzaMessaggio [font measure $font $message]
      incr LarghezzaMessaggio 10
      set AltezzaMessaggio [font metrics $font -linespace]
      incr AltezzaMessaggio 10
      set LarghezzaSchermo [winfo screenwidth .tooltip.l]
      set AltezzaSchermo [winfo screenheight .tooltip.l]

      if { [expr $tip_xorig + $LarghezzaMessaggio] > $LarghezzaSchermo } {
        set tip_xorig [expr $LarghezzaSchermo - $LarghezzaMessaggio]
      }
      if { [expr $tip_yorig + $AltezzaMessaggio] > $AltezzaSchermo } {
        set tip_yorig [expr $AltezzaSchermo - $AltezzaMessaggio]
      }

      wm geometry .tooltip +$tip_xorig+$tip_yorig

      pack .tooltip.l -ipadx 2 -ipady 2
   }

}

#################################################################################
# Procedura Ktooltip_unpost
# Con tale funzione si toglie l'help giallo su un bottone
# Input: 
#	Nessuno
# Ritorna :
# 	Niente
#################################################################################
proc Ktooltip_unpost {w} {
global ToolTipStatus

   set ToolTipStatus($w) 0
   catch {destroy .tooltip}
   return
}

#################################################################################
# Procedura CniBalloon
# Con tale funzione si associa l'help giallo ad un bottone
# Input: 
#     	w	oggetto per cui si vuole l'help
#     	message	messaggio che si vuole visulizzare
#     	font	font del messaggio
#     	delay	ritardo di visualizzazione
# Ritorna :
# 	Niente
#################################################################################
proc CniBalloon { widget tiptext font delay} {
global ToolTipStatus

  bind $widget <Enter> "  \
     if {[string length $tiptext]} {
       set ToolTipStatus($widget) 1
       after $delay {Ktooltip_post $widget \"$tiptext\" $font}
    } \
    "

  bind $widget <Leave> " \
      if {[string length $tiptext]} {
	 Ktooltip_unpost  $widget
      } \
      "
  bind $widget <Configure> " \
      if {[string length $tiptext]} {
	 Ktooltip_unpost  $widget
      } \
      "
  bind $widget <Destroy> " \
      if {[string length $tiptext]} {
	 Ktooltip_unpost  $widget
      } \
      "
}

#################################################################################
# Procedura TrovaFile
# Con tale procedura si ricerca dove e' il file specificato in ingresso
# andando a cercare prima in /home/xnc/bin e poi /home/edit_plc/util
# pagato
# Input: 
#     	FileDaCercare   File che si desidera cercare
# Ritorna :
# 	Percorso in cui si trova il file
#       -1 File non trovato
#################################################################################
proc TrovaFile { FileDaCercare  } {
global EditPrefs Home

  if [file exists $EditPrefs(binpath)/$FileDaCercare] {
    return $EditPrefs(binpath)
  } elseif [file exists $Home/util/$FileDaCercare] {
    return $Home/util
  } else {
    return -1
  }
}

#################################################################################
# Procedura ApriFileScambio
# Con tale procedura si aprono i file di scambio scatici e dinamici
# Input: 
#     	Nessuno
# Ritorna :
# 	Niente
#################################################################################
proc ApriFileScambio { } {
global EditPrefs Home input QualeScambio
global input2 QualeScambdin

  # apro il file che contiene i segnali di scambio
  set Dove [TrovaFile $EditPrefs(qualescambio)]
  if { $Dove != -1 } {
    set input [open $Dove/$EditPrefs(qualescambio) r]
    set QualeScambio $Dove/$EditPrefs(qualescambio)
  } else {
    set input -1
    set QualeScambio ""
    AlertBox -text "[GetString FileNonTrovato]\n$EditPrefs(qualescambio)"
  }
  
  # apro il file che contiene i segnali di scambio dinamici
  set Dove [TrovaFile $EditPrefs(qualescambdin)]
  if { $Dove != -1 } {
    set input2 [open $Dove/$EditPrefs(qualescambdin) r]
    set QualeScambdin $Dove/$EditPrefs(qualescambdin)
  } else {
    set input2 -1
    set QualeScambdin ""
    AlertBox -text "[GetString FileNonTrovato]\n$EditPrefs(qualescambdin)"
  }
  RimuoviLibrerie
}

#################################################################################
# Procedura EsistenzaFloppy
# Verifica la presenza del floppy
# Input: 
#     	Nessuno
# Ritorna :
# 	Niente
#################################################################################
proc EsistenzaFloppy { } {

  set Presente 0
  set OldPath [pwd]
  if [catch { cd a: } result] {
    set Presente 0
  } else {
    set Presente 1
  }
  cd c:
  cd $OldPath
  return $Presente
}

#################################################################################
# Procedura RimuoviLibrerie
# Rimuove nel direttorio library gli
# eventuali *.o presenti 
# Input: 
#     	Nessuno
# Ritorna :
# 	Niente
#################################################################################
proc RimuoviLibrerie { } {
global Home
 
  foreach i [lsort "[glob -nocomplain $Home/library/*.o]"] {
    if {$i != "." && $i != ".."} {
      if {![file isdirectory ./$i]} {
        file delete "$i"
      }
    } 
  }
}

#################################################################################
# Procedura VerificaDemo
# Con tale procedura si verifica se l'utilizzatore ha
# pagato
# Input: 
#     	Nessuno
# Ritorna :
# 	Niente
#################################################################################
proc VerificaDemo { t  } {
global NumeroChiamate

  if { [ProsVerificaProtezione] == "VBIT_LOW" } {
    if { $NumeroChiamate == 7 } {
      AlertBox -text MsgDemo
      CmdQuit $t
    }
    incr NumeroChiamate
    after 150000 "VerificaDemo $t"
  }
}
