
            ######################################
            #                                    #
            #     File di gestione dei bind      #
            #     del widget di testo            #
            #                                    #
            ######################################

####################################################################
# Procedura DefineBind
# Definizione dei bind del widget di testo e della finestra 
# principale
# Input: 
#   window    Nome della finestra principale
# Ritorna :
#   Niente
####################################################################
proc DefineBind { window } {
  global FocusWindow DebugState AttivatoDebug 
  global tcl_platform 

  if { $tcl_platform(platform) != "unix"} {
    # Pezza per gestire gli Alt + lettera nel caso windows
    bind Text <Alt-f> {
      if { [string first "jedit" %W] != -1} {
        set WdgMenu [winfo toplevel %W].menu.file
        $WdgMenu.m post [winfo rootx $WdgMenu] [expr [winfo rooty $WdgMenu] + \
          [winfo height $WdgMenu]]
      }
    }
    bind Text <Alt-e> {
      if { [string first "jedit" %W] != -1} {
        set WdgMenu [winfo toplevel %W].menu.edit
        $WdgMenu.m post [winfo rootx $WdgMenu] [expr [winfo rooty $WdgMenu] + \
          [winfo height $WdgMenu]]
      }
    }
    bind Text <Alt-o> {
      if { [string first "jedit" %W] != -1} {
        set WdgMenu [winfo toplevel %W].menu.user
        $WdgMenu.m post [winfo rootx $WdgMenu] [expr [winfo rooty $WdgMenu] + \
          [winfo height $WdgMenu]]
      }
    }
    bind Text <Alt-s> {
      if { [string first "jedit" %W] != -1} {
        set WdgMenu [winfo toplevel %W].menu.print
        $WdgMenu.m post [winfo rootx $WdgMenu] [expr [winfo rooty $WdgMenu] + \
          [winfo height $WdgMenu]]
      }
    }
  }

  if {[info exists FocusWindow] == 0 } {
    set FocusWindow $window
  }

  bind $window <FocusIn> {
    if { $FocusWindow != "[winfo toplevel %W]" } {
      SwapWindowsVariable %W
    } 
  }

  bind $window <F1> {
    if { [string first "jedit" %W] != -1 } {
      if { [winfo class %W] == "Text" } {
        LoadPlc %W
      } else {
        LoadPlc [TopToText [winfo toplevel %W]]
      }
    }
  }

  bind $window <F2> {
    if { [string first "jedit" %W] != -1 } {
      DefinedVariableList %W
    }
  }

  bind $window <F3> {
    if { [string first "jedit" %W] != -1 } {
      DefinedFunctionBlockList %W
    }
  }

  bind $window <F4> {
    if { [string first "jedit" %W] != -1 } {
      DeleteLine %W
    }
  }

  bind Text <1> { 
    tkTextButton1 %W %x %y
    %W tag remove sel 0.0 end
    %W tag remove selmio 0.0 end
    %W tag remove linea_errata 0.0 end
    if { [string first "jedit" %W] != -1 } {
      DisplayCurrentLine %W
      AbsoluteColumn %W
    } 
  }

  bind Text <Delete> {
    if { [string first "jedit" %W] != -1 } {
      if {$DebugState == 0} {
  	DeleteRight %W %K %A
	SetModiFile %W 1
      }
    } elseif {[string first "pr" %W] != -1 } {
      if {[%W tag nextrange sel 1.0 end] != ""} {
        %W delete sel.first sel.last
      } else {
        %W delete insert
        %W see insert
      }
    }
  } 

  bind Text <BackSpace> {
    if { [string first "jedit" %W] != -1} {
      if {$DebugState == 0} {
        Backspace %W %K %A
      }
    } elseif {[string first "pr" %W] != -1 } {
      if {[%W tag nextrange sel 1.0 end] != ""} {
        %W delete sel.first sel.last
      } elseif [%W compare insert != 1.0] {
        %W delete insert-1c
        %W see insert
      }
    }
  }

  bind Text <Up> {
    if { [string first "jedit" %W] != -1 } {
      UpKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W [tkTextUpDownLine %W -1]
    }
  }

  bind Text <Down> {
    if { [string first "jedit" %W] != -1 } {
      DownKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W [tkTextUpDownLine %W 1]
    }
  }

  bind Text <Left> {
    if { [string first "jedit" %W] != -1 } {
      LeftKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W insert-1c
    }
  }

  bind Text <Home> {
    if { [string first "jedit" %W] != -1 } {
      HomeKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W {insert linestart}
    }
  }

  bind Text <Right> {
    if { [string first "jedit" %W] != -1 } {
      RightKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W insert+1c
    }
  }

  bind Text <End> {
    if { [string first "jedit" %W] != -1 } {
      EndKey %W %K %A
    } elseif {[string first "pr" %W] != -1 } {
      tkTextSetCursor %W {insert lineend}
    }
  }
    
  bind Text <Next> {
    if { [string first "jedit" %W] != -1 } {
      ScrollDownKey %W %K %A
    } else {
      tkTextSetCursor %W [tkTextScrollPages %W 1]
    }
  }

  bind Text <Prior> {
    if { [string first "jedit" %W] != -1 } {
      ScrollUpKey %W %K %A
    } else {
      tkTextSetCursor %W [tkTextScrollPages %W -1]
    }
  }
    
  bind Text <Escape> {
    if { [string first "jedit" %W] != -1 } {
      WaitWindow %W 0
      if {$DebugState == 1 || $AttivatoDebug == 1} {
        set DebugState 0
        set AttivatoDebug 0
        DebugProgram %W 
      }
    }
  }

  bind Text <space> {
    if { [string first "jedit" %W] != -1 && $DebugState == 0 } {
      TextInsertString %W " "
      DisplayCurrentLine %W
      AbsoluteColumn %W
      SetModiFile %W 1
    } elseif { [string first "pr" %W] != -1 } {
      TextInsertString %W " "
    }
  }

  bind Text <Return> {
    if { [string first "jedit" %W] != -1 && $DebugState == 0 } {
      ReturnKey %W 
      DisplayCurrentLine %W
      SetModiFile %W 1
    } elseif { [string first "pr" %W] != -1 } {
      TextInsertString %W "\n"
    }
  }
    
  bind Text <parenright> {
    if { [string first "jedit" %W] != -1 } {
      FlashParen %W %K %A
    }
  }

  bind Text <bracketright> {
    if { [string first "jedit" %W] != -1 } {
      FlashParen %W %K %A
    }
  }

  bind Text <braceright> {
    if { [string first "jedit" %W] != -1 } {
      FlashParen %W %K %A
    }
  }

  bind Text <KeyPress> {
    if { [string first "jedit" %W] != -1 } {
      if { [string compare %K "Num_Lock"] != 0 && \
	   [string compare %K "Caps_Lock"] != 0 && \
           [string compare %K  "Scroll_Lock"] != 0 && \
           [string compare %K "Shift_L"] != 0 && \
           [string compare %K "Shift_R"] != 0 && \
           [string compare %K "Control_L"] != 0 && \
           [string compare %K "Control_R"] != 0 && \
           [string compare %K "Alt_L"] != 0 && \
           [string compare %K "Alt_R"] != 0 && \
           [string compare %K "F1"] != 0 && \
           [string compare %K "F2"] != 0 && \
           [string compare %K "F3"] != 0 && \
           [string compare %K "F4"] != 0 && \
           [string compare %K "F5"] != 0 && \
           [string compare %K "F6"] != 0 && \
           [string compare %K "F7"] != 0 && \
           [string compare %K "F8"] != 0 && \
           [string compare %K "F9"] != 0 && \
           [string compare %K "F10"] != 0 && \
           [string compare %K "F11"] != 0 && \
           [string compare %K "F12"] != 0 && \
           [string compare %K "??"] != 0 && \
           $DebugState == 0 } {
        SetModiFile %W 1
        tkTextInsert %W %A 
      } 
    } elseif { [string first "pr" %W] != -1 } {
      tkTextInsert %W %A
    }
  }

  bind Text <Double-1> {
    if { $DebugState == 1 } {
      return
    }
    if { [string first "jedit" %W] != -1 } {
      set indice [%W index insert]
      set num_linea [lindex [split $indice .] 0]
      set indice_linea [lindex [split $indice .] 1]
      set linea [%W get $num_linea.0 $num_linea.end]
      set InizioParola [string wordstart $linea $indice_linea]
      set FineParola [string wordend $linea $indice_linea]
      set UltimoCarattere [string index $linea $FineParola]
      set variable [string range $linea $InizioParola [expr $FineParola -1]]
      if {[lsearch -exact [array names VariableList] $variable] != -1} {
        set VARS_DEF(tamponata) [lindex $VariableList($variable) 0]
        set VARS_DEF(tipo_variabile) [lindex $VariableList($variable) 1]
        set VARS_DEF(visibilita) [lindex $VariableList($variable) 2]
        set VARS_DEF(locazione) [lindex $VariableList($variable) 3]
        set VARS_DEF(initialization) [lindex $VariableList($variable) 4]
        set VARS_DEF(description) [lindex $VariableList($variable) 5]
        set VARS_DEF(x) [lindex $VariableList($variable) 6]
        set VARS_DEF(y) [lindex $VariableList($variable) 7]
        if { [string first "." [lindex $VariableList($variable) 8]] != -1 } {
          set VARS_DEF(ioAssociation) [lindex $VariableList($variable) 8]
        } else {
          set VARS_DEF(ioAssociation) {}
        }
        DefineVariablePanel %W $variable 0
      }
    }
  }
}

####################################################################
# Procedura DisplayCurrentLine
# Visualizza in alto a destra in quale linea e colonna e' posizionato
# il cursore
# Input: 
#   t Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc DisplayCurrentLine { t } {
  global ContactDef 

  set insertindex [split [$t index insert] {.}]
  set line [lindex $insertindex 0]
  set column [lindex $insertindex 1]
  incr column
  if {[info exists ContactDef(linee_equazioni)] && \
      [info exists ContactDef(ordine_seg)] } {
    RicavaSegmento $line
    DisplayEquationNumber [winfo toplevel $t].main.status.position $line $column
  } else {
    [winfo toplevel $t].main.status.position configure -text "[GetString Riga] \
        $line\t[GetString Colonna] $column\t\t"
  }
}

####################################################################
# Procedura DisplayEquationNumber
# Visualizza in alto a destra in quale equazione e' posizionato
# il cursore
# Input: 
#   wdg    Nome del widget label
#   line   Linea del cusore
#   column Colonna del cursore
# Ritorna :
#   Niente
####################################################################
proc DisplayEquationNumber { Wdg line column } {
  global ContactDef 

  $Wdg configure -text "[GetString Equazione] \
    $ContactDef(SegmentoCorrente)/[expr [llength $ContactDef(ordine_seg)] -1]\t\
                        [GetString Riga] $line\t\
                        [GetString Colonna] $column\t\t"
}

####################################################################
# Procedura DeleteRight
# Cancella il carattere a destra
# il cursore
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc DeleteRight { W K A } {
  global EditPrefs
  
  set delete_to [$W index insert]
  RightKey $W $K $A
  set delete_from [$W index insert]
  SaveCheckpoint $W
  TextDelete $W $delete_to $delete_from
}

####################################################################
# Procedura Backspace
# Cancella il carattere prima del punto di inserimento
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc Backspace { W K A } {
  global CutBuffer 
  
  if { ![TextHasSelection $W]} {
    DeleteLeft $W $K $A
    SetModiFile $W 1 
    return 
  }

  # Salva le informazioni per l'undo
  SaveCheckpoint $W

  #set CutBuffer [$W get sel.first sel.last]
  TextDelete $W sel.first sel.last
  SetModiFile $W 1
}

####################################################################
# Procedura TextHasSelection
# Determina se c'e una selezione attiva nel testo
# Input: 
#   t      Nome del widget di testo
# Ritorna :
#   vero se c'e' selezione, altrimenti falso
####################################################################
proc TextHasSelection { t } {

  set selrange [$t tag nextrange sel 1.0]
  
  if {"x$selrange" == "x"} {
    return 0
  } else {
    return 1
  }
}


####################################################################
# Procedura MoveCursor
# Sposta il cursore all'indice impostato
# Input: 
#   W      Nome del widget di testo
#   index  Indice a cui spostare il cursore
# Ritorna :
#   Niente
####################################################################
proc MoveCursor { W index } {
  global EditPrefs
  
  $W mark set insert $index
  #$W yview -pickplace insert
  $W see insert
  DisplayCurrentLine $W
}

####################################################################
# Procedura UpKey
# Sposta il cursore su di una linea
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc UpKey { W K A } {
  global DebugState 

  if { $DebugState == 0 } {
    ActivateInterpreter $W
  } else { 
    ProtocolloAttivaDebug $W IL
  }
  MoveCursor $W [RelativeColumn $W "insert - 1 line"]
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura DownKey
# Sposta il cursore giu di una linea
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc DownKey { W K A } {
  global DebugState 

  if { $DebugState == 0 } {
    ActivateInterpreter $W
  } else { 
    ProtocolloAttivaDebug $W IL
  }
  MoveCursor $W [RelativeColumn $W "insert + 1 line"]
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura LeftKey
# Sposta il cursore a sinistra di un carattere
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc LeftKey { W K A } {

  MoveCursor $W {insert - 1 char}
  AbsoluteColumn $W
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura HomeKey
# Sposta il cursore ad inizio linea
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc HomeKey { W K A } {

  MoveCursor $W {insert linestart}
  AbsoluteColumn $W
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura RightKey
# Sposta il cursore a destra di un carattere
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc RightKey { W K A } {

  MoveCursor $W {insert + 1 char}
  AbsoluteColumn $W
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura EndKey
# Sposta il cursore ad inizio linea
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc EndKey { W K A } {

  MoveCursor $W {insert lineend}
  AbsoluteColumn $W
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura ScrollDownKey
# Sposta il cursore in basso di una pagina
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc ScrollDownKey { W K A } {
  global DebugState 

  if { $DebugState == 0 } {
    ActivateInterpreter $W
  } else { 
    ProtocolloAttivaDebug $W IL
  }
  $W yview scroll 1 pages
  MoveCursor $W @0,0
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura ScrollUpKey
# Sposta il cursore in alto di una pagina
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc ScrollUpKey { W K A } {
  global DebugState 

  if { $DebugState == 0 } {
    ActivateInterpreter $W
  } else { 
    ProtocolloAttivaDebug $W IL
  }
  $W yview scroll -1 pages
  MoveCursor $W @0,0
  $W tag remove sel 1.0 end
  $W tag remove selmio 1.0 end
}

####################################################################
# Procedura TextInsertString
# Inserisce una stringa nel punto di inserimento
# Input: 
#   w        Nome della finestra di testo
#   text     Testo da inserire
# Ritorna :
#   Niente
####################################################################
proc TextInsertString { w text } {

  if { [TextHasSelection $w]} {
    TextDelete $w sel.first sel.last
  }
  $w insert insert $text
  $w see insert
}

####################################################################
# Procedura TextDelete
# Cancellazione di un pezzo di testo dall' indice from
# all' indice to
# Input: 
#   w        Nome della finestra di testo
#   from     Indice di partenza
#   to       Indice di arrivo
# Ritorna :
#   Niente
####################################################################
proc TextDelete { w from to } {

  MoveCursor $w $from
  $w delete $from $to
}

####################################################################
# Procedura TextReplace
# Sostituisce una parte di programma con la stringa in input
# principale
# Input: 
#   window    Nome della finestra di testo
#   from      indice da cui cominciare a sostituire
#   to        indice finale per la sostituzione
#   string    Stringa sostitutiva
#   datomacc  Flag che dice se e' un dato macchina o no
# Ritorna :
#   Niente
####################################################################
proc TextReplace { w from to string datomacc} {

  set start [$w index $from]
  set tags [$w tag names $from]
  
  $w mark set insert $from
  $w delete insert $to

  set IndiceCommento [string first "(*" $string ]
  if { $IndiceCommento == -1 && $datomacc == 0} {
    set string [string toupper $string]
  } else {
    if { $datomacc == 0 } {
      set string "[string toupper [string range $string 0 $IndiceCommento ]][string \
        range $string [expr $IndiceCommento +1] end]" 
    }
  }
  $w insert insert $string
  
  foreach tag [$w tag names $start] {
    $w tag remove $tag $start insert
  }
  foreach tag $tags {
    $w tag add $tag $start insert
  }
  
  $w see insert
}

####################################################################
# Procedura ReturnKey
# Azione chiamata sul tasto return
# Input: 
#   t    Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ReturnKey { t } {
  
  # prima di passare alla riga successiva lancio l'interprete 
  PlcInterpreter $t

  SaveCheckpoint $t
  
  set insertindex [split [$t index insert] {.}]
  set line [lindex $insertindex 0]
  set ContenutoLinea [$t get $line.0 $line.end]
  set indice  0
  while {[string range $ContenutoLinea $indice $indice] == " " || \
         [string range $ContenutoLinea $indice $indice] == "\t" } {
    incr indice
  }

  TextInsertString $t "\n[string range $ContenutoLinea 0 [expr $indice -1]]"
}

####################################################################
# Procedura DeleteLeft
# Cancella il carattere a sinistra
# il cursore
# Input: 
#   W      Nome del widget chiamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc DeleteLeft { W K A } {
  global EditPrefs
  
  set delete_to [$W index insert]
  LeftKey $W $K $A
  set delete_from [$W index insert]
  SaveCheckpoint $W
  TextDelete $W $delete_from $delete_to
}

####################################################################
# Procedura SaveCheckpoint
# Salva i vari livelli di undo
# Input: 
#   window    Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc SaveCheckpoint { t } {
  global UndoText UndoTag EditPrefs UndoPointer

  if $EditPrefs(undolevels) {
    set UndoText($UndoPointer) [$t get 1.0 end]
    set UndoTag($UndoPointer) [GetTagAnnotation $t]
    incr UndoPointer
    set old [expr {$UndoPointer - $EditPrefs(undolevels)}]
    catch {
      unset UndoText($old)
      unset UndoTag($old)
    }
  }
}

####################################################################
# Procedura GetTagAnnotation
# Ritorna la lista dei tag e dei mark correnti nel testo 
# Input: 
#   t    Nome della finestra di testo
# Ritorna :
#   Lista dei tag
####################################################################
proc GetTagAnnotation {t} {

  set tags {}
  set marks {}
  foreach tag [$t tag names] {
    set ranges [$t tag ranges $tag]
    if {"x$ranges" != "x"} {
      lappend tags [list $tag $ranges]
    }
  }
  foreach mark [$t mark names] {
    lappend marks [list $mark [$t index $mark]]
  }
  return [list $tags $marks]
  close $file
}

####################################################################
# Procedura SetTagAnnotation
# Setta i tag ed i mark nel buffer di undo nel testo
# Input: 
#   t     Nome della finestra di testo
#   state Elelnco dei tag da ripristinare
# Ritorna :
#   Niente
####################################################################
proc SetTagAnnotation { {t} {state} } {

  set tags [lindex $state 0]
  set marks [lindex $state 1]
  foreach pair $marks {
    $t mark set [lindex $pair 0] [lindex $pair 1]
  }
  foreach pair $tags {
    set tag [lindex $pair 0]
    set ranges [lindex $pair 1]
    set length [llength $ranges]
    for {set i 0; set i1 1} {$i1 < $length} {incr i 2; incr i1 2} {
      $t tag add $tag [lindex $ranges $i] [lindex $ranges $i1]
    }
  }
  $t yview -pickplace insert
}

####################################################################
# Procedura RelativeColumn
# Procedura che calcola l'indice relativo nella riga assegnato 
# l'indice assoluto di colonna per fare in modo che il cursore
# conservi la stessa colonna quando si va su e giu.
# Input: 
#   t     Nome della finestra di testo
#   indice indice corrente
# Ritorna :
#   Colonna relativa
####################################################################
proc RelativeColumn { t indice } {
  global ColonnaAssoluta EditPrefs

  set insertindex [split [$t index $indice] {.}]
  set riga [lindex $insertindex 0]
  set column [lindex $insertindex 1]
  set ContenutoLinea [$t get $riga.0 $riga.end]
  set ColonnaTemporanea 0
  set TmpIndice 0 
  # Calcolo il valore assoluto della lunghezza della linea risolvendo
  # i tab assumendoli di 8 caratteri
  for {set i 0 } { $i < [string length $ContenutoLinea] } {incr i} {
    if { [string range $ContenutoLinea $i $i] == "\t" } {
      set ColonnaTemporanea [expr (($ColonnaTemporanea / \
        $EditPrefs(tablength)) + 1) * $EditPrefs(tablength) ] 
    } else {
      incr ColonnaTemporanea
    }
    if {$ColonnaTemporanea >= $ColonnaAssoluta } {
      if { $ColonnaAssoluta == 0 } {
        return $riga.$i
      } else {
        return $riga.[expr $i + 1]
      }
    } 
  }
  return $riga.end
}

####################################################################
# Procedura FlashParen
# Procedura che cerca la corrispondente parentesi sinistra
# in $W e la fa flashare
# Input: 
#   W    Nome del widget chamante
#   K      Tasto Premuto
#   A      Carattere del tasto premuto
# Ritorna :
#   Niente
####################################################################
proc FlashParen { W K A } {
  global ParenLeftMatch EditPrefs 
  
  TextInsertString $W $A
  if $EditPrefs(parenflash) {
    FlashLeftParen $W $ParenLeftMatch($K) $A
  }
}

####################################################################
# Procedura FindLeftParen
# Procedura che cerca la corrispondente parentesi sinistra
# Input: 
#   t             Nome del widget di testo
#   left          Parentesi sinistra
#   right         Parentesi destra
#   close_trace   Indice in cui terminare la ricerca
# Ritorna :
#   "" se non e' stato trovata la parentesi viceversa
#   l'indice della parentesi trovata
####################################################################
proc FindLeftParen {t left right close_trace} {

  set left [string trimleft $left \\]
  set right [string trimleft $right \\]
  set open_trace $close_trace
  #while (1) 
  for { set i 0 } { $i < 10 } {incr i} {
    # go back 1 left, quit if none found.
    set backset [string last $left [$t get 1.0 $open_trace]]
    if {($backset < 0)} {return ""}
    set open_trace [$t index "1.0 +$backset chars"]
    # go back 1 right, if none after open, return current open
    set offset [string last $right [$t get \
      $open_trace $close_trace]]
    if {($offset < 0)} {
      return $open_trace
    }
    set close_trace [$t index "$open_trace +$offset chars"]
  }
   return ""
}

####################################################################
# Procedura FlashLeftParen
# Procedura che fa flashare la parentesi di sinistra
# Input: 
#   t     Nome del widget di testo
#   left  Carattere da matchare
#   right Parentesi premuta
# Ritorna :
#   Niente
####################################################################
proc FlashLeftParen {t left right} {

  set result [FindLeftParen $t $left $right [$t index "insert -1 chars"]]
  if {($result != "")} {
    Flash $t $result "$result +1 chars"
  }
}

####################################################################
# Procedura Flash
# Flash della parentesi
# Input: 
#   t     Nome del widget di testo
#   from  Partenza della flashata
#   to    Fine della flashata
# Ritorna :
#   Niente
####################################################################
proc Flash {t from to} {
  global EditPrefs
  
  set bg $EditPrefs(textbg)
  set fg $EditPrefs(textfg)
  
  $t tag configure paren_match -background $fg -foreground $bg
  $t tag raise paren_match
  
  $t tag add paren_match $from $to
  after 200 "$t tag delete paren_match"
}

