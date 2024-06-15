
            ######################################
            #                                    #
            #     File di definizione di         #
            #     widget generici                #
            #                                    #
            ######################################


######################################################################
# Procedura ButtonBar 
# Creazione di una barra di bottoni
# le opzioni sono :
#   -default (default none)
#   -padx (default 5)
#   -pady (default 5)
#   -orient (default horizontal)
#   -buttons (default {})
#   -width (default 8)
# la sintassi della lista di bottoni e' {{nome chiave comando} ... }
######################################################################
proc ButtonBar {args} {
global EditPrefs DebugNoWidget EditLadderWidget

  ParseArgs {
    {default "(NONE)"}
    {padx 5}
    {pady 5}
    {orient horizontal}
    buttons
    {width 8}
  }

  set newframe [lindex $args 0]
  
  if {$orient == "horizontal"} {
    set side left
  } else {
    set side bottom
  }

  frame $newframe
  if {$padx} {
    pack [FillerFrame $newframe -width $padx] -in $newframe -side left
    pack [FillerFrame $newframe -width $padx] -in $newframe -side right
  }
  if {$pady} {
    pack [FillerFrame $newframe -height $pady] -in $newframe -side top
    pack [FillerFrame $newframe -height $pady] -in $newframe -side bottom
  }
  
  foreach i $buttons {
    set name [lindex $i 0]
    set key [lindex $i 1]
    set command [lindex $i 2]
    
    set text [uplevel 1 [list GetString $key]]
    
    set button $newframe.$name
 
   if {[llength $i] >3 } {
        set image [lindex $i 3]
        button $button -text $text -command $command -image $image
        if {[llength $i] >4 } {
            CniBalloon $button [ GetString [lindex $i 4]] $EditPrefs(windowfont) 500
           }
       } else {
        button $button -width $width -text $text -command $command
       }

    if {[string first "jedit" [winfo toplevel $newframe]] != -1 } {
      # metto in DebugNoWidget i widget da nascondere in caso di debug
      if { [string first "debug" $button] == -1 } {
        set DebugNoWidget [lappend DebugNoWidget $button]
      }
    }

    if { [string first "delete" $button] == -1 } {
      set EditLadderWidget [lappend EditLadderWidget $button]
    }

    if [ info exists EditPrefs(windowfont) ] {
      $button configure -font $EditPrefs(windowfont)
    }
    set border $newframe.border_$name
    frame $border -relief flat -borderwidth 1
    raise $button
    pack $button -in $border -padx $padx -pady $pady
    pack $border -in $newframe -side $side -padx 2
    if [string match $default $name] {
      $border configure -relief sunken
    }
  }
  return $newframe
}

######################################################################
# Procedura VariableEntry 
# Procedura che crea un form con una etichetta ed un entry
# le opzioni sono
#   -label (default "Value:")
#   -variable (default value - not really optional)
#   -labelwidth (default 16)
#   -entrywidth (default 40)
#   -labelconfig (default "")
#   -entryconfig (default "")
######################################################################
proc VariableEntry {args} {

  ParseArgs {
    {label {Value:} }
    {variable value }
    {labelwidth {16} }
    {entrywidth {40} }
    {labelconfig {} }
    {entryconfig {} }
  }
global $variable EditPrefs
  
  set label [uplevel 1 [list GetString $label]]
  
  set newframe [lindex $args 0]
  
  frame $newframe
  set l $newframe.l
  set e $newframe.e
  label $l -text $label -width $labelwidth -anchor w
  entry $e -width $entrywidth -textvariable $variable

  if [ info exists EditPrefs(windowfont) ] {
    $l configure -font $EditPrefs(windowfont)
    $e configure -font $EditPrefs(windowfont)
  }
  
  if {$labelconfig != ""} {
    eval $l configure $labelconfig
  }
  if {$entryconfig != ""} {
    eval $e configure $entryconfig
  }
  pack $l -in $newframe -side left -fill both
  pack $e -in $newframe -side left -fill both
  
  return $newframe
}

######################################################################
# Procedura TipoDiFinestra
# Procedura che ritorna il tipo di finestra aperta
# Input: 
#   t          Nome del widget di testo
# Ritorna :
#   main        Se finestra principale
#   secondary   Se finestra secondaria
#   fb          Se function block
######################################################################
proc TipoDiFinestra { t } {
global WindowType 

  if { $WindowType([TextToTop $t]) == "main" } {
    if { [string range [TextToTop $t] end end] != 0 } {
      return secondary
    } else {
      return main
    }
  } elseif { $WindowType([TextToTop $t]) == "fb" } {
    return fb
  }
}

######################################################################
# Procedura IlPaletteCreate
# Procedura che crea una button bar con lista istruzioni AWL
# Input: 
#   w          Nome del widget padre
#   t          Nome del widget di testo
# Ritorna :
#   Niente
######################################################################
proc IlPaletteCreate { w t } {
global EditPrefs DebugNoWidget

  if ![winfo exists $w.b] {
    frame $w.b
  }

  set QualeFinestra [TipoDiFinestra $t]

  tixSelect $w.b.cmd 
  pack $w.b.cmd -side top -expand yes -anchor w \
        -padx 4 -pady 4

  if { $QualeFinestra == "main" || $QualeFinestra == "secondary" } {
    $w.b.cmd add load -image loadfile -command "CmdLoad $t"
    CniBalloon [$w.b.cmd subwidget load] [GetString CmdLoadMin] $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget load]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget load]]
    }
  }

  if { $QualeFinestra == "main" || $QualeFinestra == "secondary" } {
    $w.b.cmd add loadexe -image openexe -command "CmdLoadEsecuzione $t"
    CniBalloon [$w.b.cmd subwidget loadexe] [GetString CmdLoadEsecuzioneMin] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget loadexe]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget loadexe]]
    }
  }

  if { $QualeFinestra == "main" || $QualeFinestra == "fb" } {
    $w.b.cmd add save -image savefile -command "CmdSave $t"
    CniBalloon [$w.b.cmd subwidget save] [GetString CmdSaveMin] $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget save]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget save]]
    }
  }

  if { $QualeFinestra == "main" } {
    $w.b.cmd add backup -image backup -command "CmdBackup $t"
    CniBalloon [$w.b.cmd subwidget backup] [GetString CmdBackupMin] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget backup]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget backup]]
    }
  }

  if { $QualeFinestra == "main" } {
    $w.b.cmd add newwin -image windows -command "CmdNewWindow"
    CniBalloon [$w.b.cmd subwidget newwin] [GetString CmdNewWindowMin] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget newwin]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget newwin]]
    }
  }

  $w.b.cmd add find -image search -command "CmdFind $t"
  CniBalloon [$w.b.cmd subwidget find] [GetString CmdFindMin] $EditPrefs(windowfont) 500

  if { $QualeFinestra == "main" } {
    $w.b.cmd add connec -image connect -command "CmdIoConnect $t"
    CniBalloon [$w.b.cmd subwidget connec] [GetString CmdIoConnectMin] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget connec]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget connec]]
    }
  }

  if { $QualeFinestra == "main" } {
    $w.b.cmd add print -image printer -command "CmdPrintPlci $t"
    CniBalloon [$w.b.cmd subwidget print] [GetString CmdPrintPlciMin] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget print]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget print]]
    }
  }

  if { $EditPrefs(awl) == 0 } {
    $w.b.cmd add edit -image edtseg -command "CmdOldSegment $t"
    CniBalloon [$w.b.cmd subwidget edit] [GetString EditEquation] \
      $EditPrefs(windowfont) 500
    if { [lsearch -exact $DebugNoWidget [$w.b.cmd subwidget edit]] == -1 } {
      set DebugNoWidget [lappend DebugNoWidget [$w.b.cmd subwidget edit]]
    }
  }

  return $w.b
}

######################################################################
# Procedura LadderPaletteCreate
# Procedura che crea una button bar con bottoni per creare un ladder
# Input: 
#   w          Nome del widget padre
#   t          Nome del widget canvas
# Ritorna :
#   Niente
######################################################################
proc LadderPaletteCreate { w canvas } {
global BottoneAttivo Home EditPrefs

  if ![winfo exists $w.b] {
    frame $w.b
  }

  tixSelect $w.b.just -allowzero false -radio true

  pack $w.b.just -side left  -anchor w -padx 4 -pady 4

  button $w.b.save -bitmap @$Home/icone/SAVESEG.png \
    -command "CmdInsertSegment [TopToText [winfo toplevel $w]]"
  pack $w.b.save -side left -anchor w -pady 4
  CniBalloon $w.b.save [GetString SalvaEquaz] $EditPrefs(windowfont) 500
  

  $w.b.just add cont1b      -bitmap @$Home/icone/CONT1B.png
  $w.b.just add cont2b      -bitmap @$Home/icone/CONT2B.png
  $w.b.just add cont1rb     -bitmap @$Home/icone/CONT1RB.png
  $w.b.just add cont2rb     -bitmap @$Home/icone/CONT2RB.png
  $w.b.just add cont3b      -bitmap @$Home/icone/CONT3B.png
  $w.b.just add cont4b      -bitmap @$Home/icone/CONT4B.png
  $w.b.just add cont5b      -bitmap @$Home/icone/CONT5B.png
  $w.b.just add cont6b      -bitmap @$Home/icone/CONT6B.png
  $w.b.just add cont3rb     -bitmap @$Home/icone/CONT3RB.png
  $w.b.just add cont4rb     -bitmap @$Home/icone/CONT4RB.png
  $w.b.just add jmpcb       -bitmap @$Home/icone/JMPCB.png
  $w.b.just add jmpcnb      -bitmap @$Home/icone/JMPCNB.png
  $w.b.just add colleg      -bitmap @$Home/icone/COLLEG.png
  $w.b.just add addcoll     -image addcoll
  $w.b.just add addcolr     -image addcolr
  $w.b.just add addrow      -image addrow
  $w.b.just add delconn     -image delconn

  $w.b.just config -variable BottoneAttivo

  set BottoneAttivo cont1b

  CniBalloon [$w.b.just subwidget cont1b] [GetString CNA] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont2b] [GetString CNC] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont1rb] [GetString OI] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont2rb] [GetString OIC] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont3b] [GetString COIL] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont4b] [GetString COILN] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont5b] [GetString SET] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont6b] [GetString RESET] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont3rb] [GetString UA] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget cont4rb] [GetString UAC] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget jmpcb] [GetString JMP] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget jmpcnb] [GetString JMPCN] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget colleg] [GetString CONN] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget addcoll] [GetString ACS] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget addcolr] [GetString ACD] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget addrow] [GetString AR] $EditPrefs(windowfont) 500
  CniBalloon [$w.b.just subwidget delconn] [GetString DELCON] $EditPrefs(windowfont) 500
}

######################################################################
# Procedura VerticalButtonBarCreate
# Procedura che crea una button bar verticale
# Input: 
#   w          Nome del widget padre
#   t          Nome del widget canvas
# Ritorna :
#   Niente
######################################################################
proc VerticalButtonBarCreate { w t } {
global EditPrefs DebugState

  if [winfo exists $w] {
    destroy $w
  }

  set DebugState 0
  frame $w

  set QualeFinestra [TipoDiFinestra $t]

  if { $QualeFinestra == "main" } {
    ButtonBar $w.acce1 -pady 0 -padx 0 -orient vertical -buttons [format {
      {run BalButton6 { LoadPlc %s} %s BalButton6 }
      {make BalButton5 {StartCompileProgram %s}  %s BalButton5}
      {var BalButton4 {DefinedVariableList %s} %s BalButton4  }
      {block BalButton3 {DefinedFunctionBlockList %s}  %s BalButton3 }
      {debug BalButton2 {StartStopDebug %s}  %s BalButton2 }
      {delete BalButton1 {DeleteLine %s} %s BalButton1}
    } $t convert3 $t magic $t notepd $t filesys $t debug $t wipeout ] 
  } elseif { $QualeFinestra == "secondary" } {
    ButtonBar $w.acce1 -pady 0 -padx 0 -orient vertical -buttons [format {
      {var BalButton4 {DefinedVariableList %s} %s BalButton4  }
      {block BalButton3 {DefinedFunctionBlockList %s}  %s BalButton3 }
      {delete BalButton1 {DeleteLine %s} %s BalButton1}
    } $t notepd $t filesys $t wipeout ] 
  } elseif { $QualeFinestra == "fb" } {
    ButtonBar $w.acce1 -pady 0 -padx 0 -orient vertical -buttons [format {
      {var BalButton4 {DefinedVariableList %s} %s BalButton4  }
      {block BalButton3 {DefinedFunctionBlockList %s}  %s BalButton3 }
      {debug BalButton2 {StartStopDebug %s}  %s BalButton2 }
      {delete BalButton1 {DeleteLine %s} %s BalButton1}
    } $t notepd $t filesys $t debug $t wipeout ] 
  }

  pack $w.acce1 -side top -anchor n 

  return $w
}

######################################################################
# Procedura ScriviEquazioneMatematica
# Procedura che scrive la equazione matematica
# Input: 
#   t          Nome del widget canvas
# Ritorna :
#   Niente
######################################################################
proc ScriviEquazioneMatematica { t } {
global sel_math1 sel_math2 sel_math3 sel_math4 sel_math5 sel_ld sel_st
global entry_ld entry_math entry_st 
global VariableList

  if { $sel_math1 != "" } {
    set sel_math [string toupper $sel_math1]
  } elseif { $sel_math2 != "" } {
    set sel_math [string toupper $sel_math2]
  } elseif { $sel_math3 != "" } {
    set sel_math [string toupper $sel_math3]
  } elseif { $sel_math4 != "" } {
    set sel_math [string toupper $sel_math4]
  } elseif { $sel_math5 != "" } {
    set sel_math [string toupper $sel_math5]
  }

  set entry_ld [string toupper $entry_ld]
  set entry_math [string toupper $entry_math]
  set entry_st [string toupper $entry_st]

  if {[VerificaNoOpConst [string toupper $sel_ld] $entry_ld MsgNoFirstOperand] == 0 } {
    set entry_ld [VerificaOperando $t "LD" $entry_ld 0]
  }
 
  if {[VerificaNoOpConst $sel_math $entry_math MsgNoSecondOperand] == 0 } {
    set entry_math [VerificaOperando $t "LD" $entry_math 0]
  }

  if {[VerificaNoOpConst [string toupper $sel_st] $entry_st MsgNoStOperand] == 0 } {
    if {$sel_st != "jmpcb" && $sel_st != "jmpcnb" } {
      set entry_st [VerificaOperando $t "LD" $entry_st 0]
    }
  }

  if { $sel_ld == "cont1b" || $sel_ld == "cont1rb" } {
    $t insert insert "\tLD\t[string toupper $entry_ld]\n"
  } elseif { $sel_ld == "cont2b" || $sel_ld == "cont2rb" } {
    $t insert insert "\tLDN\t[string toupper $entry_ld]\n"
  }
  
  set ListaReali [list ADD SUB MUL DIV GT GE EQ NE LE LT ABS]
  if { $entry_math != "" } {
    if [info exist VariableList($entry_math)] {
      if { [lindex $VariableList($entry_math) 1] == "REAL" && \
	  [lsearch -exact $ListaReali $sel_math] != -1 } {
        append sel_math _REAL
      }
    } else {
      if { [string first "." $entry_math] != -1 } {
        append sel_math _REAL
      } 
    }
  }
  $t insert insert "\t$sel_math\t$entry_math\n"

  if { $sel_st == "cont3b" || $sel_st == "cont3rb" } {
    $t insert insert "\tST\t$entry_st\n"
  } elseif { $sel_st == "cont4b" || $sel_st == "cont4rb" } {
    $t insert insert "\tSTN\t$entry_st\n"
  } elseif { $sel_st == "cont5b" } {
    $t insert insert "\tS\t$entry_st\n"
  } elseif { $sel_st == "cont6b" } {
    $t insert insert "\tR\t$entry_st\n"
  } elseif { $sel_st == "jmpcb" } {
    $t insert insert "\tJMPC\t$entry_st\n"
  } elseif { $sel_st == "jmpcnb" } {
    $t insert insert "\tJMPCN\t$entry_st\n"
  }
}

######################################################################
# Procedura UnisciBottoni
# Procedura chiamata da tutti i bottoni di funzioni matematiche
# Input: 
#   w          Nome del widget chiamante
#   flag       ????
# Ritorna :
#   Niente
######################################################################
proc UnisciBottoni { w flag } {
global sel_math1 sel_math2 sel_math3 sel_math4 sel_math5

  if { $w == $sel_math1 } {
    set sel_math2 ""
    set sel_math3 ""
    set sel_math4 ""
    set sel_math5 ""
  } elseif { $w == $sel_math2 } { 
    set sel_math1 ""
    set sel_math3 ""
    set sel_math4 ""
    set sel_math5 ""
  } elseif { $w == $sel_math3 } { 
    set sel_math1 ""
    set sel_math2 ""
    set sel_math4 ""
    set sel_math5 ""
  } elseif { $w == $sel_math4 } { 
    set sel_math1 ""
    set sel_math2 ""
    set sel_math3 ""
    set sel_math5 ""
  } elseif { $w == $sel_math5 } { 
    set sel_math1 ""
    set sel_math2 ""
    set sel_math3 ""
    set sel_math4 ""
  }
}

######################################################################
# Procedura RicavaParametriMath
# Procedura che ricava i parametri della equazione
# Input: 
#   MathWdg    Nome del widget matematico
#   Equazione  testo dell' equazione
# Ritorna :
#   Niente
######################################################################
proc RicavaParametriMath { MathWdg Equazione } {
global sel_ld sel_math1 sel_math2 sel_math3 sel_math4 sel_math5 sel_st 
global entry_ld entry_math entry_st 
global VariableList

  foreach Linea [split $Equazione "\n"] {
    if { [string match {(\**} $Linea] == 0 && $Linea != ""} {
      if { [string match *: [lindex $Linea 0]] == 0 } {
        set istruzione [lindex $Linea 0]
        set operando [lindex $Linea 1]
      } else {
        set istruzione [lindex $Linea 1]
        set operando [lindex $Linea 2]
      }
      if { $istruzione == "LD" } {
        if { [lindex $VariableList($operando) 1] == "BOOL" } {
          set sel_ld cont1b
        } else {
          set sel_ld cont1rb
        }
        set entry_ld $operando
      } elseif { $istruzione == "LDN"} {
        if { [lindex $VariableList($operando) 1] == "BOOL" } {
          set sel_ld cont2b
        } else {
          set sel_ld cont2rb
        }
        set entry_ld $operando
      } elseif { $istruzione == "ST" } {
        if { [lindex $VariableList($operando) 1] == "BOOL" } {
          set sel_st cont3b
        } else {
          set sel_st cont3rb
        }
        set entry_st $operando
      } elseif { $istruzione == "STN" } {
        if { [lindex $VariableList($operando) 1] == "BOOL" } {
          set sel_st cont4b
        } else {
          set sel_st cont4rb
        }
        set entry_st $operando
      } elseif { $istruzione == "S" } {
        set sel_st cont5b
        set entry_st $operando
      } elseif { $istruzione == "R" } {
        set sel_st cont6b
        set entry_st $operando
      } elseif { $istruzione == "JMP" || $istruzione == "JMPC" } {
        set sel_st jmpcb
        set entry_st $operando
      } elseif { $istruzione == "JMPCN" } {
        set sel_st jmpcnb
        set entry_st $operando
      } else {
        set istruzione [string tolower $istruzione]
        set FigliMath1 [winfo children $MathWdg.math1.frame]
        set FigliMath2 [winfo children $MathWdg.math2.frame]
        set FigliMath3 [winfo children $MathWdg.math3.frame]
        set FigliMath4 [winfo children $MathWdg.math4.frame]
        set FigliMath5 [winfo children $MathWdg.math5.frame]
        if { [lsearch -exact $FigliMath1 $MathWdg.math1.frame.$istruzione] != -1 } {
          set sel_math1 $istruzione 
        }
        if { [lsearch -exact $FigliMath2 $MathWdg.math2.frame.$istruzione] != -1 } {
          set sel_math2 $istruzione 
        }
        if { [lsearch -exact $FigliMath3 $MathWdg.math3.frame.$istruzione] != -1 } {
          set sel_math3 $istruzione 
        }
        if { [lsearch -exact $FigliMath4 $MathWdg.math4.frame.$istruzione] != -1 } {
          set sel_math4 $istruzione 
        }
        if { [lsearch -exact $FigliMath5 $MathWdg.math5.frame.$istruzione] != -1 } {
          set sel_math5 $istruzione 
        }
        set entry_math $operando
      }
    }
  }
}

######################################################################
# Procedura PannelloEquazioneMatematica
# Procedura che crea il pannello per la definizione di una
# equazione matematica. 
# Input: 
#   t          Nome del widget di testo
#   Equazione  testo dell' equazione
# Ritorna :
#   Niente
######################################################################
proc PannelloEquazioneMatematica { t Equazione } {
global sel_ld sel_math1 sel_math2 sel_math3 sel_math4 sel_math5 sel_st 
global entry_ld entry_math entry_st 
global Home EditPrefs MsdData

  set old_focus [focus]

  set MainW [winfo toplevel [focus]]
  if [winfo exists .math] {
    return
  }

  grab release [grab current]

  toplevel .math
  wm protocol .math WM_DELETE_WINDOW procx
  set w .math
  frame $w.top

  # There can be one and only type of justification for any piece of text.
  # So we set -radio to be true. Also, -allowzero is set to false: the user
  # cannot select a "none" justification
  #
  tixSelect $w.top.ld -allowzero false -radio true 
  tixSelect $w.top.math1 -command UnisciBottoni -allowzero true -radio true 
  tixSelect $w.top.math2 -command UnisciBottoni -allowzero true -radio true 
  tixSelect $w.top.math3 -command UnisciBottoni -allowzero true -radio true 
  tixSelect $w.top.math4 -command UnisciBottoni -allowzero true -radio true 
  tixSelect $w.top.math5 -command UnisciBottoni -allowzero true -radio true 
  tixSelect $w.top.st -allowzero false -radio true 

  entry $w.top.ldentry -textvariable entry_ld -font $EditPrefs(windowfont)
  entry $w.top.mathentry -textvariable entry_math -font $EditPrefs(windowfont)
  entry $w.top.stentry -textvariable entry_st -font $EditPrefs(windowfont)
 
  grid $w.top.ld -row 0 -column 0 -sticky w -padx 8 -pady 8
  grid $w.top.math1 -row 1 -column 0 -sticky w -padx 8 
  grid $w.top.math2 -row 2 -column 0 -sticky w -padx 8
  grid $w.top.math3 -row 3 -column 0 -sticky w -padx 8
  grid $w.top.math4 -row 4 -column 0 -sticky w -padx 8
  grid $w.top.math5 -row 5 -column 0 -sticky w -padx 8
  grid $w.top.st -row 6 -column 0 -sticky w  -padx 8 -pady 8 
  grid $w.top.ldentry -row 0 -column 1 -sticky w  -padx 8 -pady 8 
  grid $w.top.mathentry -row 1 -column 1 -sticky w  -padx 8 
  grid $w.top.stentry -row 6 -column 1 -sticky w  -padx 8 -pady 8 

  # Add the choices of available ld types
  $w.top.ld add cont1b      -bitmap @$Home/icone/CONT1B.png
  $w.top.ld add cont2b      -bitmap @$Home/icone/CONT2B.png
  $w.top.ld add cont1rb     -bitmap @$Home/icone/CONT1RB.png
  $w.top.ld add cont2rb     -bitmap @$Home/icone/CONT2RB.png
  # Add the choices of available st types
  $w.top.math1 add add      -bitmap @$Home/icone/ADD.png
  $w.top.math1 add sub      -bitmap @$Home/icone/SUB.png
  $w.top.math1 add mul      -bitmap @$Home/icone/MUL.png
  $w.top.math1 add div      -bitmap @$Home/icone/DIV.png
  $w.top.math1 add gt     -bitmap @$Home/icone/GT.png
  $w.top.math1 add ge     -bitmap @$Home/icone/GE.png
  $w.top.math1 add eq       -bitmap @$Home/icone/EQ.png
  $w.top.math1 add ne      -bitmap @$Home/icone/NE.png
  $w.top.math2 add lt      -bitmap @$Home/icone/LT.png
  $w.top.math2 add le      -bitmap @$Home/icone/LE.png
  $w.top.math2 add shl      -bitmap @$Home/icone/SHL.png
  $w.top.math2 add shr      -bitmap @$Home/icone/SHR.png
  $w.top.math2 add rol      -bitmap @$Home/icone/ROL.png
  $w.top.math2 add ror      -bitmap @$Home/icone/ROR.png
  $w.top.math2 add bitset      -bitmap @$Home/icone/BITSET.png
  $w.top.math2 add bitres      -bitmap @$Home/icone/BITRES.png
  $w.top.math3 add bittest      -bitmap @$Home/icone/BITTEST.png
  $w.top.math3 add bitfield      -bitmap @$Home/icone/BITFIELD.png
  $w.top.math3 add dint_to_real      -bitmap @$Home/icone/DINT_REA.png
  $w.top.math3 add real_to_dint      -bitmap @$Home/icone/REA_DINT.png
  $w.top.math3 add abs      -bitmap @$Home/icone/ABS.png
  $w.top.math3 add sin      -bitmap @$Home/icone/SIN.png
  $w.top.math3 add cos      -bitmap @$Home/icone/COS.png
  $w.top.math3 add tan      -bitmap @$Home/icone/TAN.png
  $w.top.math4 add asin      -bitmap @$Home/icone/ASIN.png
  $w.top.math4 add acos      -bitmap @$Home/icone/ACOS.png
  $w.top.math4 add atan      -bitmap @$Home/icone/ATAN.png
  $w.top.math4 add sqrt      -bitmap @$Home/icone/SQRT.png
  $w.top.math4 add exp      -bitmap @$Home/icone/EXP.png
  $w.top.math4 add log      -bitmap @$Home/icone/LOG.png
  $w.top.math4 add ln      -bitmap @$Home/icone/LN.png
  $w.top.math4 add trunc      -bitmap @$Home/icone/TRUNC.png
  $w.top.math5 add expt      -bitmap @$Home/icone/EXPT.png
  # Add the choices of available math types
  $w.top.st add cont3b      -bitmap @$Home/icone/CONT3B.png
  $w.top.st add cont4b      -bitmap @$Home/icone/CONT4B.png
  $w.top.st add cont5b      -bitmap @$Home/icone/CONT5B.png
  $w.top.st add cont6b      -bitmap @$Home/icone/CONT6B.png
  $w.top.st add cont3rb     -bitmap @$Home/icone/CONT3RB.png
  $w.top.st add cont4rb     -bitmap @$Home/icone/CONT4RB.png
  $w.top.st add jmpcb       -bitmap @$Home/icone/JMPCB.png
  $w.top.st add jmpcnb      -bitmap @$Home/icone/JMPCNB.png

  $w.top.ld config -variable sel_ld
  $w.top.math1 config -variable sel_math1
  $w.top.math2 config -variable sel_math2
  $w.top.math3 config -variable sel_math3
  $w.top.math4 config -variable sel_math4
  $w.top.math5 config -variable sel_math5
  $w.top.st config -variable sel_st

  # Use a ButtonBox to hold the buttons.
  #
  tixButtonBox $w.box -orientation horizontal
  $w.box add ok  -image ok  -command "ScriviEquazioneMatematica $t; destroy $w"
	
  $w.box add cancel -image cancel -command "destroy $w"

  CniBalloon [$w.box subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [$w.box subwidget cancel] [GetString CANCEL] $EditPrefs(windowfont) 500

  pack $w.box -side bottom -fill x
  pack $w.top -side top -fill both -expand yes

  # Create the balloon widget and associate it with the widgets that we want
  # to provide tips for:
  CniBalloon [$w.top.ld subwidget cont1b] [GetString CNA] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.ld subwidget cont2b] [GetString CNC] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.ld subwidget cont1rb] [GetString OI] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.ld subwidget cont2rb] [GetString OIC] $EditPrefs(windowfont) 500

  CniBalloon [$w.top.math1 subwidget add] "Add" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget sub] "Sub" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget mul] "Mul" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget div] "Div" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget gt] "Gt" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget ge] "Ge" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget eq] "Eq" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math1 subwidget ne] "Ne" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget lt] "Lt" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget le] "Le" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget shl] "Shl" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget shr] "Shr" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget rol] "Rol" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget ror] "Ror" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget bitset] "Bitset" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math2 subwidget bitres] "Bitres" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget bittest] "Bittest" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget bitfield] "Bitfield" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget dint_to_real] "Dint_to_real" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget real_to_dint] "Real_to_dint" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget abs] "Abs" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget sin] "Sin" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget cos] "Cos" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math3 subwidget tan] "Tan" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget asin] "Asin" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget acos] "Acos" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget atan] "Atan" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget sqrt] "Sqrt" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget exp] "Exp" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget log] "Log" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget ln] "Ln" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math4 subwidget trunc] "Trunc" $EditPrefs(windowfont) 500
  CniBalloon [$w.top.math5 subwidget expt] "Expt" $EditPrefs(windowfont) 500

  CniBalloon [$w.top.st subwidget cont3b] [GetString COIL] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget cont4b] [GetString COILN] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget cont5b] [GetString SET] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget cont6b] [GetString RESET] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget cont3rb] [GetString UA] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget cont4rb] [GetString UAC] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget jmpcb] [GetString JMP] $EditPrefs(windowfont) 500
  CniBalloon [$w.top.st subwidget jmpcnb] [GetString JMPCN] $EditPrefs(windowfont) 500

  wm transient $w $MainW
  PosizionaFinestre $MainW $w c

  # Set the default value of the two Select widgets
  set sel_ld cont1b
  set sel_math1 ""
  set sel_math2 ""
  set sel_math3 ""
  set sel_math4 ""
  set sel_math5 ""
  set sel_st cont3b
  set entry_ld "" 
  set entry_math "" 
  set entry_st ""
  if { $Equazione == "" } {
    set sel_math1 add
  } else {
    RicavaParametriMath $w.top $Equazione
  }

  bind $w.top.ldentry <Any-KeyRelease> {
    # trasformo tutti i caratteri in MAIUSCOLO
    set appo [string toupper [%W get]]
    %W delete 0 end
    %W insert end $appo
    if {[string length $appo] > $MsdData(lunvar)} {
    %W delete $MsdData(lunvar) end
    }
  }
  bind $w.top.mathentry <Any-KeyRelease> {
    # trasformo tutti i caratteri in MAIUSCOLO
    set appo [string toupper [%W get]]
    %W delete 0 end
    %W insert end $appo
    if {[string length $appo] > $MsdData(lunvar)} {
    %W delete $MsdData(lunvar) end
    }
  }
  bind $w.top.stentry <Any-KeyRelease> {
    # trasformo tutti i caratteri in MAIUSCOLO
    set appo [string toupper [%W get]]
    %W delete 0 end
    %W insert end $appo
    if {[string length $appo] > $MsdData(lunvar)} {
    %W delete $MsdData(lunvar) end
    }
  }

  grab $w
  tkwait window $w

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
}

######################################################################
# Procedura DeleteLine
# Procedura che cancella una riga
# Input: 
#   W          Nome del widget di testo
#   Equazione  testo dell' equazione
# Ritorna :
#   Niente
######################################################################
proc DeleteLine { W } {
global EditPrefs ContactDef AlberoLadder DebugState

  if { $DebugState == 1} {
    return
  }

  if { $EditPrefs(awl)!=1} {
   # caso LADDER
   if { [string first "ladd" $W] == -1 } {
     set W [winfo parent $W].ladd.edit
   }
   $W delete cont$ContactDef(edit,curr_riga):$ContactDef(edit,curr_colonna)
   $W delete var$ContactDef(edit,curr_riga):$ContactDef(edit,curr_colonna)
   # Cancello dall'albero le connessioni
   foreach connessioni $AlberoLadder(edit) {
     if { [string first $ContactDef(edit,curr_riga):$ContactDef(edit,curr_colonna) $connessioni] != -1 } {
       $W delete conn$connessioni
       set indice [lsearch -exact $AlberoLadder(edit) $connessioni]
       set AlberoLadder(edit) [concat [lrange $AlberoLadder(edit) 0 [expr $indice -1]] [lrange $AlberoLadder(edit) [expr $indice + 1] end]]
     } 
   }
   # Verifico se ho cancellato tutti i contatti di terminazione
   if { [lsearch -regexp $AlberoLadder(edit) :fine] == -1 } {
     set ContactDef(edit,col_fine_equazione) 0
   } 
  } else {
    # caso IL
    SaveCheckpoint $W
    SetModiFile $W 1
    $W delete "insert linestart" "insert lineend+1 char"
  }
}

######################################################################
# Procedura CambiaButtonBar
# Procedura che cancella la button bar e ne crea una nuova
# Input: 
#   t          Nome del widget di testo
# Ritorna :
#   Niente
######################################################################
proc CambiaButtonBar { t } {
global EditLadderWidget LadderEditShow EditPrefs

  # cambio buttonbar
  set buttonbar [TopToButtonbar [TextToTop $t]]
  set menubar [TopToMenubar [TextToTop $t]]

  if [winfo exists $buttonbar] {
    destroy $buttonbar
  }
  frame $buttonbar
  ButtonBarCreate $buttonbar $t
  pack $buttonbar.b [SeparationLine $buttonbar] -in $buttonbar -side top -fill x
  pack $buttonbar -after $menubar -fill x

  if { $LadderEditShow } {
    for {set i 0} {$i < [llength $EditLadderWidget]} {incr i} {
      set indice_ [string first ":" [lindex $EditLadderWidget $i]]
      if { $indice_ == -1 } {
        if [winfo exist [lindex $EditLadderWidget $i]] {
          [lindex $EditLadderWidget $i] configure -state disabled
        }
      } else {
        if [winfo exist [string range [lindex $EditLadderWidget $i] 0 \
              [expr $indice_ - 1]]] {
          [string range [lindex $EditLadderWidget $i] 0 [expr $indice_ - 1]] \
              entryconfigure [string range [lindex $EditLadderWidget $i] \
              [expr $indice_ + 1] end] -state disabled
        }
      }
    }
  } else {
    for {set i 0} {$i < [llength $EditLadderWidget]} {incr i} {
      set indice_ [string first ":" [lindex $EditLadderWidget $i]]
      if { $indice_ == -1 } {
        if [winfo exist [lindex $EditLadderWidget $i]] {
          [lindex $EditLadderWidget $i] configure -state normal
        }
      } else {
        if [winfo exist [string range [lindex $EditLadderWidget $i] 0 \
              [expr $indice_ - 1]]] {
          [string range [lindex $EditLadderWidget $i] 0 [expr $indice_ - 1]] \
              entryconfigure [string range [lindex $EditLadderWidget $i] \
              [expr $indice_ + 1] end] -state normal
        }
      }
    }
  }
}
