

            ######################################
            #                                    #
            #     File di gestione del debug     #
            #                                    #
            ######################################

####################################################################
# Procedura AggiornaAccumulatoreIL
# Procedura che aggiorna il valore dell' accumulatore IL
# Input: 
#   w            Nome del widget di testo
#   Risultato    Risultato della lista di watch 
#   x            ???
#   indice_break Indice del break
#   ListaWatch   Elenco dei watchpoint impostati
#   colore       colore del foreground per valore 1
#   colore1	 secondo colore foreground per valore 0
# Ritorna :
#   Niente
####################################################################
proc AggiornaAccumulatoreIL { w Risultato x indice_break ListaWatch colore colore1} {
global LineaBreak accuwdg opwdg index0 index1 Old_Risultato Old_indice_break_lista \
       indice_break_lista 

  # C'e' solo l'accumulatore
  if [info exists Old_Risultato] {
    if [info exists Old_indice_break_lista($ListaWatch)] {
      if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
           $indice_break - $indice_break_lista($ListaWatch)]] == \
           [lindex $Risultato $indice_break] } {
        incr indice_break
        return $indice_break
      }
    }
  }

  if { [lindex $LineaBreak($x) 8] == "BOOL" } {
    if {[format "%d" [lindex $Risultato $indice_break]] >= 1 } {
      # Controllo se c'e' uno o piu' tag associati al risultato
      if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
        $w tag configure [lindex $LineaBreak($x) 7] -foreground $colore -background black
      } else {
        foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
          if {[string first "_N" $TagBreak] == -1} {
            $w tag configure $TagBreak -foreground $colore -background black
          } else {
            # Caso del negato
            $w tag configure [string trimright $TagBreak "_N"] -foreground $colore1 \
            -background white
          }
        }
      }
    } elseif {[format "%d" [lindex $Risultato $indice_break]] == 0 } {
      if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
        $w tag configure [lindex $LineaBreak($x) 7] -foreground $colore1 -background white
      } else {
        foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
	  if {[string first "_N" $TagBreak] == -1} {
            $w tag configure $TagBreak -foreground $colore1 -background white
   	  } else {
	    # Caso del negato
            $w tag configure [string trimright $TagBreak "_N"] -foreground $colore \
            -background black
   	  }
        }
      }
    }
  } else {
    if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
      set riga [string range [lindex $LineaBreak($x) 7] \
        [expr [string first "_" [lindex $LineaBreak($x) 7]] + 1] end]
      if { [lindex $LineaBreak($x) 8] == "REAL" } {
        set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
        set [string range [lindex $LineaBreak($x) 7] 0 \
            [expr [string first "_" [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp
      } else {
        set temp [format "%d" [lindex $Risultato $indice_break]]
        set [string range [lindex $LineaBreak($x) 7] 0 \
            [expr [string first "_" [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp 
      } 
    } else {
      foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
        set riga [string range $TagBreak [expr [string first "_" $TagBreak] + 1] end]
        if { [lindex $LineaBreak($x) 8] == "REAL" } {
          set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
          set [string range $TagBreak 0 [expr [string first "_" $TagBreak] - 1]]($riga) \
              $temp
        } else {
          set temp [format "%d" [lindex $Risultato $indice_break]]
          set [string range $TagBreak 0 [expr [string first "_" $TagBreak] - 1]]($riga) \
              $temp 
        }
      }
    }
  }
  incr indice_break
  return $indice_break
}

####################################################################
# Procedura AggiornaAccumulatoreOperandoIL
# Procedura che aggiorna il valore dell' accumulatore IL e
# dell' operando
# Input: 
#   w            Nome del widget testo
#   Risultato    Risultato della lista di watch 
#   x            ???
#   indice_break Indice del break
#   ListaWatch   Elenco dei watchpoint impostati
#   colore       colore del foreground per valore 1
#   colore1	 secondo colore foreground per valore 0
# Ritorna :
#   Niente
####################################################################
proc AggiornaAccumulatoreOperandoIL { w Risultato x indice_break ListaWatch colore colore1} {
  global LineaBreak accuwdg opwdg index0 index1 Old_Risultato Old_indice_break_lista \
         indice_break_lista 

  if [info exists Old_Risultato] {
    if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
         $indice_break - $indice_break_lista($ListaWatch)]] == \
         [lindex $Risultato $indice_break] } {
      incr indice_break
      if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
           $indice_break - $indice_break_lista($ListaWatch)]] == \
           [lindex $Risultato $indice_break] } {
        incr indice_break
        return $indice_break
      } else {
        incr indice_break -1
      }
    }
  }

  # operando
  if {[string first tag [lindex $LineaBreak($x) 8]] != -1} {
    if {[format "%d" [lindex $Risultato $indice_break]] >= 1 } {
      $w tag configure [lindex $LineaBreak($x) 8] -foreground $colore -background black
    } elseif {[format "%d" [lindex $Risultato $indice_break]] == 0 } {
      $w tag configure [lindex $LineaBreak($x) 8] -foreground $colore1 -background white
    }
  } else {
    set riga [string range [lindex $LineaBreak($x) 8] [expr [string first "_" \
             [lindex $LineaBreak($x) 8]] + 1] end]
    if { [lindex $LineaBreak($x) 9] == "REAL" } {
      set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
      set [string range [lindex $LineaBreak($x) 8] 0 \
          [expr [string first "_" [lindex $LineaBreak($x) 8]] - 1]]($riga) $temp
    } else {
      set [string range [lindex $LineaBreak($x) 8] 0 \
          [expr [string first "_" [lindex $LineaBreak($x) 8]] - 1]]($riga) \
          [format "%d" [lindex $Risultato $indice_break]]
    }
  }

  incr indice_break

  # accumulatore
  if {[string first tag [lindex $LineaBreak($x) 7]] != -1} {
    if {[format "%d" [lindex $Risultato $indice_break]] >= 1 } {
      # Controllo se c'e' uno o piu' tag associati al risultato
      if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
        $w tag configure [lindex $LineaBreak($x) 7] -foreground $colore -background black
      } else {
        foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
          if {[string first "_N" $TagBreak] == -1} {
            $w tag configure $TagBreak -foreground $colore -background black
          } else {
            # Caso del negato
            $w tag configure [string trimright $TagBreak "_N"] -foreground $colore1 \
               -background white
          }
        }
      }
    } elseif {[format "%d" [lindex $Risultato $indice_break]] == 0 } {
      if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
        $w tag configure [lindex $LineaBreak($x) 7] -foreground $colore1 -background white
      } else {
        foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
          if {[string first "_N" $TagBreak] == -1} {
            $w tag configure $TagBreak -foreground $colore1 -background white
          } else {
            # Caso del negato
            $w tag configure [string trimright $TagBreak "_N"] -foreground $colore \
              -background black
          }
        }
      }
    }
  } else {
    if { [string first ";" [lindex $LineaBreak($x) 7]] == -1 } {
      set riga [string range [lindex $LineaBreak($x) 7] \
               [expr [string first "_" [lindex $LineaBreak($x) 7]] + 1] end]
      if { [lindex $LineaBreak($x) 8] == "REAL" } {
        set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
        set [string range [lindex $LineaBreak($x) 7] 0 \
            [expr [string first "_" [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp
      } else {
        set temp [format "%d" [lindex $Risultato $indice_break]]
        set [string range [lindex $LineaBreak($x) 7] 0 \
            [expr [string first "_" [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp 
      }
    } else {
      foreach TagBreak [split [lindex $LineaBreak($x) 7] ";"] {
        set riga [string range $TagBreak [expr [string first "_" $TagBreak] + 1] end]
        if { [lindex $LineaBreak($x) 8] == "REAL" } {
          set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
          set [string range $TagBreak 0 [expr [string first "_" $TagBreak] - 1]]($riga) \
            $temp
        } else {
          set temp [format "%d" [lindex $Risultato $indice_break]]
          set [string range $TagBreak 0 [expr [string first "_" $TagBreak] - 1]]($riga) \
            $temp 
        }
      }
    }
  }
  incr indice_break

  return $indice_break
}

####################################################################
# Procedura AggiornaAccumulatoreLADDER
# Procedura che aggiorna il valore dell' accumulatore Ladder
# Input: 
#   Risultato    Risultato della lista di watch 
#   x            ???
#   indice_break Indice del break
#   ListaWatch   Elenco dei watchpoint impostati
#   acceso       colore acceso
#   spento       colore spento
# Ritorna :
#   Niente
####################################################################
proc AggiornaAccumulatoreLADDER { Risultato x indice_break ListaWatch acceso spento} {

  global LineaBreak accuwdg opwdg index0 index1 Old_Risultato Old_indice_break_lista 
  global AlberoLadder ContactDef EditPrefs indice_break_lista 

  if [info exists Old_Risultato] {
    if [info exists Old_indice_break_lista($ListaWatch)] {
      if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
           $indice_break - $indice_break_lista($ListaWatch)]] == \
           [lindex $Risultato $indice_break] } {
        incr indice_break
        return $indice_break
      }
    }
  }

  # C'e' solo l'accumulatore
  if { [lindex $LineaBreak($x) 8] == "BOOL" } {
    # Legenda relativa alla variabile ValoreOperando
    #0 var 0 contact 0
    #1 var 1 contact 1
    #2 var 0 contact 1
    #3 var 1 contact 0
    set ValoreOperando [format "%d" [lindex $Risultato $indice_break]]
    set canvas [lindex [lindex $LineaBreak($x) 7] 0]
    set canvasCorrente [lindex [split $canvas .] end]
    set ContattoAssociato [lindex [lindex $LineaBreak($x) 7] 1]
    if {[string first "_N" $ContattoAssociato] != -1} {
      set ContattoAssociato [string trimright $ContattoAssociato "_N"]
      set ValoreOperando [expr $ValoreOperando + 2]
    }
    set ColonnaContatto [lindex [split $ContattoAssociato :] 1]
    set indiceDietro ""
    foreach connection $AlberoLadder($canvasCorrente) {
      if [string match "$ContattoAssociato|*" $connection] {
        set indiceDietro [lappend indiceDietro $connection]
      }
    }

    if { $ValoreOperando == 0 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
          -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 1 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 2 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 3 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    }
  } else {
    set riga [string range [lindex $LineaBreak($x) 7] [expr [string first "_" \
             [lindex $LineaBreak($x) 7]] + 1] end]
    if { [lindex $LineaBreak($x) 8] == "REAL" } {
      set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
    } else {
      set temp [format "%d" [lindex $Risultato $indice_break]]
    }
    set [string range [lindex $LineaBreak($x) 7] 0 [expr [string first "_" \
             [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp 
  } 
  incr indice_break
  return $indice_break
}

####################################################################
# Procedura AggiornaAccumulatoreIL
# Procedura che aggiorna il valore dell' accumulatore e dell' operando
# Ladder
# Input: 
#   Risultato    Risultato della lista di watch 
#   x            ???
#   indice_break Indice del break
#   ListaWatch   Elenco dei watchpoint impostati
#   acceso       colore acceso
#   spento       colore spento
# Ritorna :
#   Niente
####################################################################
proc AggiornaAccumulatoreOperandoLADDER { Risultato x indice_break ListaWatch acceso spento} {
  global LineaBreak accuwdg opwdg index0 index1 Old_Risultato Old_indice_break_lista 
  global AlberoLadder ContactDef EditPrefs indice_break_lista

  if [info exists Old_Risultato] {
    if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
         $indice_break - $indice_break_lista($ListaWatch)]] == \
         [lindex $Risultato $indice_break] } {
      incr indice_break
      if { [lindex $Old_Risultato [expr $Old_indice_break_lista($ListaWatch) + \
           $indice_break - $indice_break_lista($ListaWatch)]] == \
           [lindex $Risultato $indice_break] } {
        incr indice_break
        return $indice_break
      } else {
        incr indice_break -1
      }
    }
  }

  if { [lindex $LineaBreak($x) 9] == "BOOL" } {
    # operando
    set ValoreOperando [format "%d" [lindex $Risultato $indice_break]]
    set canvas [lindex [lindex $LineaBreak($x) 8] 0]
    set canvasCorrente [lindex [split $canvas .] end]
    set ContattoAssociato [lindex [lindex $LineaBreak($x) 8] 1]
    if {[string first "_N" $ContattoAssociato] != -1} {
      set ContattoAssociato [string trimright $ContattoAssociato "_N"]
      set ValoreOperando [expr $ValoreOperando + 2]
    }
    set ColonnaContatto [lindex [split $ContattoAssociato :] 1]
    set indiceDietro ""
    foreach connection $AlberoLadder($canvasCorrente) {
      if [string match "$ContattoAssociato|*" $connection] {
        set indiceDietro [lappend indiceDietro $connection]
      }
    }
    if { $ValoreOperando == 0 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 1 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 2 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 3 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    }
    incr indice_break
    # accumulatore
    set ValoreOperando [format "%d" [lindex $Risultato $indice_break]]
    set canvas [lindex [lindex $LineaBreak($x) 7] 0]
    set canvasCorrente [lindex [split $canvas .] end]
    set ContattoAssociato [lindex [lindex $LineaBreak($x) 7] 1]
    if {[string first "_N" $ContattoAssociato] != -1} {
      set ContattoAssociato [string trimright $ContattoAssociato "_N"]
      set ValoreOperando [expr $ValoreOperando + 2]
    }
    set ColonnaContatto [lindex [split $ContattoAssociato :] 1]
    set indiceDietro ""
    foreach connection $AlberoLadder($canvasCorrente) {
      if [string match "$ContattoAssociato|*" $connection] {
        set indiceDietro [lappend indiceDietro $connection]
      }
    }
    if { $ValoreOperando == 0 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 1 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 2 } {
      $canvas itemconfigure var$ContattoAssociato -fill $spento 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $acceso 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $acceso
        $canvas raise conn$indiceDietroIndex
      }
    } elseif { $ValoreOperando == 3 } {
      $canvas itemconfigure var$ContattoAssociato -fill $acceso 
      $canvas itemconfigure cont$ContattoAssociato \
         -foreground $spento 
      foreach indiceDietroIndex $indiceDietro {
        $canvas itemconfigure conn$indiceDietroIndex -fill $spento
        $canvas lower conn$indiceDietroIndex
      }
    }
    incr indice_break
  } else {
    # operando
    set riga [string range [lindex $LineaBreak($x) 8] [expr [string first "_" \
        [lindex $LineaBreak($x) 8]] + 1] end]
    if { [lindex $LineaBreak($x) 9] == "REAL" } {
      set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
    } else {
      set temp [format "%d" [lindex $Risultato $indice_break]]
    }
    set [string range [lindex $LineaBreak($x) 8] 0 [expr [string first "_" \
        [lindex $LineaBreak($x) 8]] - 1]]($riga) $temp 
    incr indice_break
    # accumulatore
    set riga [string range [lindex $LineaBreak($x) 7] [expr [string first "_" \
        [lindex $LineaBreak($x) 7]] + 1] end]
    if { [lindex $LineaBreak($x) 9] == "REAL" } {
      set temp [ConvReal [format "%d" [lindex $Risultato $indice_break]]]
    } else {
      set temp [format "%d" [lindex $Risultato $indice_break]]
    }
    set [string range [lindex $LineaBreak($x) 7] 0 [expr [string first "_" \
        [lindex $LineaBreak($x) 7]] - 1]]($riga) $temp 
    incr indice_break
  } 
  return $indice_break
}

####################################################################
# Procedura AggiornaJmpLADDER
# Procedura che aggiorna il JMP del  LADDER
# Input: 
#   Risultato    Risultato della lista di watch 
#   x            ???
#   colore       colore da far assumere
# Ritorna :
#   Niente
####################################################################
proc AggiornaJmpLADDER { Risultato x colore} {
global LineaBreak accuwdg opwdg index0 index1 
global AlberoLadder ContactDef 

  set canvas [lindex [lindex $LineaBreak($x) 7] 0]
  set canvasCorrente [lindex [split $canvas .] end]
  set ContattoAssociato [lindex [lindex $LineaBreak($x) 7] 1]
  if {[string first "_N" $ContattoAssociato] != -1} {
    set ContattoAssociato [string trimright $ContattoAssociato "_N"]
  }
  set ColonnaContatto [lindex [split $ContattoAssociato :] 1]
  if { $ColonnaContatto != [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] } {
    set indice [lsearch $AlberoLadder($canvasCorrente) "*|$ContattoAssociato"]
  } else {
    set indice -1
  }
  set indiceST [lsearch $AlberoLadder($canvasCorrente) \
    "$ContattoAssociato|*:[expr $ContactDef($canvasCorrente,col_fine_equazione) -1]"]
  $canvas itemconfigure var$ContattoAssociato -fill $colore 
  $canvas itemconfigure cont$ContattoAssociato -foreground $colore 
  if { $indice != -1 } {
    $canvas itemconfigure conn[lindex $AlberoLadder($canvasCorrente) $indice] -fill $colore
  }
  if { $indiceST != -1 } {
    $canvas itemconfigure conn[lindex $AlberoLadder($canvasCorrente) $indiceST] -fill $colore
  }
  if { [llength $LineaBreak($x)] == 10 } {
    set canvas [lindex [lindex $LineaBreak($x) 8] 0]
    set canvasCorrente [lindex [split $canvas .] end]
    set ContattoAssociato [lindex [lindex $LineaBreak($x) 8] 1]
    if {[string first "_N" $ContattoAssociato] != -1} {
      set ContattoAssociato [string trimright $ContattoAssociato "_N"]
    }
    set ColonnaContatto [lindex [split $ContattoAssociato :] 1]
    if { $ColonnaContatto != [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] } {
      set indice [lsearch $AlberoLadder($canvasCorrente) "*|$ContattoAssociato"]
    } else {
      set indice -1
    }
    set indiceST [lsearch $AlberoLadder($canvasCorrente) \
      "$ContattoAssociato|*:[expr $ContactDef($canvasCorrente,col_fine_equazione) -1]"]
    $canvas itemconfigure var$ContattoAssociato -fill $colore 
    $canvas itemconfigure cont$ContattoAssociato -foreground $colore 
    if { $indice != -1 } {
      $canvas itemconfigure conn[lindex $AlberoLadder($canvasCorrente) $indice] -fill $colore
    }
    if { $indiceST != -1 } {
      $canvas itemconfigure conn[lindex $AlberoLadder($canvasCorrente) $indiceST] \
         -fill $colore
    }
  }
}

####################################################################
# Procedura LeggiVariabili
# Procedura che a tempo va ad interrogare il PLC sullo stato delle
# variabili
# Input: 
#   w            Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc LeggiVariabili { w } {
global EditPrefs LineaBreak Old_Risultato accuwdg opwdg index0 index1 
global AlberoLadder ContactDef Old_indice_break_lista indice_break_lista 
global DebugState ListeOld
  
  # Se il debug non e' piu' attivo fermo tutto
  if { $DebugState != 1 } {
    return
  } 
  # Leggo il valore delle variabili
  set Risultato [watchBreak show]
  if {[lindex $Risultato 0] == "1"} {
    AlertBox -text MsgWatchErrShow
    return
  } elseif {[lindex $Risultato 0] == "2"} {
    StartStopDebug $w
    return
  } elseif {[lindex $Risultato 0] == "3"} {
    AlertBox -text MsgNoAllData
    return
  }

  #Verifichiamo se le liste sono uguali
  if [info exists Old_Risultato] {
    if { $Old_Risultato == $Risultato } {
      set DaAggiornare 0
      foreach Lista [array names ListeOld] { 
        if { $ListeOld($Lista) > 0 } { 
          if { $ListeOld($Lista) < 20 } {
            incr ListeOld($Lista)
          } else {
            set ListeOld($Lista) 0
            set DaAggiornare 1
          }
        }
      }
      if { $DaAggiornare == 0 } {
        after 500 "LeggiVariabili $w"
        return
      }
    }
  }

  if { $EditPrefs(awl) == 1 } {
    if { [lindex $Risultato 0] == "" } {
      $w tag raise watchtag0
      $w tag configure watchtag0 -background white -foreground red 
      after 500 "LeggiVariabili $w"
      return
    }

    set indice_break 0

    foreach Elemento $Risultato {
      if {[string first "Lista" $Elemento] != -1 } {
        set ListaWatch [string trimleft $Elemento "Lista"]
        set ListaWatch [string trimright $Elemento "_o"]
      }
      lappend Organizza($ListaWatch) $Elemento
    }
    set Risultato ""
    foreach x [lsort -increasing [array names Organizza]] {
      set Risultato [concat $Risultato $Organizza($x)]
    }

    foreach x [lsort -increasing [array names LineaBreak]] {
      if {[string first "Lista" [lindex $Risultato $indice_break]] != -1 } {
        set ListaWatch [string trimleft [lindex $Risultato $indice_break] "Lista"]
        if {[string first "_o" $ListaWatch] == -1 } {
          set ListeOld($ListaWatch) 0
          $w tag configure watchtag$ListaWatch -background white -foreground black
          $w tag lower watchtag$ListaWatch
        } else {
          set ListaWatch [string trimright $ListaWatch "_o"]
          set ListeOld($ListaWatch) 1
          $w tag configure watchtag$ListaWatch -background white -foreground tan
          $w tag lower watchtag$ListaWatch
        }
        set indice_break_lista($ListaWatch) $indice_break
        incr indice_break
      }

      if { [lindex $LineaBreak($x) 0] == $ListaWatch } {
        if { $ListeOld($ListaWatch) > 0 } {
          set colore tan
          set colore1 tan
        } else {
          set colore white
          set colore1 black
        }
        if { [llength $LineaBreak($x)] == 9 } {
          set indice_break [AggiornaAccumulatoreIL $w $Risultato $x $indice_break \
             $ListaWatch $colore $colore1]
        } elseif { [llength $LineaBreak($x)] == 10 } {
          set indice_break [AggiornaAccumulatoreOperandoIL $w $Risultato $x \
             $indice_break $ListaWatch $colore $colore1]
        }
      } else {
        set ListaNonPerc [lindex $LineaBreak($x) 0]
        if { $ListeOld($ListaNonPerc) > 0 } { 
          if { $ListeOld($ListaNonPerc) < 20 } {
            incr ListeOld($ListaNonPerc)
          } else {
            set ListeOld($ListaNonPerc) 0
          }
        } else {
          $w tag raise watchtag$ListaNonPerc
          $w tag configure watchtag$ListaNonPerc -background white \
            -foreground red
        }
      }
    }
  } elseif { $EditPrefs(awl) == 0 } { 
    # Caso ladder
    set indice_break 0

    foreach x [lsort -increasing [array names LineaBreak]] {
      if {[string first "Lista" [lindex $Risultato $indice_break]] != -1 } {
        set ListaWatch [string trimleft [lindex $Risultato $indice_break] "Lista"]
        if {[string first "_o" $ListaWatch] == -1 } {
          set ListeOld($ListaWatch) 0
        } else {
          set ListaWatch [string trimright $ListaWatch "_o"]
          set ListeOld($ListaWatch) 1
        }
        set indice_break_lista($ListaWatch) $indice_break
        incr indice_break
      }
      if { [lindex $LineaBreak($x) 0] == $ListaWatch } {
        if { $ListeOld($ListaWatch) > 0 } {
          set acceso tan
          set spento grey
        } else {
          set acceso $EditPrefs(debugcoloracceso)
          set spento $EditPrefs(debugcolorspento)
        }
        if { [llength $LineaBreak($x)] == 9 } {
          set indice_break [AggiornaAccumulatoreLADDER  $Risultato $x $indice_break \
             $ListaWatch $acceso $spento]
        } elseif { [llength $LineaBreak($x)] == 10 } {
          set indice_break [AggiornaAccumulatoreOperandoLADDER  $Risultato $x \
             $indice_break $ListaWatch $acceso $spento]
        }
      } else {
        set ListaNonPerc [lindex $LineaBreak($x) 0]
        if { $ListeOld($ListaNonPerc) > 0 } { 
          if { $ListeOld($ListaNonPerc) < 20 } {
            incr ListeOld($ListaNonPerc)
          } else {
            set ListeOld($ListaNonPerc) 0
          }
        } else {
          AggiornaJmpLADDER $Risultato $x red
        }
      }
    }
  }
  set Old_Risultato $Risultato
  array set Old_indice_break_lista [array get indice_break_lista] 
  after 500 "LeggiVariabili $w"
}

####################################################################
# Procedura InizializzaSimboliDebug 
# Procedura che inizializza gli array per il debug del plc
# Input: 
#   Nomefile  Nome del file in debug
# Ritorna :
#   Niente
####################################################################
proc InizializzaSimboliDebug { nomefile } {
global user_symbol tmp_symbol ind_ass EditPrefs

  # Prima chiamata
  if [info exists tmp_symbol] {
    unset tmp_symbol
  } 
  if [info exists user_symbol] { 
    unset user_symbol
  }

  # Calcolo l'indirizzo assoluto ed il nome del modulo caricato 
  set ind_nome [RicavaIndirizzoAssoluto]
  if { [lindex $ind_nome 0] == "1"} {
    return 1
  }
  set ind_ass [lindex $ind_nome 0]
  set nome_modulo [lindex $ind_nome 1]

  # Catturo gli indirizzi delle variabili nel modulo
  set user_symbol_tmp [tclnm 0 $nome_modulo]
  if { [lindex $user_symbol_tmp 0] == "1"} {
    return 1
  }
  foreach riga_nm $user_symbol_tmp {
    set indice_nm [lindex $riga_nm 2]
    if { $indice_nm != ""} {
      set user_symbol($indice_nm) $riga_nm
    }
  }
  # Catturo i simboli di debug di Beppe
  set tmp_symbol_tmp [tclnm 1 $nome_modulo]
  if { [lindex $tmp_symbol_tmp 0] == "1"} {
    return 1
  }

  # Calcolo l'indirizzo iniziale e finale del programma
  # principale
  set InizioMain 0
  set FineMain 0
  foreach riga_nm $tmp_symbol_tmp {
    set indice_nm [lindex [split $riga_nm "$"] 1]
    if { $indice_nm != ""} {
      if { [string first BF $indice_nm] != -1} {
        set NomeEsadecimale [string trimleft $indice_nm "BF"]
        set NomeAscii ""
        for {set i 0 } { $i < [expr [string length $NomeEsadecimale]/2] } { incr i} {
          set NomeAscii [append NomeAscii [format "%c" [format "%i" \
            0x[string range $NomeEsadecimale [expr $i*2] [expr $i*2 +1]]]]]
        }
        if { [string toupper $NomeAscii] == \
             [string toupper [file tail [file root $nomefile]].plc]} {
          set InizioMain [format "%i" 0x[lindex $riga_nm 0]]
        } 
      }
      if { [string first EF $indice_nm] != -1} {
        set NomeEsadecimale [string trimleft $indice_nm "EF"]
        set NomeAscii ""
        for {set i 0 } { $i < [expr [string length $NomeEsadecimale]/2] } { incr i} {
          set NomeAscii [append NomeAscii [format "%c" [format "%i" \
            0x[string range $NomeEsadecimale [expr $i*2] [expr $i*2 +1]]]]]
        }
        if { [string toupper $NomeAscii] == \
             [string toupper [file root [file tail $nomefile]].plc] } {
          set FineMain [format "%i" 0x[lindex $riga_nm 0]]
        } 
      }
    }
  }

  #metto la tabella dei simboli sotto forma di vettore
  foreach riga_nm $tmp_symbol_tmp {
    set indice_nm [lindex [split $riga_nm "$"] 1]
    if { $indice_nm != ""} {
      set IndirizzoLinea [format "%i" 0x[lindex $riga_nm 0]]
      if { $IndirizzoLinea >= $InizioMain && $IndirizzoLinea <= $FineMain } {
        if { [string first VI $indice_nm] == -1} { 
          set tmp_symbol($indice_nm) $riga_nm
        } else {
          set tmp_symbol($indice_nm:[string index [lindex $riga_nm 2] \
            [expr [string length [lindex $riga_nm 2]] - 1]]) $riga_nm
        }
      }
    }
  }
  return 0
}

####################################################################
# Procedura ResetDebug
# Procedura che disattiva il debug del PLC
# Input: 
#   t            Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ResetDebug { t } {
global DebugState AttivatoDebug

  set DebugState 0
  ChiudiPLC
  WaitWindow $t 0
  set AttivatoDebug 0
}

####################################################################
# Procedura DebugProgram
# Procedura che inizializza le variabili per il debug del plc
# la chiamata e' 0 se proviene dal menu 1 se dai tasti
# Input: 
#   t            Nome del widget di testo
#   args         0 se proviene dal menu 1 se dai tasti
# Ritorna :
#   Niente
####################################################################
proc DebugProgram { t args } {
global InstructionList DebugState LineaBreak ByteLineeDebug
global accuwdg opwdg index0 index1 EditPrefs ListaWatch OldWdg
global ModiFile DebugNoWidget ListaWidgetTag LadderWidget
global WindowType ContactDef Old_Risultato
global user_symbol tmp_symbol ind_ass AttivatoDebug 
global MainOfFunctionBlock 
  
  if { $AttivatoDebug == 1 } {
    set DebugState [expr !$DebugState]
    SetStartCycle -1
    return
  } else {
    set AttivatoDebug 1
  }

  if { $ModiFile==1 } {
    AlertBox -text MsgCompile
    set DebugState 0
    set AttivatoDebug 0
    return
  }

  if { [GetFilename $t] == "" } {
    set DebugState 0
    set AttivatoDebug 0
    return
  }

  if [info exists Old_Risultato] {
    unset Old_Risultato
  }

  if { $DebugState == 1 } {

  # Calcolo della prima linea e dell'ultima linea visualizzata
  if { $EditPrefs(awl) == 1 } {
    # Caso IL
    set endindex [split [$t index end] {.}]
    set line [expr [lindex $endindex 0] -1]
    set text_view [$t yview]
    set inizio_testo [expr round([lindex $text_view 0] * $line ) + 1]
    set fine_testo [expr round([lindex $text_view 1] * $line )]
    set canvas ""
  } else {
    # Caso ladder
    set canvas [TextToTop $t].main.ladd 
    if { $ContactDef(MaxSegment) < [expr $ContactDef(SegmentoCorrente) + \
         $EditPrefs(eqnum)] } {
      set ContactDef(FineDebug) $ContactDef(MaxSegment) 
    } else {
      set ContactDef(FineDebug) [expr $ContactDef(SegmentoCorrente) + \
         $EditPrefs(eqnum) -1]
    }

    for { set i $ContactDef(SegmentoCorrente) } { $i <= $ContactDef(FineDebug)} \
        { incr i} {
      global LINE_CONTACT[lindex $ContactDef(ordine_seg) $i]
    }
    set inizio_testo [lindex $ContactDef(linee_equazioni) $ContactDef(SegmentoCorrente)]
    set fine_testo [lindex $ContactDef(linee_equazioni) [expr $ContactDef(FineDebug) + 1]] 
    set canvas [TextToTop $t].main.ladd.[lindex $ContactDef(ordine_seg) \
        $ContactDef(SegmentoCorrente)]
    set QualeSegment [expr $ContactDef(SegmentoCorrente) + 1]
  }

    if { [lindex $args 0] == 1 } {
      # Verifico se il PLC c'e'
      if {[VerificaEsistenzaPlc]} { 
        AlertBox -text MsgNoPlc
	set DebugState 0
        set AttivatoDebug 0
	return
      } 
      # Apertura del PLC
      ApriPLC
    }
    set ListaWatch 0
    # Nel caso di debug eseguo le seguenti operazioni
    WaitWindow $t 1

    if { [lindex $args 0] == 1 } {

      if { $WindowType([TextToTop $t])=="main" } {
        set NomeDelFile [GetFilename $t]
      } else {
        set NomeDelFile [GetFilename [TopToText $MainOfFunctionBlock([TextToTop $t])]]
      }
      if {[InizializzaSimboliDebug $NomeDelFile]} {
        AlertBox -text MsgInitSimbolErr
        ResetDebug $t
        return
      }

      # Verifico la correttezza del CRC
      if {[CRCcorretto $ind_ass $t] == 1 || [CRCcorretto $ind_ass $t] == 2 } {
        if {[CRCcorretto $ind_ass $t] == 1 } {
          AlertBox -text MsgNoCRC
        } elseif {[CRCcorretto $ind_ass $t] == 2} {
          AlertBox -text MsgCRCCalcErr
        }
        ResetDebug $t
        return
      }

      if { $WindowType([TextToTop $t])=="fb" } {
	set ListaWatch [AggiungiBreakLineaCondizionata $t $ind_ass]
        if { $ListaWatch == "-1" } {
          AlertBox -text MsgWatchErrCreateCond
          ResetDebug $t
          return
        } elseif { $ListaWatch == "-2" } {
          ResetDebug $t
          return
        }
      }
    }

    #disabilito i Widget che non servono durante il debug
    for {set i 0} {$i < [llength $DebugNoWidget]} {incr i} {
      set indice_ [string first ":" [lindex $DebugNoWidget $i]]
      if { $indice_ == -1 } {
        if [winfo exist [lindex $DebugNoWidget $i]] {
          [lindex $DebugNoWidget $i] configure -state disabled
        }
      } else {
        if [winfo exist [string range [lindex $DebugNoWidget $i] 0 [expr $indice_ - 1]]] {
          [string range [lindex $DebugNoWidget $i] 0 [expr $indice_ - 1]] entryconfigure \
          [string range [lindex $DebugNoWidget $i] [expr $indice_ + 1] end] \
          -state disabled
        }
      }
    }
    $t configure -state disabled

    # Resetto la situazione precedente
    # Creo la lista di break
    if { $WindowType([TextToTop $t])=="main" } {
      # resetto la situazione precedente
      set ResetLista [watchBreak reset]
      if {$ResetLista == "1" } {
        ResetDebug $t
        AlertBox -text MsgWatchErrReset
        return
      } elseif {$ResetLista == "2" } {
        ResetDebug $t
        return
      }
      set ListaWatch [watchBreak create $ListaWatch]
      if { $ListaWatch == "-1" } {
        ResetDebug $t
        AlertBox -text MsgWatchErrCreate
        return
      } elseif { $ListaWatch == "-2" } {
        ResetDebug $t
        return
      }
    } else {
      set ListaWatch [watchBreak create $ListaWatch ?log or opc=1]
      if { $ListaWatch == "-1" } {
        ResetDebug $t
        AlertBox -text MsgWatchErrCreate
        return
      } elseif { $ListaWatch == "-2" } {
        ResetDebug $t
        return
      }
    }
    # Definisco le righe per i tag dei jmp
    set JmpTag($ListaWatch) [list $inizio_testo]
    # Resetto la variabile che tiene traccia dell'accumulatore della riga precedente
    set OldWdg -1
    # deselect any already-selected text:
    catch {$t tag remove sel sel.first sel.last}
    if [info exist ListaWidgetTag] {
      unset ListaWidgetTag
    }

    for {set i $inizio_testo} {$i <= $fine_testo} {incr i} {
      # Cerco gli indici di fine e inizio parola per ogni riga
      set linea_corrente [$t get $i.0 $i.end]
      # Escludo le linee con commento
      if { [lindex $linea_corrente 0] == "(*" } {
        continue
      }
      set j 0
      set parola 0
      while { $j < [string length $linea_corrente]} {
        if { [string range $linea_corrente $j $j] != "\t" && \
             [string range $linea_corrente $j $j] != " " } {
          set indici_parole($parola) $j
          while { $j < [string length $linea_corrente] && \
                  [string range $linea_corrente $j $j] != "\t" && \
                  [string range $linea_corrente $j $j] != " "} {
            incr j
          }
          set indici_parole($parola) [lappend indici_parole($parola) $j]
          incr parola
        } else {
          incr j
        }
      }

      # Verifico se nella linea corrente c'e' una istruzione
      set index_istruzione -1
      # Se la linea non e' vuota
      if { [lindex $linea_corrente 0] != "" } {
        foreach x [array names InstructionList] {
          if { [lsearch -exact $InstructionList($x) [lindex $linea_corrente 0]] != -1 } {
            set index_istruzione 0
          } elseif {[lsearch -exact $InstructionList($x) \
                    [lindex $linea_corrente 1]] != -1 } {
            set index_istruzione 1
          } 
        }
      }

      # Caso della etichetta 
      if {[string match *: [lindex $linea_corrente 0]] != 0} { 
        set JmpTag($ListaWatch) [lappend JmpTag($ListaWatch) [expr $i -1]]
        if { $WindowType([TextToTop $t])=="main" } {
          set ListaWatch [watchBreak create [expr $ListaWatch + 1]]
          if { $ListaWatch == "-1" } {
            ResetDebug $t
            AlertBox -text MsgWatchErrCreate
            return
          } elseif { $ListaWatch == "-2" } {
            ResetDebug $t
            return
          }
        } else {
          set ListaWatch [watchBreak create [expr $ListaWatch + 1] ?log or opc=1]
          if { $ListaWatch == "-1" } {
            ResetDebug $t
            AlertBox -text MsgWatchErrCreate
            return
          } elseif { $ListaWatch == "-2" } {
            ResetDebug $t
            return
          }
        }
        set JmpTag($ListaWatch) [list $i]
      }

      # Se c'e' l'istruzione
      if { $index_istruzione != -1 } {
        if { $EditPrefs(awl) == 0 } {
          if { $i == [lindex $ContactDef(linee_equazioni) $QualeSegment] && \
               $i != $fine_testo } {
            set canvas [TextToTop $t].main.ladd.[lindex $ContactDef(ordine_seg) \
                $QualeSegment]
            incr QualeSegment
          }
        # Se l'operando e' una variabile aggiungo il break a seconda del tipo di variabile
        set TipoVariabile [VerificaTipoVariabile [lindex $linea_corrente \
            [expr $index_istruzione + 1]] [lindex $linea_corrente $index_istruzione] \
            [expr $QualeSegment -1 ] $t]
        } else {
        # Se l'operando e' una variabile aggiungo il break a seconda del tipo di variabile
        set TipoVariabile [VerificaTipoVariabile [lindex $linea_corrente \
           [expr $index_istruzione + 1]] [lindex $linea_corrente $index_istruzione] "" ""]
        }
        if {$TipoVariabile != 0 } {
          if {$TipoVariabile == "BOOL"} {
            AggiungiBreakLinea $t $canvas $ListaWatch $i $ind_ass $TipoVariabile \
       	         [lindex $indici_parole($index_istruzione) 0] \
	         [lindex $indici_parole($index_istruzione) 1] \
                 [lindex $indici_parole([expr $index_istruzione + 1]) 0] \
                 [lindex $indici_parole([expr $index_istruzione + 1]) 1] \
                 [lindex $linea_corrente $index_istruzione] \
                 [lindex $linea_corrente [expr $index_istruzione + 1]] 
          } elseif {$TipoVariabile == "INT" || $TipoVariabile == "DINT" } {
            AggiungiBreakLinea $t $canvas $ListaWatch $i $ind_ass $TipoVariabile \
                  -1 -1 -1 -1 [lindex $linea_corrente $index_istruzione] \
                  [lindex $linea_corrente [expr $index_istruzione + 1]]
          } elseif {$TipoVariabile == "UINT" || $TipoVariabile == "UDINT" } {
            AggiungiBreakLinea $t $canvas $ListaWatch $i $ind_ass $TipoVariabile \
                  -1 -1 -1 -1 [lindex $linea_corrente $index_istruzione] \
                  [lindex $linea_corrente [expr $index_istruzione + 1]]
          } elseif {$TipoVariabile == "REAL" || $TipoVariabile == "LREAL" } {
            AggiungiBreakLinea $t $canvas $ListaWatch $i $ind_ass $TipoVariabile \
                  -1 -1 -1 -1 [lindex $linea_corrente $index_istruzione] \
                  [lindex $linea_corrente [expr $index_istruzione + 1]]
          }
        }
        # Se l'istruzione e' al secondo posto (preceduta da label)
        # oppure se e' un jmp cambio lista di break
        set nome_istruzione [lindex $linea_corrente $index_istruzione ]
  	if { [string match "JMP" $nome_istruzione] || \
             [string match "JMPC" $nome_istruzione] || \
             [string match "JMPCN" $nome_istruzione] } {
          set JmpTag($ListaWatch) [lappend JmpTag($ListaWatch) $i]
          if { $WindowType([TextToTop $t])=="main" } {
            set ListaWatch [watchBreak create [expr $ListaWatch + 1]]
            if { $ListaWatch == "-1" } {
              ResetDebug $t
              AlertBox -text MsgWatchErrCreate
              return
            } elseif { $ListaWatch == "-2" } {
              ResetDebug $t
              return
            }
          } else {
            set ListaWatch [watchBreak create [expr $ListaWatch + 1] ?log or opc=1]
            if { $ListaWatch == "-1" } {
              ResetDebug $t
              AlertBox -text MsgWatchErrCreate
              return
            } elseif { $ListaWatch == "-2" } {
              ResetDebug $t
              return
            }
          }
          set JmpTag($ListaWatch) [list [expr $i + 1]]
  	}
      } else {
        # Caso della riga con una istruzione MD_
        if { [string first "MD_" $linea_corrente] != -1 } {
          set CercaTipo [split $linea_corrente ","]
          set CercaTipo [lindex [lindex $CercaTipo end] 0]
          if { [string toupper $CercaTipo] == "'F'" } {
            set TipoVariabile REAL
          } else {
            set TipoVariabile DINT
          }
           AggiungiBreakLinea $t $canvas $ListaWatch $i $ind_ass $TipoVariabile \
                  -1 -1 -1 -1 [lindex $linea_corrente 0] \
                  [lindex $linea_corrente 1]
        }
      }
    }
    foreach x [lsort -increasing [array names LineaBreak]] {
      set BreakAdd [watchBreak add [lindex $LineaBreak($x) 0] $x \
         [lindex $LineaBreak($x) 1] [lindex $LineaBreak($x) 2]  \
         [lindex $LineaBreak($x) 3] [lindex $LineaBreak($x) 4] \
         [lindex $LineaBreak($x) 5] [lindex $LineaBreak($x) 6] +log]
      if { $BreakAdd == "1" } {
        ResetDebug $t
        AlertBox -text MsgWatchErrAdd
        return
      } elseif { $BreakAdd == "2" } {
        ResetDebug $t
        return
      }
#puts $LineaBreak($x)
#puts "watchBreak add [lindex $LineaBreak($x) 0] $x [lindex $LineaBreak($x) 1] [lindex $LineaBreak($x) 2] [lindex $LineaBreak($x) 3] [lindex $LineaBreak($x) 4] [lindex $LineaBreak($x) 5] [lindex $LineaBreak($x) 6]"
    }
    # Finisco di aggiornare i tag 
    set JmpTag($ListaWatch) [lappend JmpTag($ListaWatch) $fine_testo]
    # Abilito le liste di break
    for { set i 0 } { $i <= $ListaWatch } {incr i } {
      if [watchBreak enable $i] {
        #ResetDebug $t
        #AlertBox -text MsgWatchErrEnable
        #return
      }
      # setto i tag
      if { [info exist JmpTag($i)] == 1 } {
        $t tag add watchtag$i [lindex $JmpTag($i) 0].0 [lindex $JmpTag($i) 1].end
      }
    }
    # Distruggo i messaggi di calcolo break
    WaitWindow $t 0
    # lancio la funzione di debug

    MemorizzaCicloIniziale

    if { $EditPrefs(awl) == 1 } {
      # Caso IL
      if { !([array names LineaBreak] == "" && $DebugState == 1) } {
        after 500 "LeggiVariabili $t"
      }
    } else {
      if { !([array names LineaBreak] == "" && $DebugState == 1) } {
        after 500 "LeggiVariabili $canvas"
      }
    }
  } else {
    # riabilito i Widget che non servono durante il debug
    for {set i 0} {$i < [llength $DebugNoWidget]} {incr i} {
      set indice_ [string first ":" [lindex $DebugNoWidget $i]]
      if { $indice_ == -1 } {
        if [winfo exist [lindex $DebugNoWidget $i]] {
          [lindex $DebugNoWidget $i] configure -state normal
        }
      } else {
        if [winfo exist [string range [lindex $DebugNoWidget $i] 0 [expr $indice_ - 1]]] {
          [string range [lindex $DebugNoWidget $i] 0 [expr $indice_ - 1]] entryconfigure \
          [string range [lindex $DebugNoWidget $i] [expr $indice_ + 1] end] -state normal
        }
      }
    }

    if { $EditPrefs(awl) == 0 } {
      #disabilito i Widget che non servono per il ladder in caso di modalita' 
      # ladder
      for {set i 0} {$i < [llength $LadderWidget]} {incr i} {
        set indice_ [string first ":" [lindex $LadderWidget $i]]
        if { $indice_ != -1 } {
          if [winfo exist [string range [lindex $LadderWidget $i] 0 \
             [expr $indice_ - 1]]] {
            [string range [lindex $LadderWidget $i] 0 [expr $indice_ - 1]] \
            entryconfigure [string range [lindex $LadderWidget $i] \
            [expr $indice_ + 1] end] -state disabled
          }
        }
      }
    }

    $t configure -state normal
 
    if { $EditPrefs(awl) == 1 } {
      if [info exists ListaWidgetTag] {  
        set LineaIniziale [lindex [split [$t index end] .] 0]
        set LineaFinale 0
        for {set i 0} { $i < [llength $ListaWidgetTag] } {incr i} {
          if { [string first accutag [lindex $ListaWidgetTag $i]] != -1 || \
               [string first optag [lindex $ListaWidgetTag $i]] != -1 } {
            $t tag configure [lindex $ListaWidgetTag $i] -foreground black \
              -background white
            $t tag delete [lindex $ListaWidgetTag $i]
          } elseif { [string first accuwdg [lindex $ListaWidgetTag $i]] != -1 || \
  	             [string first opwdg [lindex $ListaWidgetTag $i]] != -1 || \
                     [string first index0 [lindex $ListaWidgetTag $i]] != -1 || \
                     [string first index1 [lindex $ListaWidgetTag $i]] != -1 || \
                     [string first label [lindex $ListaWidgetTag $i]] != -1 } {
            set LineaCorrente [lindex [split [lindex $ListaWidgetTag $i] _] 1]
            if { $LineaCorrente < $LineaIniziale } {
              set LineaIniziale $LineaCorrente
            }
            if { $LineaCorrente > $LineaFinale } {
              set LineaFinale $LineaCorrente
            }
            destroy [lindex $ListaWidgetTag $i]
          }
        }
        set IndiciDaCancellare ""
        foreach finestra [$t dump -window $LineaIniziale.0 $LineaFinale.end] {
          if { [string first "." $finestra] != -1 } {
            set IndiciDaCancellare [concat $finestra $IndiciDaCancellare]
          }
        }
        foreach finestra $IndiciDaCancellare {
          $t delete $finestra
        }
      }
    } else {
      for { set i $ContactDef(SegmentoCorrente) } { $i <= $ContactDef(FineDebug)} \
          { incr i} {
        set canvas [TextToTop $t].main.ladd.[lindex $ContactDef(ordine_seg) $i]
        set canvaswidth [$canvas configure -width]
        set canvasheight [$canvas configure -height]
        foreach x [$canvas find enclosed 0 0 [lindex $canvaswidth 4] \
                  [expr [lindex $canvasheight 4] + $ContactDef(height_conn)]] {
          if { [$canvas type $x] == "bitmap" } {
            $canvas itemconfigure $x -foreground black
          } elseif { [$canvas type $x] == "window" } { 
            destroy [lindex [$canvas itemconfigure $x -window] 4]
            $canvas delete $x
          } elseif { [$canvas type $x] != "rectangle" } { 
            $canvas itemconfigure $x -fill black
          }
        }
      }
    }

    if [info exist accuwdg] {
        unset accuwdg 
      }
      if [info exist opwdg] {
        unset opwdg
      }
      if [info exist index0] {
        unset index0
      }
      if [info exist index1] {
        unset index1
      }

    for { set i 0 } { $i <= $ListaWatch } {incr i } {
      # cancello i tag
      $t tag delete watchtag$i 
    }
    set BreakReset [watchBreak reset]
    if { $BreakReset == "1" } {
      ResetDebug $t
      AlertBox -text MsgWatchErrReset
      return
    } elseif { $BreakReset == "2" } {
      ResetDebug $t
      return
    }
    if { [lindex $args 0] == 1 } {
      #Chiusura PLC
      ChiudiPLC
    }
    if [info exists LineaBreak] {
      unset LineaBreak
    }
    if [info exists ByteLineeDebug] {
      unset ByteLineeDebug
    }
  }
  set AttivatoDebug 0
}

####################################################################
# Procedura CreaAccuTag
# Procedura che crea il tag per l'accumulatore
# Input: 
#   t            Nome del widget di testo
#   linea        Linea in cui creare il tag
#   AccuInizio   Carattere iniziale in cui posizionare il tag
#   AccuFine     Carattere finale in cui posizionare il tag
#   ListaWatch   Elenco dei watchpoint impostati
# Ritorna :
#   Niente
####################################################################
proc CreaAccuTag { t Linea AccuInizio AccuFine } {
global ListaWidgetTag

  $t tag add accutag$Linea $Linea.$AccuInizio $Linea.$AccuFine
  set ListaWidgetTag [lappend ListaWidgetTag accutag$Linea]
}

####################################################################
# Procedura CreaOpTag 
# Procedura che crea il tag per l'operando
# Input: 
#   t            Nome del widget di testo
#   Linea        Linea in cui creare il tag
#   OpInizio     Carattere iniziale in cui posizionare il tag
#   OpFine       Carattere finale in cui posizionare il tag
#   ListaWatch   Elenco dei watchpoint impostati
# Ritorna :
#   Niente
####################################################################
proc CreaOpTag { t Linea OpInizio OpFine } {
global ListaWidgetTag

  $t tag add optag$Linea $Linea.$OpInizio $Linea.$OpFine
  set ListaWidgetTag [lappend ListaWidgetTag optag$Linea]
}

##############################################################################
# Procedura che crea il widget di etichetta per separare
##############################################################################
####################################################################
# Procedura CreaDebugWidget
# Procedura che crea il widget di etichetta per separare
# Input: 
#   t            Nome del widget di testo
#   Linea        Linea in cui creare il tag
# Ritorna :
#   Niente
####################################################################
proc CreaDebugWidget { t Linea } {
global accuwdg opwdg index0 index1 ListaWidgetTag EditPrefs

  # Creo etichetta
  set ContenutoLinea [$t get $Linea.0 $Linea.end]
  set LunghezzaLinea 0
  set TmpIndice 0 
  # Calcolo il valore assoluto della lunghezza della linea risolvendo
  # i tab assumendoli di 8 caratteri
  for {set i 0 } { $i < [string length $ContenutoLinea] } {incr i} {
    if { [string range $ContenutoLinea $i $i] == "\t" } {
      set LunghezzaLineaPrimaTab $LunghezzaLinea
      set LunghezzaLinea [expr (($LunghezzaLinea / $EditPrefs(tablength)) + 1) * \
         $EditPrefs(tablength) ] 
    } else {
      incr LunghezzaLinea
    }
    if { $LunghezzaLinea >= $EditPrefs(iniziowidget) && $TmpIndice == 0 } {
      if { [string range $ContenutoLinea $i $i] == "\t" } {
        set widthlabel [expr $EditPrefs(iniziowidget) - $LunghezzaLineaPrimaTab + \
          $EditPrefs(defaultlabelsize)]
        set TmpIndice $i
      } else {
        set widthlabel $EditPrefs(defaultlabelsize)
        set TmpIndice [expr $i + 1]
      }
    }
  }

  if { $TmpIndice == 0 } {
    # non ci sono commenti in zona debug
    label $t.label_$Linea -width [expr $EditPrefs(iniziowidget) + \
        $EditPrefs(defaultlabelsize) - $LunghezzaLinea] -background white \
        -font $EditPrefs(textfont)
    $t window create $Linea.end -window $t.label_$Linea
  } else {
    label $t.label_$Linea -width $widthlabel -background white -font $EditPrefs(textfont)
    set IndiceInizioWidget $TmpIndice
    $t window create $Linea.$IndiceInizioWidget  -window $t.label_$Linea
  }
  set ListaWidgetTag [lappend ListaWidgetTag $t.label_$Linea]


  # Creo il widget per accumulatore
  entry $t.accuwdg_$Linea -width $EditPrefs(defaultwidgetsize) -justify right \
      -state disabled -background skyblue -highlightthickness 0 \
      -bd 0 -font $EditPrefs(textfont) -textvariable accuwdg($Linea)
  if { $TmpIndice == 0 } {
    $t window create $Linea.end -window $t.accuwdg_$Linea
  } else {
    $t window create $Linea.[expr $IndiceInizioWidget + 1] -window $t.accuwdg_$Linea
  }
  set accuwdg($Linea) -
  set ListaWidgetTag [lappend ListaWidgetTag $t.accuwdg_$Linea]

  # Creo widget per operando
  entry $t.opwdg_$Linea -width $EditPrefs(defaultwidgetsize) -justify right \
      -state disabled -background mediumturquoise -highlightthickness 0 \
      -bd 0 -font $EditPrefs(textfont) -textvariable opwdg($Linea)
  if { $TmpIndice == 0 } {
    $t window create $Linea.end -window $t.opwdg_$Linea
  } else {
    $t window create $Linea.[expr $IndiceInizioWidget + 2] -window $t.opwdg_$Linea
  }
  set opwdg($Linea) -
  set ListaWidgetTag [lappend ListaWidgetTag $t.opwdg_$Linea]

  # Creo il widget per il primo indice
  entry $t.index0_$Linea -width $EditPrefs(defaultwidgetsize) -justify right \
       -state disabled -background mediumseagreen -highlightthickness 0 \
       -bd 0 -font $EditPrefs(textfont) -textvariable index0($Linea)
  if { $TmpIndice == 0 } {
    $t window create $Linea.end -window $t.index0_$Linea
  } else {
    $t window create $Linea.[expr $IndiceInizioWidget + 3] -window $t.index0_$Linea
  }
  set index0($Linea) -
  set ListaWidgetTag [lappend ListaWidgetTag $t.index0_$Linea]

  # Creo widget per il secondo indice
  entry $t.index1_$Linea -width $EditPrefs(defaultwidgetsize) -justify right \
      -state disabled -background lightseagreen -highlightthickness 0 \
      -bd 0 -font $EditPrefs(textfont) -textvariable index1($Linea)
  if { $TmpIndice == 0 } {
    $t window create $Linea.end -window $t.index1_$Linea
  } else {
    $t window create $Linea.[expr $IndiceInizioWidget + 4] -window $t.index1_$Linea
  }
  set index1($Linea) -
  set ListaWidgetTag [lappend ListaWidgetTag $t.index1_$Linea]
}

####################################################################
# Procedura CreaDebugWidgetLadder
# Procedura che crea i widget per il debug del ladder nel caso
# di contatto non booleano
# Input: 
#   canvas       Nome del widget canvas
#   contatto     Contatto da inserire
#   Linea        ???
# Ritorna :
#   Niente
####################################################################
proc CreaDebugWidgetLadder {canvas contatto Linea} {
global EditPrefs ContactDef accuwdg opwdg

  set riga [lindex [split $contatto ":"] 0]
  set colonna [lindex [split $contatto ":"] 1]
  # Creo il widget per accumulatore
  entry $canvas.accuwdg_$contatto -width $ContactDef(defaultwidgetsize) -justify right \
     -state disabled -background skyblue -highlightthickness 0 \
     -bd 0 -textvariable accuwdg($Linea)

  # Creo widget per operando
  entry $canvas.opwdg_$contatto -width $ContactDef(defaultwidgetsize) -justify right \
     -state disabled -background mediumturquoise -highlightthickness 0 \
     -bd 0 -textvariable opwdg($Linea)

  $canvas create window [expr $ContactDef(offset_x) + \
     ($ContactDef(width_op)*($colonna + 1)) + ($ContactDef(width_conn)* $colonna)] \
     [expr $ContactDef(offset_y)/2 + $ContactDef(offset_x_text) + \
     $ContactDef(height_conn) * $riga] -anchor nw \
     -window $canvas.accuwdg_$contatto -tag accuwdg_$contatto

  $canvas create window [expr $ContactDef(offset_x) + \
     ($ContactDef(width_op)*($colonna + 1)) + ($ContactDef(width_conn)* $colonna)] \
     [expr 2.2*$ContactDef(offset_y) + $ContactDef(offset_x_text) + \
     $ContactDef(height_conn) * $riga] -anchor nw \
     -window $canvas.opwdg_$contatto -tag opwdg_$contatto

  # inizializzo le variabili con un trattino
  set accuwdg($Linea) -
  set opwdg($Linea) -
}

####################################################################
# Procedura VerificaTipoVariabileParametroFB
# Procedura che verifica il tipo di variabile del parametro di un FB
# Input: 
#   variabile    Variavbile da verificare
# Ritorna :
#   Niente
####################################################################
proc VerificaTipoVariabileParametroFB { variabile } {
global FunctionBlockList Home

  set ListaFilePrototipi [list $Home/util/illib.ilc \
      [lsort "[glob -nocomplain $Home/util/*.h]"]]
  set Blocco [lindex [split $variabile .] 0]
  set Parametro [lindex [split $variabile .] 1]

  foreach i $ListaFilePrototipi {
    if { $i != "" } {
      if [catch { set fId [open $i r] } result] {
        AlertBox -text $result
        continue
      }
      set TuttoIlFile [read -nonewline $fId]
    
      set ind_FB [string first $FunctionBlockList($Blocco) $TuttoIlFile]
      if { $ind_FB != -1 } {
        set TuttoIlFile [string range $TuttoIlFile $ind_FB end]
        set ind_par [string first $Parametro $TuttoIlFile] 
        if { $ind_par != -1 } {
          set TuttoIlFile [string range $TuttoIlFile $ind_par end]
          set linea [string range $TuttoIlFile 0 [string first "\n" $TuttoIlFile]]
          set PuntoVirgola [string first ";" $linea ]
          if { $PuntoVirgola != -1 } {
            set linea [string range $linea 0 $PuntoVirgola]
          }
          set Tipo [string trimright [lindex "$linea" 2] ";"]
          switch [string toupper $Tipo] {
            BOOL {close $fId; return BOOL}
            INT {close $fId; return INT}
            UINT {close $fId; return INT}
            DINT {close $fId; return INT}
            UDINT {close $fId; return INT}
            REAL {close $fId; return REAL}
            LREAL {close $fId; return REAL}
            TIME {close $fId; return INT}
            DATE {close $fId; return INT}
            STRING {close $fId; return STRING}
            BYTE {close $fId; return INT}
            WORD {close $fId; return INT}
            DWORD {close $fId; return INT}
            default {close $fId; return INT}
          }
        }
      }
      close $fId
    }
  }
  return INT
}

####################################################################
# Procedura VerificaTipoEquazione
# Procedura che verifica se in una equazione con GENERAL c'e' un BOOL
# Input: 
#   equazione    Equazione da esaminare
# Ritorna :
#   Niente
####################################################################
proc VerificaTipoEquazione { equazione } {
global InstructionList VariableList

  foreach linea [split $equazione "\n"] {
    # Verifico se nella linea corrente c'e' una istruzione
    set index_istruzione -1
    # Se la linea non e' vuota
    if { [lindex $linea 0] != "" } {
      foreach x [array names InstructionList] {
        if { [lsearch -exact $InstructionList($x) [lindex $linea 0]] != -1 } {
          set index_istruzione 0
        } elseif {[lsearch -exact $InstructionList($x) [lindex $linea 1]] != -1 } {
          set index_istruzione 1
        } 
      }
    }
    if { $index_istruzione != -1 } {
      set istruzione [lindex $linea $index_istruzione]
      set variabile [lindex $linea [incr index_istruzione]]
      if { [string first "GENERAL" $variabile] == -1 } {
        set Tipo [VerificaTipoVariabile $variabile $istruzione "" ""]
        if {$Tipo == "BOOL" } {
           return BOOL
        }
      }
    }
  }
  return INT
}

####################################################################
# Procedura VerificaTipoVariabile
# Procedura che verifica il tipo di variabile
# Input: 
#   variabile      Variabile da esasminare
#   istruzione     Istruzione associata alla variabile
#   CanvasCorrente Canvas corrente
#   t              Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc VerificaTipoVariabile { variabile istruzione CanvasCorrente t} {
global VariableList FunctionBlockList input input2 EditPrefs ContactDef
  
    # Gestione del segnale di scambio GENERAL nel caso ladder
  if { $EditPrefs(awl) == 0 && $t != ""} {
    if { [string first "GENERAL" $variabile] != -1 } {
      set Equazione [$t get [lindex $ContactDef(linee_equazioni) $CanvasCorrente].0 \
           [expr [lindex $ContactDef(linee_equazioni) [expr $CanvasCorrente +1]] -1].end] 
      return [VerificaTipoEquazione $Equazione]
    }
  }

  # Se 0 o 1 assumiamo che sia booleana
  if { ($variabile == 0 || $variabile == 1) && ([string match {*.*} $variabile] == 0) } {
    return BOOL
  }
  set aritmetica 0

  # Se l'istruzione e' aritmetica
  if {[string match "ADD" $istruzione] || \
          [string match "SUB" $istruzione] || \
          [string match "MUL" $istruzione] || \
          [string match "DIV" $istruzione] } {
    set aritmetica 1
  }

  # Se l'istruzione e' un jmp ritorna INT
  # Le informazioni di debug vengono catturate con $LJ 
  if {[string match "JMP" $istruzione] || \
          [string match "JMPC" $istruzione] || \
          [string match "JMPCN" $istruzione] } {
    return INT
  }

  # Verifico se e' una costante di tempo o una data
  if {[string match {T\#*} $variabile] || \
      [string match {t\#*} $variabile] || \
      [string match {TIME\#*} $variabile] || \
      [string match {time\#*} $variabile] || \
      [string match {D\#*} $variabile] || \
      [string match {d\#*} $variabile] || \
      [string match {date\#*} $variabile] || \
      [string match {DATE\#*} $variabile] } {
    return INT
  }

  # Se la seconda parola dopo l'istruzione inizia con una lettera vado a fare tutte 
  # le ricerche del caso
  if {[string match {[^a-zA-Z]*} $variabile]} {
   
     # Verifico se si tratta di array ed elimino le quadre []
     set var_array [string first "\[" $variabile]
     if {$var_array  != -1 } {
       set var_range [string range $variabile  $var_array end]
       if [string match {*[a-zA-Z]*} $var_range] {
         set var_range index
       }
       set variabile [string range $variabile  0 [expr $var_array -1]]
     } 

     # Verifico se non rientra nel caso delle due costanti
     if {$variabile == "TRUE" || $variabile == "FALSE" } {
       return BOOL
     } 

     # Verifico se e' una parola divisa da un punto 
     # nel qual caso e' una chiamata a funzione
     if {[string match {[^a-zA-Z]*.*} $variabile]} {
       return [VerificaTipoVariabileParametroFB $variabile]
     }

     # Verifico se rientra tra i function block
     if {[lsearch -exact [array names FunctionBlockList] $variabile] != -1 } {
       return INT
     }

     # Verifico se rientra tra le variabili definite dall' utente
     if [info exist VariableList($variabile)] {
       if { $aritmetica == 1 && [lindex $VariableList($variabile) 1] == "BOOL" } {
         return INT
       } else { 
         return [lindex $VariableList($variabile) 1]
       }
     } else {
      # Verifico se rientra tra i segnali di scambio o tra i segnali di scambio dinamico
      set indice_variabile [lsearch -exact [read $input] $variabile]
      seek $input 0 start
      if { $input2 != -1 } {
        set indice_variabile_dinamica [lsearch -exact [read $input2] $variabile]
        seek $input2 0 start
      } else {
        set indice_variabile_dinamica -1
      }
      if { $indice_variabile != -1} {
        # se e' un segnale di scambio verifica se e' scalare o vettoriale
        set tipo [lrange [read $input] [expr $indice_variabile +4] \
          [expr $indice_variabile + 6]]
        seek $input 0 start

        if { [string first "ARRAY" [lindex $tipo 0]] == -1 } {
          if { $aritmetica == 1 && [lindex $tipo 0] == "BOOL" } {
            return INT
          } else {
            return [lindex $tipo 0]
          }
        } else {
          if { $aritmetica == 1 && [lindex $tipo 2] == "BOOL" } {
            return INT
          } else {
            return [lindex $tipo 2]
          }
        }
      } elseif { $indice_variabile_dinamica != -1} {
        # se e' un segnale di scambio verifica se e' scalare o vettoriale
        set tipo [lrange [read $input2] [expr $indice_variabile_dinamica +4] \
          [expr $indice_variabile_dinamica + 6]]
        seek $input2 0 start

        if { [string first "ARRAY" [lindex $tipo 0]] == -1 } {
          if { $aritmetica == 1 && [lindex $tipo 0] == "BOOL" } {
            return INT
          } else {
            return [lindex $tipo 0]
          }
        } else {
          if { $aritmetica == 1 && [lindex $tipo 2] == "BOOL" } {
            return INT
          } else {
            return [lindex $tipo 2]
          }
        }
      } else {
        # non rientra neppure tra i segnali di scambio
        return 0
      }
    }
  } else {
    if {[string match {*.*} $variabile] || $istruzione == "DINT_TO_REAL" || \
        $istruzione == "STRING_TO_REAL" } {
       return REAL
     } else {
       return INT
     }
  }
}

####################################################################
# Procedura AggiungiBreakLinea
# Procedura che aggiunge i break alla linea considerata
# Input: 
# t              Nome del widget di testo 
# canvas         Nome del widget canvas
# ListaWatch     Lista dei breakpoint impostati 
# Linea          Numero di linea 
# ind_ass        Indirizzo assoluto del modulo
# TipoVariabile  Tipo della variabile 
# AccuInizio     Indice di inizio dell'accumulatore 
# AccuFine       Indice di fine dell' accumulatore 
# OpInizio       Indice di inzio dell'operando 
# OpFine         Indice di fine dell'operando 
# Ritorna :
#   Niente
####################################################################
proc AggiungiBreakLinea { t canvas ListaWatch Linea ind_ass \
                          TipoVariabile AccuInizio AccuFine OpInizio OpFine \
                          istruzione variabile } {

  # Gli indici di LineaBreak sono gli indirizzi dei break,
  # gli elementi sono : ListaWatch r1 r2 scale size off ar widgets tipo_linea istruzione

global LineaBreak OffsetLineaProg accuwdg opwdg index0 index1 ProgramComponent WindowType 
global EditPrefs ContactDef tmp_symbol user_symbol

  set addr_acc -1
  set addr_op -1
  set addr_index0 -1
  set addr_index1 -1
  set ar -1
  set ar_index0 -1
  set ar_index1 -1
  set indice_riga -1
  set indice_colonna -1

  set file_label [TextToTop $t].main.status.name
  set FB_name [lindex [$file_label cget -text] \
    [expr [llength [$file_label cget -text]] -1]]

  # Cerco se tra i simboli di debug c'e' $LE di Linea
  if { $WindowType([TextToTop $t])=="main" } {
    if { [string first JMP $istruzione] == -1 } {
      set indice_linea_debug "LE[expr $Linea + $OffsetLineaProg]"
    } else {
      set indice_linea_debug "LJ[expr $Linea + $OffsetLineaProg]"
    }
  } else {
    foreach x [array names ProgramComponent] {
      if { [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]] == $FB_name } {
        if { [string first JMP $istruzione] == -1 } {
          set indice_linea_debug "LE[expr $Linea + $OffsetLineaProg + \
            [lindex $ProgramComponent($x) 1] -1]"
        } else {
          set indice_linea_debug "LJ[expr $Linea + $OffsetLineaProg + \
            [lindex $ProgramComponent($x) 1] -1]"
        }
        break
      }  
    }
  }
  if [info exists tmp_symbol($indice_linea_debug)] {
    set ar [string index [lindex $tmp_symbol($indice_linea_debug) 2] \
      [expr [string length [lindex $tmp_symbol($indice_linea_debug) 2]] - 1]]
    if { $ar != "0" } {
      set addr_acc [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]] 
    } 
  } 

  # Cerco se tra i simboli di debug c'e' $VA di Linea
  if { $WindowType([TextToTop $t])=="main" } {
    set indice_linea_debug "VA[expr $Linea + $OffsetLineaProg]"
  } else {
    foreach x [array names ProgramComponent] {
      if { [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]] == $FB_name } {
        set indice_linea_debug "VA[expr $Linea + $OffsetLineaProg + \
           [lindex $ProgramComponent($x) 1] -1]"
        break
      }  
    }
  }
# Nel caso di ) l'informazione di VA non e' significativa
  if {[info exists tmp_symbol($indice_linea_debug)] && $istruzione != ")"}  {
    set addr_op [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]]

    set spezza_info [split [lindex $tmp_symbol($indice_linea_debug) 2] "$"]

    set r1 [string index [lindex $spezza_info 2] 0]
    set r2 [string index [lindex $spezza_info 2] 1]
    set scale [string index [lindex $spezza_info 2] 2]
    set size [string index [lindex $spezza_info 2] 3]

    set off [lindex $spezza_info 3]
     
    if {[lindex $spezza_info 4] != "" } {
      if [info exists user_symbol([lindex $spezza_info 4])] {
        set off [expr $off + $ind_ass + \
         0x[lindex $user_symbol([lindex $spezza_info 4]) 0]]
      }
    }
  }

  # Cerco se tra i simboli di debug c'e' $VI di Linea
  if { $WindowType([TextToTop $t])=="main" } {
    set indice_linea_debug "VI[expr $Linea + $OffsetLineaProg]:0"
  } else {
    foreach x [array names ProgramComponent] {
      if { [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]] == $FB_name } {
        set indice_linea_debug "VI[expr $Linea + $OffsetLineaProg]:0"
        break
      }  
    }
  }
  if [info exists tmp_symbol($indice_linea_debug)] {
    set spezza_info [split [lindex $tmp_symbol($indice_linea_debug) 2] "$"]

    set addr_index1 [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]] 
    set ar_index1 [lindex $spezza_info 2]
  }

  if { $WindowType([TextToTop $t])=="main" } {
    set indice_linea_debug "VI[expr $Linea + $OffsetLineaProg]:1"
  } else {
    foreach x [array names ProgramComponent] {
      if { [RicavaNomeBlocco [lindex $ProgramComponent($x) 0]] == $FB_name } {
        set indice_linea_debug "VI[expr $Linea + $OffsetLineaProg]:1"
        break
      }  
    }
  }  
  if [info exists tmp_symbol($indice_linea_debug)] {
    set spezza_info [split [lindex $tmp_symbol($indice_linea_debug) 2] "$"]

    set addr_index0 [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]] 
    set ar_index0 [lindex $spezza_info 2]
  }

  # Vado a settare i breakpoint
  if { $EditPrefs(awl) == 1 } {
    # Caso IL
    if { $addr_acc != -1 } {
      # Se ha senso il valore dell'accumulatore alla linea
      if {$TipoVariabile == "BOOL" } {
        CreaAccuTag $t $Linea $AccuInizio $AccuFine
      } else {
        CreaDebugWidget $t $Linea 
      }
      if { $addr_op != -1 } {
        # Se c'e' un operando significativo
        if {$TipoVariabile == "BOOL" } {
          CreaOpTag $t $Linea $OpInizio $OpFine
        } 
        if [info exists LineaBreak($addr_op)] {
          # Se ho gia' il break dell'accumulatore allo stesso indirizzo
          if {$TipoVariabile == "BOOL" } {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
                accutag$Linea $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off \
                [lindex $LineaBreak($addr_op) 6]  [lindex $LineaBreak($addr_op) 7] \
                optag$Linea $TipoVariabile]
          } else {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
                accuwdg_$Linea $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off \
                [lindex $LineaBreak($addr_op) 6] [lindex $LineaBreak($addr_op) 7] \
                opwdg_$Linea $TipoVariabile]
          }
        } else {
          # Se i due indirizzi sono diversi faccio due break
          if {$TipoVariabile == "BOOL" } {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar accutag$Linea \
               $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off 0 \
               optag$Linea $TipoVariabile]
          } else {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar accuwdg_$Linea \
               $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off 0 \
               opwdg_$Linea $TipoVariabile]
          }
        }
        # Se l'operando e' un vettore o una matrice
        if { $addr_index0 != -1 && $addr_index1 == -1} {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea
          }
          set LineaBreak($addr_index0) [list $ListaWatch 0 0 0 0 0 $ar_index0 \
              index0_$Linea INT]
        } elseif { $addr_index0 == -1 && $addr_index1 != -1} {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea
          }
          set LineaBreak($addr_index1) [list $ListaWatch 0 0 0 0 0 $ar_index1 \
              index1_$Linea INT]
        } elseif { $addr_index0 != -1 && $addr_index1 != -1 } {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea
          }
          set LineaBreak($addr_index1) [list $ListaWatch 0 0 0 0 0 $ar_index1 \
              index1_$Linea INT]
          set LineaBreak($addr_index0) [list $ListaWatch 0 0 0 0 0 $ar_index0 \
              index0_$Linea INT]
        } 
      } else {
        if {$TipoVariabile == "BOOL" } {
          CreaOpTag $t $Linea $OpInizio $OpFine

          if {$istruzione == "LDN" || $istruzione == "STN" } {
            set linea_neg $Linea
            append linea_neg _N
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "accutag$Linea;optag$linea_neg" $TipoVariabile]
          } elseif { [string first "(" $istruzione] != -1 } {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "optag$Linea" $TipoVariabile]
          } else {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "accutag$Linea;optag$Linea" $TipoVariabile]
          }
        } else {
          # Nel caso di parentesi chiusa non conta l'operando
          if { $istruzione != ")" } {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "accuwdg_$Linea;opwdg_$Linea" $TipoVariabile]
          } else {
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "accuwdg_$Linea" $TipoVariabile]
          }
        }
        # Se l'operando e' un vettore o una matrice
        if { $addr_index0 != -1 && $addr_index1 == -1} {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea 
          }
          set LineaBreak($addr_index0) [list $ListaWatch 0 0 0 0 0 $ar_index0 \
              index0_$Linea INT]
        } elseif { $addr_index0 == -1 && $addr_index1 != -1} {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea
          }
          set LineaBreak($addr_index1) [list $ListaWatch 0 0 0 0 0 $ar_index1 \
              index1_$Linea INT]
        } elseif { $addr_index0 != -1 && $addr_index1 != -1 } {
          if {$TipoVariabile == "BOOL"} {
            CreaDebugWidget $t $Linea 
          }
          set LineaBreak($addr_index1) [list $ListaWatch 0 0 0 0 0 $ar_index1 \
              index1_$Linea INT]
          set LineaBreak($addr_index0) [list $ListaWatch 0 0 0 0 0 $ar_index0 \
              index0_$Linea INT]
        }
      }
    } else {
      # Questo caso si verifica se sto debuggando 
      # la prima istruzione di LD
      if {$TipoVariabile == "BOOL" } {
        CreaAccuTag $t $Linea $AccuInizio $AccuFine
        CreaOpTag $t $Linea $OpInizio $OpFine
      } else {
        CreaDebugWidget $t $Linea 
      }
    } 
  } elseif { $EditPrefs(awl) == 0 } {
    # Caso LADDER
    if {[string match {[^a-zA-Z]*.*} $variabile] || $istruzione == "CAL"} {
      # se si tratta di chiamata a blocco funzionale non mettere break
      return
    }
    # Se la variabile e' di tipo booleano considero solo 
    # l'operando viceversa considero entrambi
    global LINE_CONTACT[lindex [split $canvas .] end]
    set CanvasTempo LINE_CONTACT[lindex [split $canvas .] end]
    append CanvasTempo "($Linea)"
    if { [info exist $CanvasTempo] == 0 } {
      # Se non esiste il contatto associato alla linea passo oltre
      return
    }
    set cmd "lindex \$$CanvasTempo 0"
    set cmd1 "lindex \$$CanvasTempo 1"
    set contatto "[eval $cmd]:[eval $cmd1]"
    if { $addr_acc != -1 } {
      # Se ha senso il valore dell'accumulatore alla linea
      if { $addr_op == -1 } {
        if {$TipoVariabile == "BOOL" } {
          if {$istruzione == "LDN" || $istruzione == "STN" } {
            append contatto _N
          } 
          set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar \
              "$canvas $contatto" $TipoVariabile]
        } else {
          CreaDebugWidgetLadder $canvas $contatto $Linea
          set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar accuwdg_$Linea \
              $TipoVariabile]
        }
      } else {
        # Se c'e' un operando significativo
        if [info exists LineaBreak($addr_op)] {
          # Se ho gia' il break dell'accumulatore allo stesso indirizzo
          if {$TipoVariabile == "BOOL" } {
            if {$istruzione == "ANDN" || $istruzione == "ORN" || $istruzione == "XORN" } {
              append contatto _N
            } 
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off \
                [lindex $LineaBreak($addr_op) 6] [lindex $LineaBreak($addr_op) 7] \
                "$canvas $contatto" $TipoVariabile]
          } else {
            CreaDebugWidgetLadder $canvas $contatto $Linea
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar accuwdg_$Linea \
                $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off \
                [lindex $LineaBreak($addr_op) 6] [lindex $LineaBreak($addr_op) 7] \
                opwdg_$Linea $TipoVariabile]
          }
        } else {
          # Se i due indirizzi sono diversi faccio due break
          if {$TipoVariabile == "BOOL" } {
            if {$istruzione == "ANDN" || $istruzione == "ORN" || $istruzione == "XORN" } {
              append contatto _N
            } 
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off 0 \
                "$canvas $contatto" $TipoVariabile]
          } else {
            CreaDebugWidgetLadder $canvas $contatto $Linea
            set LineaBreak($addr_acc) [list $ListaWatch 0 0 0 0 0 $ar accuwdg_$Linea \
                $TipoVariabile]
            set LineaBreak($addr_op) [list $ListaWatch $r1 $r2 $scale $size $off 0 \
                op_wdg$Linea $TipoVariabile]
          } 
        }
      }
    } 
  }
}

####################################################################
# Procedura AggiungiBreakLineaCondizionata
# Procedura che aggiunge i break alla linea che contiene la chiamata al FB
# Input: 
#   t            Nome del widget di testo
#   ind_ass      Indirizzo assoluto
# Ritorna :
#   Niente
####################################################################
proc AggiungiBreakLineaCondizionata { t  ind_ass } {

  # Gli indici di LineaBreak sono gli indirizzi dei break,
  # gli elementi sono : ListaWatch r1 r2 scale size off ar widgets tipo_linea istruzione

global ProgramComponent LineaBreak tmp_symbol 

  set addr_acc -1
  set ar -1
  set LineaPrima [PromptBox -text PromptLineBefore  -title PromptBefore]
  if { $LineaPrima == "" } {
    return -1
  }
  set LineaDopo [PromptBox -text PromptLineAfter -title PromptAfter]
  if { $LineaDopo == "" } {
    return -1
  }
  # Resetto la situazione precedente
  if [watchBreak reset] {
    return -1
  }

  # Cerco se tra i simboli di debug c'e' $LE di Linea
  set indice_linea_debug "LE[expr $LineaPrima  + [lindex $ProgramComponent(0) 2]]"
  if [info exists tmp_symbol($indice_linea_debug)] {
    set ar [string index [lindex $tmp_symbol($indice_linea_debug) 2] \
          [expr [string length [lindex $tmp_symbol($indice_linea_debug) 2]] - 1]]
    if { $ar != "0" } {
      set addr_acc [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]] 
    } 
    # Creo la lista di break
    set ListaTmp [watchBreak create 0 -log +set opa1=1]
    if { $ListaTmp == "-1" } {
       return -1
    }
    if [watchBreak add $ListaTmp $addr_acc 0 0 0 0 0 $ar -log] {
       return -1
    } 
  }

  # Cerco se tra i simboli di debug c'e' $LE di Linea
  set indice_linea_debug "LE[expr $LineaDopo  + [lindex $ProgramComponent(0) 2]]"
  if [info exists tmp_symbol($indice_linea_debug)] {
    set ar [string index [lindex $tmp_symbol($indice_linea_debug) 2] \
          [expr [string length [lindex $tmp_symbol($indice_linea_debug) 2]] - 1]]
    if { $ar != "0" } {
      set addr_acc [expr $ind_ass + 0x[lindex $tmp_symbol($indice_linea_debug) 0]] 
    } 
    # Creo la lista di break
    set ListaTmp [watchBreak create 1 -log +reset opa2=1]
    if { $ListaTmp == "-1" } {
       return -1
    }
    if [watchBreak add $ListaTmp $addr_acc 0 0 0 0 0 $ar -log] {
       return -1
    } 
  }
  return 2
}

####################################################################
# Procedura CRCcorretto
# Procedura che verifica la correttezza del CRC dell'oggetto e del file
# Input: 
#   ind_ass      Indirizzo assoluto del modulo
#   t            Nome del widget di testo
# Ritorna :
#	0	tutto ok
#	1	CRC non corrisponde
#	2	Fallita la funzione che ricava il CRC dall'oggetto
####################################################################
proc CRCcorretto { ind_ass t } {
global VariableList MainOfFunctionBlock user_symbol 
  
  if [info exists user_symbol(_CRC)] {
    if { [TestTipoFinestra $t]=="main" } {
      set Nome_File [GetFilename $t]
    } else {
      set Nome_File [GetFilename $MainOfFunctionBlock([TextToTop $t])]
    }
    set _CRCaddr [expr $ind_ass + 0x[lindex $user_symbol(_CRC) 0]]
    set userCRC [RicavaCRCModulo $_CRCaddr]
    if { $userCRC != 1 } {
      # inizializzo la varibile _CRC con il CRC di *.plc *.msd
      if {[file extension $Nome_File] == ""} {
        set Nome_File $Nome_File.plc
      }
      set progCRC [CalcolaCRC $Nome_File]
      set msdCRC [CalcolaCRC [file root $Nome_File].msd]
      if { $userCRC == [expr $progCRC + $msdCRC] } {
        return 0
      } else {
        return 1
      }
    } else {
      return 2
    }
  }
}

####################################################################
# Procedura ProtocolloAttivaDebug
# Procedura che raccoglie le procedure per l'attivazione debug
# DebugState = 0 Il debug e' fermo
# DebugState = 1 Il debug e' attivo
# DebugState = 2 E' stata lanciata la procedura di timeout attivazione debug 
# Input: 
#   W            Nome del widget di testo
#   TipoProtocollo ??? 
# Ritorna :
#   Niente
####################################################################
proc ProtocolloAttivaDebug { W TipoProtocollo } {
  global DebugState AttivatoDebug 

  if { $AttivatoDebug == 1 } {
    after 50 "ProtocolloAttivaDebug $W $TipoProtocollo"
    return
  }

  if { $TipoProtocollo == "IL" } {
    if { $DebugState == 1 } { 
      set DebugState 0
      DebugProgram $W
      set DebugState 1
      after 1000 "AttivaDebug $W"
      set DebugState 2
    } elseif { $DebugState == 2 } {
      after cancel "AttivaDebug $W"
      after 1000 "AttivaDebug $W"
    }
  } elseif { $TipoProtocollo == "LADDER1" } {
    if { $DebugState == 1 } { 
      # Se la variabile di debug e' a 1 vado a resettare prima di passare alla equazione
      # successiva
      set DebugState 0
      DebugProgram $W
      set DebugState 1
    }
  } elseif { $TipoProtocollo == "LADDER2" } {
    if { $DebugState == 1 } { 
      after 1000 "AttivaDebug $W"
      set DebugState 2
    } elseif { $DebugState == 2 } {
      after cancel "AttivaDebug $W"
      after 1000 "AttivaDebug $W"
    }
  }
}

####################################################################
# Procedura AttivaDebug
# Procedura che aspetta 1 secondi prima di attivare la procedura di debug
# Input: 
#   args         ???
# Ritorna :
#   Niente
####################################################################
proc AttivaDebug { args } {
  global DebugState

  set DebugState 1
  DebugProgram [lindex $args 0]
}

####################################################################
# Procedura LoadPlc
# Procedura che copia il file oggetto filename.oo nel modulo canonico e carica il PLC
# Input: 
#   t            Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc LoadPlc { t } {
global EditPrefs tcl_platform Home
global SiIoconf IoconfModificato ModiFile

  if { [TextToTop $t]==".jedit0" && $ModiFile==1 } {
    if [ConfirmBox -text ConfirmFileChanged] {
      CmdSave $t
    } else {
      return
    }
    set ModiFile 0
  } else {
    set filename [GetFilename $t]
    if {[file extension $filename] == ""} {
      set filename $filename.plc
    }
    SetFilename $t $filename
  }

  if { [CompileProgram $t 0] == 1 } {
    return
  }


  if {$EditPrefs(plclocale) || ($EditPrefs(plclocale) == "0" && \
      $EditPrefs(plcremoto) == "localhost")} {
    if [file exists $EditPrefs(objpath)/[file root [file tail [GetFilename $t]]].oo] {
      if [ConfirmBox -text MsgLoadPLC] {
        if { $SiIoconf == "yes" } {
          ScriviIoconfFile
        }
        file copy -force $EditPrefs(objpath)/[file root [file tail [GetFilename $t]]].oo \
                         $EditPrefs(objpath)/$EditPrefs(nomemodulo)
        # scrivo il plc caricato in un file a parte
        if [catch {set FileId [open $EditPrefs(objpath)/$EditPrefs(plc-caricato) w] } \
           result] {
          AlertBox -text $result
        } else {
          puts $FileId [GetFilename $t]
          close $FileId
        }

  	# Verifico se il PLC c'e'
  	if {[VerificaEsistenzaPlc]} { 
          WaitWindow $t 0
    	  AlertBox -text MsgNoPlc
    	  return
  	} 
        ApriPLC
        # Resetto la situazione precedente
        watchBreak reset
        if [CaricaOggetto $EditPrefs(objpath)/$EditPrefs(nomemodulo)] {
          AlertBox -text MsgCaricaModErr
        }
        ChiudiPLC
      }
    } else {
      set fileobject [file root [file tail [GetFilename $t]]].oo
      AlertBox -text "$fileobject [GetString MsgFileObject]"
    }
  } else {
    if [file exists $EditPrefs(progpath)/$EditPrefs(nomemodulo)] {
      set TipoConferma [ConfirmBox -text MsgLoadPLC -thirdbutton SOURCE] 
      WaitWindow $t 1
      if $TipoConferma {
        if { $SiIoconf == "yes" } {
          ScriviIoconfFile
        }
        # Facciamo finta di aprire un socket per avere i servizi a disposizione
        catch {socket}  res
        TransmitFile $EditPrefs(nomemodulo) $EditPrefs(objpath) $EditPrefs(progpath) \
                     $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)

        # scrivo il plc caricato in un file a parte
        if [catch {set FileId [open $EditPrefs(progpath)/$EditPrefs(plc-caricato) w] } \
             result] {
          AlertBox -text $result
        } else {
          if { $TipoConferma == 1 } {
            # Se non trasferisco i sorgenti nel plc-caricato metto il percorso
            # locale
            puts $FileId [GetFilename $t]
          } elseif { $TipoConferma == 2 } {
            # Se trasferisco anche i sorgenti metto in plc-caricato il percorso
            # remoto dove cioe' vado a scaricare i file
            puts $FileId "$EditPrefs(progpath1)/[file tail [GetFilename $t]]"
          }
          close $FileId
        }

        if { [TransmitFile $EditPrefs(plc-caricato) $EditPrefs(objpath) \
             $EditPrefs(progpath) $EditPrefs(plcremoto) $EditPrefs(plclogin) \
             $EditPrefs(plcpassword)] != "0" } {
          AlertBox -text MsgTransmitErr
        }
        file delete $EditPrefs(progpath)/$EditPrefs(plc-caricato)

        if { $TipoConferma == 2 } {
          # Trasmetto i sorgenti zippati
          if { $EditPrefs(plcremoto) == "2.2.2.2" } {
            set SorgenteFileCaricato [GetFilename $t]
            file copy -force $SorgenteFileCaricato $EditPrefs(progpath)/source
            file copy -force [file root $SorgenteFileCaricato].msd \
                  $EditPrefs(progpath)/srcmsd
            file copy -force [file root $SorgenteFileCaricato].mod \
                  $EditPrefs(progpath)/srcmod
            if {$tcl_platform(platform) == "unix"} {
              set comando gzip
            } else {
              set comando $Home/gzip
            }
            catch {exec $comando $EditPrefs(progpath)/source} Risultato
            if { $Risultato == "" } {
              if { [TransmitFile source.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
                 $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                 $EditPrefs(plcpassword)] != "0" } {
                AlertBox -text MsgTransmitErr
              }
            } else {
              AlertBox -text MsgZipFile
            }
            catch {exec $comando $EditPrefs(progpath)/srcmsd} Risultato
            if { $Risultato == "" } {
              if { [TransmitFile srcmsd.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
                 $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                 $EditPrefs(plcpassword)] != "0" } {
                AlertBox -text MsgTransmitErr
              }
            } else {
              AlertBox -text MsgZipFile
            }
            catch {exec $comando $EditPrefs(progpath)/srcmod} Risultato
            if { $Risultato == "" } {
              if { [TransmitFile srcmod.gz $EditPrefs(progpath1) $EditPrefs(progpath) \
                 $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                 $EditPrefs(plcpassword)] != "0" } {
                AlertBox -text MsgTransmitErr
              }
            } else {
              AlertBox -text MsgZipFile
            }
            file delete $EditPrefs(progpath)/source.gz
            file delete $EditPrefs(progpath)/srcmsd.gz
            file delete $EditPrefs(progpath)/srcmod.gz
          }
          # Trasmetto anche i sorgenti non zippati
          if { $EditPrefs(plcremoto) != "2.2.2.2" } {
            set SorgenteFileCaricato [GetFilename $t]
            set NomeSorgente [file tail $SorgenteFileCaricato]
            set NomeMsd [file root [file tail $SorgenteFileCaricato]].msd
            set NomeMod [file root [file tail $SorgenteFileCaricato]].mod
            if { [TransmitFile $NomeSorgente $EditPrefs(progpath1) $EditPrefs(progpath) \
               $EditPrefs(plcremoto) $EditPrefs(plclogin) \
               $EditPrefs(plcpassword)] != "0" } {
              AlertBox -text MsgTransmitErr
            }
            if { [TransmitFile $NomeMsd $EditPrefs(progpath1) $EditPrefs(progpath) \
               $EditPrefs(plcremoto) $EditPrefs(plclogin) \
               $EditPrefs(plcpassword)] != "0" } {
              AlertBox -text MsgTransmitErr
            }
            if { [TransmitFile $NomeMod $EditPrefs(progpath1) $EditPrefs(progpath) \
               $EditPrefs(plcremoto) $EditPrefs(plclogin) \
               $EditPrefs(plcpassword)] != "0" } {
              AlertBox -text MsgTransmitErr
            }
          }
        }

        # Trasmetto e scrivo il file dei simboli zippati
        ScriviSymbolFile
        if {$tcl_platform(platform) == "unix"} {
          set comando gzip
        } else {
          set comando $Home/gzip
        }
        catch {exec $comando $EditPrefs(progpath)/$EditPrefs(symbol)} Risultato
        if { $Risultato == "" } {
          if { [TransmitFile $EditPrefs(symbol).gz $EditPrefs(progpath1) \
                 $EditPrefs(progpath) $EditPrefs(plcremoto) $EditPrefs(plclogin) \
                 $EditPrefs(plcpassword)] != "0" } {
            AlertBox -text MsgTransmitErr
          }
        } else {
          AlertBox -text MsgZipFile
        }
        file delete $EditPrefs(progpath)/$EditPrefs(symbol).gz

  	if {[VerificaEsistenzaPlc]} { 
          WaitWindow $t 0
    	  AlertBox -text MsgNoPlc
    	  return
        } 
        ApriPLC
        # Resetto la situazione precedente
        watchBreak reset
        if [CaricaOggetto $EditPrefs(objpath)/$EditPrefs(nomemodulo)] {
          AlertBox -text MsgCaricaModErr
        }
        ChiudiPLC
      }
    } else {
      set fileobject [file root [file tail [GetFilename $t]]].oo
      AlertBox -text "$fileobject [GetString MsgFileObject]"
    }
  }

  if { $SiIoconf == "yes" } {
    if { $IoconfModificato == "yes" } {
      set IoconfModificato "no"
    }
  }

  WaitWindow $t 0
}

####################################################################
# Procedura ScriviIoconfFile
# Procedura che scrive il file ioconf per discriminare quali moduli sono necessari
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc ScriviIoconfFile { } {
global EditPrefs tcl_platform VariableList
global elenco_moduli MsdData IoconfModificato

  if { $IoconfModificato == "no" } {
    return
  }

  if [catch { set fIoconf [open $EditPrefs(progpath)/$EditPrefs(ioconf) w] } \
       result] {
    AlertBox -text $result
    return
  }

  set Indice 0
  for { set i 0 } { $i < $MsdData(maxl) } {incr i} {
    for { set j 0 } { $j < $MsdData(maxmios) } {incr j} {
      if { [lindex $elenco_moduli($i,$j) end] == "S" } {
        puts $fIoconf "# $i $j [lindex $elenco_moduli($i,$j) 1] [lindex $elenco_moduli($i,$j) 2] S"
        set TipoIo S
        # Se gli input sono <= degli output scrivo prima loro
        if { [lindex $elenco_moduli($i,$j) 1] <= [lindex $elenco_moduli($i,$j) 2] } {
          for { set k 0 } { $k < [lindex $elenco_moduli($i,$j) 1] } {incr k} {
            set ioconf($Indice) "%IXIOS.0.$i.$j.0.$k $TipoIo"
            incr Indice
          }
          for { set k [lindex $elenco_moduli($i,$j) 1] } \
              { $k < [expr [lindex $elenco_moduli($i,$j) 1] + \
              [lindex $elenco_moduli($i,$j) 2]] } {incr k} {
            set ioconf($Indice) "%QXIOS.0.$i.$j.0.$k $TipoIo"
            incr Indice
          }
        } else {
          for { set k 0 } { $k < [lindex $elenco_moduli($i,$j) 2] } {incr k} {
            set ioconf($Indice) "%QXIOS.0.$i.$j.0.$k $TipoIo"
            incr Indice
          }
          for { set k [lindex $elenco_moduli($i,$j) 2] } \
              { $k < [expr [lindex $elenco_moduli($i,$j) 1] + \
              [lindex $elenco_moduli($i,$j) 2]] } {incr k} {
            set ioconf($Indice) "%IXIOS.0.$i.$j.0.$k $TipoIo"
            incr Indice
          }
        }
      }
    }
  }

  foreach iocnf [lsort -integer [array names ioconf]] {
    puts $fIoconf $ioconf($iocnf)
  }

  foreach NomeVar [array names VariableList] {
    # I/O Speciale
    if {[lindex $VariableList($NomeVar) 3] == "input" || \
        [lindex $VariableList($NomeVar) 3] == "output" } {
      if { [string first "." [lindex $VariableList($NomeVar) 8]] != -1 && \
           [string first "_S" [lindex $VariableList($NomeVar) 8]] != -1 } {
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
        set IoString [string trimright [lindex $VariableList($NomeVar) 8] "_S"]
        puts $fIoconf "$Versus$IoString S"
      }
    }
  } 

  close $fIoconf

  if {$EditPrefs(plclocale) || ($EditPrefs(plclocale) == "0" && \
      $EditPrefs(plcremoto) == "localhost")} {
    file copy -force $EditPrefs(progpath)/$EditPrefs(ioconf) \
                         $EditPrefs(progpath1)/$EditPrefs(ioconf)
  } else {
    catch {socket}  res
    TransmitFile $EditPrefs(ioconf) $EditPrefs(progpath1) $EditPrefs(progpath) \
               $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)
  }
  file delete $EditPrefs(progpath)/$EditPrefs(ioconf)
}

####################################################################
# Procedura RitornaPrimaDimensione
# Procedura che ricava le dimensioni di una variabile 
# data la sua definizione
# Input: 
#   linea   Linea di definizione della variabile
# Ritorna :
#   La prima dimensione della variabile
####################################################################
proc RitornaPrimaDimensione { linea } {

  set tipo [lindex $linea 4]
  if { [string first "ARRAY" $tipo] == -1 } {
    return 1
  } else {
    set primosplit [split $tipo ","]
    if {[llength $primosplit] == 1} {
        set dim1 [string trimleft [lindex $primosplit 0] "ARRAY\[0.."]
        set dim1 [string trimright $dim1 "\]"]
      } else {
        set dim1 [string trimleft [lindex $primosplit 0] "ARRAY\[0.."]
      }
    return [expr $dim1 + 1]
  }
return 1
}

####################################################################
# Procedura RitornaSecondaDimensione
# Procedura che ricava le dimensioni di una variabile 
# data la sua definizione
# Input: 
#   linea   Linea di definizione della variabile
# Ritorna :
#   La seconda dimensione della variabile
####################################################################
proc RitornaSecondaDimensione { linea } {

  set tipo [lindex $linea 4]
  if { [string first "ARRAY" $tipo] == -1 } {
    return 1
  } else {
    set primosplit [split $tipo ","]
    if {[llength $primosplit] == 1} {
      set dim2 1
    } else {
      set dim2 [string trimleft [lindex $primosplit 1] "0.."]
      set dim2 [string trimright $dim2 "\]" ]
      set dim2 [expr $dim2 + 1]
    }
    return $dim2
  }
return 1
}

####################################################################
# Procedura ScriviSymbolFile
# Procedura che scrive il file USER_SYM  che contiene l'elenco 
# delle variabili usate nel programma PLC
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc ScriviSymbolFile { } {
global EditPrefs tcl_platform VariableList
global input input2

  foreach Variabile [lsort -ascii [array names VariableList]] {
    if { [lindex $VariableList($Variabile) 3] == "memory" } {
      set StringaAppoggio "U " 
    } else {
      set StringaAppoggio "I " 
    }
    append StringaAppoggio "[lindex $VariableList($Variabile) 1] " 
    append StringaAppoggio "[lindex $VariableList($Variabile) 6] " 
    append StringaAppoggio "[lindex $VariableList($Variabile) 7] " 
    set Simboli($Variabile) $StringaAppoggio
  }

  # Gestione segnali di scambio statici
  set TuttoIlFile [read $input]
  seek $input 0 start
  set InizioVariabili 0
  foreach linea [split $TuttoIlFile "\n"] {
    if { [lindex $linea 0] == "VAR_GLOBAL"} {
      set InizioVariabili 1
    } elseif  { [lindex $linea 0] == "END_VAR"} {
      set InizioVariabili 0
    } else {
      if { $InizioVariabili == 1 } {
        set StringaAppoggio "S " 
        append StringaAppoggio "[lindex $linea [expr [llength $linea] -2]] " 
        append StringaAppoggio "[RitornaPrimaDimensione $linea] " 
        append StringaAppoggio "[RitornaSecondaDimensione $linea] " 
        set Simboli([lindex $linea 0]) $StringaAppoggio
      }
    }
  }

  # Gestione segnali di scambio dimanici
  set TuttoIlFile [read $input2]
  seek $input2 0 start
  set InizioVariabili 0
  foreach linea [split $TuttoIlFile "\n"] {
    if { [lindex $linea 0] == "VAR_GLOBAL"} {
      set InizioVariabili 1
    } elseif  { [lindex $linea 0] == "END_VAR"} {
      set InizioVariabili 0
    } else {
      if { $InizioVariabili == 1 } {
        set StringaAppoggio "S " 
        append StringaAppoggio "[lindex $linea [expr [llength $linea] -2]] " 
        append StringaAppoggio "[RitornaPrimaDimensione $linea] " 
        append StringaAppoggio "[RitornaSecondaDimensione $linea] " 
        set Simboli([lindex $linea 0]) $StringaAppoggio
      }
    }
  }

  if [catch { set fSymbol [open $EditPrefs(progpath)/$EditPrefs(symbol) w] } \
      result] {
    AlertBox -text $result
    return
  }
  puts $fSymbol [lindex $VariableList(_CREATED) 4]

  foreach Symbol [lsort -ascii [array names Simboli]] {
    puts $fSymbol "$Symbol $Simboli($Symbol)"
  }
  close $fSymbol
}

####################################################################
# Procedura StartStopDebug
# Procedura che cambia stato al flag di debug e chiama la procedura
# per le operazioni relative
# Input: 
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc StartStopDebug { t } {
global DebugState

set DebugState [expr !$DebugState]
DebugProgram $t 1
}

####################################################################
# Procedura SetNewDebugOption
# Procedura che salva i valori di impostati per il debug
# Input: 
#   t         Nome del widget di testo
#   WdgName   Nome della toplevel per i parametri
# Ritorna :
#   Niente
####################################################################
proc SetNewDebugOption { t WdgName } { 
global OptDebug OptDebugOri EditPrefs PrtOpt
global prt_opz_change

  set prt_opz_change yes

  if { $EditPrefs(plclocale) == 1 } {
    [TextToTop $t].remote config -text "[GetString PlcLocaleMin]"
    set OptDebug(objpathlocale) $OptDebug(objpath)
    set OptDebug(progpath1locale) $OptDebug(progpath1)
  } elseif { $EditPrefs(plclocale) == 0 } {
    [TextToTop $t].remote config -text "[GetString PlcRemotoMin] \
        : \t$OptDebug(plcremoto)"
    set OptDebug(objpathremoto) $OptDebug(objpath)
    set OptDebug(progpath1remoto) $OptDebug(progpath1)

  }

  foreach elemento [array names OptDebug] {
    set EditPrefs($elemento) $OptDebug($elemento)
    set PrtOpt($elemento) $OptDebug($elemento)
  }
  destroy $WdgName
}

####################################################################
# Procedura SetDefaultDebugOption
# Procedura che ripristina i valori di default per il debug
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc SetDefaultDebugOption { } {
global OptDebug OptDebugOri EditPrefs

  foreach elemento [array names OptDebugOri] {
    set EditPrefs($elemento) $OptDebugOri($elemento)
    set OptDebug($elemento) $OptDebugOri($elemento)
  }
  if { $EditPrefs(plclocale) == 1 } {
    set OptDebug(objpath) $OptDebug(objpathlocale)
    set OptDebug(progpath1) $OptDebug(progpath1locale)
  } elseif { $EditPrefs(plclocale) == 0 } {
    set OptDebug(objpath) $OptDebug(objpathremoto)
    set OptDebug(progpath1) $OptDebug(progpath1remoto)
  }
}

###################################################################e
# Procedura DebugOptionWindow
# Procedura che crea la finestra di definizione parametri per 
# il debug
# Input: 
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc DebugOptionWindow { t } {
global OptDebug OptDebugOri EditPrefs tcl_platform

  set old_focus [focus]
  set MainW [winfo toplevel [focus]]
  grab release [grab current]

  toplevel .debwin
  wm title .debwin [GetString CmdDebugOption]
  wm protocol .debwin WM_DELETE_WINDOW procx

  if { $tcl_platform(platform) == "unix" } {
    frame .debwin.locrem

    radiobutton .debwin.locrem.locale -text [GetString PlcLocale] \
          -variable EditPrefs(plclocale) -value 1 -font $EditPrefs(windowfont) \
          -command "SetRemoteLocalVariable $t .debwin.macchina .debwin.socket \
          .debwin.locrem"
    radiobutton .debwin.locrem.remoto -text [GetString PlcRemoto] \
          -variable EditPrefs(plclocale) -value 0 -font $EditPrefs(windowfont) \
          -command "SetRemoteLocalVariable $t .debwin.macchina .debwin.socket \
          .debwin.locrem"
  }

  VariableEntry .debwin.macchina -label [GetString MachineName] \
    -labelwidth 30 -entrywidth 30 -variable OptDebug(plcremoto)
  VariableEntry .debwin.socket -label [GetString SocketNumber] \
    -labelwidth 30 -entrywidth 30 -variable OptDebug(plcsocket)
  VariableEntry .debwin.progpath -label [GetString ProgramPath] \
    -labelwidth 30 -entrywidth 30 -variable OptDebug(progpath1)
  VariableEntry .debwin.objpath -label [GetString ObjPath] \
    -labelwidth 30 -entrywidth 30 -variable OptDebug(objpath)


  tixButtonBox .debwin.b -orientation horizontal

  .debwin.b add ok -image ok -font $EditPrefs(windowfont) \
    -command " SetNewDebugOption $t .debwin"
  .debwin.b add default -image debdef -font $EditPrefs(windowfont) \
    -command { SetDefaultDebugOption }
  .debwin.b add cancel -image cancel -font $EditPrefs(windowfont) \
    -command { destroy .debwin }

  CniBalloon [.debwin.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.debwin.b subwidget default] [GetString prt_opz_lbrel] \
      $EditPrefs(windowfont) 500
  CniBalloon [.debwin.b subwidget cancel] [GetString CANCEL] \
      $EditPrefs(windowfont) 500

  if { $tcl_platform(platform) == "unix" } {
    pack .debwin.locrem.locale .debwin.locrem.remoto -side left -fill x -ipadx 10
    pack .debwin.locrem -side top -pady 10
  }

  if { $EditPrefs(plclocale) } {
    pack .debwin.progpath .debwin.objpath -side top -pady 5
  } else {
    pack .debwin.macchina .debwin.socket .debwin.progpath .debwin.objpath -side top \
      -pady 5
  }

  pack .debwin.b -side top -fill x -pady 5

  wm transient .debwin $MainW
  PosizionaFinestre $MainW .debwin c

  bind .debwin <Key-Escape> {destroy .debwin}

  DefaultButton [.debwin.b subwidget ok] .debwin.macchina.e .debwin.socket.e \
     .debwin.progpath.e .debwin.objpath.e

  grab .debwin
  tkwait window .debwin

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }
}

