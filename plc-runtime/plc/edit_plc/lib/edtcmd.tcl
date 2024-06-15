
            ######################################
            #                                    #
            #     File di gestione callback      #
            #     delle voci presenti nella      #
            #     menu bar                       #
            #                                    #
            ######################################

# GG 06/12/00 Usata la variabile "ProjectConf" per contenere il nome
#             del file di configurazione.

####################################################################
# Funzione CmdNew
# Funzione che gestisce l'apertura di un file nuovo
# Input: 
#   t           Nome della finestra di testo
#   filename    Nome del File nuovo da aprire
# Ritorna :
#   Niente
####################################################################
proc CmdNew { t filename } {
global EditFiles VariableList FunctionBlockList LineNumber
global SelectedLine ModiFile ProgramComponent 
global PredefinedFunctionBlock EditPrefs

  if [ConfirmBox -text ConfirmNewFile] {
    # Inizializzazione variabili
    set LineNumber 1
    set SelectedLine 0
    set ModiFile 0

    catch {unset PredefinedFunctionBlock}
    catch {unset ProgramComponent}
    catch {unset VariableList}
    catch {unset FunctionBlockList}

    # reset nome programma
    set EditFiles([TextToTop $t]) {}

    # preset libreria di Blocchi Funzionali
    PredefinedFunctionBlockListPreset

    # reset moduli connessi
    ResetBoardList

    # reset widget di testo
    TextDelete $t 1.0 end
    SetFilename $t $filename
    set filename [file root $filename]

    if {[string first $EditPrefs(libpath) $filename] != -1 } {
      $t configure -background lemonchiffon
      $t configure -state disabled
    }
  }
}

####################################################################
# Funzione CmdLoad
# Funzione che carica un file esistente o nuovo
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLoad { t } {
global ModiFile EditPrefs  LadderEditShow
global DebugState ComandiDos tcl_platform
global predir nc1000

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow } {
    return
  }

  if { [TextToTop $t]==".jedit0" && $ModiFile==1 } {
    if [ConfirmBox -text ConfirmFileChanged] {
     CmdSave $t
    }
    set ModiFile 0
  }

  SaveCheckpoint $t

  set filename [FileSelection $t  -title PromptLoad]

  if {"x$filename" != "x"} then {
    if {$EditPrefs(fs_diskA) == 0} {
      if {[string first "/tmp" $filename] == -1 } {
        # Caso locale 
        if {[file extension $filename] == ""} {
          set filename $filename.plc
        }
      } else {
        # Caso remoto
        # aggiungo l'estensione
        if {[lindex [split $filename .] 1] == "" } {
          set filename $filename.plc
        }

        if {$tcl_platform(platform) == "unix"} {
          set PercorsoAttivo $EditPrefs(progpath1)
        } else {
          set PercorsoAttivo $EditPrefs(progpath)
        } 
        if [file exists "$PercorsoAttivo/[file tail $filename]"] {
	  if ![ConfirmBox -text ConfirmExistFile] {
            set EditPrefs(fs_diskA) 0
            return
          }
        }
        WaitWindow $t 1
        update idletasks
        set NomeDelFile [file tail $filename]
        set DirettorioRemoto $EditPrefs(progpath1)
        append DirettorioRemoto [string range $filename [string length "/tmp"] end]
        set DirettorioRemoto [file dirname $DirettorioRemoto]
        if { [ReceiveFile $NomeDelFile $DirettorioRemoto $PercorsoAttivo \
           $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
          AlertBox -text MsgNoRemoteProgram
          return
        }

        set NomeDelFile [file root $NomeDelFile].msd
        if { [ReceiveFile $NomeDelFile $DirettorioRemoto $PercorsoAttivo \
           $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
          AlertBox -text MsgNoRemoteProgram
          return
        }

        set NomeDelFile [file root $NomeDelFile].mod
        if { [ReceiveFile $NomeDelFile $DirettorioRemoto $PercorsoAttivo \
           $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
          AlertBox -text MsgNoRemoteProgram
          return
        }

        set filename $PercorsoAttivo/[file tail $filename]
        WaitWindow $t 0
        update idletasks
      }
    } else {
      # aggiungo l'estensione
      if {[lindex [split $filename .] 1] == "" } {
        set filename $filename.plc
      }

      if {$tcl_platform(platform) == "unix"} {
        set PercorsoAttivo $EditPrefs(progpath1)
      } else {
        if { $nc1000 } {
          set PercorsoAttivo "/$predir$EditPrefs(progpath1)"
          if { ![file exists $PercorsoAttivo] } {
            set CurrentDir $EditPrefs(progpath)
          }
        } else {
          set PercorsoAttivo $EditPrefs(progpath)
        }
      } 
      # Se nel percorso del file su floppy c'e' library
      # carico una libreria
      if { [string first "library" $filename] != -1 } {
        set PercorsoAttivo $EditPrefs(libpath)
      }
      if [file exists "$PercorsoAttivo/[file tail $filename]"] {
	if ![ConfirmBox -text ConfirmExistFile] {
          set EditPrefs(fs_diskA) 0
          return
        }
      }

      if {$tcl_platform(platform) != "unix"} {
         if ![EsistenzaFloppy] {
           AlertBox -text MsgNoDosDiskInsert
           return
         }
      }

      set comando "catch {exec $ComandiDos(copy) A:$filename \
        $PercorsoAttivo } ErroreCopia" 
      eval $comando

      if { $ErroreCopia != "" && [string first Copying $ErroreCopia] == -1 } {
        AlertBox -text $ErroreCopia
      } else {
        set comando "catch {exec $ComandiDos(copy) A:[file root $filename].msd \
          $PercorsoAttivo } ErroreCopia"
        eval $comando

        if { $ErroreCopia != "" && [string first Copying $ErroreCopia] == -1 } {
          AlertBox -text $ErroreCopia
        } else {
          set comando "catch { exec $ComandiDos(copy) \
            A:[file root $filename].mod $PercorsoAttivo } ErroreCopia" 
          eval $comando

          if { $ErroreCopia != "" && \
               [string first Copying $ErroreCopia] == -1 } {
            AlertBox -text $ErroreCopia
          } 
        }
      }
      set filename $PercorsoAttivo/[file tail $filename]
    }
    if {[file exist $filename] != 0} {
      WaitWindow $t 1
      LoadFilePlc $filename $t
      WaitWindow $t 0
      if { $EditPrefs(fs_diskA) } {
        LibraryNeeded
      }
      set EditPrefs(fs_diskA) 0
    } else {
      CmdNew $t $filename
      if { $EditPrefs(awl) == 0 } {
        ResettaCanvas $t
      }
      set EditPrefs(fs_diskA) 0
      return
    }
    LoadModule $filename
    if { $EditPrefs(awl) == 0 } {
      ResettaCanvas $t
      AggiornaLadder  [TextToTop $t] $t [TextToTop $t].main.ladd
    }
  }
}
  
####################################################################
# Funzione CmdLoadEsecuzione
# Funzione che va a vedere se esiste il file plc-caricato
# e tenta di caricare il file il cui nome e' li' scritto
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLoadEsecuzione { t args } {
global ModiFile DebugState Home EditPrefs tcl_platform
global LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  if { [TextToTop $t]==".jedit0" && $ModiFile==1 } {
    if [ConfirmBox -text ConfirmFileChanged] {
       CmdSave $t
    }
    set ModiFile 0
  }

  SaveCheckpoint $t

  if {$EditPrefs(plclocale) || ($EditPrefs(plclocale) == "0" && \
      $EditPrefs(plcremoto) == "localhost")} {
    if [file exists $EditPrefs(objpath)/$EditPrefs(plc-caricato)] {
      if [catch { set fId \
         [open $EditPrefs(objpath)/$EditPrefs(plc-caricato) r] } result] {
        AlertBox -text $result
        return
      }
      set filename [gets $fId]
      close $fId
    } else {
      AlertBox -text MsgPlcCaricatoNoExist 
      return
    }
  } else {
    # Facciamo finta di aprire un socket per avere i servizi a disposizione
    WaitWindow $t 1
    update idletasks
    catch {socket}  res
    if { [ReceiveFile $EditPrefs(plc-caricato) $EditPrefs(objpath) \
         $EditPrefs(progpath) $EditPrefs(plcremoto) $EditPrefs(plclogin) \
         $EditPrefs(plcpassword)] != "0" } {
      AlertBox -text MsgPlcCaricatoNoExist 
      return
    }
    if [catch { set fId \
       [open $EditPrefs(progpath)/$EditPrefs(plc-caricato) r] } result] {
      AlertBox -text $result
      return
    } 
    set filename [gets $fId]
    close $fId
    file delete $EditPrefs(progpath)/$EditPrefs(plc-caricato)
    set Direttorio [file dirname $filename]
    set PrelevaDaRemoto 0
    if [ file exists $Direttorio ] {
      if { [file isdirectory $Direttorio] == 0 } {
        set filename "$EditPrefs(progpath)/[file tail $filename]"
        set PrelevaDaRemoto 1
      }
    } else {
      set filename "$EditPrefs(progpath)/[file tail $filename]"
      set PrelevaDaRemoto 1
    }

    if { $PrelevaDaRemoto == 1 } {
      # Ricerco i sorgenti non zippati
      set NomeSorgente [file tail $filename]
      set NomeMsd [file root [file tail $filename]].msd
      set NomeMod [file root [file tail $filename]].mod 
      if [file exists $filename] { 
        file copy -force $filename $filename.bak
      }
      if [file exists [file root $filename].msd] {
        file copy -force [file root $filename].msd [file root $filename].msd.bak
      }
      if [file exists [file root $filename].mod] {
        file copy -force [file root $filename].mod [file root $filename].mod.bak
      }
      if { [ReceiveFile $NomeSorgente $EditPrefs(progpath1) $EditPrefs(progpath) \
          $EditPrefs(plcremoto) $EditPrefs(plclogin) \
          $EditPrefs(plcpassword)] == "0" } {
        if { [ReceiveFile $NomeMsd $EditPrefs(progpath1) $EditPrefs(progpath) \
            $EditPrefs(plcremoto) $EditPrefs(plclogin) \
            $EditPrefs(plcpassword)] == "0" } {
          if { [ReceiveFile $NomeMod $EditPrefs(progpath1) $EditPrefs(progpath) \
              $EditPrefs(plcremoto) $EditPrefs(plclogin) \
              $EditPrefs(plcpassword)] == "0" } {
          }
        }
      } else {
        # Ricerco in remoto i sorgenti zippati
        if { [ReceiveFile source.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
              $EditPrefs(plcremoto) $EditPrefs(plclogin) \
              $EditPrefs(plcpassword)] == "0" } {
          if { [ReceiveFile srcmsd.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
                $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                $EditPrefs(plcpassword)] == "0" } {
            if { [ReceiveFile srcmod.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
                  $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                  $EditPrefs(plcpassword)] == "0" } {
              if { $tcl_platform(platform) == "unix" } {
                set comando "gzip -df"
              } else {
                set comando "$Home/gzip -df"
              }
              set comando1 "catch {exec $comando $EditPrefs(progpath)/source.gz} \
                Risultato"
              eval $comando1
              if { $Risultato == "" } {
                if [file exists $filename] { 
                  file copy -force $filename $filename.bak
                }
                file rename -force $EditPrefs(progpath)/source $filename
              }
              set comando1 "catch {exec $comando $EditPrefs(progpath)/srcmsd.gz} \
                Risultato"
              eval $comando1
              if { $Risultato == "" } {
                if [file exists [file root $filename].msd] {
                  file copy -force [file root $filename].msd [file root $filename].msd.bak
                }
                file rename -force $EditPrefs(progpath)/srcmsd \
                  [file root $filename].msd
              }
              set comando1 "catch {exec $comando $EditPrefs(progpath)/srcmod.gz} \
                 Risultato"
              eval $comando1
              if { $Risultato == "" } {
                if [file exists [file root $filename].mod] {
                  file copy -force [file root $filename].mod [file root $filename].mod.bak
                }
                file rename -force $EditPrefs(progpath)/srcmod \
                  [file root $filename].mod
              }
            } else {
              AlertBox -text MsgNoSorgentePLC
            }
          } else {
            AlertBox -text MsgNoSorgentePLC
          }
        } else {
          AlertBox -text MsgNoSorgentePLC
        }
      }
    }
    WaitWindow $t 0
    update idletasks
  }
 
  if {"x$filename" != "x"} then {
    if {[file exist $filename] != 0} {
      WaitWindow $t 1
      LoadFilePlc $filename $t
      WaitWindow $t 0
    } else {
      CmdNew $t $filename
      if { $EditPrefs(awl) == 0 } {
        ResettaCanvas $t
      }
      return
    }
    LoadModule $filename
    if { $EditPrefs(awl) == 0 } {
      ResettaCanvas $t
      AggiornaLadder  [TextToTop $t] $t [TextToTop $t].main.ladd
    }
  }
}

####################################################################
# Funzione CmdSave
# Funzione che scrive su file il programma PLC
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSave { t } {
global DebugState EditPrefs tcl_platform
global LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  set filename [GetFilename $t]

  if { [TestTipoFinestra $t]=="fb" } {
    if {"x$filename" != "x"} then {
      CmdSaveFB $t
    } else {
      set filename [FileSelection $t -title PromptSaveas]
      if {"x$filename" != "x"} then {
        if {[file extension $filename] == ""} {
          set filename $filename.plc
        }
        CmdSaveFB $t
      }
    }
    return
  }

  WaitWindow $t 1

  if {[string first $EditPrefs(libpath) $filename] != -1 } {
    AlertBox -text MsgNoProgFile
    return
  }

  if {"x$filename" != "x"} then {
    if {[file extension $filename] == ""} {
      set filename $filename.plc
    }
    WritePlc $filename $t
  } else {
    set filename [FileSelection $t -title PromptSaveas]
    if {"x$filename" != "x"} then {
      if {[file extension $filename] == ""} {
        set filename $filename.plc
      }
      WritePlc $filename $t
    }
  }
  WaitWindow $t 0
}

####################################################################
# Funzione CmdSaveas
# Funzione che scrive su file con nuovo nome il programma PLC
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSaveas { t } {
global DebugState EditPrefs OldFilename
global tcl_platform LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  set filename [GetFilename $t]

  if {[string first $EditPrefs(libpath) $filename] != -1 } {
    AlertBox -text MsgNoProgFile
    return
  }

  set filename [FileSelection $t  -title PromptSaveas]
  
  if {"x$filename" != "x" && ( ! [file exists $filename] || \
       [ConfirmBox -text "$filename [GetString ConfirmExistsReplace]"] )} {
    if {[file extension $filename] == ""} {
      set filename $filename.plc
    }
    WritePlc $filename $t
  }
}

####################################################################
# Funzione CmdLoadLibrary
# Funzione che carica un file di libreria
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLoadLibrary { t } {
global ModiFile  DebugState EditPrefs ComandiDos
global ProgramComponent tcl_platform

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { [TextToTop $t]==".jedit0" && $ModiFile==1 } {
    if [ConfirmBox -text ConfirmFileChanged] {
      CmdSave $t
    }
    set ModiFile 0
  }

  SaveCheckpoint $t

  set filename [FileSelection $t  -title PromptLoad -type Library]
  if {"x$filename" != "x"} then {
    if {$EditPrefs(fs_diskA) == 0} {
      if {[file extension $filename] == ""} {
        set filename $filename.plc
      }
    } else {
      # aggiungo l'estensione
      if {[lindex [split $filename .] 1] == "" } {
        set filename $filename.plc
      }
      set filename_disk $filename
      set filename $EditPrefs(libpath)/[file tail $filename]
      if [file exists $filename] {
        if ![ConfirmBox -text ConfirmExistFile] {
          set EditPrefs(fs_diskA) 0
          return
        }
      }
      if {$tcl_platform(platform) != "unix"} {
         if ![EsistenzaFloppy] {
           AlertBox -text MsgNoDosDiskInsert
           return
         }
      }
      set comando "catch { exec $ComandiDos(copy) A:$filename_disk \
        $EditPrefs(libpath) } ErroreCopia"
      eval $comando
      if { $ErroreCopia != "" && [string first Copying $ErroreCopia] == -1 } {
        AlertBox -text $ErroreCopia
      } 
    }

    if {[file exist $filename] != 0} {
      WaitWindow $t 1
      LoadFilePlc $filename $t
      WaitWindow $t 0
    } else {
      CmdNew $t $filename
    }
    set ProgramComponent(0) [list [file rootname [file tail $filename]] 1]
  }
  set EditPrefs(fs_diskA) 0
}

####################################################################
# Funzione CmdSaveLibrary
# Funzione che salva il file di libreria
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSaveLibrary { t } {
global DebugState EditPrefs  ProgramComponent

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { [llength [array names ProgramComponent]] <=1 } {
    AlertBox -text MsgNoFuncBlock
    return
  }
  WaitWindow $t 1 
  set filename [GetFilename $t]

  if {[string first $EditPrefs(libpath) $filename] == -1 } {
    AlertBox -text MsgNoLibFile
    return
  }

  if {"x$filename" != "x"} then {
    WritePlc $filename $t
  } else {
    set filename [FileSelection $t -title PromptSaveas -type Library]
    if {"x$filename" != "x"} then {
      if {[file extension $filename] == ""} {
        set filename $filename.plc
      }
      WritePlc $filename $t
    }
  }
  WaitWindow $t 0
}

####################################################################
# Funzione CmdSaveasLibrary
# Funzione che salva il file di libreria con un nome diverso
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSaveasLibrary { t } {
global DebugState EditPrefs tcl_platform ProgramComponent

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { [llength [array names ProgramComponent]] <=1 } {
    AlertBox -text MsgNoFuncBlock
    return
  }

  set filename [GetFilename $t]

  if {[string first $EditPrefs(libpath) $filename] == -1 } {
    AlertBox -text MsgNoLibFile
    return
  }

  set filename [FileSelection $t  -title PromptSaveas -type Library]
  if {"x$filename" != "x" && ( ! [file exists $filename] || \
       [ConfirmBox -text "$filename [GetString ConfirmExistsReplace]"] )} {
    if {[file extension $filename] == ""} {
      set filename $filename.plc
    }
    WritePlc $filename $t
  }
}

####################################################################
# Funzione CmdBackup
# Funzione che salva il file e lo copia su dischetto nel direttorio
# plc
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdBackup { t } {
global DebugState EditPrefs tcl_platform ComandiDos ModiFile
global LadderEditShow lista_librerie FunctionBlockList
global Home

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  if { [ConfirmBox -text ConfirmBackup] == 0} {
    return
  }

  set filename [GetFilename $t]
  if { $filename == "" } {
    AlertBox -text NoFileYet 
    return
  }
  if {[file extension $filename] == ""} {
    set filename $filename.plc
  }

  if { $ModiFile == 1 } {
    # salvo il file
    if {[string first $EditPrefs(libpath) $filename] == -1 } {
      CmdSave $t
    } else {
      CmdSaveLibrary $t
    }
  }

  if {$tcl_platform(platform) != "unix"} {
    if ![EsistenzaFloppy] {
       AlertBox -text MsgNoDosDiskInsert
       return
    }
  }
  if {[string first $EditPrefs(libpath) $filename] == -1 } {
    set comando "catch { exec $ComandiDos(makeDir) A:/plc} ErroreCopia"
    eval $comando

    set nomeFile [file tail $filename]
    set filename_disk "/plc/$nomeFile"
    set comando "catch { exec $ComandiDos(copy) $filename A:$filename_disk} \
      ErroreCopia"
    eval $comando
    if { $ErroreCopia != "" } {
      AlertBox -text $ErroreCopia
    } 

    set comando "catch { exec $ComandiDos(copy) [file root $filename].msd \
      A:[file root $filename_disk].msd } ErroreCopia"
    eval $comando
    if { $ErroreCopia != "" } {
      AlertBox -text $ErroreCopia
    } 

    set comando "catch { exec $ComandiDos(copy) [file root $filename].mod \
      A:[file root $filename_disk].mod } ErroreCopia"
    eval $comando
    if { $ErroreCopia != "" } {
      AlertBox -text $ErroreCopia
    }
    LibraryNeeded
  } else {
    set comando "catch { exec $ComandiDos(makeDir) A:/library} ErroreCopia"
    eval $comando

    set nomeFile [file tail $filename]
    set filename_disk "/library/$nomeFile"

    set comando "catch { exec $ComandiDos(copy) $filename A:$filename_disk} \
      ErroreCopia"
    eval $comando

    if { $ErroreCopia != "" } {
      AlertBox -text $ErroreCopia
    }
  }
}

####################################################################
# Funzione CmdNewWindow
# Funzione che apre una nuova finestra di editor
# plc
# Input: 
#   args    Opzioni
# Ritorna :
#   Niente
####################################################################
proc CmdNewWindow { args } {
global DebugState EditPrefs

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }
  CreateWindow
}

####################################################################
# Funzione CmdClose
# Funzione che chiude una finestra
# plc
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdClose { t } {
global WindowNumber VariableList FunctionBlockList
global VariableList[TextToTop $t] FunctionBlockList[TextToTop $t]
global ProgramComponent EditPrefs DebugState MainOfFunctionBlock
global LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }
 
  # non si puo' chiudere la finestra principale : bisogna usare "Quit"
  if { [TextToTop $t] == ".jedit0" } {
     CmdQuit $t
     return
  }

  foreach FuncBlock [array names MainOfFunctionBlock] {
    if {$MainOfFunctionBlock($FuncBlock) == [TextToTop $t] } {
      if [winfo exist $FuncBlock] {
        AlertBox -text MsgNoCloseMainFB 
        return
      }
    }
  }

  if {$WindowNumber == 1} {
    CmdQuit $t
  } else {
    incr WindowNumber -1    
    #reset variabili 
    catch {unset VariableList}
    catch {unset FunctionBlockList}
    catch {unset VariableList[TextToTop $t]}
    catch {unset unset FunctionBlockList[TextToTop $t]}

    set RootPath [file dirname [GetFilename $t]]
    destroy [TextToTop $t]
    # Verifico se ho generato file temporanei 
    # legati ai FB e li cancello
    if [info exist ProgramComponent] {
      foreach i [array names ProgramComponent] {
        if [file exists $RootPath/[lindex $ProgramComponent($i) 0]] {
          file delete $RootPath/[lindex $ProgramComponent($i) 0]
        }
      }
    }
  }
}

####################################################################
# Funzione CmdQuit
# Funzione che esce dall'applicativo
# plc
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdQuit { t } {
global input input2 inputkw ModiFile ProgramComponent Home EditPrefs
global prt_opz_change PrtOpt DebugState WindowType
global tcl_platform nc1000
global ProjectConf

  if { $DebugState == 1 } {
    set DebugState 0
    set AttivatoDebug 0
    DebugProgram $t
  }

  if { [TestTipoFinestra $t]=="fb" } {
    CmdCloseFB $t
    return
  }

  # Mi accerto che tutte le finestre siano state chiuse 
  # prima di uscire
  foreach x [array names WindowType] {
    if { $x != ".jedit0" && [winfo exists $x] } {
      AlertBox -text MsgTooWindow
      return 
    }
  } 

  if { $ModiFile==1 } {
    if [ConfirmBox -text ConfirmFileChanged] {
      set filename [GetFilename $t]
      if {[string first $EditPrefs(libpath) $filename] != -1} {
        CmdSaveLibrary $t
      } else {
        CmdSave $t
      }
    }
  }
  close $input
  if { $input2 != -1 } {
    close $input2
  }
  close $inputkw
  # Verifico se ho generato file temporanei 
  # legati ai FB e li cancello
  set RootPath [file dirname [GetFilename $t]]
  if [info exist ProgramComponent] {
    foreach i [array names ProgramComponent] {
      if [file exists $RootPath/[lindex $ProgramComponent($i) 0]] {
        file delete $RootPath/[lindex $ProgramComponent($i) 0]
      }
    }
  }
  # Verifico se sono state modificate le opzioni di stampa
  if { $prt_opz_change == "yes" } {
    if [ConfirmBox -text ConfirmOptionChanged] {
      if [catch { set fId [open $ProjectConf w] } out] {
        AlertBox -text $out
        return
      }
      foreach parametro [lsort [array names EditPrefs]] {
        if { $parametro == "plclocale" } {
          puts $fId "set EditPrefs($parametro) $EditPrefs($parametro)"
        } else {
          if { [llength $PrtOpt($parametro)] != 1 } {
            puts $fId "set EditPrefs($parametro) \"$PrtOpt($parametro)\""
          } else {
            puts $fId "set EditPrefs($parametro) $PrtOpt($parametro)"
          }
        }
      }
      close $fId
    }
  }
  
  if {($tcl_platform(platform) != "unix") && ($nc1000 == 0)} {
    ProsJollyStringa
    ProsChiudiProtezione
  }
  exit 0
}

####################################################################
# Funzione CmdCut
# Funzione che cancella la selezione fatta e la copia nella
# variabile CutBuffer
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdCut { t } {
global CutBuffer DebugState EditPrefs

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { ![TextHasSelection $t]} {
    AlertBox -text MsgNoSelection
    return 1
  }

  SaveCheckpoint $t

  set CutBuffer [$t get sel.first sel.last]
  TextDelete $t sel.first sel.last
  SetModiFile $t 1
}

####################################################################
# Funzione CmdCopy
# Funzione che copia il contenuto della seleziona nella variabile 
# CutBuffer
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdCopy { t } {
  global CutBuffer CopyFromText VariableList FunctionBlockList 
  global CopyVariable CopyFunctionBlock EditPrefs

  global DebugState
  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { ! [TextHasSelection $t]} {
    AlertBox -text MsgNoSelection
    return 1
  }

  set CutBuffer [$t get sel.first sel.last]
  set CopyFromText $t

  catch {unset CopyVariable}
  catch {unset CopyFunctionBlock}

  foreach x [array names VariableList] {
    if {[lsearch -exact $CutBuffer $x] != -1} {
      set CopyVariable($x) $VariableList($x)
    }
    if {[string first " $x\[" $CutBuffer] != -1 || \
        [string first "\t$x\[" $CutBuffer] != -1} {
      set CopyVariable($x) $VariableList($x)
    }
  }

  foreach x [array names FunctionBlockList] {
    if {[lsearch $CutBuffer "$x\.*"] != -1 || \
        [lsearch -exact $CutBuffer $x] != -1} {
      set CopyFunctionBlock($x) $FunctionBlockList($x)
    }
  }
}

####################################################################
# Funzione CmdPaste
# Funzione che inserisce il contenuto della variabile CutBuffer
# nel widget di testo
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPaste { t } {
  global CutBuffer CopyFromText DebugState elenco_moduli
  global VariableList FunctionBlockList CopyVariable CopyFunctionBlock
  global EditPrefs
  
  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  SaveCheckpoint $t

  if { $CutBuffer!={} } {
     SetModiFile $t 1
  }

  TextInsertString $t $CutBuffer

  set SkipMessage 0

  if [info exists CopyFromText] {
    if { $CopyFromText != $t } {
      set variabili_correnti [array names VariableList]
      foreach x [array names CopyVariable] {
        if {[lsearch -exact $variabili_correnti $x] == -1} {
          set NewAssociation [lrange $CopyVariable($x) 8 end]
          set SameAssociation 0
          if { $NewAssociation != "" } {
            foreach associazioni $variabili_correnti {
              if {[llength $NewAssociation] > 1} {
                if { [lindex $CopyVariable($x) 9] != "" } {
                  if { $elenco_moduli([lindex $CopyVariable($x) 9],[lindex \
                       $CopyVariable($x) 10]) == -1 } {
                    # Non esiste questo modulo
                    set SameAssociation 2
                    break
                  }
                  if { $NewAssociation == [lrange \
                    $VariableList($associazioni) 8 end]} {
                    # bit gia' occupato
                    set SameAssociation 1
                    break
                  } 
                }
              }  else {
                if { $NewAssociation == [lrange $VariableList($associazioni) \
                   8 end]} {
                  # bit gia' occupato
                  set SameAssociation 1
                  break
                } 
              }
            }
          }
          if { $SameAssociation != 0 } {
            set VariableList($x) [lrange $CopyVariable($x) 0 7]
            if { $SkipMessage == 0 } {
              if { $SameAssociation == 1} {
                set TestoMessaggio "[GetString MsgVarCopied] $x\n \
                  [GetString MsgChannelBusy] [lrange $CopyVariable($x) 8 end]"
              } else {
                set TestoMessaggio "[GetString MsgVarCopied] $x\n \
                  [GetString MsgNoModDef] [lindex $CopyVariable($x) 10]"
              }
              if { [ConfirmBox -text $TestoMessaggio -nobutton OKALL] == 0 } {
                set SkipMessage 1
              }
            }
          } else {
            set VariableList($x) $CopyVariable($x) 
          }
        } else {
          if { $SkipMessage == 0 } {
            if { [ConfirmBox -text "[GetString MsgVarExist] : $x" \
                             -nobutton OKALL] == 0 } {
              set SkipMessage 1
            }
          }
        }
      }

      set function_block_correnti [array names FunctionBlockList]
      foreach x [array names CopyFunctionBlock] {
        if {[lsearch -exact $function_block_correnti $x] == -1} {
          set FunctionBlockList($x) $CopyFunctionBlock($x)
        } else { 
          if { $SkipMessage == 0 } {
            if { [ConfirmBox -text "[GetString MsgFBExist] : $x" \
                             -nobutton OKALL] == 0 } {
              set SkipMessage 1
            }
          }
        }
      }
    }
  }
}

####################################################################
# Funzione CmdSelectAll
# Funzione che seleziona l'intero testo
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSelectAll { t } {
global DebugState EditPrefs

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }
  $t tag add sel 1.0 end
}

####################################################################
# Funzione CmdFind
# Funzione che attiva il box di ricerca stringa
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdFind { t } {
global LadderEditShow

  if { $LadderEditShow } {
    return
  }

  SaveCheckpoint $t
  FindBox $t
}

####################################################################
# Funzione CmdFindAgain
# Funzione che fa la ricerca sul testo della stringa
# preimpostata nel box di ricerca
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdFindAgain { t } {
global LadderEditShow

  if { $LadderEditShow } {
    return
  }

  SaveCheckpoint $t
  FindAgain $t
}

####################################################################
# Funzione CmdGoToLine
# Funzione che attiva il box di richiesta di linea su cui
# posizionarsi
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdGoToLine { t } {
global DebugState EditPrefs 

  set prompt_result [PromptBox -text PromptGoToLine -title PromptPrompt]
  if {$prompt_result != {}} then {
    if { $EditPrefs(awl) == 1 } {
      # IL
      GoToLine $t $prompt_result
    } else {
      # LADDER
      GoToSegment $t $prompt_result
    }
    ProtocolloAttivaDebug $t IL
  }
}

####################################################################
# Funzione CmdUndo
# Funzione che reinserisce l'ultima modifica fatta ripercorrendo
# un buffer di modifiche
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdUndo { t } {
global UndoText UndoTag EditPrefs UndoPointer DebugState

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if $EditPrefs(undolevels) {
    set UndoText($UndoPointer) [$t get 1.0 end]
    set UndoTag($UndoPointer) [GetTagAnnotation $t]
    incr UndoPointer -1
    if {![info exists UndoText($UndoPointer)]} {
      incr UndoPointer
      AlertBox -text MsgUndoNoMoreUndo
    }
    set fract [$t yview]
    $t delete 1.0 end
    $t insert end $UndoText($UndoPointer)
    SetTagAnnotation $t $UndoTag($UndoPointer)
    $t yview moveto [lindex $fract 0]
  } else {
    AlertBox -text MsgUndoNotOn
  }
}

####################################################################
# Funzione CmdRedo
# Funzione che ripercorre le mofiche fatte ripercorrendo 
# il buffer in avanti
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdRedo { t } {
global UndoText UndoTag EditPrefs UndoPointer DebugState

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if $EditPrefs(undolevels) {
    set UndoText($UndoPointer) [$t get 1.0 end]
    set UndoTag($UndoPointer) [GetTagAnnotation $t]
    incr UndoPointer
    if {![info exists UndoText($UndoPointer)]} {
      incr UndoPointer -1
      AlertBox -text MsgUndoNoMoreRedo
    }
    $t delete 1.0 end
    $t insert end $UndoText($UndoPointer)
    SetTagAnnotation $t $UndoTag($UndoPointer)
    $t yview -pickplace insert
  } else {
    AlertBox -text MsgUndoNotOn
  }
}

####################################################################
# Funzione CmdWrapline
# Funzione che spezza la linea
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdWrapline {t } {
  global DebugState EditPrefs

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  SaveCheckpoint $t

  set insert_corrente [$t index insert]
  $t mark set initline "insert linestart"
  $t mark set endline "insert lineend"

  set linea_wrap "[$t get initline insert]\n[$t get insert endline]"
  TextReplace $t initline endline $linea_wrap 0

  #riporto il cursore a prima del wrap
  $t mark set insert $insert_corrente 
  #avverto che il testo e' stato modificato
  SetModiFile $t 1
}

######################################################################
# Funzione CmdIoConnect
# Funzione che attiva il box di connessione IO
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdIoConnect { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }
  MsdBox $t
}

######################################################################
# Funzione CmdLoadFB
# Funzione che attiva il box con l'elenco dei bocchi funzionali
# definiti dall'utente
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLoadFB { t } {
global DebugState EditPrefs

  if { $DebugState == 1} {
    return
  }
  if { $EditPrefs(awl) == 0 } {
    return
  }

  if { [GetFilename $t] == "" } {
    AlertBox -text MsgNoNomeProg 
    return
  }
  LoadFB $t
}

######################################################################
# Funzione CmdCalcolaEquazioni
# Funzione che suddivide il programma plc in equazioni
# Il criterio di suddivisione e' l'istruzione LD o l'etichetta
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdCalcolaEquazioni { t } {
global PredefinedFunctionBlock FunctionBlockList ContactDef
global EditPrefs

  catch {unset ContactDef(linee_equazioni)}
  catch {unset ContactDef(linee_equazioni_FB)}
  catch {unset ContactDef(ordine_seg)}

  WaitWindow $t 1
  set indice_testo 0
  set indice_st 1
  set indice_tmp 0
  set LarghezzaCanvas 0
  set MaxLinee [lindex [split [$t index end] .] 0]

  for {set CurLine 1} {$CurLine <= $MaxLinee } {incr CurLine}  {
    set linee_testo [$t get $CurLine.0 $CurLine.end]
    set TrovatoFB 0
    incr indice_testo
    if { [expr $indice_testo + $indice_tmp -1] > [expr $indice_testo] } {
      incr indice_tmp -1
      continue
    }
    set contenuto_linea [lindex $linee_testo 0]
    if { [string range $contenuto_linea end end] == ":" } {
      set ContactDef(linee_equazioni) [lappend ContactDef(linee_equazioni) \
        $indice_testo]
      set ContactDef(linee_equazioni_FB) [lappend ContactDef(linee_equazioni_FB) 0]
      set ContactDef(ordine_seg) [lappend ContactDef(ordine_seg) [expr \
        [llength $ContactDef(linee_equazioni)] -1]]
    } elseif { $contenuto_linea == "LD" || $contenuto_linea == "LDN" || \
        [string first "MD_" $contenuto_linea] != -1} {
      if { $indice_st == 0 } {
        continue
      }
      set ContactDef(linee_equazioni) [lappend ContactDef(linee_equazioni) \
        $indice_testo]
      set ContactDef(ordine_seg) [lappend ContactDef(ordine_seg) [expr \
        [llength $ContactDef(linee_equazioni)] -1]]
      set riga_successiva_LD [$t get [expr $indice_testo + 1].0 [expr \
        $indice_testo +1].end]
      set indice_tmp 0
      if {([lindex $riga_successiva_LD 0] == "ST") && \
                [string match {[^a-zA-Z]*.*} [lindex $riga_successiva_LD 1]] } {
        # Se ho uno ST con una variabile che ha un punto in mezzo ed alla riga 
        # prima avevo un LD allora ci sono le condizioni per 
        # l'inizio di una chiamata a function block
        set tipovar "input"
        set funzione [lindex [split [lindex $riga_successiva_LD 1] .] 0]
        for { set i 0 } { $i< [llength \
          $PredefinedFunctionBlock($FunctionBlockList($funzione))] } {incr i } {
          set parametro [lindex \
            $PredefinedFunctionBlock($FunctionBlockList($funzione)) $i]
          if { $parametro != "###"} {
            if { $tipovar == "input" } {
              set lineaFB [$t get [expr $indice_testo + $indice_tmp].0 \
                [expr $indice_testo + $indice_tmp].end]
              if { [lindex $lineaFB 0] == "LD"} {
                incr indice_tmp
                set lineaFB [$t get [expr $indice_testo + $indice_tmp].0 \
                  [expr $indice_testo + $indice_tmp].end]
              } else {
                set indice_tmp 0
                break
              } 
              if { [lindex $lineaFB 0] == "ST" && \
                   [lindex $lineaFB 1] == "$funzione.$parametro" } {
                incr indice_tmp
              } else {
                set indice_tmp 0
                break
              }
            } elseif { $tipovar == "output" } {
              set lineaFB [$t get [expr $indice_testo + $indice_tmp].0 \
                [expr $indice_testo + $indice_tmp].end]
              if {([lindex $lineaFB 0] == "LD") && \
                   [lindex $lineaFB 1] == "$funzione.$parametro"} {
                incr indice_tmp
                set lineaFB [$t get [expr $indice_testo + $indice_tmp].0 \
                  [expr $indice_testo + $indice_tmp].end]
              } else {
                break
              } 
              if { [lindex $lineaFB 0] == "ST"} {
                incr indice_tmp
              } else {
                set indice_tmp 0
                break
              }
            }
          } else {
            set tipovar "output"
            set lineaFB [$t get [expr $indice_testo + $indice_tmp].0 \
              [expr $indice_testo + $indice_tmp].end]
            if { [lindex $lineaFB 0] == "CAL" && \
                 [lindex $lineaFB 1] == $funzione } {
              incr indice_tmp 
              set TrovatoFB 1
            } else {
              set indice_tmp 0
              break
            }
          } 
        }
        if { $TrovatoFB } {
          set DopoFB [$t get [expr $indice_testo + $indice_tmp].0 end]
          foreach LineeDopoFB [split $DopoFB "\n"] {
            if { [lindex $LineeDopoFB 0] == "" || \
                 [lindex $LineeDopoFB 0] == "(*" } {
              continue
            }
            if { [lindex $LineeDopoFB 0] != "LD" && \
                 [lindex $LineeDopoFB 0] != "LDN" && \
                 [string first "MD_" $LineeDopoFB] == -1} {
              set TrovatoFB 0
              set indice_tmp 0
            }
            break
          }
        }
      } 
      if { $indice_tmp != 0 } {
        set ContactDef(linee_equazioni_FB) [lappend \
            ContactDef(linee_equazioni_FB) 1]
      } else {
        set ContactDef(linee_equazioni_FB) [lappend \
            ContactDef(linee_equazioni_FB) 0]
        set indice_st 0
      }
    } elseif { $contenuto_linea == "ST" || $contenuto_linea == "STN" || \
               $contenuto_linea == "S" || $contenuto_linea == "R" || \
               $contenuto_linea == "JMP" || $contenuto_linea == "JMPC" || \
               $contenuto_linea == "JMPCN" } {
      # gestisce il caso 
      # and(
      # ld fc1
      set indice_st 1
    }
  }

  if [info exist ContactDef(ordine_seg)] {
    set ContactDef(MaxSegment) [expr [llength $ContactDef(ordine_seg)] -1]
  } else {
    set ContactDef(MaxSegment) -1
  }
  set ContactDef(linee_equazioni) [lappend ContactDef(linee_equazioni) \
    [lindex [split [$t index end] .] 0]]
  WaitWindow $t 0
  DisplayCurrentLine $t
}

######################################################################
# Funzione CmdDebugOption
# Funzione che attiva il pannello per la definizione
# delle opzioni per il debug
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdDebugOption { t } {
global LadderEditShow

  if { $LadderEditShow == 1 } {
    return
  }
  DebugOptionWindow $t
}

######################################################################
# Funzione CmdCompileOption
# Funzione che attiva il pannello per le opzioni della compilazione
# 
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdCompileOption { t } {
global LadderEditShow

  if { $LadderEditShow == 1 } {
    return
  }
  CompileOptionWindow $t
}

######################################################################
# Funzione Cmdlingua
# Funzione che seleziona la lingua
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLingua { t } {
global EditPrefs prt_opz_change PrtOpt

  set NuovaLingua [PromptBox -text PromptNewLingua -title PromptSelectLingua]

  if { [string length $NuovaLingua] == 3 } {
    set EditPrefs(lingua) $NuovaLingua
    if { "[GetString NoFile]" == "NoFile" } {
      set NuovaLingua ing
    }
    set EditPrefs(lingua) $NuovaLingua
    set PrtOpt(lingua) $NuovaLingua
    set prt_opz_change yes
  }
}

######################################################################
# Funzione CmdPrintPlci
# Funzione che lancia la stampa del sorgente PLC
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintPlci { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  PrintIlProgram $t
}

######################################################################
# Funzione CmdPrintPlcl
# Funzione che lancia la stampa del sorgente PLC in ladder
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintPlcl { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }
}

######################################################################
# Funzione CmdPrintCross
# Funzione che lancia la stampa della croos reference
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintCross { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

   PrintCrossReference $t
}

######################################################################
# Funzione CmdPrintMsd
# Funzione che lancia la stampa dell ' MSD
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintMsd { w } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

  PrintMsd $w
}

######################################################################
# Funzione CmdPrintDocal
# Funzione che lancia la stampa della documentazione completa
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintDocal { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

   PrintDocAl $t
}

######################################################################
# Funzione CmdPrintPopz
# Funzione che attiva il pannello di opzioni stampa
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdPrintPopz { t } {
global DebugState LadderEditShow

  if { $DebugState == 1} {
    return
  }
  if { $LadderEditShow == 1} {
    return
  }

   PrintOptions
}

######################################################################
# Funzione CmdSaveFB
# Funzione che salva un blocco funzionale
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdSaveFB { t } {
global LoadErrorFB 

  if { $LoadErrorFB != {} } {
    AlertBox -text "[GetString MsgDoubleDef] $LoadErrorFB"
    set errore 1
  } else {
    WaitWindow $t 1
    set errore [WritePlc [GetFilename [TextToTop $t]] $t]
    WaitWindow $t 0
  }
  return $errore
}

######################################################################
# Funzione CmdCloseFB
# Funzione che chiude una finestra contenente un blocco funzionale
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdCloseFB { t } {
global WindowNumber ModiFile EditFiles
global VariableList[TextToTop $t] FunctionBlockList[TextToTop $t]

  # preset errore di scrittura =0
  set errore 0
  if {$ModiFile==1} {
    if [ConfirmBox -text ConfirmFunctionBlockChanged] {
       set errore [CmdSaveFB $t]
    }
    set ModiFile 0
  }
  if { $errore == 0 } {
    incr WindowNumber -1 
    catch {unset VariableList[TextToTop $t]}
    catch {unset FunctionBlockList[TextToTop $t]}
    unset EditFiles([TextToTop $t])
    destroy [TextToTop $t]
  }
}

######################################################################
# Funzione CmdNewSegment
# Funzione che apre un nuovo segmento di ladder
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdNewSegment { t } {
global ContactDef ContactComment LadderEditShow

  if { $LadderEditShow } {
    return
  }
  set LadderEditShow 1
  CambiaButtonBar $t

  ResettaCanvasEdit [winfo parent $t].ladd.edit
  set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
      -title [GetString MsgCommTitle]] 
  ModalitaEdit [winfo parent $t].ladd
  set ContactDef(TipoEdit) NEW
}

######################################################################
# Funzione CmdNewFunctioBlock
# Funzione che apre un nuovo blocco funzionale di ladder
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdNewFunctioBlock { t } {
global ContactDef ContactComment LadderEditShow

  if { $LadderEditShow } {
    return
  }

  set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
      -title [GetString MsgCommTitle]] 
  set ContactDef(TipoEdit) NEWFB
  SalvaSegmento "" [winfo parent $t].ladd $t
}

######################################################################
# Funzione CmdNewMathEq
# Funzione che apre un nuova equazione matematica di ladder
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdNewMathEq { t } {
global ContactDef ContactComment LadderEditShow

  if { $LadderEditShow } {
    return
  } 
  set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
       -title [GetString MsgCommTitle]] 
  set ContactDef(TipoEdit) NEWMATH
  SalvaSegmento "" [winfo parent $t].ladd $t
}

######################################################################
# Funzione CmdOldSegment
# Funzione che richiama un vecchio segmento
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdOldSegment { t } {
global ContactDef ContactComment LadderEditShow

  if { $LadderEditShow } {
    return
  }
  set canvas [winfo parent $t].ladd
  if { [lindex $ContactDef(linee_equazioni_FB) \
       $ContactDef(SegmentoCorrente)] == 0 } {
    set LadderEditShow 1
    CambiaButtonBar $t
    set righe_colonne_occupate [AnalizeIlEquation $canvas.edit $t \
      $ContactDef(SegmentoCorrente)]
    DimensionaCanvas $canvas.edit [lindex $righe_colonne_occupate 0] \
      [lindex $righe_colonne_occupate 1]
    ModalitaEdit $canvas
    set ContactDef(TipoEdit) OLD
    RicercaCommentiEdit $t $canvas.edit
  } else {
    RicercaCommentiEdit $t $canvas.[lindex $ContactDef(ordine_seg) \
      $ContactDef(SegmentoCorrente)]
    set ContactDef(TipoEdit) OLD
    SalvaSegmento "" $canvas $t
  }
}

######################################################################
# Funzione CmdLabel
# Funzione che inserisce una etichetta nel ladder
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdLabel { t } {
global ContactDef ContactComment LadderEditShow

  if { $LadderEditShow } {
    return
  }
  set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
         -title [GetString MsgCommTitle]] 
  set ContactDef(TipoEdit) LABELB
  SalvaSegmento "" [winfo parent $t].ladd $t
}

######################################################################
# Funzione CmdShowLadder
# Funzione che mostra le equazioni convertite
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdShowLadder { t } {
global ContactDef ContactComment LadderEditShow
global EditPrefs

  set LadderEditShow 0
  CambiaButtonBar $t
  MostraLadder [winfo parent $t].ladd
}

######################################################################
# Funzione CmdInsertSegment
# Funzione che inserisce il nuovo segmento
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdInsertSegment { t } {
global ContactDef ContactComment EditPrefs
global LadderEditShow

  set NewSegment [ConvertiSegmento [winfo parent $t].ladd.edit]
  SalvaSegmento $NewSegment [winfo parent $t].ladd $t

  if { $NewSegment != -1 } {
    set LadderEditShow 0
    CambiaButtonBar $t
  }
}

######################################################################
# Funzione CmdDeleteSegment
# Funzione che cancella un segmento
# Input: 
#   t           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc CmdDeleteSegment { t } {
global ContactDef ContactComment EditPrefs LadderEditShow

  if { $LadderEditShow } {
    return
  }
  if {[ConfirmBox -text [GetString MsgSure]] == 0 } {
    return
  }
  SetModiFile $t 1
  set canvas [winfo parent $t].ladd
  set PuntoInserimentoInizio [lindex $ContactDef(linee_equazioni) \
    $ContactDef(SegmentoCorrente)]
  # Vado alla ricerca della linea di fine segmento precedente
  # includendo il caso di commenti intersegmento
  set index_linea [expr $PuntoInserimentoInizio - 1]
  set linea [$t get $index_linea.0 $index_linea.end]
  while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "") && \
         ($index_linea >=0)} {
    incr PuntoInserimentoInizio -1
    incr index_linea -1
    set linea [$t get $index_linea.0 $index_linea.end]
  }
  set PuntoInserimentoFine [lindex $ContactDef(linee_equazioni) \
    [expr $ContactDef(SegmentoCorrente) + 1]]
  # Vado alla ricerca della linea di fine segmento precedente
  # includendo il caso di commenti intersegmento
  set index_linea [expr $PuntoInserimentoFine - 1]
  set linea [$t get $index_linea.0 $index_linea.end]
  while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "" || \
          [lindex $linea 0] == "END_PROGRAM") && ($index_linea >=0)} {
    incr PuntoInserimentoFine -1
    incr index_linea -1
    set linea [$t get $index_linea.0 $index_linea.end]
  }
  set LineeVecchioSegmento [expr $PuntoInserimentoFine - \
    $PuntoInserimentoInizio]
  destroy $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)]
  $canvas delete segment[lindex $ContactDef(ordine_seg) \
    $ContactDef(SegmentoCorrente)]
  $t delete $PuntoInserimentoInizio.0 $PuntoInserimentoFine.end
  set ContactDef(ordine_seg) [concat [lrange $ContactDef(ordine_seg) \
              0 [expr $ContactDef(SegmentoCorrente) - 1]] \
              [lrange $ContactDef(ordine_seg) \
              [expr $ContactDef(SegmentoCorrente) +1] end ]]
  set ContactDef(linee_equazioni) [concat [lrange $ContactDef(linee_equazioni) \
              0 [expr $ContactDef(SegmentoCorrente) - 1]] \
              [lrange $ContactDef(linee_equazioni) \
              [expr $ContactDef(SegmentoCorrente) +1] end ]]
  set ContactDef(linee_equazioni_FB) [concat [lrange \
              $ContactDef(linee_equazioni_FB) \
              0 [expr $ContactDef(SegmentoCorrente) - 1]] \
              [lrange $ContactDef(linee_equazioni_FB) \
              [expr $ContactDef(SegmentoCorrente) +1] end ]]
  for { set i $ContactDef(SegmentoCorrente) } \
      { $i < [llength $ContactDef(linee_equazioni)]} {incr i} {
    set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) \
              $i $i [expr [lindex $ContactDef(linee_equazioni) $i] - \
              $LineeVecchioSegmento]] 
  }
  # Aggiungo una equazione in sostituzione di quella che ho cancellato
  if {[expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum) -1] <= \
      [expr [llength $ContactDef(ordine_seg)] -1] && \
      [winfo exists $canvas.[lindex $ContactDef(ordine_seg) \
      [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum) -1]]] == 0 } {
    ScriviEquazione [TextToTop $t] $canvas $t [expr \
    $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum) -1] 
  }
  MostraLadder $canvas 
  $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
    configure -background lemonchiffon
}

######################################################################
# Funzione GoToLine
# Funzione che posiziona il cursore alla linea specificata in
# ingresso
# Input: 
#   t           Nome della finestra di testo
#   lineno      Numero linea su cui posizionarsi
# Ritorna :
#   Niente
####################################################################
proc GoToLine { t {lineno 0} } {

  set result [catch { MoveCursor $t $lineno.0 }]
  if {$result} {
    AlertBox -text "$lineno [GetString MsgBadLineNumber]"
  }
}

######################################################################
# Funzione GoToSegment
# Funzione che posiziona il cursore al segmento specificato in
# ingresso
# Input: 
#   t           Nome della finestra di testo
#   lineno      Numero linea su cui posizionarsi
# Ritorna :
#   Niente
####################################################################
proc GoToSegment { t {lineno 0} } {
global ContactDef EditPrefs LadderEditShow

  if { $LadderEditShow } {
    return
  }
  set canvas [winfo parent $t].ladd
  if { $lineno <= $ContactDef(MaxSegment) } {
    if [winfo exist $canvas.[lindex $ContactDef(ordine_seg) \
        $ContactDef(SegmentoCorrente)]] {
      $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
         configure -background white 
    }
    set ContactDef(SegmentoCorrente) $lineno
    if { $ContactDef(MaxSegment) < [expr $ContactDef(SegmentoCorrente) + \
         $EditPrefs(eqnum)] } {
      if [info exist ContactDef(ordine_seg)] {
        for { set i $ContactDef(SegmentoCorrente) } \
            { $i <= $ContactDef(MaxSegment) } {incr i} {
          if {[winfo exist $canvas.$i] == 0 } {
            ScriviEquazione [TextToTop $t] $canvas $t $i 
          }
        }
      }
    } else {
      if [info exist ContactDef(ordine_seg)] {
        for { set i $ContactDef(SegmentoCorrente) } \
            { $i < [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] } \
            {incr i} {
          if {[winfo exist $canvas.$i] == 0 } {
            ScriviEquazione [TextToTop $t] $canvas $t $i 
          }
        }
      }
    }
    set PosizioneSegmento [$canvas coords segment$ContactDef(SegmentoCorrente)]
    $canvas yview moveto [expr \
      [lindex $PosizioneSegmento 1]/[lindex [lindex \
      [$canvas configure -scrollregion] 4] 3]] 
    DisplayEquationNumber [TextToTop $t].main.status.position \
           [lindex $ContactDef(linee_equazioni) \
           [expr $ContactDef(SegmentoCorrente) -1]] 1
    $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
           configure -background lemonchiffon
  }
}

######################################################################
# Funzione LibraryNeeded
# Funzione che verifica se il file prelevato o backuppato
# necessita di librerie
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc LibraryNeeded { } {
global lista_librerie FunctionBlockList
global Home

  # Verifico se questo programma fa uso di librerie
  # ed in caso affermativo chiedo se si desiderano
  # copiare anche quelle
  set elenco_librerie [array names lista_librerie]
  if { $elenco_librerie != "" } {
    set lib_used ""
    foreach nomiFB [array names FunctionBlockList] {
      set TipoFunctionBlock [lindex $FunctionBlockList($nomiFB) 0]
      if { [lsearch -exact $elenco_librerie $TipoFunctionBlock] != -1 } {
        set NomeLibreria [file tail $lista_librerie($TipoFunctionBlock)]
        set NomeLibreria $Home/library/[file root $NomeLibreria]
        if { [string first $NomeLibreria $lib_used] == -1 } {
          set lib_used [append lib_used "$NomeLibreria;\n"]
        }
      }
    }
    if { [lindex $lib_used 0] != "" } {
      set comando "AlertBox -text \"[GetString MsgBackupLibrary] \n $lib_used\"" 
      eval $comando
    }
  }
}
