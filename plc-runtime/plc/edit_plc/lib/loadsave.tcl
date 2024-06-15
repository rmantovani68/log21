 
            ######################################
            #                                    #
            #     File di definizione delle      #
            #     funzioni di carica e           #
            #     salvataggio file               #
            #                                    #
            ######################################


####################################################################
# Procedura SpezzaFile
# Procedura che effettua lo spezzettamento del file in tanti file 
# che contengono il main ed i function box
# Input: 
#   filename   Nome del file 
#   t          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc SpezzaFile { filename t } {
global ProgramComponent EditPrefs

  # Apro il file da spezzare
  if [catch { set file_sorg [open $filename r] } result] {
    AlertBox -text $result
    return
  }
  # Inizializzo l'array globale in cui per ciascun elemento memorizzo:
  #     il nome del programma principale o dei blocchi funz
  #     la riga di inizio del p.p. o dei b.f.
  #     le righe nascoste dall'editor del p.p. o dei b.f. 
  set ProgramComponent(0) [list [file tail [file root $filename]] 1]
  # indice dell'array su descritto
  set index 1
  set linee_tmp 0

  set tmpfile [file root $filename]
  if [catch {set id [open $tmpfile w]} result] {
    AlertBox -text $result
    return
  }

  # Fino a che il programma non e' finito
  while { [gets $file_sorg line] >= 0 } {
    incr linee_tmp
    if { "[lindex $line 0]" == "FUNCTION_BLOCK" } {
      set RootPath [file dirname [GetFilename $t]]
      set NumTop [string index [winfo toplevel $t] \
                 [expr [string length [winfo toplevel $t]] -1]]
      set tmpfile "$RootPath/[lindex $line 1]$NumTop"
      set ProgramComponent($index) [list [lindex $line 1]$NumTop $linee_tmp]
      set index [expr $index + 1]
      close $id
      if [catch {set id [open $tmpfile w]} result ] {
        AlertBox -text $result
      } else {
        puts $id $line
      }
    } else {
        puts $id $line
    }
  }
  close $id
  close $file_sorg
}

####################################################################
# Procedura RicercaPrototipiFB
# Procedura che ricerca i prototipi di un eventuale function block
# Input: 
#   t          Nome del widget di testo
#   FlagOff    Flag per il calcolo dell'offset
# Ritorna :
#   Niente
####################################################################
proc RicercaPrototipiFB {t FlagOff} {
global ProgramComponent PredefinedFunctionBlock EditPrefs
global InstructionList MachineDataList 

  foreach x [lsort -integer -increasing [array names ProgramComponent]] {
    if { $x != 0 } {
      if [info exist PredefinedFunctionBlock([RicavaNomeBlocco \
         [lindex $ProgramComponent($x) 0]])] {
        unset PredefinedFunctionBlock([RicavaNomeBlocco [lindex $ProgramComponent($x) 0]])
      }
      set RootPath [file dirname [GetFilename $t]]
      if [catch { set file_sorg \
         [open "$RootPath/[lindex $ProgramComponent($x) 0]" r] } result] {
        AlertBox -text $result
        continue
      }
      set ind_in_var 0
      set ind_out_var 0

      set inizio_programma 0
      set linee_vuote 0
      set offset_linea_FB 0

      while { [gets $file_sorg linea] >=0 } {
        if { $inizio_programma == 1 } {
           break
        } else {
          # Verifico se la prima parola rientra nel set delle istruzioni 
          foreach instr [array names InstructionList] {
            set indice_lista_istruzioni [lsearch -exact $InstructionList($instr) \
              [string toupper [lindex $linea 0]]]
            if { $indice_lista_istruzioni != -1 } {
              set inizio_programma 1
              break
            }
          }

          foreach datimacc_op $MachineDataList {
            set indice_lista_datimacc [lsearch -exact $datimacc_op \
              [string toupper [lindex $linea 0]]]
            if { $indice_lista_datimacc != -1 } {
              set inizio_programma 1
              break
            }
          }

          if { $indice_lista_istruzioni != -1 || $indice_lista_datimacc != -1 } {
            continue
          }

          #  ---  Inizio ricerca offset ---
          if { $FlagOff } { 
            if {[lindex $linea 0] == "VAR_GLOBAL"} {
              set offset_linea_FB [expr $offset_linea_FB + 1 + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "VAR"} {
              set offset_linea_FB [expr $offset_linea_FB + 1 + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "VAR_INPUT"} {
              set offset_linea_FB [expr $offset_linea_FB + 1 + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "VAR_OUTPUT"} {
              set offset_linea_FB [expr $offset_linea_FB + 1 + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "VAR_EXTERNAL"} {
              set offset_linea_FB [expr $offset_linea_FB + 1 + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "FUNCTION_BLOCK"} {
              set offset_linea_FB [expr $offset_linea_FB + $linee_vuote]
	      set linee_vuote 0
            } elseif {[lindex $linea 0] == "END_FUNCTION_BLOCK"} {
	      set linee_vuote 0
	      close $file_sorg
	      return
            } elseif {[lindex $linea 0] == ""} {
	      set linee_vuote [expr $linee_vuote + 1 ]
            } elseif {[string match {(\**} [lindex $linea 0]] == 1} {
	      set linee_vuote 0
            } elseif {[lindex $linea 0] != "END_VAR" } {
              set offset_linea_FB [expr $offset_linea_FB +1]
            } else {
              set offset_linea_FB [expr $offset_linea_FB +1]
            }
          }
          # ---  Fine ricerca offset  ---
          # --- Ricerca dei prototipi ---
          if { "[lindex $linea 0]" == "VAR_INPUT" } {
            set ind_in_var 2
            continue
          } 
          if { "[lindex $linea 0]" == "VAR_OUTPUT" } {
            set ind_out_var 2
            set PredefinedFunctionBlock([RicavaNomeBlocco \
              [lindex $ProgramComponent($x) 0]]) \
              [lappend PredefinedFunctionBlock([RicavaNomeBlocco \
              [lindex $ProgramComponent($x) 0]]) "###"]
            continue
          } 

          if { "[lindex $linea 0]" == "END_VAR" && $ind_in_var == 2} {
            set ind_in_var 1
          }

          if { "[lindex $linea 0]" == "END_VAR" && $ind_in_var == 2} {
            set ind_in_var 1
          }

          if { $ind_in_var == 2 || $ind_out_var == 2} {
            set PredefinedFunctionBlock([RicavaNomeBlocco \
                [lindex $ProgramComponent($x) 0]]) [lappend \
                PredefinedFunctionBlock([RicavaNomeBlocco \
                [lindex $ProgramComponent($x) 0]]) [lindex $linea 0]]
          } 
          #  ---  Fine ricerca prototipo ---
        }
      }
      if { $ind_in_var == 0 || $ind_out_var == 0} {
        set PredefinedFunctionBlock([RicavaNomeBlocco \
          [lindex $ProgramComponent($x) 0]]) \
          [lappend PredefinedFunctionBlock([RicavaNomeBlocco \
          [lindex $ProgramComponent($x) 0]]) "###"]
      }
      close $file_sorg
      if { $FlagOff } {
        set ProgramComponent($x) [lappend ProgramComponent($x) $offset_linea_FB] 
      }
    }
  }
}

####################################################################
# Procedura CalcolaHeaderFB
# Procedura che ottiene il file header della libreria
# Input: 
#   t          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CalcolaHeaderFB {t} {
global ProgramComponent InstructionList MachineDataList Home EditPrefs 

  set RootPath [file dirname [GetFilename $t]]

  foreach x [lsort -integer -increasing [array names ProgramComponent]] {
    if { $x != 0 } {
      if [catch { set file_sorg \
         [open "$RootPath/[lindex $ProgramComponent($x) 0]" r] } result] {
        AlertBox -text $result
        return
      }

      set inizio_programma 0

      while { [gets $file_sorg linea] >= 0 } {
        # Verifico se la prima parola rientra nel set delle istruzioni 
        foreach instr [array names InstructionList] {
          set indice_lista_istruzioni [lsearch -exact $InstructionList($instr) \
            [string toupper [lindex $linea 0]]]
          if { $indice_lista_istruzioni != -1 } {
            puts $file_header "END_FUNCTION_BLOCK\n"
            set inizio_programma 1
            break
          }
        }

        foreach datimacc_op $MachineDataList {
          set indice_lista_datimacc [lsearch -exact $datimacc_op [string toupper \
            [lindex $linea 0]]]
          if { $indice_lista_datimacc != -1 } {
            puts $file_header "END_FUNCTION_BLOCK\n"
            set inizio_programma 1
            break
          }
        }
        
        if { $inizio_programma } {
          break
        } else {
          puts $file_header $linea
        }
      }
      close $file_sorg
    } else {
      if [file exists "$RootPath/[lindex $ProgramComponent($x) 0].h"] {
        file delete "$RootPath/[lindex $ProgramComponent($x) 0].h"
      }
      if [catch {set file_header [open "$RootPath/[lindex $ProgramComponent(0) 0].h" a]} \
              result] {
        AlertBox -text $result
        return
      }
    }
  }
  close $file_header
  file copy -force "$RootPath/[lindex $ProgramComponent(0) 0].h" \
                $Home/util/[lindex $ProgramComponent(0) 0].h
  file delete "$RootPath/[lindex $ProgramComponent(0) 0].h"
}

####################################################################
# Procedura CalcoloOffsetFB
# Procedura che normalizza l'offset per vis. l'errore in caso di FB
# Input: 
#   lin       linea fisica dell'errore nel file .plc
# Ritorna :
#   Niente
####################################################################
proc CalcoloOffsetFB {lin} {
global ProgramComponent 

  if ![array exists ProgramComponent] {
    return [list main 0] 
  }

  foreach x [lsort -integer -increasing [array names ProgramComponent]] {
    if { [lindex $ProgramComponent($x) 1] > $lin } {
      set x [expr $x - 1 ]
      if { $x == 0 } {
      	return [list main [expr [lindex $ProgramComponent($x) 1] - 1]]
      } else {
      	return [list "F. Block [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]]" \
          [expr [lindex $ProgramComponent($x) 1] + [lindex $ProgramComponent($x) 2] - 1]]
      }
    }
  }

  #l'errore e' sicuramente nell'ultimo FB
  return [list "F. Block [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]]" \
        [expr [lindex $ProgramComponent($x) 1] + [lindex $ProgramComponent($x) 2] - 1]]
}

####################################################################
# Procedura LoadFilePlc
# Procedura che carica il file principale
# Input: 
#   filename    Nome del file da caricare
#   t           Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc LoadFilePlc { filename t } {
global ProgramComponent ModiFile EditPrefs 
global VariableList FunctionBlockList

  # Se la finestra e' la principale cancello gli eventuali file temporanei 
  # di appoggio
  if { [TestTipoFinestra $t]=="main" } {
    if [info exist ProgramComponent] {
      foreach i [array names ProgramComponent] {
        if [file exists $EditPrefs(progpath)/[lindex $ProgramComponent($i) 0]] {
          file delete $EditPrefs(progpath)/[lindex $ProgramComponent($i) 0]
        }
        if [file exists $EditPrefs(progpath1)/[lindex $ProgramComponent($i) 0]] {
          file delete $EditPrefs(progpath1)/[lindex $ProgramComponent($i) 0]
        }
        if [file exists $EditPrefs(libpath)/[lindex $ProgramComponent($i) 0]] {
          file delete $EditPrefs(libpath)/[lindex $ProgramComponent($i) 0]
        }
      }
    }
  }

  if {[info exists ProgramComponent]} {
     unset ProgramComponent
  }

  # preset libreria di FB
  PredefinedFunctionBlockListPreset

  set ModiFile 0

  if [catch { set file_sorg [open $filename r] } result] {
    AlertBox -text $result
    return
  }

  # Se non ci sono dei function block 
  set ind_fb -1
  while { [gets $file_sorg linea] >= 0 } {
    if { "[lindex $linea 0]" == "FUNCTION_BLOCK" } {
      set ind_fb 1
      break
    }
  }
  close $file_sorg

  SetFilename $t $filename

  if { $ind_fb == -1 } {
    $t configure -background $EditPrefs(textbg)
    $t configure -state normal
    LoadProgram $filename $t 0
  } else {
    SpezzaFile $filename $t
    RicercaPrototipiFB $t 1
    if {[string first $EditPrefs(libpath) $filename] == -1 } {
      $t configure -background $EditPrefs(textbg)
      $t configure -state normal
      LoadProgram "[file dirname $filename]/[lindex $ProgramComponent(0) 0]" $t 0
    } else {
      # cancellazione del widget di testo 
      $t delete 1.0 end
      # reset delle variabili precedentemete definite
      if [info exists VariableList] {
        unset VariableList
      }
      #reset function block definiti
      if {[info exists FunctionBlockList]} {
        unset FunctionBlockList
      }
      $t configure -background lemonchiffon
      $t configure -state disabled
      LoadGlobalVariable "[file dirname $filename]/[lindex $ProgramComponent(0) 0]" 
    }
  }
}

####################################################################
# Procedura LoadGlobalVariable
# Procedura che carica le variabili globali della libreria
# Input: 
#   filename    Nome del file da caricare
# Ritorna :
#   Niente
####################################################################
proc LoadGlobalVariable { filename } {
global EditPrefs VariableList FunctionBlockList  InstructionList
global ProgramComponent MachineDataList 

  # inizializzazione variabili in caso di nessuna dichiarazione di variabili
  set  nvram ""
  set  tipo ""
  set  scope ""
  set  location ""
  set  inizializzazione ""
  set  descrizione ""
  set  dim1 ""
  set  dim2 ""

  # reset delle variabili precedentemete definite
  if [info exists VariableList] {
    unset VariableList
  }

  #reset function block definiti
   if {[info exists FunctionBlockList]} {
      unset FunctionBlockList
   }

  if [catch { set file_sorg [open $filename r] } result] {
    AlertBox -text $result
    return
  }

  while { [gets $file_sorg linea] >=0 } {
    if {[lindex $linea 0] == "VAR_GLOBAL"} {
      set scope global
      set location memory
      if {[lindex $linea 1] == "RETAIN"} {
        set nvram retain
      } else {
        set nvram noretain
      }
    } elseif {[lindex $linea 0] == "VAR"} {
      set scope local
      set location memory
      if {[lindex $linea 1] == "RETAIN"} {
        set nvram retain
      } else {
        set nvram noretain
      }
    } elseif {[lindex $linea 0] == "VAR_INPUT"} {
      set scope local
      set location input
      if {[lindex $linea 1] == "RETAIN"} {
        set nvram retain
      } else {
        set nvram noretain
      }
    } elseif {[lindex $linea 0] == "VAR_OUTPUT"} {
      set scope local
      set location output
      if {[lindex $linea 1] == "RETAIN"} {
        set nvram retain
      } else {
        set nvram noretain
      }
    } elseif {[lindex $linea 0] == "VAR_EXTERNAL"} {
      set scope extern
      set location memory
      if {[lindex $linea 1] == "RETAIN"} {
        set nvram retain
      } else {
        set nvram noretain
      }
    } elseif {[lindex $linea 0] != "END_VAR" } {
      if { $linea == "" } {
        continue
      }
      set primosplit [split $linea :]
      set nome [string trim [lindex $primosplit 0]]

      ##########
      # Controllo se ci sono delle definizioni di function  block

      if { $scope == "local" && $location == "memory" && \
           $nvram == "noretain" } {
        set tipo [lindex $primosplit 1]
        set tipo [string trimright $tipo ";"]
        set tipo [string trim $tipo ]
        if { $tipo != "BOOL" && $tipo != "DINT" && $tipo != "UDINT"  && \
               $tipo != "REAL"  && $tipo != "LREAL" && $tipo != "TIME" && \
               $tipo != "DATE" && $tipo != "STRING" && $tipo != "BYTE" && \
               $tipo != "WORD" && $tipo != "DWORD" && $tipo != "INT"  && \
               $tipo != "UINT" && [string first "ARRAY" $tipo] == -1} {
	  set FunctionBlockList($nome) $tipo
          continue
	}
      }
      #######

      set arrai 0
      if { [string first ARRAY [lindex $primosplit 1]] != -1 } {
        set arrai 1
        set tipo [lindex [lindex $primosplit 1] 2]
        set tipo [string trimright $tipo ";"]
        set secondosplit [split [lindex [lindex $primosplit 1] 0] ,]
        if {[llength $secondosplit] == 1} {
          set dim1 [string trimleft [lindex $secondosplit 0] "ARRAY\[0.."]
          set dim1 [string trimright $dim1 "\]"]
          incr dim1
          set dim2 1
        } else {
          set dim1 [string trimleft [lindex $secondosplit 0] "ARRAY\[0.."]
          set dim2 [string trimleft [lindex $secondosplit 1] "0.."]
          set dim2 [string trimright $dim2 "\]" ]
          incr dim1
          incr dim2
        }
      } elseif { [string first STRING [lindex $primosplit 1]] != -1 } {
        set tipo STRING
        #set secondosplit [split [lindex $primosplit 1] "\["]
        #set dim1 [string trim [lindex $secondosplit 1]]
        #set dim1 [string trimright $dim1 ";"]
        #set dim1 [string trimright $dim1 "\]"]
        #set dim1 [string trim $dim1]
        #set dim2 1
###!!!!!!!!!!! TEMPORANEO FINO A CHE BEPPE NON SISTEMA LA INIZIALIZZAZIONE DELLE STRINGHE
        set dim1 10
        set dim2 1
      } else {
        set tipo [string trimright [lindex $primosplit 1] ";"]
        set tipo [string trim $tipo]
        set dim1 1
        set dim2 1
      } 
      if {[llength $primosplit] == 2} {
        set terzosplit [split [lindex $primosplit 1] ";"]
        set inizializzazione 0
        # sistemo l'inizializzazione delle variabili non scalari
        if {$arrai==1} {
          set inizializzazione "\[[expr $dim1*$dim2]\(0\)\]"
        }
        if {[llength $terzosplit] > 1} {
          set descrizione [string trim [lindex $terzosplit 1]]
          set descrizione [string trimright $descrizione "*)"]
          set descrizione [string trimleft $descrizione "(*"]
          set descrizione [string trim $descrizione]
        } else {
          set descrizione {}
        }
      } elseif {[llength $primosplit] == 3} {
        set terzosplit [split [lindex $primosplit 2] ";"]
        set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
        set inizializzazione [string trim $inizializzazione]
        if {[llength $terzosplit] > 1} {
          set descrizione [string trim [lindex $terzosplit 1]]
          set descrizione [string trimright $descrizione "*)"]
          set descrizione [string trimleft $descrizione "(*"]
          set descrizione [string trim $descrizione]
        } else {
          set descrizione {}
        }
      } elseif {[llength $primosplit] > 3} {
        # ci sono i : anche nel commento 
        set terzosplit [split [lindex $primosplit 2] ";"]
        set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
        set inizializzazione [string trim $inizializzazione]
        set inizio_commento [string first "(" $linea]
        set fine_commento [string first ")" $linea]
        if { $inizio_commento != -1 && $fine_commento != -1 } {
          set descrizione [string range $linea [expr $inizio_commento + 2] \
              [expr $fine_commento - 2]]
          set descrizione [string trim $descrizione]
        }
      }

     set VariableList($nome) \
	    	[list $nvram \
  	    	$tipo \
	    	$scope \
 	    	$location \
  	    	$inizializzazione \
	    	$descrizione \
 	    	$dim1 \
	    	$dim2] 
    }
  }
  close $file_sorg
}

####################################################################
# Procedura LoadProgram
# Procedura che carica le variabili legate al file PLC e dell'MSD
# Input: 
#   filename    Nome del file da caricare
#   t           Nome del widget di testo
#   MAIN_FB     Flag che indica se Main o FB
# Ritorna :
#   Niente
####################################################################
proc LoadProgram { filename t MAIN_FB} {
global EditPrefs VariableList FunctionBlockList  InstructionList
global OffsetLineaProg Main_Fb ProgramComponent MachineDataList 

  # inizializzazione variabili in caso di nessuna dichiarazione di variabili
  set  nvram ""
  set  tipo ""
  set  scope ""
  set  location ""
  set  inizializzazione ""
  set  descrizione ""
  set  dim1 ""
  set  dim2 ""

  SetFilename $t $filename

  set Main_Fb $MAIN_FB

  # reset delle variabili precedentemete definite
  if [info exists VariableList] {
    unset VariableList
  }

  #reset function block definiti
   if {[info exists FunctionBlockList]} {
      unset FunctionBlockList
   }

  # cancellazione del widget di testo 
  $t delete 1.0 end
 
  set inizio_programma 0
  set linee_vuote 0
  # variabile globale che tiene traccia delle righe nascoste dall'editor
  set OffsetLineaProg 0
  # parole chiave che cambiano a seconda che sia nel main o in un function block
  if {$MAIN_FB == 0 } {
    set init_keyword PROGRAM
    set end_keyword END_PROGRAM
  } else {
    set init_keyword FUNCTION_BLOCK
    set end_keyword END_FUNCTION_BLOCK
  }

  if [catch { set file_sorg [open $filename r] } result] {
    AlertBox -text $result
    return
  }

  ##########################################
  #  Caricamento del testo
  set Trovato_prog 0
  set ind_prog 0
  set linea {}
  set TuttoIlFile [read -nonewline $file_sorg]
  while { $Trovato_prog == 0 } {
    set ind_prog [string first "$init_keyword " $TuttoIlFile]
    set TuttoIlFile [string range $TuttoIlFile $ind_prog end]
    set linea [string range $TuttoIlFile 0 [string first "\n" $TuttoIlFile]]  
    set TuttoIlFile [string range $TuttoIlFile [string length $linea] end]
    if {[string first "*)" $linea] == -1 } {
      $t insert end "$linea"
      set Trovato_prog 1
    }
  }
  set ind_list [string last "END_VAR" $TuttoIlFile]
  if {$ind_list != -1} {
    set TuttoIlFile [string range $TuttoIlFile $ind_list end]
    set TuttoIlFile [string range $TuttoIlFile \
      [expr [string first "\n" $TuttoIlFile] + 1] end ]
  }

  $t insert end $TuttoIlFile     
  unset TuttoIlFile

  seek $file_sorg 0 start

  while { [gets $file_sorg linea] >=0 } {
    if { $inizio_programma == 1 } {
       break
    } else {
      # Verifico se la prima parola rientra nel set delle istruzioni 
      foreach x [array names InstructionList] {
        set indice_lista_istruzioni [lsearch -exact $InstructionList($x) \
          [string toupper [lindex $linea 0]]]
        if { $indice_lista_istruzioni != -1 } {
          set inizio_programma 1
          break
        }
      }
   
      foreach x $MachineDataList {
        set indice_lista_datimacc [lsearch -exact $x [string toupper [lindex $linea 0]]]
        if { $indice_lista_datimacc != -1 } {
          set inizio_programma 1
          break
        }
      }

      if { $indice_lista_istruzioni != -1 || $indice_lista_datimacc != -1 } {
        continue 
      }
 
      if {[lindex $linea 0] == "VAR_GLOBAL"} {
        set scope global
        set location memory
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
        set OffsetLineaProg [expr $OffsetLineaProg + 1 + $linee_vuote]
				set linee_vuote 0
      } elseif {[lindex $linea 0] == "VAR"} {
        set scope local
        set location memory
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
        set OffsetLineaProg [expr $OffsetLineaProg + 1 + $linee_vuote]
				 #resetto le linee vuote poiche' quelle contate fino ad ora separavano
         #la definizione di variabili
	set linee_vuote 0
      } elseif {[lindex $linea 0] == "VAR_INPUT"} {
        set scope local
        set location input
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
        set OffsetLineaProg [expr $OffsetLineaProg + 1 + $linee_vuote]
	set linee_vuote 0
      } elseif {[lindex $linea 0] == "VAR_OUTPUT"} {
        set scope local
        set location output
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
        set OffsetLineaProg [expr $OffsetLineaProg + 1 + $linee_vuote]
	set linee_vuote 0
      } elseif {[lindex $linea 0] == "VAR_EXTERNAL"} {
        set scope extern
        set location memory
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
        set OffsetLineaProg [expr $OffsetLineaProg + 1 + $linee_vuote]
	set linee_vuote 0
      } elseif {[lindex $linea 0] == $init_keyword} {
        set OffsetLineaProg [expr $OffsetLineaProg + $linee_vuote]
	set linee_vuote 0
      } elseif {[lindex $linea 0] == $end_keyword} {
	set linee_vuote 0
	#close $file_sorg
	break
      } elseif {[lindex $linea 0] == ""} {
	set linee_vuote [expr $linee_vuote + 1 ]
      } elseif {[string match {(\**} [lindex $linea 0]] == 1} {
	set linee_vuote 0
      } elseif {[lindex $linea 0] != "END_VAR" } {
        set primosplit [split $linea :]
        set nome [string trim [lindex $primosplit 0]]

	###########
	# Controllo se ci sono delle definizioni di function  block

        if { $scope == "local" && $location == "memory" && $nvram == "noretain" } {
          set tipo [lindex $primosplit 1]
	  set tipo [string trimright $tipo ";"]
	  set tipo [string trim $tipo ]
	  if { $tipo != "BOOL" && $tipo != "DINT" && $tipo != "UDINT"  && \
               $tipo != "REAL"  && $tipo != "LREAL" && $tipo != "TIME" && \
               $tipo != "DATE" && $tipo != "STRING" && $tipo != "BYTE" && \
               $tipo != "WORD" && $tipo != "DWORD" && $tipo != "INT"  && \
               $tipo != "UINT" && [string first "ARRAY" $tipo] == -1} {
	    set FunctionBlockList($nome) $tipo
	    set OffsetLineaProg [expr $OffsetLineaProg +1]
            continue
	  }
	}
	#######

        set arrai 0
        if { [string first ARRAY [lindex $primosplit 1]] != -1 } {
          set arrai 1
          set tipo [lindex [lindex $primosplit 1] 2]
          set tipo [string trimright $tipo ";"]
          set secondosplit [split [lindex [lindex $primosplit 1] 0] ,]
          if {[llength $secondosplit] == 1} {
            set dim1 [string trimleft [lindex $secondosplit 0] "ARRAY\[0.."]
	    set dim1 [string trimright $dim1 "\]"]
	    incr dim1
            set dim2 1
          } else {
            set dim1 [string trimleft [lindex $secondosplit 0] "ARRAY\[0.."]
            set dim2 [string trimleft [lindex $secondosplit 1] "0.."]
	    set dim2 [string trimright $dim2 "\]" ]
	    incr dim1
	    incr dim2
          }
        } elseif { [string first STRING [lindex $primosplit 1]] != -1 } {
          set tipo STRING
          #set secondosplit [split [lindex $primosplit 1] "\["]
          #set dim1 [string trim [lindex $secondosplit 1]]
          #set dim1 [string trimright $dim1 ";"]
          #set dim1 [string trimright $dim1 "\]"]
          #set dim1 [string trim $dim1]
          #set dim2 1
###!!!!!!!!!!! TEMPORANEO FINO A CHE BEPPE NON SISTEMA LA INIZIALIZZAZIONE DELLE STRINGHE
          set dim1 10
          set dim2 1
        } else {
          set tipo [string trimright [lindex $primosplit 1] ";"]
          set tipo [string trim $tipo]
          set dim1 1
          set dim2 1
        } 
        if {[llength $primosplit] == 2} {
          set terzosplit [split [lindex $primosplit 1] ";"]
          set inizializzazione 0
	  # sistemo l'inizializzazione delle variabili non scalari
          if {$arrai==1} {
            set inizializzazione "\[[expr $dim1*$dim2]\(0\)\]"
          }
          if {[llength $terzosplit] > 1} {
            set descrizione [string trim [lindex $terzosplit 1]]
            set descrizione [string trimright $descrizione "*)"]
            set descrizione [string trimleft $descrizione "(*"]
            set descrizione [string trim $descrizione]
	   } else {
            set descrizione {}
          }
        } elseif {[llength $primosplit] == 3} {
          set terzosplit [split [lindex $primosplit 2] ";"]
          set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
          set inizializzazione [string trim $inizializzazione]
          if {[llength $terzosplit] > 1} {
            set descrizione [string trim [lindex $terzosplit 1]]
            set descrizione [string trimright $descrizione "*)"]
            set descrizione [string trimleft $descrizione "(*"]
	    set descrizione [string trim $descrizione]
	  } else {
	    set descrizione {}
          }
        } elseif {[llength $primosplit] > 3} {
          # ci sono i : anche nel commento 
          set terzosplit [split [lindex $primosplit 2] ";"]
          set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
          set inizializzazione [string trim $inizializzazione]
          set inizio_commento [string first "(" $linea]
	  set fine_commento [string first ")" $linea]
          if { $inizio_commento != -1 && $fine_commento != -1 } {
 	    set descrizione [string range $linea [expr $inizio_commento + 2] \
              [expr $fine_commento - 2]]
	    set descrizione [string trim $descrizione]
          }
        }

       set VariableList($nome) \
	    	[list $nvram \
  	    	$tipo \
	    	$scope \
 	    	$location \
  	    	$inizializzazione \
	    	$descrizione \
 	    	$dim1 \
	    	$dim2] 
#				puts "Variabile Nome : $nome Tipo : $tipo Descrizione : $descrizione "
         set OffsetLineaProg [expr $OffsetLineaProg +1]
       } else {
         set OffsetLineaProg [expr $OffsetLineaProg +1]
      }
    }
  }

  if {$MAIN_FB == 0 && [string first $EditPrefs(libpath) $filename] == -1} {
    # Vediamo se esistono gia' le variabili globali _VERSION e _CREATED
    # in caso contrario le inseriamo noi
    if { [lsearch -exact [array names VariableList] _VERSION] == -1 } {
      set VariableList(_VERSION) [list noretain STRING global memory '1.0.0.0' {} 10 1]
    }
    if { [lsearch -exact [array names VariableList] _CREATED] == -1 } {
      set VariableList(_CREATED) [list noretain UDINT global memory 0 {} 1 1]
    }
  }

  if [info exist ProgramComponent] {
    set ProgramComponent(0) [lappend ProgramComponent(0) $OffsetLineaProg]
  }

  close $file_sorg
  if {$MAIN_FB == 0 && [string first $EditPrefs(libpath) $filename] == -1} {
    LoadMsd $filename
  }
  MoveCursor $t 1.0
}

####################################################################
# Procedura LeggiIoconfFile
# Procedura che verifica se esiste il file ioconf ed in caso 
# affermativo lo va a leggere ed interpretare per vedere i moduli richiesti
# e quelli no
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc LeggiIoconfFile { } {
  global elenco_moduli VariableList
  global EditPrefs IoconfModificato


  set EsisteIlFile 0
  set IoconfModificato "no"
  if {$EditPrefs(plclocale) || ($EditPrefs(plclocale) == "0" && \
      $EditPrefs(plcremoto) == "localhost")} {
    if { [file exist $EditPrefs(progpath1)/$EditPrefs(ioconf)] } {
      set EsisteIlFile 1
      file copy -force $EditPrefs(progpath1)/$EditPrefs(ioconf) \
                         $EditPrefs(progpath)/$EditPrefs(ioconf)
    }
  } else {
    catch {socket}  res
    if {[ReceiveFile $EditPrefs(ioconf) $EditPrefs(objpath) $EditPrefs(progpath) \
        $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] == "0" } {
      set EsisteIlFile 1
    }
  }

  # Metto come default che tutti i moduli sono richiesti
  foreach modulo [array names elenco_moduli] {
    if { $elenco_moduli($modulo) != -1 } {
      lappend elenco_moduli($modulo) R
    } 
  }

  if { $EsisteIlFile == 1 } {
    if [catch { set fIoconf \
       [open $EditPrefs(progpath)/$EditPrefs(ioconf) r] } result] {
      AlertBox -text $result
      return
    }
    set TuttoIoconf [read $fIoconf]
    close $fIoconf
    set TrovataConf 0
    set ListaConf ""
    foreach linea [split $TuttoIoconf "\n"] {
      # Se ho trovato nel file ioconf una riga che inizia con
      # un cancelletto, che ha 6 elementi di cui l'ultimo e' una
      # R o una S e' una linea di configurazione
      if { [string index $linea 0] == "#" && \
           [llength $linea] == 6 && \
           ([lindex $linea end] == "R" || \
           [lindex $linea end] == "S")} {
        set TrovataConf 1
        lappend ListaConf [lindex $linea 1],[lindex $linea 2]
        if { $elenco_moduli([lindex $linea 1],[lindex $linea 2]) != -1 } {
          # Corrispondono il numero di input e di output
          if { [lindex $elenco_moduli([lindex $linea 1],[lindex $linea 2]) 1] == \
               [lindex $linea 3] && \
               [lindex $elenco_moduli([lindex $linea 1],[lindex $linea 2]) 2] == \
               [lindex $linea 4] } {
            set elenco_moduli([lindex $linea 1],[lindex $linea 2]) \
               [lreplace $elenco_moduli([lindex $linea 1],[lindex $linea 2]) end end \
               [lindex $linea end]]
          } else {
            set IoconfModificato "yes"
          }
        } else {
          set IoconfModificato "yes"
        }
      } elseif {[string index $linea 0] == "%" } {
        break
      }
    }

    if { $TrovataConf == 0 } {
      set IoconfModificato "yes"
      foreach modulo [array names elenco_moduli] {
        if { $elenco_moduli($modulo) != -1 } {
          set SplitVirgola [split $modulo ","]
          set TrovatoIo [string first \
            "IOS.0.[lindex $SplitVirgola 0].[lindex $SplitVirgola 1]" $TuttoIoconf]
          if { $TrovatoIo != -1 } {
            # dopo aver trovato il modulo debbo verificare se e' richiesto 
            # o simulato
            set TuttoIoconfTmp [string range $TuttoIoconf $TrovatoIo end]
            if { [lindex $TuttoIoconfTmp 1] == "R" || \
                 [lindex $TuttoIoconfTmp 1] == "S" } { 
              set elenco_moduli($modulo) [lreplace $elenco_moduli($modulo) end end \
                   [lindex $TuttoIoconfTmp 1]]
            }
          } 
        } 
      }
    }

    foreach NomeVar [array names VariableList] {
      # I/O Speciale
      if {[lindex $VariableList($NomeVar) 3] == "input" || \
          [lindex $VariableList($NomeVar) 3] == "output" } {
        if { [string first "." [lindex $VariableList($NomeVar) 8]] != -1} {
          if {[lindex $VariableList($NomeVar) 3] == "input" } {
            set Versus %I
          } elseif {[lindex $VariableList($NomeVar) 3] == "output" } {
            set Versus %Q
          }
          switch [lindex $VariableList($NomeVar) 1] {
              BOOL {append Versus X}
              BYTE {append Versus B}
              WORD {append Versus W}
              INT  {append Versus W}
              UINT {append Versus W}
              DINT
               -
              UDINT
               -
              DWORD
               -
              REAL {append Versus D}
              LREAL {append Versus L}
          }
          set IoString $Versus[lindex $VariableList($NomeVar) 8]
          set TrovatoIo [string first $IoString $TuttoIoconf]
          if { $TrovatoIo != -1 } {
            # dopo aver trovato il modulo debbo verificare se e' richiesto 
            # o simulato
            set TuttoIoconfTmp [string range $TuttoIoconf $TrovatoIo end]
            if { [lindex $TuttoIoconfTmp 1] == "R" || \
                 [lindex $TuttoIoconfTmp 1] == "S" } { 
              set NuovoIO "[lindex $VariableList($NomeVar) 8]_[lindex $TuttoIoconfTmp 1]"
              set VariableList($NomeVar) [lreplace $VariableList($NomeVar) 8 8 \
                $NuovoIO]
            }
          } 
        }
      }
    } 

    file delete $EditPrefs(progpath)/$EditPrefs(ioconf)
  }
}

####################################################################
# Procedura LoadModule
# Procedura che carica il file appoggio della MSD contenente
# informazioni sui tipi moduli
# Input: 
#   filename      Nome del file PLC
# Ritorna :
#   Niente
####################################################################
proc LoadModule { filename } {
global elenco_moduli max_linee max_moduli LineNumber SelectedLine
global MsdData VariableList SiIoconf

  if { [file exists [file root $filename].mod] } {
    if [info exists elenco_moduli] {
      unset elenco_moduli
    }
    source "[file root $filename].mod"
  
    # controllo che abbia caricato bene la matrice ELENCO_MODULI
    if { ![info exists elenco_moduli] } {
      AlertBox -text MsgBadIOConn
      ResetBoardList			;# azzera il buffer ELENCO_MODULI
    }
  } else {
    AlertBox -text MsgFileConnNotFound
    ResetBoardList			;# azzera il buffer ELENCO_MODULI
  }

  if { $SiIoconf == "yes" } {
    # gestione del file ioconf
    LeggiIoconfFile
  }

  #ottengo numero di linee presenti
  set max 0
  foreach i [array names elenco_moduli] {
    set ind [string range $i 0 [string match {,} $i]]
    if {$ind > $max && $elenco_moduli($i) != -1} {
      set max $ind
    }
  }
  set LineNumber [expr $max+1]
  set MsdData(LineNumberios) [expr $max+1]
  set SelectedLine 0

  foreach i [array names VariableList] {
    if { [lindex $VariableList($i) 8] != {} } {
      if { [string first "." [lindex $VariableList($i) 8]] != -1 } {
        if { [string first "UDC" [string toupper [lindex $VariableList($i) 8]]] != -1 } {
          set LineaOccupata [lindex [split [lindex $VariableList($i) 8] "."] 1]
          if {$MsdData(LineNumberudc) < [incr LineaOccupata]} {
            set MsdData(LineNumberudc) $LineaOccupata
          }
        } elseif { [string first "MPP" [string toupper [lindex $VariableList($i) 8]]] \
                  != -1 } {
          set LineaOccupata [lindex [split [lindex $VariableList($i) 8] "."] 1]
          if {$MsdData(LineNumbermpp) < [incr LineaOccupata]} {
            set MsdData(LineNumbermpp) $LineaOccupata
          }
        }
      }
    }
  }
}

####################################################################
# Procedura LoadMsd
# Procedura che carica l'MSD
# Input: 
#   filename      Nome del file PLC
# Ritorna :
#   Niente
####################################################################
proc LoadMsd {filename} {
global VariableList MsdData

  # considero ora il caricamento dell' MSD completando la lista 
  # VariableList 
  if {[file exists "[file root $filename].msd"] == 0} {
    AlertBox -text MsgFileConnNotFound
    return
  }
  if [catch { set file_msd [open "[file root $filename].msd" r] } result] {
    AlertBox -text $result
    return
  }
  set inizio_programma 0

  while  { [gets $file_msd linea] >= 0 } {
    if {$inizio_programma == 0 } {
      if {[lindex $linea 0] == "VAR_GLOBAL"} {
        set scope global
        set location memory
        if {[lindex $linea 1] == "RETAIN"} {
          set nvram retain
        } else {
          set nvram noretain
        }
      } elseif {[lindex $linea 0] == "PROGRAM"} {
        set inizio_programma 1
        continue
      } elseif {[lindex $linea 0] != "CONFIGURATION" && [lindex $linea 0] != "END_VAR" \
                && [lindex $linea 0] != "" && [lindex $linea 0] != "RESOURCE"} {
        set primosplit [split $linea :]
        if { [llength [lindex $primosplit 0]] == 1} {
          set nome [string trim [lindex $primosplit 0]]
        } elseif {[llength [lindex $primosplit 0]] == 3} {
          set nome [string trim [lindex [lindex $primosplit 0] 0]]
          if { [string first "I" [lindex [lindex $primosplit 0] 2]] == 1  } {
            set location input
          } elseif { [string first "Q" [lindex [lindex $primosplit 0] 2]] == 1 } { 
            set location output
          }
        }
        set arrai 0
        if { [string first ARRAY [lindex $primosplit 1]] != -1 } {
          set arrai 1
          set tipo [lindex [lindex $primosplit 1] 3]
          set tipo [string trimright $tipo ";"]
          set secondosplit [split [lindex [lindex $primosplit 1] 1] ,]
          if {[llength $secondosplit] == 1} {
            set dim1 [string trimleft [lindex $secondosplit 0] "\[0.."]
            set dim1 [string trimright $dim1 "\]"]
            set dim2 1
          } else {
            set dim1 [string trimleft [lindex $secondosplit 0] "\[0.."]
            set dim2 [string trimleft [lindex $secondosplit 1] "0.."]
            set dim2 [string trimright $dim2 "\]" ]
          }
        } else {
          set tipo [string trimright [lindex $primosplit 1] ";"]
          set tipo [string trim $tipo]
          set dim1 1
          set dim2 1
        } 
        if {[llength $primosplit] == 2} {
          set terzosplit [split [lindex $primosplit 1] ";"]
          set inizializzazione 0
          if {[llength $terzosplit] > 1} {
            set descrizione [string trim [lindex $terzosplit 1]]
            set descrizione [string trimright $descrizione "*)"]
            set descrizione [string trimleft $descrizione "(*"]
            set descrizione [string trim $descrizione]
          } else {
            set descrizione {}
          }
        } elseif {[llength $primosplit] == 3} {
          set terzosplit [split [lindex $primosplit 2] ";"]
          set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
          set inizializzazione [string trim $inizializzazione]
          if {[llength $terzosplit] > 1} {
            set descrizione [string trim [lindex $terzosplit 1]]
            set descrizione [string trimright $descrizione "*)"]
            set descrizione [string trimleft $descrizione "(*"]
            set descrizione [string trim $descrizione]
          } else {
            set descrizione {}
          }
        } elseif {[llength $primosplit] > 3} {
          # ci sono i : anche nel commento 
          set terzosplit [split [lindex $primosplit 2] ";"]
          set inizializzazione [string trimleft [lindex $terzosplit 0] "="]
          set inizializzazione [string trim $inizializzazione]
          set inizio_commento [string first "(" $linea]
          set fine_commento [string first ")" $linea]
          if { $inizio_commento != -1 && $fine_commento != -1 } {
            set descrizione [string range $linea [expr $inizio_commento + 2] \
               [expr $fine_commento - 2]]
            set descrizione [string trim $descrizione]
          }
        }

        # sistemo l'inizializzazione delle variabili non scalari
        if {$arrai==1} {
          set inizializzazione "\[[expr $dim1*$dim2]\(0\)\]"
        }
        set VariableList($nome) \
            [list $nvram \
            $tipo \
            $scope \
	          $location \
            $inizializzazione \
            $descrizione \
            $dim1 \
            $dim2] 

# rm
#				puts "Variabile Nome : $nome Tipo : $tipo Location : $location Scope : $scope Descrizione : $descrizione "
         
         if {[llength [lindex $primosplit 0]] == 3} {
           set msdsplit [split [lindex [lindex $primosplit 0] 2] .]
           set IoSpeciale 0
           if { [string first "IOS" [lindex [lindex $primosplit 0] 2]] != -1 } {
             foreach elemMsd [lrange $msdsplit 1 end] {
               if { [string length $elemMsd] > 2 } {
# rm tolto su segnalazione di beppe per 
# problemi in visualizzazione connessioni I/O
#                 set IoSpeciale 1
                 set IoSpeciale 0
                 break
               }
             }
           } 
           # I/O Speciale
           if { [string first "IOS" [lindex [lindex $primosplit 0] 2]] == -1 } {
             set VariableList($nome) [lappend \
              VariableList($nome) \
              [string trimright [string range [lindex \
                 [lindex $primosplit 0] 2] 3 end] ","]] 
           } elseif { [string first "IOS" [lindex [lindex $primosplit 0] 2]] != -1 && \
                ([string first "DAC" [string toupper \
                [lindex [lindex $primosplit 0] 2]]] != -1 || \
                [string first "ADC" [string toupper \
                [lindex [lindex $primosplit 0] 2]]] != -1 || \
                [string first "UDC" [string toupper \
                [lindex [lindex $primosplit 0] 2]]] != -1 || \
                [string first "MPP" [string toupper \
                [lindex [lindex $primosplit 0] 2]]] != -1 || $IoSpeciale == 1) } { 
             set VariableList($nome) [lappend \
              VariableList($nome) [string trimright [string range \
                 [lindex [lindex $primosplit 0] 2] 3 end] ","]] 
           } else {
             if { [lindex $msdsplit 2] != {} && [lindex $msdsplit 3] != {} && \
                  [lindex $msdsplit 5] != {} } {
               if { [lindex $msdsplit 1] == 0 } {
                 set VariableList($nome) [lappend VariableList($nome) \
                   [string range [lindex $msdsplit 0] 3 end] \
                   [lindex $msdsplit 2] [lindex $msdsplit 3] [lindex $msdsplit 5] ]
               } else {
                 set VariableList($nome) [lappend VariableList($nome) \
                   [string range [lindex $msdsplit 0] 3 end] \
                   [expr ($MsdData(maxlPerScheda)*[lindex $msdsplit 1]) \
                   +[lindex $msdsplit 2]] [lindex $msdsplit 3] [lindex $msdsplit 5] ]
               }
             } else {
               set VariableList($nome) [lappend VariableList($nome) {} {} {} {} ]
             }
           }
         }
       }
    } else {
          if {[lindex $linea 0] != "(*" && \
                  [lindex $linea 0] != "(" && \
                  [lindex $linea 0] != ")" && \
                  [lindex $linea 0] != ");" && \
                  [lindex $linea 0] != "" && \
                  [lindex $linea 0] != "END_RESOURCE" && \
                  [lindex $linea 0] != "END_CONFIGURATION"} {
        set primosplit_msd [split $linea =]
        set nome [string trimright [lindex $primosplit_msd 0] ":"]
        set nome [string trim $nome]
        if { [lindex $VariableList($nome) 1] == "INT" || \
             [lindex $VariableList($nome) 1] == "UINT" } {
            set VariableList($nome) [lappend VariableList($nome) \
                [string trimright [string range [lindex [lindex $primosplit_msd 1] \
                [expr [llength [lindex $primosplit_msd 1]] -1]] 3 end] ","]] 
        } else {
         set secondosplit_msd [split [lindex [lindex $primosplit_msd 1] \
             [expr [llength [lindex $primosplit_msd 1]] -1]] .]
            set VariableList($nome) [lappend VariableList($nome) \
                  [string range [lindex $secondosplit_msd 0] 3 end] \
                  [lindex $secondosplit_msd 2] [lindex $secondosplit_msd 3] \
                  [string trimright [lindex $secondosplit_msd 5] ","]] 
         }
        }
      }
    }
close $file_msd
}

####################################################################
# Procedura WritePlc
# Procedura che salva un file plc
# Input: 
#   filename      Nome del file PLC
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc WritePlc { filename t } {
global VariableList ModiFile ModiFileVect EditPrefs 
global ProgramComponent OffsetLineaProg MainOfFunctionBlock OldFilename

  SetFilename $t $filename

  if {[TestTipoFinestra $t]=="main"} {
    set key_word END_PROGRAM
  } else {
    set key_word END_FUNCTION_BLOCK
  }
  #verifico se il programma e' stato chiuso tranne che in caso di main
  #di libreria
  if {[string first $EditPrefs(libpath) $filename] == -1 || \
      [TestTipoFinestra $t]=="fb" } {
    set chiuso_programma [$t search -exact $key_word 1.0 end]
    if { $chiuso_programma == "" } {
      $t insert end "\n$key_word"
    }
  }

  set ModiFile 0
  if {[TestTipoFinestra $t]=="main"} {
    #gestione del nome del file da salvare 
    set nomefile [file tail $filename]
    set lung_nome [string first . $nomefile]
    #l'utente non ha inserito l'estensione
    if {$lung_nome == -1} {set lung_nome [string length $nomefile]}
    #limito il nome a soli 8 carattere 
    if {$lung_nome > 8} {set lung_nome 8}
      set filename \
        "[file dirname $filename]/[string range $nomefile 0 [expr $lung_nome-1]].plc"

    if {[string first $EditPrefs(libpath) $filename] == -1 } {
      # Vediamo se esistono gia' le variabili globali _VERSION e _CREATED
      # in caso contrario le inseriamo noi
      if { [lsearch -exact [array names VariableList] _VERSION] == -1 } {
        set VariableList(_VERSION) [list noretain STRING global memory '1.0.0.0' {} 10 1]
      }
      if { [lsearch -exact [array names VariableList] _CREATED] == -1 } {
       set VariableList(_CREATED) [list noretain UDINT global memory 0 {} 1 1]
      }
      # Inizializzo la variabile globale _CREATED con la data in secondi
      set VariableList(_CREATED) [lreplace $VariableList(_CREATED) 4 4 [clock seconds]] 
    }
  }

  if { $OldFilename != "" } { 
    if { [file tail $filename] != [file tail $OldFilename]} { 
      if [file exists [file rootname $OldFilename]] {
        file delete [file rootname $OldFilename]
      }
    }
    if { [file dirname $filename] != [file dirname $OldFilename]} { 
      foreach elemento [array names ProgramComponent] {
        if { $elemento == 0 } {
          if [file exists [file rootname $OldFilename]] {
            file delete [file rootname $OldFilename]
          }
        } else {
          set FileFB [file dirname $OldFilename]/[lindex $ProgramComponent($elemento) 0]
          if [file exists $FileFB] {
            file rename -force $FileFB \
              [file dirname $filename]/[lindex $ProgramComponent($elemento) 0]
          }
        }
      }
    }
  }

  # Scrittura del programma principale (main e FB) tranne che per il main di libreria
  if {[string first $EditPrefs(libpath) $filename] == -1 || \
       [TestTipoFinestra $t]=="fb" } {
    WriteVarsDef $filename $t 		
  } else {
    # Cancello file poiche' per i function block lo apro in append
    #file delete $filename
    WriteVarsDef $filename $t 		
  }

  if {[TestTipoFinestra $t]=="main"} {

    # sto salvando un programma

    if [array exists ProgramComponent] {
      AccodaFB $t
    }
    #salvataggio della msd tranne che nel caso di libreria
    if {[string first $EditPrefs(libpath) $filename] == -1 || \
        [TestTipoFinestra $t]=="fb" } {
      WriteMsd $filename $t
    }

    if [info exist ProgramComponent] {
      # Questo if e' stato messo per il caso di programma
      # che aveva dei blocchi funzionali e poi sono
      # stati tutti cancellati
      # e per il caso di programmi a cui sono
      # stati aggiunti ex novo Fb .
      # il Primo salvataggio era OK il secondo
      # per una mal scrittura di ProgramComponent perdeva
      # tutti i FB
      if { [llength [array names ProgramComponent]] > 1 || \
           ([llength [array names ProgramComponent]] == 1 && \
           [array names ProgramComponent] == 1)} {
        unset ProgramComponent
        SpezzaFile $filename $t
        RicercaPrototipiFB $t 1
        if [info exist OffsetLineaProg] {
          set ProgramComponent(0) [lappend ProgramComponent(0) $OffsetLineaProg]
        }
      } else {
        unset ProgramComponent
      }

      if {[string first $EditPrefs(libpath) $filename] != -1} {
        CalcolaHeaderFB $t
      }
      if {[string first $EditPrefs(libpath) $filename] == -1 || \
          [TestTipoFinestra $t]=="fb" } {
        set ProgramComponent(0) [lappend ProgramComponent(0) $OffsetLineaProg]
      }
    } 
  } else {
    # sto salvando una FB

    # vedo se e' una FB new oppure old
    set FB_new 1
    foreach x [array names ProgramComponent] {
      if { [lindex $ProgramComponent($x) 0] == [file tail $filename] } {
         set FB_new 0
         break
      }
    }

    if { $FB_new == 1 } {
      if {[array size ProgramComponent] == 0 } {
        #poiche' lo 0 e' riservato al main
        set ProgramComponent(1) [file tail $filename] 
      } else {
        set nuovo_indice [expr [lindex [lsort -decreasing \
          [array names ProgramComponent]] 0] + 1]
        set ProgramComponent($nuovo_indice) [file tail $filename] 
      }
    }
    RicercaPrototipiFB $MainOfFunctionBlock([TextToTop $t]) 0
    # setto il flag di modifica della finestra principale a 1
    set ModiFileVect(.jedit0) 1
  }

  # nessun errore di scrittura
  return 0
}

####################################################################
# Procedura GetNomeProg
# Procedura che ricerca del nome programma; il nome DEVE essere assolutamente a
# fianco della parola chiave PROGRAM
# Input: 
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc GetNomeProg { t } {

  set posizione [$t search -exact PROGRAM 0.0 end]
  if { $posizione == "" } {
    # parola chiave PROGRAM non trovata
    return {}
  } else {
    return [lindex [$t get $posizione [lindex [split $posizione {.}] 0].end] 1]
  }
}

####################################################################
# Procedura WriteMsd
# Procedura che scrive il file di msd secondo le normative
# Input: 
#   filename      Nome del file da salvare
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc WriteMsd {filename t} {
global VariableList elenco_moduli MsdData

  set program [GetNomeProg $t]
  #scrivo la MSD su file
  if [catch {set msd_id [open [file root $filename].msd w]} result] {
    AlertBox -text $result
    return
  }

  #scrivo anche il file contenente le informazioni sui tipi di modulo su file
  if [catch {set msd2_id [open [file root $filename].mod w]} result] {
    AlertBox -text $result
    return
  }

  foreach i [array names elenco_moduli] {
    if {[lindex $elenco_moduli($i) end] == "R" ||
        [lindex $elenco_moduli($i) end] == "S" } {
      # Filtro il flag che dice se il  modulo e' richiesto oppure
      # no in quanto l'informazione e' contenuta nel file ioconf
      puts $msd2_id "set elenco_moduli($i) [list [lrange $elenco_moduli($i) 0\
            [expr [llength $elenco_moduli($i)] -2]]]"
    } else {
      puts $msd2_id "set elenco_moduli($i) [list $elenco_moduli($i)]"
    }
  }
  close $msd2_id

  set save_var(glob_ret,in) {}
  set save_var(glob_noret,in) {}
  set save_var(glob_ret,out) {}
  set save_var(glob_noret,out) {}
  set save_var(loc_in,0) {}
  set save_var(loc_out,0) {}

  set keyword_var(glob_ret) "VAR_GLOBAL RETAIN" 
  set keyword_var(glob_noret) VAR_GLOBAL
  set NoCol 0

  # loop che discrimina i sei tipi di variabili:
  # globali ritentive input, globali non ritentive input, globali ritentive output, 
  # globali non ritentive output,
  # locali input , locali output .
  foreach i [lsort [array names VariableList]] {
    if { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "input" } {
      set save_var(glob_ret,in) [lappend save_var(glob_ret,in) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "input" } {
      set save_var(glob_noret,in) [lappend save_var(glob_noret,in) $i]
    } elseif { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "output" } {
      set save_var(glob_ret,out) [lappend save_var(glob_ret,out) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "output" } {
      set save_var(glob_noret_out) [lappend save_var(glob_noret,out) $i]
    } elseif { [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "input" } {
      set save_var(loc_in,0) [lappend save_var(loc_in,0) $i]
    } elseif { [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "output" } {
      set save_var(loc_out,0) [lappend save_var(loc_out,0) $i]
    }
  }

  #scrivo intestazione MSD
  puts -nonewline $msd_id "CONFIGURATION $program"
  puts $msd_id "_CONTROL\n"

  foreach i {glob_noret glob_ret} {
    # scrivo prima le variabili non ritentive e poi le ritentive
    if { [llength $save_var($i,in)] > 0 || [llength $save_var($i,out)] > 0 } { 
      puts $msd_id " $keyword_var($i)"
      #scrivo gli input e gli output  
      foreach j {in out} {
        foreach k $save_var($i,$j) {
          set linea_def  "\t$k AT" 
          if { $j == "in" } {
      	    # e' una variabile di input  : definisco il size
      	    switch [lindex $VariableList($k) 1] {
            BOOL {set direct %IX}
            BYTE {set direct %IB}
            WORD {set direct %IW}
            INT  {set direct %IW}
            UINT {set direct %IW}
            DINT
             -
            UDINT
             -
            DWORD
             -
            REAL {set direct %ID}
            LREAL {set direct %IL}
      	    } 
          } elseif { $j == "out" } {
            # e' una variabile di input  : definisco il size
            switch [lindex $VariableList($k) 1] {
              BOOL {set direct %QX}
              BYTE {set direct %QB}
              WORD {set direct %QW}
              INT  {set direct %QW}
              UINT {set direct %QW}
              DINT
               -
              UDINT
               -
              DWORD
               -
              REAL {set direct %QD}
              LREAL {set direct %QL}
            }
          }
          # I/O speciale
	  if { [string first "." [lindex $VariableList($k) 8]] != -1 } {
	    if { [string first "_S" [lindex $VariableList($k) 8]] != -1 } {
              set IoString [string trimright [lindex $VariableList($k) 8] "_S"]
            } else {
              set IoString [lindex $VariableList($k) 8]
            }
            set  linea_def "$linea_def $direct$IoString : "
	  } else {
            if { [lindex $VariableList($k) 8] == "" && \
                 [llength $VariableList($k)] == 8 } {
              set VariableList($k) [lappend VariableList($k) "IOS"]
            } elseif { [lindex $VariableList($k) 8] == "" && \
                       [llength $VariableList($k)] > 8 } {
              set VariableList($k) [lreplace $VariableList($k) 8 8 "IOS"]
            }
            if { [lindex $VariableList($k) 9] != "" } {
              if { [lindex $VariableList($k) 9] < $MsdData(maxlPerScheda) } {
                set  linea_def "$linea_def $direct[lindex \
                   $VariableList($k) 8].0.[lindex $VariableList($k) 9].[lindex \
                   $VariableList($k) 10].0.[lindex $VariableList($k) 11] : "
              } else {
                set  linea_def "$linea_def $direct[lindex \
                   $VariableList($k) 8].[expr [lindex $VariableList($k) \
                   9]/$MsdData(maxlPerScheda)].[expr [lindex $VariableList($k) \
                   9]%$MsdData(maxlPerScheda)].[lindex \
                   $VariableList($k) 10].0.[lindex $VariableList($k) 11] : "
              }
            } else {
              set linea_def "$linea_def $direct"
              append linea_def "NULL : "
              set NoCol 1
            }
	  }
          if { [lindex $VariableList($k) 6] == 1 && [lindex $VariableList($k) 7] == 1 } {
            set linea_def [format "%s[lindex $VariableList($k) 1]" $linea_def]
          } elseif { [lindex $VariableList($k) 6] > 1 && \
                     [lindex $VariableList($k) 7] == 1 } {
            set linea_def [format "%sARRAY\[0..%s\] OF [lindex $VariableList($k) 1]" \
                $linea_def [lindex $VariableList($k) 6]]
          } elseif { [lindex $VariableList($k) 6] > 1 && \
                     [lindex $VariableList($k) 7] > 1 } {
            set linea_def \
              [format "%sARRAY\[0..%s,0..%s\] OF [lindex $VariableList($k) 1]" \
              $linea_def [lindex $VariableList($k) 6] [lindex $VariableList($k) 7]]
          }
          set linea_def [format "%s:= [lindex $VariableList($k) 4];" $linea_def]
        
          if {[string length [lindex $VariableList($k) 5]] > 0 } {
            set linea_def [format "%s  (* %s *)" $linea_def [lindex $VariableList($k) 5]]
          } 
          puts $msd_id $linea_def
        }
      }
      puts $msd_id " END_VAR\n"
    }
  }

  puts $msd_id " RESOURCE unica ON sola_cpu_presente\n"
  puts $msd_id "  PROGRAM G : $program\n"
  puts $msd_id "   (* Input *)"
  puts $msd_id "    ("

  #primo loop per scrivere gli input
  foreach i $save_var(loc_in,0) {
    switch [lindex $VariableList($i) 1] {
      BOOL {set direct %IX}
      BYTE {set direct %IB}
      WORD {set direct %IW}
      INT  {set direct %IW}
      UINT {set direct %IW}
      DINT
       -
      UDINT
       -
      DWORD
       -
      REAL {set direct %ID}
      LREAL {set direct %IL}
    }
    # I/O Speciale
    if { [lindex $VariableList($i) 1] == "INT" || \
         [lindex $VariableList($i) 1] == "UINT" } {
      if { [string first "_S" [lindex $VariableList($i) 8]] != -1 } {
        set IoString [string trimright [lindex $VariableList($i) 8] "_S"]
      } else {
        set IoString [lindex $VariableList($i) 8]
      }
      puts  $msd_id "   $i := $direct$IoString, "
    } else {
      if { [lindex $VariableList($i) 9] != "" } {
        if { [lindex $VariableList($i) 9] < $MsdData(maxlPerScheda) } {
          puts $msd_id "   $i := $direct[lindex $VariableList($i) 8].0.[lindex \
            $VariableList($i) 9].[lindex $VariableList($i) 10].0.[lindex \
            $VariableList($i) 11]," 
        } else {
          puts $msd_id "   $i := $direct[lindex $VariableList($i) 8].[expr [lindex \
            $VariableList($i) 9]/$MsdData(maxlPerScheda)].[expr [lindex \
            $VariableList($i) 9]%$MsdData(maxlPerScheda)].[lindex $VariableList($i) \
            10].0.[lindex $VariableList($i) 11]," 
        }
      } else {
        set StringTmp "   $i := $direct"
        append StringTmp "NULL,"
        puts $msd_id $StringTmp 
        set NoCol 1
      }
    }
  }

  puts $msd_id "\n  (* Output *)"

  #secondo loop per scrivere gli output
  foreach i $save_var(loc_out,0) {
    switch [lindex $VariableList($i) 1] {
      BOOL {set direct %QX}
      BYTE {set direct %QB}
      WORD {set direct %QW}
      INT  {set direct %QW}
      UINT {set direct %QW}
      DINT
       -
      UDINT
       -
      DWORD
       -
      REAL {set direct %QD}
      LREAL {set direct %QL}
    }
    # I/O Speciale
    if { [lindex $VariableList($i) 1] == "INT" || \
         [lindex $VariableList($i) 1] == "UINT" } {
      if { [string first "_S" [lindex $VariableList($i) 8]] != -1 } {
        set IoString [string trimright [lindex $VariableList($i) 8] "_S"]
      } else {
        set IoString [lindex $VariableList($i) 8]
      }
      puts  $msd_id "   $i => $direct$IoString, "
    } else {
      if { [lindex $VariableList($i) 9] != "" } {
        if { [lindex $VariableList($i) 9] < $MsdData(maxlPerScheda) } {
          puts $msd_id "   $i => $direct[lindex $VariableList($i) 8].0.[lindex \
            $VariableList($i) 9].[lindex $VariableList($i) 10].0.[lindex \
            $VariableList($i) 11]," 
        } else {
          puts $msd_id "   $i => $direct[lindex $VariableList($i) 8].[expr [lindex \
            $VariableList($i) 9]/$MsdData(maxlPerScheda)].[expr [lindex \
            $VariableList($i) 9]%$MsdData(maxlPerScheda)].[lindex $VariableList($i) \
            10].0.[lindex $VariableList($i) 11]," 
        }
      } else {
        set StringTmp "   $i => $direct"
        append StringTmp "NULL,"
        puts $msd_id $StringTmp 
        set NoCol 1
      }
    }
  }
  puts $msd_id "   );"
  puts $msd_id " END_RESOURCE"
  puts $msd_id "END_CONFIGURATION"
  close $msd_id

  if { $NoCol == 1 } {
    AlertBox -text MsgNoCol
  }
}

####################################################################
# Procedura WriteVarsDef
# Procedura che scrive la definizione delle variabili secondo normative
# Input: 
#   filename      Nome del file da salvare
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc WriteVarsDef { filename t } {
global VariableList FunctionBlockList OffsetLineaProg

  set save_var(glob_ret) {}
  set save_var(glob_noret) {}
  set save_var(loc_ret) {}
  set save_var(loc_noret) {}
  set save_var(in_ret) {}
  set save_var(in_noret) {}
  set save_var(out_ret) {}
  set save_var(out_noret) {}
  set save_var(ext) {}

  set keyword_var(glob_ret) "VAR_GLOBAL RETAIN" 
  set keyword_var(glob_noret) VAR_GLOBAL 
  set keyword_var(loc_ret) "VAR RETAIN" 
  set keyword_var(loc_noret) VAR 
  set keyword_var(in_ret) "VAR_INPUT RETAIN" 
  set keyword_var(in_noret) VAR_INPUT 
  set keyword_var(out_ret) "VAR_OUTPUT RETAIN" 
  set keyword_var(out_noret) VAR_OUTPUT 
  set keyword_var(ext) VAR_EXTERNAL 

  set trovato_program 0
  set OffsetLineaProg 0
  set scritte_function_block 0

  #scrivo la definizione delle variabili su file 
  if [catch {set var_def_id [open $filename w]} result] {
    AlertBox -text $result
    return
  } 

  # loop che discrimina i nove tipi di variabili:
  # globali ritentive, globali non ritentive, locali ritentive, locali non ritentive,
  # input ritentive, input non ritentive, output ritentive, output non ritentive.
  # external
  foreach i [lsort [array names VariableList]] {

   # se sto salvando un FB non salvo le variabili globali
   if { [TestTipoFinestra $t]=="main" || ([lindex $VariableList($i) 2] != "global" && \
     [lindex $VariableList($i) 2] != "extern") } {
    if { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "memory" } {
      set save_var(glob_ret) [lappend save_var(glob_ret) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "global" && \
         [lindex $VariableList($i) 3] == "memory" } {
      set save_var(glob_noret) [lappend save_var(glob_noret) $i]
    } elseif { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "memory" } {
      set save_var(loc_ret) [lappend save_var(loc_ret) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "memory" } {
      set save_var(loc_noret) [lappend save_var(loc_noret) $i]
    } elseif { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "input" } {
      set save_var(in_ret) [lappend save_var(in_ret) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "input" } {
      set save_var(in_noret) [lappend save_var(in_noret) $i]
    } elseif { [lindex $VariableList($i) 0] == "retain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "output" } {
      set save_var(out_ret) [lappend save_var(out_ret) $i]
    } elseif { [lindex $VariableList($i) 0] == "noretain" && \
         [lindex $VariableList($i) 2] == "local" && \
         [lindex $VariableList($i) 3] == "output" } {
      set save_var(out_noret) [lappend save_var(out_noret) $i]
    } elseif { [lindex $VariableList($i) 2] == "extern" } {
      set save_var(ext) [lappend save_var(ext) $i]
    }		
   }
  }

  #scrivo definizione delle variabili
  foreach j {glob_ret glob_noret ext loc_ret loc_noret in_ret in_noret out_ret \
            out_noret} {
    if { [llength $save_var($j)] > 0 } {
      if { $j != "glob_ret" && $j != "glob_noret" && $j != "ext" && \
           $trovato_program == 0 } {
	set prima_linea [$t get 1.0 1.end]

        if {[TestTipoFinestra $t]=="main"} {
          set key_word "PROGRAM"
        } else {
          set key_word "FUNCTION_BLOCK"
        }

	if {[string first $key_word $prima_linea] != -1 } {
	  puts $var_def_id "$prima_linea"
	  set trovato_program 1
	}
      }
      if { $j != "glob_ret" && $j != "glob_noret" && $j != "ext"} {
        puts $var_def_id "\n  $keyword_var($j)" 
        set OffsetLineaProg [expr $OffsetLineaProg +2]
      } else {
        puts $var_def_id $keyword_var($j) 
        set OffsetLineaProg [expr $OffsetLineaProg +1]
      }	
      foreach i $save_var($j) {
        set linea_def "\t$i : "
        if { [lindex $VariableList($i) 6] == 1 && [lindex $VariableList($i) 7] == 1 } {
	  set linea_def [format "%s[lindex $VariableList($i) 1]" $linea_def]
        } elseif {[lindex $VariableList($i) 6] > 1 && [lindex $VariableList($i) 7] == 1} {
	  if { [lindex $VariableList($i) 1] != "STRING" } {
            set linea_def [format "%sARRAY\[0..%s\] OF [lindex $VariableList($i) 1]" \
               $linea_def [expr [lindex $VariableList($i) 6] - 1]]
	  } else {
            set linea_def [format "%sSTRING" $linea_def]
  	  }
        } elseif {[lindex $VariableList($i) 6] > 1 && [lindex $VariableList($i) 7] > 1} {
	  set linea_def [format "%sARRAY\[0..%s,0..%s\] OF [lindex $VariableList($i) 1]" \
              $linea_def [expr [lindex $VariableList($i) 6] - 1] \
              [expr [lindex $VariableList($i) 7] - 1]]
 	}
	if { $j != "ext" } {
	  set linea_def [format "%s:= [lindex $VariableList($i) 4];" $linea_def]
	} else {
	  set linea_def [format "%s;" $linea_def]
	}
		
	if {[string length [lindex $VariableList($i) 5]] > 0 } {
	  set linea_def [format "%s  (* %s *)" $linea_def [lindex $VariableList($i) 5]]
	} 
	puts $var_def_id $linea_def
        set OffsetLineaProg [expr $OffsetLineaProg +1]
      }

      ##########
      #INSERIMENTO DICHIARAZIONE FUNCTION BLOCK SE CI SONO VARIABILI LOCALI 
      #NON RITENTIVE 
      if { $j == "loc_noret" } {
        foreach i [array names FunctionBlockList] {
          set linea_def "\t$i : $FunctionBlockList($i);"
	  puts $var_def_id $linea_def
          set OffsetLineaProg [expr $OffsetLineaProg +1]
	}
	set scritte_function_block 1 
      }
      ##########

      if { $j != "glob_ret" && $j != "glob_noret" && $j != "ext"} {
        puts $var_def_id "  END_VAR"
        set OffsetLineaProg [expr $OffsetLineaProg +1]
      } else {
        puts $var_def_id "END_VAR\n"
        set OffsetLineaProg [expr $OffsetLineaProg +2]
      }
    }
  }

  ##########
  #INSERIMENTO DICHIARAZIONE FUNCTION BLOCK SE NON CI SONO VARIABILI LOCALI 
  #NON RITENTIVE 
  if { $scritte_function_block == 0 && [array names FunctionBlockList] != "" } {
    if { $trovato_program == 0} {
      puts $var_def_id [$t get 1.0 1.end] 
      set trovato_program 1
    } 
    puts $var_def_id "\n  VAR" 
    set OffsetLineaProg [expr $OffsetLineaProg +2]
    foreach i [array names FunctionBlockList] {
      set linea_def "\t$i : $FunctionBlockList($i);"
      puts $var_def_id $linea_def
      set OffsetLineaProg [expr $OffsetLineaProg +1]
    }
    puts $var_def_id "  END_VAR"
    set OffsetLineaProg [expr $OffsetLineaProg +1]
  }
  ##########

  if { $trovato_program == 0} {
    set StartPrg 1
  } else {
    set StartPrg 2
  }
  set EndPrg [lindex [$t index end] 0]
  for { set numLin $StartPrg } { $numLin < $EndPrg } { incr numLin } {
    puts $var_def_id [$t get $numLin.0 $numLin.end]
  } 
  close $var_def_id
}

####################################################################
# Procedura AccodaFB
# Procedura che accoda ad un un file programma i suoi function block
# Input: 
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc AccodaFB {t} {
global ProgramComponent EditPrefs

  if [catch {set id_main [open [GetFilename $t] a]} result] {
    AlertBox -text $result
    return
  }
  seek $id_main 0 end

  foreach i [array names ProgramComponent] {
    if { $i != 0 } {
      set RootPath [file dirname [GetFilename $t]]
      if [catch { set id_fb \
         [open $RootPath/[lindex $ProgramComponent($i) 0]] } result] {
        AlertBox -text $result
        continue
      }
      set var_fb [read -nonewline $id_fb]
      puts $id_main $var_fb
      close $id_fb
    }
  }
  close $id_main
}

