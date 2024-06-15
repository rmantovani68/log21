
            ######################################
            #                                    #
            #     File di definizione dei        #
            #     comandi per il debug remoto    #
            #                                    #
            ######################################

# 06/12/00 GG Aggiunta una pausa di sue secondi tra la trasmissione del
#             programma e l'invio del comando di caricamento, per dare tempo
#             al server ftp remoto di scaricare su disco il file ricevuto.
#             Senza questo artificio, il comando "compila e carica" poteva
#             provocare l'errore PLC 184 (formato del file errato),
#             perche` qplc andava a leggere un file ancora incompleto.

####################################################################
# Procedura DebugColloquio
# Procedura che crea una finestra di testo per la visualizzazione
# dei comandi verso il plc e la relativa risposta.
# La finestra viene attivata se nella variabile EditPrefs(DebugColloquio)
# c'e' specificato un nome  
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc DebugColloquio { } { 
global EditPrefs StartStopDebugColloquio

  if { $EditPrefs(DebugColloquio) == 0 } {
    return
  }
  set w $EditPrefs(DebugColloquio)
  catch {destroy $w}
  toplevel $w
  wm title $w [GetString DebugColloquio]
  wm protocol $w WM_DELETE_WINDOW procx
  set StartStopDebugColloquio 1

  frame $w.buttons
  pack $w.buttons -side bottom -fill x -pady 2m
  button $w.buttons.dismiss -text [GetString Dismiss] -font $EditPrefs(windowfont) \
     -command "destroy $w"
  button $w.buttons.code -text [GetString Clear] -font $EditPrefs(windowfont) \
     -command "$w.text delete 0.0 end"
  button $w.buttons.stop -text [GetString Stop] -font $EditPrefs(windowfont) \
     -command "set StartStopDebugColloquio 0
               $w.buttons.stop config -state disabled
               $w.buttons.start config -state normal"
  button $w.buttons.start -text [GetString Start] -font $EditPrefs(windowfont) \
     -command "set StartStopDebugColloquio 1
               $w.buttons.start config -state disabled
               $w.buttons.stop config -state normal"
  pack $w.buttons.dismiss $w.buttons.code $w.buttons.stop $w.buttons.start -side left \
     -expand 1

  $w.buttons.start config -state disabled

  text $w.text -relief sunken -bd 2 -yscrollcommand "$w.scroll set" -setgrid 1 \
	-height $EditPrefs(textheight) -width $EditPrefs(textwidth) \
        -font $EditPrefs(textfont) 
  scrollbar $w.scroll -command "$w.text yview"
  pack $w.scroll -side right -fill y
  pack $w.text -expand yes -fill both
}

####################################################################
# Procedura ScriviDebugColloquio
# Procedura che scrive nella finestra di debug del colloquio
# con il PLC
# Input: 
#   comando    Comando che viene inviato e che si vuole
#              scrivere
# Ritorna :
#   Niente
####################################################################
proc ScriviDebugColloquio { comando } { 
global EditPrefs StartStopDebugColloquio

  if { $EditPrefs(DebugColloquio) != 0 && [winfo exist $EditPrefs(DebugColloquio)] && \
       $StartStopDebugColloquio == 1} {
    $EditPrefs(DebugColloquio).text insert end "$comando\n"
  }
}

####################################################################
# Procedura AspettaRispostaPLC
# Procedura che aspetta la risposta del PLC
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc AspettaRispostaPLC { } {
global sId RispostaPLC DebugState

  set carattere [read $sId 1]
  append Risultato $carattere

  while { $carattere != "\n" && $carattere != "\0"} {
    set carattere [read $sId 1]
    append Risultato $carattere
  }
  ScriviDebugColloquio $Risultato
  if {[lindex $Risultato 0] != "A" } {
    if {[lindex $Risultato 0] == "L" } {
      set TuttiBuffer 0
      set QuantiBuffer 1
      while { $TuttiBuffer == 0 && $QuantiBuffer < 5} {
        set RisultatoTmp ""
        set carattere [read $sId 1]
        append RisultatoTmp $carattere
        while { $carattere != "\n" && $carattere != "\0"} {
          set carattere [read $sId 1]
          append RisultatoTmp $carattere
        }
        # Se il buffer di dati prosegue
        if {[string range $RisultatoTmp 0 1] == "D-" } {
          set RisultatoTmp "[string range $RisultatoTmp 0 0] [string range $RisultatoTmp 2 end]"
        } else {
          set TuttiBuffer 1
        }
        if { $QuantiBuffer == 1 } {
          append Risultato $RisultatoTmp
        } else {
          set Risultato [string trimright $Risultato "\n"]
          append Risultato [lindex $RisultatoTmp 1] 
        }
        ScriviDebugColloquio $Risultato
        incr QuantiBuffer
      }
      set RispostaPLC $Risultato
    } else {
      if { $Risultato == "\0"} {
        # Se ricevo un carattere nullo significa che ho
        # perso il colloquio con PLC e debbo chiudere il debug
        close $sId
        set sId -1
        AlertBox -text MsgNoPlc
        set RispostaPLC E
      } else {
        set RispostaPLC $Risultato
      }
    }
  } else {
    if {[lindex $Risultato 1] == 1 && $DebugState == 1} {
      # Se ricevo il comando A 1 significa che il Plc e'
      # in halt
      close $sId
      set sId -1
      AlertBox -text MsgPlcHalt
    }
  }
}

####################################################################
# Procedura HaltPlcRemoto
# Procedura che mette in halt il plc remoto
# Input: 
#   Nessuno
# Ritorna :
#   	0 	se e' andato tutto bene
#	1	in caso di errore
#	2	in caso di mancanza plc
####################################################################
proc HaltPlcRemoto { } {
global EditPrefs sId RispostaPLC

  if { $sId == -1 } {
    return 2
  } 
  ScriviDebugColloquio halt
  puts $sId "halt"
  flush $sId
  vwait RispostaPLC

  if { [lindex $RispostaPLC 0] == "E" } {
    return 1
  }

  set StatoEsecutore 1
  set contasecondi 0
  while { $StatoEsecutore == 1 && $contasecondi <= 10} {
    after 1000
    incr contasecondi
    ScriviDebugColloquio status
    puts $sId "status"
    flush $sId
    vwait RispostaPLC

    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "D" } {
      set StatoEsecutore [lindex $RispostaPLC 1]
    }
  }
  return 0
}

####################################################################
# Procedura GoPlcRemoto
# Procedura che mette in run il plc remoto
# Input: 
#   Nessuno
# Ritorna :
#   	0 	se e' andato tutto bene
#	1	in caso di errore
#	2	in caso di mancanza plc
####################################################################
proc GoPlcRemoto { } {
global EditPrefs sId RispostaPLC

  if { $sId == -1 } {
    return 2
  } 
  ScriviDebugColloquio go
  puts $sId "go"
  flush $sId
  vwait RispostaPLC

  if { [lindex $RispostaPLC 0] == "E" } {
    return 1
  } elseif { [lindex $RispostaPLC 0] == "OK" } {
    return 0
  }
}

####################################################################
# Procedura GoPlcRemoto
# Procedura che verifica l'esistenza del processo qplc
# Input: 
#   Nessuno
# Ritorna :
#   	0 	Il PLC e' presente
#	1	Il PLC non e' presente
####################################################################
proc VerificaEsistenzaPlc { } {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    catch { exec ps -ax } Result
    if {[string first "qplc" $Result] == -1 } {
      return 1
    } else {
      return 0
    }
  } else {
    if [catch {set sId [socket $EditPrefs(plcremoto) $EditPrefs(plcsocket)]} Result] {
      return 1
    } else {
      DebugColloquio
      fileevent $sId readable AspettaRispostaPLC
      vwait RispostaPLC
      return 0
    }
  }
}

####################################################################
# Procedura ApriPLC 
# Procedura che apre la connessione con il PLC in caso di PLC locale
# Input: 
#   Nessuno
# Ritorna :
#   	Niente
####################################################################
proc ApriPLC { } {
global EditPrefs

  if {$EditPrefs(plclocale)} {
    OpenPLC
  }
}

####################################################################
# Procedura ChiudiPLC
# Procedura che chiude la connessione con il PLC in caso di PLC locale
# o chiude il socket in caso di PLC remoto
# Input: 
#       Nessuno
# Ritorna :
#   	Niente
####################################################################
proc ChiudiPLC { } {
global EditPrefs sId

  if {$EditPrefs(plclocale)} {
    ClosePLC
  } else {
    if { $sId == -1 } {
      return
    } 
    close $sId
  }
}

####################################################################
# Procedura RicavaIndirizzoAssoluto 
# Procedura che restituisce l'indirizzo assoluto del modulo caricato
# dal PLC ed il nome del modulo stesso
# Input: 
#       Nessuno
# Ritorna :
#	1 	in caso di errore
#       2       in caso di mancanza plc
#       oppure	una lista il cui primo elemento e' l'indirizzo ed il secondo 
#		e' il nome del modulo da debuggare
####################################################################
proc RicavaIndirizzoAssoluto { } {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [indirizzo_assoluto U_CODE]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "mod U_CODE"
    puts $sId "mod U_CODE"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "D" } {
      if { $EditPrefs(plcremoto) == "localhost" } {
        set NomeModulo [lindex $RispostaPLC end]
        return [list 0x[lindex $RispostaPLC 1] $NomeModulo]
      } else {
        set NomeModulo [lindex $RispostaPLC end]
        if { [ReceiveFile [file tail $NomeModulo] [file dirname $NomeModulo ] \
             $EditPrefs(progpath) $EditPrefs(plcremoto) \
             $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
          return 1
        }
        set EditPrefs(nomemodulo) [file tail $NomeModulo]
        return [list 0x[lindex $RispostaPLC 1] \
          $EditPrefs(progpath)/$EditPrefs(nomemodulo)]
      }
    }
  }
}

####################################################################
# Procedura RicavaCRCModulo
# Procedura che restituisce il valore del CRC del modulo caricato
# Input: 
#      _CRCaddr    indirizzo della variabile
# Ritorna :
#	1 	in caso di errore
#       2       in caso di mancanza plc
#       oppure	Valore CRC
####################################################################
proc RicavaCRCModulo { _CRCaddr } {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [LeggiCRC $_CRCaddr]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "mem [format %x $_CRCaddr] 4"
    puts $sId "mem [format %x $_CRCaddr] 4"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "D" } {
      set CRC [InvertiByte [lindex $RispostaPLC 1]]
      return [format "%d" 0x$CRC]
    }
  }
}

####################################################################
# Procedura watchReset
# Procedura che esegue il comando di reset delle liste di watch
# Input: 
#      Niente
# Ritorna :
#       0       se e' andato bene
#	1	in caso di errore
#       2       in caso di mancanza plc
####################################################################
proc watchReset { } {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [watch reset]
  } else {
    if { $sId == -1 } {
      return 2
    }
    set Ritorno 0
    ScriviDebugColloquio wreset
    puts $sId "wreset"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      set Ritorno 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      set Ritorno 0
    }
    return $Ritorno
  }
}

####################################################################
# Procedura DeterminaParametri
# Procedura che crea una lista di watch con particolari condizioni 
# Input: 
#      Niente
# Ritorna :
#	-1	in caso di errore
#       i parametri della lista se e' andato bene
####################################################################
proc DeterminaParametri {args} {

  set act [expr 0x08 | 0x10 | 0x20]
  set condact 0x00
  set cond  0x01
  set apar 0
  set opc 0
  set opa1 0
  set opa2 0

  foreach  Parametri [lindex $args 0] {
    # Parametri che agiscono su act 
    if {$Parametri == "+set"} {
      set act [expr $act | 0x01]
     } elseif {$Parametri == "?set" } {
      set act [expr $act | 0x01]
      set condact [expr $condact | 0x01] 
    } elseif {$Parametri == "-set" } {
      set act [expr $act & ~0x01]
    } elseif {$Parametri == "+reset" } {
      set act [expr $act | 0x02]
    } elseif {$Parametri == "?reset" } {
      set act [expr $act | 0x02]
      set condact [expr $condact | 0x02]
    } elseif {$Parametri == "-reset" } {
      set act [expr $act & ~0x02]
    } elseif {$Parametri == "+tooggle" } {
      set act [expr $act | 0x04]
    } elseif {$Parametri == "?tooggle" } {
      set act [expr $act | 0x04]
      set condact [expr $condact | 0x04]
    } elseif {$Parametri == "-tooggle" } {
      set act [expr $act & ~0x04]
    } elseif {$Parametri == "+inc" } {
      set act [expr $act | 0x08]
    } elseif {$Parametri == "?inc" } {
      set act [expr $act | 0x08]
      set condact [expr $condact | 0x08]
    } elseif {$Parametri == "-inc" } {
      set act [expr $act & ~0x08]
    } elseif {$Parametri == "+cyc" } {
      set act [expr $act | 0x10]
    } elseif {$Parametri == "?cyc" } {
      set act [expr $act | 0x10]
      set condact [expr $condact | 0x10]
    } elseif {$Parametri == "-cyc" } {
      set act [expr $act & ~0x10]
    } elseif {$Parametri == "+logfl" } {
      set act [expr $act | 0x40]
    } elseif {$Parametri == "?logfl" } {
      set act [expr $act | 0x40]
      set condact [expr $condact | 0x40]
    } elseif {$Parametri == "-logfl" } {
      set act [expr $act & ~0x40]
    } elseif {$Parametri == "+log" } {
      set act [expr $act | 0x20]
    } elseif {$Parametri == "?log" } {
      set act [expr $act | 0x20]
      set condact [expr $condact | 0x20]
    } elseif {$Parametri == "-log" } {
      set act [expr $act & ~0x20]
    } elseif {$Parametri == "+brk" } {
      set act [expr $act | 0x80]
    } elseif {$Parametri == "?brk" } {
      set act [expr $act | 0x80]
      set condact [expr $condact | 0x80]
    } elseif {$Parametri == "-brk" } {
      set act [expr $act & ~0x80]
    } elseif {$Parametri == "false" } {		
       # Parametri che agiscono su cond
      set cond 0x00
    } elseif {$Parametri == "true" } {
      set cond 0x01
    } elseif {$Parametri == "and" } {
      set cond 0x02
    } elseif {$Parametri == "or" } {
      set cond 0x03
    } elseif {$Parametri == "nor" } {
      set cond 0x04
    } elseif {$Parametri == "tgt" } {
      set cond 0x05
    } elseif {$Parametri == "tle" } {
      set cond 0x06
    } elseif {$Parametri == "teq" } {
      set cond 0x07
    } elseif {$Parametri == "cgt" } {
      set cond 0x08
    } elseif {$Parametri == "cle" } {
      set cond 0x09
    } elseif {$Parametri == "ceq" } {
      set cond 0x0a
    } elseif {$Parametri == "rz" } {
      set cond 0x0b
    } elseif {$Parametri == "nrz" } {
      set cond 0x0c
    } elseif {$Parametri == "mz" } {
      set cond 0x0d
    } elseif {$Parametri == "nmz" } {
      set cond 0x0e
    } elseif {[string first "apar=" $Parametri] != -1 } {
      # Settaggio di apar
      set apar [string range $Parametri 5 end] 
    } elseif {[string first "opc=" $Parametri] != -1 } {
      # Settaggio di opc
      set opc [string range $Parametri 4 end]
    } elseif {[string first "opa1=" $Parametri] != -1} {
      # Settaggio di opa1
      set opa1 [string range $Parametri 5 end]
    } elseif {[string first "opa2=" $Parametri] != -1} {
      # Settaggio di opa2
      set opa2 [string range $Parametri 5 end]
    }
  }
  return [list [format %x $act] [format %x $condact] [format %x $cond] [format %x $apar] \
               [format %x $opc] [format %x $opa1] [format %x $opa2]] 
}

####################################################################
# Procedura watchCreate
# Procedura che esegue il comando di creazione di una lista di watch
# Input: 
#     WatchId   Identificativo della lista di watch 
#     args      Argomenti del comando
# Ritorna :
#	-1	in caso di errore
#       -2      in caso di mancanza plc
#       il descritttore della lista se e' andato bene
####################################################################
proc watchCreate {WatchId args} {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    set Comando "watch create $WatchId [lindex $args 0]"
    return [eval $Comando]
  } else {
    if { $sId == -1 } {
      return -2
    }
    set Comando "DeterminaParametri $args"
    set ParametriWatchCreate [eval $Comando]
    ScriviDebugColloquio "wcreate $ParametriWatchCreate"
    puts $sId "wcreate $ParametriWatchCreate"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return -1
    } elseif { [lindex $RispostaPLC 0] == "D" } {
      return [format %d 0x[lindex $RispostaPLC 1]]
    }
  }
}

####################################################################
# Procedura watchDelete
# Procedura che esegue il comando di cancellazione di una lista di watch
# Input: 
#     WatchId   Identificativo della lista di watch 
# Ritorna :
#       0       se e' andato bene
#	1	in caso di errore
#       2       in caso di mancanza plc
####################################################################
proc watchDelete {WatchId} {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [watch delete $WatchId]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "wdel $WatchId"
    puts $sId "wdel $WatchId"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      return 0
    }
  }
}

####################################################################
# Procedura watchAdd
# Procedura che esegue il comando di aggiunta di un break in una lista
# di watch
# Input: 
#     WatchId   Identificativo della lista di watch 
# Ritorna :
#       0       se e' andato bene
#	1	in caso di errore
#       2       in caso di mancanza plc
####################################################################
proc watchAdd {args} {
global EditPrefs sId RispostaPLC ByteLineeDebug OldNumLista

  set Parametri [lindex $args 0]
  set Log [lindex $Parametri end]
  if {$EditPrefs(plclocale)} {
    set ComandoDaEseguire "watch add [lrange $Parametri 1 [expr [llength $Parametri]-2]]"
    return [eval $ComandoDaEseguire]
  } else {
    if { $sId == -1 } {
      return 2
    }
    if { $Log == "+log" } {
      #Inizializza la lista dei byte di ciascuna variabile in debug
      if { [info exist OldNumLista] == 0 } {
        set OldNumLista 0
        while { $OldNumLista != [lindex $Parametri 1] && $OldNumLista < 50 } {
          set ByteLineeDebug [lappend ByteLineeDebug "#"]
          incr OldNumLista
        }
      } else {
        while { $OldNumLista != [lindex $Parametri 1] && $OldNumLista < 50 } {
          set ByteLineeDebug [lappend ByteLineeDebug "#"]
          incr OldNumLista
        }
        set OldNumLista [lindex $Parametri 1]
      }
      set r1 [lindex $Parametri 3] 
      set r2 [lindex $Parametri 4] 
      set scale [lindex $Parametri 5] 
      set size [lindex $Parametri 6] 
      set ar [lindex $Parametri 8] 
   
      if {$scale == 0 } {
        if {$r1} {
          set ByteLineeDebug [lappend ByteLineeDebug 4]
        } elseif {$r2 } {
          set ByteLineeDebug [lappend ByteLineeDebug 4]
        }
      } else {
        set ByteLineeDebug [lappend ByteLineeDebug $size]
      }
      if {$ar} {
        set ByteLineeDebug [lappend ByteLineeDebug 4]
      }
    }
    
    foreach argomenti [lrange $Parametri 1 [expr [llength $Parametri]-2]] {
      lappend ArgomentiEsadecimali [format %x $argomenti]
    }
    ScriviDebugColloquio "wadd $ArgomentiEsadecimali"
    puts $sId "wadd $ArgomentiEsadecimali"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      return 0
    }
  }
}

####################################################################
# Procedura watchEnable
# Procedura che esegue il comando di abilitazione di una lista
# di watch
# Input: 
#     WatchId   Identificativo della lista di watch 
# Ritorna :
#       0       se e' andato bene
#	1	in caso di errore
#       2       in caso di mancanza plc
####################################################################
proc watchEnable {WatchId} {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [watch enable $WatchId]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "wenable $WatchId"
    puts $sId "wenable $WatchId"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      return 0
    }
  }
}

####################################################################
# Procedura watchDisable
# Procedura che esegue il comando di disabilitazione di una lista
# di watch
# Input: 
#     WatchId   Identificativo della lista di watch 
# Ritorna :
#       0       se e' andato bene
#	1	in caso di errore
#       2       in caso di mancanza plc
####################################################################
proc watchDisable {WatchId} {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [watch disable $WatchId]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "wdisable $WatchId"
    puts $sId "wdisable $WatchId"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      return 0
    }
  }
}

####################################################################
# Procedura watchShow
# Procedura che esegue il comando di ricezione dei valori dei
# breakpoint impostati
# Input: 
#     Niente
# Ritorna :
#       1       in caso di errore
#       2       in caso di mancanza plc
#       I valori se e' andato bene
####################################################################
proc watchShow { } {
global EditPrefs sId RispostaPLC ByteLineeDebug LineaBreak OldNumLista
global TipoDebug

  if {$EditPrefs(plclocale)} {
    return [watch show]
  } else {
    if { $sId == -1 } {
      return 2
    }
    if { $TipoDebug == "show" } {
      ScriviDebugColloquio wshow
      puts $sId "wshow"
      flush $sId
      vwait RispostaPLC
      if { [lindex $RispostaPLC 0] == "E" } {
        return 1
      } elseif { [lindex $RispostaPLC 0] == "L" && [lindex $RispostaPLC 2] == "D" } {
        set LunghezzaRisposta [format %d 0x[lindex $RispostaPLC 1]]
        set comando "watchShowRemote $LunghezzaRisposta [lindex $RispostaPLC 3] \
           $ByteLineeDebug"
        if [info exist OldNumLista] {
          unset OldNumLista
        }
        return [eval $comando]
      }
    } elseif { $TipoDebug == "history" } {
      ScriviDebugColloquio "whistory v"
      puts $sId "whistory v"
      flush $sId
      vwait RispostaPLC
      if { [lindex $RispostaPLC 0] == "E" } {
        return 1
      } elseif { [lindex $RispostaPLC 0] == "L" && [lindex $RispostaPLC 2] == "D" } {
        set LunghezzaRisposta [format %d 0x[lindex $RispostaPLC 1]]
        set comando "watchHistoryRemote $LunghezzaRisposta [lindex $RispostaPLC 3] \
           $ByteLineeDebug"
        if [info exist OldNumLista] {
          unset OldNumLista
        }
        return [eval $comando]
      }
    }
  }
}

####################################################################
# Procedura watchBreak
# Procedura che esegue i comandi di break
# breakpoint impostati
# Input: 
#     Niente
# Ritorna :
#	-1	in caso di errore
#       0 o altro a seconda del comando se e' andato bene
####################################################################
proc watchBreak {args} {
global EditPrefs RispostaPLC

  set comando [lindex $args 0]
  
  switch $comando {

    reset { if [watchReset] { 
              return -1
            } else {
              return 0
            }
          }
    create { return [watchCreate [lindex $args 1] [lrange $args 2 end]] } 
    delete { if [watchDelete [lindex $args 1]] { 
              return -1
            } else {
              return 0
            }
          }
    add { if [watchAdd $args] { 
              return -1
            } else {
              return 0
            }
          }
    enable { if [watchEnable [lindex $args 1]] { 
              return -1
            } else {
              return 0
            }
          }
    disable { if [watchDisable [lindex $args 1]] { 
              return -1
            } else {
              return 0
            }
          }
    show { return [watchShow] }
  }
}

####################################################################
# Procedura CaricaOggetto 
# Procedura che carica l'oggetto compilato
# Input: 
#   NomeOggetto   Nome dell'oggetto da caricare  
#       0 	se e' andato tutto bene
#	1	in caso di errore
#       2       in caso di mancanza plc
# Ritorna :
####################################################################
proc CaricaOggetto {NomeOggetto} {
global EditPrefs sId RispostaPLC

  if {$EditPrefs(plclocale)} {
    return [CaricaUser $NomeOggetto]
  } else {
    if { $sId == -1 } {
      return 2
    }
    if { [HaltPlcRemoto] } {
      return 1
    }

    if { $EditPrefs(plcremoto) != "localhost" } {
      if { [TransmitFile $EditPrefs(nomemodulo) $EditPrefs(objpath) \
           $EditPrefs(progpath) $EditPrefs(plcremoto) \
           $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
        return 1
      }
    }
    ScriviDebugColloquio "load $EditPrefs(objpath)/$EditPrefs(nomemodulo)"
#
# Pausa, abbastanza arbitraria (due secondi), per dare tempo al server ftp
# remoto di scaricare il file su disco (problema visto su Linux RH6.2
# con RAID-5).
#
    after 2000
#
    puts $sId "load $EditPrefs(objpath)/$EditPrefs(nomemodulo)"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      return 1
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      if { [GoPlcRemoto] } {
        return 1
      } else {
        return 0
      }
    }
  }
}


####################################################################
# Procedura watchShowRemoteTcl 
# Procedura che analizza il buffer fornito da Beppe e che contiene
# il valore dei punti di break impostati.
# il Buffer e' un pezzo di memoria che ha rispecchia le seguenti
# strutture:
#
#typedef struct _brk_event {
# Dimensione dell'intera struttura.
#        unsigned short be_size;
# Indice della lista di watch, come ottenuto da "brkCreate".
#        char be_id;
# ZERO o piu` byte di dati. Sono i valori delle variabili o dei registri.
#        char be_data[1]; 
#} brk_event;
#
#typedef struct _brk_event_list {
# Dimensione dell'intero blocco di dati.
#        unsigned long bl_size;
# Ciclo di Plc durante il quale sono stati registrati gli eventi.
#        unsigned long bl_cycle;
# Lista degli eventi. Ce ne sara` sempre almeno uno.
#        brk_event bl_event[1];
#} brk_event_list;

# Input: 
#    LunghezzaRisposta  Lunghezza del buffer
#    Buffer 		Buffer dei punti di break
#    ByteLineeDebug	Lista delle dimensioni dei punti di break
# Ritorna :
#     1 Errore
#     Lista dei valori della lista di break
####################################################################
proc watchShowRemoteTcl {LunghezzaRisposta Buffer ByteLineeDebug} {

# Bisogna tenere presente che ogni carattere del Buffer 
# rappresenta 4 bit e che per dati superiori al byte 
# i byte devono essere inveriti
# Tolgo i primi 4 byte
  set BufferTmp [string range $Buffer 16 end]
  set ByteLineeDebugTmp $ByteLineeDebug
  set DatiAspettati [llength $ByteLineeDebugTmp]
  while {$BufferTmp != "" && $DatiAspettati > 0} {
    set DatoTmp "0x[string range $BufferTmp 2 3][string range $BufferTmp 0 1]"
    set SizeList [format "%d" 0x[string range $BufferTmp 2 3][string range $BufferTmp 0 1]]
    append Risultato "Lista[format "%d" 0x[string range $BufferTmp 4 5]]"
    if { [string length $BufferTmp] < [expr $SizeList * 2]} {
      # ERRORE
      break
    }
    set BufferTmp [string range $BufferTmp 6 end]
    set Indice 0
    foreach elemento $ByteLineeDebugTmp {
      set DatiAspettati [expr $DatiAspettati -1]
      if { $elemento == "#" } {
        set ByteLineeDebugTmp [lrange $ByteLineeDebugTmp [expr $Indice +1] end]
        set BufferTmp "$BufferTmp "
        break
      } elseif { $elemento == 1 } {
        append Risultato " 0x[string range $BufferTmp 0 1]"
        set BufferTmp [string range $BufferTmp 2 end]
      } elseif { $elemento == 4 } {
        append Risultato " 0x[string range $BufferTmp 6 7][string range $BufferTmp 4 5][string range $BufferTmp 2 3][string range $BufferTmp 0 1]"
        set BufferTmp [string range $BufferTmp 8 end]
      }
      incr Indice
    }
#puts $Risultato
  }
}

####################################################################
# Procedura MemorizzaCicloIniziale
# Procedura che ricava il ciclo PLC con il quale inizia
# il debug
# Input: 
#      _CRCaddr    indirizzo della variabile
# Ritorna :
#       0       tutto ok
#	1 	in caso di errore
#       2       in caso di mancanza plc
#       oppure	Valore Ciclo Corrente
####################################################################
proc MemorizzaCicloIniziale { } {
global EditPrefs sId RispostaPLC TipoDebug
global ListeOld

  if {$EditPrefs(plclocale)} {
    SetStartCycle [ReadSegnaleScambio {$DEB_CYCLE}]
  } else {
    if { $sId == -1 } {
      return 2
    }
    ScriviDebugColloquio "whistory s"
    puts $sId "whistory s"
    flush $sId
    vwait RispostaPLC
    if { [lindex $RispostaPLC 0] == "E" } {
      set TipoDebug show
    } elseif { [lindex $RispostaPLC 0] == "OK" } {
      set TipoDebug history
    }
  }
  for { set i 0 } {$i < 256 } { incr i } {
    set ListeOld($i) 0
  }
return 0
}
