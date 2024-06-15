
            ######################################
            #                                    #
            #     File di definizione del        #
            #     box per MSD                    #
            #                                    #
            ######################################


####################################################################
# Procedura DisegnoRack
# Procedura che grafica l'intera disposizione della IOS 
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc DisegnoRack {} {
global SelectedLine elenco_moduli RACK_DEF MsdData
global RackFrame BoardLibrary

  set MsdData(maxm) $MsdData(maxm$MsdData(TipoScheda))
  set region [expr $MsdData(maxm) * $MsdData(lx) + [expr $MsdData(offx)+1]]
  $RackFrame.totslot config -scrollregion "$MsdData(offx) 0 $region \
                   [expr $MsdData(ly) + $MsdData(offx)]"

  # set binding associati alla grafica della IOS quando si usa la tastiera
  bind $RackFrame.totslot <Left> {CambiaModuloSelezionato %K}
  bind $RackFrame.totslot <Right> {CambiaModuloSelezionato %K}
  bind $RackFrame.totslot <Return> {CmdDefineBoardMsd %W}

  for {set i 0} {$i < $MsdData(maxm)} {incr i} {
    set RACK_DEF(MOD) $i
    if { $MsdData(TipoScheda) == "ios" } {
      if { $elenco_moduli($SelectedLine,$i)==-1} {
        set RACK_DEF(IN) 0
        set RACK_DEF(OUT) 0
      } else {
        set elemento $elenco_moduli($SelectedLine,$i)
        set RACK_DEF(IN) [lindex $elemento 1]
        set RACK_DEF(OUT) [lindex $elemento 2]
      }
    } elseif {$MsdData(TipoScheda) == "udc" } {
      foreach TipoModulo [array names BoardLibrary] {
        if {[lindex $BoardLibrary($TipoModulo) 3] == "UDC"} {
          set RACK_DEF(IN) [lindex $BoardLibrary($TipoModulo) 1]
          set RACK_DEF(OUT) [lindex $BoardLibrary($TipoModulo) 2]
        }
      }
    } elseif {$MsdData(TipoScheda) == "mpp" } {
      foreach TipoModulo [array names BoardLibrary] {
        if {[lindex $BoardLibrary($TipoModulo) 3] == "MPP"} {
          set RACK_DEF(IN) [lindex $BoardLibrary($TipoModulo) 1]
          set RACK_DEF(OUT) [lindex $BoardLibrary($TipoModulo) 2]
        }
      }
    }

    DisegnoSlot
    $RackFrame.totslot bind nmod$MsdData(TipoScheda)$RACK_DEF(MOD) <Button-1> \
       "SelezioneModulo %W nmod$MsdData(TipoScheda) $RACK_DEF(MOD)"
    $RackFrame.totslot bind mod$MsdData(TipoScheda)$RACK_DEF(MOD) <Button-1> \
       "SelezioneModulo %W nmod$MsdData(TipoScheda) $RACK_DEF(MOD)"
    if { $MsdData(TipoScheda) == "ios" } {
      $RackFrame.totslot bind nmod$MsdData(TipoScheda)$RACK_DEF(MOD) <Double-Button-1> \
        "SelModuloELib %W nmod$MsdData(TipoScheda) $RACK_DEF(MOD)"
      $RackFrame.totslot bind mod$MsdData(TipoScheda)$RACK_DEF(MOD) <Double-Button-1> \
        "SelModuloELib %W nmod$MsdData(TipoScheda) $RACK_DEF(MOD)"
    }
  }
  SetVariabiliRack    ;#preset array globale RACK_DEF
  if { $MsdData(TipoScheda) == "udc" ||
       $MsdData(TipoScheda) == "mpp" } {
    DisegnoModulo 1
  }
}

####################################################################
# Procedura DisegnoSlot
# Procedura che smista le funzioni di grafica del modulo
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc DisegnoSlot { } {
global RACK_DEF RackFrame

  if { $RACK_DEF(IN)==0 && $RACK_DEF(OUT)==0 } {
    BoxVuoto $RackFrame.totslot $RACK_DEF(MOD)
  } else {
    if { $RACK_DEF(IN) == 0 } {
        BoxOut $RackFrame.totslot $RACK_DEF(MOD)
    } elseif { $RACK_DEF(OUT) == 0 } {
        BoxIn $RackFrame.totslot $RACK_DEF(MOD)
    } else {
        BoxInout $RackFrame.totslot $RACK_DEF(MOD)
    }
  }
}

####################################################################
# Procedura BoxIn 
# Procedura che realizza il box di input, dato il canvas e la 
# posizione in cui si desidera inserirlo
# Input: 
#   wm        Widget dello slot
#   pso       Posizione in cui graficare
# Ritorna :
#   Niente
####################################################################
proc BoxIn { wm  pos} {
  global MsdData 

  set offsety 30
  set bordo [expr $MsdData(lx) / 3.0]
  set div [expr ($MsdData(lx)-$bordo*2)/2]
  set x1 [expr $MsdData(offx) + $bordo + $div]      ;# punta freccia in
  set i $pos

  BoxVuoto $wm $i

  # creo le figure
  $wm create poly \
  	[expr $x1 + ($i * $MsdData(lx))]        [expr 30 + $offsety] \
 	[expr $x1-$div + ($i * $MsdData(lx))]   [expr 40 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 40 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 55 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 55 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 40 + $offsety] \
	[expr $x1+$div + ($i * $MsdData(lx))]   [expr 40 + $offsety] \
	-fill green4 -tag mod$i -outline black

  $wm create line  [expr 10 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 28 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
		 -fill black -tag mod$i
}

####################################################################
# Procedura BoxOut 
# Procedura che realizza il box di output, dato il canvas e la 
# posizione in cui si desidera inserirlo
# Input: 
#   wm        Widget dello slot
#   pso       Posizione in cui graficare
# Ritorna :
#   Niente
####################################################################
proc BoxOut { wm  pos} {
  global MsdData 

  set offsety 30
  set bordo [expr $MsdData(lx) / 3.0]
  set div [expr ($MsdData(lx)-$bordo*2)/2]
  set x1 [expr $MsdData(offx) + $bordo + $div]           ;# punta freccia out
  set i $pos

  BoxVuoto $wm $i

  # creo le figure
  $wm create poly \
	[expr $x1 + ($i * $MsdData(lx))]        [expr 55 + $offsety] \
	[expr $x1-$div + ($i * $MsdData(lx))]   [expr 45 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 45 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 30 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 30 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 45 + $offsety] \
	[expr $x1+$div + ($i * $MsdData(lx))]   [expr 45 + $offsety] \
	-fill red -tag mod$i -outline black

  $wm create line  [expr 10 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 28 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
		 -fill black -tag mod$i
}

####################################################################
# Procedura BoxInout 
# Procedura che realizza il box di input/output, dato il canvas e la 
# posizione in cui si desidera inserirlo
# Input: 
#   wm        Widget dello slot
#   pso       Posizione in cui graficare
# Ritorna :
#   Niente
####################################################################
proc BoxInout { wm pos } {
global MsdData 

  set offsety 30
  set i $pos
  set bordo 4
  set div [expr ($MsdData(lx)-$bordo*2)/4]
  set x1 [expr $MsdData(offx) + $bordo + $div]		;# punta freccia in
  set x2 [expr $MsdData(offx) + $bordo + 3*$div]		;# punta freccia out

  BoxVuoto $wm $i

  # creo le figure
  $wm create poly \
	[expr $x1 + ($i * $MsdData(lx))]        [expr 30 + $offsety] \
	[expr $x1-$div + ($i * $MsdData(lx))]   [expr 40 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 40 + $offsety] \
	[expr $x1-$div+3 + ($i * $MsdData(lx))] [expr 55 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 55 + $offsety] \
	[expr $x1+$div-3 + ($i * $MsdData(lx))] [expr 40 + $offsety] \
	[expr $x1+$div + ($i * $MsdData(lx))]   [expr 40 + $offsety] \
	-fill green4 -tag mod$i -outline black

  $wm create poly \
        [expr $x2 + ($i * $MsdData(lx))]        [expr 55 + $offsety] \
	[expr $x2-$div + ($i * $MsdData(lx))]   [expr 45 + $offsety] \
        [expr $x2-$div+3 + ($i * $MsdData(lx))] [expr 45 + $offsety] \
        [expr $x2-$div+3 + ($i * $MsdData(lx))] [expr 30 + $offsety] \
        [expr $x2+$div-3 + ($i * $MsdData(lx))] [expr 30 + $offsety] \
        [expr $x2+$div-3 + ($i * $MsdData(lx))] [expr 45 + $offsety] \
        [expr $x2+$div + ($i * $MsdData(lx))]   [expr 45 + $offsety] \
	-fill red -tag mod$i -outline black

  $wm create line  [expr 10 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 16 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 0 + $offsety] \
                 [expr 22 + ($i * $MsdData(lx))] [expr 5 + $offsety] \
                 [expr 28 + ($i * $MsdData(lx))] [expr 5 + $offsety]  \
	-fill black -tag mod$i 
}

####################################################################
# Procedura BoxVuoto 
# Procedura che realizza il box vuoto, dato il canvas e la 
# posizione in cui si desidera inserirlo
# Input: 
#   wm        Widget dello slot
#   pso       Posizione in cui graficare
# Ritorna :
#   Niente
####################################################################
proc BoxVuoto { wm pos} {
  global MsdData elenco_moduli SelectedModule EditPrefs
  global SelectedLine Home

  set i $pos

  $wm create rect [expr $MsdData(offx) + $i * $MsdData(lx)] 2 \
      [expr $MsdData(offx) + [expr $i+1] * $MsdData(lx)] $MsdData(numy) \
      -tag nmod$MsdData(TipoScheda)$i -fill lightgrey 
  $wm create text [expr $i * $MsdData(lx) +$MsdData(lx)/2] \
      [expr $MsdData(numy)-8] -text $i -font $EditPrefs(windowfont) -fill red \
      -tag nmod$MsdData(TipoScheda)$i  
  if { [lindex $elenco_moduli($SelectedLine,$i) end] == "S" && \
       $MsdData(TipoScheda) == "ios"} { 
    $wm create rect [expr $MsdData(offx) + $i * $MsdData(lx)] $MsdData(numy) \
      [expr $MsdData(offx) + [expr $i+1] * $MsdData(lx)] $MsdData(ly) \
      -tag mod$MsdData(TipoScheda)$i -fill grey10 -stipple @$Home/icone/gray25.png
  } else { 
    $wm create rect [expr $MsdData(offx) + $i * $MsdData(lx)] $MsdData(numy) \
        [expr $MsdData(offx) + [expr $i+1] * $MsdData(lx)] $MsdData(ly) \
        -tag mod$MsdData(TipoScheda)$i -fill lightgrey 
  }

  if {$i == $SelectedModule} {
   ReverseSlot $wm nmod$MsdData(TipoScheda) $i 1 
  }
}

####################################################################
# Procedura CreaByteRect
# Procedura che disegna il modulo con la definizione dei byte
# Input: 
#   wm        Widget del canvas
#   offy_agg  ???
# Ritorna :
#   Niente
####################################################################
proc CreaByteRect { canvas offy_agg} { 
global MsdData Home RACK_DEF EditPrefs

  if { $RACK_DEF(TIPOMODULO) == "SIR88" || \
       $RACK_DEF(TIPOMODULO) == "SOR88" } { 
    set numero_byte "0"
    $canvas create text [expr $MsdData(ixmod)+$MsdData(lxmod)-35] \
        [expr $MsdData(distfili)*(4) + $MsdData(iymod) + $offy_agg] \
        -tag tagmod -text $numero_byte -font $EditPrefs(windowfont) -fill black -anchor w
    set numero_byte "1"
    $canvas create text [expr $MsdData(ixmod)+$MsdData(lxmod)-35] \
       [expr $MsdData(distfili)*(12) + $MsdData(iymod) + $offy_agg] \
        -tag tagmod -text $numero_byte -font $EditPrefs(windowfont) -fill red -anchor w

    # Creo il rettangolo di descrizione byte
    $canvas create bitmap [expr $MsdData(ixmod)+40] \
        [expr $MsdData(iymod)+150 + $offy_agg] \
        -bitmap @$Home/icone/S16IO.png \
        -tag tagmod 
  } elseif { $RACK_DEF(TIPOMODULO) == "SI24" || \
             $RACK_DEF(TIPOMODULO) == "SI816" || \
             $RACK_DEF(TIPOMODULO) == "SO24" || \
             $RACK_DEF(TIPOMODULO) == "SO816" } { 
    set numero_byte "0"
    $canvas create text [expr $MsdData(ixmod)+$MsdData(lxmod)-35] \
        [expr $MsdData(distfili)*(4) + $MsdData(iymod) + $offy_agg] \
        -tag tagmod -text $numero_byte -font $EditPrefs(windowfont) -fill blue -anchor w
    set numero_byte "1"
    $canvas create text [expr $MsdData(ixmod)+$MsdData(lxmod)-35] \
       [expr $MsdData(distfili)*(12) + $MsdData(iymod) + $offy_agg] \
        -tag tagmod -text $numero_byte -font $EditPrefs(windowfont) -fill cyan -anchor w
    set numero_byte "2"
    $canvas create text [expr $MsdData(ixmod)+$MsdData(lxmod)-35] \
        [expr $MsdData(distfili)*(20) + $MsdData(iymod) + $offy_agg] \
        -tag tagmod -text $numero_byte -font $EditPrefs(windowfont) -fill gray -anchor w
    $canvas create bitmap [expr $MsdData(ixmod)+40] \
        [expr $MsdData(iymod)+150 + $offy_agg] \
        -bitmap @$Home/icone/S24IO.png \
        -tag tagmod 
  } elseif { $RACK_DEF(TIPOMODULO) == "CanOp128" } { 
#		puts "Si tratta di un CanOp128 !!"
  }
}

####################################################################
# Procedura ReverseSlot
# Procedura che mette in REVERSE il numero dello slot nel rack quando un
# modulo e' selezionato	
# Input: 
#   wm        Widget del canvas
#   basetag   Nome del tag di base
#   n         Numero dello slot
#   onoff     Flag che indica se deve essere colorato o no
# Ritorna :
#   Niente
####################################################################
proc ReverseSlot { w basetag n onoff } {
global MsdData EditPrefs

  if {$onoff==1} {
    set colore white
  } else {
    set colore lightgrey
  }

  $w create rect [expr $MsdData(offx) + $n * $MsdData(lx)] \
     2 [expr $MsdData(offx) + [expr $n+1] * $MsdData(lx)] \
     $MsdData(numy) -tag $basetag$n -fill $colore

  $w create text [expr $n * $MsdData(lx) +$MsdData(lx)/2] \
     [expr $MsdData(numy)-8] -text $n -font $EditPrefs(windowfont) -fill red \
     -tag $basetag$n
}

####################################################################
# Procedura DisegnoModulo
# Procedura che Disegna il modulo di I/O con la descrizione e 
# i suoi canali di I/O
# Input: 
#   disegna   Flaf (0 Cancella modulo; 1 Disegna modulo)
# Ritorna :
#   Niente
####################################################################
proc DisegnoModulo {disegna } {
global MsdData RACK_DEF EditPrefs
global ModuleFrame BoardLibrary

  set tot_io [expr $RACK_DEF(IN) + $RACK_DEF(OUT)]

  if [info exists ModuleFrame] {
    $ModuleFrame.mod1 delete tagmod	;# cancello il modulo old
    pack forget $ModuleFrame
  }

  if { $disegna==0 } {
    return		;# dovevo solo cancellare il modulo
  }

  if {$disegna==1} {
    # visualizzo il modulo
    set offy_agg 0		;# visualizzo il modulo a inizio pagina
    if {$tot_io <= $MsdData(iovis)} {
      set region [expr ($tot_io+1)*$MsdData(distfili)+1]
      $ModuleFrame.mod1 configure -height "[expr ($tot_io+1)*$MsdData(distfili)+2]"
    } else {
      $ModuleFrame.mod1 configure \
        -height "[expr ($MsdData(iovis)+1)*$MsdData(distfili)+2]"
      set region [expr ($tot_io+1)*$MsdData(distfili)+1]
    }
    $ModuleFrame.mod1 configure -scrollregion "$MsdData(offx) $MsdData(iymod) 640 $region"
  }

  #disegno il rettangolo del modulo
  $ModuleFrame.mod1 create rect \
   	$MsdData(ixmod) \
 	[expr $MsdData(iymod)+$offy_agg] \
 	[expr $MsdData(ixmod)+$MsdData(lxmod)] \
 	[expr ($tot_io+1) * $MsdData(distfili) + $offy_agg] -tag tagmod


  #disegno i fili di I/O di ogni sotto-modulo
  set bit 0
  set maxbit [DefinisciMaxBit out]

  for {set j 0} {$j < $tot_io} {incr j} {
     $ModuleFrame.mod1 create line \
     [expr $MsdData(ixmod)+$MsdData(lxmod)] \
     [expr $MsdData(distfili)*($j+1) + $MsdData(iymod) + $offy_agg] \
     [expr $MsdData(ixmod)+$MsdData(lxmod)+$MsdData(filo)] \
     [expr $MsdData(distfili)*($j+1) + $MsdData(iymod) + $offy_agg] -tag tagmod
     $ModuleFrame.mod1 create text \
     [expr $MsdData(ixmod)+$MsdData(lxmod)-15] \
     [expr $MsdData(distfili)*($j+1) + $MsdData(iymod) + $offy_agg] -tag tagmod \
     -text $bit -font $EditPrefs(windowfont)
     if { $bit < $maxbit } {
       incr bit
     } else {
       set bit 0
       set maxbit [DefinisciMaxBit in]
     }
  }


  #nome del modulo
  $ModuleFrame.mod1 create text [expr $MsdData(ixmod)+15] \
    [expr $MsdData(iymod)+15 + $offy_agg] -tag tagmod \
    -text $RACK_DEF(TIPOMODULO) -font $EditPrefs(windowfont) -fill black -anchor w

  # descrizione del modulo
  set commento [lindex $BoardLibrary($RACK_DEF(TIPOMODULO)) 4]

  #suddivisione del commento su due righe
  set pos [string first "/" $commento]
  if {$pos==-1} {
    set testo1 $commento
  } else {
    incr pos
    regexp {([^/]*)/} $commento aa bb
    set testo1 $bb
    set testo2 [string range $commento $pos end ]
  }

  $ModuleFrame.mod1 create text [expr $MsdData(ixmod)+15] \
   	[expr $MsdData(iymod)+40 + $offy_agg] \
    	-tag tagmod -text $testo1 -font $EditPrefs(windowfont) -fill black -anchor w
  if {$pos != -1} {
    $ModuleFrame.mod1 create text [expr $MsdData(ixmod)+15] \
     	[expr $MsdData(iymod)+60 + $offy_agg] \
     	-tag tagmod -text $testo2 -font $EditPrefs(windowfont) -fill black -anchor w
  }

  CreaByteRect $ModuleFrame.mod1 $offy_agg 

  EstraiVarConnesse $disegna

  # non devo stampare : visualizzo a video
  pack $ModuleFrame.mod1 -side left -fill both -expand true
  pack $ModuleFrame.yscroll -side left -fill both
  pack $ModuleFrame -fill both -expand true -anchor n
}

####################################################################
# Procedura DefinisciMaxBit
# Procedura che Definisce la suddivisione dei bit in yte nel caso di 
# modulini IOS  oppure in bit di input o bit di output in tutti gli 
# altri casi 
# Input: 
#   InOut   ??
# Ritorna :
#   Niente
####################################################################
proc DefinisciMaxBit { InOut } {

global RACK_DEF 

  if {$RACK_DEF(TIPOMODULO) == "SI24" || \
      $RACK_DEF(TIPOMODULO) == "SI8" || \
      $RACK_DEF(TIPOMODULO) == "SI816" || \
      $RACK_DEF(TIPOMODULO) == "SIR88" || \
      $RACK_DEF(TIPOMODULO) == "SO24" || \
      $RACK_DEF(TIPOMODULO) == "SO8" || \
      $RACK_DEF(TIPOMODULO) == "SO816" || \
      $RACK_DEF(TIPOMODULO) == "SOR88" } {
    return 7
  } else {
    if { $InOut == "in" } {
#			puts "DefinisciMaxBit torna IN $RACK_DEF(IN) -1"
      return [expr $RACK_DEF(IN) - 1]
    } elseif { $InOut == "out" } {
#			puts "DefinisciMaxBit torna OUT $RACK_DEF(OUT) -1"
      return [expr $RACK_DEF(OUT) - 1]
    }
  }
}

######################################################################
# Procedura MsdBox
# Procedura che crea il box di connessione IO
# Input: 
#   w           Nome della finestra di testo
# Ritorna :
#   Niente
####################################################################
proc MsdBox { w } {
global WinMsd LineFrame LineNumber SelectedModule
global RackFrame ModuleFrame
global MsdData RACK_DEF MsdModificata EditPrefs

  set SelectedModule 0
  set MsdModificata 0

  #creo la toplevel della pagina MSD
  set WinMsd .msd
  if [winfo exists $WinMsd] {
    return
  }

  set old_focus [focus]

  set MainW [winfo toplevel [focus]]

  grab release [grab current]

  toplevel $WinMsd
  wm title $WinMsd [GetString TitlePanelConnection]
  wm protocol $WinMsd WM_DELETE_WINDOW procx

  # elenco dei frame presenti in pagina MSD
  set menu [frame $WinMsd.menu -borderwidth 2 -relief raised]
  set LineFrame [frame $WinMsd.linea]
  set RackFrame [frame $WinMsd.rack -borderwidth 5]
  set ModuleFrame [frame $WinMsd.modulo -relief raised -borderwidth 1]

  #creo la menu bar
  pack $menu -fill x
  MenusMsdCreate $menu $w

  #creo i bottoni per la selezione del tipo di scheda
  tixSelect $WinMsd.tiposcheda -allowzero false -radio true 
  $WinMsd.tiposcheda add ios -text IOS -font $EditPrefs(windowfont) 
  $WinMsd.tiposcheda add udc -text UDC -font $EditPrefs(windowfont)
  $WinMsd.tiposcheda add mpp -text MPP -font $EditPrefs(windowfont)
  $WinMsd.tiposcheda config -variable MsdData(TipoScheda) -validatecmd CambioScheda
  pack $WinMsd.tiposcheda -side top -pady 5 

  #creo il radiobutton menu per selezionare la linea
  RadiobuttonLineeMsd $LineFrame $LineNumber $MsdData(maxl)

  # canvas del rack (rappresentazione grafica della IOS)
  canvas $RackFrame.totslot\
    -width [expr $MsdData(offx)+$MsdData(lx)*$MsdData(vis)-1] \
    -height $MsdData(ly) -xscrollcommand [list $RackFrame.xscroll set] 

  scrollbar $RackFrame.xscroll -orient horizontal -command [list $RackFrame.totslot xview]

  pack $RackFrame.totslot -expand true -fill x -padx 2 -pady 2 
  pack $RackFrame.xscroll -expand true -fill x -padx 2 -pady 2
  pack $RackFrame -fill x -padx 2 -pady 2 -anchor n

  # canvas del disegno modulo
  canvas $ModuleFrame.mod1 \
    -width [expr $MsdData(offx)+$MsdData(lx)*$MsdData(vis)-24] \
    -height "[expr $MsdData(distfili)*($MsdData(iovis)+1)]" \
    -yscrollcommand [list $ModuleFrame.yscroll set] 

  scrollbar $ModuleFrame.yscroll -orient vertical -command [list $ModuleFrame.mod1 yview]

  DisegnoRack

  wm transient $WinMsd $MainW
  PosizionaFinestre $MainW $WinMsd u  
  focus $WinMsd.linea.0

  grab $WinMsd
  tkwait window $WinMsd 

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }
}

######################################################################
# Procedura CambioScheda 
# Procedura chiamata ogni volta che viene scelta una nuova scheda
# Input: 
#   w           Nome della scheda cambiata
# Ritorna :
#   Niente
####################################################################
proc CambioScheda { w } {
global RackFrame ModuleFrame SelectedLine SelectedModule LineNumber 
global MsdData LineFrame

  set MsdData(SelectedLine$MsdData(TipoScheda)) $SelectedLine
  set MsdData(SelectedModule$MsdData(TipoScheda)) $SelectedModule
  set MsdData(LineNumber$MsdData(TipoScheda)) $LineNumber
  set SelectedLine $MsdData(SelectedLine$w) 
  set SelectedModule $MsdData(SelectedModule$w)
  set LineNumber $MsdData(LineNumber$w)
  set MsdData(TipoScheda) $w
  $RackFrame.totslot delete all
  DisegnoModulo 0
  DisegnoRack
  RadiobuttonLineeMsd $LineFrame $LineNumber $MsdData(maxl)
  return $w
}

######################################################################
# Procedura MenusMsdCreate 
# Procedura per la creazione della menu bar dell' msd
# Input: 
#   mb          Nome della menu bar
#   t           Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MenusMsdCreate {mb t } {
global EditPrefs
  
  set all_menus {file_msd edit_msd}
  set displayed_menus {}
  
  foreach menu $all_menus {
    if [winfo exists $mb.$menu] {
      destroy $mb.$menu
    }

    MkMenu$menu $mb.$menu $t
    catch {
      pack $mb.$menu -in $mb -side left
      lappend displayed_menus $mb.$menu
    }
  }
  SetMnemonicMenuBar $mb
}

####################################################################
# Procedura MkMenufile_msd
# Procedura di creazione del sottomenu file di msd
# Input: 
#   menu   Nome dell sottomenu bar
#   t      Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenufile_msd {menu t} {
global EditPrefs
  
  MenuMenubutton $menu $menu.m MenuFileMsd
  
  MenuCommands $menu.m $t {
    CmdQuitMsd
  }
}

####################################################################
# Procedura MkMenuedit_msd
# Procedura di creazione del sottomenu edit di msd
# Input: 
#   menu  Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenuedit_msd {menu t } {
global EditPrefs SiIoconf
  
  MenuMenubutton $menu $menu.m MenuEditMsd
  
  if { $SiIoconf == "no" } {
    MenuCommands $menu.m $t {
      CmdDefineBoardMsd 
      CmdMoveBoardMsd 
      CmdDeleteBoardMsd 
       -
      CmdConnectMsd
       -
      CmdInsertLineMsd
      CmdDeleteLineMsd
    }
  } elseif { $SiIoconf == "yes" } {
    MenuCommands $menu.m $t {
      CmdDefineBoardMsd 
      CmdMoveBoardMsd 
      CmdDeleteBoardMsd 
      CmdActivateBoardMsd 
      CmdDeactivateBoardMsd 
       -
      CmdConnectMsd
       -
      CmdInsertLineMsd
      CmdDeleteLineMsd
    }
  }
}

####################################################################
# Procedura CmdDefineBoardMsd
# Procedura associata alla voce di definizione modulo
# Input: 
#   w     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CmdDefineBoardMsd { w } {
global SelectedModule LineNumber SelectedLine
global MsdData SiIoconf IoconfModificato
global elenco_moduli

  if { $MsdData(TipoScheda) == "ios" } {
    if { $LineNumber < 1 } {
      # pannello di errore
      AlertBox -text MsgNoIOLine
      return
    }

    if { $SelectedModule == -1 } {
      # pannello di errore
      AlertBox -text MsgNoModSel
      return
    }

    if {$elenco_moduli($SelectedLine,$SelectedModule) == -1} {
      SelectionCard $w 
      if { $SiIoconf == "yes" } {
        set IoconfModificato "yes"
      }
    } else {
      AlertBox -text MsgFreeModule
    }
  }
}

####################################################################
# Procedura CmdConnectMsd
# Procedura associata alla voce di connessione modulo
# Input: 
#   w     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CmdConnectMsd { w } {
global SelectedModule SelectedLine  elenco_moduli
global MsdData

  if { $SelectedModule == -1 } {
    AlertBox -text MsgNoModSel
    return
  }

  if { $MsdData(TipoScheda) == "ios" } {
    if { $elenco_moduli($SelectedLine,$SelectedModule) == -1 } {
      AlertBox -text MsgNoModDef
      return
    }
  }
  ConnectingIoPanel $w
}

####################################################################
# Procedura CmdQuitMsd
# Procedura associata alla voce di uscita da msd
# Input: 
#   w     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CmdQuitMsd { w } {
global WinMsd MsdModificata LineNumber SelectedLine ModuleFrame EditPrefs
global ComandiDos 

  if {$MsdModificata == 1} {
    set filename [GetFilename $w]

    if { [winfo class $w] == "Toplevel" } {
      set w [TopToText $w]
    }

    if { [ConfirmBox -text ConfirmIOChanged] } {
      if { $filename == {} } {
        set filename [FileSelection $w  -title PromptSaveas]
        if {[file extension $filename] == ""} {
          set filename $filename.plc
        }

        if [file exists $filename] {
          if ![ConfirmBox -text ConfirmExistFile] {
            set EditPrefs(fs_diskA) 0
            destroy $WinMsd
            unset ModuleFrame
            unset WinMsd
            return
          }
        }
        SetFilename $w $filename
      }
      WriteMsd $filename $w
    } else {
      AzzeraMsd
      if {$filename != {} } {
        LoadMsd $filename
        LoadModule $filename
      }
    }
  }
  destroy $WinMsd
  unset ModuleFrame
  unset WinMsd
  set EditPrefs(fs_diskA) 0
}

####################################################################
# Procedura CmdInsertLineMsd
# Procedura associata alla voce di inserimento linea
# Input: 
#   w     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CmdInsertLineMsd { w } {
global LineNumber LineFrame MsdData MsdModificata
global elenco_moduli

  if { $LineNumber < $MsdData(maxl) } {
    if { [info exist elenco_moduli($LineNumber,0)] == 0 } {
      for { set i 0 } { $i < $MsdData(maxm) } {incr i } {
        set elenco_moduli($LineNumber,$i) -1
      }
    }
    set LineNumber [expr $LineNumber + 1]
    RadiobuttonLineeMsd $LineFrame $LineNumber $MsdData(maxl)
    set MsdModificata 1
  } else {
   # pannello di errore
   AlertBox -text MsgNoInsertLine
  }
}

####################################################################
# Procedura CmdDeleteLineMsd
# Procedura associata alla voce di cancellazione linea
# Input: 
#   w     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CmdDeleteLineMsd { w } {
global LineNumber LineFrame SelectedLine MsdData SelectedModule
global elenco_moduli MsdModificata VariableList

  if { $LineNumber > 1 } {
    #vedo se la linea ha moduli connessi
    if { $MsdData(TipoScheda) == "ios" } {
      foreach i [array names elenco_moduli] {
        if { [string range $i 0 [string match {,} $i] ]==[expr $LineNumber-1] \
             && $elenco_moduli($i) != -1 } {
          AlertBox -text MsgFreeLine 
          return
        }
      }
    } else {
      foreach i [array names VariableList] {
        if { [lindex $VariableList($i) 8] != {} } {
          if { [string first "." [lindex $VariableList($i) 8]] != -1 } {
            set LineaOccupata [lindex [split [lindex $VariableList($i) 8] "."] 1]
            if { [incr LineaOccupata] == $LineNumber } {
              AlertBox -text MsgFreeLine 
              return
            }
          }
        }
      }
    }

    if {[expr $SelectedLine+1]==$LineNumber && $SelectedLine >0 } {
      set SelectedLine [expr $SelectedLine -1]
      DisegnoModulo 0		;#cancello il modulo
      set SelectedModule 0
      DisegnoRack
    }
    set LineNumber [expr $LineNumber - 1]

    RadiobuttonLineeMsd $LineFrame $LineNumber $MsdData(maxl)
    set MsdModificata 1
  } else {
   # pannello di errore
   AlertBox -text MsgNoDeleteLine 
  }
}


####################################################################
# Procedura RadiobuttonLineeMsd
# Procedura che crea i radiobutton relativi alle linee impiegate
# Input: 
#   lin     Nome del widget contenente i radio button
#   n       Numero di linee correnti
#   max     Numero massimo di linee
# Ritorna :
#   Niente
####################################################################
proc RadiobuttonLineeMsd {lin n max} {
global SelectedLine SelectedModule EditPrefs MsdData

  for {set i 0} {$i < $max} {incr i} {
    if [winfo exists $lin.$i] {
      destroy $lin.$i
    }
  }

  for {set i 0} {$i < $n} {incr i} {
    radiobutton $lin.$i -anchor w \
     -text "[GetString LineNumberCheck] $i  " \
     -variable SelectedLine -value $i \
     -font $EditPrefs(windowfont) \
     -selectcolor $EditPrefs(togglecolor) \
     -command {
       global MsdData SelectedLine SelectedModule
         set SelectedModule 0
         DisegnoRack
         DisegnoModulo 0
     }

   pack $lin.$i -side left
  }
  pack $lin -side top -fill x
}

####################################################################
# Procedura SelezioneModulo
# Procedura che gestisce la selezione del modulo
# Input: 
#   w       Nome della toplevel widget della msd
#   basetag base del nome del tag
#   num     Numero del modulo
# Ritorna :
#   Niente
####################################################################
proc SelezioneModulo { w basetag num } {
global SelectedModule SelectedLine elenco_moduli MsdData

  if {$SelectedModule != -1 } {
    #tolgo dal reverse modulo OLD
    ReverseSlot $w $basetag $SelectedModule 0
  }
  ReverseSlot $w $basetag $num 1
  set SelectedModule $num

  if { $MsdData(TipoScheda) == "ios" } {
    if { $elenco_moduli($SelectedLine,$SelectedModule) != -1 } {
      SetVariabiliRack
      DisegnoModulo 1
    } else {
      DisegnoModulo 0
    }
  } else {
    SetVariabiliRack
    DisegnoModulo 1
  }
}

####################################################################
# Procedura SelModuloELib
# Procedura che seleziono il modulo e apro il box della libraria
# di moduli
# Input: 
#   w       Nome della toplevel widget della msd
#   basetag base del nome del tag
#   num     Numero del modulo
# Ritorna :
#   Niente
####################################################################
proc SelModuloELib { w basetag num } {
  SelezioneModulo $w $basetag $num
  CmdDefineBoardMsd $w
}


####################################################################
# Procedura EstraiVarConnesse
# Procedura che estrae dal vettore delle variabili quelle che sono
# connesse con i moduli
# Input: 
#   disegna Flag che se posto a 1 visualizza il modulo
# Ritorna :
#   Niente
####################################################################
proc EstraiVarConnesse {disegna} {
global VariableList SelectedLine SelectedModule
global MsdData RACK_DEF

#	puts "EstraiVarConnesse ------------------"
  foreach i [array names VariableList] {
    if { $MsdData(TipoScheda) == "ios" } {
#				puts "Tiposcheda = IOS"
      # non e' un IO speciale
			set tipo9 [lindex $VariableList($i) 9]
			set tipo10 [lindex $VariableList($i) 10]
      if { [lindex $VariableList($i) 9] == $SelectedLine && \
           [lindex $VariableList($i) 10] == $SelectedModule } {
#				puts "scrivo la var $i $tipo9 = $SelectedLine $tipo10 = $SelectedModule"
        VarInModulo $disegna $i [lindex $VariableList($i) 5] \
           [lindex $VariableList($i) 11] [lindex $VariableList($i) 3]
       } else {
#				puts "Non scrivo la var $i $tipo9 non $SelectedLine $tipo10 non $SelectedModule"
#        VarInModulo $disegna $i [lindex $VariableList($i) 5] \
#           [lindex $VariableList($i) 11] [lindex $VariableList($i) 3]
			 }
     } else {
#			puts "Tiposcheda != IOS"
      if { [lindex $VariableList($i) 8] != {} && \
           [lindex $VariableList($i) 1] == "BOOL" } {
        if { [string first [string toupper $MsdData(TipoScheda)] \
             [string toupper [lindex $VariableList($i) 8]]] != -1} {
          set ElementiConnessione [split [lindex $VariableList($i) 8] "."]
          set BitOccupato [lindex $ElementiConnessione end]
          if { [lindex $VariableList($i) 3] == "input" && \
                 $RACK_DEF(IN) > $RACK_DEF(OUT) } {
              set BitOccupato [expr $BitOccupato + $RACK_DEF(OUT)]
            } elseif { [lindex $VariableList($i) 3] == "output" && \
                 $RACK_DEF(IN) <= $RACK_DEF(OUT) } {
              set BitOccupato [expr $BitOccupato + $RACK_DEF(IN)]
            }
          if { [lindex $ElementiConnessione 1] == $SelectedLine} {
            VarInModulo $disegna $i [lindex $VariableList($i) 5] \
              $BitOccupato [lindex $VariableList($i) 3]
          }
        }
      }
    }
  }
}

####################################################################
# Procedura CmdMoveBoardMsd 
# Procedura che effettua lo spostamento di un modulo
# Input: 
#   w      Nome della toplevel dell' msd
# Ritorna :
#   Niente
####################################################################
proc CmdMoveBoardMsd { w } {
global SelectedLine SelectedModule elenco_moduli MsdData
global VariableList MsdModificata IoconfModificato SiIoconf

  if { $MsdData(TipoScheda) == "ios" } {
    set MsdData(maxm) $MsdData(maxm$MsdData(TipoScheda))
    if { $SelectedModule == -1 } {
      AlertBox -text MsgNoModSel
      return
    }
    if { $elenco_moduli($SelectedLine,$SelectedModule) == -1 } {
     AlertBox -text MsgNoModDef
     return
    }
    set moveto [PromptBox -text PromptMoveBoardPosition -title PromptMoveBoard]
    if {$moveto < 0 || $moveto >= $MsdData(maxm)} {
      if { $moveto != {} } {
       AlertBox -text MsgWrongPosition
      }
    return
    }
    if { $elenco_moduli($SelectedLine,$moveto) != -1 } {
     AlertBox -text MsgPositionBusy
     return
    }

    foreach i [array names VariableList] {
     if {[lindex $VariableList($i) 10] == $SelectedModule} {
        set VariableList($i) [lreplace $VariableList($i) 10 10 $moveto]
     }
    }

    set elenco_moduli($SelectedLine,$moveto) \
    $elenco_moduli($SelectedLine,$SelectedModule)

    set elenco_moduli($SelectedLine,$SelectedModule) -1
    SetVariabiliRack
    set SelectedModule $moveto
    DisegnoSlot
    SetVariabiliRack
    DisegnoSlot
    set MsdModificata 1
    if { $SiIoconf == "yes" } {
      set IoconfModificato "yes"
    }
  }
}

####################################################################
# Procedura CmdDeleteBoardMsd  
# Procedura che cancella un modulo di I/O
# Input: 
#   w      Nome della toplevel dell' msd
# Ritorna :
#   Niente
####################################################################
proc CmdDeleteBoardMsd { w } {
global SelectedModule VariableList elenco_moduli SelectedLine
global MsdModificata MsdData SiIoconf IoconfModificato

  if { $MsdData(TipoScheda) == "ios" } {
    if { $SelectedModule == -1 } {
     AlertBox -text MsgNoModSel
     return
    }

    if { $elenco_moduli($SelectedLine,$SelectedModule) == -1 } {
     return
    }

    if [ConfirmBox -title ConfirmDelMod] {
      foreach i [array names VariableList] {
        if {[lindex $VariableList($i) 10] == $SelectedModule} {
         set VariableList($i) [lreplace $VariableList($i) 8 11 {} {} {} {}]
        }
      }
      set elenco_moduli($SelectedLine,$SelectedModule) -1
      set MsdModificata 1
      SetVariabiliRack
      DisegnoSlot
      DisegnoModulo 0
      if { $SiIoconf == "yes" } {
        set IoconfModificato "yes"
      }
    }
  }
}

####################################################################
# Procedura CmdActivateBoardMsd
# Procedura che attiva un modulo di I/O precedentemente
# disattivato
# Input: 
#   w      Nome della toplevel dell' msd
# Ritorna :
#   Niente
####################################################################
proc CmdActivateBoardMsd { w } {
global SelectedModule elenco_moduli SelectedLine
global MsdData IoconfModificato

  if { $MsdData(TipoScheda) == "ios" } {
    if { $SelectedModule == -1 } {
     AlertBox -text MsgNoModSel
     return
    }

    if { $elenco_moduli($SelectedLine,$SelectedModule) == -1 } {
     return
    }

    if { [lindex $elenco_moduli($SelectedLine,$SelectedModule) end] == "S"} {
      set elenco_moduli($SelectedLine,$SelectedModule) [lreplace \
          $elenco_moduli($SelectedLine,$SelectedModule) end end "R"]  
      set IoconfModificato "yes"
    }
    DisegnoSlot
    DisegnoModulo 0
  }
}

####################################################################
# Procedura CmdDeactivateBoardMsd
# Procedura che disattiva un modulo di I/O precedentemente
# attivato
# Input: 
#   w      Nome della toplevel dell' msd
# Ritorna :
#   Niente
####################################################################
proc CmdDeactivateBoardMsd { w } {
global SelectedModule elenco_moduli SelectedLine
global MsdData IoconfModificato

  if { $MsdData(TipoScheda) == "ios" } {
    if { $SelectedModule == -1 } {
     AlertBox -text MsgNoModSel
     return
    }

    if { $elenco_moduli($SelectedLine,$SelectedModule) == -1 } {
     return
    }

    if { [lindex $elenco_moduli($SelectedLine,$SelectedModule) end] == "R"} {
      set elenco_moduli($SelectedLine,$SelectedModule) [lreplace \
          $elenco_moduli($SelectedLine,$SelectedModule) end end "S"]  
      set IoconfModificato "yes"
    }
    DisegnoSlot
    DisegnoModulo 0
  }
}

####################################################################
# Procedura VarInModulo
# Procedura di graficazione la variabile nel modulo
# Input: 
#   disegna   Flag che se e' a 1 il modulo e' visualizzato
#   nome      Nome della variabile
#   commento  Commento della variabile
#   nbit      Numero di bit
#   i_0       Specifica se di input o di output
# Ritorna :
#   Niente
####################################################################
proc VarInModulo { disegna nome commento nbit i_o } {
global ModuleFrame MsdData EditPrefs

  if { $disegna==1 } {
   # visualizzo il modulo
   set offy_agg 0
  }

  if { $i_o == "input" } {
# rm
   set colore green4
#   set colore yellow
  } else {
# rm
   set colore red
#   set colore blue
  }
  set off_x [expr $MsdData(filo)+5 + $MsdData(ixmod) + $MsdData(lxmod)]
  set off_y [expr ($nbit+1 )*$MsdData(distfili) + $MsdData(iymod)+ $offy_agg]

  # IN FUTURO BISOGNA OTTENERE LA LARGHEZZA DEL FONT
  set off_xc [expr $off_x+70]

  if { $i_o == "input" } {
#		puts "Scrivo la var di IN a X:$off_x Y:$off_y nbit $nbit "
	} else {
#		puts "Scrivo la var di OUT a X:$off_x Y:$off_y nbit $nbit "
	}

  $ModuleFrame.mod1 create text $off_x $off_y -text $nome -font $EditPrefs(windowfont) \
    -tag tagmod -anchor w -fill $colore

  $ModuleFrame.mod1 create text $off_xc $off_y -text $commento \
    -font $EditPrefs(windowfont) -tag tagmod -anchor w -fill black
}

####################################################################
# Procedura AzzeraMsd
# Procedura di azzeramento dell' MSD
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc AzzeraMsd {} {
global VariableList SelectedLine LineNumber

  foreach i [array names VariableList] {
    if { [lindex $VariableList($i) 8] != {} } {
      if { [string first "." [lindex $VariableList($i) 8]] != -1 } {
       continue
      }
    }
    set VariableList($i) [lrange $VariableList($i) 0 7]
  }
  ResetBoardList
  set SelectedLine 0
  set LineNumber 1
}

####################################################################
# Procedura CambiaModuloSelezionato
# Procedura che cambia il modulo selezionato quando si usano
# i tasti "freccia dx" e "freccia sx"
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc CambiaModuloSelezionato {tasto} {
global SelectedModule MsdData RackFrame

  set MsdData(maxm) $MsdData(maxm$MsdData(TipoScheda))
  if {$tasto=="Right"} {
    if {$SelectedModule < [expr $MsdData(maxm)-1]} {
      set i [expr $SelectedModule+1]
    } else {
      set i 0
    }
  } else {
    if {$SelectedModule==0} {
      set i [expr $MsdData(maxm)-1]
    } else {
      set i [expr $SelectedModule-1]
    }
  }
  if { $i == 0 } {
    $RackFrame.totslot xview moveto 0.0 
  } elseif { $i == [expr $MsdData(maxm) -1]} {
    $RackFrame.totslot xview moveto 1.0 
  } else {
    set PercentualeFinestra [$RackFrame.totslot xview]
    if {$tasto=="Right"} {
      set PercentualeModulo [expr ($i + 1)/$MsdData(maxm).0]
      if { $PercentualeModulo > [lindex $PercentualeFinestra 1] } {
        $RackFrame.totslot xview moveto \
          [expr [lindex $PercentualeFinestra 0] + 1.0/$MsdData(maxm)]
      }
    } else {
      set PercentualeModulo [expr $i/$MsdData(maxm).0]
      if { $PercentualeModulo < [lindex $PercentualeFinestra 0] } {
        $RackFrame.totslot xview moveto \
          [expr [lindex $PercentualeFinestra 0] - 1.0/$MsdData(maxm)]
      }
    }
  }
  SelezioneModulo $RackFrame.totslot nmod$MsdData(TipoScheda) $i
}

####################################################################
# Funzione ResetBoardList
# Con tale funzione si resetta la lista dei moduli connessi
# alla linea ios
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc ResetBoardList {} {
global MsdData elenco_moduli

  set MsdData(maxm) $MsdData(maxm$MsdData(TipoScheda))
  for {set i 0} {$i < $MsdData(maxl)} {incr i} {
    for {set j 0} {$j < $MsdData(maxm)} {incr j} {
      set elenco_moduli($i,$j) -1
    }
  }
}

####################################################################
# Procedura SetVariabiliRack
# Procedura che fa il preset dell' array RACK_DEF di uso
# generale
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc SetVariabiliRack { } {
global RACK_DEF SelectedLine SelectedModule elenco_moduli
global BoardLibrary MsdData 

  if { $MsdData(TipoScheda) == "ios" } {
    if { $elenco_moduli($SelectedLine,$SelectedModule) != -1 } {
      #il modulo era gia' definito : prendo il numero di I/O dal file .mod
      set elemento $elenco_moduli($SelectedLine,$SelectedModule)

      set RACK_DEF(LINE)       $SelectedLine
      set RACK_DEF(MOD)        $SelectedModule
      set RACK_DEF(TIPOMODULO) [lindex $elemento 0]
      set RACK_DEF(TIPOLINEA)  [lindex $elemento 3]
      set RACK_DEF(IN)         [lindex $elemento 1]
      set RACK_DEF(OUT)        [lindex $elemento 2]
    } else {
      set RACK_DEF(IN)         0
      set RACK_DEF(OUT)        0
    }
  } elseif {$MsdData(TipoScheda) == "udc" } {
    foreach TipoModulo [array names BoardLibrary] {
      if {[lindex $BoardLibrary($TipoModulo) 3] == "UDC"} {
        set RACK_DEF(LINE)       $SelectedLine
        set RACK_DEF(MOD) 0
        set RACK_DEF(TIPOMODULO) $TipoModulo 
        set RACK_DEF(TIPOLINEA) UDC
        set RACK_DEF(IN) [lindex $BoardLibrary($TipoModulo) 1]
        set RACK_DEF(OUT) [lindex $BoardLibrary($TipoModulo) 2]
      }
    }
  } elseif {$MsdData(TipoScheda) == "mpp" } {
    foreach TipoModulo [array names BoardLibrary] {
      if {[lindex $BoardLibrary($TipoModulo) 3] == "MPP"} {
        set RACK_DEF(LINE)       $SelectedLine
        set RACK_DEF(MOD) 0
        set RACK_DEF(TIPOMODULO) $TipoModulo 
        set RACK_DEF(TIPOLINEA) MPP
        set RACK_DEF(IN) [lindex $BoardLibrary($TipoModulo) 1]
        set RACK_DEF(OUT) [lindex $BoardLibrary($TipoModulo) 2]
      }
    }
  }
}

####################################################################
# Procedura OkCard
# Procedura di conferma della selezione del modulo
# Input: 
#   w          Nome della toplevel di elenco dei moduli
# Ritorna :
#   Niente
####################################################################
proc OkCard { w } {
global BoardLibrary CardResult EditPrefs 
global SelectedLine  SelectedModule elenco_moduli MsdModificata

  if { $CardResult != {} } {
    if { [lsearch -exact [array names BoardLibrary] $CardResult] != -1 } {
      if {$elenco_moduli($SelectedLine,$SelectedModule) == -1} {
        destroy $w
        set MsdModificata 1
        set elenco_moduli($SelectedLine,$SelectedModule) " \
          $CardResult [lindex $BoardLibrary($CardResult) 1] \
          [lindex $BoardLibrary($CardResult) 2] \
          [lindex $BoardLibrary($CardResult) 3] R"
        SetVariabiliRack
        DisegnoModulo 1
        DisegnoSlot
      } else {
        AlertBox -text MsgFreeModule
      }
    } else {
      AlertBox -text MsgInvalidModule
    }
  }
}

####################################################################
# Procedura CancelCard
# Procedura di conferma della selezione del modulo
# Input: 
#   w           Nome della toplevel di elenco dei moduli
# Ritorna :
#   Niente
####################################################################
proc CancelCard { w } {
global CardResult

  set CardResult ""
  destroy $w
}

####################################################################
# Procedura SelectionCard
# Procedura di creazione di un pannello per la selezione del tipo
# di modulo da inserire nella ios
# Input: 
#   w          Nome della toplevel di definizione Msd
#   args       argomenti vari
# Ritorna :
#   Niente
####################################################################
proc SelectionCard { w } {
global BoardLibrary CardResult EditPrefs 

  set fileprompt [GetString "ModSelFilePrompt" ]
  set title [GetString "TitleModSel"]
    
  set CardResult ""
  
  set old_focus [focus]
  
  if [winfo exists .fs] {
    return
  }

  set MainW [winfo toplevel [focus]]

  grab release [grab current]

  toplevel .fs
  wm title .fs $title
  wm minsize .fs 10 10
  wm protocol .fs WM_DELETE_WINDOW procx

  frame .fs.list
  listbox .fs.list.lb -yscroll ".fs.list.sb set" -font $EditPrefs(windowfont) \
    -width 22 -height 20
  scrollbar .fs.list.sb -command ".fs.list.lb yview"
  frame .fs.file
  label .fs.file.l -text $fileprompt -anchor e -font $EditPrefs(windowfont)
  label .fs.file.modname -text $CardResult -font $EditPrefs(windowfont) \
   -textvariable CardResult
  
  tixButtonBox .fs.b -orientation horizontal

  .fs.b add ok  -image ok -font $EditPrefs(windowfont) \
     -command [format { OkCard %s } .fs]

  .fs.b add cancel -image cancel -font $EditPrefs(windowfont) \
     -command [format { CancelCard %s} .fs]

  CniBalloon [.fs.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.fs.b subwidget cancel] [GetString EXIT] $EditPrefs(windowfont) 500

  pack .fs.list.sb -side $EditPrefs(scrollbarside) -fill y
  pack .fs.list.lb -side left -expand yes -fill both
  
  pack .fs.file.l -side left -pady 10 -padx 10
  pack .fs.file.modname -side left -expand yes -pady 10 -padx 10 -fill x
  
  pack .fs.file -side top -expand yes -fill x
  pack .fs.list -side top -expand yes -fill both
  pack .fs.b -side bottom -fill x

  wm transient .fs $MainW
  PosizionaFinestre $MainW .fs u  

  bind .fs.list.lb <Double-Button-1> {
      [.fs.b subwidget ok] invoke
  }

  bind .fs.list.lb <Return> {
    [.fs.b subwidget ok] invoke
  }

  bind .fs.list.lb <Button-1> {	
    %W selection clear 0 end 
    %W selection set [%W nearest %y]
    set CardResult [lindex [selection get] 0] 
    focus %W
  }

  # Aggiorno il modulo correntemente selezionato
  bind .fs.list.lb <Key-Up> {
    set curr_sel [.fs.list.lb index active]
    if { [expr $curr_sel - 1] < 0 } {
      set file [.fs.list.lb get 0] 
    } else {
      set file [.fs.list.lb get [expr $curr_sel - 1]] 
    }
    set CardResult $file
  }

  bind .fs.list.lb <Key-Down> {
    set curr_sel [.fs.list.lb index active]
    if { [expr $curr_sel + 1] > [.fs.list.lb index end] } {
      set file [.fs.list.lb get end] 
    } else {
      set file [.fs.list.lb get [expr $curr_sel + 1]] 
    }
    set CardResult $file
  }

  bind .fs <Key-Escape> {
    [.fs.b subwidget cancel] invoke
  }

  #inserisco i moduli della libreria nel Box di selezione
  foreach i [array names BoardLibrary] {
     if {[lindex $BoardLibrary($i) 3] == "IOS"} {
       .fs.list.lb insert end $i
     }
  }

  # Inizializzo la selezione
  .fs.list.lb selection set 0
  set CardResult [.fs.list.lb get 0]
  focus .fs.list.lb

  grab .fs
  tkwait window .fs
  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
  return $CardResult
}

####################################################################
# Procedura InserisciBitIO
# Procedura che riempie il list box dei bit del modulo di I/O della IOS
# Input: 
#   ListWdg    Nome del widget del list box
# Ritorna :
#   Niente
####################################################################
proc InserisciBitIO { ListWdg } {
global RACK_DEF

  set bit 0
  if { $RACK_DEF(OUT) < $RACK_DEF(IN) } {
    set primoFor $RACK_DEF(OUT) 
    set maxbit0 [DefinisciMaxBit out]
    set numerazMinNove0 "O 00"
    set numerazMaxNove0 "O 0"
    set maxbit1 [DefinisciMaxBit in]
    set numerazMinNove1 "I  00"
    set numerazMaxNove1 "I  0"
  } else {
    set primoFor $RACK_DEF(IN)
    set maxbit0 [DefinisciMaxBit in]
    set numerazMinNove0 "I  00"
    set numerazMaxNove0 "I  0"
    set maxbit1 [DefinisciMaxBit out]
    set numerazMinNove1 "O 00"
    set numerazMaxNove1 "O 0"
  }

  for {set i 0} {$i < $primoFor} {incr i} {
    if { $bit <= 9 } {
      $ListWdg insert end "$numerazMinNove0$bit :"
    } else {
      $ListWdg insert end "$numerazMaxNove0$bit :"
    }
    if { $bit < $maxbit0 } {
      incr bit
    } else {
      set bit 0
    }
  }

  set bit 0
  for {set i $primoFor } { $i < [expr $RACK_DEF(OUT) + $RACK_DEF(IN)]} {incr i} {
    if { $bit <= 9 } {
      $ListWdg insert end "$numerazMinNove1$bit :"  
    } else {
      $ListWdg insert end "$numerazMaxNove1$bit :"  
    }
    if { $bit < $maxbit1 } {
      incr bit
    } else {
      set bit 0
    }
  }
}

####################################################################
# Procedura OkConnection
# Procedura invocata alla pressione del tasto conferma del pannello
# di connessione variabili IO
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc OkConnection { } {
global MsdModificata connessione_corrente

  set MsdModificata 1
  DisegnoModulo 1
  EstraiVarConnesse 1
  if [info exists connessione_corrente] {
    unset connessione_corrente 
  }
  destroy .conn
}

####################################################################
# Procedura ConnectVariable 
# Procedura invocata alla pressione del tasto connetti del pannello
# di connessione variabili IO
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc ConnectVariable { } {
global RACK_DEF IO_BIT_FREE MsdData
global VariableList iosel varsel 

  if { [.conn.varlist.lb get $varsel] == {} } {
    AlertBox -text MsgNoConnection
    return
  }
  if { $IO_BIT_FREE($iosel) == 1 } {
    AlertBox -text MsgChannelBusy
    return
  }
  if { [string compare [lindex [.conn.iolist.lb get $iosel] 0] "O"] == 0 && \
       [string compare [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] \
         "output"] == 0 || \
       [string compare [lindex [.conn.iolist.lb get $iosel] 0] "I"] == 0 && \
       [string compare [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] \
       "input"] == 0} {
    set connection "[.conn.iolist.lb get $iosel]  [.conn.varlist.lb get $varsel]"
    if {[llength $VariableList([.conn.varlist.lb get $varsel])] > 8 } {
      if { $MsdData(TipoScheda) == "ios" } {
        set VariableList([.conn.varlist.lb get $varsel]) \
            [lreplace $VariableList([.conn.varlist.lb get $varsel]) 8 11 \
            $RACK_DEF(TIPOLINEA) $RACK_DEF(LINE) $RACK_DEF(MOD) $iosel ]
      } else {
        set BitOccupato $iosel
        if { [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] == "input" && \
             $RACK_DEF(IN) > $RACK_DEF(OUT) } {
          set BitOccupato [expr $iosel - $RACK_DEF(OUT)]
        } elseif { [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] == \
                   "output" && $RACK_DEF(IN) <= $RACK_DEF(OUT) } {
          set BitOccupato [expr $iosel - $RACK_DEF(IN)]
        }
        if { $MsdData(TipoScheda) == "udc" } {
          set VariableList([.conn.varlist.lb get $varsel]) \
               [lreplace $VariableList([.conn.varlist.lb get $varsel]) 8 8 \
               IOS.$RACK_DEF(LINE).UDC.$BitOccupato]
        } elseif { $MsdData(TipoScheda) == "mpp" } {
          set VariableList([.conn.varlist.lb get $varsel]) \
              [lreplace $VariableList([.conn.varlist.lb get $varsel]) 8 8 \
              IOS.$RACK_DEF(LINE).MPP.$BitOccupato]
        }
      }
    } else {
      if { $MsdData(TipoScheda) == "ios" } {
        set VariableList([.conn.varlist.lb get $varsel]) \
            [lappend VariableList([.conn.varlist.lb get $varsel]) \
            $RACK_DEF(TIPOLINEA) $RACK_DEF(LINE) $RACK_DEF(MOD) $iosel ]
      } else {
        set BitOccupato $iosel
        if { [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] == "input" && \
             $RACK_DEF(IN) > $RACK_DEF(OUT) } {
          set BitOccupato [expr $iosel - $RACK_DEF(OUT)]
        } elseif { [lindex $VariableList([.conn.varlist.lb get $varsel]) 3] == \
                    "output" && $RACK_DEF(IN) <= $RACK_DEF(OUT) } {
          set BitOccupato [expr $iosel - $RACK_DEF(IN)]
        }
        if { $MsdData(TipoScheda) == "udc" } {
          set VariableList([.conn.varlist.lb get $varsel]) \
              [lappend VariableList([.conn.varlist.lb get $varsel]) \
              IOS.$RACK_DEF(LINE).UDC.$BitOccupato {} {} {} ]
        } elseif { $MsdData(TipoScheda) == "mpp" } {
          set VariableList([.conn.varlist.lb get $varsel]) \
              [lappend VariableList([.conn.varlist.lb get $varsel]) \
              IOS.$RACK_DEF(LINE).MPP.$BitOccupato {} {} {} ]
        }
      }
    }
    # Memorizzo le variabili selezionate correntemente 
    set connessione_corrente [lappend connessione_corrente [.conn.varlist.lb get $varsel]]

    .conn.iolist.lb delete $iosel
    .conn.varlist.lb delete $varsel
    .conn.iolist.lb insert $iosel $connection 
    set IO_BIT_FREE($iosel) 1
    # Ricerco il primo bit libero
    for {set i [expr $iosel +1]} { $i<[expr $RACK_DEF(IN) + $RACK_DEF(OUT)] } { incr i} {
      if { $IO_BIT_FREE($i) == 0 } {
        set iosel $i
        .conn.iolist.lb activate $i
        break
      }
    }
  } else {
    AlertBox -text MsgTypeUncorrect
  }
}

####################################################################
# Procedura FreeVariable 
# Procedura invocata alla pressione del tasto libera del pannello
# di connessione variabili IO
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc FreeVariable { } {
global IO_BIT_FREE VariableList iosel varsel 

  set del_connection [.conn.iolist.lb get $iosel]
  if { [lindex $del_connection 3] !=  ""  } {
    .conn.iolist.lb delete $iosel
    if {[lindex $del_connection 0] == "I"} {
      .conn.iolist.lb insert $iosel \
      "[lindex $del_connection 0]   [lindex $del_connection 1] [lindex $del_connection 2]"
    } else {
      .conn.iolist.lb insert $iosel \
      "[lindex $del_connection 0]  [lindex $del_connection 1] [lindex $del_connection 2]"
    }
    set ContenutoLista [.conn.varlist.lb get 0 end]
    set ContenutoLista [lappend ContenutoLista [lindex $del_connection 3]]
    set ContenutoLista [lsort -ascii $ContenutoLista]
    .conn.varlist.lb delete 0 end
    foreach elemento $ContenutoLista {
      .conn.varlist.lb insert end $elemento
    }
    set VariableList([lindex $del_connection 3]) \
      [lreplace $VariableList([lindex $del_connection 3]) 8 11 {} {} {} {}]
    set IO_BIT_FREE($iosel) 0
    .conn.iolist.lb activate $iosel 
  } else {
    AlertBox -text MsgNoConnection
  }
}

####################################################################
# Procedura FreeAllVariable
# Procedura invocata alla pressione del tasto libera tutto del pannello
# di connessione variabili IO
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc FreeAllVariable { } {
global RACK_DEF IO_BIT_FREE MsdData
global VariableList iosel varsel 

  .conn.iolist.lb delete 0 end 
  .conn.varlist.lb delete  0 end
  set iosel 0
  set varsel 0 
  for {set i 0 } {$i < [expr $RACK_DEF(IN) + $RACK_DEF(OUT)]} {incr i} {
    set IO_BIT_FREE($i) 0
  }
  # inserisco nelle due liste rispettivamente il numero dei moduli
  # e l'elenco delle variabili

  InserisciBitIO .conn.iolist.lb
  if {$RACK_DEF(IN) == 0} {
    set PrimoIO output
    set SecondoIO output
  } elseif {$RACK_DEF(IN) > 0 && $RACK_DEF(OUT) != 0} {
    set PrimoIO input
    set SecondoIO output
  } elseif {$RACK_DEF(OUT) == 0} {
    set PrimoIO input
    set SecondoIO input
  }

  foreach i [lsort [array names VariableList]] {
    if { [lindex $VariableList($i) 1] == "BOOL" && ([lindex $VariableList($i) 3] == \
         $PrimoIO || [lindex $VariableList($i) 3] == $SecondoIO ) && \
         [lindex $VariableList($i) 6] == 1} {
      if {[lindex $VariableList($i) 9] == $RACK_DEF(LINE) && \
          [lindex $VariableList($i) 10] == $RACK_DEF(MOD) && \
          $MsdData(TipoScheda) == "ios"} {
        .conn.varlist.lb insert end $i
	# azzero la variabile 
	set VariableList($i) [lreplace $VariableList($i) 8 11 {} {} {} {}]
       } elseif {[lindex $VariableList($i) 9] == {} && \
                 [lindex $VariableList($i) 10] == {} } {
	if { [lindex $VariableList($i) 8] != {} } {
          if { [string first "." [lindex $VariableList($i) 8]] != -1 } {
            if { [string first [string toupper $MsdData(TipoScheda)] \
                 [string toupper [lindex $VariableList($i) 8]]]!= -1 && \
                 $MsdData(TipoScheda) != "ios"} {
	      # I/O speciali 
              set LineaOccupata [lindex [split [lindex $VariableList($i) 8] "."] 1]
              if { $RACK_DEF(LINE) == $LineaOccupata } {
	        .conn.varlist.lb insert end $i
	        # azzero la variabile 
	        set VariableList($i) [lreplace $VariableList($i) 8 11 {} {} {} {}]
              }
            }
          } else {
            .conn.varlist.lb insert end $i
          }
        } else {
          .conn.varlist.lb insert end $i
        }
      }
    }
  }
}

####################################################################
# Procedura CancelConnection 
# Procedura invocata alla pressione del tasto annulla del pannello
# di connessione variabili IO
# Input: 
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CancelConnection { t } {
global VariableList connessione_corrente
global ModuleFrame RackFrame 

  if [info exists connessione_corrente] {
    foreach i $connessione_corrente {
      set VariableList($i) [lreplace $VariableList($i) 8 11 {} {} {} {}]
    }
    unset connessione_corrente 
  }
  destroy .conn
  AzzeraMsd
  set filename [GetFilename $t]
  if {$filename != {} } {
    LoadMsd $filename
    LoadModule $filename
  } else {
    $ModuleFrame.mod1 delete tagmod     ;# cancello il modulo old
    pack forget $ModuleFrame
    $RackFrame.totslot delete all
    DisegnoModulo 0
    DisegnoRack
  }
}

####################################################################
# Procedura FillConnectionList
# Procedura che riempie le due liste per la connessione
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc  FillConnectionList { } {
global RACK_DEF IO_BIT_FREE MsdData
global VariableList iosel 

  if {$RACK_DEF(IN) == 0} {
    set PrimoIO output
    set SecondoIO output
  } elseif {$RACK_DEF(IN) > 0 && $RACK_DEF(OUT) != 0} {
    set PrimoIO input
    set SecondoIO output
  } elseif {$RACK_DEF(OUT) == 0} {
    set PrimoIO input
    set SecondoIO input
  }

  foreach i [lsort [array names VariableList]] {
    if { [lindex $VariableList($i) 1] == "BOOL" && ([lindex $VariableList($i) 3] == \
          $PrimoIO || [lindex $VariableList($i) 3] == $SecondoIO ) && \
          [lindex $VariableList($i) 6] == 1} {
      if {[lindex $VariableList($i) 9] == $RACK_DEF(LINE) && \
          [lindex $VariableList($i) 10] == $RACK_DEF(MOD) && \
          $MsdData(TipoScheda) == "ios"} {
        set connection "[.conn.iolist.lb get [lindex $VariableList($i) 11]]  $i"
        .conn.iolist.lb delete [lindex $VariableList($i) 11] 
        .conn.iolist.lb insert [lindex $VariableList($i) 11] $connection
        set IO_BIT_FREE([lindex $VariableList($i) 11]) 1
        if {$iosel == [lindex $VariableList($i) 11] } {
          set iosel [expr $iosel + 1]
          .conn.iolist.lb activate $iosel
        }
      } elseif {[lindex $VariableList($i) 9] == {} && \
                [lindex $VariableList($i) 10] == {} } {
        if { [lindex $VariableList($i) 8] != {} } {
          if { [string first "." [lindex $VariableList($i) 8]] != -1 } {
            if { [string first [string toupper $MsdData(TipoScheda)] \
                    [string toupper [lindex $VariableList($i) 8]]]!= -1 && \
                 $MsdData(TipoScheda) != "ios" } {
              # I/O speciali 
              set BitOccupato [lindex [split [lindex $VariableList($i) 8] "."] end]
              set LineaOccupata [lindex [split [lindex $VariableList($i) 8] "."] 1]
              if { [lindex $VariableList($i) 3] == "input" && \
                   $RACK_DEF(IN) > $RACK_DEF(OUT) } {
                set BitOccupato [expr $BitOccupato + $RACK_DEF(OUT)]
              } elseif { [lindex $VariableList($i) 3] == "output" && \
                   $RACK_DEF(IN) <= $RACK_DEF(OUT) } {
                set BitOccupato [expr $BitOccupato + $RACK_DEF(IN)]
              }
              if { $RACK_DEF(LINE) == $LineaOccupata } { 
                set connection "[.conn.iolist.lb get $BitOccupato]  $i"
                .conn.iolist.lb delete $BitOccupato
                .conn.iolist.lb insert $BitOccupato $connection
                set IO_BIT_FREE($BitOccupato) 1
                if {$iosel == $BitOccupato } {
                  set iosel [expr $iosel + 1]
                  .conn.iolist.lb activate $iosel
                }
              }
            }
          } else {
            .conn.varlist.lb insert end $i
          }
        } else {
          .conn.varlist.lb insert end $i
        }
      }
    } 
  }
}

####################################################################
# Procedura ConnectingIoPanel
# Procedura che crea il pannello per la connessione delle variabili
# di IO
# Input: 
#   t       Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ConnectingIoPanel { t } {
global EditPrefs RACK_DEF IO_BIT_FREE MsdData
global VariableList iosel varsel MsdModificata 
global connessione_corrente

  # variabili che mi indicano il bit di i/o selezionato e la variabile selezionata
  set iosel 0
  set varsel 0 
  set tot_io [expr $RACK_DEF(IN) + $RACK_DEF(OUT)]
  for {set i 0 } {$i < $tot_io} {incr i} {
	set IO_BIT_FREE($i) 0
  }

  # lista che tiene traccia delle variabili connesse nella sessione
  set connessione_corrente "" 

  # variabili per le label
  set ioprompt "[GetString ConnectIoPrompt1] $RACK_DEF(LINE) \
         [GetString ConnectIoPrompt2] $RACK_DEF(MOD)"
  set varprompt [GetString ConnectVarPrompt]
  set title [GetString TitleConnection]
    
  set old_focus [focus]
  
  if [winfo exists .conn] {
    return
  }

  set MainW [winfo toplevel [focus]]

  grab release [grab current]

  toplevel .conn
  wm title .conn $title
  wm minsize .conn 10 10
  wm protocol .conn WM_DELETE_WINDOW procx

  frame .conn.varlist
  listbox .conn.varlist.lb -selectbackground black -selectforeground white \
    -yscroll ".conn.varlist.sb set" -width 20 -height 24 -font $EditPrefs(windowfont)
  scrollbar .conn.varlist.sb -command ".conn.varlist.lb yview"
  label .conn.varlist.vartitle -text $varprompt -font $EditPrefs(windowfont) 

  frame .conn.iolist
  listbox .conn.iolist.lb -selectbackground black -selectforeground white \
    -yscroll ".conn.iolist.sb set" -width 20 -height 24 -font $EditPrefs(windowfont)
  scrollbar .conn.iolist.sb -command ".conn.iolist.lb yview"
  label .conn.iolist.iotitle -text $ioprompt -font $EditPrefs(windowfont) 
  
  tixButtonBox .conn.b -orientation horizontal

  .conn.b add ok  -image ok -font $EditPrefs(windowfont) \
     -command { OkConnection }

  .conn.b add connect -image connect -font $EditPrefs(windowfont) \
     -command { ConnectVariable } 

  .conn.b add free -image free -font $EditPrefs(windowfont) \
     -command { FreeVariable }

  .conn.b add freeall -image freeall -font $EditPrefs(windowfont) \
     -command { FreeAllVariable } 

  .conn.b add quit -image cancel -font $EditPrefs(windowfont) \
     -command [format { CancelConnection %s } $t]

  CniBalloon [.conn.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.conn.b subwidget connect] [GetString ConnectConnect] \
    $EditPrefs(windowfont) 500
  CniBalloon [.conn.b subwidget free] [GetString ConnectFree] $EditPrefs(windowfont) 500
  CniBalloon [.conn.b subwidget freeall] [GetString ConnectFreeAll] \
    $EditPrefs(windowfont) 500
  CniBalloon [.conn.b subwidget quit] [GetString CANCEL] \
    $EditPrefs(windowfont) 500

  pack .conn.iolist.iotitle -side top -fill x
  pack .conn.iolist.sb -side $EditPrefs(scrollbarside) -fill y
  pack .conn.iolist.lb -side left -expand yes -fill both
  
  pack .conn.varlist.vartitle -side top -fill x
  pack .conn.varlist.sb -side $EditPrefs(scrollbarside) -fill y
  pack .conn.varlist.lb -side left -expand yes -fill both

  pack .conn.b -side bottom -fill x 
  pack .conn.iolist -side left -expand yes -fill both
  pack .conn.varlist -side left -expand yes -fill both

  wm transient .conn $MainW
  PosizionaFinestre $MainW .conn u

  # inserisco nelle due liste rispettivamente il numero dei moduli
  # e l'elenco delle variabili
  InserisciBitIO .conn.iolist.lb

  FillConnectionList

  focus .conn.iolist.lb

  bind .conn.iolist.lb <ButtonRelease-1> {
    set iosel [.conn.iolist.lb curselection]
  }
  bind .conn.iolist.lb <Return> {
    set iosel [.conn.iolist.lb curselection]
  }
  bind .conn.iolist.lb <Key-Up> {
    set iosel [.conn.iolist.lb curselection]
  }
  bind .conn.iolist.lb  <Key-Down> {
    set iosel [.conn.iolist.lb curselection]
  }

  bind .conn.varlist.lb <ButtonRelease-1> {
    set varsel [.conn.varlist.lb curselection]
  }
  bind .conn.varlist.lb <Return> {
    set varsel [.conn.varlist.lb curselection]
  }
  bind .conn.varlist.lb <Key-Up> {
    set varsel [.conn.varlist.lb curselection]
  }
  bind .conn.varlist.lb  <Key-Down> {
    set varsel [.conn.iolist.lb curselection]
  }

  bind .conn  <Escape> "
    CancelConnection $t
  "
  
  grab .conn
  tkwait window .conn

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }
}

