
            ######################################
            #                                    #
            #     File di definizione della      #
            #     configurazione ladder          #
            #                                    #
            ######################################


####################################################################
# Procedura ModalitaEdit
# Procedura che inserisce un segmento dopo il canvas chiamante
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc ModalitaEdit {canvas} {
global ContactDef

  focus $canvas.edit
  if [info exist ContactDef(ordine_seg)] {
    foreach canvasLadder $ContactDef(ordine_seg) {
      $canvas delete segment$canvasLadder
    }
  }
  $canvas yview moveto 0.0
}

####################################################################
# Procedura MostraLadder
# Procedura che ritorna in modalita' visiva
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc MostraLadder { canvas } {
global ContactDef 

  focus $canvas
  #set offset_y 50
  set offset_y 0
  set LarghezzaCanvas 0
  set AltezzaCanvas 0
  if { [info exist ContactDef(ordine_seg)] == 0 } {
    return
  }
  foreach canvasLadder $ContactDef(ordine_seg) {
    if [winfo exists $canvas.$canvasLadder] {
      $canvas delete segment$canvasLadder
      $canvas create window 10 $offset_y -anchor nw \
           -window $canvas.$canvasLadder -tag segment$canvasLadder
      set offset_y [lindex [$canvas bbox segment$canvasLadder] 3]
      set posizione_seg [$canvas bbox segment$canvasLadder]
      if { $LarghezzaCanvas < [lindex $posizione_seg 2]} {
        set LarghezzaCanvas [lindex $posizione_seg 2] 
      }
      if { $AltezzaCanvas < [lindex $posizione_seg 3]} {
        set AltezzaCanvas [lindex $posizione_seg 3] 
      }
    }
  }
  $canvas configure -scrollregion "0 0 $LarghezzaCanvas $AltezzaCanvas" 
  set PosizioneSegmento [$canvas coords segment[lindex $ContactDef(ordine_seg) \
       $ContactDef(SegmentoCorrente)]]
  if { $PosizioneSegmento != "" } {
    $canvas yview moveto [expr [lindex $PosizioneSegmento 1]/[lindex [lindex \
      [$canvas configure -scrollregion] 4] 3]] 
  }
}

####################################################################
# Procedura InserisciRiga
# Procedura che inserisce una nuova riga in basso
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc InserisciRiga {canvas} {
global ContactDef AlberoLadder ContactComment 

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,curr_riga) > 0 && \
       $ContactDef($canvasCorrente,curr_riga) <= \
       $ContactDef($canvasCorrente,riga_fine_edt) } { 
    foreach x [lsort -decreasing $AlberoLadder($canvasCorrente)] {
      set primosplit [split $x '|']
      set riga_iniz [lindex [split [lindex $primosplit 0] ':'] 0]
      set col_iniz [lindex [split [lindex $primosplit 0] ':'] 1]
      set riga_fin [lindex [split [lindex $primosplit 1] ':'] 0]
      set col_fin [lindex [split [lindex $primosplit 1] ':'] 1]
      if { $riga_iniz >= $ContactDef($canvasCorrente,curr_riga) && $riga_fin < \
           $ContactDef($canvasCorrente,curr_riga) } {
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        DrawOp $canvas [expr $riga_iniz + 1] $col_iniz $riga_fin $col_fin "illd"
      } elseif { $riga_iniz < $ContactDef($canvasCorrente,curr_riga) && $riga_fin >= \
                 $ContactDef($canvasCorrente,curr_riga) } {
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat [lrange $AlberoLadder($canvasCorrente) \
            0 [expr $indice -1]] [lrange $AlberoLadder($canvasCorrente) \
            [expr $indice + 1] end]]
        DrawOp $canvas $riga_iniz $col_iniz [expr $riga_fin + 1] $col_fin "illd"
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        DrawCont $canvas [expr $riga_fin +1] $col_fin $tipo_contatto $variabile "illd"
        $canvas delete cont$riga_fin:$col_fin
        $canvas delete var$riga_fin:$col_fin
        set ContactComment([expr $riga_fin +1]:$col_fin) \
            $ContactComment($riga_fin:$col_fin)
        set ContactComment($riga_fin:$col_fin) ""
      } elseif { $riga_iniz >= $ContactDef($canvasCorrente,curr_riga) && \
                 $riga_fin >= $ContactDef($canvasCorrente,curr_riga) } {
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat [lrange \
            $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        DrawOp $canvas [expr $riga_iniz + 1] $col_iniz [expr $riga_fin +1] $col_fin "illd"
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        DrawCont $canvas [expr $riga_fin +1] $col_fin $tipo_contatto $variabile "illd"
        $canvas delete cont$riga_fin:$col_fin
        $canvas delete var$riga_fin:$col_fin
        set ContactComment([expr $riga_fin +1]:$col_fin) \
            $ContactComment($riga_fin:$col_fin)
        set ContactComment($riga_fin:$col_fin) ""
      }
    }
    DimensionaCanvas $canvas $ContactDef($canvasCorrente,riga_fine_edt) \
      $ContactDef($canvasCorrente,col_fine_edt)
  }
}

####################################################################
# Procedura InserisciColonnaSinistra
# Procedura che inserisce una colonna a sinistra del contatto
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc InserisciColonnaSinistra {canvas} {
global ContactDef AlberoLadder ContactComment 

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,curr_colonna) >= 0 && \
       $ContactDef($canvasCorrente,curr_colonna) <= \
       $ContactDef($canvasCorrente,col_fine_edt) } { 
    set AlberoLadderTmp $AlberoLadder($canvasCorrente)
    foreach x [lsort -decreasing $AlberoLadderTmp] {
      set primosplit [split $x '|']
      set riga_iniz [lindex [split [lindex $primosplit 0] ':'] 0]
      set col_iniz [lindex [split [lindex $primosplit 0] ':'] 1]
      set riga_fin [lindex [split [lindex $primosplit 1] ':'] 0]
      set col_fin [lindex [split [lindex $primosplit 1] ':'] 1]
      if { $col_fin == "fine" } {
        # Cancello la vecchia connessione 
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat [lrange \
            $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
      } elseif { $riga_fin == $ContactDef($canvasCorrente,curr_riga) && \
                 $col_fin == $ContactDef($canvasCorrente,curr_colonna) } {
        # Se il collegamento finale nella colonna corrente
        # sposto solo il contatto e ne inserisco un'altro
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
            $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
          InserisciContatto $canvas CONT1.png 0
          DrawOp $canvas $riga_fin  $col_fin  $riga_fin [expr $col_fin + 1] "illd"
        }
      } elseif { $col_iniz >= $ContactDef($canvasCorrente,curr_colonna) } {
        # Se collegamento iniziale e finale sono dopo la colonna corrente
        # sposto entrambi
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
              $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
        }
        DrawOp $canvas $riga_iniz  [expr $col_iniz + 1] $riga_fin \
          [expr $col_fin + 1] "illd"
      } elseif { $col_iniz < $ContactDef($canvasCorrente,curr_colonna) && \
                 $col_fin >= $ContactDef($canvasCorrente,curr_colonna) } {
        # Se solo il collegamento finale e' dopo la colonna corrente
        # sposto solo il collegamento finale
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
              $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
        }
        DrawOp $canvas $riga_iniz  $col_iniz  $riga_fin [expr $col_fin + 1] "illd"
      } 
    }
  DimensionaCanvas $canvas $ContactDef($canvasCorrente,riga_fine_edt) \
    $ContactDef($canvasCorrente,col_fine_edt)
  }
}

####################################################################
# Procedura InserisciColonnaDestra
# Procedura che inserisce una colonna a destra del contatto
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc InserisciColonnaDestra {canvas} {
global ContactDef AlberoLadder ContactDef ContactComment 

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,curr_colonna) >= 0 && \
       $ContactDef($canvasCorrente,curr_colonna) <= \
       $ContactDef($canvasCorrente,col_fine_edt) } { 
    set AlberoLadderTmp $AlberoLadder($canvasCorrente)
    foreach x [lsort -decreasing $AlberoLadderTmp] {
      set primosplit [split $x '|']
      set riga_iniz [lindex [split [lindex $primosplit 0] ':'] 0]
      set col_iniz [lindex [split [lindex $primosplit 0] ':'] 1]
      set riga_fin [lindex [split [lindex $primosplit 1] ':'] 0]
      set col_fin [lindex [split [lindex $primosplit 1] ':'] 1]
      if { $col_fin == "fine" } {
        # Cancello la vecchia connessione 
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat [lrange \
            $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
      } elseif { $riga_iniz == $ContactDef($canvasCorrente,curr_riga) && \
                 $col_iniz == $ContactDef($canvasCorrente,curr_colonna) } {
        # Se il collegamento iniziale e' nella colonna corrente
        # sposto contatto e collegamento ed inserisco il nuovo contatto  
        # con relativo collegamento
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
              $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
        }
        DrawOp $canvas $riga_iniz  [expr $col_iniz + 1]  $riga_fin \
          [expr $col_fin + 1] "illd"
      } elseif { $col_iniz > $ContactDef($canvasCorrente,curr_colonna) } {
        # Se collegamento iniziale e finale sono dopo la colonna corrente
        # sposto entrambi
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
              $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
        }
        DrawOp $canvas $riga_iniz  [expr $col_iniz + 1] $riga_fin \
          [expr $col_fin + 1] "illd"
      } elseif { $col_iniz <= $ContactDef($canvasCorrente,curr_colonna) && \
                 $col_fin > $ContactDef($canvasCorrente,curr_colonna) } {
        # Se solo il collegamento finale e' dopo la colonna corrente
        # sposto solo il collegamento finale
        # Cancello le vecchie connessioni e faccio le nuove
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder($canvasCorrente) $x]
        set AlberoLadder($canvasCorrente) [concat \
            [lrange $AlberoLadder($canvasCorrente) 0 [expr $indice -1]] \
            [lrange $AlberoLadder($canvasCorrente) [expr $indice + 1] end]]
        # Cancello il vecchio contatto e scrivo il nuovo
        set tipo_contatto [file tail [$canvas itemconfig cont$riga_fin:$col_fin -bitmap]]
        set variabile [lindex [$canvas itemconfigure var$riga_fin:$col_fin -text] 4]
        if { $tipo_contatto != "" && $riga_iniz <= $riga_fin} {
          DrawCont $canvas $riga_fin [expr $col_fin +1] $tipo_contatto $variabile "illd"
          $canvas delete cont$riga_fin:$col_fin
          $canvas delete var$riga_fin:$col_fin
          set ContactComment($riga_fin:[expr $col_fin +1]) \
            $ContactComment($riga_fin:$col_fin)
          set ContactComment($riga_fin:$col_fin) ""
        }
        DrawOp $canvas $riga_iniz  $col_iniz  $riga_fin [expr $col_fin + 1] "illd"
      } 
    }
  scrollRight $canvas
  InserisciContatto $canvas CONT1.png 0
  DrawOp $canvas $ContactDef($canvasCorrente,curr_riga) \
     [expr $ContactDef($canvasCorrente,curr_colonna) -1]  \
     $ContactDef($canvasCorrente,curr_riga) $ContactDef($canvasCorrente,curr_colonna) \
     "illd"
  DimensionaCanvas $canvas $ContactDef($canvasCorrente,riga_fine_edt) \
     $ContactDef($canvasCorrente,col_fine_edt)
  }
}

####################################################################
# Procedura CancellaConnessione
# Procedura che cancella tutte le connessioni sulla destra del contatto
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc CancellaConnessione { canvas } {
global ContactDef AlberoLadder

  # Cancello dall'albero le connessioni
  foreach connessioni $AlberoLadder(edit) {
    if { [string first $ContactDef(edit,curr_riga):$ContactDef(edit,curr_colonna)| \
         $connessioni] != -1 } {
      # Non cancello la connessione finale
      if { [string first "fine" $connessioni] == -1 } {
        $canvas delete conn$connessioni
        set indice [lsearch -exact $AlberoLadder(edit) $connessioni]
        set AlberoLadder(edit) [concat [lrange $AlberoLadder(edit) 0 \
            [expr $indice -1]] [lrange $AlberoLadder(edit) [expr $indice + 1] end]]
      }
    }  
  }
}

####################################################################
# Procedura SmistaComandoPalette
# Procedura che sulla scrollIn si fa una azione in funzione della
# palette selezionata
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc SmistaComandoPalette { canvas } {
global BottoneAttivo ContactDef box 

  set canvasCorrente [lindex [split $canvas .] end]
  if { [string first "cont" $BottoneAttivo] != -1 || \
       [string first "jmpc" $BottoneAttivo] != -1 } {
    set contact [string toupper $BottoneAttivo]
    set contact [string trimright $contact "B"].png
    InserisciContatto $canvas $contact 1
  } elseif { [string first "addcoll" $BottoneAttivo] != -1 } {
    InserisciColonnaSinistra $canvas
  } elseif { [string first "addcolr" $BottoneAttivo] != -1 } {
    InserisciColonnaDestra $canvas
  } elseif { [string first "addrow" $BottoneAttivo] != -1 } {
    InserisciRiga $canvas
  } elseif { [string first "delconn" $BottoneAttivo] != -1 } {
    CancellaConnessione $canvas
  } elseif { [string first "colleg" $BottoneAttivo] != -1 } {
    if { [lindex $box(init_coll) 0] == -2 } {
      $canvas config -cursor pencil
      $canvas itemconfig cursore -outline red 
      set box(init_coll) [list $ContactDef($canvasCorrente,curr_riga) \
          $ContactDef($canvasCorrente,curr_colonna)]
    } else {
      set box(end_coll) [list $ContactDef($canvasCorrente,curr_riga) \
          $ContactDef($canvasCorrente,curr_colonna)]
      DrawOp $canvas -1 -1 -1 -1 {ldil} 
    }
  }
}

####################################################################
# Procedura ScrollaLadder
# Procedura che fa navigare all' interno del ladder
# Input: 
#   canvas     Nome del widget canvas
#   tiposcroll Definisce il tipo di scroll
# Ritorna :
#   Niente
####################################################################
proc ScrollaLadder { canvas tiposcroll } {
global ContactDef

  set canvasCorrente [lindex [split $canvas .] end]
  set coordinate [$canvas coords cursore]
  set indicex \
   [expr ($ContactDef($canvasCorrente,curr_colonna).0 + 1)/$ContactDef(max_colonna)]
  set indicey [expr ($ContactDef($canvasCorrente,curr_riga).0 + 1)/$ContactDef(max_riga)]
  set indicixcanvas [$canvas xview]
  set indiciycanvas [$canvas yview]
  if { $tiposcroll == "giu" } {
    if { $indicey > [lindex $indiciycanvas 1] } {
      $canvas yview moveto [expr [lindex $indiciycanvas 0] + 1.0/$ContactDef(max_riga)]
    }
  } elseif { $tiposcroll == "su" } {
    if { $indicey < [lindex $indiciycanvas 0] } {
      $canvas yview moveto [expr [lindex $indiciycanvas 0] - 1.0/$ContactDef(max_riga)]
    }
  } elseif { $tiposcroll == "destra" } {
    if { $indicex > [lindex $indicixcanvas 1] } {
      $canvas xview moveto [expr [lindex $indicixcanvas 0] + 1.0/$ContactDef(max_colonna)]
    }
  } elseif { $tiposcroll == "sinistra" } {
    if { $indicex < [lindex $indicixcanvas 0] } {
      $canvas xview moveto [expr [lindex $indicixcanvas 0] - 1.0/$ContactDef(max_colonna)]
    }
  }
}

####################################################################
# Procedura DefineVariable
# Procedura che associa la variabile al contatto al doppio  clic
# Input: 
#   canvas     Nome del widget canvas
#   tiposcroll Definisce il tipo di scroll
# Ritorna :
#   Niente
####################################################################
proc DefineVariable canvas {
global ContactDef ContactComment 

  focus $canvas
  set canvasCorrente [lindex [split $canvas .] end]
  if { [$canvas gettags \
      cont$ContactDef($canvasCorrente,curr_riga):$ContactDef($canvasCorrente,curr_colonna)] != "" } {
    set NomeVariabileTmp [string toupper [PromptBox -text PromptVariableName \
        -text2 PromptLineComment \
       -default2 $ContactComment($ContactDef($canvasCorrente,curr_riga):$ContactDef($canvasCorrente,curr_colonna)) \
       -title PromptVarName -numentry 2 -uppercase 1]]
    if {[lindex $NomeVariabileTmp 0] != "" } {
      set NomeVariabile [VerificaOperando [winfo parent $canvas].t "LD" \
          [lindex $NomeVariabileTmp 0] 0]
      set commento [lindex $NomeVariabileTmp 1]
      if { $commento != "" } {
        if { [string first "(*" $commento ] == -1 } {
          set commento "(* $commento"
        }
        if { [string first "*)" $commento ] == -1 } {
          set commento "$commento *)"
        }
      }
      set ContactComment($ContactDef($canvasCorrente,curr_riga):$ContactDef($canvasCorrente,curr_colonna)) $commento 
      $canvas itemconfigure var$ContactDef($canvasCorrente,curr_riga):$ContactDef($canvasCorrente,curr_colonna) -text $NomeVariabile
    }
  }
}

####################################################################
# Procedura scrollIn
# Procedura che disegna il rettangolo al click
# Input: 
#   canvas     Nome del widget canvas
#   x          Coordinate x
#   y          Coordinate y
#   TipoScroll Definisce il tipo di scroll
# Ritorna :
#   Niente
####################################################################
proc scrollIn {canvas x y TipoScroll} {
global ContactDef BottoneAttivo box EditPrefs 

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,language) == "ladder" } {
    focus $canvas 
    # La x e la y sono relative alla parte visualizzata
    # necessario calcolare l'offset
    set off_x [expr $EditPrefs(ladderwidth) * [lindex [$canvas xview] 0]]
    set x round([expr $x + $off_x])
    set off_y [expr $EditPrefs(ladderheight) * [lindex [$canvas yview] 0]]
    set y round([expr $y + $off_y])
    set riga [expr ($y - $ContactDef(offset_y))/$ContactDef(height_conn)]
    set colonna [expr ($x - $ContactDef(offset_x))/($ContactDef(width_op) + \
        $ContactDef(width_conn))]
    $canvas move cursore [expr ($colonna - $ContactDef($canvasCorrente,curr_colonna)) * \
        ($ContactDef(width_op) + $ContactDef(width_conn))] \
        [expr ($riga - $ContactDef($canvasCorrente,curr_riga)) * $ContactDef(height_conn)]
    set ContactDef($canvasCorrente,curr_riga) $riga
    set ContactDef($canvasCorrente,curr_colonna) $colonna
    $canvas raise cursore
    if { $TipoScroll == "Single" && [string first "colleg" $BottoneAttivo] != -1} {
      $canvas config -cursor pencil
      $canvas itemconfig cursore -outline red 
      set box(init_coll) [list $ContactDef($canvasCorrente,curr_riga) \
        $ContactDef($canvasCorrente,curr_colonna)]
    } elseif { $TipoScroll == "Double" } {
      SmistaComandoPalette $canvas
    }
  }
}

####################################################################
# Procedura scrollDrag 
# Procedura che sposta il rettangolo al muoversi del mouse
# Input: 
#   canvas     Nome del widget canvas
#   x          Coordinate x
#   y          Coordinate y
# Ritorna :
#   Niente
####################################################################
proc scrollDrag { w x y } {
global EditPrefs ContactDef box

  focus $w 
  set canvasCorrente [lindex [split $w .] end]
  # La x e la y sono relative alla parte visualizzata
  # necessario calcolare l'offset
  set off_x [expr $EditPrefs(ladderwidth) * [lindex [$w xview] 0]]
  set x round([expr $x + $off_x])
  set off_y [expr $EditPrefs(ladderheight) * [lindex [$w yview] 0]]
  set y round([expr $y + $off_y])
  set riga [expr ($y - $ContactDef(offset_y))/$ContactDef(height_conn)]
  set colonna [expr ($x - $ContactDef(offset_x))/($ContactDef(width_op) + \
      $ContactDef(width_conn))]
  $w move cursore [expr ($colonna - $ContactDef($canvasCorrente,curr_colonna)) * \
      ($ContactDef(width_op) + $ContactDef(width_conn))] \
      [expr ($riga - $ContactDef($canvasCorrente,curr_riga)) * $ContactDef(height_conn)]
  set ContactDef($canvasCorrente,curr_riga) $riga
  set ContactDef($canvasCorrente,curr_colonna) $colonna
  $w raise cursore
  set box(end_coll) [list $ContactDef($canvasCorrente,curr_riga) \
      $ContactDef($canvasCorrente,curr_colonna)]
}

####################################################################
# Procedura scrollUp
# Procedura che sposta il cursore sopra
# Input: 
#   canvas     Nome del widget canvas
#   type       Ha come valori edit o show
#   args       ????
# Ritorna :
#   Niente
####################################################################
proc scrollUp { canvas type args} {
  global ContactDef BottoneAttivo DebugState

  set text [winfo parent $canvas].t
  if { $type == "edit" } {
    set canvasCorrente [lindex [split $canvas .] end]
    if { $ContactDef($canvasCorrente,curr_riga) > 0 } {
      set ContactDef($canvasCorrente,curr_riga) \
        [expr $ContactDef($canvasCorrente,curr_riga) - 1] 
      $canvas move cursore 0 [expr -($ContactDef(height_conn))]
    }
    $canvas raise cursore
    ScrollaLadder $canvas "su"
  } elseif { $type == "show" } {
    if { $ContactDef(SegmentoCorrente) > 0 } {
      ProtocolloAttivaDebug $text LADDER1

      $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
         configure -background white
      if {[expr $ContactDef(SegmentoCorrente) - 1] >= 0 && \
          [winfo exists $canvas.[lindex $ContactDef(ordine_seg) \
          [expr $ContactDef(SegmentoCorrente) - 1]]] == 0 } {
        ScriviEquazione [lindex $args 0] $canvas [lindex $args 1] \
          [expr $ContactDef(SegmentoCorrente) - 1] 
        MostraLadder $canvas
      } 
      incr ContactDef(SegmentoCorrente) -1
      set PosizioneSegmento [$canvas coords segment[lindex $ContactDef(ordine_seg) \
        $ContactDef(SegmentoCorrente)]]
      $canvas yview moveto [expr [lindex $PosizioneSegmento 1]/[lindex [lindex \
         [$canvas configure -scrollregion] 4] 3]] 
      DisplayEquationNumber [winfo parent $canvas].status.position \
        [lindex $ContactDef(linee_equazioni) $ContactDef(SegmentoCorrente)] 1
      $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
        configure -background lemonchiffon

      ProtocolloAttivaDebug $text LADDER2
    }
  }
}

####################################################################
# Procedura scrollDown
# Procedura che sposta il cursore sotto
# Input: 
#   canvas     Nome del widget canvas
#   type       Ha come valori edit o show
#   args       ????
# Ritorna :
#   Niente
####################################################################
proc scrollDown { canvas type args} {
  global ContactDef BottoneAttivo EditPrefs DebugState 

  set text [winfo parent $canvas].t
  if { $type == "edit" } {
    set canvasCorrente [lindex [split $canvas .] end]
    if { $ContactDef($canvasCorrente,curr_riga) < [expr $ContactDef(max_riga) - 1] } {
      incr ContactDef($canvasCorrente,curr_riga) 
      $canvas move cursore 0 $ContactDef(height_conn)
    }
    $canvas raise cursore
    ScrollaLadder $canvas "giu"
  } elseif { $type == "show" } {
    if { $ContactDef(SegmentoCorrente) < [expr [llength $ContactDef(ordine_seg)] -1] } {
      ProtocolloAttivaDebug $text LADDER1
      $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
         configure -background white
      if {[expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] <= \
          [expr [llength $ContactDef(ordine_seg)] -1] && \
          [winfo exists $canvas.[lindex $ContactDef(ordine_seg) \
          [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)]]] == 0 } {
        ScriviEquazione [lindex $args 0] $canvas [lindex $args 1] \
          [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] 
      } 
      incr ContactDef(SegmentoCorrente)
      set PosizioneSegmento [$canvas coords segment[lindex $ContactDef(ordine_seg) \
        $ContactDef(SegmentoCorrente)]]
      $canvas yview moveto [expr [lindex $PosizioneSegmento 1]/[lindex [lindex \
         [$canvas configure -scrollregion] 4] 3]] 
      DisplayEquationNumber [winfo parent $canvas].status.position \
         [lindex $ContactDef(linee_equazioni) $ContactDef(SegmentoCorrente)] 1
      $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
         configure -background lemonchiffon

      ProtocolloAttivaDebug $text LADDER2
    }
  }
}

####################################################################
# Procedura scrollRight
# Procedura che sposta il cursore a destra
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc scrollRight { canvas } {
global ContactDef BottoneAttivo

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,curr_colonna) < [expr $ContactDef(max_colonna) - 1] } {
    incr ContactDef($canvasCorrente,curr_colonna) 
    $canvas move cursore [expr $ContactDef(width_conn) + $ContactDef(width_op)] 0 
  }
  $canvas raise cursore
  ScrollaLadder $canvas "destra"
}

####################################################################
# Procedura scrollLeft
# Procedura che sposta il cursore a sinistra
# Input: 
#   canvas     Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc scrollLeft { canvas } {
global ContactDef BottoneAttivo

  set canvasCorrente [lindex [split $canvas .] end]
  if { $ContactDef($canvasCorrente,curr_colonna) > 0 } {
    set ContactDef($canvasCorrente,curr_colonna) \
      [expr $ContactDef($canvasCorrente,curr_colonna) - 1] 
    $canvas move cursore [expr -($ContactDef(width_conn) + $ContactDef(width_op))] 0
  }
  $canvas raise cursore
  ScrollaLadder $canvas "sinistra"
}

####################################################################
# Procedura CancellaCommento
# Procedura che cancella il commento per ogni istruzione 
# Input: 
#   main     Nome del widget main
#   w        Nome del widget canvas
#   x        Coordinata x
#   y        Coordinata y
# Ritorna :
#   Niente
####################################################################
proc CancellaCommento { main w x y } {
global ContactDef

  $main.status.name  configure -text $ContactDef(nomefile) 
  $w delete recttype
}

####################################################################
# Procedura RicercaCommentiEdit
# Procedura che ricerca i commenti per ogni riga della equazione
# in editor
# Input: 
#   t       Nome del widget di testo
#   canvas  Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc RicercaCommentiEdit { t canvas } {
global ContactDef EditPrefs FunctionBlockList ContactComment

  set canvasCorrente [lindex [split $canvas .] end]
  global LINE_CONTACT$canvasCorrente

  foreach indice [array names LINE_CONTACT$canvasCorrente] {
    set CercaComm [$t get $indice.0 $indice.end]
    set InizioComm [string first "(*" $CercaComm]
    set CanvasTempo LINE_CONTACT$canvasCorrente
    append CanvasTempo "($indice)"
    set cmd "lindex \$$CanvasTempo 0"
    set cmd1 "lindex \$$CanvasTempo 1"
    if { $InizioComm != -1 } {
      set ContactComment([eval $cmd]:[eval $cmd1]) \
        [string range $CercaComm $InizioComm end]
    } else {
      set ContactComment([eval $cmd]:[eval $cmd1]) "" 
    }
  }
}

####################################################################
# Procedura ScriviCommento
# Procedura che ricerca il commento per ogni istruzione e lo
# visualizza
# Input: 
#   t       Nome del widget di testo
#   main    Nome del widget main
#   w       Nome del widget canvas
#   x       Coordinata x
#   y       Coordinata y
# Ritorna :
#   Niente
####################################################################
proc ScriviCommento { t main w x y } {
global ContactDef EditPrefs FunctionBlockList

  set canvasCorrente [lindex [split $w .] end]
  global LINE_CONTACT$canvasCorrente

  set contatto [lindex [$w gettags [$w find closest $x $y]] 0]
  if { [string first "cont" $contatto] != -1 } {
    set contatto [string trimleft $contatto "cont"]
    set riga [lindex [split $contatto :] 0]
    set colonna [lindex [split $contatto :] 1]
    set Linea -1
    foreach indice [array names LINE_CONTACT$canvasCorrente] {
      set CanvasTempo LINE_CONTACT$canvasCorrente
      append CanvasTempo "($indice)"
      set cmd "lindex \$$CanvasTempo 0"
      set cmd1 "lindex \$$CanvasTempo 1"
      if { $riga == [eval $cmd] && $colonna == [eval $cmd1] } {
        set Linea $indice
        break
      }  
    }
    if { $Linea != -1 } {
      set CercaComm [$t get $Linea.0 $Linea.end]
      set InizioComm [string first "(*" $CercaComm]
      if { $InizioComm != -1 } {
       $main.status.name configure -text [string range $CercaComm $InizioComm end] 
      }
    }
  } elseif { [string first "rect_" $contatto] != -1 || \
             [string first "rectin_" $contatto] != -1 || \
             [string first "rectout_" $contatto] != -1 || \
             [string first "recttitle_" $contatto] != -1 } {
    set contatto [string range $contatto [expr [string first "_" $contatto ] +1] end]
    set riga [lindex [split $contatto :] 0]
    set colonna [lindex [split $contatto :] 1]
    set Linea -1
    foreach indice [array names LINE_CONTACT$canvasCorrente] {
      set CanvasTempo LINE_CONTACT$canvasCorrente
      append CanvasTempo "($indice)"
      set cmd "lindex \$$CanvasTempo 0"
      set cmd1 "lindex \$$CanvasTempo 1"
      if { $riga == [eval $cmd] && $colonna == [eval $cmd1] } {
        set Linea $indice
        break
      }  
    }
    if { $Linea != -1 } {
      set CercaComm [$t get $Linea.0 $Linea.end]
      set InizioComm [string first "(*" $CercaComm]
      if { $InizioComm != -1 } {
       $main.status.name configure -text [string range $CercaComm $InizioComm end] 
      }
    }
    # Scrivo in caso di FB anche il tipo.
    set titlecoord [$w coords recttitle_$contatto]
     if { [info exist FunctionBlockList([lindex [$w itemconfigure recttitle_$contatto \
        -text] 4])] } {
      $w create text [lindex $titlecoord 0] \
        [expr [lindex $titlecoord 1] + 10] -text \
        $FunctionBlockList([lindex [$w itemconfigure recttitle_$contatto -text] 4]) \
        -font $EditPrefs(textfont) -tag recttype
      $w select from recttype 0
      $w select to recttype end
    }
  } 
}

####################################################################
# Procedura AggiornaCanvasCorrente
# Procedura che aggiorna il canvas al click del mouse
# Input: 
#   w       Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc AggiornaCanvasCorrente { w } { 
global ContactDef EditPrefs

  [winfo parent $w].[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] \
     configure -background white 
  set CanvasCorrente [lindex [split $w .] end]
  set ContactDef(SegmentoCorrente) [lsearch -exact $ContactDef(ordine_seg) \
     $CanvasCorrente] 
  $w configure -background lemonchiffon

  set text [TopToText [winfo toplevel $w]]
  if { $ContactDef(MaxSegment) < [expr $ContactDef(SegmentoCorrente) + \
         $EditPrefs(eqnum)] } {
    if [info exist ContactDef(ordine_seg)] {
      for { set i $ContactDef(SegmentoCorrente) } \
          { $i <= $ContactDef(MaxSegment) } {incr i} {
        if {[winfo exist [winfo parent $w].$i] == 0 } {
          ScriviEquazione [winfo toplevel $w] [winfo parent $w] $text $i 
        }
      }
    }
  } else {
    if [info exist ContactDef(ordine_seg)] {
      for { set i $ContactDef(SegmentoCorrente) } \
          { $i < [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] } \
          {incr i} {
        if {[winfo exist [winfo parent $w].$i] == 0 } {
          ScriviEquazione [winfo toplevel $w] [winfo parent $w] $text $i 
        }
      }
    }
  }
}

####################################################################
# Procedura CreaSegmentCanvas
# Procedura che crea i canvas che conterranno i segmenti
# Input: 
#   w       Nome del widget canvas
#   i       indice del canvas
# Ritorna :
#   Niente
####################################################################
proc CreaSegmentCanvas {w i} {
global ContactDef

  set main $w.main
  set text [TopToText $w]

  if { $i == 0} {
    set PosizioneCanvasY 0
  } elseif { $i > 0 } {
    set posizione_seg_prio [$main.ladd bbox segment[expr $i - 1]]
    set PosizioneCanvasY [lindex $posizione_seg_prio 3]
    # Default in caso di segmenti non visualizzati
    if { $PosizioneCanvasY == "" } {
      set PosizioneCanvasY 10
    }
  } 

  canvas $main.ladd.$i -background white -highlightthickness 0 -selectbackground grey76 
  $main.ladd create window 10 $PosizioneCanvasY -anchor nw \
                               -window $main.ladd.$i -tag segment$i

  $main.ladd.$i bind all <Motion> " ScriviCommento $main.t $main %W %x %y " 
  $main.ladd.$i bind all <Leave> " CancellaCommento $main %W %x %y " 
  bind $main.ladd.$i <Button-1> { AggiornaCanvasCorrente %W }  

  # reset delle variabili
  set ContactDef($i,multiple_st) 0         
  set ContactDef($i,curr_riga) 0          
  set ContactDef($i,curr_colonna) 0      
  set ContactDef($i,riga_fine_edt) 0    
  set ContactDef($i,col_fine_edt) 0    
  set ContactDef($i,col_fine_equazione) 0    
  set ContactDef($i,language) ""
}

####################################################################
# Procedura ResettaCanvas
# Procedura che resetta il canvas di visualizzazione del ladder
# Input: 
#   w       Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ResettaCanvas { t } {
global ContactDef 

  set canvas [winfo parent $t]
  
  for { set i $ContactDef(MaxSegment) } { $i >= 0} { incr i -1} {
    $canvas.ladd delete tag segment$i
    if [winfo exist $canvas.ladd.$i] {
      destroy $canvas.ladd.$i 
    }
  }

  # Resetto la variabile box e CONTACT
  set box(init_coll) [list -2 -2]
  set box(end_coll) [list -2 -2]
  set ContactDef(linee_equazioni) ""    
  set ContactDef(linee_equazioni_FB) "" 
  set ContactDef(ordine_seg) ""
  set ContactDef(nomefile) [lindex [[winfo parent $t].status.name configure -text] 4]

  ResettaCanvasEdit [winfo parent $t].ladd.edit

  CmdCalcolaEquazioni $t 
}

####################################################################
# Procedura ResettaCanvasEdit
# Procedura che resetta il canvas di editor del ladder
# Input: 
#   canvas   Nome del widget canvas
# Ritorna :
#   Niente
####################################################################
proc ResettaCanvasEdit { canvas } {
global  AlberoLadder ContactDef 

  # Resetto la variabile box e CONTACT
  set ContactDef(edit,multiple_st) 0         
  set ContactDef(edit,curr_riga) 0          
  set ContactDef(edit,curr_colonna) 0      
  set ContactDef(edit,riga_fine_edt) 0    
  set ContactDef(edit,col_fine_edt) 0    
  set ContactDef(edit,col_fine_equazione) 0    
  set ContactDef(edit,language) ladder

  $canvas delete all
  DrawCursor $canvas
  DimensionaCanvas $canvas 0 0
  DrawBarra $canvas

  set AlberoLadder(edit) ""
}

####################################################################
# Procedura RicavaSegmento
# Procedura che ricava il segmento dalla lista delle linee
# di inizio segmento
# Input: 
#   linea   ????
# Ritorna :
#   Niente
####################################################################
proc RicavaSegmento { linea } {
global ContactDef

  set ContactDef(SegmentoCorrente) -1 

  if { $linea < [lindex $ContactDef(linee_equazioni) 0] } {
    set ContactDef(SegmentoCorrente) 0
    return
  }

  foreach lineeSeg $ContactDef(linee_equazioni) {
    if { $lineeSeg <= $linea } {
      incr ContactDef(SegmentoCorrente)
    } else {
      break
    }
  }
}

####################################################################
# Procedura ScriviEquazione
# Procedura che converte il segmento e lo inserisce nel canvas creandolo
# Input: 
#   main      Nome del widget di main
#   canvas    Nome del widget di canvas
#   t         Nome del widget di testo
#   Segmento  Numero del segmento
# Ritorna :
#   Niente
####################################################################
proc ScriviEquazione { main canvas t Segmento } {
global ContactDef

  if {[winfo exists $canvas.[lindex $ContactDef(ordine_seg) $Segmento]] == 0 } {
    CreaSegmentCanvas $main [lindex $ContactDef(ordine_seg) $Segmento]
    set righe_colonne_occupate \
      [AnalizeIlEquation $canvas.[lindex $ContactDef(ordine_seg) $Segmento] $t $Segmento]
    DimensionaCanvas $canvas.[lindex $ContactDef(ordine_seg) $Segmento] \
      [lindex $righe_colonne_occupate 0] [lindex $righe_colonne_occupate 1] 
    set posizione_seg [$canvas bbox segment[lindex $ContactDef(ordine_seg) $Segmento]]
    set LarghezzaCanvas [lindex [lindex [$canvas configure -scrollregion] 4] 2]
    if { $LarghezzaCanvas < [lindex $posizione_seg 2]} {
      set LarghezzaCanvas [lindex $posizione_seg 2] 
    }
    $canvas configure -scrollregion "0 0 $LarghezzaCanvas [lindex $posizione_seg 3]"
  }
}

####################################################################
# Procedura ScorriBarraDetra
# Procedura che scorre la barra dei bottoni verso destra
# Input: 
#   buttonbar Nome del widget della buttonbar
# Ritorna :
#   Niente
####################################################################
proc ScorriBarraDetra { buttonbar } {
global BottoneAttivo

  set ListaBottoni [winfo children $buttonbar.b.just.frame]
  set IndiceBottoneAttivo [lsearch -regexp $ListaBottoni $BottoneAttivo]
  if { $IndiceBottoneAttivo != -1 && $IndiceBottoneAttivo < \
       [expr [llength $ListaBottoni] -1]} {
    set BottoneAttivo [lindex [split [lindex $ListaBottoni \
       [expr $IndiceBottoneAttivo + 1]] .] end]
  }
}

####################################################################
# Procedura ScorriBarraSinistra
# Procedura che scorre la barra dei bottoni verso sinistra
# Input: 
#   buttonbar Nome del widget della buttonbar
# Ritorna :
#   Niente
####################################################################
proc ScorriBarraSinistra { buttonbar } {
global BottoneAttivo

  set ListaBottoni [winfo children $buttonbar.b.just.frame]
  set IndiceBottoneAttivo [lsearch -regexp $ListaBottoni $BottoneAttivo]
  if { $IndiceBottoneAttivo != -1 && $IndiceBottoneAttivo > 0} {
    set BottoneAttivo [lindex [split [lindex $ListaBottoni \
      [expr $IndiceBottoneAttivo - 1]] .] end]
  }
}

####################################################################
# Procedura AggiornaLadder
# Procedura che aggiorna il canvas del ladder quando 
# sono in modalita' ladder
# Input: 
#   w 		Nome del widget di canvas
#   text 	Nome del widget di testo
#   ladder	Nome del widget ladder 
# Ritorna :
#   edit Se non ho convertito nulla / show se ho converito
####################################################################
proc AggiornaLadder { w text ladder } {
global EditPrefs ContactDef box LadderWidget ContactComment

  set main $w.main

  # All' interno dei canvas ci inserisco tanti altri canvas
  # quanti sono i segmenti
  CmdCalcolaEquazioni $text 
  RicavaSegmento [expr round([lindex [$text yview] 0] * \
    [lindex [split [$text index end] {.}] 0])]
  if { $ContactDef(MaxSegment) < \
     [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] } {
    if [info exist ContactDef(ordine_seg)] {
      for { set i $ContactDef(SegmentoCorrente) } { $i <= $ContactDef(MaxSegment) } \
          {incr i} {
        ScriviEquazione $w $ladder $text $i 
      }
    }
  } else {
    for { set i $ContactDef(SegmentoCorrente) } { $i < \
        [expr $ContactDef(SegmentoCorrente) + $EditPrefs(eqnum)] } {incr i} {
      ScriviEquazione $w $ladder $text $i 
    }
  }
  set PosizioneSegmento [$ladder coords segment$ContactDef(SegmentoCorrente)]
  if { $PosizioneSegmento != "" } {
    $ladder yview moveto [expr [lindex $PosizioneSegmento 1]/[lindex \
       [lindex [$main.ladd configure -scrollregion] 4] 3]] 
    DisplayEquationNumber $main.status.position [lindex $ContactDef(linee_equazioni) \
       $ContactDef(SegmentoCorrente)] 1
    $ladder.$ContactDef(SegmentoCorrente) configure -background lemonchiffon
  } else {
    set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
     -title [GetString MsgCommTitle]]
  }

  if { $ContactDef(MaxSegment) != -1 } {
    focus $ladder
    return show
  } else {
    focus $ladder.edit
    return edit
  }
}

####################################################################
# Procedura IlLadderConvert
# Procedura che cambia il widget di testo con un canvas per il ladder
# Input: 
#   w 		Nome dellla toplevel
# Ritorna :
#   Niente
####################################################################
proc IlLadderConvert {w} {
global EditPrefs ContactDef box LadderWidget ContactComment
global LadderEditShow 

   set buttonbar [TopToButtonbar $w]
   set main $w.main
   set text [TopToText $w]
   set menubar [TopToMenubar $w]

  # Se tento di attivare un bottone gia' attivo esco
  if { $EditPrefs(awl)==1 && [winfo ismapped $main.t] == 1 } {
    return
  }  
  if { $EditPrefs(awl)==0 && [winfo ismapped $main.ladd] == 1 } {
    return
  }

  if { $EditPrefs(awl)==1} {
     MenusCreate $menubar $main.t IL
     pack forget $main.ladd
     pack forget $main.xscroll

     # impacco i widget in modo AWL
     pack $main.s -side right -fill y
     pack $main.t -side right -expand yes -fill both
     pack [winfo parent $main].h -side bottom -fill x
     pack $main -expand yes -fill both

     # riabilito i Widget che non servono durante il debug
     for {set i 0} {$i < [llength $LadderWidget]} {incr i} {
       set indice_ [string first ":" [lindex $LadderWidget $i]]
       if { $indice_ != -1 } {
         if [winfo exist [string range [lindex $LadderWidget $i] 0 [expr $indice_ - 1]]] {
           [string range [lindex $LadderWidget $i] 0 [expr $indice_ - 1]] entryconfigure \
           [string range [lindex $LadderWidget $i] [expr $indice_ + 1] end] -state normal
         }
       }
     }

     CambiaButtonBar $text

     # Visualizzo la parte IL corrispondente alla ultima equazione
     if { [llength $ContactDef(linee_equazioni)] > 1 } {
       $main.t yview [lindex $ContactDef(linee_equazioni) $ContactDef(SegmentoCorrente)].0
       $main.t mark set insert [lindex $ContactDef(linee_equazioni) \
          $ContactDef(SegmentoCorrente)].0
       # distruggo i canvas dei segmenti
       for { set i $ContactDef(MaxSegment) } { $i >= 0} { incr i -1} {
         [winfo parent $main.t].ladd delete tag segment$i
         if [winfo exists [winfo parent $main.t].ladd.$i] {
           destroy [winfo parent $main.t].ladd.$i 
         }
       }
     }
     destroy $main.ladd.edit 
     focus $main.t
  } else {
    set geometria [wm geometry $w]
    # Resetto la variabile box e CONTACT
    set box(init_coll) [list -2 -2]
    set box(end_coll) [list -2 -2]
    set ContactDef(TipoEdit) FIRST
    set ContactDef(SegmentoCorrente) 0
    # contiene le linee di inizio delle equazioni in visualizzazione 
    set ContactDef(linee_equazioni) ""    
    # contiene le linee di inizio dei FB (se lo sono) corrispondenti 
    set ContactDef(linee_equazioni_FB) ""
    # ordine di visualizzazione dei canvas
    set ContactDef(ordine_seg) ""
    # tengo traccia del nome file
    set ContactDef(nomefile) [lindex [$main.status.name configure -text] 4] 

    pack forget $main.t
    pack forget $main.s
    pack forget [winfo parent $main].h

    # impacco i widget in modo LADDER
    MenusCreate $menubar $main.t LADDER
    pack $main.xscroll -side bottom -fill x
    pack $main.ladd  -side right -expand yes -fill both

    #disabilito i Widget che non servono per il ladder
    for {set i 0} {$i < [llength $LadderWidget]} {incr i} {
      set indice_ [string first ":" [lindex $LadderWidget $i]]
      if { $indice_ != -1 } {
        if [winfo exist [string range [lindex $LadderWidget $i] 0 [expr $indice_ - 1]]] {
          [string range [lindex $LadderWidget $i] 0 [expr $indice_ - 1]] entryconfigure \
          [string range [lindex $LadderWidget $i] [expr $indice_ + 1] end] -state disabled
        }
      }
    }

    $menubar.edit.m entryconfigure [GetString CmdGoToLine] -label [GetString CmdGoToEq]

    # Resetto tutti i tag del canvas
    $main.ladd delete all

    canvas $main.ladd.edit -background white -highlightthickness 0 \
      -selectbackground grey76 
    $main.ladd create window 10 0 -anchor nw \
      -window $main.ladd.edit -tag segmentedit

    bind $main.ladd.edit <Double-Button-1> "scrollIn %W %x %y Double" 
    bind $main.ladd.edit <Button-1> "scrollIn %W %x %y Single" 
    bind $main.ladd.edit <B1-Motion> "scrollDrag %W %x %y" 
    bind $main.ladd.edit <ButtonRelease-1> "DrawOp %W -1 -1 -1 -1 {ldil} " 
    bind $main.ladd.edit <Return> "SmistaComandoPalette %W"
    bind $main.ladd.edit <Up> "scrollUp %W edit" 
    bind $main.ladd.edit <Down> "scrollDown %W edit" 
    bind $main.ladd.edit <Left> "scrollLeft %W" 
    bind $main.ladd.edit <Alt-Left> "ScorriBarraSinistra $buttonbar"
    bind $main.ladd.edit <Right> "scrollRight %W" 
    bind $main.ladd.edit <Alt-Right> "ScorriBarraDetra $buttonbar"
    
    ResettaCanvasEdit $main.ladd.edit

    bind $main.ladd <Up> "scrollUp %W show $w $text" 
    bind $main.ladd <Down> "scrollDown %W show $w $text" 
    bind $main.ladd <Prior> "scrollUp %W show $w $text" 
    bind $main.ladd <Next> "scrollDown %W show $w $text" 
    bind $main.ladd <Key-Escape> "ModalitaEdit %W" 

    wm geometry $w $geometria
    if { [AggiornaLadder  $w $text $main.ladd] == "edit" } {
      set LadderEditShow 1
    } else {
      set LadderEditShow 0
    }
    CambiaButtonBar $text
  }
}

