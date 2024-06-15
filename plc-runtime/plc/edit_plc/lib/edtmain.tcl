
            ######################################
            #                                    #
            #     File di creazione della        #
            #     finestra principale            #
            #                                    #
            ######################################

# GG 06/12/00 Usata la variabile "ProjectConf" per contenere il nome
#             del file di configurazione.

####################################################################
# Procedura InitilizeEditor
# Inizializzazione di tutte le variabili 
# Input: 
#   Nessuno
# Ritorna :
#   Niente
####################################################################
proc InitilizeEditor {} {
  global tk_strictMotif tcl_platform
  global WindowNumber Home UndoPointer CutBuffer 
  global KeywordList MachineDataList 
  global ParenLeftMatch EditPrefs KeyAccelerator InstructionList 
  global PrtOpt BoardLibrary MsdData ContactDef 
  global OptDebugOri OptDebug
  global ProjectConf
  
  set WindowNumber 0
  set UndoPointer 0
  set ParenLeftMatch(parenright) \(
  set ParenLeftMatch(bracketright) \[
  set ParenLeftMatch(braceright) \{
  set CutBuffer {}
  
  if [file exists $ProjectConf] {
    source $ProjectConf
  } else {
    set EditPrefs(awl) 1
    set EditPrefs(binpath) /home/xnc/bin
    set EditPrefs(binpathremote) /home/xnc/bin
    set EditPrefs(confirm) 1
    set EditPrefs(DebugColloquio) 0
    set EditPrefs(DebugStampa) 0
    set EditPrefs(debugcoloracceso) magenta
    set EditPrefs(debugcolorspento) black
    set EditPrefs(defaultlabelsize) 2
    set EditPrefs(defaultwidgetsize) 5
    set EditPrefs(eqnum) 4
    set EditPrefs(fs_diskA) 0
    set EditPrefs(iniziowidget) 35
    set EditPrefs(ioconf) ioconf
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(ladderfont) 5x7
    } else {
      set EditPrefs(ladderfont) -adobe-courier-medium-r-normal--*-140-*-*-*-*-iso8859-*
    }
    set EditPrefs(ladderheight) 800
    set EditPrefs(ladderheight1) 300
    set EditPrefs(ladderwidth) 560
    set EditPrefs(ladderwidth1) 400
    set EditPrefs(libpath) $Home/library
    set EditPrefs(lingua) ita
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(messagefont) -adobe-helvetica-medium-r-normal--*-120-*-*-*-*-iso8859-*
    } else {
      set EditPrefs(messagefont) -adobe-helvetica-medium-r-normal--*-160-*-*-*-*-iso8859-*
    }
    set EditPrefs(nomemodulo) user.oo
    set EditPrefs(nonintrepretaerrori) 0
    set EditPrefs(objpath) /home/xnc/bin/plc-objs
    set EditPrefs(objpathlocale) /home/xnc/bin/plc-objs
    set EditPrefs(objpathremoto) /home/d_unc/plc
    set EditPrefs(parenflash) 1
    set EditPrefs(plc-caricato) plc-caricato
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(plclocale) 1
    } else {
      set EditPrefs(plclocale) 0
    }
    set EditPrefs(plclogin) f_xnc
    set EditPrefs(plcpassword) roberto
    set EditPrefs(plcremoto) 2.2.2.2
    set EditPrefs(plcsocket) 8039
    set EditPrefs(progpath) $Home/prog
    set EditPrefs(progpath1) /home/d_xnc/plc
    set EditPrefs(progpath1locale) /home/d_xnc/plc
    set EditPrefs(progpath1remoto) /home/d_unc/plc
    set EditPrefs(prt_altfoglio) 300.0
    set EditPrefs(prt_copert) 1
    set EditPrefs(prt_crossr) 1
    set EditPrefs(prt_foglio) A4
    set EditPrefs(prt_formdata) g
    set EditPrefs(prt_indice) 1
    set EditPrefs(prt_larfoglio) 200.0
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(prt_logo) $Home/icone/DEFLPRT.png
    } else {
      set EditPrefs(prt_logo) $Home/icone/DEFLPRT.bmp
    }
    set EditPrefs(prt_marasx) 0
    set EditPrefs(prt_marsxdx) 0
    set EditPrefs(prt_note) " "
    set EditPrefs(prt_progil) 1
    set EditPrefs(prt_progld) 0
    set EditPrefs(prt_ragso) " C.N.I. - Controlli Numerici Ind. "
    set EditPrefs(prt_specorn) s
    set EditPrefs(prt_tabiu) 1
    set EditPrefs(qualescambio) scambio
    set EditPrefs(qualescambdin) scambdin
    set EditPrefs(prt_tabiu) 1
    set EditPrefs(scrollbarside) right
    set EditPrefs(symbol) USER_SYM
    set EditPrefs(tablength) 8
    set EditPrefs(textbg) white
    set EditPrefs(textbw) 4
    set EditPrefs(textfg) black
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(textfont) cnifm
    } else {
      set EditPrefs(textfont) -adobe-courier-medium-r-normal--*-140-*-*-*-*-iso8859-*
    }
    set EditPrefs(textheight) 24
    set EditPrefs(textiw) 2
    set EditPrefs(textsb) black
    set EditPrefs(textsbw) 2
    set EditPrefs(textsf) white
    set EditPrefs(textwidth) 70
    set EditPrefs(textwrap) none
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(togglecolor) red
    } else {
      set EditPrefs(togglecolor) white
    }
    set EditPrefs(checkcolor) red
    set EditPrefs(tk_strictMotif) 1
    set EditPrefs(undolevels) 10
    set EditPrefs(visiblebell) 1
    if {$tcl_platform(platform) == "unix"} {
      set EditPrefs(windowfont) -adobe-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*
    } else {
      set EditPrefs(windowfont) -adobe-helvetica-medium-r-normal--*-140-*-*-*-*-iso8859-*
    }
    set EditPrefs(xpmatt) $Home/icone/095.png
  } 

  # sovrascrivo con il contenuto dei registri WINDOWS il nome Host Remoto
  # e il numero del servizio per il collegamento.(Se il registro non esiste
  # rimane settato il contenuto di default).
  if {$tcl_platform(platform) != "unix"} {
    set RegistroBase [QualeRegistro]
    if { $RegistroBase == "errore" } {
      set RegistroBase PDS
    }
    set appoggio [ReadReg $RegistroBase HostName]
    if {$appoggio != "unix" && $appoggio != "errore_registro" } {
      set EditPrefs(plcremoto) $appoggio
    }

    set appoggio [ReadReg $RegistroBase Service]
    if {$appoggio != "unix" && $appoggio != "errore_registro" } {
      set EditPrefs(plcsocket) $appoggio
    }

    set appoggio [ReadReg $RegistroBase SymRemPathName]
    if {$appoggio != "unix" && $appoggio != "errore_registro" } {
      set EditPrefs(progpath1) $appoggio
      set EditPrefs(progpath1remoto) $appoggio
      # Per compatibilita' con il passato
      set EditPrefs(objpath) $appoggio
      set EditPrefs(objpathremoto) $appoggio
    }
    set appoggio [ReadReg $RegistroBase ObjPath]
    if {$appoggio != "unix" && $appoggio != "errore_registro" } {
      set EditPrefs(objpath) $appoggio
      set EditPrefs(objpathremoto) $appoggio
    }
    set appoggio [ReadReg Editor_Plc DebugStampa]
    if {$appoggio != "unix" && $appoggio != "errore_registro" } {
      set EditPrefs(DebugStampa) $appoggio
    }
  }

  set OptDebugOri(plcremoto) 2.2.2.2
  set OptDebugOri(plcsocket) 8039
  set OptDebugOri(progpath1) /home/d_xnc/plc
  set OptDebugOri(progpath1locale) /home/d_xnc/plc
  set OptDebugOri(progpath1remoto) /home/d_unc/plc
  set OptDebugOri(objpath) /home/xnc/bin/plc-objs
  set OptDebugOri(objpathlocale) /home/xnc/bin/plc-objs
  set OptDebugOri(objpathremoto) /home/d_unc/plc

  set OptDebug(plcremoto) $EditPrefs(plcremoto)
  set OptDebug(plcsocket) $EditPrefs(plcsocket)
  set OptDebug(progpath1) $EditPrefs(progpath1)
  set OptDebug(progpath1locale) $EditPrefs(progpath1locale)
  set OptDebug(progpath1remoto) $EditPrefs(progpath1remoto)
  set OptDebug(objpath) $EditPrefs(objpath)
  set OptDebug(objpathlocale) $EditPrefs(objpathlocale)
  set OptDebug(objpathremoto) $EditPrefs(objpathremoto)

  SetRemoteLocalVariable "" "" "" ""


  array set PrtOpt [array get EditPrefs]

  # Elenco istruzioni
  set InstructionList(0) [list LD LDN ]
  set InstructionList(1) [list ST STN ]
  set InstructionList(2) [list S R ]
  set InstructionList(3) [list AND ANDN AND( ANDN( ]
  set InstructionList(4) [list OR ORN OR( ORN( ]
  set InstructionList(5) [list XOR XORN XOR( XORN( ]
  set InstructionList(6) [list NOT ]
  set InstructionList(7) [list ADD ADD( ]
  set InstructionList(8) [list SUB SUB( ]
  set InstructionList(9) [list MUL MUL( ]
  set InstructionList(10) [list DIV DIV( ]
  set InstructionList(11) [list MOD MOD( ]
  set InstructionList(12) [list GT GT( ]
  set InstructionList(13) [list GE GE( ]
  set InstructionList(14) [list EQ EQ( ]
  set InstructionList(15) [list NE NE( ]
  set InstructionList(16) [list LE LE( ]
  set InstructionList(17) [list LT LT( ]
  set InstructionList(18) [list JMP JMPC JMPCN ]
  set InstructionList(19) [list CAL CALC CALCN ]
  set InstructionList(20) [list RET RETC RETCN ]
  set InstructionList(21) [list )]
  set InstructionList(22) [list SHL SHR ROL ROR ]
  set InstructionList(23) [list BITSET BITRES BITTEST BITFIELD ]
  set InstructionList(24) [list ADD_REAL ADD_REAL( ]
  set InstructionList(25) [list SUB_REAL SUB_REAL( ]
  set InstructionList(26) [list MUL_REAL MUL_REAL( ]
  set InstructionList(27) [list DIV_REAL DIV_REAL( ]
  set InstructionList(28) [list GT_REAL GT_REAL( ]
  set InstructionList(29) [list GE_REAL GE_REAL( ]
  set InstructionList(30) [list EQ_REAL EQ_REAL( ]
  set InstructionList(31) [list NE_REAL NE_REAL( ]
  set InstructionList(32) [list LE_REAL LE_REAL( ]
  set InstructionList(33) [list LT_REAL LT_REAL( ]
  set InstructionList(34) [list ABS_REAL ]
  set InstructionList(35) [list DINT_TO_REAL REAL_TO_DINT \
           STRING_TO_DINT STRING_TO_REAL ]
  set InstructionList(36) [list ABS SIN COS TAN ASIN ACOS \
           ATAN SQRT EXP LOG LN TRUNC EXPT ]
  set InstructionList(37) [list RDAX]

  # Inizializzazione dei bind delle menu bar 
  # Menu File
  set KeyAccelerator(CmdLoad) ^l
  set KeyAccelerator(CmdSave) ^s
  set KeyAccelerator(CmdSaveas) ^a
  set KeyAccelerator(CmdNewWindow) ^n
  # Menu Edit
  set KeyAccelerator(CmdCut) ^x
  set KeyAccelerator(CmdCopy) ^c
  set KeyAccelerator(CmdPaste) ^v
  set KeyAccelerator(CmdFind) ^f
  set KeyAccelerator(CmdFindAgain) ^e
  set KeyAccelerator(CmdUndo) ^z
  set KeyAccelerator(CmdRedo) ^y
  set KeyAccelerator(CmdWrapline) ^w
  set KeyAccelerator(DeleteLine) F4 
  # Menu Options
  set KeyAccelerator(DefinedFunctionBlockList) F3
  set KeyAccelerator(DefinedVariableList) F2
  set KeyAccelerator(LoadPlc) F1
  # Menu files della MSD
  set KeyAccelerator(QuitMsd) ^x
  # menu Edit della MSD

  set tk_strictMotif $EditPrefs(tk_strictMotif)

  # preset libreria dei Function Block
  PredefinedFunctionBlockListPreset

  set KeywordList [list PROGRAM END_PROGRAM FUNCTION_BLOCK END_FUNCTION_BLOCK]
  set MachineDataList [list MD_GENERAL MD_USER MD_STATUS MD_AXIS \
     MD_TOOLCHG MD_CONF MD_SET_USER RDAXQ MD_SET_AXIS MD_SAVE_AXIS \
     MD_ORIG MD_SAVE_USER MID LEN ASCII WRAXQ]

  #preset dati che servono alla MSD
  set MsdData(TipoScheda) ios
  set MsdData(offx) 2
  set MsdData(lunvar) 7          ;#n.car.max. var.
  set MsdData(lx) 30             ;#larghezza di ogni slot
  set MsdData(ly) 100            ;#altezza di ogni slot
  set MsdData(numy) 20           ;#altezza sub-box contenente il n.ordine
  set MsdData(maxl) 4            ;#numero massimo di linee
  set MsdData(maxlPerScheda) 2   ;#numero massimo di linee per scheda
  set MsdData(vis) 16            ;#numero di moduli visualizzati nel rack
  set MsdData(filo) 20           ;#lunghezza del filo di I/O (pixel)
  set MsdData(ixmod) 20          ;#coordinata x di inizio canvas modulo (pixel)
  set MsdData(iymod) 2           ;#coordinata y di inizio canvas modulo (pixel)
  set MsdData(lxmod) 180         ;#larghezza rettangolo del modulo (pixel)
  set MsdData(distfili) 14       ;#distanza fra due fili di I/O (pixel)
  set MsdData(iovis) 12          ;#numero di i/o visualizzabili per ogni modulo
  set MsdData(yprn) 200          ;#offset Y disegno modulo quando lo stampo
  
  set MsdData(maxm) 32           ;#numero massimo moduli per linea generico
  set MsdData(maxmios) 32        ;#numero massimo moduli per linea scheda IOS
  set MsdData(maxmudc) 1         ;#numero massimo moduli per linea scheda UDC
  set MsdData(maxmmpp) 1         ;#numero massimo moduli per linea scheda MPP
  set MsdData(SelectedLineios) 0
  set MsdData(SelectedLineudc) 0
  set MsdData(SelectedLinempp) 0
  set MsdData(SelectedModuleios) 0
  set MsdData(SelectedModuleudc) 0
  set MsdData(SelectedModulempp) 0
  set MsdData(LineNumberios) 1
  set MsdData(LineNumberudc) 1
  set MsdData(LineNumbermpp) 1
  
  # preset dati che servono al ladder
  set ContactDef(width_op) 10         ;# larghezza operando
  set ContactDef(width_conn) 40       ;# larghezza connessione
  set ContactDef(height_conn) 45      ;# altezza operando
  set ContactDef(offset_x) 10         ;# offset dal lato sinistro del canvas
  set ContactDef(offset_y) 10         ;# offset dal lato alto del canvas
  set ContactDef(offset_x_text) 20    ;# offset del testo dal lato sinistro
  set ContactDef(offset_y_text) 10    ;# offset del testo dal lato alto
  set ContactDef(offset_coll) 30      ;# offset del collegamento
  set ContactDef(defaultwidgetsize) 6 ;# larghezza widget per il debug
  # Destinati a cadere
  set ContactDef(max_riga) 17
  set ContactDef(max_colonna) 10

  # reset matrice elenco_moduli
  ResetBoardList

  # Carico il file dei moduli IOS
  if {[file exists $Home/util/moduli]} {
    source $Home/util/moduli
  }
}

####################################################################
# Procedura SetRemoteLocalVariable
# Con tale funzione si commutano i percorsi per il locale ed il
# remoto
# Input: 
#   t           Nome del widget di testo
#   Macchina    Nome del widget per il nome macchina remota
#   Socket      Nome del widget per il numero del socket
#   WdgAfter    Nome del widget dopo il quale fare il pack
# Ritorna :
#   Niente
####################################################################
proc SetRemoteLocalVariable { t Macchina Socket WdgAfter} {
global EditPrefs OptDebug

  if { $EditPrefs(plclocale) == 1 } {
    if { $Macchina != "" } {
      pack forget $Macchina
      pack forget $Socket
      set OptDebug(objpathremoto) $OptDebug(objpath)
      set OptDebug(progpath1remoto) $OptDebug(progpath1)
    }
    set OptDebug(objpath) $OptDebug(objpathlocale)
    set OptDebug(progpath1) $OptDebug(progpath1locale)
    if { $t != "" } {
      [TextToTop $t].remote config -text "[GetString PlcLocaleMin]"
    }
  } elseif { $EditPrefs(plclocale) == 0 } {
    if { $Macchina != "" } {
      pack $Macchina -after $WdgAfter -pady 5
      pack $Socket -after $WdgAfter -pady 5
      set OptDebug(objpathlocale) $OptDebug(objpath)
      set OptDebug(progpath1locale) $OptDebug(progpath1)
    }
    set OptDebug(objpath) $OptDebug(objpathremoto)
    set OptDebug(progpath1) $OptDebug(progpath1remoto)
    if { $t != "" } {
      [TextToTop $t].remote config -text "[GetString PlcRemotoMin] \
        : \t$EditPrefs(plcremoto)"
    }
  }
}

####################################################################
# Procedura CreateWindow
# Con tale funzione si apre una nuova finestra di editing
# Input: 
#   args   Argomenti a coppie es. -tipo_win main
# Ritorna :
#   Il nome della toplevel creata
####################################################################
proc CreateWindow { args } {
  global WindowType WindowNumber DebugNoWidget iconic
  
  ParseArgs {
    {window unspecified}
    {file {}}
    {tipo_win main}
  }
  
  # pick a window name if the user hasn't supplied one
  if { "x$window" == "xunspecified" } {
    set window [NewWindowName]
  }
  
  if { ! [winfo exists $window] } {
    toplevel $window
    if {[string first 0 $window] != -1 && $iconic == "yes"} {
      wm iconify $window
    }

    wm protocol $window WM_DELETE_WINDOW procx
    # Resetto la variabile che contiene i widget da nascondere 
    # durante la fase di debug
    if [info exist DebugNoWidget] {
      foreach list_element [lrange $DebugNoWidget 0 end] {
        if { [string first $window $list_element] == -1 } {
          set tmpDebugNoWidget [lappend tmpDebugNoWidget $list_element]
        } 
      }
    unset DebugNoWidget
    set DebugNoWidget $tmpDebugNoWidget
    }
  }
  
  # l'array WindowType contiene il tipo di ogni toplevel generata 
  # per ora main oppure fb
  set WindowType($window) $tipo_win

  incr WindowNumber		;# keep count of each window opened
  
  set text [TopToText $window]
  
  if {"x$file" != "x"} {
    SetFilename $window $file
  }

  MainWindowCreate $window
  DefineBind $window

  return $window
}

####################################################################
# Procedura NewWindowName
# Con tale funzione si ricerca un nome non usato per una
# nuova finestra
# Input: 
#   Nessuno
# Ritorna :
#   Il nome della finestra
####################################################################
proc NewWindowName {} {
  set i 0
  while {[winfo exists .jedit$i]} {
    incr i
  }
  return .jedit$i
}

####################################################################
# Procedura SetFilename
# Memorizzo in un vettore il nome del file caricato per
# ogni finestra
# Input: 
#   w          Nome del widget di testo
#   filename   Nome del file caricato
# Ritorna :
#   Niente
####################################################################
proc SetFilename { w filename } {
  global EditFiles WindowType VersioneEditor
  
  set window [TextToTop $w]

  set EditFiles($window) $filename

  set FileLabel $window.main.status.name
  
  if {$WindowType($window)=="fb"} {
    set filename [string range $filename 0 [expr [string length $filename] -2]]
  }

  if {"x$filename" == "x"} {
    set filename "[GetString NoFile] $VersioneEditor"
  }
  
  set title [file tail $filename]
  
  if {$WindowType($window) == "main"} {
    set key "PROGRAM"
  } else {
    set key "FUNCTION BLOCK"
  } 
  if [winfo exists $FileLabel] {
    $FileLabel configure -text "$key : $title"
  }
  
  if {[winfo class $window] == "Toplevel"} {
    wm iconname $window $title
  }
}

####################################################################
# Procedura GetFilename
# Prelevo da un vettore il nome del file caricato per
# ogni finestra. La finestra puo' essere specificata o come
# widget di testo o come toplevel
# Input: 
#   w          Nome del widget di testo
# Ritorna :
#   filename   Nome del file caricato
####################################################################
proc GetFilename { w } {
  global EditFiles
  
  set window [TextToTop $w]
  
  if [info exists EditFiles($window)] {
    return $EditFiles($window)
  } else {
    return {}
  }
}

####################################################################
# Procedura MainWindowCreate
# Creazione della finestra principale
# Input: 
#   window    Nome della toplevel
# Ritorna :
#   Niente
####################################################################
proc MainWindowCreate { window } {
global VersioneEditor

  set prefix $window
  
  set menu $prefix.menu
  set main $prefix.main

  set text [TopToText $window]
  set menubar [TopToMenubar $window]
  set buttonbar [TopToButtonbar $window]
  set accessoria $window.accessoria
  
  frame $menu -borderwidth 2 -relief raised
  pack $menu -fill x
  
  MainCreate $main
  pack $main -expand yes -fill both
  
  if [winfo exists $accessoria] {
    destroy $accessoria
  }
  if [winfo exists $buttonbar] {
    destroy $buttonbar
  }

  MenusCreate $menubar $text IL
  frame $buttonbar
  ButtonBarCreate $buttonbar $text
  pack $buttonbar.b [SeparationLine $buttonbar] -in $buttonbar -side top -fill x
  pack $buttonbar -after $menubar -fill x

  VerticalButtonBarCreate $accessoria $text
  pack $accessoria -side right -after $buttonbar -anchor se

  if {[winfo class $window] == "Toplevel"} {
    set NumeroFinestra [string range $window end end]
    if { $NumeroFinestra == 0 } {
      wm title $window "[GetString NoFile] $VersioneEditor"
      wm iconname $window "[GetString NoFile] $VersioneEditor"
    } else {
      wm title $window "[GetString NoFile] $VersioneEditor   N.$NumeroFinestra"
      wm iconname $window "[GetString NoFile] $VersioneEditor   N.$NumeroFinestra"
    }
  }
}

####################################################################
# Procedura ScrollSet
# Procedura di gestione della scrollbar orizzontale
# Input: 
#   scrollbar    Nome della scrollbar
#   geoCmd       Comando di pack
#   offset       Offest dello scroll
#   size         Dimensione ?
# Ritorna :
#   Niente
####################################################################
proc ScrollSet { scrollbar geoCmd offset size } {
global EditPrefs

  if { $EditPrefs(awl)==1 } {
    if { $offset != 0.0 || $size != 1.0 } {
      eval $geoCmd
      $scrollbar set $offset $size
    } else {
      set manager [lindex $geoCmd 0]
      $manager forget $scrollbar
    }
  }
}

####################################################################
# Procedura MainCreate
# Creazione delle etichette e del widget di testo e canvas
# Input: 
#   window    Nome della toplevel
# Ritorna :
#   Niente
####################################################################
proc MainCreate { w } {
  global EditPrefs

  if {[lsearch [array names EditPrefs] {scrollbarside}] == -1} {
    set EditPrefs(scrollbarside) right 
  }

  frame $w
  
  frame $w.status
  label $w.status.name -relief flat -text [GetString NoFileYet] \
      -font -adobe-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-* 

  label $w.status.position -relief flat \
      -text "[GetString Riga] 1\t[GetString Colonna] 1\t\t" \
      -font -adobe-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*
  label $w.status.mode -relief flat -text plc \
      -font -adobe-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-* 
  if [ info exists EditPrefs(windowfont) ] {
    $w.status.name configure -font $EditPrefs(windowfont)
    $w.status.position configure -font $EditPrefs(windowfont)
    $w.status.mode configure -font $EditPrefs(windowfont)
  }

  #text $w.t -yscroll "$w.s set" -xscroll [list ScrollSet [winfo parent $w].h \
  #           [list pack [winfo parent $w].h -side bottom -expand yes -fill x]]
  #text $w.t -yscroll "$w.s set" -xscroll "[winfo parent $w].h set"  
  text $w.t -yscroll [list $w.s set] -xscroll [list [winfo parent $w].h set]  
  catch {$w.t configure \
    -background $EditPrefs(textbg) \
    -foreground $EditPrefs(textfg) \
    -insertbackground $EditPrefs(textfg) \
    -selectbackground $EditPrefs(textsb) \
    -selectforeground $EditPrefs(textsf) \
    -selectborderwidth $EditPrefs(textsbw) \
    -borderwidth $EditPrefs(textbw) \
    -insertwidth $EditPrefs(textiw) \
    -width $EditPrefs(textwidth) \
    -height $EditPrefs(textheight) \
    -wrap $EditPrefs(textwrap)
  }
  $w.t configure -font $EditPrefs(textfont)

  bind $w.t <Button-3> {
    CreaDescrizione %W %x %y
  }
  
  scrollbar $w.s -command [list $w.t yview] 
  scrollbar [winfo parent $w].h -command [list $w.t xview] -orient horizontal 

  bind $w.s <B1-Motion> [format { 
               global DebugState 
               if { $DebugState == 1 } {
                 ProtocolloAttivaDebug %s IL
               }
    } $w.t]

  bind $w.s <Button-1> [format { 
               global DebugState 
               if { $DebugState == 1 } {
                 ProtocolloAttivaDebug %s IL
               }
    } $w.t]

  # genero il canvas che conterra' i contatti
  canvas $w.ladd \
    -width $EditPrefs(ladderwidth1) \
    -height $EditPrefs(ladderheight1) \
    -xscrollcommand "$w.xscroll set" \
    -scrollregion "0 0 $EditPrefs(ladderwidth) $EditPrefs(ladderheight)" \
    -background white 
  scrollbar $w.xscroll -orient horizontal \
    -command "$w.ladd xview"

  pack $w.status.name -in $w.status -side left -fill x
  pack $w.status.mode -in $w.status -side right -fill x
  pack $w.status.position -in $w.status -side right -fill x
  pack $w.status -in $w -side top -fill x 

  pack $w.s -in $w -side $EditPrefs(scrollbarside) -fill y
  pack $w.t -in $w -side $EditPrefs(scrollbarside) -expand yes -fill both

  label [winfo parent $w].remote -relief sunken -bd 1 -font $EditPrefs(windowfont)
  pack [winfo parent $w].remote -side bottom -anchor w -fill both -padx 2 -pady 1

  pack [winfo parent $w].h -side bottom -fill x
  SetRemoteLocalVariable $w.t "" "" ""

  focus $w.t
  catch {focus default $w.t}
  
  return $w
}

####################################################################
# Procedura ButtonBarCreate
# Creazione della button Bar
# Input: 
#   window    Nome della toplevel
#   t         Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ButtonBarCreate { w t } {
  global EditPrefs LadderEditShow
  
  if [winfo exists $w] {
    destroy $w
  }
  
  frame $w
  
  if { $LadderEditShow==0 } {
    IlPaletteCreate $w $t
  } else {
    LadderPaletteCreate $w [winfo parent $w].ladd 
  }

  pack $w.b [SeparationLine $w] -in $w -side top -fill x

  return $w
}

####################################################################
# Procedura TextToTop
# Procedura che, dato il widget di testo risale alla toplevel
# Input: 
#   Widget di testo
# Ritorna :
#   Toplevel
####################################################################
proc TextToTop { t } {
  if { [winfo class $t] == "Text" } {
    return [winfo parent [winfo parent $t]]
  } else {
    return $t
  }
}

####################################################################
# Procedura MenuToTop
# Procedura che, dato il widget della menu bar, risale alla toplevel
# Ad. es .jedit0.menu.user.m -> .jedit0
# Input: 
#   Widget di testo
# Ritorna :
#   Toplevel
####################################################################
proc MenuToTop { mb } {
  return [winfo parent [winfo parent [winfo parent $mb]]]
}

####################################################################
# Procedura TopToText
# Procedura che ritorna, data la toplevel, il percorso
# del widget di testo
# Ad es .jedit -> .jedit.main.t, but . -> .main.t
# Input: 
#   top   Il nome della toplevel
# Ritorna :
#   Il nome del widget di testo
####################################################################
proc TopToText { top } {
  if {[string compare "." $top] == 0} {
    set prefix ""
  } else {
    set prefix $top
  }
  return ${prefix}.main.t
}

####################################################################
# Procedura TopToButtonbar
# Procedura che ritorna, data la toplevel, il percorso
# del widget della buttonbar
# Ad es .jedit -> .jedit.buttons, but . -> .buttons
# Input: 
#   top   Il nome della toplevel
# Ritorna :
#   Il nome del widget di testo
####################################################################
proc TopToButtonbar { top } {
  if {[string compare "." $top] == 0} {
    set prefix ""
  } else {
    set prefix $top
  }
  return ${prefix}.buttons
}

####################################################################
# Procedura TopToMenubar
# Procedura che ritorna, data la toplevel, il percorso
# del widget della menubar
# Ad es .jedit -> .jedit.menu, but . -> .menu
# Input: 
#   top   Il nome della toplevel
# Ritorna :
#   Il nome del widget di testo
####################################################################
proc TopToMenubar { top } {
  if {[string compare "." $top] == 0} {
    set prefix ""
  } else {
    set prefix $top
  }
  return ${prefix}.menu
}
