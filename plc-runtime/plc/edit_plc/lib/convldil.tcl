
            ######################################
            #                                    #
            #     File di definizione di         #
            #     funzioni per la conversione    #
            #     da ladder a il                 #
            #                                    #
            ######################################


####################################################################
# Procedura DrawBarra
# Procedura che disegna la linea di alimentazione
# Input: 
#   w            Nome del widget cancas
# Ritorna :
#   Niente
####################################################################
proc DrawBarra { w } {
global ContactDef EditPrefs

  set canvasCorrente [lindex [split $w .] end]
  $w create line $ContactDef(offset_x) 0 $ContactDef(offset_x) \
     [expr ($ContactDef($canvasCorrente,riga_fine_edt) + 3) * $ContactDef(height_conn)] \
      -width 2 -tag barra1
}

####################################################################
# Procedura DrawCursor
# Procedura che disegna il cursore
# Input: 
#   w            Nome del widget cancas
# Ritorna :
#   Niente
####################################################################
proc DrawCursor { w } {
global ContactDef EditPrefs

  set canvasCorrente [lindex [split $w .] end]
  # creo il cursore
  $w create rect 0 0 40 45  -tag cursore
  $w move cursore [expr $ContactDef(offset_x) + \
    $ContactDef(width_op)] $ContactDef(offset_y)
}

####################################################################
# Procedura DrawOp
# Procedura che disegna l'operando specificato da tipo_operando
# Input: 
#   w                 Nome del widget cancas
#   riga_iniz         Riga iniziale
#   colonna_iniz      Colonna iniziale
#   riga_fin          Riga finale
#   colonna_fin       Colonna finale
#   tipo_conversione  "illd" "ldil"
# Ritorna :
#   Niente
####################################################################
proc DrawOp { w riga_iniz colonna_iniz riga_fin colonna_fin tipo_conversione} {
global EditPrefs ContactDef box AlberoLadder BottoneAttivo

  set canvasCorrente [lindex [split $w .] end]
  if { $tipo_conversione == "ldil" } {
    if { [string first "colleg" $BottoneAttivo] != -1 && \
         [lindex $box(init_coll) 0] != -2 && [lindex $box(end_coll) 0] != -2} {
      $w config -cursor left_ptr
      $w itemconfig cursore -outline black
      set x1 [expr [lindex $box(init_coll) 1] + 1]
      set y1 [lindex $box(init_coll) 0]
      set x2 [expr [lindex $box(end_coll) 1] + 1]
      set y2 [lindex $box(end_coll) 0]
      set box(init_coll) [list -2 -2]
      set box(end_coll) [list -2 -2]
    } else {
      return
    }
  } elseif { $tipo_conversione == "illd" } {
    set x1 [incr colonna_iniz]
    set y1 $riga_iniz
    set x2 [incr colonna_fin]
    set y2 $riga_fin
  }

  if { $x2 == $ContactDef($canvasCorrente,col_fine_equazione)  && $y2 > $y1 } {
    if { [expr $x2 -$x1] > 1 } {
      AlertBox -text MsgNoRightConnection
      return
    }
    # ST Multiplo
    set inizio_linea [expr $ContactDef(offset_x) + ($ContactDef(width_op) + \
       $ContactDef(width_conn))*$x1]
    $w create line [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    $w create line [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
                   [expr $ContactDef(offset_x) + ($ContactDef(width_op)*$x2) + \
                   ($ContactDef(width_conn)*($x2 -1))] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    set x1 [expr $x1 - 1]
    set x2 [expr $x2 - 1]
    set AlberoLadder($canvasCorrente) \
       [lappend AlberoLadder($canvasCorrente) $y1:$x1|$y2:$x2]
    return
  }

  if { ($x1 < $x2) && ($y1 == $y2) } {
    for { set i $x1  } { $i < [expr $x2 -1]} { incr i} {
      if { [$w gettags cont$y1:$i] != "" } {
        AlertBox -text MsgNoRightConnection
        return
      }
    }
    # Caso da sinistra a destra
    $w create line [expr $ContactDef(offset_x) + ($ContactDef(width_op) + \
                   $ContactDef(width_conn))*$x1] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $ContactDef(offset_x) + ($ContactDef(width_op)*$x2) + \
                   ($ContactDef(width_conn)*($x2 -1))] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y1:[expr $x2 -1]
    set x1 [expr $x1 - 1]
    set x2 [expr $x2 - 1]
    set AlberoLadder($canvasCorrente) \
      [lappend AlberoLadder($canvasCorrente) $y1:$x1|$y1:$x2]
  } elseif { (($x1 < $x2) && ($y1 < $y2)) } {
    for { set i $x1  } { $i < [expr $x2 -1]} { incr i} {
      if { [$w gettags cont$y2:$i] != "" } {
        AlertBox -text MsgNoRightConnection
        return
      }
    }
    # Caso da alto a basso
    set inizio_linea [expr $ContactDef(offset_x) + ($ContactDef(width_op) + \
      $ContactDef(width_conn))*$x1]
    $w create line $inizio_linea \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $inizio_linea + $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    $w create line [expr $inizio_linea + $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $inizio_linea + $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    $w create line [expr $inizio_linea + $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
                   [expr $ContactDef(offset_x) + ($ContactDef(width_op)*$x2) + \
                   ($ContactDef(width_conn)*($x2 -1))] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    set x1 [expr $x1 - 1]
    set x2 [expr $x2 - 1]
    set AlberoLadder($canvasCorrente) \
                   [lappend AlberoLadder($canvasCorrente) $y1:$x1|$y2:$x2]
  } elseif { (($x1 < $x2) && ($y1 > $y2)) } {
    for { set i $x1  } { $i < [expr $x2 -1]} { incr i} {
      if { [$w gettags cont$y1:$i] != "" } {
        AlertBox -text MsgNoRightConnection
        return
      }
    }
    # Caso da basso a alto
    set fine_linea [expr $ContactDef(offset_x) + ($ContactDef(width_op)*$x2) + \
                   ($ContactDef(width_conn)*($x2 -1))]
    $w create line [expr $ContactDef(offset_x) + ($ContactDef(width_op) + \
                   $ContactDef(width_conn))*$x1] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $fine_linea - $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    $w create line [expr $fine_linea - $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y1) + \
                   $ContactDef(offset_coll)] \
                   [expr $fine_linea - $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    $w create line [expr $fine_linea - $ContactDef(width_op)/2.0] \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
                   $fine_linea \
                   [expr $ContactDef(offset_y) + ($ContactDef(height_conn)*$y2) + \
                   $ContactDef(offset_coll)] \
              -width 2 -tags conn$y1:[expr $x1 -1]|$y2:[expr $x2 -1]
    set x1 [expr $x1 - 1]
    set x2 [expr $x2 - 1]
    set AlberoLadder($canvasCorrente) \
                   [lappend AlberoLadder($canvasCorrente) $y1:$x1|$y2:$x2]
  }
}

####################################################################
# Procedura DrawCont 
# Procedura che disegna il contatto definito da tipo_contatto
# Input: 
#   w                 Nome del widget cancas
#   riga              Riga dove disegnare il contatto
#   colonna           Colonna dove diegnare il contatto
#   tipo_contatto     tipo di contatto da disegnare
#   variabile         Variabile associata al contatto
#   tipo_conversione  "illd" "ldil"
# Ritorna :
#   Niente
####################################################################
proc DrawCont { w riga colonna tipo_contatto variabile tipo_conversione} {
global ContactDef Home AlberoLadder EditPrefs box BottoneAttivo

  set canvasCorrente [lindex [split $w .] end]
  if { $tipo_conversione == "ldil" } {
    set x_pos [lindex [$w coords cursore] 0]
    set y_pos [lindex [$w coords cursore] 1]
  } elseif { $tipo_conversione == "illd" } {
    set x_pos [expr $ContactDef(offset_x) + ($ContactDef(width_op)*($colonna + 1)) + \
      ($ContactDef(width_conn)* $colonna) ]
    set y_pos [expr $ContactDef(offset_y) + $ContactDef(height_conn) * $riga]
  }
  if { [$w gettags cont$riga:$colonna] == "" } {
    $w create bitmap $x_pos $y_pos \
          -bitmap @$Home/icone/$tipo_contatto \
          -anchor nw -background white -foreground black -tags cont$riga:$colonna

    if { $tipo_contatto == "JMPC.png" || \
         $tipo_contatto == "JMPCN.png" } {
      # Caso del salto
      $w create text [expr $x_pos + $ContactDef(offset_x_text)+$ContactDef(width_conn)] \
		     [expr $y_pos + $ContactDef(offset_coll) ] \
            -text $variabile -font $EditPrefs(ladderfont) -tag var$riga:$colonna
# ATTENZIONE necessario per la conversione LD - IL
      set AlberoLadder($canvasCorrente) \
        [lappend AlberoLadder($canvasCorrente) $riga:$colonna|$riga:fine]    
      if { [expr $colonna + 1] > $ContactDef($canvasCorrente,col_fine_equazione) } {
        set ContactDef($canvasCorrente,col_fine_equazione) [expr $colonna + 1]
      }
    } elseif { $tipo_contatto == "LABEL.png" } {
      # Caso della etichetta
      $w create text [expr $ContactDef(offset_x) + $ContactDef(offset_x_text)] \
	             [expr $y_pos + $ContactDef(offset_coll) ] \
            -text $variabile \
            -font $EditPrefs(ladderfont) -tag var$riga:$colonna
    } else {
      # variabili
      $w create text [expr $x_pos + $ContactDef(offset_x_text)] \
            [expr $y_pos + $ContactDef(offset_y_text)] \
            -text $variabile -font $EditPrefs(ladderfont) -tag var$riga:$colonna
    }

    # Se il contatto e' in prima linea inserisco direttamente la
    # connessione sinistra.
    if { $ContactDef($canvasCorrente,curr_colonna) == 0 && $tipo_conversione == "ldil" } {
      set box(init_coll) [list $ContactDef($canvasCorrente,curr_riga) -1]
      set box(end_coll) [list $ContactDef($canvasCorrente,curr_riga) \
        $ContactDef($canvasCorrente,curr_colonna)]
      set OldBottoneAttivo $BottoneAttivo
      set BottoneAttivo colleg
      DrawOp $w -1 -1 -1 -1 {ldil}
      set BottoneAttivo $OldBottoneAttivo
    }

    if { $tipo_contatto == "CONT3.png" || \
         $tipo_contatto == "CONT4.png" || \
         $tipo_contatto == "CONT5.png" || \
         $tipo_contatto == "CONT6.png" || \
         $tipo_contatto == "CONT3R.png" || \
         $tipo_contatto == "CONT4R.png" } {
       # Contatto finale 
       set inizio_linea [expr $x_pos + $ContactDef(width_conn)]
       $w create line $inizio_linea \
                   [expr $y_pos + $ContactDef(offset_coll)] \
                   [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $y_pos + $ContactDef(offset_coll)] \
              -width 2 -tags conn$riga:$colonna|$riga:fine
       $w create line [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $y_pos] \
                   [expr $inizio_linea + $ContactDef(width_op)] \
                   [expr $ContactDef(height_conn) + $y_pos] \
              -width 2 -tags conn$riga:$colonna|$riga:fine
      set AlberoLadder($canvasCorrente) \
        [lappend AlberoLadder($canvasCorrente) $riga:$colonna|$riga:fine]    
      set ContactDef($canvasCorrente,col_fine_equazione) [expr $colonna + 1]
    }
  } else {
   $w itemconfig cont$riga:$colonna -bitmap @$Home/icone/$tipo_contatto 
   $w itemconfig var$riga:$colonna -text $variabile 
  }
  if { $ContactDef($canvasCorrente,riga_fine_edt) < $riga } {
    set ContactDef($canvasCorrente,riga_fine_edt) $riga
  }
  if { $ContactDef($canvasCorrente,col_fine_edt) < $colonna } {
    set ContactDef($canvasCorrente,col_fine_edt) $colonna
  }
}

####################################################################
# Procedura DefineInoutRect
# Procedura che calcola gli input e gli output della chiamata a 
# blocco funzionale
# Input: 
#   canvas            Nome del widget cancas
#   text              Nome del widget di testo
#   inizio_equazione  Riga iniziale della equazione
#   fine_equazione    Riga finale della equazione
# Ritorna :
#   Niente
####################################################################
proc DefineInoutRect { canvas text inizio_equazione fine_equazione} {

  global ContactDef
  set riga_corrente 0
  set RigaColonnaCommento [EstraiCommento $text $canvas $inizio_equazione]
  if { [lindex $RigaColonnaCommento 0] != -1 } {
    set riga_corrente [expr [lindex $RigaColonnaCommento 0] + 1]
  }

  set canvasCorrente [lindex [split $canvas .] end]
  set linee_aperte_LD -1
  set linee_aperte_ST -1
  set index_linea $inizio_equazione
  set InputOutput 0
  foreach x [split [$text get $inizio_equazione.0 $fine_equazione.end] \n ] {
    if { ([lindex $x 0] == "LD" || [lindex $x 0] == "LDN") && \
          $InputOutput == 0  } {
      incr linee_aperte_LD
      LdInstr  $canvas [expr $riga_corrente + $linee_aperte_LD] 0 [lindex $x 1] \
        [lindex $x 0] $index_linea
      DrawOp $canvas [expr $linee_aperte_LD + $riga_corrente] 0 \
        [expr $linee_aperte_LD + $riga_corrente] 1 "illd"
    } elseif { ([lindex $x 0] == "ST" || [lindex $x 0] == "STN") && \
                $InputOutput == 1  } { 
      incr linee_aperte_ST
      StInstr  $canvas [expr $linee_aperte_ST + $riga_corrente] \
       [expr $linee_aperte_ST + $riga_corrente] 3 [lindex $x 1] [lindex $x 0] $index_linea
      DrawOp $canvas [expr $linee_aperte_ST + $riga_corrente] 2  \
        [expr $linee_aperte_ST + $riga_corrente] 3 "illd"
    } elseif { [lindex $x 0] == "CAL" } {
      DefineRect $canvas [lindex $x 1] $riga_corrente 1 $index_linea
      set InputOutput 1
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

  if { $linee_aperte_LD >= $linee_aperte_ST } {
    return [list [expr $riga_corrente + $linee_aperte_LD] $numero_colonne]
  } else {
    return [list [expr $riga_corrente + $linee_aperte_ST] $numero_colonne]
  }
}

####################################################################
# Procedura DefineRect
# Procedura che setta il vettore BOX_DEF a seconda del tipo
# di function_block definito
# Input: 
#   canvas         Nome del widget cancas
#   titolo         Titolo del rettangolo
#   riga           Riga in cui inserire il rettangolo
#   colonna        Colonna in cui inserire il rettangolo
#   index_linea    ????
# Ritorna :
#   Niente
####################################################################
proc DefineRect { w titolo riga colonna index_linea } {
global ContactDef BLOCK PredefinedFunctionBlock FunctionBlockList
 
  if [info exists BLOCK] {
    unset BLOCK
  }
  set tipovar "input"
  set indice_in 0
  set indice_out 0

  # Disegno il box
  set BLOCK(title) $titolo
  for {set i 0} {$i< [llength $PredefinedFunctionBlock($FunctionBlockList($titolo))] } \
      {incr i } {
     set parametro [lindex $PredefinedFunctionBlock($FunctionBlockList($titolo)) $i]
     if { $parametro != "###"} {
       if { $tipovar == "input" } {
          set BLOCK(in$indice_in) [list $parametro 0] 
          incr indice_in
       } else {
          set BLOCK(out$indice_out) [list $parametro 0]
          incr indice_out
       }
     } else {
       set tipovar "output"
     }
  }
  if { $indice_in > $indice_out } {
   set BLOCK(input) $indice_in
   for { set i $indice_out } { $i < $indice_in } { incr i} {
     set BLOCK(out$i) [list "" 0]
   }
  } elseif { $indice_in < $indice_out } {
   set BLOCK(input) $indice_out
   for { set i [expr $indice_in + 1] } { $i <= $indice_out } { incr i} {
     set BLOCK(in$i) [list "" 0]
   }
  } else {
   set BLOCK(input) $indice_out
  }
  return [DrawRect $w $riga $colonna $index_linea]
}

####################################################################
# Procedura DrawRect
# Procedura che disegna il rettangolo dei box
# Input: 
#   w              Nome del widget cancas
#   riga           Riga in cui inserire il rettangolo
#   colonna        Colonna in cui inserire il rettangolo
#   index_linea    ????
# Ritorna :
#   Niente
####################################################################
proc DrawRect { w riga colonna index_linea} {

  set canvasCorrente [lindex [split $w .] end]
  global ContactDef BLOCK LINE_CONTACT$canvasCorrente EditPrefs

  if { $BLOCK(title) == "XOR" } {
    set BOX_DEF(offset_box_x) 20 
  } else {
    set BOX_DEF(offset_box_x) 3 
  }
  set BOX_DEF(offset_box_y) 6 
  set BOX_DEF(offset_x_name) [expr 45 + (5 * [string length $BLOCK(title)])/2] 
  #set BOX_DEF(offset_x_out) [expr 90 - $BOX_DEF(offset_box_x)]
  set BOX_DEF(offset_y_in_out)  40 

  set offset_input 0
  set offset_output 0

  for { set i 0 } { $i < $BLOCK(input) } { incr i } {
    set offset_input [expr $offset_input + [lindex $BLOCK(in$i) 1]]
    set offset_output [expr $offset_output + [lindex $BLOCK(out$i) 1]]
    if { [lindex $BLOCK(in$i) 0] != "" } {
      # Creazione degli input
      set BOX_DEF(offset_x_in) [expr 15 + $BOX_DEF(offset_box_x) + \
        (([string length [lindex $BLOCK(in$i) 0]] * 5)/2) ]
      $w create text [expr $BOX_DEF(offset_x_in) + (($ContactDef(width_conn) + \
        $ContactDef(width_op)) * $colonna) + $ContactDef(width_op) ] \
        [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
        ($riga + $i + $offset_input)] \
        -text [lindex $BLOCK(in$i) 0] -font $EditPrefs(ladderfont) \
        -tag rectin_$riga:$colonna

      $w create line [expr $ContactDef(offset_x) + (($ContactDef(width_conn) + \
        $ContactDef(width_op)) * $colonna) + $ContactDef(width_op) ] \
        [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
        ($riga + $i + $offset_input)] \
        [expr $ContactDef(offset_x) + $BOX_DEF(offset_box_x) + \
        (($ContactDef(width_conn) + $ContactDef(width_op)) * $colonna) + \
        $ContactDef(width_op) ] \
        [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
        ($riga + $i + $offset_input)] \
        -width 2 -tags rectin_$riga:$colonna
    }
    if { [lindex $BLOCK(out$i) 0] != "" } {
      # Con questo if faccio in modo che l'ultimo output sia sempre allineato in fondo
      set BOX_DEF(offset_x_out) [expr 95 - $BOX_DEF(offset_box_x) - \
        (([string length [lindex $BLOCK(out$i) 0]] * 5)/2) ]
      if { $i == [expr $BLOCK(input) -1] && $offset_input > $offset_output} {
        $w create text [expr $BOX_DEF(offset_x_out) + (($ContactDef(width_conn) + \
           $ContactDef(width_op)) * $colonna) + $ContactDef(width_op) ] \
           [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
           ($riga + $i + $offset_input)] \
            -text [lindex $BLOCK(out$i) 0] \
            -font $EditPrefs(ladderfont) -tag rectout_$riga:$colonna	

        $w create line [expr $ContactDef(offset_x) - $BOX_DEF(offset_box_x) + \
           (($ContactDef(width_conn) + $ContactDef(width_op)) * ($colonna + 2))] \
           [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
           ($riga + $i + $offset_input)] \
           [expr $ContactDef(offset_x) + (($ContactDef(width_conn) + \
           $ContactDef(width_op)) * ($colonna + 2))] \
           [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
           ($riga + $i + $offset_input)] \
           -width 2 -tags rectout_$riga:$colonna
      } else {
        $w create text [expr $BOX_DEF(offset_x_out) + (($ContactDef(width_conn) + \
          $ContactDef(width_op)) * $colonna) + $ContactDef(width_op) ] \
          [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
          ($riga + $i + $offset_output)] \
          -text [lindex $BLOCK(out$i) 0] \
          -font $EditPrefs(ladderfont) -tag rectout_$riga:$colonna	

        $w create line [expr $ContactDef(offset_x) - $BOX_DEF(offset_box_x) + \
          (($ContactDef(width_conn) + $ContactDef(width_op)) * ($colonna + 2))] \
          [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
          ($riga + $i + $offset_output)] \
          [expr $ContactDef(offset_x) + (($ContactDef(width_conn) + \
          $ContactDef(width_op)) * ($colonna + 2))] \
          [expr $BOX_DEF(offset_y_in_out) + $ContactDef(height_conn) * \
          ($riga + $i + $offset_output)] \
          -width 2 -tags rectout_$riga:$colonna
      }
    }
  }

  if { $offset_input > $offset_output } {
    set offset_globale $offset_input
  } else {
    set offset_globale $offset_output
  }
  # Creazione del rettangolo
  $w create rectangle [expr $ContactDef(offset_x) + $BOX_DEF(offset_box_x) + \
    (($ContactDef(width_conn) + $ContactDef(width_op)) * $colonna) + \
    $ContactDef(width_op)] \
    [expr $ContactDef(offset_y) + $BOX_DEF(offset_box_y) + $ContactDef(height_conn) * \
    $riga] \
    [expr $ContactDef(offset_x) - $BOX_DEF(offset_box_x) + (($ContactDef(width_conn) + \
    $ContactDef(width_op)) * ($colonna + 2))] \
    [expr $ContactDef(offset_y) - $BOX_DEF(offset_box_y) + $ContactDef(height_conn) * \
    $riga + (($BLOCK(input) + $offset_globale) * $ContactDef(height_conn))] \
    -width 2 -tags rect_$riga:$colonna

  # Creazione del titolo
  $w create text [expr $BOX_DEF(offset_x_name) + (($ContactDef(width_conn) + \
    $ContactDef(width_op)) * $colonna) + $ContactDef(width_op) ] \
    [expr $ContactDef(offset_y) + $ContactDef(offset_y_text) + $BOX_DEF(offset_box_y) + \
    $ContactDef(height_conn) * $riga] \
    -text $BLOCK(title) -font $EditPrefs(ladderfont) -tag recttitle_$riga:$colonna

  set canvasTempo LINE_CONTACT$canvasCorrente
  append canvasTempo "($index_linea)"
  set $canvasTempo [list $riga $colonna]
  return [expr $BLOCK(input) + $offset_globale] 
}

####################################################################
# Procedura ConvertiSegmento
# Procedura che converte un segmento da ld a il
# Input: 
#   w              Nome del widget cancas
# Ritorna :
#   Niente
####################################################################
proc ConvertiSegmento { w } {
global ContactDef AlberoLadder PrimoElemento ContactComment
  
  # Il primo elemento in caso di commento prima della equazione
  # puo' essere anche sulla riga 1 quindi devo andare 
  # ad individuare la riga di inizio equazione.
  set PrimoElemento -1
  # Calcolo gli incroci
  set canvasCorrente [lindex [split $w .] end]
  set canvaswidth [$w configure -width]
  set canvasheight [$w configure -height]
  foreach x [$w find enclosed 0 0 [lindex $canvaswidth 4] \
          [expr [lindex $canvasheight 4] + $ContactDef(height_conn)]] {
    set NomeTag [$w gettags $x]
    if { [string first cont $NomeTag] != -1 } {
      set NomeTag [string trimleft $NomeTag "cont"]
      if { [lsearch $AlberoLadder($canvasCorrente) "$NomeTag\|*"] == -1 } {
        AlertBox -text MsgNoAllConnection
        return -1
      }
    }
  }

  foreach x [lsort -ascii $AlberoLadder($canvasCorrente)] {
    set primosplit [split $x '|']
    set riga_iniz [lindex [split [lindex $primosplit 0] ':'] 0]
    if { $PrimoElemento == -1 } {
      set PrimoElemento $riga_iniz
    }
    set col_iniz [lindex [split [lindex $primosplit 0] ':'] 1]
    # L'incremento e' dovuto al fatto che le colonne partono da -1
    incr col_iniz
    set riga_fin [lindex [split [lindex $primosplit 1] ':'] 0]
    set col_fin [lindex [split [lindex $primosplit 1] ':'] 1]
    if {[info exists incroci_eq($riga_iniz)] == 0 } {
      set incroci_eq($riga_iniz) ""
    }
    if {[info exists incroci_eq($riga_fin)] == 0 } {
      set incroci_eq($riga_fin) ""
    }
    if { $riga_iniz < $riga_fin } {
      # Alto basso
      if { [lsearch -exact $incroci_eq($riga_iniz) $col_iniz] == -1 } {
        set incroci_eq($riga_iniz) [lappend incroci_eq($riga_iniz) $col_iniz]
      }
      if { [lsearch -exact $incroci_eq($riga_fin) $col_iniz] == -1 } {
        set incroci_eq($riga_fin) [lappend incroci_eq($riga_fin) $col_iniz]
      }
    } elseif { $riga_iniz > $riga_fin } {
      # Basso alto
      if { [lsearch -exact $incroci_eq($riga_iniz) $col_fin] == -1 } {
        set incroci_eq($riga_iniz) [lappend incroci_eq($riga_iniz) $col_fin]
      }
      if { [lsearch -exact $incroci_eq($riga_fin) $col_fin] == -1 } {
        set incroci_eq($riga_fin) [lappend incroci_eq($riga_fin) $col_fin]
      }
    } elseif { $col_iniz == 0 } {
      # Escludo il primo elemento in assoluto
      if { $riga_iniz != $PrimoElemento } { 
        if { [lsearch -exact $incroci_eq($riga_iniz) $col_iniz] == -1 } {
          set incroci_eq($riga_iniz) [lappend incroci_eq($riga_iniz) $col_iniz]
        }
      }
    } elseif { $col_fin == "fine" } {
      if { [lsearch -exact $incroci_eq($riga_iniz) $col_iniz] == -1 } {
        set incroci_eq($riga_iniz) [lappend incroci_eq($riga_iniz) $col_iniz]
      }
    }
  }

  if { [info exist incroci_eq] == 0 } {
    return -1
  }

  foreach x [array names incroci_eq] {
    set incroci_eq($x) [lsort -increasing $incroci_eq($x)]
  }
  
  # comincio a tradurre le equazioni
  set iniz_eq [lindex [lsort -increasing [array names incroci_eq]] 0]
  set fine_eq [lindex [lsort -decreasin [array names incroci_eq]] 0]
  set colonna_corrente 0
  set riga_corrente $iniz_eq
  set colonna_or 0
  set riga_and $iniz_eq
  set max_righe [expr $fine_eq - $iniz_eq + 1]
  set parentesi_aperte 0
  set or_num 0
  while { $colonna_corrente < $ContactDef($canvasCorrente,col_fine_equazione)}  { 
    # Flag che mi dice se devo aumentare la colonna dopo aver 
    # verificato l'istruzione di ST
    # proseguo verso destra fino all'incontro con l'incrocio
    while { $colonna_corrente < [lindex $incroci_eq($riga_corrente) 0] } {
      if { $riga_corrente == $iniz_eq && $colonna_corrente == 0 } {
        if { [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
          set variabile [lindex [$w itemconfigure \
             var$riga_corrente:$colonna_corrente -text] 4]
          set commento $ContactComment($riga_corrente:$colonna_corrente)
          set NewSegment [lappend NewSegment "\tLD\t$variabile\t$commento"]
        } else {
          set variabile [lindex [$w itemconfigure \
             var$riga_corrente:$colonna_corrente -text] 4]
          set commento $ContactComment($riga_corrente:$colonna_corrente)
          set NewSegment [lappend NewSegment "\tLDN\t$variabile\t$commento"]
	}
 	incr colonna_corrente 
      } else {
        # se c'e' contatto e' un and
        if { [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
          if { [file tail [$w itemconfig \
               cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
               [file tail [$w itemconfig \
               cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:$colonna_corrente -text] 4]
            set commento $ContactComment($riga_corrente:$colonna_corrente)
            set NewSegment [lappend NewSegment "\tAND\t$variabile\t$commento"]
	  } else {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:$colonna_corrente -text] 4]
            set commento $ContactComment($riga_corrente:$colonna_corrente)
            set NewSegment [lappend NewSegment "\tANDN\t$variabile\t$commento"]
          }
        }
 	incr colonna_corrente 
      }
    }
    # Chiusura parentesi di OR(
    if  { $or_num > 1 && [lsearch -exact $incroci_eq($riga_corrente) $colonna_or] \
        != -1 } {
       set NewSegment [lappend NewSegment "\t)"]
       set parentesi_aperte [expr $parentesi_aperte - 1]
    }
  
    if { $colonna_corrente < $ContactDef($canvasCorrente,col_fine_equazione) } {
      # Passo alla riga successiva
      incr riga_corrente 
      if { [info exists incroci_eq($riga_corrente)] && \
           [lsearch -exact $incroci_eq($riga_corrente) $colonna_corrente] != -1 } {
        set colonna_or $colonna_corrente
        set colonna_corrente [lindex $incroci_eq($riga_corrente) 0]
      }

    if { $colonna_corrente < $colonna_or } {
      # verifico se tra colonna corrente e colonna_or c'e' piu' di
      # un contatto
      # Analizzo la riga successiva ancora da sinistra a destra
      set or_num 0
      for { set l $colonna_corrente } { $l < $colonna_or } {incr l} {
        set contatto_temporaneo [file tail [$w itemconfig cont$riga_corrente:$l -bitmap]]
        if { $contatto_temporaneo == "CONT1.png" || \
             $contatto_temporaneo == "CONT2.png" || \
             $contatto_temporaneo == "CONT1R.png" || \
             $contatto_temporaneo == "CONT2R.png" } {
          set or_num [expr $or_num +1]
        }
      }
      if { $or_num == 0 } {
        incr colonna_corrente 
      } elseif { $or_num == 1 } {
        if { [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
          set variabile [lindex [$w itemconfigure \
             var$riga_corrente:$colonna_corrente -text] 4]
          set commento $ContactComment($riga_corrente:$colonna_corrente)
          set NewSegment [lappend NewSegment "\tOR\t$variabile\t$commento"]
        } else {
	  set variabile [lindex [$w itemconfigure \
            var$riga_corrente:$colonna_corrente -text] 4]
          set commento $ContactComment($riga_corrente:$colonna_corrente)
          set NewSegment [lappend NewSegment "\tORN\t$variabile\t$commento"]
        }
	# Chiusura parentesi di AND(
	if  { $colonna_corrente != 0 } {
          if [info exists incroci_eq([expr $riga_corrente + 1])] {
            if { [lsearch -exact $incroci_eq([expr $riga_corrente + 1]) \
                 $colonna_corrente] == -1  } {
              set NewSegment [lappend NewSegment "\t)"]
              set parentesi_aperte [expr $parentesi_aperte - 1]
            } 
          } else {
            set NewSegment [lappend NewSegment "\t)"]
            set parentesi_aperte [expr $parentesi_aperte - 1]
          }
	}
	incr colonna_corrente 
      } elseif { $or_num > 1} {
        set variabile [lindex [$w itemconfigure \
          var$riga_corrente:$colonna_corrente -text] 4]
        set commento $ContactComment($riga_corrente:$colonna_corrente)
        set NewSegment [lappend NewSegment "\tOR(\t$variabile\t$commento"]
        incr colonna_corrente 
        incr parentesi_aperte 
        set riga_and $riga_corrente
      }
      set lun_incroci [expr [llength $incroci_eq($riga_corrente)] - 1]
      set incroci_eq($riga_corrente) [lrange $incroci_eq($riga_corrente) 1 $lun_incroci] 
    } else {
      # Non ci sono piu' contatti in fondo quindi ritorno alla riga 
      # in cui avevamo cominciato l'and.
      if { $colonna_corrente == [lindex $incroci_eq($iniz_eq) 0] } {
        set riga_corrente $iniz_eq
        # se c'e' il contatto
        if { [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
	  if { [file tail [$w itemconfig \
             cont[expr $riga_corrente + 1]:$colonna_corrente -bitmap]] == "CONT1.png" || \
               [file tail [$w itemconfig \
             cont[expr $riga_corrente + 1]:$colonna_corrente -bitmap]] == "CONT2.png" || \
               [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" || \
               [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
	    # se c'e' il contatto sotto la riga.
            # Chiudo le parentesi aperte e metto un AND(
            while { $parentesi_aperte > 0 } {
              set NewSegment [lappend NewSegment "\t)"]
              set parentesi_aperte [expr $parentesi_aperte - 1]
            }
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:$colonna_corrente -text] 4]
            set commento $ContactComment($riga_corrente:$colonna_corrente)
            if { [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
                 [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
              set NewSegment [lappend NewSegment "\tAND(\t$variabile\t$commento"]
            } elseif { [file tail [$w itemconfig \
                    cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2.png" || \
                    [file tail [$w itemconfig \
                    cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
              set NewSegment [lappend NewSegment "\tAND("]
              set NewSegment [lappend NewSegment "\tLDN\t$variabile\t$commento"]
            }
            set parentesi_aperte [expr $parentesi_aperte + 1]
          } else {
            # Chiudo le parentesi aperte
            while { $parentesi_aperte > 0 } {
              set NewSegment [lappend NewSegment "\t)"]
              set parentesi_aperte [expr $parentesi_aperte - 1]
            }
            if { [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
                 [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
	      set variabile [lindex [$w itemconfigure \
                var$riga_corrente:$colonna_corrente -text] 4]
              set commento $ContactComment($riga_corrente:$colonna_corrente)
              set NewSegment [lappend NewSegment "\tAND\t$variabile\t$commento"]
            } else {
              set variabile [lindex [$w itemconfigure \
                var$riga_corrente:$colonna_corrente -text] 4]
              set commento $ContactComment($riga_corrente:$colonna_corrente)
              set NewSegment [lappend NewSegment "\tANDN\t$variabile\t$commento"]
            }
          }
          incr colonna_corrente 
        } 
        if [info exist incroci_eq($riga_corrente)] {
          set lun_incroci [expr [llength $incroci_eq($riga_corrente)] - 1]
          set incroci_eq($riga_corrente) [lrange $incroci_eq($riga_corrente) 1 \
            $lun_incroci] 
        }
      } else {
        set riga_corrente $riga_and
        # se c'e' il contatto
        if { [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
 	  # Se non c'e' il contatto sotto la riga.
	  if { [file tail [$w itemconfig \
             cont[expr $riga_corrente + 1]:$colonna_corrente -bitmap]] == "CONT1.png" || \
               [file tail [$w itemconfig \
             cont[expr $riga_corrente + 1]:$colonna_corrente -bitmap]] == "CONT2.png" || \
               [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" || \
               [file tail [$w itemconfig \
             cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
            # se c'e' il contatto sotto la riga.
	    set variabile [lindex [$w itemconfigure \
              var$riga_corrente:$colonna_corrente -text] 4]
            set commento $ContactComment($riga_corrente:$colonna_corrente)
            if { [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
                 [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
              set NewSegment [lappend NewSegment "\tAND(\t$variabile\t$commento"]
            } elseif { [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2.png" || \
                 [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT2R.png" } {
              set NewSegment [lappend NewSegment "\tAND("]
              set NewSegment [lappend NewSegment "\tLDN\t$variabile\t$commento"]
            }
	    set parentesi_aperte [expr $parentesi_aperte + 1]
          } else {
            if { [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1.png" || \
                 [file tail [$w itemconfig \
                 cont$riga_corrente:$colonna_corrente -bitmap]] == "CONT1R.png" } {
	      set variabile [lindex [$w itemconfigure \
                var$riga_corrente:$colonna_corrente -text] 4]
              set commento $ContactComment($riga_corrente:$colonna_corrente)
              set NewSegment [lappend NewSegment "\tAND\t$variabile\t$commento"]
            } else {
	      set variabile [lindex [$w itemconfigure \
                var$riga_corrente:$colonna_corrente -text] 4]
              set commento $ContactComment($riga_corrente:$colonna_corrente)
              set NewSegment [lappend NewSegment "\tANDN\t$variabile\t$commento"]
            }
          }
	incr colonna_corrente 
        } 
        if [info exist incroci_eq($riga_corrente)] {
          set lun_incroci [expr [llength $incroci_eq($riga_corrente)] - 1]
          set incroci_eq($riga_corrente) \
            [lrange $incroci_eq($riga_corrente) 1 $lun_incroci] 
        }
      }
    } 
} else {
    # Istruzione ST che chiude l'equazione
      # Chiudo tutte le parentesi ancora aperte
      while { $parentesi_aperte > 0 } {
        set NewSegment [lappend NewSegment "\t)"]
        set parentesi_aperte [expr $parentesi_aperte - 1]
      }
      foreach x [array names incroci_eq] {
        if { [lsearch -exact $incroci_eq($x) \
              $ContactDef($canvasCorrente,col_fine_equazione)] != -1 } {
          if { [file tail [$w itemconfig \
             cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT3.png" || \
             [file tail [$w itemconfig \
             cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT3R.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tST\t$variabile\t$commento"]
          } elseif { [file tail [$w itemconfig \
              cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT4.png" || \
              [file tail [$w itemconfig \
              cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT4R.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tSTN\t$variabile\t$commento"]
          } elseif { [file tail [$w itemconfig \
              cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT5.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tS\t$variabile\t$commento"]
          } elseif { [file tail [$w itemconfig \
              cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "CONT6.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tR\t$variabile\t$commento"]
          } elseif { [file tail [$w itemconfig \
               cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "JMPC.png" } {
            set variabile [lindex [$w itemconfigure \
              var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tJMPC\t$variabile\t$commento"]
          } elseif { [file tail [$w itemconfig \
              cont$riga_corrente:[expr $colonna_corrente -1] -bitmap]] == "JMPCN.png" } {
            set variabile [lindex [$w itemconfigure \
               var$riga_corrente:[expr $colonna_corrente -1] -text] 4]
            set commento $ContactComment($riga_corrente:[expr $colonna_corrente -1])
            set NewSegment [lappend NewSegment "\tJMPCN\t$variabile\t$commento"]
          } 
        incr riga_corrente
        }
      }
    }
  }
  # caso dei JMP assoluti
  for { set JmpAbs $riga_corrente } { $JmpAbs <= \
        [expr $ContactDef($canvasCorrente,riga_fine_edt) -1] } { incr JmpAbs} {
    if { [file tail [$w itemconfig cont$JmpAbs:0 -bitmap]] == "JMPC.png" } {
      set variabile [lindex [$w itemconfigure var$JmpAbs:0 -text] 4]
      set commento $ContactComment($JmpAbs:0)
      set NewSegment [lappend NewSegment "\tJMP\t$variabile\t$commento"]
    } 
  } 
  return $NewSegment
}

####################################################################
# Procedura InserisciContatto
# Procedura che disegna il contatto
# Input: 
#   canvas    Nome del widget cancas
#   contact   ????
#   Modifica  ????
# Ritorna :
#   Niente
####################################################################
proc InserisciContatto {canvas contact Modifica} {
global ContactDef ContactComment AlberoLadder 

  set canvasCorrente [lindex [split $canvas .] end]

  if { $ContactDef($canvasCorrente,col_fine_equazione) > 0 && \
        $ContactDef($canvasCorrente,curr_colonna) >= \
        $ContactDef($canvasCorrente,col_fine_equazione) } {
    AlertBox -text MsgNoContactInsert
    return
  }

  if { [string first "JMPC" $contact] == -1 } {
    set NomeVariabileTmp [string toupper [PromptBox -text PromptVariableName \
      -text2 PromptLineComment -title PromptVarName -numentry 2 \
      -uppercase 1]]
    set NomeVariabile [VerificaOperando [winfo parent [winfo parent $canvas]].t \
       "LD" [lindex $NomeVariabileTmp 0] 0]
  } else {
    set NomeVariabileTmp [string toupper [PromptBox -text MsgLabelprompt \
      -text2 PromptLineComment -title MsgLabelTitle -numentry 2]]
    set NomeVariabile [string toupper [lindex $NomeVariabileTmp 0]]
  }
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
  DrawCont  $canvas $ContactDef($canvasCorrente,curr_riga) $ContactDef($canvasCorrente,curr_colonna) $contact $NomeVariabile "ldil"

  foreach x $AlberoLadder(edit) { 
    if { [string first $ContactDef($canvasCorrente,curr_riga): $x] != -1 } {
      set primosplit [split $x "|"]
      set riga_iniziale [lindex [split [lindex $primosplit 0] ":"] 0]
      set colonna_iniziale [lindex [split [lindex $primosplit 0] ":"] 1]
      set colonna_finale [lindex [split [lindex $primosplit 1] ":"] 1]
      if { $riga_iniziale == $ContactDef($canvasCorrente,curr_riga) && \
           $colonna_iniziale < $ContactDef($canvasCorrente,curr_colonna) && \
           $colonna_finale > $ContactDef($canvasCorrente,curr_colonna) } {
        $canvas delete conn$x
        set indice [lsearch -exact $AlberoLadder(edit) $x]
       set AlberoLadder(edit) [concat [lrange $AlberoLadder(edit) 0 [expr $indice -1]] \
         [lrange $AlberoLadder(edit) [expr $indice + 1] end]]
      }
    }
  }

  # Ridimensiono il canvas in cui vado ad inserire il nuovo contatto
  DimensionaCanvas $canvas [expr $ContactDef($canvasCorrente,riga_fine_edt) + 1] \
    [expr $ContactDef($canvasCorrente,col_fine_edt) + 1] 
  # Se Modifica == 1 la fz e' stata chiamata dalla fz Smista:Comando.. altrimenti
  # da Inserisci:colonna.....
  if { $Modifica == 1 } {
    if { $ContactDef($canvasCorrente,curr_colonna) < \
      [expr $ContactDef(max_colonna) - 1] } {
      scrollRight $canvas
    } else {
      scrollDown $canvas
    }
  }
  $canvas delete barra1
  DrawBarra $canvas
}

####################################################################
# Procedura SalvaSegmento
# Procedura che salva ed inserisce il segmento nuovo o modificato
# Input: 
#   NewSegment ???
#   canvas     Nome del widget cancas
#   t          Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc SalvaSegmento { NewSegment canvas t} {
global ContactDef RisultatoParametri ContactComment

  if { $NewSegment == -1 } {
    return
  }

  SetModiFile $t 0

  if { $ContactDef(TipoEdit) == "FIRST" } {
    if { [string first "PROGRAM" [$t get 1.0 end]] == -1 } {
      $t insert end "PROGRAM NONAME\n\n"
    }
    $t insert end "$ContactComment(EqComm)\n"
    foreach linea $NewSegment {
      $t insert end "$linea\n"
    } 
    set ContactDef(linee_equazioni) ""
    set ContactDef(linee_equazioni) [lappend ContactDef(linee_equazioni) \
       [expr 3 + [llength [split $ContactComment(EqComm) "\n"]]]]
    set ContactDef(linee_equazioni) [lappend ContactDef(linee_equazioni) \
       [lindex [split [$t index end] .] 0]]
    set ContactDef(linee_equazioni_FB) 0 
    CreaSegmentCanvas [TextToTop $t] 0
    set ContactDef(ordine_seg) [lappend ContactDef(ordine_seg) 0]
    set righe_colonne_occupate [AnalizeIlEquation $canvas.0 $t 0]
    DimensionaCanvas $canvas.0 [lindex $righe_colonne_occupate 0] \
       [lindex $righe_colonne_occupate 1] 
    ResettaCanvasEdit $canvas.edit
    set posizione_seg_prio [$canvas bbox segment0]
    $canvas configure -scrollregion "0 0 [lindex $posizione_seg_prio 2] \
       [lindex $posizione_seg_prio 3]" 
    MostraLadder $canvas
    set ContactDef(SegmentoCorrente) 0
    $canvas.0 configure -background lemonchiffon
  } elseif { $ContactDef(TipoEdit) == "NEW" } {
    if { $ContactDef(SegmentoCorrente) == 0 } {
      if { $ContactDef(MaxSegment) == -1 } {
        set Direzione sopra
      } else {
        if [ConfirmBox -text [GetString ConfirmUp]] {
          set Direzione sopra
        } else {
          set Direzione sotto
        }
      }
    } else {
      set Direzione sotto
    }
    if { $Direzione == "sotto" } {
      set PuntoInserimento [lindex $ContactDef(linee_equazioni) \
        [expr $ContactDef(SegmentoCorrente) + 1]]
      if { $PuntoInserimento == "" } {
        set PuntoInserimento 1
      }
      # Vado alla ricerca della linea di fine segmento precedente
      # includendo il caso di commenti intersegmento
      set index_linea [expr $PuntoInserimento - 1]
      set linea [$t get $index_linea.0 $index_linea.end]
      while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "" || \
             [lindex $linea 0] == "END_PROGRAM") \
              && ($index_linea >=0)} {
        incr PuntoInserimento -1
        incr index_linea -1
        set linea [$t get $index_linea.0 $index_linea.end]
      }
    } else {
      set PuntoInserimento 2
    } 

    # Inserisco il nuovo segmento
    $t insert $PuntoInserimento.0 "\n"
    incr PuntoInserimento
    $t insert $PuntoInserimento.0 "$ContactComment(EqComm)\n"
    set PuntoInserimento [expr $PuntoInserimento + \
       [llength [split $ContactComment(EqComm) "\n"]]]
    if { $Direzione == "sotto" } {
      set ContactDef(linee_equazioni) [concat [lrange $ContactDef(linee_equazioni) \
          0 $ContactDef(SegmentoCorrente)] \
          $PuntoInserimento [lrange $ContactDef(linee_equazioni) \
          [expr $ContactDef(SegmentoCorrente) +1] end ]]
    } else {
      set ContactDef(linee_equazioni) [concat $PuntoInserimento \
        $ContactDef(linee_equazioni)] 
    }
    foreach linea $NewSegment {
      $t insert $PuntoInserimento.0 "$linea\n" 
      incr PuntoInserimento
    } 
    $t insert $PuntoInserimento.0 "\n"
    set RigheAggiunte [expr [llength $NewSegment] + 2 + \
       [llength [split $ContactComment(EqComm) "\n"]]]
    if { $Direzione == "sotto" } {
      for { set i [expr $ContactDef(SegmentoCorrente) +2] } \
          { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
        set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
            [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
      } 

      set ContactDef(linee_equazioni_FB) [concat [lrange $ContactDef(linee_equazioni_FB) \
          0 $ContactDef(SegmentoCorrente)] \
          0 [lrange $ContactDef(linee_equazioni_FB) \
          [expr $ContactDef(SegmentoCorrente) +1] end ]]
      incr ContactDef(MaxSegment) 
      CreaSegmentCanvas [TextToTop $t] $ContactDef(MaxSegment)
      set ContactDef(ordine_seg) [concat [lrange $ContactDef(ordine_seg) \
          0 $ContactDef(SegmentoCorrente)] \
          $ContactDef(MaxSegment) [lrange $ContactDef(ordine_seg) \
          [expr $ContactDef(SegmentoCorrente) +1] end ]]
      set righe_colonne_occupate [AnalizeIlEquation $canvas.$ContactDef(MaxSegment) \
        $t [expr $ContactDef(SegmentoCorrente) + 1]]
      DimensionaCanvas $canvas.$ContactDef(MaxSegment) \
        [lindex $righe_colonne_occupate 0] [lindex $righe_colonne_occupate 1] 
      ResettaCanvasEdit $canvas.edit
      $canvas.$ContactDef(SegmentoCorrente) configure -background white
      incr ContactDef(SegmentoCorrente) 
    } else {
      for { set i [expr $ContactDef(SegmentoCorrente) +1] } \
          { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
        set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
            [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
      } 

      set ContactDef(linee_equazioni_FB) [concat 0 $ContactDef(linee_equazioni_FB)] 
      incr ContactDef(MaxSegment) 
      CreaSegmentCanvas [TextToTop $t] $ContactDef(MaxSegment)
      set ContactDef(ordine_seg) [concat $ContactDef(MaxSegment) $ContactDef(ordine_seg)]
      set righe_colonne_occupate [AnalizeIlEquation $canvas.$ContactDef(MaxSegment) \
        $t $ContactDef(SegmentoCorrente) ]
      DimensionaCanvas $canvas.$ContactDef(MaxSegment) \
        [lindex $righe_colonne_occupate 0] [lindex $righe_colonne_occupate 1] 
      ResettaCanvasEdit $canvas.edit
      $canvas.$ContactDef(SegmentoCorrente) configure -background white
    }
    $canvas.$ContactDef(MaxSegment) configure -background lemonchiffon
    $canvas delete segment$ContactDef(MaxSegment)
    MostraLadder $canvas
  } elseif { $ContactDef(TipoEdit) == "NEWFB" || $ContactDef(TipoEdit) == "NEWMATH" } {
    if { $ContactDef(SegmentoCorrente) == 0 } {
      if { $ContactDef(MaxSegment) == -1 } {
        set Direzione sopra
      } else {
        if [ConfirmBox -text [GetString ConfirmUp]] {
          set Direzione sopra
        } else {
          set Direzione sotto
        }
      }
    } else {
      set Direzione sotto
    }
    if { $Direzione == "sotto" } {
      set PuntoInserimento [lindex $ContactDef(linee_equazioni) \
        [expr $ContactDef(SegmentoCorrente) + 1]]
      if { $PuntoInserimento == "" } {
        set PuntoInserimento 1
      }
      # Vado alla ricerca della linea di fine segmento precedente
      # includendo il caso di commenti intersegmento
      set index_linea [expr $PuntoInserimento - 1]
      set linea [$t get $index_linea.0 $index_linea.end]
      while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "" || \
             [lindex $linea 0] == "END_PROGRAM") && ($index_linea >=0)} {
        incr PuntoInserimento -1
        incr index_linea -1
        set linea [$t get $index_linea.0 $index_linea.end]
      }
    } else {
      $t mark set initline "insert linestart"
      $t mark set endline "insert lineend"
      if { [$t compare initline == 1.0] && [$t compare initline == endline]} {
        $t insert initline "PROGRAM NONAME\n"
      }
      set PuntoInserimento 2
    }

    # Inserisco il nuovo segmento
    set PuntoInserimentoUndo $PuntoInserimento
    $t insert $PuntoInserimento.0 "\n"
    incr PuntoInserimento
    if {$ContactComment(EqComm) != "" } {
      $t insert $PuntoInserimento.0 "$ContactComment(EqComm)\n"
      set PuntoInserimento [expr $PuntoInserimento + \
        [llength [split $ContactComment(EqComm) "\n"]]]
    }
    $t mark set insert $PuntoInserimento.0

    
    if { $ContactDef(TipoEdit) == "NEWFB" } { 
      set function_block [PromptBox -text MsgNewFBprompt -title MsgNewFBTitle\
        -uppercase 1]
      if { $function_block == "" } {
        $t delete $PuntoInserimentoUndo.0 $PuntoInserimento.end
        return
      }
      SelectionFunctionBlock $t [string toupper $function_block] NewFB
    } elseif { $ContactDef(TipoEdit) == "NEWMATH" } {
      PannelloEquazioneMatematica $t ""
    }

    if { $PuntoInserimento == [lindex [split [$t index insert] {.}] 0] } {
      $t delete $PuntoInserimentoUndo.0 $PuntoInserimento.end
      return
    }
    $t insert insert "\n"

    if { $Direzione == "sotto" } {
      set ContactDef(linee_equazioni) [concat [lrange $ContactDef(linee_equazioni) \
          0 $ContactDef(SegmentoCorrente)] \
          $PuntoInserimento [lrange $ContactDef(linee_equazioni) \
          [expr $ContactDef(SegmentoCorrente) +1] end ]]
      set RigheAggiunte [expr [lindex [split [$t index insert] {.}] 0] - \
        $PuntoInserimento + 1 + [llength [split $ContactComment(EqComm) "\n"]]] 
      for { set i [expr $ContactDef(SegmentoCorrente) +2] } \
          { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
        set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
            [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
      } 

      if { $ContactDef(TipoEdit) == "NEWFB" } { 
        set ContactDef(linee_equazioni_FB) [concat [lrange \
            $ContactDef(linee_equazioni_FB) 0 $ContactDef(SegmentoCorrente)] \
            1 [lrange $ContactDef(linee_equazioni_FB) \
            [expr $ContactDef(SegmentoCorrente) +1] end ]]
      } elseif { $ContactDef(TipoEdit) == "NEWMATH" } {
        set ContactDef(linee_equazioni_FB) [concat [lrange \
            $ContactDef(linee_equazioni_FB) 0 $ContactDef(SegmentoCorrente)] \
            2 [lrange $ContactDef(linee_equazioni_FB) \
            [expr $ContactDef(SegmentoCorrente) +1] end ]]
      }

      incr ContactDef(MaxSegment) 
      CreaSegmentCanvas [TextToTop $t] $ContactDef(MaxSegment)
      set ContactDef(ordine_seg) [concat [lrange $ContactDef(ordine_seg) \
          0 $ContactDef(SegmentoCorrente)] \
          $ContactDef(MaxSegment) [lrange $ContactDef(ordine_seg) \
          [expr $ContactDef(SegmentoCorrente) +1] end ]]
      set righe_colonne_occupate [AnalizeIlEquation $canvas.$ContactDef(MaxSegment) $t \
          [expr $ContactDef(SegmentoCorrente) + 1]]
      DimensionaCanvas $canvas.$ContactDef(MaxSegment) \
          [lindex $righe_colonne_occupate 0] [lindex $righe_colonne_occupate 1] 
      $canvas.$ContactDef(SegmentoCorrente) configure -background white
      incr ContactDef(SegmentoCorrente) 
    } else {
      set ContactDef(linee_equazioni) [concat $PuntoInserimento \
          $ContactDef(linee_equazioni)]
      set RigheAggiunte [expr [lindex [split [$t index insert] {.}] 0] - \
          $PuntoInserimento + 1 + [llength [split $ContactComment(EqComm) "\n"]]] 
      for { set i [expr $ContactDef(SegmentoCorrente) +1] } \
          { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
        set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
            [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
      } 

      if { $ContactDef(TipoEdit) == "NEWFB" } { 
        if { [info exist ContactDef(linee_equazioni_FB)] } {
          set ContactDef(linee_equazioni_FB) [concat 1 $ContactDef(linee_equazioni_FB)]
        } else {
          set ContactDef(linee_equazioni_FB) 1 
        }
      } elseif { $ContactDef(TipoEdit) == "NEWMATH" } {
        if { [info exist ContactDef(linee_equazioni_FB)] } {
          set ContactDef(linee_equazioni_FB) [concat 2 $ContactDef(linee_equazioni_FB)]
        } else {
          set ContactDef(linee_equazioni_FB) 2 
        }
      }
      incr ContactDef(MaxSegment) 
      CreaSegmentCanvas [TextToTop $t] $ContactDef(MaxSegment)
      if { [info exist ContactDef(ordine_seg)] } {
        set ContactDef(ordine_seg) [concat $ContactDef(MaxSegment) \
           $ContactDef(ordine_seg)]
      } else {
        set ContactDef(ordine_seg) $ContactDef(MaxSegment)
      }
      set righe_colonne_occupate [AnalizeIlEquation $canvas.$ContactDef(MaxSegment) $t \
         $ContactDef(SegmentoCorrente) ]
      DimensionaCanvas $canvas.$ContactDef(MaxSegment) \
         [lindex $righe_colonne_occupate 0] [lindex $righe_colonne_occupate 1] 
      $canvas.$ContactDef(SegmentoCorrente) configure -background white
    }
    $canvas.$ContactDef(MaxSegment) configure -background lemonchiffon
    $canvas delete segment$ContactDef(MaxSegment)
    MostraLadder $canvas
  } elseif { $ContactDef(TipoEdit) == "LABELB" } {
    set Etichetta [PromptBox -text MsgLabelPrompt -title MsgLabelTitle]
    if { $Etichetta == "" } {
      return
    }
    set PuntoInserimento [lindex $ContactDef(linee_equazioni) \
        [expr $ContactDef(SegmentoCorrente) + 1]]
    if { $PuntoInserimento == "" } {
      set PuntoInserimento 1
    }
    # Vado alla ricerca della linea di fine segmento precedente
    # includendo il caso di commenti intersegmento
    set index_linea [expr $PuntoInserimento - 1]
    set linea [$t get $index_linea.0 $index_linea.end]
    while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "" || \
            [lindex $linea 0] == "END_PROGRAM") \
            && ($index_linea >=0)} {
      incr PuntoInserimento -1
      incr index_linea -1
      set linea [$t get $index_linea.0 $index_linea.end]
    }

    # Inserisco il nuovo segmento
    $t insert $PuntoInserimento.0 "\n"
    incr PuntoInserimento
    $t insert $PuntoInserimento.0 "$ContactComment(EqComm)\n"
    set PuntoInserimento [expr $PuntoInserimento + \
        [llength [split $ContactComment(EqComm) "\n"]]]
    set ContactDef(linee_equazioni) [concat [lrange $ContactDef(linee_equazioni) \
        0 $ContactDef(SegmentoCorrente)] \
        $PuntoInserimento [lrange $ContactDef(linee_equazioni) \
        [expr $ContactDef(SegmentoCorrente) +1] end ]]
    if { [string first ":" $Etichetta] == -1} {
      set Etichetta "$Etichetta:"
    }
    $t insert $PuntoInserimento.0 "[string toupper $Etichetta]\n\n"
    # il 3 e' per i 2 \n e l'etichetta
    set RigheAggiunte [expr 3 + [llength [split $ContactComment(EqComm) "\n"]]] 
    for { set i [expr $ContactDef(SegmentoCorrente) +2] } \
        { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
      set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
          [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
    } 

    set ContactDef(linee_equazioni_FB) [concat [lrange $ContactDef(linee_equazioni_FB) \
        0 $ContactDef(SegmentoCorrente)] \
        0 [lrange $ContactDef(linee_equazioni_FB) \
        [expr $ContactDef(SegmentoCorrente) +1] end ]]
    incr ContactDef(MaxSegment) 
    CreaSegmentCanvas [TextToTop $t] $ContactDef(MaxSegment)
    set ContactDef(ordine_seg) [concat [lrange $ContactDef(ordine_seg) \
        0 $ContactDef(SegmentoCorrente)] \
        $ContactDef(MaxSegment) [lrange $ContactDef(ordine_seg) \
        [expr $ContactDef(SegmentoCorrente) +1] end ]]
    set righe_colonne_occupate [AnalizeIlEquation $canvas.$ContactDef(MaxSegment) $t \
        [expr $ContactDef(SegmentoCorrente) + 1]]
    DimensionaCanvas $canvas.$ContactDef(MaxSegment) [lindex $righe_colonne_occupate 0] \
        [lindex $righe_colonne_occupate 1] 
    $canvas.$ContactDef(SegmentoCorrente) configure -background white
    incr ContactDef(SegmentoCorrente) 
    $canvas.$ContactDef(MaxSegment) configure -background lemonchiffon
    $canvas delete segment$ContactDef(MaxSegment)
    MostraLadder $canvas
  } elseif { $ContactDef(TipoEdit) == "OLD" } {
    set PuntoInserimentoInizio [lindex $ContactDef(linee_equazioni) \
        $ContactDef(SegmentoCorrente)]
    # Vado alla ricerca della linea di fine segmento precedente
    # includendo il caso di commenti intersegmento
    set index_linea [expr $PuntoInserimentoInizio - 1]
    set linea [$t get $index_linea.0 $index_linea.end]
    while {([lindex $linea 0] == "(*" || [lindex $linea 0] == "") && ($index_linea >=0)} {
      incr PuntoInserimentoInizio -1
      incr index_linea -1
      set linea [$t get $index_linea.0 $index_linea.end]
    }
    # Tiro fuori il commento
    if { [lindex $ContactDef(linee_equazioni_FB) $ContactDef(SegmentoCorrente)] == 1 || \
         [lindex $ContactDef(linee_equazioni_FB) $ContactDef(SegmentoCorrente)] == 2 } {
      set commento [$t get $PuntoInserimentoInizio.0 \
        [expr [lindex $ContactDef(linee_equazioni) $ContactDef(SegmentoCorrente)] -1].end]
      set ContactComment(EqComm) [CommentBox -text [GetString MsgCommPrompt ] \
        -title [GetString MsgCommTitle] -default $commento] 
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
    set LineeVecchioSegmento [expr $PuntoInserimentoFine - $PuntoInserimentoInizio]
    if { [lindex $ContactDef(linee_equazioni_FB) $ContactDef(SegmentoCorrente)] == 1 } {
      set OLDFB [$t get $PuntoInserimentoInizio.0 $PuntoInserimentoFine.end] 
      set nome_fb [lindex $OLDFB [expr [lsearch -exact $OLDFB "CAL"] + 1]]
      set RisultatoParametri [RicavaParametri $t $nome_fb]
      set FunctionBlockUndo [$t get $PuntoInserimentoInizio.0 $PuntoInserimentoFine.end]
    } elseif { [lindex $ContactDef(linee_equazioni_FB) \
        $ContactDef(SegmentoCorrente)] == 2 } {
      set MathUndo [$t get $PuntoInserimentoInizio.0 $PuntoInserimentoFine.end]
    }
    $t delete $PuntoInserimentoInizio.0 $PuntoInserimentoFine.end
    # Inserisco il nuovo segmento
    set PuntoInserimentoUndo $PuntoInserimentoInizio
    $t insert $PuntoInserimentoInizio.0 "\n"
    incr PuntoInserimentoInizio
    if {$ContactComment(EqComm) != "" } {
      $t insert $PuntoInserimentoInizio.0 "$ContactComment(EqComm)\n"
      set PuntoInserimentoInizio [expr $PuntoInserimentoInizio + \
          [llength [split $ContactComment(EqComm) "\n"]]]
    }
    if { [lindex $ContactDef(linee_equazioni_FB) $ContactDef(SegmentoCorrente)] == 0 } {
      set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) \
          $ContactDef(SegmentoCorrente) $ContactDef(SegmentoCorrente) \
          $PuntoInserimentoInizio ] 
      foreach linea $NewSegment {
        $t insert $PuntoInserimentoInizio.0 "$linea\n" 
        incr PuntoInserimentoInizio
      } 
      $t insert $PuntoInserimentoInizio.0 "\n"
      set RigheAggiunte [expr [llength $NewSegment] + \
          [llength [split $ContactComment(EqComm) "\n"]] + 2 - $LineeVecchioSegmento]
    } elseif { [lindex $ContactDef(linee_equazioni_FB) \
               $ContactDef(SegmentoCorrente)] == 1 } { 
      $t mark set insert $PuntoInserimentoInizio.0
      SelectionFunctionBlock $t $nome_fb "OldFB"
      if { $PuntoInserimentoInizio == [lindex [split [$t index insert] {.}] 0] } {
        $t delete $PuntoInserimentoUndo.0 $PuntoInserimentoInizio.end
        $t insert $PuntoInserimentoUndo.0 $FunctionBlockUndo
        return
      }
      $t insert insert "\n"
      set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) \
          $ContactDef(SegmentoCorrente) $ContactDef(SegmentoCorrente) \
          $PuntoInserimentoInizio ] 
      set RigheAggiunte [expr [lindex [split [$t index insert] {.}] 0] - \
          $PuntoInserimentoInizio + 1 + [llength [split $ContactComment(EqComm) "\n"]] \
          - $LineeVecchioSegmento] 
    } elseif { [lindex $ContactDef(linee_equazioni_FB) \
               $ContactDef(SegmentoCorrente)] == 2 } { 
      $t mark set insert $PuntoInserimentoInizio.0
      PannelloEquazioneMatematica $t $MathUndo 
      if { $PuntoInserimentoInizio == [lindex [split [$t index insert] {.}] 0] } {
        $t delete $PuntoInserimentoUndo.0 $PuntoInserimentoInizio.end
        $t insert $PuntoInserimentoUndo.0 $MathUndo
        return
      }
      $t insert insert "\n"
      set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) \
          $ContactDef(SegmentoCorrente) $ContactDef(SegmentoCorrente) \
          $PuntoInserimentoInizio ] 
      set RigheAggiunte [expr [lindex [split [$t index insert] {.}] 0] - \
          $PuntoInserimentoInizio + 1 + \
          [llength [split $ContactComment(EqComm) "\n"]] -$LineeVecchioSegmento] 
    }

    for { set i [expr $ContactDef(SegmentoCorrente) +1] } \
        { $i < [llength $ContactDef(linee_equazioni)] } {incr i} {
      set ContactDef(linee_equazioni) [lreplace $ContactDef(linee_equazioni) $i $i \
          [expr [lindex $ContactDef(linee_equazioni) $i] + $RigheAggiunte]] 
    } 

    $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] delete all
    set righe_colonne_occupate [AnalizeIlEquation \
       $canvas.[lindex $ContactDef(ordine_seg) $ContactDef(SegmentoCorrente)] $t \
       $ContactDef(SegmentoCorrente)]
    DimensionaCanvas $canvas.[lindex $ContactDef(ordine_seg) \
       $ContactDef(SegmentoCorrente)] [lindex $righe_colonne_occupate 0] \
       [lindex $righe_colonne_occupate 1] 
    ResettaCanvasEdit $canvas.edit
    MostraLadder $canvas
  }
}
