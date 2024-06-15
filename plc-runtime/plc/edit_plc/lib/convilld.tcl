
            ######################################
            #                                    #
            #     File di definizione di         #
            #     funzioni per la conversione    #
            #     da il a ladder                 #
            #                                    #
            ######################################


####################################################################
# Procedura DefineMathRect
# Procedura che calcola gli input e gli output della chiamata a blocco
# di funzioni matematiche
# Input: 
#   canvas            Nome del widget cancas
#   text              Nome del widget di testo
#   inizio_equazione  Riga di inizio equazione
#   fine_equazione    Riga di fine equazione
# Ritorna :
#   Niente
####################################################################
proc DefineMathRect { canvas text inizio_equazione fine_equazione} {
global ContactDef BLOCK

  set riga_corrente 0
  set RigaColonnaCommento [EstraiCommento $text $canvas $inizio_equazione]
  if { [lindex $RigaColonnaCommento 0] != -1 } {
    set riga_corrente [expr [lindex $RigaColonnaCommento 0] + 1]
  }

  set canvasCorrente [lindex [split $canvas .] end]
  set index_linea $inizio_equazione
  foreach x [split [$text get $inizio_equazione.0 $fine_equazione.end] \n ] {
    if { [string match {(\**} $x] == 0 && $x != ""} { 
      if { [string match *: [lindex $x 0]] == 0 } {
        set istruzione [lindex $x 0]
        set operando [lindex $x 1]
      } else {
        # Caso label: LD Variabile
        DrawCont $canvas $riga_corrente 0 "LABEL.png" [lindex $x 0 ] "illd"
        set istruzione [lindex $x 1]
        set operando [lindex $x 2]
        incr riga_corrente
      }
      if { $istruzione == "LD" || $istruzione == "LDN" } {
        LdInstr  $canvas $riga_corrente 0 $operando $istruzione $index_linea
        DrawOp $canvas $riga_corrente 0 $riga_corrente 1 "illd"
      } elseif { $istruzione == "ST" || $istruzione == "STN" || $istruzione == "S" || \
                 $istruzione == "R" } { 
        StInstr  $canvas $riga_corrente $riga_corrente 3 $operando $istruzione \
                  $index_linea
        DrawOp $canvas $riga_corrente 2  $riga_corrente 3 "illd"
      } elseif { $istruzione == "JMP" || $istruzione == "JMPC" || \
                 $istruzione == "JMPCN"} { 
        JmpInstr $canvas 0 $riga_corrente 3 $operando $istruzione 
        DrawOp $canvas $riga_corrente 2  $riga_corrente 3 "illd"
      } elseif {$istruzione != "" && $istruzione != "END_PROGRAM" && \
         $istruzione != "END_FUNCTION_BLOCK"} { 
        # Disegno il box
        set BLOCK(title) $istruzione
        if { $operando != "" } { 
          set BLOCK(input) 2
          set BLOCK(in0) [list " " 0]
          set BLOCK(in1) [list " " 0]
          set BLOCK(out0) [list " " 0]
          set BLOCK(out1) [list "" 0]
          DrawRect $canvas $riga_corrente 1 $index_linea 
          # Scrivi l'operando
          LdInstr $canvas [expr $riga_corrente + 1] 0 $operando $istruzione $index_linea 
          DrawOp $canvas [expr $riga_corrente + 1] 0 [expr $riga_corrente + 1] 1 "illd"
        } else {
          set BLOCK(input) 1
          set BLOCK(in0) [list " " 0]
          set BLOCK(out0) [list " " 0]
          DrawRect $canvas $riga_corrente 1 $index_linea 
        }
      }
    }
    incr index_linea
  }
  DrawBarra $canvas
  set ContactDef($canvasCorrente,col_fine_equazione) 4
  set ContactDef($canvasCorrente,col_fine_edt) 4

  if { [lindex $RigaColonnaCommento 0] != -1 } {
    if { [lindex $RigaColonnaCommento 1] > \
         $ContactDef($canvasCorrente,col_fine_equazione) } {
      set numero_colonne [lindex $RigaColonnaCommento 1]
    } else {
      set numero_colonne $ContactDef($canvasCorrente,col_fine_equazione)
    }
  } else {
    set numero_colonne $ContactDef($canvasCorrente,col_fine_equazione)
  }
  return [list [expr $riga_corrente + 1] $numero_colonne]
}

####################################################################
# Procedura AggiustaOperatoreParentesi
# Procedura che attacca eventualmente l'operatore parentesi
# all' operatore che lo precede
# Input: 
#   linea            Linea da anlizzare
# Ritorna :
#   Niente
####################################################################
proc AggiustaOperatoreParentesi { linea } {

  set StringaFinale $linea
  set TrovataParentesi [string first "(" $linea]
  if { $TrovataParentesi != -1 && \
       [string index $linea [expr $TrovataParentesi +1]] != "*" } {
    # Se ho trovato una parentesi e non e' un commento
    if {[string index $linea [expr $TrovataParentesi -1]] == " " || \
        [string index $linea [expr $TrovataParentesi -1]]  == "\t" } {
      set PrimaParte [string range $linea 0 [expr $TrovataParentesi -1]]
      set PrimaParte [string trimright $PrimaParte]
      set StringaFinale "$PrimaParte[string range $linea $TrovataParentesi end]"
    }
  } 
  return $StringaFinale
}

####################################################################
# Procedura WriteLadderEquation
# Procedura che determina l'equazione e la disegna
# Input: 
#   canvas            Nome del widget cancas
#   text              Nome del widget di testo
#   inizio_equazione  Riga di inizio equazione
#   fine_equazione    Riga di fine equazione
# Ritorna :
#   Niente
####################################################################
proc WriteLadderEquation { w t inizio_equazione fine_equazione } {
global ContactDef BLOCK AlberoLadder 

  # riga_corrente variabile che tiene traccia della riga in cui comincia l'equazione
  set riga_corrente 0
  # vettore che tiene traccia delle colonne aperte per ciascuna riga
  set colonne_aperte($riga_corrente) 0
  # variabile che tiene traccia della colonna in cui si e' verificato OR(
  set colonna_or 0
  # variabile che tiene traccia della colonna in cui si e' verificato AND(
  set riga_and $riga_corrente
  # variabile che tiene traccia delle numero massimo di colonne aperte per equazione
  set max_col 1
  # variabile che tiene traccia delle numero massimo di righe aperte per equazione
  set max_riga $riga_corrente
  # variabile che tiene conto dei nesting di OR e AND
  set nesting 0
  # array che contiene per ogni nesting il valore della
  # riga e della colonna corrispondente
  set riga_colonna_nest(0) [list $riga_corrente 0]
  # lista che contiene l'elenco delle linee che sono state aperte
  set quali_linee_aperte $riga_corrente

  set canvasCorrente [lindex [split $w .] end]
  set ContactDef($canvasCorrente,multiple_st) 0
  if [info exists AlberoLadder($canvasCorrente)] {
    unset AlberoLadder($canvasCorrente)
  }

  if [info exists LINE_CONTACT$canvasCorrente] {
    unset LINE_CONTACT$canvasCorrente
  }

  set RigaColonnaCommento [EstraiCommento $t $w $inizio_equazione]
  if { [lindex $RigaColonnaCommento 0] != -1 } {
    set riga_corrente [expr [lindex $RigaColonnaCommento 0] + 1]
    set colonne_aperte($riga_corrente) 0
    set riga_and $riga_corrente
    set max_riga $riga_corrente
    set riga_colonna_nest(0) [list $riga_corrente 0]
    set quali_linee_aperte $riga_corrente
  }

  for { set index_linea $inizio_equazione } { $index_linea <= $fine_equazione} \
      { incr index_linea } {
    set linea [$t get $index_linea.0 $index_linea.end]
    set linea [AggiustaOperatoreParentesi $linea]
    if { [string match *: [lindex $linea 0]] == 0 } {
      set istruzione [lindex $linea 0]
      set operando [lindex $linea 1]
    } else {
      set linee_aperte 0
      if { [lindex $linea 1] == "" } {
        DrawCont $w $riga_corrente 0 "LABEL.png" [lindex $linea 0 ] "illd"
        set linee_aperte 0
        incr riga_corrente
        set colonne_aperte($riga_corrente) 0
        set riga_and $riga_corrente
        set max_riga $riga_corrente
        set riga_colonna_nest(0) [list $riga_corrente 0]
        set quali_linee_aperte $riga_corrente
        continue
      } else {
        # Caso label: LD Variabile
        DrawCont $w $riga_corrente 0 "LABEL.png" [lindex $linea 0 ] "illd"
        set istruzione [lindex $linea 1]
        set operando [lindex $linea 2]
        incr riga_corrente
        set colonne_aperte($riga_corrente) 0
        set riga_and $riga_corrente
        set max_riga $riga_corrente
        set riga_colonna_nest(0) [list $riga_corrente 0]
        set quali_linee_aperte $riga_corrente
      }
    }
    if {$istruzione == "LD" || $istruzione == "LDN"} {
      LdInstr $w $riga_corrente $colonne_aperte($riga_corrente) $operando $istruzione \
              $index_linea
      set colonne_aperte($riga_corrente) [expr $colonne_aperte($riga_corrente) +1]
      set linee_aperte 0
      set quali_linee_aperte $riga_corrente
    } elseif {$istruzione == "AND" || $istruzione == "ANDN" || $istruzione == "AND(" } {
      # se ho linee aperte e non sono all'interno di un OR( chiudo e proseguo 
      # sulla riga di equazione 
      if { $linee_aperte > 0 } {
        # Cerco per prima cosa il numero massimo di colonne aperte su 
        # ciascuna riga aperta
        set max_col $colonne_aperte($riga_and)
        for {set i [expr $riga_and + 1]} {$i <= [expr $riga_corrente + $linee_aperte]} \
            { incr i} {
          if { $colonne_aperte($i) > $max_col } {
            set max_col $colonne_aperte($i)
          }
        }
        # Allineo tutte le righe aperte
        for {set i $riga_and } { $i <= [expr $riga_and + $linee_aperte] } { incr i} {
          # Se e' nella lista delle linee aperte
          set EsisteLineaAperta [lsearch -exact $quali_linee_aperte $i]
          if { $EsisteLineaAperta != -1 } {
            DrawOp $w $i [expr $colonne_aperte($i) -1] $riga_and $max_col "illd"
          }
        }
      } 
      # se non sono all' interno di una istruzione OR(
      if { $riga_corrente == $riga_and } {
	# Disegno il contatto sulla riga di equazione
        if { $linee_aperte > 0 } {
	  # Serve a gestire il caso AND(
          #                         LD FC1
          if { $istruzione == "AND(" && $operando == "" } {
            incr index_linea
            set linea_tmp [$t get $index_linea.0 $index_linea.end]
            set linea [lappend linea [lindex $linea_tmp 1]] 
            if { [lindex $linea_tmp 0] == "LD" } {
	      AndInstr $w $linee_aperte $riga_corrente $max_col [lindex $linea 1 ] \
                [lindex $linea 0 ] $index_linea
	    } elseif { [lindex $linea_tmp 0] == "LDN" } {
	      AndInstr $w $linee_aperte $riga_corrente $max_col [lindex $linea 1 ] \
                ANDN $index_linea 
            }
          } else {
	    AndInstr $w $linee_aperte $riga_corrente $max_col $operando $istruzione \
              $index_linea
          }
	} else {
          # Serve a gestire il caso AND(
          #                         LD FC1
          if { $istruzione == "AND(" && $operando == "" } {
            incr index_linea
            set linea_tmp [$t get $index_linea.0 $index_linea.end]
            set linea [lappend linea [lindex $linea_tmp 1]] 
	    if { [lindex $linea_tmp 0] == "LD" } {
	      AndInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                [lindex $linea 1 ] [lindex $linea 0 ] $index_linea
	    } elseif { [lindex $linea_tmp 0] == "LDN" } {
	      AndInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                [lindex $linea 1 ] ANDN $index_linea 
            }
          } else {
	    AndInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                $operando $istruzione $index_linea
	  }
	}
	# Setto la variabile di apertura AND(
	if {$istruzione == "AND(" } {
          # Modifica versione 1.0.7.7
          if { $linee_aperte > 0 } {
            # Cerco per prima cosa il numero massimo di colonne aperte su 
            # ciascuna riga aperta
            set colonna_or $colonne_aperte($riga_and)
            for {set i [expr $riga_and + 1]} \
                {$i <= [expr $riga_corrente + $linee_aperte]} { incr i} {
              if { $colonne_aperte($i) > $colonna_or } {
                set colonna_or $colonne_aperte($i)
              }
            }
          }
	  #set colonna_or $colonne_aperte($riga_corrente) 
          set nesting [expr $nesting +1]
          set riga_colonna_nest($nesting) [list $riga_and $colonna_or]
	}
	# se ci sono linee aperte aggiorno le colonne_aperte della riga corrente
	if { $linee_aperte > 0 } {
	  set colonne_aperte($riga_corrente) [expr $max_col +1]
	} else {
	  set colonne_aperte($riga_corrente) [expr $colonne_aperte($riga_corrente) +1]
	}
	# Resetto le linee_aperte poiche' ho chiuso tutto
	set linee_aperte 0
        set quali_linee_aperte $riga_corrente
      } else {
        # al posto delle linee aperte ci metto 0 perche' non deve chiudere nulla
	# Serve a gestire il caso AND(
        #                         LD FC1
        if { $istruzione == "AND(" && $operando == "" } {
          incr index_linea
          set linea_tmp [$t get $index_linea.0 $index_linea.end]
          set linea [lappend linea [lindex $linea_tmp 1]] 
          if { [lindex $linea_tmp 0] == "LD" } {
	    AndInstr $w 0 $riga_and $colonne_aperte($riga_and) [lindex $linea 1 ] \
                [lindex $linea 0 ] $index_linea
          } elseif { [lindex $linea_tmp 0] == "LDN" } {
	    AndInstr $w 0 $riga_and $colonne_aperte($riga_and) [lindex $linea 1 ] ANDN \
                $index_linea 
          }
        } else {
	  AndInstr $w 0 $riga_and $colonne_aperte($riga_and) $operando $istruzione \
                $index_linea
        }
	if {$istruzione == "AND(" } {
	  set colonna_or $colonne_aperte($riga_and) 
	  set nesting [expr $nesting +1]
          set riga_colonna_nest($nesting) [list $riga_and $colonna_or]
	}
	set colonne_aperte($riga_and) [expr $colonne_aperte($riga_and) +1]
        for {set i [expr $riga_and + 1]} { $i <= [expr $riga_and + $linee_aperte] } \
            { incr i} {
          # Se e' nella lista delle linee aperte
          set EsisteLineaAperta [lsearch -exact $quali_linee_aperte $i]
          if { $EsisteLineaAperta != -1 } {
            set quali_linee_aperte [lreplace $quali_linee_aperte \
              $EsisteLineaAperta end [lrange $quali_linee_aperte \
              [expr $EsisteLineaAperta +1] end]]
          }
        }
        # Setto le linee_aperte alla riga in cui sto eseguendo l'and
	set linee_aperte [expr $riga_and - $riga_corrente]
      }
    } elseif {$istruzione == "OR" || $istruzione == "ORN" || $istruzione == "OR("} {
      incr linee_aperte 
      if { [expr $riga_corrente + $linee_aperte] > $max_riga } {
        set max_riga [expr $riga_corrente + $linee_aperte]
      } elseif {$colonna_or == 0} {
        incr max_riga
      }
      # Se non sono all' interno di un AND(
      if { $colonna_or == 0 } {
        # Faccio partire gli OR dall'inizio e le linee aperte dalla massima
        set linee_aperte [expr $max_riga - $riga_corrente]
#puts "max $max_riga riga corrente $riga_corrente linee aperte $linee_aperte"
        set colonne_aperte([expr $riga_corrente + $linee_aperte]) 0
        # Serve a gestire il caso OR(
        #                         LD FC1
        if { $istruzione == "OR(" && $operando == "" } {
          incr index_linea
          set linea_tmp [$t get $index_linea.0 $index_linea.end]
          set linea [lappend linea [lindex $linea_tmp 1]] 
          if { [lindex $linea_tmp 0] == "LD" } {
            LdInstr $w [expr $riga_corrente + $linee_aperte] \
              $colonne_aperte([expr $riga_corrente + $linee_aperte]) \
              [lindex $linea 1 ] [lindex $linea 0 ] $index_linea 
          } elseif { [lindex $linea_tmp 0] == "LDN" } {
            LdInstr $w [expr $riga_corrente + $linee_aperte] \
            $colonne_aperte([expr $riga_corrente + $linee_aperte]) \
            [lindex $linea 1 ] LDN $index_linea 
          }
        } else {
          LdInstr $w [expr $riga_corrente + $linee_aperte] \
          $colonne_aperte([expr $riga_corrente + $linee_aperte]) $operando $istruzione \
          $index_linea 
	}
        set colonne_aperte([expr $riga_corrente + $linee_aperte]) \
          [expr $colonne_aperte([expr $riga_corrente + $linee_aperte]) +1]
      } else {
        # Faccio partire l'or dalla colonna in cui si e' verificato l' AND(
	if [info exists colonne_aperte([expr $riga_corrente + $linee_aperte])] {
	  # Sulla stessa riga in cui voglio mettere l'OR c'e' gia' un contatto
	  # Serve a gestire il caso OR(
          #                         LD FC1
          if { [expr $colonne_aperte([expr $riga_corrente + $linee_aperte]) -1] == \
               $colonna_or } {
            incr max_riga
            set linee_aperte [expr $max_riga - $riga_corrente]
          }
          if { $istruzione == "OR(" && $operando == "" } {
            incr index_linea
            set linea_tmp [$t get $index_linea.0 $index_linea.end]
            set linea [lappend linea [lindex $linea_tmp 1]] 
            if { [lindex $linea_tmp 0] == "LD" } {
              OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                [lindex $linea 1 ] [lindex $linea 0 ] $index_linea 
            } elseif { [lindex $linea_tmp 0] == "LDN" } {
              OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                [lindex $linea 1 ] ORN $index_linea
            }
          } else {
            OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                $operando $istruzione $index_linea  
	  }
	} else {
	  # Serve a gestire il caso OR(
          #                         LD FC1
          if { $istruzione == "OR(" && $operando == "" } {
            incr index_linea
            set linea_tmp [$t get $index_linea.0 $index_linea.end]
            set linea [lappend linea [lindex $linea_tmp 1]] 
            if { [lindex $linea_tmp 0] == "LD" } {
              OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                [lindex $linea 1 ] [lindex $linea 0 ] $index_linea 
            } elseif { [lindex $linea_tmp 0] == "LDN" } {
              OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                [lindex $linea 1 ]  ORN $index_linea 
            }
          } else {
            OrInstr $w $riga_and [expr $riga_corrente + $linee_aperte] $colonna_or \
                $operando $istruzione $index_linea 
          }
	}
        set colonne_aperte([expr $riga_corrente + $linee_aperte]) [expr $colonna_or + 1]
      }
      if { [lindex $linea 0 ] == "OR(" } {
        set riga_and [expr $riga_corrente + $linee_aperte]
        set nesting [expr $nesting +1]
        set riga_colonna_nest($nesting) [list $riga_and $colonna_or]
      }
      set quali_linee_aperte [lappend quali_linee_aperte \
         [expr $riga_corrente + $linee_aperte]]
    } elseif {$istruzione == "ST" || $istruzione == "STN" || $istruzione == "S" || \
              $istruzione == "R"} {
      if { $linee_aperte > 0 } {
        set max_col $colonne_aperte($riga_corrente)
        for {set i [expr $riga_corrente + 1]} \
            { $i <= [expr $riga_corrente + $linee_aperte] } { incr i} {
	  if { $colonne_aperte($i) > $max_col } {
	    set max_col $colonne_aperte($i)
          }
        }
        for {set i $riga_corrente } \
            { $i <= [expr $riga_corrente + $linee_aperte] } { incr i} {
          # Se e' nella lista delle linee aperte
          if { [lsearch -exact $quali_linee_aperte $i] != -1 } {
            DrawOp $w $i [expr $colonne_aperte($i) -1] $riga_corrente $max_col "illd"
          }
	}
	StInstr $w $linee_aperte $riga_corrente $max_col $operando $istruzione \
                $index_linea 
        # Reset generale per l'equazione successiva
        set linee_aperte 0
        set colonne_aperte($riga_corrente) 0
      } else {
        if { $colonne_aperte($riga_corrente) != 0 } {
	  StInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                $operando $istruzione $index_linea 
          # Reset generale per l'equazione successiva
          set linee_aperte 0
          set colonne_aperte($riga_corrente) 0
	} else {
          if {$ContactDef($canvasCorrente,col_fine_equazione) == 0 } {
	    StInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                    $operando $istruzione $index_linea 
            # Reset generale per l'equazione successiva
            set linee_aperte 0
            set colonne_aperte($riga_corrente) 0
          } else {
            StMultiplo $w $riga_corrente $linea $index_linea
          }
	}
      }
    } elseif {$istruzione == "JMP" || $istruzione == "JMPC" || $istruzione == "JMPCN" } {
      if { $linee_aperte > 0 } {
        set max_col $colonne_aperte($riga_corrente)
        for {set i [expr $riga_corrente + 1]} \
            { $i <= [expr $riga_corrente + $linee_aperte] } { incr i} {
	  if { $colonne_aperte($i) > $max_col } {
	    set max_col $colonne_aperte($i)
          }
        }
        for {set i $riga_corrente } { $i <= [expr $riga_corrente + $linee_aperte] } \
            { incr i} {
          # Se e' nella lista delle linee aperte
          if { [lsearch -exact $quali_linee_aperte $i] != -1 } {
            DrawOp $w $i [expr $colonne_aperte($i) -1] $riga_corrente $max_col "illd"
          }
	}
	JmpInstr $w $linee_aperte $riga_corrente $max_col $operando $istruzione 
        # Reset generale per l'equazione successiva
        set linee_aperte 0
        set colonne_aperte($riga_corrente) 0
      } else {
        if { $colonne_aperte($riga_corrente) != 0 } {
	  JmpInstr $w $linee_aperte $riga_corrente $colonne_aperte($riga_corrente) \
                   $operando $istruzione 
          # Reset generale per l'equazione successiva
          set linee_aperte 0
          set colonne_aperte($riga_corrente) 0
	} else {
          JmpMultiplo $w $riga_corrente $operando $istruzione 
	}
      }
      if { [lindex $linea 0 ] == "JMP" } {
        incr max_riga
      }
    } elseif {$istruzione == "XOR" || $istruzione == "XORN"} {
       #Allineo tutte le righe aperte
       for {set i $riga_corrente } { $i <= [expr $riga_corrente + $linee_aperte] } \
           { incr i} {
         # Se e' nella lista delle linee aperte
         if { [lsearch -exact $quali_linee_aperte $i] != -1 } {
           # il -1 e' dato dal fatto che ho gia' incrementato l'indice di colonna
           DrawOp $w $i [expr $colonne_aperte($i) -1] $riga_corrente \
                  $colonne_aperte($riga_corrente) "illd"
         }
       }
       # Disegno il box
       set BLOCK(title) $istruzione
       set BLOCK(input) 2
       set BLOCK(in0) [list " " 0]
       set BLOCK(in1) [list " " $linee_aperte]
       set BLOCK(out0) [list "" 0]
       set BLOCK(out1) [list " " 0]
       DrawRect $w $riga_corrente $colonne_aperte($riga_corrente) $index_linea 
       # Scrivi l'operando
       incr linee_aperte
       incr max_riga
       LdInstr $w [expr $riga_corrente + $linee_aperte] 0 $operando $istruzione \
         $index_linea 
       DrawOp $w [expr $riga_corrente + $linee_aperte] 0 \
         [expr $riga_corrente + $linee_aperte] $colonne_aperte($riga_corrente) "illd"
       set colonne_aperte([expr $riga_corrente + $linee_aperte]) \
         [expr $colonne_aperte($riga_corrente) + 2 ]
       set riga_corrente [expr $riga_corrente + $linee_aperte]
       set riga_and $riga_corrente
       set linee_aperte 0
       DrawOp $w $riga_corrente [expr $colonne_aperte($riga_corrente) - 1] \
         $riga_corrente $colonne_aperte($riga_corrente) "illd"
    } elseif {[lindex $linea 0 ] == "CAL" } {
       if { $max_riga >= $ContactDef($canvasCorrente,multiple_st) } {
         set max_riga [expr $max_riga + [DefineRect $w $operando \
           [expr $max_riga + 1] 0 $index_linea]]
       } else {
         # vado a scrivere dopo gli ST
         set max_riga [expr $ContactDef($canvasCorrente,multiple_st) + 2]
         set max_riga [expr $max_riga + [DefineRect $w $operando \
           [expr $max_riga + 1] 0 $index_linea]]
       }
    } elseif {[lindex $linea 0 ] == ")" } {
      if { $nesting > 0 } {
        set nesting [expr $nesting - 1]
      }
      set riga_and [lindex $riga_colonna_nest($nesting) 0] 
      set colonna_or [lindex $riga_colonna_nest($nesting) 1]
    }
  }
  if { $max_riga  >= [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] } {
    set ContactDef($canvasCorrente,riga_fine_edt) $max_riga 
  } elseif { $max_riga < \
             [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] } {
    set ContactDef($canvasCorrente,riga_fine_edt) \
      [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] 
  }
  DrawBarra $w

  if { [lindex $RigaColonnaCommento 0] != -1 } {
    if { [lindex $RigaColonnaCommento 1] > \
         $ContactDef($canvasCorrente,col_fine_equazione) } {
      set numero_colonne [lindex $RigaColonnaCommento 1]
    } else {
      set numero_colonne $ContactDef($canvasCorrente,col_fine_equazione)
    }
  } else {
    set numero_colonne $ContactDef($canvasCorrente,col_fine_equazione)
  }

  if { $max_riga  >= \
       [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] } {
    return [list $max_riga $numero_colonne]
  } elseif { $max_riga < \
             [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] } {
    return [list [expr $riga_corrente + $ContactDef($canvasCorrente,multiple_st)] \
      $numero_colonne]
  }
}

####################################################################
# Procedura WriteIlEquation
# Procedura che scrive l'equazione non traducibile
# Input: 
#   canvas            Nome del widget cancas
#   text              Nome del widget di testo
#   inizio_equazione  Riga di inizio equazione
#   fine_equazione    Riga di fine equazione
#   testo             Testo dell' equazione
# Ritorna :
#   Niente
####################################################################
proc WriteIlEquation { canvas t inizio_equazione fine_equazione testo} {
global EditPrefs ContactDef

  if { $inizio_equazione != -1 && $fine_equazione != -1 } {
    # Vado alla ricerca della linea di fine segmento precedente
    # includendo il caso di commenti intersegmento
    set index_linea [expr $inizio_equazione - 1]
    set linea [$t get $index_linea.0 $index_linea.end]
    while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "") && ($index_linea >=0)} {
      incr inizio_equazione -1
      incr index_linea -1
      set linea [$t get $index_linea.0 $index_linea.end]
    }
    # Vado alla ricerca della linea di fine segmento precedente
    # includendo il caso di commenti intersegmento
    set index_linea $fine_equazione 
    set linea [$t get $index_linea.0 $index_linea.end]
    while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "" || \
            [lindex $linea 0] == "END_PROGRAM") && ($index_linea >=0)} {
      incr fine_equazione -1
      incr index_linea -1
      set linea [$t get $index_linea.0 $index_linea.end]
    }
  }

  if { $testo != "" } {
    $canvas create text [expr $ContactDef(offset_x) * 2] $ContactDef(offset_y) \
      -anchor nw -text $testo -font $EditPrefs(textfont) -tag equation_il
  } else {
    $canvas create text [expr $ContactDef(offset_x) * 2] $ContactDef(offset_y) \
      -anchor nw -text [$t get $inizio_equazione.0 $fine_equazione.end] \
      -font $EditPrefs(textfont) -tag equation_il
  }
  
  set contorno [$canvas bbox equation_il]
  return [list [expr ([lindex $contorno 3] - \
     [lindex $contorno 1])/$ContactDef(height_conn)] \
     [expr ([lindex $contorno 2] - \
     [lindex $contorno 0])/($ContactDef(width_conn)+$ContactDef(width_op))]] 
}

####################################################################
# Procedura AnalizeIlEquation
# Procedura che analizza il tipo di equazione e chiama
# la funzione adatta per la conversione
# Input: 
#   canvas            Nome del widget cancas
#   text              Nome del widget di testo
#   QualeEquazione    Indice dell'equazione da esaminare
# Ritorna :
#   Niente
####################################################################
proc AnalizeIlEquation { canvas text QualeEquazione } {
global ContactDef MachineDataList

  set canvasCorrente [lindex [split $canvas .] end]
  set inizio_equazione [lindex $ContactDef(linee_equazioni) $QualeEquazione]
  set fine_equazione [expr [lindex $ContactDef(linee_equazioni) \
    [expr $QualeEquazione + 1]] - 1]
  if { [lindex $ContactDef(linee_equazioni_FB) $QualeEquazione] == 1} {
    # Se l'indice di inizio equazione coincide con un indice di inizio FB
    set ContactDef($canvasCorrente,language) ladder
    return [DefineInoutRect $canvas $text $inizio_equazione $fine_equazione]
  }

  set equazione_temporanea [$text get $inizio_equazione.0 $fine_equazione.end]
  set lista_noconv [list ADD( ADD_REAL( SUB( SUB_REAL( MUL( MUL_REAL( DIV( DIV_REAL( \
      GT( GT_REAL( GE( GE_REAL( EQ( EQ_REAL( NE( NE_REAL( LE( LE_REAL( LT( LT_REAL( \
      XOR( XORN( ANDN( ORN( NOT]
  set lista_noconv [concat $lista_noconv $MachineDataList]

  set lista_math [list ADD ADD_REAL SUB SUB_REAL MUL MUL_REAL DIV DIV_REAL ABS ABS_REAL \
                       GT GT_REAL GE GE_REAL EQ EQ_REAL NE NE_REAL LE LE_REAL LT LT_REAL \
                       DINT_TO_REAL REAL_TO_DINT STRING_TO_DINT STRING_TO_REAL \
                       SHL SHR ROL ROR BITSET BITRES BITTEST BITFIELD \
                       SIN COS TAN ASIN ACOS ATAN SQRT EXP LOG LN TRUNC EXPT \
                       XOR XORN]

  set posizione_ld 0
  set posizione_math 0
  set posizione_assoluta 0 
  set posizione_st 0 
  set posizione_jmp 0 
  set TrovatoEtichetta 0

  foreach x [split $equazione_temporanea \n] {
    if { [lsearch -exact $lista_noconv [lindex $x 0]] != -1 } {
      # Se e' presente una istruzione matematica o di confronto innestata 
      # o di lettura dati macchina non converto
      set ContactDef($canvasCorrente,language) il
      return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
    }

    if { [string match {(\**} $x] == 0 && $x != ""} { 
      # memorizzo la posizione di LD o LDN
      if { [lindex $x 0 ] == "LD" || [lindex $x 0] == "LDN" || [lindex $x 1 ] == "LD" || \
           [lindex $x 1] == "LDN" } {
        set posizione_ld $posizione_assoluta
      }

      # memorizzo la posizione di una istruzione matematica
      if { [lsearch -exact $lista_math [lindex $x 0]] != -1 } {
        set posizione_math $posizione_assoluta
      }

      # nel caso in cui ci sia uno ST in mezzo alla equazione non converto
      if { [lindex $x 0 ] == "ST" || [lindex $x 0] == "STN" || \
           [lindex $x 0 ] == "JMPC" || [lindex $x 0] == "JMPCN" || \
           [lindex $x 0 ] == "JMP" || [lindex $x 0 ] == "S" || [lindex $x 0] == "R" } {
        if { [lindex $x 0 ] == "JMPC" || [lindex $x 0] == "JMPCN" || \
             [lindex $x 0 ] == "JMP" } {
          set posizione_jmp $posizione_assoluta
        }
        if { $posizione_st != 0 && [expr $posizione_assoluta - $posizione_st] > 1 } {
          return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
        } else {
          set posizione_st $posizione_assoluta
        } 
      } 
      if { $posizione_jmp != 0 && $posizione_st > $posizione_jmp} {
        return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
      }

      incr posizione_assoluta
      if { $TrovatoEtichetta == 0 || $TrovatoEtichetta == 1} {
        # Nel caso in cui dopo una etichetta ci sia una istruzione diversa da LD o ST
        # non converto
        if { [string range [lindex $x 0] end end] == ":" } {
          if { [lindex $x 1] == "" } {
            set TrovatoEtichetta 1
            continue
          } elseif { [lindex $x 1] != "LD" && [lindex $x 1] != "LDN" && \
                     [lindex $x 1] != "ST" && [lindex $x 1] != "STN" && \
                     [lindex $x 1] != "S" && [lindex $x 1] != "R" && \
                     [lindex $x 1] != "JMP" && [lindex $x 1] != "JMPC" && \
                     [lindex $x 1] != "JMPCN" } {
            return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
          }  else {
            set TrovatoEtichetta 2
            continue
          }
        }
        if { $TrovatoEtichetta == 1 && [lindex $x 0] != "LD" && \
             [lindex $x 0] != "LDN" && [lindex $x 0] != "ST" && \
             [lindex $x 0] != "STN" && [lindex $x 0] != "S" && [lindex $x 0] != "R" && \
             [lindex $x 0] != "JMP" && [lindex $x 0] != "JMPC" && \
             [lindex $x 0] != "JMPCN" && \
             [lindex $x 0] != "END_PROGRAM" && [lindex $x 0] != "END_FUNCTION_BLOCK"} {
          return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
        }
      }
    }
  }

  if { $posizione_math != 0 } {
    if { [expr $posizione_math - $posizione_ld] == 1 && \
         [expr $posizione_st - $posizione_math] == 1 } {
      set ContactDef($canvasCorrente,language) ladder
      set ContactDef(linee_equazioni_FB) \
         [lreplace $ContactDef(linee_equazioni_FB) $QualeEquazione $QualeEquazione 2] 
      return [DefineMathRect $canvas $text $inizio_equazione $fine_equazione]
    } else {
      set ContactDef($canvasCorrente,language) il
      return [WriteIlEquation $canvas $text $inizio_equazione $fine_equazione ""]
    }
  }
  set ContactDef($canvasCorrente,language) ladder
  return [WriteLadderEquation $canvas $text $inizio_equazione $fine_equazione]
}

####################################################################
# Procedura DimensionaCanvas
# Procedura per il dimensionamento del canvas
# Input: 
#   canvas            Nome del widget cancas
#   righe_occupate    Righe occupate dal canvas
#   colonne_occupate  Colonne occupate dal canvas
# Ritorna :
#   Niente
####################################################################
proc DimensionaCanvas { canvas righe_occupate colonne_occupate } {
global ContactDef

  $canvas configure -height [expr $ContactDef(height_conn)*($righe_occupate + 1.5) + \
     $ContactDef(offset_y)]
  $canvas configure -width \
   [expr ($ContactDef(width_conn)+$ContactDef(width_op))*($colonne_occupate + 1.5)]
}

####################################################################
# Procedura LdInstr
# Procedura che rappresenta l'istruzione di caricamento,
# viene utilizzata anche in caso di OR
# Input: 
#   w            Nome del widget cancas
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc LdInstr { w riga colonna variabile modificatore index_linea} {

  set canvasCorrente [lindex [split $w .] end]
  set text [winfo parent [winfo parent $w]].t
  global ContactDef LINE_CONTACT$canvasCorrente

  DrawOp $w $riga [expr $colonna - 1 ] $riga $colonna "illd"
  if { $modificatore == "LDN" || $modificatore == "ORN" || $modificatore == "XORN"} {
    if { [VerificaTipoVariabile $variabile "LDN" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT2.png" $variabile "illd" 
    } else {
      DrawCont $w $riga $colonna "CONT2R.png" $variabile "illd" 
    }
  } else {
    if { [VerificaTipoVariabile $variabile "LD" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT1.png" $variabile "illd" 
    } else {
      DrawCont $w $riga $colonna "CONT1R.png" $variabile "illd" 
    }
  }
  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list $riga $colonna]
}

####################################################################
# Procedura AndInstr
# Procedura che rappresenta l'istruzione AND
# Input: 
#   w            Nome del widget cancas
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc AndInstr { w linee_aperte riga colonna variabile modificatore index_linea } {

  set canvasCorrente [lindex [split $w .] end]
  global ContactDef LINE_CONTACT$canvasCorrente

  set text [winfo parent [winfo parent $w]].t

  if { $linee_aperte == 0 } {
    DrawOp $w $riga [expr $colonna -1] $riga $colonna "illd" 
  }

  if { $modificatore == "ANDN" } {
    if { [VerificaTipoVariabile $variabile "ANDN" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT2.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT2R.png" $variabile "illd"
    }
  } else {
    if { [VerificaTipoVariabile $variabile "AND" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT1.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT1R.png" $variabile "illd"
    }
  }
  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list $riga $colonna]
}

####################################################################
# Procedura OrInstr
# Procedura che rappresenta l'istruzione OR
# Input: 
#   w            Nome del widget cancas
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc OrInstr { w riga_and riga colonna variabile modificatore index_linea} {

  set canvasCorrente [lindex [split $w .] end]
  global ContactDef LINE_CONTACT$canvasCorrente 

  set text [winfo parent [winfo parent $w]].t

  DrawOp $w $riga_and [expr $colonna - 1] $riga $colonna "illd"

  if { $modificatore == "ORN" } {
    if { [VerificaTipoVariabile $variabile "ORN" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT2.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT2R.png" $variabile "illd"
    }
  } else {
    if { [VerificaTipoVariabile $variabile "OR" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT1.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT1R.png" $variabile "illd"
    }
  }
  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list $riga $colonna]
}

####################################################################
# Procedura DrawCoil
# Procedura che rappresenta l'istruzione di storage
# Input: 
#   w            Nome del widget cancas
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc DrawCoil { w riga colonna variabile modificatore } {
global ContactDef

  set canvasCorrente [lindex [split $w .] end]
  set text [winfo parent [winfo parent $w]].t

  if { $modificatore == "ST" } {
    if { [VerificaTipoVariabile $variabile "ST" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT3.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT3R.png" $variabile "illd"
    }
  } elseif { $modificatore == "STN" } {
    if { [VerificaTipoVariabile $variabile "STN" $canvasCorrente $text] == "BOOL" } {
      DrawCont $w $riga $colonna "CONT4.png" $variabile "illd"
    } else {
      DrawCont $w $riga $colonna "CONT4R.png" $variabile "illd"
    }
  } elseif { $modificatore == "S" } {
    DrawCont $w $riga $colonna "CONT5.png" $variabile "illd"
  } elseif { $modificatore == "R" } {
    DrawCont $w $riga $colonna "CONT6.png" $variabile "illd"
  }
}

####################################################################
# Procedura StInstr
# Procedura che rappresenta l'istruzione di storage
# Input: 
#   w            Nome del widget cancas
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc StInstr { w linee_aperte riga colonna variabile modificatore index_linea } {

  set canvasCorrente [lindex [split $w .] end]
  global ContactDef LINE_CONTACT$canvasCorrente
  
  if { $linee_aperte == 0 } {
    DrawOp $w $riga [expr $colonna -1] $riga $colonna "illd"
  }
  # disegna il coil
  DrawCoil  $w $riga $colonna $variabile $modificatore 
  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list $riga $colonna]
}

####################################################################
# Procedura StMultiplo
# Procedura che rappresenta piu' ST consecutivi
# Input: 
#   w             Nome del widget cancas
#   riga_iniziale Riga iniziale
#   linea         Linea di inizio ST
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc StMultiplo { w riga_iniziale linea index_linea} {
  set canvasCorrente [lindex [split $w .] end]
  global ContactDef LINE_CONTACT$canvasCorrente 

  set canvasCorrente [lindex [split $w .] end]
  DrawCoil $w [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
    [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] [lindex $linea 1] \
    [lindex $linea 0] 
  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list [expr $riga_iniziale + \
      $ContactDef($canvasCorrente,multiple_st) + 1] \
      [expr $ContactDef($canvasCorrente,col_fine_equazione) -1]]
  DrawOp $w $riga_iniziale [expr $ContactDef($canvasCorrente,col_fine_equazione) -2] \
        [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
       [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] "illd"
  incr ContactDef($canvasCorrente,multiple_st) 
}

####################################################################
# Procedura JmpInstr
# Procedura che rappresenta di jmp
# Input: 
#   w            Nome del widget cancas
#   linee_aperte ???
#   riga         Riga di inserimento contatto
#   colonna      Colonna inserimento contatto
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
#   index_linea  ???
# Ritorna :
#   Niente
####################################################################
proc JmpInstr { w linee_aperte riga colonna variabile modificatore } {
global ContactDef
  
  if { $linee_aperte == 0  && $modificatore != "JMP"} {
    DrawOp $w $riga [expr $colonna -1] $riga $colonna "illd"
  }

  if { $modificatore == "JMP" } {
    # disegna il coil
    DrawOp $w [expr $riga + $linee_aperte + 1] -1 [expr $riga + $linee_aperte + 1] \
      0 "illd"
    DrawCont $w [expr $riga + $linee_aperte + 1] 0 "JMPC.png" $variabile "illd"
  } elseif { $modificatore == "JMPC" } { 
    DrawCont $w $riga $colonna "JMPC.png" $variabile "illd"
  } elseif { $modificatore == "JMPCN" } { 
    DrawCont $w $riga $colonna "JMPCN.png" $variabile "illd"
  }
}

####################################################################
# Procedura JmpMultiplo
# Procedura che rappresenta jmp multipli
# Input: 
#   w             Nome del widget cancas
#   riga_iniziale Riga iniziale
#   variabile    Variabile associata al contatto
#   modificatore Modificatore associato al contatto
# Ritorna :
#   Niente
####################################################################
proc JmpMultiplo { w riga_iniziale variabile modificatore } {
  global ContactDef 

  set canvasCorrente [lindex [split $w .] end]
  if { $modificatore == "JMP" } {
    DrawCont  $w [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] 0 \
      "JMPC.png" $variabile "illd"
    DrawOp $w [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] -1 \
      [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] 0 "illd"
  } elseif { $modificatore == "JMPC" } { 
    DrawCont  $w [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
      [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] "JMPC.png" $variabile \
      "illd"
    DrawOp $w $riga_iniziale [expr $ContactDef($canvasCorrente,col_fine_equazione) -2] \
       [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
       [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] "illd"
    incr ContactDef($canvasCorrente,multiple_st) 
  } elseif { $modificatore == "JMPCN" } { 
    DrawCont  $w [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
      [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] "JMPCN.png" $variabile \
      "illd"
    DrawOp $w $riga_iniziale [expr $ContactDef($canvasCorrente,col_fine_equazione) -2] \
      [expr $riga_iniziale + $ContactDef($canvasCorrente,multiple_st) + 1] \
      [expr $ContactDef($canvasCorrente,col_fine_equazione) -1] "illd"
    incr ContactDef($canvasCorrente,multiple_st) 
  }
}

####################################################################
# Procedura EstraiCommento
# Procedura che cerca il commento alla equazione se c'e'
# Input: 
#   t             Nome del widget di testo
#   w             Nome del widget cancas
#   riga_iniziale Riga iniziale
# Ritorna :
#   Niente
####################################################################
proc EstraiCommento { t w riga_iniziale } {
global ContactComment

  set index_linea [expr $riga_iniziale - 1]
  set linea [$t get $index_linea.0 $index_linea.end]
  while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "") && ($index_linea >=0)} {
    incr index_linea -1
    set linea [$t get $index_linea.0 $index_linea.end]
  }
  if { $index_linea < [expr $riga_iniziale - 1] } {
    set testo_commento [$t get [expr $index_linea + 1].0 [expr $riga_iniziale - 1].end] 
    if { $testo_commento != "" } {
      if { [string compare "edit" [lindex [split $w .] end]] != 0 } {
        return [WriteIlEquation $w $t -1 -1 $testo_commento]
      } else {
        set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
          -title [GetString MsgCommTitle] -default $testo_commento] 
        return -1
      }
    } else {
      if { [string compare "edit" [lindex [split $w .] end]] == 0 } {
        set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
          -title [GetString MsgCommTitle]] 
      }
      return -1
    }
  } else {
    return -1
  }
}

