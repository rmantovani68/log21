
            ######################################
            #                                    #
            #     File di definizione del        #
            #     box che contiene l'elenco      #
            #     delle variabili e del box      #
            #     di definizione delle variabili #
            #                                    #
            ######################################


####################################################################
# Procedura OkVariable
# Procedura di conferma della variabile selezionata
# Input: 
#   w                Nome del widget di testo
#   SelectedVariable Variabile selezionata nel list box
# Ritorna :
#   Niente
####################################################################
proc OkVariable { w SelectedVariable} {
global VARS_DEF VariableList

  if {[lsearch -exact [array names VariableList] $SelectedVariable] != -1} {
    set VARS_DEF(tamponata) [lindex $VariableList($SelectedVariable) 0]
    set VARS_DEF(tipo_variabile) [lindex $VariableList($SelectedVariable) 1]
    set VARS_DEF(visibilita) [lindex $VariableList($SelectedVariable) 2]
    set VARS_DEF(locazione) [lindex $VariableList($SelectedVariable) 3]
    set VARS_DEF(initialization) [lindex $VariableList($SelectedVariable) 4]
    set VARS_DEF(description) [lindex $VariableList($SelectedVariable) 5]
    set VARS_DEF(x) [lindex $VariableList($SelectedVariable) 6]
    set VARS_DEF(y) [lindex $VariableList($SelectedVariable) 7]
    if { [string first "." [lindex $VariableList($SelectedVariable) 8]] != -1 } {
      set VARS_DEF(ioAssociation) [lindex $VariableList($SelectedVariable) 8]
    } else {
      set VARS_DEF(ioAssociation) {}
    }
    DefineVariablePanel $w $SelectedVariable 0
  } else {
    if {([string length $SelectedVariable] > 0) && \
        ([string first " " $SelectedVariable] == -1) } {
      if [ConfirmBox -text [GetString ConfirmNewvar]] then {
        DefineVariablePanel $w $SelectedVariable 1
      }
    }
  }
}

####################################################################
# Procedura SearchVariable
# Procedura di ricerca nel testo della variabile selezionata
# Input: 
#   w                Nome del widget di testo
#   SelectedVariable Variabile selezionata nel list box
# Ritorna :
#   Niente
####################################################################
proc SearchVariable { w SelectedVariable} {

  if {[FindPattern "" $SelectedVariable $w] == 0} {
    AlertBox -text MsgNotFound
  }
}

####################################################################
# Procedura DeleteVariable
# Procedura di cancellazione della variabile selezionata
# Input: 
#   w                Nome del widget di testo
#   ListBoxWdg       Nome del widget listBox
#   SelectedVariable Variabile selezionata nel list box
# Ritorna :
#   Niente
####################################################################
proc DeleteVariable { w ListBoxWdg SelectedVariable} {
global VariableList WindowType EntryVariable LoadErrorFB
global MainOfFunctionBlock

  if { $WindowType([TextToTop $w]) == "fb" && \
       [lindex $VariableList($EntryVariable) 2] == "global" } {
    AlertBox -text MsgNoDelGlobal
    return
  }
  if {$EntryVariable != "" && [info exists VariableList($EntryVariable)] } {
    if ![ConfirmBox -text [GetString ConfirmCancelvar]] {
      return
    }
    # setta la variabile di modifica file
    SetModiFile $w 0
    unset VariableList($EntryVariable)
    set TutteVar [.vl.list.lb get 0 end]
    set TrovataVarDel [lsearch -exact $TutteVar $EntryVariable]
    if { $TrovataVarDel != -1 } {
      $ListBoxWdg delete $TrovataVarDel
    }

    if [info exist LoadErrorFB] {
      if {$LoadErrorFB == $EntryVariable } {
        set LoadErrorFB ""
      }
    }

    set EntryVariable ""
    # Cancello la variabile anche nelle copie di elenco variabili
    foreach x [array names WindowType] {
      global VariableList$x
    }
    foreach elenchi [info vars "VariableList*"] {
      if { [string trimleft $elenchi "VariableList"] != "" } {
        set NomeFinestra [string trimleft $elenchi "VariableList"]
        if { $WindowType($NomeFinestra) == "fb" && \
           [TextToTop $w] == $MainOfFunctionBlock($NomeFinestra)} {
	  if { [info exist "$elenchi\($EntryVariable\)"] } {
            unset "$elenchi\($EntryVariable\)"
          }
 	}
      }
    }
  } else {
    AlertBox -text MsgNOVar
  }
}

####################################################################
# Procedura CreaDescrizione
# Procedura che crea la descrizione di una variabile sulla
# pressione del tasto destro
# Input: 
#   t                Nome del widget di testo
#   Lista            Nome del widget lista
#   PopUp            Nome del widget per il pop up
#   x                Coordinata assoluta x del mouse
#   y                Coordinata assoluta y del mouse
# Ritorna :
#   True : si deve visualizzare il pop up
#   False : non si deve visualizzare il pop up
####################################################################
proc CreaDescrizione {Lista x y} {
global EditPrefs VariableList 

  if { [winfo class $Lista] == "Text" } {
    if { [string first "jedit" $Lista] != -1 } {
      set indice [$Lista index @$x,$y]
      set num_linea [lindex [split $indice .] 0]
      set indice_linea [lindex [split $indice .] 1]
      set linea [$Lista get $num_linea.0 $num_linea.end]
      set InizioParola [string wordstart $linea $indice_linea]
      set FineParola [string wordend $linea $indice_linea]
      set UltimoCarattere [string index $linea $FineParola]
      set Variabile [string range $linea $InizioParola [expr $FineParola -1]]
      if {[lsearch -exact [array names VariableList] $Variabile] == -1} {
        return 
      }
    }
  } else {
    set IndiceVariabile [$Lista nearest $y]
    if { $IndiceVariabile == -1 } {
      return 
    }
    set Variabile [$Lista get $IndiceVariabile $IndiceVariabile]
  }


  if { [winfo exist $Lista.mm] == 0 } {
    set menu [menu $Lista.mm -tearoff 0]
  } else {
    set menu $Lista.mm
  }
  $menu delete 0 end

  $menu add command -label $Variabile -font $EditPrefs(textfont) 
  $menu add command -label [format "%-20s %s" [GetString {VariableType}] \
     [lindex $VariableList($Variabile) 1]] \
    -font $EditPrefs(textfont) 
  $menu add command -label [format "%-20s %s" [GetString {Scope:}] \
    [lindex $VariableList($Variabile) 2]] \
    -font $EditPrefs(textfont) 
  $menu add command -label [format "%-20s %s" [GetString {Location:}] \
    [lindex $VariableList($Variabile) 3]]  \
    -font $EditPrefs(textfont) 
  $menu add command -label [format "%-20s %s %s" [GetString VarDimension] \
    [lindex $VariableList($Variabile)) 6] \
    [lindex $VariableList($Variabile) 7]] \
    -font $EditPrefs(textfont)
  $menu add command -label [format "%-20s %s" [GetString VarDescription] \
    [lindex $VariableList($Variabile) 5]] \
    -font $EditPrefs(textfont)
  if { [string first "." [lindex $VariableList($Variabile) 8]] != -1 } {
    $menu add command -label [format "%-20s %s" [GetString CONN] \
    [lindex $VariableList($Variabile) 8]] \
      -font $EditPrefs(textfont)
  } elseif {[lindex $VariableList($Variabile) 8] == "IOS" && \
         [lindex $VariableList($Variabile) 9] != "" && \
         [lindex $VariableList($Variabile) 10] != "" && \
         [lindex $VariableList($Variabile) 11] != "" } {
       set Testo "[lindex $VariableList($Variabile) 8] \
                  [lindex $VariableList($Variabile) 9] \
                  [GetString prt_stam_modu1] \
                  [lindex $VariableList($Variabile) 10] \
                  BYTE [expr [lindex $VariableList($Variabile) 11] / 8] \
                  BIT [expr [lindex $VariableList($Variabile) 11] %8]"
    $menu add command -label [format "%-20s %s" [GetString CONN] $Testo] \
      -font $EditPrefs(textfont)
  }
  if { [winfo class $Lista] == "Text" } {
    if { [string first "jedit" $Lista] != -1 } {
      $menu add command -label [GetString CmdFindAgain] \
        -font $EditPrefs(textfont) -command "SearchVariable $Lista $Variabile"
    }
  }
  tk_popup $menu [expr {$x + [winfo rootx $Lista] - 5}] \
    [expr {$y + [winfo rooty $Lista] - 5}]
}

####################################################################
# Procedura DefinedVariableList
# Elenco delle varili definite e possibilita' di inserirne
# delle nuove
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc DefinedVariableList { w } {
global MsdData EditPrefs VariableList EntryVariable

  if [winfo exists .vl] {
    return
  }

  set varprompt [GetString VlVarPrompt]
  set title [GetString TitleVarList]
    
  set EntryVariable ""

  set MainW [winfo toplevel [focus]]

  set old_focus [focus]

  grab release [grab current]

  toplevel .vl
  wm title .vl $title
  wm minsize .vl 10 10
  wm protocol .vl WM_DELETE_WINDOW procx

  frame .vl.list
  listbox .vl.list.lb -yscroll ".vl.list.sb set" -font $EditPrefs(windowfont) -width 20 \
    -height 20 
  scrollbar .vl.list.sb -command ".vl.list.lb yview"

  frame .vl.variable
  label .vl.variable.l -text $varprompt -anchor e -font $EditPrefs(windowfont)
  entry .vl.variable.e -textvariable EntryVariable -font $EditPrefs(windowfont) \
    -width 20 
  
  bind .vl.list.lb <Button-3> {
    CreaDescrizione %W %x %y
  }

  tixButtonBox .vl.button -orientation horizontal

  .vl.button add ok  -image ok -font $EditPrefs(windowfont) \
     -command [format { OkVariable %s $EntryVariable } $w]

  .vl.button add search  -image search -font $EditPrefs(windowfont) \
     -command [format { SearchVariable %s $EntryVariable } $w]

  .vl.button add delete  -image delete -font $EditPrefs(windowfont) \
     -command [format { DeleteVariable %s %s $EntryVariable } $w .vl.list.lb]

  .vl.button add quit -image cancel -font $EditPrefs(windowfont) \
     -command {destroy .vl}

  CniBalloon [.vl.button subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.vl.button subwidget search] [GetString Search] $EditPrefs(windowfont) 500
  CniBalloon [.vl.button subwidget delete] [GetString DELETE] \
    $EditPrefs(windowfont) 500
  CniBalloon [.vl.button subwidget quit] [GetString EXIT] \
    $EditPrefs(windowfont) 500

  pack .vl.list.sb -side $EditPrefs(scrollbarside) -fill y
  pack .vl.list.lb -side left -expand yes -fill both
  pack .vl.variable.l -side left -pady 10 -padx 10
  pack .vl.variable.e -side left -expand yes -pady 10 -padx 10 -fill x
  pack .vl.list -side top -expand yes -fill both
  pack .vl.variable -side top -expand yes -fill x
  pack .vl.button  -side top -fill both

  wm transient .vl $MainW
  PosizionaFinestre $MainW .vl c


  bind .vl.variable.e <Key-Return> [format { OkVariable %s $EntryVariable } $w]
  bind .vl.variable.e <Key-Return> "+\nbreak"

  # faccio in modo che la scrittura della variabile nel campo di editing sia 
  # maiuscolo
  bind .vl.variable.e <Any-KeyRelease> {
    set EntryVariable [string toupper $EntryVariable]
    if {[string length $EntryVariable] > $MsdData(lunvar)} {
       .vl.variable.e delete $MsdData(lunvar) end
    }
  }

  bind .vl.list.lb <Button-1> {	
    %W selection clear 0 end 
    %W selection set [%W nearest %y]
    if { [.vl.list.lb curselection] != {} } {
       set EntryVariable [lindex [selection get] 0]
    }
    focus %W
  }

  # stessa azione del bottone OK
  bind .vl.list.lb <Double-Button-1> [format { OkVariable %s $EntryVariable } $w]
  bind .vl.list.lb <Double-Button-1> "+\nbreak\n"
  bind .vl.list.lb <Key-Return> [format { OkVariable %s $EntryVariable } $w]


  # Aggiorno la variabile correntemente selezionata
  bind .vl.list.lb <Key-Up> {
    set curr_sel [.vl.list.lb index active]
    if { [expr $curr_sel - 1] < 0 } {
      set EntryVariable [.vl.list.lb get 0] 
    } else {
      set EntryVariable [.vl.list.lb get [expr $curr_sel - 1]] 
    }
  }

  bind .vl.list.lb <Key-Down> {
    set curr_sel [.vl.list.lb index active]
    if { [expr $curr_sel + 1] > [.vl.list.lb index end] } {
      set EntryVariable [.vl.list.lb get end] 
    } else {
      set EntryVariable [.vl.list.lb get [expr $curr_sel + 1]] 
    }
  }

  bind .vl <Key-Escape> {destroy .vl}

  foreach i [lsort [array names VariableList]] {
   .vl.list.lb insert end $i
  }

  focus .vl.variable.e

  set EntryVariable [.vl.list.lb get 0]

  grab .vl
  tkwait window .vl

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
}

####################################################################
# Procedura ControlloPerTipo
# Procedura che controlla la congruenza tra il tipo ed il valore
# di inizializzazione
# Input: 
#   dato       Valore di inizializzazione
#   Messaggio  Flag che indica se dare segnalazione o no
# Ritorna :
#   Niente
####################################################################
proc ControlloPerTipo { dato Messaggio } {
  global VARS_DEF

  if { $VARS_DEF(tipo_variabile) == "BOOL" } {
    if { [string match "0" $dato] == 0 && [string match "1" $dato] == 0 } {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    }
    return 0
  } elseif { $VARS_DEF(tipo_variabile) == "INT" || \
             $VARS_DEF(tipo_variabile) == "DINT" } {
    if { [string first "." $dato] != -1 || \
         [string first "e" $dato] != -1 || \
         [string first "E" $dato] != -1 || \
	 [string first "#" $dato] != -1 || \
	 [string first "'" $dato] != -1 } {
      if { $Messaggio == "SiMess" } { 
        AlertBox -text MsgWrongIni
      }
      return 1
    }
    return 0
  } elseif { $VARS_DEF(tipo_variabile) == "UINT" || \
             $VARS_DEF(tipo_variabile) == "UDINT" } {
    if { [string first "." $dato] != -1 || \
         [string first "e" $dato] != -1 || \
         [string first "E" $dato] != -1 || \
         [string first "#" $dato] != -1 || \
         [string first "'" $dato] != -1 || \
	 [string first "-" $dato] != -1 } {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    }
    return 0
  } elseif { $VARS_DEF(tipo_variabile) == "REAL" || \
             $VARS_DEF(tipo_variabile) == "LREAL" } {
    if {[string match {*.*} $dato] || \
        [string match {*E*} $dato] || \
        [string match {*e*} $dato] } {
      return 0
    } else {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    }
  } elseif { $VARS_DEF(tipo_variabile) == "TIME" } {
    if {[string match {T\#*} $dato] || \
      [string match {t\#*} $dato] || \
      [string match {TIME\#*} $dato] || \
      [string match {time\#*} $dato] } {
      return 0
    } else {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    }
  } elseif { $VARS_DEF(tipo_variabile) == "DATE" } {
    if {[string match {D\#*} $dato] || \
      [string match {d\#*} $dato] || \
      [string match {DATE\#*} $dato] || \
      [string match {date\#*} $dato] } {
      return 0
    } else {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    }
  } elseif { $VARS_DEF(tipo_variabile) == "BYTE" || \
             $VARS_DEF(tipo_variabile) == "WORD" || \
             $VARS_DEF(tipo_variabile) == "DWORD"} {
    if { [string first "." $dato] != -1 || \
         [string first "e" $dato] != -1 || \
         [string first "E" $dato] != -1 || \
         [string first "'" $dato] != -1 } {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    } elseif { [string match {2\#*} $dato] || \
               [string match {8\#*} $dato] || \
               [string match {16\#*} $dato] } {
      return 0
    }
    return 0
  }
}

####################################################################
# Procedura ControlloValoreInizializzazione 
# Procedura che controlla se il valore di inizializzzazione e' 
# corretto
# Input: 
#   Messaggio  Flag che indica se dare segnalazione o no
# Ritorna :
#   Niente
####################################################################
proc ControlloValoreInizializzazione { Messaggio } {
  global VARS_DEF

  if { $VARS_DEF(ioAssociation) != "" } {
    if { ($VARS_DEF(locazione) == "input" || \
          $VARS_DEF(locazione) == "output") } {
      return 0
    } else {
      AlertBox -text MsgNoINT
      return 1
    }
  }

  if { $VARS_DEF(x) == 1 && $VARS_DEF(y) == 1} {
    if {$VARS_DEF(tipo_variabile) == "STRING"} {
      AlertBox -text MsgWrongDimString
      return 1
    }
    return [ControlloPerTipo $VARS_DEF(initialization) $Messaggio]
  } else {
    if {$VARS_DEF(tipo_variabile) == "STRING"} {
      if { $VARS_DEF(y) > 1} {
        AlertBox -text MsgWrongDimString
        return 1
      }
      if {[string match {'*'} $VARS_DEF(initialization)] } {
        return 0
      } else {
        if { $Messaggio == "SiMess" } {
          AlertBox -text MsgWrongIni
        }
        return 1
      }
    } elseif {[string match {\[*\]} $VARS_DEF(initialization)] == 0 } {
      if { $Messaggio == "SiMess" } {
        AlertBox -text MsgWrongIni
      }
      return 1
    } else {
      set temp [string trimleft $VARS_DEF(initialization) "\["]
      set temp [string trimright $temp "\]"]
      foreach x [split $temp ","] {
        if {[string first "(" $x] != -1 && [string first ")" $x] != -1 } {
          set x [string range $x [expr \
                [string first "(" $x] + 1] [expr [string first ")" $x] -1]]
        }
        if [ControlloPerTipo $x $Messaggio] {
          return 1
        } 
      }
      return 0
    }
  }
return 0
}

####################################################################
# Procedura InizializzaVariabile
# Procedura che inizializza la variabile
# Input: 
#   val_iniz  Valore di inizializzazione
# Ritorna :
#   Niente
####################################################################
proc InizializzaVariabile { val_iniz } {
  global VARS_DEF 

  if [ControlloValoreInizializzazione NoMess] {
    if {$VARS_DEF(x) == 1 } {
      set VARS_DEF(initialization) $val_iniz
    } elseif { $VARS_DEF(x) > 1 && $VARS_DEF(y) == 1 } { 
      set VARS_DEF(initialization) "\[$VARS_DEF(x)\($val_iniz\)\]"
    } elseif {$VARS_DEF(x) > 1 && $VARS_DEF(y) > 1 } {
      set VARS_DEF(initialization) "\[[expr $VARS_DEF(x) * $VARS_DEF(y)]\(0.0\)\]"
    }
  }
}

####################################################################
# Procedura InputOutputRetainCommand
# Procedura chiamata sulla selezione di variabile di 
# input o output
# Input: 
#   CallType  Identificativo del bottone chiamante
# Ritorna :
#   Niente
####################################################################
proc InputOutputRetainCommand { CallType } {
  global VARS_DEF 

  if { $CallType == "retain" } {
    if { $VARS_DEF(locazione) == "input" } {
      AlertBox -text MsgInputNoRet
      set VARS_DEF(tamponata) noretain
    } elseif {$VARS_DEF(locazione) == "output" } {
      AlertBox -text MsgOutputNoRet
      set VARS_DEF(tamponata) noretain
    }
  } elseif { $CallType == "Input"  || $CallType == "Output" } {
    if { $VARS_DEF(tamponata) == "retain" } {
      if { $CallType == "Input" } {
        AlertBox -text MsgInputNoRet
      } elseif { $CallType == "Output" } {
        AlertBox -text MsgOutputNoRet
      }
      set VARS_DEF(tamponata) noretain
    }
    if {$VARS_DEF(x) > 1 } {
      set VARS_DEF(x) 1
    }
    if {$VARS_DEF(y) > 1 } {
      set VARS_DEF(y) 1
    }
    set VARS_DEF(initialization) 0
  }
}

####################################################################
# Procedura ControlloDimensioni
# Procedura chiamata alla pressione di un tasto su uno dei
# campi per le dimensioni per aggiustare l'inizializzazione
# Input: 
#   CallType  Identificativo del campo che si sta digitando
#   Key       Tasto premuto
# Ritorna :
#   Niente
####################################################################
proc ControlloDimensioni { CallType Key} {
global VARS_DEF 

  if { [string  match  {[0-9]} $Key] == 1 || $Key == "BackSpace"} {
    if { ($CallType == "y" && $VARS_DEF(x) == 1) || $CallType == "x" } {
      if { $VARS_DEF(tipo_variabile) != "STRING"} {
        set old_init $VARS_DEF(initialization)
        set VARS_DEF(initialization) ""
        if { $VARS_DEF(tipo_variabile) == "BOOL" && ($VARS_DEF(locazione) == "input" || \
             $VARS_DEF(locazione) == "output") } {
          if {$VARS_DEF(CallType) > 1 } {
            set VARS_DEF(CallType) 1
          }
        }
	if { $VARS_DEF(tipo_variabile) == "BOOL" || \
             $VARS_DEF(tipo_variabile) == "INT" || \
             $VARS_DEF(tipo_variabile) == "UINT" || \
	     $VARS_DEF(tipo_variabile) == "DINT" || \
	     $VARS_DEF(tipo_variabile) == "UDINT" } {
           
           if { $VARS_DEF(x) != "" && $VARS_DEF(y) != "" } {
             set VARS_DEF(initialization) "\[[expr $VARS_DEF(x) * $VARS_DEF(y)]\(0\)\]"
           }
         } elseif { $VARS_DEF(tipo_variabile) == "REAL" || \
                    $VARS_DEF(tipo_variabile) == "LREAL" } {
           if { $VARS_DEF(x) != "" && $VARS_DEF(y) != "" } {
	     set VARS_DEF(initialization) "\[[expr $VARS_DEF(x) * $VARS_DEF(y)]\(0.0\)\]"
            }
         }
       }
     } else {
      AlertBox -text MsgNoFirstDim
    }
  } else {
    if {$CallType == "x" } {
      set VARS_DEF(x) [string range $VARS_DEF(x) 0 \
        [expr [string length $VARS_DEF(x)] -2]]
    } elseif { $CallType == "y" } {
      set VARS_DEF(y) [string range $VARS_DEF(y) 0 \
        [expr [string length $VARS_DEF(y)] -2]]
    }
  }
}

####################################################################
# Procedura SaveDefine
# Procedura chiamata alla pressione del bottone di salva
# Input: 
#   t      Nome del widget di testo
#   tl      Nome del pannello di definizione variabile
# Ritorna :
#   Niente
####################################################################
proc SaveDefine { t tl } {
global VARS_DEF VariableList WindowType MainOfFunctionBlock

  if { [TestTipoFinestra $t]=="fb" && ($VARS_DEF(visibilita)=="global" || \
     $VARS_DEF(visibilita)=="extern") } {
    AlertBox -text MsgNoDefGlob
    return
  }

  set errore_inizializzazione [ControlloValoreInizializzazione SiMess]

  if {$errore_inizializzazione == 0} {
    # Se esiste la connessione la salvo
    if [info exist VariableList($VARS_DEF(nome_variabile))] {
      set VariableList($VARS_DEF(nome_variabile)) \
        [lreplace $VariableList($VARS_DEF(nome_variabile)) 0 7 $VARS_DEF(tamponata) \
                                 $VARS_DEF(tipo_variabile) $VARS_DEF(visibilita) \
                                 $VARS_DEF(locazione) $VARS_DEF(initialization) \
                                 $VARS_DEF(description) $VARS_DEF(x) \
			         $VARS_DEF(y) \
                                 ]
      if { $VARS_DEF(ioAssociation) != "" || \
                 ($VARS_DEF(OldIoAssociation) != "" && $VARS_DEF(ioAssociation) == "")} {
        if { [llength $VariableList($VARS_DEF(nome_variabile))] >= 9 } {
	  set VariableList($VARS_DEF(nome_variabile)) \
            [lreplace $VariableList($VARS_DEF(nome_variabile)) 8 8 \
            $VARS_DEF(ioAssociation)]
        } else {
	  set VariableList($VARS_DEF(nome_variabile)) \
            [lappend VariableList($VARS_DEF(nome_variabile)) $VARS_DEF(ioAssociation)]
        }
      }
    } else {
      set VariableList($VARS_DEF(nome_variabile)) [list $VARS_DEF(tamponata) \
                       $VARS_DEF(tipo_variabile) $VARS_DEF(visibilita) \
                       $VARS_DEF(locazione) $VARS_DEF(initialization) \
                       $VARS_DEF(description) $VARS_DEF(x) $VARS_DEF(y) \
		       ] 
      if { $VARS_DEF(ioAssociation) != "" } {
        set VariableList($VARS_DEF(nome_variabile)) \
             [lappend VariableList($VARS_DEF(nome_variabile)) $VARS_DEF(ioAssociation)]
      }
    }
    if { $VARS_DEF(visibilita) == "global" } {
      # Verifico se ci sono dei FB aperti ed eventualmente 
      # aggiorno i rispettivi elenchi variabili
      foreach x [array names WindowType] {
        global VariableList$x
      }
      foreach elenchi [info vars "VariableList*"] {
        if { [string trimleft $elenchi "VariableList"] != "" } {
          set NomeFinestra [string trimleft $elenchi "VariableList"]
          if { $WindowType($NomeFinestra) == "fb" && \
             [TextToTop $t] == $MainOfFunctionBlock($NomeFinestra)} {
            set "$elenchi\($VARS_DEF(nome_variabile)\)" \
              $VariableList($VARS_DEF(nome_variabile))
          }
        }
      }
    }
    # inserisco la nuova variabile nella lista
    if [winfo exists .vl.list.lb] {
      if {[lsearch -exact [.vl.list.lb get 0 end] $VARS_DEF(nome_variabile)] == -1} {
        .vl.list.lb insert end $VARS_DEF(nome_variabile)
      }
    }
    SetModiFile $t 0
    destroy $tl 
  }
}

####################################################################
# Procedura ExitDefine
# Procedura chiamata alla pressione del bottone di uscita
# Input: 
#   vl      Nome del pannello di definizione variabile
# Ritorna :
#   Niente
####################################################################
proc ExitDefine { tl } {
global VARS_DEF VariableList

  destroy $tl
  set VARS_DEF(x) -1 

  if { $VARS_DEF(OldIoAssociation) != "" } {
    set VariableList($VARS_DEF(nome_variabile)) \
          [lreplace $VariableList($VARS_DEF(nome_variabile)) 8 8 \
          $VARS_DEF(OldIoAssociation)]
  }
  if { $VARS_DEF(OldIosConn) != "" } {
    set VariableList($VARS_DEF(nome_variabile)) [lreplace \
      $VariableList($VARS_DEF(nome_variabile)) 8 11 \
      [lindex $VARS_DEF(OldIosConn) 0] [lindex $VARS_DEF(OldIosConn) 1] \
      [lindex $VARS_DEF(OldIosConn) 3] [expr ([lindex $VARS_DEF(OldIosConn) 5] *8) + \
      [lindex $VARS_DEF(OldIosConn) 7]]]
  }
}

####################################################################
# Procedura DeselectConnection
# Procedura chiamata alla pressione del tasto di memoria
# per verificare l'eventuale deselezione della connessione
# Input: 
#   frame1 - frame5  Frame contenenti i bottoni da attivare 
#                    o disattivare
# Ritorna :
#   Niente
####################################################################
proc DeselectConnection { frame1 frame2 frame3 frame4 frame5 entryWdg} {
global VariableList VARS_DEF EditPrefs 

  if {[lindex $VariableList($VARS_DEF(nome_variabile)) 8] == "IOS" && \
      [lindex $VariableList($VARS_DEF(nome_variabile)) 9] != "" && \
      [lindex $VariableList($VARS_DEF(nome_variabile)) 10] != "" && \
      [lindex $VariableList($VARS_DEF(nome_variabile)) 11] != "" } {
    set VARS_DEF(OldIosConn) $VARS_DEF(IosConn)
    set VARS_DEF(IosConn) ""
    set VariableList($VARS_DEF(nome_variabile)) [lreplace \
      $VariableList($VARS_DEF(nome_variabile)) 8 11 {} {} {} {}]
    destroy $entryWdg
    entry $entryWdg -width 25 -textvariable VARS_DEF(ioAssociation) \
        -font $EditPrefs(windowfont)
    pack $entryWdg -in [winfo parent $entryWdg] \
      -expand yes -side left -padx 10 -pady 10
  }
  if { [string first "." [lindex $VariableList($VARS_DEF(nome_variabile)) 8]] != -1 } {
    set VARS_DEF(ioAssociation) ""
    set VariableList($VARS_DEF(nome_variabile)) [lreplace \
      $VariableList($VARS_DEF(nome_variabile)) 8 8 {}]
  }
  AttivaDisattivaBottoni Attiva $frame1 $frame2 $frame3 $frame4 $frame5 $entryWdg
}

####################################################################
# Procedura AttivaDisattivaBottoni
# Procedura che attiva o disattiva i toggle button
# Input: 
#   Flag     (Attiva/Disattiva) fla di attivazione o disattivazione
#            dei bottoni
#   frame1 - frame5  Frame contenenti i bottoni da attivare 
#                    o disattivare
# Ritorna :
#   Niente
####################################################################
proc AttivaDisattivaBottoni { Flag frame1 frame2 frame3 frame4 frame5 frame6} {

  if { $Flag == "Attiva" } {
    set Stato normal
  } elseif { $Flag == "Disattiva" } {
    set Stato disabled
  }

  foreach figli [winfo children $frame1] {
    if { "[winfo class $figli]" == "Radiobutton" } {
      $figli config -state $Stato
    }
  }
  foreach figli [winfo children $frame2] {
    if { "[winfo class $figli]" == "Radiobutton" } {
      $figli config -state $Stato
    }
  }
  foreach figli [winfo children $frame3] {
    if { "[winfo class $figli]" == "Radiobutton" } {
      $figli config -state $Stato
    }
  }
  foreach figli [winfo children $frame4] {
    if { "[winfo class $figli]" == "Radiobutton" } {
      $figli config -state $Stato
    }
  }
  foreach figli [winfo children $frame5] {
    if { "[winfo class $figli]" == "Radiobutton" } {
      $figli config -state $Stato
    }
  }
  foreach figli [winfo children $frame6] {
    if { "[winfo class $figli]" == "Entry" } {
      $figli config -state $Stato
    }
  }
}

####################################################################
# Procedura ActivateDeactivateIO
# Procedura che attiva o disattiva un eventuale io speciale
# Input: 
#   ActionType     	Tipo di azione da fare sull' IO
#   SpecialIOEntry     	Nome del widget che definisce l'IO speciale
# Ritorna :
#   Niente
####################################################################
proc ActivateDeactivateIO { ActionType SpecialIOEntry } {
global IoconfModificato VARS_DEF

  if {$VARS_DEF(ioAssociation) == "" } {
    return
  }
  set IoconfModificato "yes"

  if { $ActionType == "active" } {
    $SpecialIOEntry configure -state normal
    $SpecialIOEntry configure -fg black
    set VARS_DEF(ioAssociation) [string trimright $VARS_DEF(ioAssociation) "_S"]
  } elseif { $ActionType == "deactive" } {
    $SpecialIOEntry configure -state disabled
    $SpecialIOEntry configure -fg lightgray
    if { [string first "_S" $VARS_DEF(ioAssociation)] == -1 } {
      set VARS_DEF(ioAssociation) "$VARS_DEF(ioAssociation)_S"
    }
  }
}

####################################################################
# Procedura DefineVariablePanel
# Procedura che crea il pannello di definizione variabili del Plc
# Input: 
#   -title - title of toplevel window
# Ritorna :
#   Niente
####################################################################
proc DefineVariablePanel { t variable_name nuova_variabile } {
  global VARS_DEF VariableList EditPrefs SiIoconf

  set tl .vars
  if [winfo exist $tl] {
    return
  }
  
  grab release [grab current]

  # se FB se variabili di default sono locali
  if { [TestTipoFinestra [TextToTop $t]] == "main" } {
    set tipo_visibilita "global"
  } else {
    set tipo_visibilita "local"
  }
  if { $nuova_variabile == 1 } {
    set VARS_DEF(tamponata) noretain
    set VARS_DEF(tipo_variabile) BOOL
    set VARS_DEF(visibilita) $tipo_visibilita
    set VARS_DEF(locazione) memory
    set VARS_DEF(x) 1
    set VARS_DEF(y) 1
    set VARS_DEF(initialization) 0
    set VARS_DEF(description) ""
    set VARS_DEF(ioAssociation) ""
  }

  set VARS_DEF(nome_variabile) $variable_name
  set VARS_DEF(OldIoAssociation) $VARS_DEF(ioAssociation)
  set VARS_DEF(OldIosConn) ""

  set old_focus [focus]

  set MainW [winfo toplevel [focus]]

  toplevel $tl
  wm title $tl "[GetString VarName] $VARS_DEF(nome_variabile)"
  wm protocol $tl WM_DELETE_WINDOW procx

  frame $tl.horizontal

  frame $tl.retain
  radiobutton $tl.retain.tamp -anchor w -text [GetString VarRetain] \
	-variable VARS_DEF(tamponata) -value retain -font $EditPrefs(windowfont) \
        -selectcolor $EditPrefs(togglecolor) \
        -command { InputOutputRetainCommand "retain" }
  radiobutton $tl.retain.volat -anchor w -text [GetString VarNoretain] \
    -variable VARS_DEF(tamponata) -value noretain -selectcolor $EditPrefs(togglecolor) \
    -font $EditPrefs(windowfont)

  frame $tl.type
  label $tl.type.l -text [GetString {VariableType}] -font $EditPrefs(windowfont)
  radiobutton $tl.type.boolean -anchor w -text "BOOLEAN" \
    -variable VARS_DEF(tipo_variabile) -value BOOL -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0 }
  radiobutton $tl.type.int -anchor w -text "INT" \
    -variable VARS_DEF(tipo_variabile) -value INT -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0 }
  radiobutton $tl.type.uint -anchor w -text "UINT" \
    -variable VARS_DEF(tipo_variabile) -value UINT -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0}
  radiobutton $tl.type.dint -anchor w  -text "DINT" \
    -variable VARS_DEF(tipo_variabile) -value DINT -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0}
  radiobutton $tl.type.udint -anchor w  -text "UDINT" \
    -variable VARS_DEF(tipo_variabile) -value UDINT -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0}
  radiobutton $tl.type.real -anchor w -text "REAL" \
    -variable VARS_DEF(tipo_variabile) -value REAL -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0.0}
  radiobutton $tl.type.lreal -anchor w  -text "LREAL" \
    -variable VARS_DEF(tipo_variabile) -value LREAL -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0.0}

  frame $tl.type1
  label $tl.type1.l -text "    "
  radiobutton $tl.type1.time -anchor w -text "TIME" \
    -variable VARS_DEF(tipo_variabile) -value TIME -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile T#0s} 
  radiobutton $tl.type1.date -anchor w -text "DATE" \
    -variable VARS_DEF(tipo_variabile) -value DATE -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile D#0001-01-01} 
  radiobutton $tl.type1.string -anchor w -text "STRING" \
    -variable VARS_DEF(tipo_variabile) -value STRING -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { set VARS_DEF(initialization) ''} 
  radiobutton $tl.type1.byte -anchor w -text "BYTE" \
    -variable VARS_DEF(tipo_variabile) -value BYTE -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0} 
  radiobutton $tl.type1.word -anchor w -text "WORD" \
    -variable VARS_DEF(tipo_variabile) -value WORD -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0} 
  radiobutton $tl.type1.dword -anchor w -text "DWORD" \
    -variable VARS_DEF(tipo_variabile) -value DWORD -font $EditPrefs(windowfont) \
    -selectcolor $EditPrefs(togglecolor) -command { InizializzaVariabile 0} 

  frame $tl.locat
  label $tl.locat.l -text [GetString {Location:}] -font $EditPrefs(windowfont)
  radiobutton $tl.locat.input -anchor w -text [GetString VarInput] \
    -font $EditPrefs(windowfont) -selectcolor $EditPrefs(togglecolor) \
    -variable VARS_DEF(locazione) -value input \
    -command { InputOutputRetainCommand "Input" }
  radiobutton $tl.locat.output -anchor w  -text [GetString VarOutput] \
    -font $EditPrefs(windowfont)  -variable VARS_DEF(locazione) -value output \
    -selectcolor $EditPrefs(togglecolor) \
    -command { InputOutputRetainCommand "Output" }
  radiobutton $tl.locat.memory -anchor w -text [GetString VarMemory] \
    -font $EditPrefs(windowfont) -selectcolor $EditPrefs(togglecolor) \
    -variable VARS_DEF(locazione) -value memory

  frame $tl.scope
  label $tl.scope.l -text [GetString {Scope:}] -font $EditPrefs(windowfont)
  radiobutton $tl.scope.global -anchor w -text [GetString VarGlobal] \
    -font $EditPrefs(windowfont) -selectcolor $EditPrefs(togglecolor) \
    -variable VARS_DEF(visibilita) -value global 
  radiobutton $tl.scope.local -anchor w -text [GetString VarLocal] \
    -font $EditPrefs(windowfont) -selectcolor $EditPrefs(togglecolor) \
    -variable VARS_DEF(visibilita) -value local 
  radiobutton $tl.scope.extern -anchor w -text [GetString VarExtern] \
    -font $EditPrefs(windowfont) -selectcolor $EditPrefs(togglecolor) \
    -variable VARS_DEF(visibilita) -value extern 
  
  frame $tl.dimension
  label $tl.dimension.l -text [GetString VarDimension] -font $EditPrefs(windowfont)
  entry $tl.dimension.x -width 10 -textvariable VARS_DEF(x) -font $EditPrefs(windowfont)
  entry $tl.dimension.y -width 10 -textvariable VARS_DEF(y) -font $EditPrefs(windowfont)

  frame $tl.initialization
  label $tl.initialization.l -text [GetString VarInitialization] \
                             -font $EditPrefs(windowfont)
  entry $tl.initialization.e -width 25 -textvariable VARS_DEF(initialization) \
    -font $EditPrefs(windowfont)

  frame $tl.description
  label $tl.description.l -text [GetString VarDescription] -font $EditPrefs(windowfont)
  entry $tl.description.e -width 25 -textvariable VARS_DEF(description) \
    -font $EditPrefs(windowfont)

  frame $tl.ioAssociation
  label $tl.ioAssociation.l -text [GetString VarIoAssociation] \
                            -font $EditPrefs(windowfont)
  if {$nuova_variabile == 0 } {
     if {[lindex $VariableList($VARS_DEF(nome_variabile)) 8] == "IOS" && \
         [lindex $VariableList($VARS_DEF(nome_variabile)) 9] != "" && \
         [lindex $VariableList($VARS_DEF(nome_variabile)) 10] != "" && \
         [lindex $VariableList($VARS_DEF(nome_variabile)) 11] != "" } {
       label $tl.ioAssociation.e -width 30 -textvariable VARS_DEF(IosConn) \
          -font $EditPrefs(windowfont)
       set Testo "[lindex $VariableList($VARS_DEF(nome_variabile)) 8] \
                  [lindex $VariableList($VARS_DEF(nome_variabile)) 9] \
                  [GetString prt_stam_modu1] \
                  [lindex $VariableList($VARS_DEF(nome_variabile)) 10] \
                  BYTE [expr [lindex $VariableList($VARS_DEF(nome_variabile)) 11] / 8] \
                  BIT [expr [lindex $VariableList($VARS_DEF(nome_variabile)) 11] %8]" 
       set VARS_DEF(IosConn) $Testo
     } else {
       entry $tl.ioAssociation.e -width 25 -textvariable VARS_DEF(ioAssociation) \
         -font $EditPrefs(windowfont)
     }
  } else {
    entry $tl.ioAssociation.e -width 25 -textvariable VARS_DEF(ioAssociation) \
      -font $EditPrefs(windowfont)
  }

  tixButtonBox $tl.b -orientation horizontal
  $tl.b add save -image save -font $EditPrefs(windowfont) \
     -command [format { SaveDefine %s %s } $t $tl]

  if {$nuova_variabile == 0 } {
    if {([lindex $VariableList($VARS_DEF(nome_variabile)) 8] == "IOS" && \
        [lindex $VariableList($VARS_DEF(nome_variabile)) 9] != "" && \
        [lindex $VariableList($VARS_DEF(nome_variabile)) 10] != "" && \
        [lindex $VariableList($VARS_DEF(nome_variabile)) 11] != "" ) || \
        $VARS_DEF(ioAssociation) != "" } {
     $tl.b add disconnect -image free -font $EditPrefs(windowfont) \
        -command [format { DeselectConnection %s %s %s %s %s %s} \
        $tl.retain $tl.type $tl.type1 $tl.locat $tl.scope $tl.ioAssociation.e]
     CniBalloon [$tl.b subwidget disconnect] [GetString ConnectFree] \
       $EditPrefs(windowfont) 500
     AttivaDisattivaBottoni Disattiva $tl.retain $tl.type $tl.type1 $tl.locat $tl.scope $tl.dimension
    }
  }

  if { $SiIoconf == "yes" && $VARS_DEF(ioAssociation) != ""} {
    $tl.b add activeio -image activio -font $EditPrefs(windowfont) \
       -command [format { ActivateDeactivateIO active %s } $tl.ioAssociation.e]
    $tl.b add deactiveio -image deactio -font $EditPrefs(windowfont) \
       -command [format { ActivateDeactivateIO deactive %s } $tl.ioAssociation.e]
    CniBalloon [$tl.b subwidget activeio] [GetString ActiveIo] $EditPrefs(windowfont) 500
    CniBalloon [$tl.b subwidget deactiveio] [GetString DeactiveIo] \
      $EditPrefs(windowfont) 500
    if { [string first "_S" $VARS_DEF(ioAssociation)] == -1 } {
      $tl.ioAssociation.e configure -state normal
      $tl.ioAssociation.e configure -fg black
    } else {
      $tl.ioAssociation.e configure -state disabled
      $tl.ioAssociation.e configure -fg lightgray
    }
  }

  $tl.b add cancel -image cancel -font $EditPrefs(windowfont) \
     -command [format { ExitDefine %s } $tl]

  CniBalloon [$tl.b subwidget save] [GetString VarSave] $EditPrefs(windowfont) 500
  CniBalloon [$tl.b subwidget cancel] [GetString CANCEL] $EditPrefs(windowfont) 500

  pack $tl.type.l $tl.type.boolean $tl.type.int $tl.type.uint \
    $tl.type.dint $tl.type.udint $tl.type.real $tl.type.lreal \
    -in $tl.type -side top -fill x -anchor n

  pack $tl.type1.l $tl.type1.time $tl.type1.date $tl.type1.string \
    $tl.type1.byte $tl.type1.word $tl.type1.dword \
    -in $tl.type1 -side top -fill x -anchor n
  
  pack $tl.scope.l $tl.scope.global $tl.scope.local $tl.scope.extern \
    -in $tl.scope -side top -fill x -anchor n
  
  pack $tl.locat.l $tl.locat.memory $tl.locat.input $tl.locat.output \
    	-in $tl.locat -side top -fill x -anchor n

  pack $tl.retain.volat $tl.retain.tamp -in $tl.retain -side top -fill x -anchor n

  pack $tl.type -in $tl.horizontal -side left -expand yes -fill y
  pack $tl.type1 -in $tl.horizontal -side left -expand yes -fill y
  pack [SeparationLine $tl.horizontal] -in $tl.horizontal -side left -fill y
  pack $tl.scope -in $tl.horizontal -side left -expand yes -fill y
  pack [SeparationLine $tl.horizontal] -in $tl.horizontal -side left -fill y
  pack $tl.locat -in $tl.horizontal -side left -expand yes -fill y

  pack [SeparationLine $tl.horizontal] -in $tl.horizontal -side left -fill y
  pack $tl.retain -in $tl.horizontal -side left -expand yes -fill y

  pack $tl.dimension.l -in $tl.dimension -side left
  pack $tl.dimension.x $tl.dimension.y -in $tl.dimension \
    -expand yes -side left -padx 5 -pady 5

  pack $tl.initialization.l -in $tl.initialization -side left
  pack $tl.initialization.e -in $tl.initialization \
    -expand yes -side left -padx 10 -pady 10

  pack $tl.description.l -in $tl.description -side left
  pack $tl.description.e -in $tl.description \
    -expand yes -side left -padx 10 -pady 10

  pack $tl.ioAssociation.l -in $tl.ioAssociation -side left
  pack $tl.ioAssociation.e -in $tl.ioAssociation \
    -expand yes -side left -padx 10 -pady 10

  pack [SeparationLine $tl] -in $tl -side top -fill x
  pack $tl.horizontal -in $tl -side top -expand yes -fill x
  pack [SeparationLine $tl] -in $tl -side top -fill x
  pack $tl.dimension -in $tl -side top -expand yes -fill x
  pack $tl.initialization -in $tl -side top -expand yes -fill x
  pack $tl.description -in $tl -side top -expand yes -fill x
  pack [SeparationLine $tl] -in $tl -side top -fill x
  pack $tl.ioAssociation -in $tl -side top -expand yes -fill x
  pack $tl.b -in $tl -side top -expand yes -fill x

  wm transient $tl $MainW
  PosizionaFinestre $MainW $tl c

  focus $tl

  DefaultButton [$tl.b subwidget save] $tl.initialization.e $tl.description.e \
    $tl.ioAssociation.e $tl.dimension.x $tl.dimension.y $tl

  focus $tl.initialization.e

  bind $tl <Key-Tab> "focus $tl.initialization.e"

  # tengo sotto controllo i campi relativi alla dimensione per 
  # vedere se ci sono array
  bind $tl.dimension.x <Any-KeyRelease> { ControlloDimensioni x %K }
  bind $tl.dimension.y <Any-KeyRelease> { ControlloDimensioni y %K }
  bind $tl.dimension.x <FocusIn> { %W selection range 0 end }
  bind $tl.dimension.y <FocusIn> { %W selection range 0 end }

  bind $tl <Key-Escape> [format { ExitDefine %s} $tl]

  grab $tl
  tkwait window $tl

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }
}
