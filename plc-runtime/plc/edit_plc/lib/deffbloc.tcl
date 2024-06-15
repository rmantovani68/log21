
            ######################################
            #                                    #
            #     File di definizione dei        #
            #     box per la creazione di        #
            #     blocchi funzionali             #
            #                                    #
            ######################################

####################################################################
# Procedura OkFunctionBlock
# Procedura eseguita sulla pressione del bottone OK
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc OkFunctionBlock { w } {
global FBResult FunctionBlockList 

  if {[lsearch -exact [array names FunctionBlockList] $FBResult] != -1} {
    SelectionFunctionBlock $w $FBResult OldFB
  } else {
    if {([string length $FBResult] > 0) && ([string first " " $FBResult] == -1) } {
      if [ConfirmBox -text [GetString ConfirmNewfb]] then {
        if [winfo exists .fbl.list.lb] {
          .fbl.list.lb insert end $FBResult
	}
        SelectionFunctionBlock $w $FBResult NewFB
      }
    }
  }
}

####################################################################
# Procedura DeleteFunctionBlock
# Procedura eseguita sulla pressione del bottone Delete
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc DeleteFunctionBlock { w } {
global FBResult FunctionBlockList 

  if {$FBResult != "" && [info exists FunctionBlockList($FBResult)] } {
    if ![ConfirmBox -text [GetString ConfirmCancelfb]] {
      return
    }
    # setto la variabile di modifica file
    SetModiFile $w 0
    unset FunctionBlockList($FBResult)
    set FBResult ""
    .fbl.list.lb delete [.fbl.list.lb curselection]
  }
}

####################################################################
# Procedura CancelFunctionBlock
# Procedura eseguita sulla pressione del bottone Cancel
# Input: 
#   w          Nome del widget del pannello dell' elenco FB
# Ritorna :
#   Niente
####################################################################
proc CancelFunctionBlock { w } {
global FBResult

  set FBResult ""
  destroy $w
}

####################################################################
# Procedura DefinedFunctionBlockList
# Definizione del box con l'elenco dei blocchi funzionali definiti
# principale
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc DefinedFunctionBlockList { w } {
global MsdData EditPrefs FBResult FunctionBlockList 

  if { $EditPrefs(awl) == 0 } {
    # Caso LADDER
    CmdNewFunctioBlock $w
    return
  }

  if [winfo exists .fbl] {
    return
  }

  set fbprompt [GetString "FblFbPrompt"]
  set title [GetString "TitleIstanzeFbList"]

  set FBResult ""
  
  set MainW [winfo toplevel [focus]]

  set old_focus [focus]

  grab release [grab current]

  toplevel .fbl
  wm title .fbl $title
  wm minsize .fbl 10 10
  wm protocol .fbl WM_DELETE_WINDOW procx

  frame .fbl.list
  listbox .fbl.list.lb -yscroll ".fbl.list.sb set" -font $EditPrefs(windowfont) \
    -width 15 -height 20
  scrollbar .fbl.list.sb -command ".fbl.list.lb yview"
  frame .fbl.function
  label .fbl.function.l -text $fbprompt -font $EditPrefs(windowfont) -anchor e
  entry .fbl.function.e -text $FBResult -font $EditPrefs(windowfont) \
    -textvariable FBResult -width 12
  
  tixButtonBox .fbl.b -orientation horizontal
  .fbl.b add ok -image ok -font $EditPrefs(windowfont) -command [format {
      OkFunctionBlock %s} $w]
  .fbl.b add delete -image delete -font $EditPrefs(windowfont) -command [format {
      DeleteFunctionBlock %s} $w]
  .fbl.b add cancel -image cancel -font $EditPrefs(windowfont) -command [format {
      CancelFunctionBlock %s} .fbl]

  CniBalloon [.fbl.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.fbl.b subwidget delete] [GetString DELETE] $EditPrefs(windowfont) 500
  CniBalloon [.fbl.b subwidget cancel] [GetString EXIT] $EditPrefs(windowfont) 500

  pack .fbl.list.sb -side $EditPrefs(scrollbarside) -fill y
  pack .fbl.list.lb -side left -expand yes -fill both
  
  pack .fbl.function.l -side left -pady 10 -padx 10
  pack .fbl.function.e -side left -expand yes -pady 10 -padx 10 -fill x
  
  pack .fbl.list -side top -expand yes -fill both
  pack .fbl.function -side top -expand yes -fill x
  pack .fbl.b  -side bottom -fill x

  wm transient .fbl $MainW
  PosizionaFinestre $MainW .fbl c

  .fbl.function.e insert end $FBResult

  focus .fbl.list.lb

  bind .fbl.function.e <Key-Return> {
    [.fbl.b subwidget ok] invoke
  }
  bind .fbl.function.e <Key-Return> "+\nbreak"

  # faccio in modo che la scrittura della variabile nel campo di editing sia 
  # maiuscolo
  bind .fbl.function.e <Any-KeyRelease> {
    set FBResult [string toupper $FBResult] 
    if {[string length $FBResult] > $MsdData(lunvar)} {
       .fbl.function.e delete $MsdData(lunvar) end
    }
  }

  bind .fbl.list.lb <Button-1> {
    %W selection clear 0 end 
    %W selection set [%W nearest %y]
    if { [.fbl.list.lb curselection] != {} } {
       set FBResult [lindex [selection get] 0]
    }
    focus %W
  }

  bind .fbl.list.lb <Double-Button-1> {
    [.fbl.b subwidget ok] invoke
  }

  bind .fbl.list.lb <Double-Button-1> "+\nbreak\n"
  
  # Aggiorno la variabile correntemente selezionata
  bind .fbl.list.lb <Key-Up> {
    set curr_sel [.fbl.list.lb index active]
    if { [expr $curr_sel - 1] < 0 } {
      set FBResult [.fbl.list.lb get 0] 
    } else {
      set FBResult [.fbl.list.lb get [expr $curr_sel - 1]] 
    }
  }
  bind .fbl.list.lb <Key-Down> {
    set curr_sel [.fbl.list.lb index active]
    if { [expr $curr_sel + 1] > [.fbl.list.lb index end] } {
      set FBResult [.fbl.list.lb get end] 
    } else {
      set FBResult [.fbl.list.lb get [expr $curr_sel + 1]] 
    }
  }

  # Faccio in modo che sia attivo anche il tasto di return nella lista
  bind .fbl.list.lb <Return> {
    [.fbl.b subwidget ok] invoke
  }

  foreach i [lsort [array names FunctionBlockList]] {
    .fbl.list.lb insert end $i
  }

  focus .fbl.function.e

  bind .fbl <Key-Escape> { destroy .fbl }

  set FBResult [.fbl.list.lb get 0]

  grab .fbl
  tkwait window .fbl

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
}

####################################################################
# Procedura CreazioneParametri
# Procedura che associa ai parametri di ingresso uscita di 
# un blocco funzionale le variabili
# Input: 
#   NomeFunctionBlock  Nome del blocco funzionale
#   VecchiParametri    Lista dei parametri ricercati sul testo 
#                      in caso di richiamo di vecchio blocco
# Ritorna :
#   Niente
####################################################################
proc CreazioneParametri { NomeFunctionBlock VecchiParametri} {
global VariableList FunctionBlockList PredefinedFunctionBlock FB EditPrefs
global CommentoVarFb FuncBlocSel MsdData tcl_platform

 if { $NomeFunctionBlock != ""} {
   set FB $FunctionBlockList($NomeFunctionBlock)
 } else {
   set FB $FuncBlocSel
 }

 if { $FB !={} && [info exist PredefinedFunctionBlock($FB)]} {
   if {[llength $PredefinedFunctionBlock($FB)] == 1 } {
     AlertBox -text "$FB [GetString MsgNoIOParameters]"
     return 
   }
   if { $FB == "SEND_BYTES" || $FB == "RECV_BYTES" } {
     AlertBox -text "$FB [GetString MsgNoAutomaticParameters]"
     return 
   }
   if [winfo exists .fbs.param] {
     destroy .fbs.param
   }
   tixScrolledWindow .fbs.param -height 300
   set ParFrame [.fbs.param subwidget window]

   # Definizione dei widget per il passaggio dei parametri
   frame $ParFrame.labels
   frame $ParFrame.entries
 
   set tipovar "input"
   for {set i 0} {$i< [llength $PredefinedFunctionBlock($FB)] } {incr i } {
     if { [lindex $PredefinedFunctionBlock($FB) $i] != "###"} {
       if { $tipovar == "input" } {
          set colore "green4"
       } else {
          set colore "red"
       }
       label $ParFrame.labels.lab$i -text [lindex $PredefinedFunctionBlock($FB) $i] \
         -font $EditPrefs(windowfont) -foreground $colore 
       entry $ParFrame.entries.ent$i -font $EditPrefs(windowfont)
       if { [lindex $VecchiParametri $i] != " " } {
         $ParFrame.entries.ent$i insert 0 [lindex $VecchiParametri $i]
       }

       set testo [GetString $FB:[lindex $PredefinedFunctionBlock($FB) $i]_]
       if { "$testo" != "$FB:[lindex $PredefinedFunctionBlock($FB) $i]_" } {
         CniBalloon $ParFrame.entries.ent$i $testo $EditPrefs(windowfont) 500
       } elseif { [info exist CommentoVarFb($FB,[lindex $PredefinedFunctionBlock($FB) $i])] } {
         CniBalloon $ParFrame.entries.ent$i \
          $CommentoVarFb($FB,[lindex $PredefinedFunctionBlock($FB) $i]) \
                      $EditPrefs(windowfont) 500
       }

       bind $ParFrame.entries.ent$i <Key-Return> [format {
	  set ParFrame %s
          set focus_wid [focus]
          set j [string range $focus_wid [string length "$ParFrame.entries.ent"] \
             [string length $focus_wid]]
          if { $j == [expr [llength $PredefinedFunctionBlock($FB)] - 2] && \
                     [lindex $PredefinedFunctionBlock($FB) end] == "###" } {
            # Caso dei soli input
            set j 0
          } elseif { $j < [expr [llength $PredefinedFunctionBlock($FB)] - 1] } {
            set j [expr $j + 1]
          } else {
            set j 0
          }
	  if {[llength $PredefinedFunctionBlock($FB)] > 2} {
            focus $ParFrame.entries.ent$j
	  }
       } $ParFrame]

       bind $ParFrame.entries.ent$i <Key-Return> "+\nbreak"

       bind $ParFrame.entries.ent$i <FocusIn> { %W selection range 0 end }

       bind $ParFrame.entries.ent$i <Any-KeyRelease> {
         if { "%K" != "Tab" && "%K" != "Return"} {
           # trasformo tutti i caratteri in MAIUSCOLO
           set appo [string toupper [%W get]]
           %W delete 0 end
           %W insert end $appo
         }
       }

       pack $ParFrame.labels.lab$i   -in $ParFrame.labels -padx 5 -pady 9 -side top 
       if { $tcl_platform(platform) == "unix" } { 
         pack $ParFrame.entries.ent$i  -in $ParFrame.entries -side top -padx 5 \
         -pady 6 -expand yes -fill x
       } else {
         pack $ParFrame.entries.ent$i  -in $ParFrame.entries -side top -padx 5 \
         -pady 7 -expand yes -fill x
       }
     } else {
        set tipovar "output"
        # il $i != 0 e' legato al fatto che il FB puo' avere solo output
	if {$i < [expr [llength $PredefinedFunctionBlock($FB)] -1] && $i != 0 } {
          # Faccio il salto di uno nelle callback perche' c'e' il buco 
	  # legato al simbolo ### usato per separare gli ingressi e le uscite dei function
	  # block
          bind $ParFrame.entries.ent[expr $i - 1] <Key-Return> [format {
            set ParFrame %s
            set focus_wid [focus]
            set j [string range $focus_wid [string length "$ParFrame.entries.ent"] \
                  [string length $focus_wid]]
            if { $j < [expr [llength $PredefinedFunctionBlock($FB)] - 1] } {
              set j [expr $j + 2]
            } else {
              set j 0
            }
            focus $ParFrame.entries.ent$j
         } $ParFrame]
         bind $ParFrame.entries.ent[expr $i - 1] <Key-Return> "+\nbreak"
       }
     }
   }

   pack $ParFrame.labels $ParFrame.entries  -side left
   pack .fbs.base .fbs.param -side left


   if { [lindex $PredefinedFunctionBlock($FB) 0] != "###" } {
     focus $ParFrame.entries.ent0
   } else {
     # Perche' ci sono solo output
     focus $ParFrame.entries.ent1
   }
 } else {
   AlertBox -text MsgNotFB
 }
}

####################################################################
# Procedura RicavaParametri
# Procedura che ricava i parametri di ingresso uscita di 
# un blocco funzionale gia' definito e le variabili associate 
# Input: 
#   t               Nome del widget di testo
#   nome_fb         Nome del blocco funzionale
# Ritorna :
#   Niente
####################################################################
proc RicavaParametri { t nome_fb} {
global VariableList FunctionBlockList PredefinedFunctionBlock EditPrefs

  set FB $FunctionBlockList($nome_fb)
  set INGR_USC 0
  set RigaIniziale -1
  set linea_var_number -1

  if { $FB !={} } {
    for {set i 0} {$i< [llength $PredefinedFunctionBlock($FB)] } {incr i } {
      if { [lindex $PredefinedFunctionBlock($FB) $i] != "###"} {
        set parameter_line \
           [$t search -exact $nome_fb.[lindex $PredefinedFunctionBlock($FB) $i] 1.0] 
        if { $parameter_line != {} } {
	  if { $INGR_USC == 0 } {
	    set linea_var_number [expr [lindex [split $parameter_line .] 0] - 1]
            if { $RigaIniziale == -1 } {
              set RigaIniziale $linea_var_number
            }
      	    set linea_var [$t get $linea_var_number.0 $linea_var_number.end]
            # Caso della etichetta 
            if { [string match *: [lindex $linea_var 0]] != 0} {
              set indice_variabile 2
            } else {
              set indice_variabile 1
            }
            # Caso della chiamata non standard
            if { [lindex $linea_var [expr $indice_variabile -1]] == "LD" } {
	      set text_parameter [lappend text_parameter \
                [lindex $linea_var $indice_variabile]]
            } else {
	      set text_parameter [lappend text_parameter ?]
            }
	  } else {
	    set linea_var_number [expr [lindex [split $parameter_line .] 0] + 1]
            if { $RigaIniziale == -1 } {
              set RigaIniziale $linea_var_number
            }
      	    set linea_var [$t get $linea_var_number.0 $linea_var_number.end]
            if { [string match *: [lindex $linea_var 0]] != 0} {
              set indice_variabile 2
            } else {
              set indice_variabile 1
            }
	    # Caso della chiamata non standard
            if { [lindex $linea_var [expr $indice_variabile -1]] == "ST" } {
	      set text_parameter [lappend text_parameter \
                [lindex $linea_var $indice_variabile]]
            } else {
              set text_parameter [lappend text_parameter ?]
            }
	  }
        } else {
          if { $INGR_USC == 0 } {
            set text_parameter [lappend text_parameter " "]
          } else {
	    set text_parameter [lappend text_parameter " "]
	  }
        }	
      } else {
        set INGR_USC 1
        set text_parameter [lappend text_parameter ###]
      }
    }

    set parameter_line [$t search -exact "CAL\t$nome_fb" 1.0] 
    if { $parameter_line != {} } {
      set tmp_line [expr [lindex [split $parameter_line .] 0]]
      if { $tmp_line > $linea_var_number } {
        set linea_var_number $tmp_line
      }
    }
    if { $linea_var_number != -1 && $RigaIniziale == -1 } {
      set RigaIniziale $linea_var_number
    }

    return "$RigaIniziale $linea_var_number $text_parameter"
  } else {
    AlertBox -text MsgNotFB
  }
}

####################################################################
# Funzione PredefinedFunctionBlockListPreset
# Con tale funzione si accoda all'elenco delle function block
# standard le eventuali function block scritte in una libreria
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc PredefinedFunctionBlockListPreset {} {
global PredefinedFunctionBlock Home EditPrefs lista_librerie
global tipo_var_fb CommentoVarFb

  if {[info exists PredefinedFunctionBlock]} {
    unset PredefinedFunctionBlock
  }
  if {[info exists tipo_var_fb]} {
    unset tipo_var_fb
  }
  if {[info exists CommentoVarFb]} {
    unset CommentoVarFb
  }

  # Inserimento dei Blocchi Funzionali di libreria
  set NonCiSonoFB 0
  if { [glob -nocomplain $Home/util/*.h] != "" } {
    set ElencoFile "$Home/util/illib.ilc \
      [lsort [glob -nocomplain $Home/util/*.h]]"
  } else {
    set ElencoFile $Home/util/illib.ilc
  }

  foreach i $ElencoFile {
    if [catch { set fId [open $i r] } result] {
      AlertBox -text $result
      continue
    }
    set TuttoIlFileIn [read -nonewline $fId]
    close $fId
    set TuttoIlFileIn [string toupper $TuttoIlFileIn]

    while { $NonCiSonoFB == 0 } {
      set ind_FB [string first "FUNCTION_BLOCK" $TuttoIlFileIn]
      set ind_end_FB [string first "END_FUNCTION_BLOCK" $TuttoIlFileIn]
      if { $ind_FB != -1 && $ind_end_FB != -1 && $ind_FB < $ind_end_FB } {
        set TuttaLaFunzioneIn [string range $TuttoIlFileIn $ind_FB $ind_end_FB]
        set TuttaLaFunzioneOut $TuttaLaFunzioneIn
        set TuttoIlFileIn [string range $TuttoIlFileIn $ind_end_FB end]
        set TuttoIlFileIn [string range $TuttoIlFileIn \
            [expr [string first "\n" $TuttoIlFileIn] +1] end]
        set NomeFb [lindex [string range $TuttaLaFunzioneIn \
            0 [string first "\n" $TuttaLaFunzioneIn]] 1]
        if { [file extension $i] == ".h" } {
          if { [string index $NomeFb 0] == "_" } {
            continue
          }
          set lista_librerie($NomeFb) $i 
        }
      } else {
        set NonCiSonoFB 1
        continue
      }
      if [info exist PredefinedFunctionBlock($NomeFb)] {
        unset PredefinedFunctionBlock($NomeFb)
      }
      if [info exist tipo_var_fb($NomeFb)] {
        unset tipo_var_fb($NomeFb)
      }
      set ind_in_var [string first "VAR_INPUT" $TuttaLaFunzioneIn] 
      if { $ind_in_var != -1 } { 
        set TuttaLaFunzioneIn [string range $TuttaLaFunzioneIn $ind_in_var end]
        # tiro fuori VAR_INPUT
        set TuttaLaFunzioneIn [string range $TuttaLaFunzioneIn \
             [expr [string first "\n" $TuttaLaFunzioneIn] +1] end]
        set linea [string range $TuttaLaFunzioneIn 0 \
             [string first "\n" $TuttaLaFunzioneIn]]
        set TuttaLaFunzioneIn [string range $TuttaLaFunzioneIn \
             [string length $linea] end]
        while { [lindex $linea 0] != "END_VAR" } {
          set PredefinedFunctionBlock($NomeFb) [lappend \
            PredefinedFunctionBlock($NomeFb) [lindex $linea 0]]
          set primosplit [split $linea :]
          set tipo [lindex $primosplit 1]
          set tipo [string trimright $tipo ";\n"]
          set tipo [string trim $tipo ]
          set tipo_var_fb($NomeFb) [lappend tipo_var_fb($NomeFb) $tipo]
          set InizioCommento [string first "(*" $linea] 
          set FineCommento [string first "*)" $linea] 
          if { $InizioCommento != -1 && $FineCommento != -1 } {
            incr InizioCommento 2
            incr FineCommento -2
            set CommentoVarFb($NomeFb,[lindex $linea 0]) \
               [string range $linea $InizioCommento $FineCommento] 
          }
          set linea [string range $TuttaLaFunzioneIn 0 \
             [string first "\n" $TuttaLaFunzioneIn]]
          set TuttaLaFunzioneIn [string range $TuttaLaFunzioneIn \
             [string length $linea] end]
        }
      }
      set PredefinedFunctionBlock($NomeFb) [lappend \
          PredefinedFunctionBlock($NomeFb) "###"]
      set tipo_var_fb($NomeFb) [lappend \
          tipo_var_fb($NomeFb) "###"]
      set ind_out_var [string first "VAR_OUTPUT" $TuttaLaFunzioneOut]
      if { $ind_out_var != -1 } { 
        set TuttaLaFunzioneOut [string range $TuttaLaFunzioneOut \
             $ind_out_var end]
        # tiro fuori VAR_OUTPUT
        set TuttaLaFunzioneOut [string range $TuttaLaFunzioneOut \
                     [expr [string first "\n" $TuttaLaFunzioneOut] +1] end]
        set linea [string range $TuttaLaFunzioneOut 0 \
           [string first "\n" $TuttaLaFunzioneOut]]
        set TuttaLaFunzioneOut [string range $TuttaLaFunzioneOut \
           [string length $linea] end]
        while { [lindex $linea 0] != "END_VAR" } {
          set PredefinedFunctionBlock($NomeFb) [lappend \
            PredefinedFunctionBlock($NomeFb) [lindex $linea 0]]
          set primosplit [split $linea :]
          set tipo [lindex $primosplit 1]
          set tipo [string trimright $tipo ";\n"]
          set tipo [string trim $tipo ]
          set tipo_var_fb($NomeFb) [lappend tipo_var_fb($NomeFb) $tipo]
          set InizioCommento [string first "(*" $linea] 
          set FineCommento [string first "*)" $linea] 
          if { $InizioCommento != -1 && $FineCommento != -1 } {
            incr InizioCommento 2
            incr FineCommento -2
            set CommentoVarFb($NomeFb,[lindex $linea 0]) \
               [string range $linea $InizioCommento $FineCommento] 
          }
          set linea [string range $TuttaLaFunzioneOut 0 \
               [string first "\n" $TuttaLaFunzioneOut]]
          set TuttaLaFunzioneOut [string range $TuttaLaFunzioneOut \
               [string length $linea] end]
        }
      }
    }
    unset TuttoIlFileIn
    set NonCiSonoFB 0
  }
}

####################################################################
# Procedura OkFunctionBlockCall
# Procedura chiamata sulla pressione del bottone OK
# Input: 
#   w               Nome del widget di testo
#   nome_fb         Nome del blocco funzionale
#   type_fb         Tipo del blocco funzionale
# Ritorna :
#   Niente
####################################################################
proc OkFunctionBlockCall {w nome_fb type_fb} {
global VariableList FunctionBlockList PredefinedFunctionBlock
global EditPrefs RisultatoParametri MsdData
global input input2

  if { "$type_fb" == "OldFB" && $RisultatoParametri != "" && $EditPrefs(awl) == 1 } {
    $w delete [lindex $RisultatoParametri 0].0 [lindex $RisultatoParametri 1].end
    $w mark set insert [lindex $RisultatoParametri 0].0
  }
  set FB [.fbs.list.lb get [.fbs.list.lb index active]]
  if {[lsearch -exact [array names PredefinedFunctionBlock] $FB]==-1} {
    return
  }
  set FunctionBlockList($nome_fb)  $FB
  if [winfo exists .fbs.param]  {
    set ParFrame [.fbs.param subwidget window]
    set FB  [.fbs.list.lb get [.fbs.list.lb index active]]
    for {set i 0} {$i< [llength $PredefinedFunctionBlock($FB)] } {incr i } {
      if { [lindex $PredefinedFunctionBlock($FB) $i] != "###"} {
        set variable_name [string toupper [$ParFrame.entries.ent$i get]]
	# Verifico se si tratta di array ed elimino le quadre []
        set var_array [string first "\[" $variable_name]
        if {$var_array  != -1 } {
          set var_range [string range $variable_name  $var_array end]
          set variable_name [string range $variable_name  0 [expr $var_array -1]]
        } else {
          set var_range -1
        }
        # faccio saltar fuori il pannello di definizione variabili solo se la 
        # seconda parola non e' un numero
	# oppure una definizione di un tempo o di una data
        if {[string match {[^a-zA-Z]*} $variable_name] && \
            !( [string match {T\#*} $variable_name] || \
            [string match {TIME\#*} $variable_name] || \
            [string match {D\#*} $variable_name] || \
            [string match {DATE\#*} $variable_name] || \
            [string first "." $variable_name] != -1)} {
 	  set indice_variabile [lsearch -exact [read $input] $variable_name]
          seek $input 0 start
          if { $input2 != -1 } {
            set indice_variabile_dinamica [lsearch -exact [read $input2] $variable_name]
            seek $input2 0 start
          } else {
            set indice_variabile_dinamica -1
          }
          if { [lsearch -exact [array names VariableList] $variable_name] == -1 && \
               $indice_variabile == -1 && $indice_variabile_dinamica == -1} {
            if {[string length $variable_name] > $MsdData(lunvar)} {
              AlertBox -text "$variable_name\n [GetString MsgMaxVarLen]"
            } else {
              DefineVariablePanel $w $variable_name 1
            }
          }
        }
      }
    }
    $w mark set initline "insert linestart"
    $w mark set endline "insert lineend"
    if { [$w compare initline == 1.0] && [$w compare initline == endline]} {
      $w insert initline "PROGRAM NONAME\n"
    }
    if { [$w compare initline != insert] } {
      TextReplace $w initline endline "[$w get initline endline]\n" 0
    }
    set FB [.fbs.list.lb get [.fbs.list.lb index active]]
    set IO 0
    set DefinitiTuttiInput 0
    for {set i 0} {$i< [llength $PredefinedFunctionBlock($FB)] } {incr i } {
      # i simboli ### sono una convenzione mia per separare gli ingressi
      # e le uscite
      if { [string toupper [lindex $PredefinedFunctionBlock($FB) $i]] != "###" } {
        if { $IO == 0 } {
 	  set variable_name [string toupper [$ParFrame.entries.ent$i get]]
	  if {$variable_name != {} } {
            if {[string match {T\#*} $variable_name] || \
                [string match {TIME\#]*} $variable_name] || \
                [string match {D\#*} $variable_name] || \
                [string match {DATE\#*} $variable_name]} {
              $w insert insert "\tLD\t[$ParFrame.entries.ent$i get]\n"
	    } else {
              $w insert insert "\tLD\t[string toupper [$ParFrame.entries.ent$i get]]\n"
            }
            $w insert insert \
            "\tST\t$nome_fb.[string toupper [lindex $PredefinedFunctionBlock($FB) $i]]\n"
   	  } else {
            set DefinitiTuttiInput 1
          }
	} elseif { $IO == 1 } {
   	  if {[string toupper [$ParFrame.entries.ent$i get]] != {} } { 
            $w insert insert \
            "\tLD\t$nome_fb.[string toupper [lindex $PredefinedFunctionBlock($FB) $i]]\n"
            $w insert insert "\tST\t[string toupper [$ParFrame.entries.ent$i get]]\n"
       	  }
	}
      } else {
        set IO 1
        $w insert insert \
          "\tCAL\t$nome_fb\t(* FUNCTION BLOCK $FunctionBlockList($nome_fb) *)\n"
      }
    }
    # Cancello l'ultimo CR
    $w delete "insert -1 char" insert
    if { $DefinitiTuttiInput == 1 } {
      AlertBox -text MsgNoAllInput
    }
    SetModiFile $w 1
  } elseif {[llength $PredefinedFunctionBlock($FB)] } {
    # Caso di blocco funzionale senza ingressi e senza
    # uscite
    if { $FB != "SEND_BYTES" && $FB != "RECV_BYTES" && \
         [llength $PredefinedFunctionBlock($FB)] == 1} {
      $w mark set initline "insert linestart"
      $w mark set endline "insert lineend"
      if { [$w compare initline == 1.0] && [$w compare initline == endline]} {
        $w insert initline "PROGRAM NONAME\n"
      }
      if { [$w compare initline != insert] } {
        TextReplace $w initline endline "[$w get initline endline]\n" 0
      }
      $w insert insert \
          "\tCAL\t$nome_fb\t(* FUNCTION BLOCK $FunctionBlockList($nome_fb) *)"
      SetModiFile $w 1
    }
  }
  if { $EditPrefs(awl) == 1 } {
    # Caso IL
    DisplayCurrentLine $w
  }
  destroy .fbs
}

####################################################################
# Procedura FunctionBlockParameter
# Procedura chiamata sulla pressione del bottone PARAMETRI
# Input: 
#   Main            Nome del widget Main
#   Fbs             Nome della toplevel della lista 
#   type_fb         Tipo del blocco funzionale
# Ritorna :
#   Niente
####################################################################
proc FunctionBlockParameter  {Main Fbs} {

  CreazioneParametri "" "" 
  PosizionaFinestre $Main $Fbs c
}

####################################################################
# Procedura CancelFunctionBlockCall
# Procedura chiamata sulla pressione del bottone PARAMETRI
# Input: 
#   Fbs             Nome della toplevel della lista 
#   type_fb         Tipo del blocco funzionale
# Ritorna :
#   Niente
####################################################################
proc CancelFunctionBlockCall {Fbs type_fb } {

  if { "$type_fb" == "NewFB" } {
    set i [ lsearch -exact [.fbl.list.lb get 0 end] [.fbl.function.e get] ]
    $Fbs.list.lb delete $i $i
  }
  destroy $Fbs
}

####################################################################
# Procedura SelectionFunctionBlock
# Procedura che crea il box per l'introduzione dei parametri 
# associati ad un blocco funzionale
# Input: 
#   w               Nome del widget di testo
#   nome_fb         Nome del blocco funzionale
#   type_fb         Tipo del blocco funzionale
# Ritorna :
#   Niente
####################################################################
proc SelectionFunctionBlock { w nome_fb type_fb args } {
global VariableList FunctionBlockList PredefinedFunctionBlock
global EditPrefs RisultatoParametri FuncBlocSel

  set fileprompt [GetString "FblFbPrompt"]
  set title [GetString "TitleFbList"]
    
  if { $EditPrefs(awl) == 1 } { 
    set RisultatoParametri ""
  }

  if [winfo exists .fbs] {
    return
  }

  set old_focus [focus]
  set MainW [winfo toplevel [focus]]

  grab release [grab current]

  toplevel .fbs 
  frame .fbs.base
  wm title .fbs $title
  wm minsize .fbs 10 10
  wm protocol .fbs WM_DELETE_WINDOW procx

  # Definizione della lista function block
  frame .fbs.list
  listbox .fbs.list.lb -yscroll ".fbs.list.sb set" -font $EditPrefs(windowfont) \
    -width 10 -height 15
  scrollbar .fbs.list.sb -command ".fbs.list.lb yview"

  # Definizione dell' entry widget
  frame .fbs.function
  label .fbs.function.l -anchor w -font $EditPrefs(windowfont) -width 45

  tixButtonBox .fbs.b -orientation horizontal
  .fbs.b add ok -image ok -font $EditPrefs(windowfont) -command [format {
      OkFunctionBlockCall %s %s %s} $w $nome_fb $type_fb]
  .fbs.b add delete -image parametri -font $EditPrefs(windowfont) -command [format {
      FunctionBlockParameter %s %s} $MainW .fbs]
  .fbs.b add cancel -image cancel -font $EditPrefs(windowfont) -command [format {
      CancelFunctionBlockCall %s %s} .fbs $type_fb]

  CniBalloon [.fbs.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.fbs.b subwidget delete] [GetString ParametersList] \
    $EditPrefs(windowfont) 500
  CniBalloon [.fbs.b subwidget cancel] [GetString CANCEL] $EditPrefs(windowfont) 500

  pack .fbs.list.sb -side $EditPrefs(scrollbarside) -fill y
  pack .fbs.list.lb -side left -expand yes -fill both
  
  pack .fbs.function.l -side left -pady 10 -padx 10
  pack [FillerFrame .fbs.function] -side left
  
  pack .fbs.list -in .fbs.base -side top -expand yes -fill both
  pack .fbs.function -in .fbs.base -side top -expand yes -fill x
  pack .fbs.b  -in .fbs.base -side top -fill x
  pack .fbs.base -side top -fill both

  wm transient .fbs $MainW
  PosizionaFinestre $MainW .fbs c

  focus .fbs.list.lb

  bind .fbs.list.lb <ButtonRelease-1> {
    %W selection clear 0 end 
    %W selection set [%W nearest %y]
    set FuncBlocSel [lindex [selection get] 0]
    set testo1 $FuncBlocSel:$FuncBlocSel
    append testo1 _
    set testo [GetString $testo1]
    if { "$testo" != $testo1 } {
      .fbs.function.l config -text $testo
    } else {
      .fbs.function.l config -text ""
    }
    # Se esistono i parametri dei FB aggiorno anche quelli
    if [winfo exists .fbs.param] {
      CreazioneParametri "" "" 
    }
    focus %W
  }

  # stessa azione del bottone OK
  bind .fbs.list.lb <Double-ButtonRelease-1> [format {
      FunctionBlockParameter %s %s} $MainW .fbs]

  #Faccio in  modo che si possa selezionare il function block anche con il tasto Return
  bind .fbs.list.lb <Return> {
    CreazioneParametri "" "" 
  } 

  # Aggiorno il function block correntemente selezionato
  bind .fbs.list.lb <Key-Up> {
    set curr_sel [.fbs.list.lb index active]
    if { [expr $curr_sel - 1] < 0 } {
      set FuncBlocSel [.fbs.list.lb get 0] 
    } else {
      set FuncBlocSel [.fbs.list.lb get [expr $curr_sel - 1]] 
    }
    set testo1 $FuncBlocSel:$FuncBlocSel
    append testo1 _
    set testo [GetString $testo1]
    if { "$testo" != $testo1 } {
      .fbs.function.l config -text $testo
    } else {
      .fbs.function.l config -text ""
    }
  }

  bind .fbs.list.lb <Key-Down> {
    set curr_sel [.fbs.list.lb index active]
    if { [expr $curr_sel + 1] > [.fbs.list.lb index end] } {
      set FuncBlocSel [.fbs.list.lb get end] 
    } else {
      set FuncBlocSel [.fbs.list.lb get [expr $curr_sel + 1]] 
    }
    set testo1 $FuncBlocSel:$FuncBlocSel
    append testo1 _
    set testo [GetString $testo1]
    if { "$testo" != $testo1 } {
      .fbs.function.l config -text $testo
    } else { 
      .fbs.function.l config -text ""
    }
  }

  bind .fbs.list.lb <Double-ButtonRelease-1> "+\nbreak\n"
  
   foreach i [lsort [array names PredefinedFunctionBlock]]  {
     .fbs.list.lb insert end $i
   }

  bind .fbs <Key-Escape> {destroy .fbs}

  if { $type_fb == "OldFB"} {
    set chiamata_fb [$w search -regexp "CAL\( |\t)+$nome_fb" 1.0] 
    if { $chiamata_fb != {} } {
      set RisultatoParametri [RicavaParametri $w $nome_fb]
      CreazioneParametri $nome_fb [lrange $RisultatoParametri 2 end]
      if { [lsearch -exact $RisultatoParametri "?"] != -1 } {
        AlertBox -text MsgNoFBOk
        destroy .fbs
        return
      }
    } elseif { $EditPrefs(awl) == 0 } { 
      # caso ladder
      CreazioneParametri $nome_fb [lrange $RisultatoParametri 2 end]
      if { [lsearch -exact $RisultatoParametri "?"] != -1 } {
        AlertBox -text MsgNoFBOk
        destroy .fbs
        return
      }
    } else {
      CreazioneParametri $nome_fb ""
    }
    set testo1 $FunctionBlockList($nome_fb):$FunctionBlockList($nome_fb)
    append testo1 _
    set testo [GetString $testo1]
    if { "$testo" != "$testo1" } {
      .fbs.function.l config -text $testo
    } else {
      .fbs.function.l config -text ""
    }
    .fbs.list.lb activate [lsearch -exact [.fbs.list.lb get 0 end] \
       $FunctionBlockList($nome_fb)]
  } else {
     set FB [.fbs.list.lb get active]
     set FuncBlocSel $FB
     set testo1 $FB:$FB
     append testo1 _
     set testo [GetString $testo1]
     if { "$testo" != $testo1 } {
       .fbs.function.l config -text $testo
     } else {
       .fbs.function.l config -text ""
     }
  }

  grab .fbs
  tkwait window .fbs 

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
}

####################################################################
# Procedura OkCustomFB
# Procedura di conferma per creazione blocco funzionale
# utente
# Input: 
#   TextWidget         Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc OkCustomFB { TextWidget } {
global EditFiles MainOfFunctionBlock EntryFunctionBlock 

  if {$EntryFunctionBlock == "" } {
    return
  }
  set Top [TextToTop $TextWidget]
  set FBPresente [lsearch -regexp [array get EditFiles] $EntryFunctionBlock]
  if { $FBPresente != -1 } {
    if { $MainOfFunctionBlock([lindex [array get EditFiles] [expr $FBPresente -1]]) != \
         $Top} {
      # Se il nome e' lo stesso ma il padre e' diverso devo aprire comunque
      # un'altra finestra
      set FBPresente -1
    }
  }
  if { $FBPresente == -1 } {
    if {([string length $EntryFunctionBlock] > 0) && \
        ([string first " " $EntryFunctionBlock] == -1) } {
      if {[lsearch -exact [.fbl.list.lb get 0 end] $EntryFunctionBlock]!=-1} {
        set w [CreateWindow -tipo_win fb]
        set MainOfFunctionBlock($w) $Top 
        destroy .fbl
        set t [TopToText $w]
        SwapWindowsVariable $w
        set RootPath [file dirname [GetFilename $TextWidget]]
        LoadProgram "$RootPath/[CompletaNomeBlocco $EntryFunctionBlock $Top]" $t 1
        InizializzaFB $t 1
      } else {
        if [ConfirmBox -text [GetString ConfirmNewfb]] then {
          set w [CreateWindow -tipo_win fb]
          # toplevel main che genera questo FB
          set MainOfFunctionBlock($w) $Top 
          set t [TopToText $w]
          SwapWindowsVariable $w
          set RootPath [file dirname [GetFilename $TextWidget]]
          SetFilename $t "$RootPath/[CompletaNomeBlocco $EntryFunctionBlock $Top]"
          InizializzaFB [TopToText $w] 0
          destroy .fbl
        }
      }
    }
  } else {
    destroy .fbl
    raise [lindex [array get EditFiles] [expr $FBPresente -1]]
  }
}

####################################################################
# Procedura DeleteCustomFB
# Procedura di cancellazione di un blocco funzionale utente
# Input: 
#   TextWidget                Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc DeleteCustomFB { TextWidget } {
global EditFiles MainOfFunctionBlock EntryFunctionBlock 
global ProgramComponent

  if {([string length $EntryFunctionBlock] > 0) && \
      ([string first " " $EntryFunctionBlock] == -1) } {
    if ![ConfirmBox -text [GetString ConfirmCancelfb]] {
      return
    }
    if {[lsearch -exact [.fbl.list.lb get 0 end] $EntryFunctionBlock]!=-1} {
      foreach index [lsort -integer -increasing [array names ProgramComponent]] { 
        if { $index != 0 } {
          # non inserisco nella lista il main (indice 0)
          set NomeBlocco [RicavaNomeBlocco [lindex $ProgramComponent($index) 0]]
          if { $NomeBlocco == $EntryFunctionBlock } {
            set RootPath [file dirname [GetFilename $TextWidget]]
            file delete $RootPath/[lindex $ProgramComponent($index) 0]
	    unset ProgramComponent($index)
            .fbl.list.lb delete [lsearch -exact [.fbl.list.lb get 0 end] \
               $EntryFunctionBlock]
            set EntryFunctionBlock ""
            SetModiFile $TextWidget 0
          }
        }
      }
    } else {
      AlertBox -text MsgNOFb
    }
  }
}

####################################################################
# Procedura CancelCustomFB
# Procedura di uscita dal pannello di definizione di un blocco 
# funzionale utente
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc CancelCustomFB { } {
global EntryFunctionBlock

  set EntryFunctionBlock ""
  destroy .fbl
}

####################################################################
# Procedura LoadFB
# Procedura che definisce la toplevel con l'elenco di tutti
# i function block utente creati
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc LoadFB { w } {
global MsdData EditFiles MainOfFunctionBlock
global EntryFunctionBlock EditPrefs  ProgramComponent 

  if [winfo exists .fbl] {
    return
  }

  set fbprompt [GetString "FblFbPrompt"]
  set title [GetString "TitlePrototipiFbList"]
    
  set EntryFunctionBlock ""
  
  set old_focus [focus]

  set MainW [winfo toplevel [focus]]

  grab release [grab current]

  toplevel .fbl
  wm title .fbl $title
  wm minsize .fbl 10 10
  wm protocol .fbl WM_DELETE_WINDOW procx

  frame .fbl.list
  set nome_file [file root [file tail [GetFilename $w]]]
  label .fbl.l -text "[GetString FblTrovatoBlocco] $nome_file" \
    -font $EditPrefs(textfont) -relief raised
  listbox .fbl.list.lb -yscroll ".fbl.list.sb set" -font $EditPrefs(windowfont) \
    -width 20 -height 20
  scrollbar .fbl.list.sb -command ".fbl.list.lb yview"
  frame .fbl.function
  label .fbl.function.l -text $fbprompt -font $EditPrefs(windowfont) -anchor e
  entry .fbl.function.e -text $EntryFunctionBlock -font $EditPrefs(windowfont) \
    -textvariable EntryFunctionBlock -width 15
  
  tixButtonBox .fbl.b -orientation horizontal

  .fbl.b add ok  -image ok -font $EditPrefs(windowfont) \
     -command [format { OkCustomFB %s } $w]

  .fbl.b add delete  -image delete -font $EditPrefs(windowfont) \
     -command [format { DeleteCustomFB %s } $w]

  .fbl.b add quit -image cancel -font $EditPrefs(windowfont) \
     -command { CancelCustomFB } 

  CniBalloon [.fbl.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.fbl.b subwidget delete] [GetString DELETE] \
    $EditPrefs(windowfont) 500
  CniBalloon [.fbl.b subwidget quit] [GetString EXIT] \
    $EditPrefs(windowfont) 500

  pack .fbl.l -side top -fill both -pady 3
  pack .fbl.list.sb -side $EditPrefs(scrollbarside) -fill y
  pack .fbl.list.lb -side left -expand yes -fill both
  
  pack .fbl.function.l -side left -pady 10 -padx 10
  pack .fbl.function.e -side left -expand yes -pady 10 -padx 10 -fill x
  
  pack .fbl.list -side top -expand yes -fill both
  pack .fbl.function -side top -expand yes -fill x
  pack .fbl.b  -side bottom -fill x

  wm transient .fbl $MainW
  PosizionaFinestre $MainW .fbl c

  focus .fbl.list.lb

  bind .fbl.function.e <Key-Return> {
    [.fbl.b subwidget ok] invoke
  }

  bind .fbl.function.e <Key-Return> "+\nbreak"

  # faccio in modo che la scrittura della variabile nel campo di editing sia 
  # maiuscolo
  bind .fbl.function.e <Any-KeyRelease> {
    set EntryFunctionBlock [string toupper $EntryFunctionBlock]
    if {[string length $EntryFunctionBlock] > $MsdData(lunvar)} {
       .fbl.function.e delete $MsdData(lunvar) end
    }
  }

  bind .fbl.list.lb <Button-1> {
    %W selection clear 0 end
    %W selection set [%W nearest %y]
    if { [.fbl.list.lb curselection] != {} } {
      set EntryFunctionBlock [lindex [selection get] 0]
    }
    focus %W
  }

  bind .fbl.list.lb <Double-Button-1> {
    [.fbl.b subwidget ok] invoke
  }

  bind .fbl.list.lb <Double-Button-1> "+\nbreak\n"
  
  # Aggiorno la variabile correntemente selezionata
  bind .fbl.list.lb <Key-Up> {
    set curr_sel [.fbl.list.lb index active]
    if { [expr $curr_sel - 1] < 0 } {
      set EntryFunctionBlock [.fbl.list.lb get 0] 
    } else {
      set EntryFunctionBlock [.fbl.list.lb get [expr $curr_sel - 1]] 
    }
  }

  bind .fbl.list.lb <Key-Down> {
    set curr_sel [.fbl.list.lb index active]
    if { [expr $curr_sel + 1] > [.fbl.list.lb index end] } {
      set EntryFunctionBlock [.fbl.list.lb get end] 
    } else {
      set EntryFunctionBlock [.fbl.list.lb get [expr $curr_sel + 1]] 
    }
  }

  bind .fbl.list.lb <Return> {
    [.fbl.b subwidget ok] invoke
  }

  foreach i [lsort -integer -increasing [array names ProgramComponent]] { 
    if { $i != 0 } {
       # non inserisco nella lista il main (indice 0)
       .fbl.list.lb insert end [RicavaNomeBlocco [lindex $ProgramComponent($i) 0]] 
    }
  }

  focus .fbl.function.e

  bind .fbl <Key-Escape> {destroy .fbl}
 
  set EntryFunctionBlock [.fbl.list.lb get 0]

  grab .fbl
  tkwait window .fbl
}

####################################################################
# Procedura TestTipoFinestra
# Procedura che identifica la finestra come contenente un 
# blocco funzionale oppure un main
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#         fb se e' una toplevel dedicata all'edit dei FB
#         main se e' una toplevel dedicata all'edit del main
####################################################################
proc TestTipoFinestra { w } {
  global WindowType

  set window [TextToTop $w]
  return $WindowType($window)
}

####################################################################
# Procedura InizializzaFB
# Procedura che inizializza VariableList prelevando le variabili globali
# del main e inserisce la prima riga FUNCTION_BLOCK ....
# Input: 
#   t          Nome del widget di testo
#   type       type = 0   FB new ; type = 1   FB old
# Ritorna :
#   Niente
####################################################################
proc InizializzaFB { t type} {
global EditFiles VariableList MainOfFunctionBlock LoadErrorFB

  if { $type==0 } {
    # FB nuovo
    # reset delle variabili precedentemete definite
    if [info exists VariableList] {
      unset VariableList
    }
    $t mark set initline "insert linestart"
    $t mark set endline "insert lineend"

    set filename [string range $EditFiles([TextToTop $t]) 0 \
       [expr [string length $EditFiles([TextToTop $t])] -2]]
    TextReplace $t initline endline \
      "FUNCTION_BLOCK [file tail $filename]\n" 0
  }
  # inserisco in VariableList le globali del main
  set w $MainOfFunctionBlock([TextToTop $t])

  global VariableList$w

  # sono costretto ad usare un array appoggio perche' i nomi di array con il
  # punto '.' hanno dei problemi in TclTk
  array set elenco_appoggio [array get VariableList$w]

  set LoadErrorFB {}
  foreach x [array names elenco_appoggio] {
    if { [lindex $elenco_appoggio($x) 2] == "global" } {
      if { [lsearch -exact [array names VariableList] $x] != -1 } {
        set LoadErrorFB $x
        AlertBox -text "[GetString MsgDoubleDef] $LoadErrorFB"
        break
      } else {
        set VariableList($x) $elenco_appoggio($x)
      }
    }
  }
}

####################################################################
# Procedura RicavaNomeBlocco
# Procedura che toglie il numero della finestra principale al nome
# del blocco funzionale
# Input: 
#   NomeBlocco  Nome del blocco
# Ritorna :
#   Nuovo nome del blocco
####################################################################
proc RicavaNomeBlocco { NomeBlocco} {

  set NewNomeBlocco [string range $NomeBlocco 0 \
        [expr [string length $NomeBlocco] -2]]
  return $NewNomeBlocco
}

####################################################################
# Procedura CompletaNomeBlocco
# Procedura che aggiunge il numero della finestra principale al nome
# del blocco funzionale
# Input: 
#   NomeBlocco  Nome del blocco
# Ritorna :
#   Nuovo nome del blocco
####################################################################
proc CompletaNomeBlocco { NomeBlocco Top} {

  set NumTop [string index $Top [expr [string length $Top] -1]]
  set NewNomeBlocco $NomeBlocco$NumTop
  return $NewNomeBlocco
}
