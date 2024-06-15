
            ######################################
            #                                    #
            #     File di gestione del box       #
            #     di ricerca parola              #
            #                                    #
            ######################################

####################################################################
# Procedura FindBox
# Creazione Box di ricerca ed eventuale sostituzione parola
# Input: 
#   t    Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc FindBox {t} {
global FindVector EditPrefs DebugState

  set FindVector(widget) $t
  append FindVector(searchinstr) {}
  append FindVector(searchfor) {}
  append FindVector(replaceinstr) {}
  append FindVector(replacewith) {}

  if {! [info exists FindVector(backwards)]} {
    set FindVector(backwards) 0
  }
  if {! [info exists FindVector(case)]} {
    set FindVector(case) 0
  }

  set replace !$DebugState

  if [winfo exists .find] {
    wm withdraw .find
    wm deiconify .find
    focus .find.t.search.e
    .find.t.search.e select range 0 end
    return 0
  }

  set old_focus [focus]

  set MainW [winfo toplevel [focus]]
                 
  #grab release [grab current]

  toplevel .find
  wm title .find [GetString TitleFindPanel]
  wm protocol .find WM_DELETE_WINDOW procx
  frame .find.t
  VariableEntry .find.t.searchinstr \
    -label [GetString FindSearchInstr] \
    -labelwidth 23 \
    -variable FindVector(searchinstr)
  VariableEntry .find.t.search \
    -label [GetString FindSearchFor] \
    -labelwidth 23 \
    -variable FindVector(searchfor)
  if $replace {
    VariableEntry .find.t.replaceinstr \
      -label [GetString FindReplaceWithInstr] \
      -labelwidth 23 \
      -variable FindVector(replaceinstr)
    VariableEntry .find.t.replace \
      -label [GetString FindReplaceWith] \
      -labelwidth 23 \
      -variable FindVector(replacewith)
  }
  frame .find.t.options
  label .find.t.options.filler -text {} -width 16 -anchor e
  checkbutton .find.t.options.backwards -anchor w \
    -text [GetString FindBackwards] \
    -selectcolor $EditPrefs(checkcolor) \
    -variable FindVector(backwards)
  checkbutton .find.t.options.case -anchor w \
    -text [GetString FindCase] \
    -selectcolor $EditPrefs(checkcolor) \
    -variable FindVector(case)
  
  if [ info exists EditPrefs(windowfont) ] {
    .find.t.options.filler configure -font $EditPrefs(windowfont)
    .find.t.options.backwards configure -font $EditPrefs(windowfont)
    .find.t.options.case configure -font $EditPrefs(windowfont)
  }

  tixButtonBox .find.b -orientation horizontal
  .find.b add search -image search -font $EditPrefs(windowfont) \
    -command {
      if {[FindPattern $FindVector(searchinstr) \
          $FindVector(searchfor) $FindVector(widget)] == 0} {
        AlertBox -text MsgNotFound
      }
    }

  if $replace {
    .find.b add replace  -image replace -font $EditPrefs(windowfont) \
      -command {
        FindReplace $FindVector(searchfor) $FindVector(replacewith) \
           $FindVector(replaceinstr) $FindVector(widget) NotAll
        FindPattern $FindVector(searchinstr) $FindVector(searchfor) \
           $FindVector(widget)
      }
    .find.b add replaceall -image replaceall -font $EditPrefs(windowfont) \
      -command {
        FindReplaceAll $FindVector(searchinstr) $FindVector(searchfor) \
            $FindVector(replacewith) $FindVector(replaceinstr) \
            $FindVector(widget)
      }
  }
  .find.b add cancel -image cancel -font $EditPrefs(windowfont) \
    -command {
      if { $EditPrefs(awl) == 0 } {
        # Deseleziono le selezioni precedenti
        if [info exist LadderSelection] {
          set canvas [lindex $LadderSelection 0]
          set contatto [lindex $LadderSelection 1]
          if [winfo exists $canvas] {
            $canvas itemconfigure cont$contatto -foreground black
            $canvas itemconfigure var$contatto -fill black
          }
        }
      }
      destroy .find
    }

  CniBalloon [.find.b subwidget search] [GetString Search] $EditPrefs(windowfont) 500
  CniBalloon [.find.b subwidget cancel] [GetString EXIT] $EditPrefs(windowfont) 500
  if $replace {
    CniBalloon [.find.b subwidget replace] [GetString ReplaceButton] \
      $EditPrefs(windowfont) 500
    CniBalloon [.find.b subwidget replaceall] [GetString ReplaceAll] \
      $EditPrefs(windowfont) 500
  }
  
  pack .find.t.options.filler -side left -fill both
  pack .find.t.options.backwards [FillerFrame .find.t.options] \
      .find.t.options.case -side left -fill y
  
  pack [FillerFrame .find.t] -side top
  pack .find.t.searchinstr -side top -expand yes -fill x
  pack .find.t.search -side top -expand yes -fill x
  if $replace {
    pack [FillerFrame .find.t] -side top
    pack .find.t.replaceinstr -side top -expand yes -fill x
    pack .find.t.replace -side top -expand yes -fill x
  }
  pack .find.t.options -side top -expand yes -fill both
  pack .find.t -side top -fill both -padx 10 -pady 5
  pack .find.b -side bottom -fill x

  #wm transient .find $MainW
  PosizionaFinestre $MainW .find ne

  bind .find <Key-Escape> {destroy .find}
  if $replace {
    DefaultButton .find.b.search .find.t.search.e .find.t.replace.e \
       .find.t.searchinstr.e .find.t.replaceinstr.e
  } else {
    DefaultButton .find.b.search .find.t.search.e .find.t.searchinstr.e 
  }

  focus .find.t.search.e
  .find.t.search.e select range 0 end
  
  #grab .find
  tkwait window .find

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }
}

####################################################################
# Procedura FindAgain
# Ricerca la stringa gia' impostata
# Input: 
#   t    Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc FindAgain {t} {
global FindVector

  append FindVector(searchinstr) {}
  append FindVector(searchfor) {}
  append FindVector(replacewith) {}
  set FindVector(widget) $t

  if {$FindVector(searchinstr) == {} && $FindVector(searchfor) == {}} {
    FindBox $t
  } else {
    if {[FindPattern $FindVector(searchinstr) $FindVector(searchfor) $t] == 0} {
      AlertBox -text MsgNotFound
    }
  }
}

####################################################################
# Procedura FindPattern
# Ricerca la stringa  impostata
# Input: 
#   stringinstr  Stringa impostata nel campo istruzione
#   stringop     Stringa impostata nel campo operando
#   t    Nome della finestra di testo
# Ritorna :
#   0 Stringa non trovata; 1 Stringa trovata
####################################################################
proc FindPattern { stringinstr stringop t } {
global DebugState EditPrefs FindVector
global tcl_platform

  set FindVector(widget) $t
  append FindVector(searchinstr) {}
  append FindVector(searchfor) {}
  append FindVector(replacewith) {}

  if {! [info exists FindVector(backwards)]} {
    set FindVector(backwards) 0
  }
  if {! [info exists FindVector(case)]} {
    set FindVector(case) 0
  }

  if {$stringinstr == {} && $stringop == {}} {
    catch {$t tag remove sel sel.first sel.last}
    if { $tcl_platform(platform) != "unix" } {
      catch {$t tag remove selmio}
    }
    return 0
  }
  
  if $FindVector(backwards) {
    set search_option -backwards
    set addChar -1
    set SearchSign -
    set InitSearch end
  } else {
    set search_option -forwards
    set addChar +1
    set SearchSign +
    set InitSearch 1.0
  }

  if {!$FindVector(case)} {
    set CaseNoCase -nocase
  } else {
    set CaseNoCase --
  }
  set count 0

  if { $stringinstr != {} && $stringop == {} } {
    set type -regexp
    set stringa "( |\t)+$stringinstr\( |\t)+"  
  } elseif { $stringinstr == {} && $stringop != {} } {
    set type -exact
    set stringa $stringop
  } elseif { $stringinstr != {} && $stringop != {} } {
    # Se ci sono delle quadre il match non va
    # ci devo aggiungere 3 controbarre 
    set IndiceQuadra [string first "\[" $stringop]
    if { $IndiceQuadra != -1 } {
      set stringop "[string range $stringop 0 [expr $IndiceQuadra -1]]\\\[[string range $stringop [expr $IndiceQuadra +1] end]"
    }
    set IndiceQuadra [string first "\]" $stringop]
    if { $IndiceQuadra != -1 } {
      set stringop "[string range $stringop 0 [expr $IndiceQuadra -1]]\\\][string range $stringop [expr $IndiceQuadra +1] end]"
    }
    set type -regexp
    set stringa "$stringinstr\( |\t)+$stringop"
  }

  set w [$t search $search_option $type -count count \
       $CaseNoCase $stringa "insert $addChar chars"]
  if {"$w"==""} {
    set w [$t search $search_option $type -count count \
       $CaseNoCase $stringa $InitSearch insert]
  }
  if {"$w"!=""} {
    if { $EditPrefs(awl) == 0 } {
      # caso ladder
      EvidenziaLadder $t $w $stringinstr $stringop
    }
    catch {$t tag remove sel sel.first sel.last}
    if { $tcl_platform(platform) != "unix" } {
      catch {$t tag remove selmio}
    }
    MoveCursor $t "$w"
    set TmpInsert [$t index insert]
    MoveCursor $t "insert $SearchSign$count chars"
    $t tag add sel $TmpInsert "$TmpInsert +$count chars"
    if { $tcl_platform(platform) != "unix" } {
      $t tag configure selmio -foreground white -background black
      $t tag add selmio $TmpInsert "$TmpInsert +$count chars"
    }
  } else {
    catch {$t tag remove sel sel.first sel.last}
    if { $tcl_platform(platform) != "unix" } {
      catch {$t tag remove selmio}
    }
    return 0
  }

  ProtocolloAttivaDebug $t IL
  return 1
}

####################################################################
# Procedura FindReplace
# Sotituisce la stringa  impostata
# Input: 
#   searchfor    Stringa da cercare
#   replacewith  Stringa che va a sostituire
#   replaceinstr Stringa di sostituzione istruzione
#   t            Nome della finestra di testo
#   type         Tipo di ricerca
# Ritorna :
#   0 Stringa non trovata; 1 Stringa trovata
####################################################################
proc FindReplace { searchfor replacewith replaceinstr t type } {
global FindVector LadderSelection ContactDef EditPrefs

  if [catch {selection get} s] {
    return 0
  }
  set string [$t get sel.first sel.last]
  set stringtoreplace $replaceinstr
  for {set i 0 } { $i < [string length $string] } {incr i} {
    if { [string range $string $i $i] == "\t" } {
      set stringtoreplace "$stringtoreplace\t"
    } elseif {[string range $string $i $i] == " " } {
      set stringtoreplace "$stringtoreplace "
    }
  }
  set stringtoreplace "$stringtoreplace$replacewith"
  TextReplace $t sel.first sel.last $stringtoreplace 0

  # Vado ad inserire la variabile o il FB in sostituzione
  ReplaceVariable $FindVector(searchfor) $FindVector(replacewith) \
        $FindVector(widget) $type 
  ReplaceFunctionBlock $FindVector(searchfor) $FindVector(replacewith) \
        $FindVector(widget) 
  SetModiFile $FindVector(widget) 1

  if { $EditPrefs(awl) == 0 } {
    set canvas [lindex $LadderSelection 0]
    $canvas delete all
    set righe_colonne_occupate [AnalizeIlEquation $canvas $t \
        $ContactDef(SegmentoCorrente)]
    DimensionaCanvas $canvas [lindex $righe_colonne_occupate 0] \
        [lindex $righe_colonne_occupate 1]
    MostraLadder [winfo parent $canvas]
  }
  return 1
}

####################################################################
# Procedura FindReplaceAll
# Sostituisce nel testo tutte le occorrenze della stringa  impostata
# Input: 
#   frominstr    Istruzione da cercare
#   from         Stringa da cercare
#   to           Stringa che va a sostituire
#   toinstr      Stringa di sostituzione istruzione
#   t            Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc FindReplaceAll {frominstr from to toinstr t} {

  MoveCursor $t 0.0
  set InizioSostituzione -1
  while {[FindPattern $frominstr $from $t]} {
    if { $InizioSostituzione == -1 } {
      set InizioSostituzione [lindex [split [$t index insert] "."] 0]
    } else {
      if { $InizioSostituzione == [lindex [split [$t index insert] "."] 0] } {
        break
      }
    }
    FindReplace $from $to $toinstr $t All
  }
}

####################################################################
# Procedura ReplaceVariable
# Verifica se e' avvenuta una sostituzione di variabile
# Input: 
#   search       Istruzione da cercare
#   replace      Stringa che va a sostituire
#   t            Nome della finestra di testo
#   Type         Tipo di ricerca
# Ritorna :
#   Niente
####################################################################
proc ReplaceVariable { search replace t Type} {
global VariableList

  set search [string toupper $search]
  set replace [string toupper $replace]

  if { [lsearch -exact [array names VariableList] $search] != -1} {
    if {[string first $search [$t get 0.0 end]] ==  -1 || $Type == "All" } {
      if {[info exist VariableList($replace)] == 0 } {
        set VariableList($replace) $VariableList($search)
      }
      unset VariableList($search)
    } else {
      if {[info exist VariableList($replace)] == 0 } {
        set VariableList($replace) \
                        [list [lindex $VariableList($search) 0] \
                        [lindex $VariableList($search) 1] \
                        [lindex $VariableList($search) 2] \
                        [lindex $VariableList($search) 3] \
                        [lindex $VariableList($search) 4] \
                        [lindex $VariableList($search) 5] \
                        [lindex $VariableList($search) 6] \
                        [lindex $VariableList($search) 7]]
      }
    }
  } 
}

####################################################################
# Procedura ReplaceFunctionBlock
# Verifica se e' avvenuta una sostituzione di Function Block
# Input: 
#   search       Istruzione da cercare
#   replace      Stringa che va a sostituire
#   t            Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc ReplaceFunctionBlock { search replace t } {
global FunctionBlockList

  set search [string toupper $search]
  set replace [string toupper $replace]
  if { [lsearch -exact [array names FunctionBlockList] $search] != -1} {
    if {[string first $search [$t get 0.0 end]] ==  -1 } {
      set FunctionBlockList([string toupper $replace]) \
        $FunctionBlockList($search) 
      unset FunctionBlockList([string toupper $search])
    } else {
      set FunctionBlockList([string toupper $replace]) \
        $FunctionBlockList($search) 
    }
  } 
}

####################################################################
# Procedura ReplaceFunctionBlock
# EvidenziaLadder Evidenzia la stringa trovata nel ladder
# Input: 
#   t            Nome della finestra di testo
#   riga         Riga in cui si e' trovata la stringa
#   stringinstr  Istruzione da cercare
#   stringop     Stringa da cercare
# Ritorna :
#   Niente
####################################################################
proc EvidenziaLadder { t riga stringinstr stringop} {
global ContactDef EditPrefs LadderSelection 

  set insertindex [split $riga {.}]
  set line [lindex $insertindex 0]
  set column [lindex $insertindex 1]
  incr column

  if [info exists ContactDef(linee_equazioni)] {
    # Deseleziono le selezioni precedenti
    if [info exist LadderSelection] {
      set canvas [lindex $LadderSelection 0]
      set contatto [lindex $LadderSelection 1]
      if [winfo exists $canvas] {
        $canvas itemconfigure cont$contatto -foreground black
        $canvas itemconfigure var$contatto -fill black
      }
    }
    set ladder [winfo parent $t].ladd.[lindex $ContactDef(ordine_seg) \
       $ContactDef(SegmentoCorrente)]
    $ladder configure -background white
    RicavaSegmento $line
    DisplayEquationNumber [winfo toplevel $t].main.status.position $line $column
    GoToSegment $t $ContactDef(SegmentoCorrente)

    global LINE_CONTACT[lindex $ContactDef(ordine_seg) \
      $ContactDef(SegmentoCorrente)]
    set CanvasTempo LINE_CONTACT[lindex $ContactDef(ordine_seg) \
      $ContactDef(SegmentoCorrente)]
    append CanvasTempo "($line)"
    if { [info exist $CanvasTempo] == 0 } {
      # Se non esiste il contatto associato alla linea passo oltre
      return
    }
    set cmd "lindex \$$CanvasTempo 0"
    set cmd1 "lindex \$$CanvasTempo 1"
    set contatto "[eval $cmd]:[eval $cmd1]"
    set ladder [winfo parent $t].ladd.[lindex $ContactDef(ordine_seg) \
      $ContactDef(SegmentoCorrente)]
    if { $stringinstr != {} && $stringop == {} } {
      $ladder itemconfigure cont$contatto -foreground \
        $EditPrefs(debugcoloracceso)
      set LadderSelection [list $ladder $contatto] 
    } elseif { $stringinstr == {} && $stringop != {} } {
      $ladder itemconfigure var$contatto -fill $EditPrefs(debugcoloracceso)
      set LadderSelection [list $ladder $contatto] 
    } elseif { $stringinstr != {} && $stringop != {} } {
      $ladder itemconfigure cont$contatto -foreground \
        $EditPrefs(debugcoloracceso)
      $ladder itemconfigure var$contatto -fill $EditPrefs(debugcoloracceso)
      set LadderSelection [list $ladder $contatto] 
    }
  } else {
    [winfo toplevel $t].main.status.position configure \
      -text "[GetString Riga] $line\t[GetString Colonna] $column\t\t"
  }
}
