
            ######################################
            #                                    #
            #     File contenente le procedure   #
            #     per la compilazione e la       #
            #     interpretazione del file       #
            #                                    #
            ######################################

####################################################################
# Procedura ReverseLineaErrata
# Procedura che mette in reverse la linea che contiene l'errore
# Input: 
#   t           Nome del widget di testo
#   linea       linea del testo errata
#   t_err       Nome del widget di testo con gli errori
#   linea_t_err linea del testo con gli errori
# Ritorna :
#   Niente
####################################################################
proc ReverseLineaErrata { t linea t_err linea_t_err} {

  set range_linea_err [$t tag ranges linea_errata]
  if { [lindex $range_linea_err 0 ] != {} } {
    $t tag remove linea_errata [lindex $range_linea_err 0 ] [lindex $range_linea_err 1 ]
    set range_linea_t_err [$t_err tag ranges linea_t_errata]
    $t_err tag remove linea_t_errata [lindex $range_linea_t_err 0 ] \
      [lindex $range_linea_t_err 1 ]
  }
  if { $linea != -1 } {
    $t tag add linea_errata $linea.0 $linea.end 
    $t tag configure linea_errata -foreground white -background black	
    $t_err tag add linea_t_errata [expr $linea_t_err*2 +1].0 [expr $linea_t_err*2 +1].end 
    $t_err tag configure linea_t_errata -foreground white -background black	
  }
}

####################################################################
# Procedura AttivaFunctionBlock
# Procedura carica il function block con l'errore
# Input: 
#   PadreTesto    Nome del widget del padre del testo
#   Linea_err     linea del testo errata
#   ErrTesto      Nome del widget di testo con gli errori
#   LineaErrTesto linea del testo con gli errori
#   Function_name Nome del blocco funzionale
#   Function_line Linea del blocco funzionale 
# Ritorna :
#   Niente
####################################################################
proc AttivaFunctionBlock { PadreTesto Linea_err ErrTesto LineaErrTesto Function_name \
                           Function_line } {
global EditFiles MainOfFunctionBlock EditPrefs

  if [info exist EditFiles] {
    set indice_FB [lsearch -regexp [array get EditFiles] [lindex $Function_name 2]]
    if { $indice_FB != -1 } {
      set t [TopToText [lindex [array get EditFiles] [expr $indice_FB - 1]]]
      GoToLine $t [expr $Linea_err - $Function_line]
      ReverseLineaErrata $t [expr $Linea_err - $Function_line] $ErrTesto $LineaErrTesto
    } else {
      set w [CreateWindow -tipo_win fb]
      # toplevel main che genera questo FB
      set MainOfFunctionBlock($w) [TextToTop $PadreTesto]
      set t [TopToText $w]
      SwapWindowsVariable $w
      set RootPath [file dirname [GetFilename $PadreTesto]]
      LoadProgram "$RootPath/[CompletaNomeBlocco [lindex $Function_name 2] \
          $MainOfFunctionBlock($w)]" $t 1
      InizializzaFB $t 1
      # Evidenzio le linee incriminate
      GoToLine $t [expr $Linea_err - $Function_line]
      ReverseLineaErrata $t [expr $Linea_err - $Function_line] $ErrTesto $LineaErrTesto
    }
  } else { 
    set w [CreateWindow -tipo_win fb]
    # toplevel main che genera questo FB
    set MainOfFunctionBlock($w) [TextToTop $PadreTesto]
    set t [TopToText $w]
    SwapWindowsVariable $w
    set RootPath [file dirname [GetFilename $PadreTesto]]
    LoadProgram "$RootPath/[CompletaNomeBlocco [lindex $Function_name 2] \
          $MainOfFunctionBlock($w)]" $t 1
    InizializzaFB $t 1
    # Evidenzio le linee incriminate
    GoToLine $t [expr $Linea_err - $Function_line]
    ReverseLineaErrata $t [expr $Linea_err - $Function_line] $ErrTesto $LineaErrTesto
  }
}

####################################################################
# Procedura VisualizzazioneErroriCompilazione
# Creazione del widget di testo  che visualizza gli errori
# Input: 
#   passo_comp    Flag che se messo a 0 non interpreta gli errori
#                 dati dal compilatore
#   t             Nome del widget di testo
#   Risultato     Risultato della compilazione
# Ritorna :
#   Niente
####################################################################
proc VisualizzazioneErroriCompilazione { passo_comp t Risultato} {
global OffsetLineaProg EditPrefs 

  if {[info exist OffsetLineaProg] == 0 } {
    set OffsetLineaProg 0
  }
  [winfo toplevel $t].main.status.mode configure -text plc
  update idletasks

  if {$Risultato != ""} {
    # Creazione del widget di testo che contiene gli errori
    if [winfo exists .errPLC] {
      raise .errPLC
      focus .errPLC
      return
    }
    set errWin .errPLC
    toplevel $errWin
    wm title $errWin [GetString ErrorPLCprogram]
    wm protocol $errWin WM_DELETE_WINDOW procx
    if { $passo_comp == 0 } { 
      button $errWin.ok -image ok -command "
                                         ReverseLineaErrata $t -1 $errWin.text -1
					 destroy $errWin"
    } else {
      button $errWin.ok -image ok -command "destroy $errWin"
    }
										 
    CniBalloon $errWin.ok [GetString OK] $EditPrefs(windowfont) 500
    text $errWin.text -relief sunken -bd 2 -yscrollcommand "$errWin.scroll set" \
          -setgrid true -width 50 -height 20
    scrollbar $errWin.scroll -relief sunken -command "$errWin.text yview"
    pack $errWin.ok -side bottom -padx 3m -pady 2m
    pack $errWin.scroll -side right -fill y
    pack $errWin.text -side left -expand yes -fill both

    bind $errWin <Control-d> "set EditPrefs(nonintrepretaerrori) 1"
    bind $errWin <Control-q> "set EditPrefs(nonintrepretaerrori) 0"

    if { $passo_comp == 0 } { 
      if {[winfo depth $errWin] > 1} {
        set bold "-background white -relief raised -borderwidth 1"
        set normal "-background {} -relief flat"
      } else {
        set bold "-foreground white -background black"
        set normal "-foreground {} -background {}"
      }

      set i 0
      foreach linea [split $Risultato \n] {
        if {$linea != {} } {
         if {[lindex $linea 0] == "Internal" } {
           # teniamo conto degli errori anomali ritornati dal compilatore
           $errWin.text insert insert $linea
         } elseif {[string first "home" [lindex $linea 0]] == -1  } {
           $errWin.text insert insert $linea
         } else {
    	   set linea_err [string range [lindex $linea 0] \
                         [expr [string last ":" [lindex $linea 0]]+1] end]
           # Controllo se l'errore e' interno ad un FB
           set offset_FB [CalcoloOffsetFB $linea_err]
           if {[lindex $offset_FB 1] == 0} {
             set LineaErrore [expr $linea_err - $OffsetLineaProg]
             if { $LineaErrore > 0 } {
               set da_stampare \
                 "Main: Line $LineaErrore [lrange $linea 1 end]\n\n"
               $errWin.text insert insert $da_stampare d$i
               $errWin.text tag bind d$i <1> "
                 GoToLine $t $LineaErrore
	         ReverseLineaErrata $t $LineaErrore $errWin.text $i"
             } else {
               set nomedelfileconpath [GetFilename $t]
               set fId [open $nomedelfileconpath r]
               set NumLinea 1
               while { [gets $fId line] >= 0 } {
                 if { $NumLinea == $linea_err } {
                   $errWin.text insert insert "$line \n"
                   $errWin.text insert insert "$linea \n"
                   break
                 }
                 incr NumLinea
               }
               close $fId
             }
           } else {
             set LineaErrore [expr $linea_err - [lindex $offset_FB 1]]
             if { $LineaErrore > 0 } {
               set da_stampare "
                 [lindex $offset_FB 0]: Line $LineaErrore [lrange $linea 1 end]\n\n"
               $errWin.text insert insert $da_stampare d$i
               $errWin.text tag bind d$i <1> "
                 AttivaFunctionBlock $t $linea_err $errWin.text $i $offset_FB"
             } else {
               set nomedelfileconpath [GetFilename $t]
               set nomedelfileconpath [file root $nomedelfileconpath].plc
               set fId [open $nomedelfileconpath r]
               set NumLinea 1
               while { [gets $fId line] >= 0 } {
                 if { $NumLinea == $linea_err } {
                   $errWin.text insert insert "Errore in Blocco Funzionale\n"
                   $errWin.text insert insert "$line \n"
                   $errWin.text insert insert "$linea \n"
                   break
                 }
                 incr NumLinea
               }
               close $fId
             }
           }

           $errWin.text tag bind d$i <Any-Enter> "$errWin.text tag configure d$i $bold"
           $errWin.text tag bind d$i <Any-Leave> "$errWin.text tag configure d$i $normal"
           set i [expr $i +1]
         }
       }
     }
   } else {
      foreach linea [split $Risultato \n] {
	$errWin.text insert insert "$linea\n"
      }
    } 
  }
}

####################################################################
# Procedura StartCompileProgram
# Comando che lancia la compilazione con flag a 1
# Input: 
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc StartCompileProgram { t } {
 
  CompileProgram $t 1
}

####################################################################
# Procedura VisualizzazioneErroriCompilazione
# Comando che lancia la compilazione 
# Input: 
#   t             Nome del widget di testo
#   NoMess        Flag ?????
# Ritorna :
#   Niente
####################################################################
proc CompileProgram { t NoMess} {
global Home EditPrefs ModiFile input2 lista_librerie FunctionBlockList
global tcl_platform QualeScambio QualeScambdin

  set filename [GetFilename $t]
  if { $filename == "" } {
    AlertBox -text MsgNoFileLoad
    return 1
  } 
  if {[file extension $filename] == ""} {
    set filename $filename.plc
  }

  set w [TextToTop $t]

  if { $ModiFile==1 } {
    AlertBox -text MsgFileChanged
    return 1
  }

  # Lancio del compilatore
  WaitWindow $t 1

  DebugColloquio

  ScriviDebugColloquio $QualeScambio
  ScriviDebugColloquio $QualeScambdin

  set segnali $QualeScambio
  set function_block_code $Home/util/illib.ilc
  set prog_msd [file root $filename].msd
  set prog_crc [file root $filename].crc

  # Caso della libreria
  if {[string first $EditPrefs(libpath) $filename] != -1 } {
    [winfo toplevel $t].main.status.mode configure -text [GetString Compiling]
    set libraryName [file root $filename]
    update idletasks
    set ScambdinFile $QualeScambdin
    ScriviDebugColloquio "$Home/ilc -il -g -w 0 -header $segnali $function_block_code \
           $ScambdinFile -code $filename -os $filename.s"
    catch {exec $Home/ilc -il -g -w 0 -header $segnali $function_block_code \
           $ScambdinFile -code $filename -os $filename.s} Risultato
    if {$Risultato == ""} { 
      [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
      update idletasks
      if {$tcl_platform(platform) == "unix"} {
        ScriviDebugColloquio "as $filename.s -o $libraryName.o"
        catch {exec as $filename.s -o $libraryName.o} Risultato 
      } else {
        ScriviDebugColloquio "$Home/as $filename.s -o $libraryName.o"
        catch {exec $Home/as $filename.s -o $libraryName.o} Risultato 
      }
    } else {
      [winfo toplevel $t].main.status.mode configure -text plc
      update idletasks
      VisualizzazioneErroriCompilazione $EditPrefs(nonintrepretaerrori) $t $Risultato
      WaitWindow $t 0
      return 1
    }
    if {$Risultato == ""} {
      [winfo toplevel $t].main.status.mode configure -text plc
      update idletasks
      if {$NoMess == 1 } {
        AlertBox -text MsgNoError
      }
      file delete $filename.s
    }
    WaitWindow $t 0
    return
  }

  if [catch { set crcId [open $prog_crc w] } result] {
    AlertBox -text $result
    return
  }
  # inizializzo la varibile _CRC con il CRC di *.plc *.msd
  set progCRC [CalcolaCRC $filename]
  set msdCRC [CalcolaCRC [file root $filename].msd]
  puts $crcId "VAR_GLOBAL"
  puts $crcId "\t_CRC : UDINT:= [expr $progCRC + $msdCRC];"
  puts $crcId "END_VAR"
  close $crcId

  [winfo toplevel $t].main.status.mode configure -text [GetString Compiling]
  update idletasks

  # Ricavo le librerie necessarie
  set elenco_librerie [array names lista_librerie]
  set lib_header ""
  set lib_obj ""
  foreach nomiFB [array names FunctionBlockList] {
    set TipoFunctionBlock [lindex $FunctionBlockList($nomiFB) 0]
    if { [lsearch -exact $elenco_librerie $TipoFunctionBlock] != -1 } {
      if { [lsearch -exact $lib_header $lista_librerie($TipoFunctionBlock)] == -1 } {
        set lib_header [lappend lib_header $lista_librerie($TipoFunctionBlock)]
        set NomeLibreria [file tail $lista_librerie($TipoFunctionBlock)]
        set NomeLibreria $Home/library/[file root $NomeLibreria].o
        set lib_obj [lappend lib_obj $NomeLibreria]
      }
    }
  }

  if [RicreaLibrerie $lib_obj] {
    return
  }

  if { $input2 != -1 } {
    set ScambdinFile $QualeScambdin
    set comando "catch {exec $Home/ilc -il -g -w 0 \
       -header $segnali $function_block_code $lib_header -code $filename \
       -os $filename.s $prog_msd -os $prog_msd.s $prog_crc \
       -os $prog_crc.s $ScambdinFile -os $EditPrefs(progpath)/scambdin.s} Risultato"
    ScriviDebugColloquio $comando
    eval $comando
  } else {
    set comando "catch {exec $Home/ilc -il -g -w 0 \
       -header $segnali $function_block_code $lib_header -code $filename \
       -os $filename.s $prog_msd -os $prog_msd.s $prog_crc -os $prog_crc.s} Risultato"
    ScriviDebugColloquio $comando
    eval $comando
  }

  if {$Risultato == ""} { 
    [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
    update idletasks
    if { $tcl_platform(platform) == "unix" } {
      ScriviDebugColloquio "as $filename.s -o $filename.o"
      catch {exec as $filename.s -o $filename.o} Risultato 
    } else {
      ScriviDebugColloquio "$Home/as $filename.s -o $filename.o"
      catch {exec $Home/as $filename.s -o $filename.o} Risultato 
    }
  } else {
    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks
    VisualizzazioneErroriCompilazione $EditPrefs(nonintrepretaerrori) $t $Risultato
    WaitWindow $t 0
    return 1
  }

  if {$Risultato == ""} { 
    [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
    update idletasks
    if { $tcl_platform(platform) == "unix" } {
      ScriviDebugColloquio "as $prog_msd.s -o $prog_msd.o"
      catch {exec as $prog_msd.s -o $prog_msd.o} Risultato 
    } else {
      ScriviDebugColloquio "$Home/as $prog_msd.s -o $prog_msd.o"
      catch {exec $Home/as $prog_msd.s -o $prog_msd.o} Risultato 
    }
  } else {
    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks
    VisualizzazioneErroriCompilazione 1 $t $Risultato
    WaitWindow $t 0
    return 1
  }
  
  if {$Risultato == ""} { 
    [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
    update idletasks
    if { $tcl_platform(platform) == "unix" } {
      ScriviDebugColloquio "as $prog_crc.s -o $prog_crc.o"
      catch {exec as $prog_crc.s -o $prog_crc.o} Risultato 
    } else {
      ScriviDebugColloquio "$Home/as $prog_crc.s -o $prog_crc.o"
      catch {exec $Home/as $prog_crc.s -o $prog_crc.o} Risultato 
    }
  } else {
    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks
    VisualizzazioneErroriCompilazione 1 $t $Risultato
    WaitWindow $t 0
    return 1
  }

  if { $input2 != -1 } {
    if {$Risultato == ""} {
      [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
      update idletasks
      if { $tcl_platform(platform) == "unix" } {
        ScriviDebugColloquio "as $EditPrefs(progpath)/scambdin.s \
              -o $EditPrefs(progpath)/scambdin.o"
        catch {exec as $EditPrefs(progpath)/scambdin.s \
              -o $EditPrefs(progpath)/scambdin.o} Risultato 
      } else {
        ScriviDebugColloquio "$Home/as $EditPrefs(progpath)/scambdin.s \
              -o $EditPrefs(progpath)/scambdin.o"
        catch {exec $Home/as $EditPrefs(progpath)/scambdin.s \
              -o $EditPrefs(progpath)/scambdin.o} Risultato 
      }
    } else {
      [winfo toplevel $t].main.status.mode configure -text plc
      update idletasks
      VisualizzazioneErroriCompilazione 1 $t $Risultato
      WaitWindow $t 0
      return 1
    }
  }

  if {$Risultato == ""} { 
    [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
    update idletasks
    if { $input2 != -1 } {
      if { $tcl_platform(platform) == "unix" } {
        set comando "catch {exec ld -r $Home/ilc0.o $prog_crc.o \
          $EditPrefs(progpath)/scambdin.o $lib_obj $filename.o $prog_msd.o \
          $Home/ilcn.o $Home/libil.a -o $EditPrefs(progpath)/user.o} Risultato"
      } else {
        set comando "catch {exec $Home/ld -r $Home/ilc0.o $prog_crc.o \
          $EditPrefs(progpath)/scambdin.o $lib_obj $filename.o $prog_msd.o \
          $Home/ilcn.o $Home/libil.a -o $EditPrefs(progpath)/user.o} Risultato"
      }
      ScriviDebugColloquio $comando
      eval $comando
    } else {
      if { $tcl_platform(platform) == "unix" } {
        set comando "catch {exec ld -r $Home/ilc0.o $prog_crc.o $lib_obj $filename.o \
         $prog_msd.o $Home/ilcn.o $Home/libil.a -o $EditPrefs(progpath)/user.o} Risultato"
      } else {
        set comando "catch {exec $Home/ld -r $Home/ilc0.o $prog_crc.o $lib_obj \
         $filename.o $prog_msd.o $Home/ilcn.o $Home/libil.a \
         -o $EditPrefs(progpath)/user.o} Risultato"
      }
      ScriviDebugColloquio $comando
      eval $comando
    }
  } else {
    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks
    VisualizzazioneErroriCompilazione 1 $t $Risultato
    WaitWindow $t 0
    return 1
  }

  if {$Risultato == ""} { 
    file delete $prog_crc
    file delete $prog_crc.s
    file delete $prog_crc.o

    if { $input2 != -1 } {
      file delete $EditPrefs(progpath)/scambdin.s
      file delete $EditPrefs(progpath)/scambdin.o
    } 

    file delete $filename.s
    file delete $prog_msd.s
    file delete $filename.o
    file delete $prog_msd.o

    [winfo toplevel $t].main.status.mode configure -text [GetString Assembling]
    update idletasks

    ScriviDebugColloquio "$Home/c2a $EditPrefs(progpath)/user.o \
             $EditPrefs(progpath)/$EditPrefs(nomemodulo)"
    catch {exec $Home/c2a $EditPrefs(progpath)/user.o \
             $EditPrefs(progpath)/$EditPrefs(nomemodulo)} Risultato 
    if { $Risultato != "" } {
      [winfo toplevel $t].main.status.mode configure -text plc
      update idletasks
      VisualizzazioneErroriCompilazione 1 $t $Risultato
      WaitWindow $t 0
      return 1
    }
    if {$NoMess == 1 } {
      AlertBox -text MsgNoError
    }

    file delete $EditPrefs(progpath)/user.o
    if {$EditPrefs(plclocale) || ($EditPrefs(plclocale) == "0" && \
        $EditPrefs(plcremoto) == "localhost")} {
      file copy -force $EditPrefs(progpath)/$EditPrefs(nomemodulo) \
         $EditPrefs(objpath)/[file root [file tail $filename]].oo 
      file delete $EditPrefs(progpath)/$EditPrefs(nomemodulo)
    }

    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks

  } else {
    [winfo toplevel $t].main.status.mode configure -text plc
    update idletasks
    VisualizzazioneErroriCompilazione 1 $t $Risultato
    WaitWindow $t 0
    return 1
  }
}

####################################################################
# Procedura VerificaNoOpConst
# Procedura che verifica il caso di non operando e costante di tempo.
# Input: 
#   operator      Operatore
#   variable_name Nome variabile
#   Message       Messaggio da inviare in caso di errore
# Ritorna :
#   0 nessun caso, 1 costante di tempo, 2 manca operando
####################################################################
proc VerificaNoOpConst { operator variable_name Message} {

  # Se la seconda parola inizia con t# o #time e' la definizione di un tempo costante
  # se inizia con d# o #date e' una data
  if {[string match {T\#*} $variable_name] || \
          [string match {TIME\#*} $variable_name] || \
          [string match {D\#*} $variable_name] || \
          [string match {DATE\#*} $variable_name] || \
          [string match {%MX*} $variable_name] || \
          [string match {%MB*} $variable_name] || \
          [string match {%MW*} $variable_name] || \
          [string match {%IX*} $variable_name] || \
          [string match {%IB*} $variable_name] || \
          [string match {%IW*} $variable_name] || \
          [string match {%QX*} $variable_name] || \
          [string match {%QB*} $variable_name] || \
          [string match {%QW*} $variable_name] } {
    return 1
  }

  # Verifico se non rientra nel caso delle due costanti
  if {$variable_name == "TRUE" || \
      $variable_name == "FALSE" } {
    return 1
  }

  # Se la seconda parola non esiste affatto e prima c'era una istruzione invio 
  # un messaggio di errore
  if {$variable_name == "" || $variable_name == "(*"} {
    if {[string match *: $operator] == 0 && \
            $operator != "NOT" && \
            $operator != "RET" && \
            $operator != "RETC" && \
            $operator != "RETCN" && \
            $operator != "MD_STATUS" && \
            $operator != "MD_SAVE_AXIS" && \
            $operator != "MD_SAVE_USER" && \
            $operator != "DINT_TO_REAL" && \
            $operator != "REAL_TO_DINT" && \
            $operator != "STRING_TO_DINT" && \
            $operator != "STRING_TO_REAL" && \
            $operator != "ABS" && \
            $operator != "ABS_REAL" && \
            $operator != "TRUNC" && \
            $operator != "SQRT" && \
            $operator != "SIN" && \
            $operator != "COS" && \
            $operator != "TAN" && \
            $operator != "ASIN" && \
            $operator != "ACOS" && \
            $operator != "ATAN" && \
            $operator != "LN" && \
            $operator != "LOG" && \
            $operator != "EXP" && \
            $operator != "AND(" && \
            $operator != "ANDN(" && \
            $operator != "OR(" && \
            $operator != "ORN(" && \
	    $operator != "RDAX" && \
	    $operator != "LEN" && \
            $operator != ")" } {
      AlertBox -text $Message
    }
    return 1
  }
  return 0
}

####################################################################
# Procedura VerificaFile
# Procedura che verifica che la variabile non sia una KEYWORD oppure 
# in caso di segnale di scambio, verifica la congruenza delle dimensioni 
# Input: 
#   variable_name Nome variabile
#   var_range     Indice della variabile
#   t             Nome del widget di testo
# Ritorna :
#   2 non in questi casi, 1 OK,  0 errore + nuova parola 
####################################################################
proc VerificaFile { variable_name var_range t} {
global input input2 inputkw

  # Verifico se per caso non sia una KEYWORD
  if { $variable_name != "S" } {
    set indice_lista_istruzioni [lsearch -exact [read $inputkw] $variable_name] 
    seek $inputkw 0 start
    if { $indice_lista_istruzioni != -1 } {
      set variable_name [string toupper [PromptBox -text \
        "$variable_name [GetString MsgWordKeyword]" -title TitleFirstWordPanel]]
      return [list 0 $variable_name]
    }
  }

  set indice_variabile [lsearch -exact [read $input] $variable_name]
  seek $input 0 start
  if { $input2 != -1 } {
    set indice_variabile_dinamica [lsearch -exact [read $input2] $variable_name]
    seek $input2 0 start
  } else {
    set indice_variabile_dinamica -1
  }
  if { $indice_variabile != -1} {
    # se e' un segnale di scambio verifica se e' scalare o vettoriale
    set ritorno [VerificaDimensioniScambio $input $variable_name $indice_variabile \
       $var_range $t]
    if { $ritorno == 1 } {
      return 1
    } else {
      set variable_name [string toupper [PromptBox -text [lindex $ritorno 1] \
        -title TitleFirstWordPanel]]
      return [list 0 $variable_name]
    }
  } elseif { $indice_variabile_dinamica != -1} {
    # se e' un segnale di scambio dinamico verifica se e' scalare o vettoriale
    set ritorno [VerificaDimensioniScambio $input2 $variable_name \
       $indice_variabile_dinamica $var_range $t]
    if { $ritorno == 1 } {
      return 1
    } else {
      set variable_name [string toupper [PromptBox -text [lindex $ritorno 1] \
        -title TitleFirstWordPanel]]
      return [list 0 $variable_name]
    }
  }
  return 2
}

####################################################################
# Procedura ActivateInterpreter
# Procedura che attiva l'interprete in linea se la linea e' stata cambiata
# Input: 
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc ActivateInterpreter { t } {
global ModiFile InterpreteAttivo

  if {$InterpreteAttivo =="0" } {
    if { $ModiFile==1 } {
       set InterpreteAttivo 1
       PlcInterpreter $t
       set InterpreteAttivo 0
    }
  }
}

####################################################################
# Procedura EstraiCaratteriControllo
# Procedura che estrae dalla linea i bianchi ed i tab
# Input: 
#  lista_linea      Lista della linea di cui si vuole estrarre 
#                   i caretteri di controllo
# Ritorna :
#   Niente
####################################################################
proc EstraiCaratteriControllo { lista_linea  } {
global car_control

  catch { [unset car_control] }
  set j 0
  set i 0
  while { $i < [string length $lista_linea] } {
    if { [string range $lista_linea $i $i] == "\t" } {
      set car_control($j) TAB
      set j [expr $j + 1]
      set i [expr $i + 1]
    } elseif {[string range $lista_linea $i $i] == " " } {
      set car_control($j) BLANC
      set j [expr $j + 1]
      set i [expr $i + 1]
    } else {
      set car_control($j) WORD
      set j [expr $j + 1]
      while { $i < [string length $lista_linea] && \
                   [string range $lista_linea $i $i] != "\t" && \
                   [string range $lista_linea $i $i] != " "} {
        set i [expr $i + 1]
      }
    }
  }
}

####################################################################
# Procedura InserisciCaratteriControllo
# Procedura che inserisce nella linea i bianchi ed i tab
# Input: 
#  lista_linea      Lista della linea di cui si vuole estrarre 
#                   i caretteri di controllo
# Ritorna :
#   La nuova stringa con i caratteri reinseriti
####################################################################
proc InserisciCaratteriControllo { lista_linea } {
global car_control 

  set j 0
  set new_line ""

  for {set i 0 } { $i < [llength [array names car_control]] } {incr i} {
    if {$car_control($i) == "TAB"} {
      set new_line "$new_line\t"
    } elseif {$car_control($i) == "BLANC"} {
      set new_line "$new_line "
    } elseif {$car_control($i) == "WORD"} {
      set new_line "$new_line[lindex $lista_linea $j]"
      set j [expr $j + 1]
    } else  {
      for {set k $j } { $k < [expr $car_control($i) - 1] } { incr k } {
        set new_line "$new_line[lindex $lista_linea $j]"
        set new_line "$new_line "
        set j [expr $j + 1]
      } 
      set new_line "$new_line[lindex $lista_linea $j]"
      set j [expr $j + 1]
    }
  }
  if {[lsearch -exact $lista_linea "(*" ] != -1 } {
    if {[lsearch -exact $lista_linea "*)" ] == -1 } {
      set new_line "$new_line *)"
    }
  }
  if [info exist car_control] {
    unset car_control
  }
  return $new_line
}

####################################################################
# Procedura VerificaDimensioniScambio
# Procedura che verifica la correttezza delle dimensioni del segnale
# di scambio statico o dinamico
# Input: 
#  id               Identificatore del file dei segnali
#  variable_name    Nome della variabile da verificare
#  indice_variabile Indice della variabile
#  var_range        range della variabile
#  t                Nome del widget di testo
# Ritorna :
#   0 Ok 1 Errore
####################################################################
proc VerificaDimensioniScambio { id variable_name indice_variabile var_range t } {

  set tipo [lindex [read $id] [expr $indice_variabile + 4]]
  set messaggio ""
  seek $id 0 start

  if {$var_range != "index" } {
    if { [string first "ARRAY" $tipo] == -1 && $var_range != -1} {
      set messaggio "[GetString MsgScalar] $variable_name"
    } elseif { [string first "ARRAY" $tipo] != -1 && $var_range == -1} {
      set messaggio "[GetString MsgArray] $variable_name"
    } elseif { [string first "ARRAY" $tipo] != -1 && $var_range != -1} {
      set primosplit [split $tipo ","]
      if {[llength $primosplit] == 1} {
        set dim1 [string trimleft [lindex $primosplit 0] "ARRAY\[0.."]
        set dim1 [string trimright $dim1 "\]"]
        set dim2 -1
      } else {
        set dim1 [string trimleft [lindex $primosplit 0] "ARRAY\[0.."]
        set dim2 [string trimleft [lindex $primosplit 1] "0.."]
        set dim2 [string trimright $dim2 "\]" ]
      }
      set primosplit [split $var_range ","]
      if {[llength $primosplit] == 1} {
        set dim1_real [string trimleft [lindex $primosplit 0] "\[0.."]
        set dim1_real [string trimright $dim1_real "\]"]
        set dim2_real -1
      } else {
        set dim1_real [string trimleft [lindex $primosplit 0] "\[0.."]
        set dim2_real [string trimleft [lindex $primosplit 1] "0.."]
        set dim2_real [string trimright $dim2_real "\]" ]
      }
      if {$dim2 == -1 && $dim2_real != -1 } {
	set messaggio "[GetString MsgMonoDim] $variable_name"
      } elseif {$dim2 != -1 && $dim2_real == -1 } {
        set messaggio "[GetString MsgBiDi] $variable_name"
      } elseif {$dim1 < $dim1_real } {
	set messaggio  "[GetString MsgDim1] $dim1"
      } elseif {$dim2 < $dim2_real } {    
        set messaggio "[GetString MsgDim2] $dim2"
      } else {
        return 1
      }	
    } else {
      return 1
    }
  } else {
    return 1
  }
return [list 0 $messaggio]
}

####################################################################
# Procedura VerificaIntestazione
# Procedura che verifica che il programma inizi con le parole chiave corrette
# Input: 
#  t                Nome del widget di testo
#  lista_linea      Lista della linea da analizzare
# Ritorna :
#   0 Ok 1 Errore
####################################################################
proc VerificaIntestazione { t lista_linea } {
global EditFiles

  # verifico che il testo inizi con la parola chiave program
  if { [$t compare initline == 1.0] } {
    if { [TestTipoFinestra $t]=="main"} {
      set iniz_key PROGRAM
      set title NONAME
    } else {
      set iniz_key FUNCTION_BLOCK
      set title [file tail $EditFiles([TextToTop $t])]
    }
    if { [string compare [string toupper [lindex $lista_linea 0]] $iniz_key ] == 0} {
      if { [llength $lista_linea] > 1} {
	TextReplace $t initline endline "[string toupper [lindex $lista_linea 0]] \
           [string toupper [lindex $lista_linea 1]]" 0 
      } else {
        TextReplace $t initline endline "$iniz_key $title" 0
      }
      return 1
    } else {
      $t insert initline "$iniz_key $title\n"
    }
  }
  return 0
}

####################################################################
# Procedura VerificaLabelIstruzione 
# Procedura che verifica che nel primo campo ci sia o una etichetta o una istruzione
# Input: 
#  t                Nome del widget di testo
#  lista_linea      Lista della linea da analizzare
# Ritorna :
#   0 nel caso di : etichetta sola, etichetta con commmento,
#			  commento, uscita dal pannello errore con annulla.
#   Nuovo indice, flag dato macchina, eventuale lista nuova.
####################################################################
proc VerificaLabelIstruzione { t lista_linea indice_parola_corrente } { 
global InstructionList MachineDataList KeywordList

  set linea_ok 0
  set datomacc 0
  set prima_parola [string toupper [lindex $lista_linea 0]]

  while { $linea_ok == 0 } {
    foreach x [array names InstructionList] {
      set indice_lista_istruzioni [lsearch -exact $InstructionList($x) $prima_parola]
      if { $indice_lista_istruzioni != -1 } {
	break
      }
    }
    if { $indice_lista_istruzioni != -1 } {
      # La prima parola rientra nel set delle istruzioni
      set lista_linea [lreplace $lista_linea $indice_parola_corrente \
         $indice_parola_corrente $prima_parola]
      incr indice_parola_corrente 
      set linea_ok 1
    } elseif {[lsearch -exact $MachineDataList $prima_parola] != -1 } { 
      # Parole chiavi di lettura dati macchina
      set lista_linea [lreplace $lista_linea $indice_parola_corrente \
         $indice_parola_corrente $prima_parola]
      incr indice_parola_corrente 
      set linea_ok 1
      set datomacc 1
    } elseif { [string match {(\**} $prima_parola] == 1} {
      # caso del commento
      set new_line $lista_linea
      if { [string match {*\*)} \
           [lindex $lista_linea [expr [llength $lista_linea] -1]]] != 1} {
        set new_line [format "%s *)" $new_line ]
      }
      TextReplace $t initline endline $new_line 0
      return 0
    } elseif { [string match *: $prima_parola] != 0 } {
      # caso della etichetta 
      if { [lindex $lista_linea [expr $indice_parola_corrente + 1]] == "" } {
        # Se dopo l'etichetta non c'e' nulla  scrivo la linea ed esco
        TextReplace $t initline endline [string toupper $lista_linea] 0
        return 0
      } elseif {[lindex $lista_linea [expr $indice_parola_corrente + 1]] == "(*" } {
        # Se dopo l'etichetta c'e' un commento scrivo la linea ed esco
	set lista_linea [lreplace $lista_linea $indice_parola_corrente \
           $indice_parola_corrente $prima_parola] 
        # Reinserisci i caratteri di   controllo nella  linea 
        set lista_linea [InserisciCaratteriControllo  $lista_linea]
        TextReplace $t initline endline $lista_linea 0
        return 0
      } elseif {$prima_parola == ":" } {
        set lista_linea [lreplace $lista_linea $indice_parola_corrente \
          $indice_parola_corrente ""]
        incr indice_parola_corrente 
        set prima_parola [string toupper [lindex $lista_linea $indice_parola_corrente]]
      } else {
        set lista_linea [lreplace $lista_linea $indice_parola_corrente \
          $indice_parola_corrente $prima_parola]
        incr indice_parola_corrente 
        set prima_parola [string toupper [lindex $lista_linea $indice_parola_corrente]] 
      }
    } elseif {[lsearch -exact $KeywordList $prima_parola] != -1 } { 
      # caso della parola chiave
      TextReplace $t initline endline "[string toupper $lista_linea]" 0
      return 0
    } else {
      set word_to_replace $prima_parola
      set prima_parola [PromptBox -text "[GetString MsgReplaceWord] $word_to_replace" \
        -title TitleFirstWordPanel]
      set prima_parola [string toupper $prima_parola]
      if { $prima_parola == "" } {
        # se sono uscito con cancel dal pannello significa che annullo tutto
	set new_line $lista_linea
	set linea_ok 1
        TextReplace $t initline endline $new_line 0
	return 0
      } else {
        set lista_linea [lreplace $lista_linea $indice_parola_corrente \
          $indice_parola_corrente $prima_parola]
      }
    } 
  }
return [list $indice_parola_corrente $datomacc $lista_linea]
}

####################################################################
# Procedura VerificaOperando
# Procedura che verifica che nel secondo campo ci sia una variabile nota
# e se non nota la fa definire
# Input: 
#  t                Nome del widget di testo
#  operator         Operatore
#  variable_name    Nome variabile
#  datomacc         ????
# Ritorna :
#    variabile corretta
####################################################################
proc VerificaOperando { t operator variable_name datomacc} {
global VariableList PredefinedFunctionBlock VARS_DEF

  if { [string toupper $variable_name] == "VERLIB" } {
    return [string toupper $variable_name]
  }
  set variable_name_tmp $variable_name
  set linea_ok 0
  while { $linea_ok == 0 } {
    if { ($variable_name_tmp != $variable_name) && $variable_name == "" } {
      break
    }
    set ritorno [VerificaNoOpConst $operator $variable_name MsgMissing]
    if { $ritorno != 0 } {
      break
    }

    # Se la seconda parola dopo l'istruzione inizia con una lettera vado a fare 
    # tutte le ricerche del caso
    if {[string match {[^a-zA-Z_]*} $variable_name] && $datomacc == 0} {
      # Verifico se si tratta di array ed elimino le quadre []
      set var_array [string first "\[" $variable_name]
      if {$var_array  != -1 } {
        set var_index [string range $variable_name  $var_array end]
        set var_range [string range $variable_name  $var_array end]
        if [string match {*[a-zA-Z]*} $var_range] {
          set var_range index
        }
        set variable_name [string range $variable_name  0 [expr $var_array -1]]
      } else {
        set var_range -1
      }
      # Ricerca tra le KEYWORD ed i segnali di scambio
      set risultato [VerificaFile $variable_name $var_range $t]
      if { [lindex $risultato 0] == 0 } {
	set linea_ok 0
        set variable_name [lindex $risultato 1]
        continue
      } elseif { $risultato == 1} {
        set linea_ok 1
        if { $var_array  != -1 } {
          set variable_name "$variable_name$var_index"
        }
      } elseif { $risultato == 2} {
        # Non e' un segnale di scambio per cui verifico se e' una function block o 
        # una variabile
        if { $operator  == "JMP" || \
             $operator  == "JMPC" || \
             $operator  == "JMPCN"} {
          set linea_ok 1
        } elseif { $operator == "CAL" || \
	           $operator == "CALC" || \
	           $operator == "CALCN"} {
	  set linea_ok 1
	} elseif {[string match {[^a-zA-Z]*.*} $variable_name]} {
	    set linea_ok 1
            if { $var_array  != -1 } {
              set variable_name "$variable_name$var_index"
            }
        } else {
          # non e' un segnale di scambio ne' un function block ricerco tra le variabili 
          # definite
  	  set indice_variabili_utente [lsearch -exact [array names VariableList] \
            $variable_name]
	  if { $indice_variabili_utente == -1 } {
            # faccio saltar fuori il pannello di definizione variabili solo se 
            # la seconda parola non e' un numero
	    if {[string length $variable_name] > 7 } {
              set variable_name [string toupper [PromptBox -text MsgMaxVarLen \
                -title TitleFirstWordPanel]]
	      continue
	    } elseif {[string match {*[-]*}  $variable_name] == 1 } {
              set variable_name [string toupper [PromptBox -text MsgNoMenoCar \
                -title TitleFirstWordPanel]]
	      continue
	    } else {
              if {[string match {[^a-zA-Z_]*} $variable_name]} {
                set w [TextToTop $t]
                DefineVariablePanel $t $variable_name 1
	        if { $VARS_DEF(x) == -1 } {
	          set linea_ok 1
	          continue
	        }
	      }
            }
          }
  	  # faccio il controllo sulle dimensioni degli array
	  if {$var_range != "index" } {
	    if {[lindex $VariableList($variable_name) 1] != "STRING"} {
   	      if { [lindex $VariableList($variable_name) 6] > 1} {
	        set dim1 [lindex $VariableList($variable_name) 6]
	      } else {
	        set dim1 -1
	      }
	      if { [lindex $VariableList($variable_name) 7] > 1} {
                set dim2 [lindex $VariableList($variable_name) 7]
              } else {
                set dim2 -1
              }
	      if { $var_range != -1} {
	        set primosplit [split $var_range ","]
                if {[llength $primosplit] == 1} {
                  set dim1_real [string trimleft [lindex $primosplit 0] "\[0.."]
                  set dim1_real [string trimright $dim1_real "\]"]
                  set dim2_real -1
                } else {
                  set dim1_real [string trimleft [lindex $primosplit 0] "\[0.."]
                  set dim2_real [string trimleft [lindex $primosplit 1] "0.."]
                  set dim2_real [string trimright $dim2_real "\]" ]
                }
              } else {
	        set dim1_real -1 
                set dim2_real -1 
     	      } 
	      if { $dim1 == -1 && $dim1_real != -1 } {
	        set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgScalar] $variable_name" -title TitleFirstWordPanel]]
	      } elseif { $dim1 != -1 && $dim1_real == -1 } {
	        set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgArray] $variable_name" -title TitleFirstWordPanel]]
	      } elseif {$dim2 == -1 && $dim2_real != -1 } {
	        set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgArray] $variable_name" -title TitleFirstWordPanel]]
              } elseif {$dim2 != -1 && $dim2_real == -1 } {
                set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgBiDi] $variable_name" -title TitleFirstWordPanel]]
              } elseif {$dim1 < $dim1_real } {
	        set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgDim1] $dim1" -title TitleFirstWordPanel]]
              } elseif {$dim2 < $dim2_real } {    
	        set variable_name [string toupper [PromptBox -text \
                  "[GetString MsgDim2] $dim2" -title TitleFirstWordPanel]]
              } else {
                if { $var_range != -1 } {
                  set variable_name "$variable_name$var_range"
                }
                set linea_ok 1
              }
	    } else {
              set linea_ok 1
	    }
	  } else {
            set variable_name "$variable_name$var_index"
	    set linea_ok 1
	  }
        }
      }
    } else {
      if { [string range $variable_name 0 0] == "'" } {
        # Caso della stringa
        set linea_ok 1
      } elseif {[string match {*[a-zA-Z]*} $variable_name]  && $datomacc == 0 } {
      # Se non e' un numero puro c'e' stato un errore
        if {[string first "16#" $variable_name] == -1 } {
          set variable_name [string toupper [PromptBox -text \
            "$variable_name [GetString MsgNotValid]" -title TitleFirstWordPanel]]
	} else {
	  set linea_ok 1
	}
      } else {
        set linea_ok 1
      }
    }
  }
  return $variable_name
}

####################################################################
# Procedura PlcInterpreter
# Procedura che esegue un controllo in linea della riga su cui e' 
# stato premuto invio
# e se non nota la fa definire
# Input: 
#  t                Nome del widget di testo
# Ritorna :
#    Niente
####################################################################
proc PlcInterpreter { t } {

  # Estrai la linea corrente ed i caratteri di controllo dalla medesima
  $t mark set initline "insert linestart"
  $t mark set endline "insert lineend"
  set lista_linea [$t get initline endline]
  EstraiCaratteriControllo $lista_linea

  set IndiceDoppiApici [string first \" $lista_linea]
  if {$IndiceDoppiApici != -1} {
    set lista_linea "[string range $lista_linea 0 \
        [expr $IndiceDoppiApici -1]]'[string range \
	$lista_linea [expr $IndiceDoppiApici +1] end]" 
  } 
  # verifico che il testo inizi con la parola chiave program
  if { [VerificaIntestazione $t $lista_linea] == 1 } {
    return 
  }

  # Continuo con il resto del programma
  if {[$t compare initline != endline]} {
    # se il primo elemento e' vuoto esco senza controlli
    if { [lindex $lista_linea 0] == {} } {
      return
    }

    #inizializzazioni
    set indice_parola_corrente 0
    set datomacc 0

    set ritorno [VerificaLabelIstruzione $t $lista_linea $indice_parola_corrente]

    if { $ritorno == 0 } {
      return
    } else {
      set indice_parola_corrente [lindex $ritorno 0]
      set datomacc [lindex $ritorno 1]
      set lista_linea [lrange [lindex $ritorno 2] 0 end]
    }

    if { $datomacc == 0 && \
         [string range [lindex $lista_linea $indice_parola_corrente] 0 0] != "'"} {
      set variable_name [string toupper [lindex $lista_linea $indice_parola_corrente]]
    } else {
      set variable_name [lindex $lista_linea $indice_parola_corrente]
    }
    set operator [string toupper [lindex $lista_linea [expr $indice_parola_corrente - 1]]]

    set variable_name_tmp [VerificaOperando $t $operator $variable_name $datomacc]
    if { $variable_name_tmp != "" } {
      set variable_name $variable_name_tmp
#MODIFICA del 23.05.00
      # Se e' una stringa setto il flag di dato macchina
      if { [string range $variable_name 0 0] == "'" } {
        set datomacc 1
      }
    }
    if { $indice_parola_corrente <= [expr [llength $lista_linea] - 1] } {
      set lista_linea [lreplace $lista_linea $indice_parola_corrente \
        $indice_parola_corrente $variable_name]
    }

    # Reinserisci i caratteri di   controllo nella  linea 
    set lista_linea [InserisciCaratteriControllo  $lista_linea] 

    # Se c'e' il commento dopo l'operando verificare che sia chiuso
    if { [string first "(*" $lista_linea ] != -1 } {
      if {[string first "*)" $lista_linea] == -1 } {
        set lista_linea "$lista_linea *)"
      }
    }

    TextReplace $t initline endline $lista_linea $datomacc
  }
}

####################################################################
# Procedura SetNewCompileOption
# Procedura che salva i valori di impostati per la compilazione
# Input:
#   WdgName   Nome della toplevel per i parametri
# Ritorna :
#   Niente
####################################################################
proc SetNewCompileOption { WdgName } {
global EditPrefs PrtOpt prt_opz_change
global qualescambdin PercorsoRemoto
global input input2

  set Dove [TrovaFile $EditPrefs(qualescambio)]
  if { $Dove != -1 } {
    set estensione [split $qualescambdin .]
    if { [llength $estensione] == 1 } {
      set Dove [TrovaFile scambio]
    } else {
      set Dove [TrovaFile scambio.[lindex $estensione 1]]
    }
    if { $Dove == -1 } {
      AlertBox -text "scambio.[lindex $estensione 1] [GetString FileNonTrovato]"
      return
    }
  } else {
    AlertBox -text "$qualescambdin [GetString FileNonTrovato]"
    return
  }

  if { "$EditPrefs(qualescambdin)" != "$qualescambdin" } {
    set EditPrefs(qualescambdin) $qualescambdin
    set PrtOpt(qualescambdin) $qualescambdin
    set estensione [split $qualescambdin .]
    if { [llength $estensione] == 1 } {
      set EditPrefs(qualescambio) scambio
      set PrtOpt(qualescambio) scambio
    } else {
      set EditPrefs(qualescambio) scambio.[lindex $estensione 1]
      set PrtOpt(qualescambio) scambio.[lindex $estensione 1]
    }
  }

  set prt_opz_change yes

  if { $input != -1 } {
    close $input
  }
  if { $input2 != -1 } {
    close $input2
  }
  ApriFileScambio

  if { "$EditPrefs(binpathremoto)" != "$PercorsoRemoto" } {
    set EditPrefs(binpathremoto) $PercorsoRemoto
    set PrtOpt(binpathremoto) $PercorsoRemoto
  }

  destroy $WdgName
}

####################################################################
# Procedura SetRemoteCompileOption
# Procedura che richede i file scambio e scambdin alla macchina remota
# Input:
#   LblWdg   Widget di label da aggiornare
# Ritorna :
#   Niente
####################################################################
proc SetRemoteCompileOption { LblWdg } {
global EditPrefs PrtOpt prt_opz_change
global qualescambdin PercorsoRemoto
global input input2 QualeScambio QualeScambdin

  if { $input != -1 } {
    close $input
  }
  if { $input2 != -1 } {
    close $input2
  }

  # Creo due file di backup
  if [file exist $QualeScambio] {
    file copy -force $QualeScambio [file root $QualeScambio].bck
  }
  if [file exist $QualeScambdin] {
    file copy -force $QualeScambdin [file root $QualeScambdin].bck
  }

  set NomeDelFile [file tail $QualeScambio]
  set DirettorioRemoto $EditPrefs(binpathremoto)
  set DirettorioLocale [file dirname $QualeScambio]

  catch {socket}  res

  if { [ReceiveFile $NomeDelFile $DirettorioRemoto $DirettorioLocale \
         $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
     AlertBox -text "scambio [GetString FileNonTrovato] "
    if [file exist [file root $QualeScambio].bck] {
      file copy -force [file root $QualeScambio].bck $QualeScambio
    }
  }

  set NomeDelFile [file tail $QualeScambdin]
  set DirettorioRemoto $EditPrefs(binpathremoto)
  set DirettorioLocale [file dirname $QualeScambdin]

  if { [ReceiveFile $NomeDelFile $DirettorioRemoto $DirettorioLocale \
         $EditPrefs(plcremoto) $EditPrefs(plclogin) $EditPrefs(plcpassword)] != "0" } {
     AlertBox -text "scambdin [GetString FileNonTrovato] "
    if [file exist [file root $QualeScambdin].bck] {
      file copy -force [file root $QualeScambdin].bck $QualeScambdin
    }
  }

  AggiornaVersione $LblWdg
  ApriFileScambio
}

####################################################################
# Procedura EstraiVersione
# Funzione che estrae la versione dei file scambdin
# Input: 
#   Nessuna
# Ritorna :
#   Niente
####################################################################
proc EstraiVersione { } {
global qualescambdin

  set Dove [TrovaFile scambdin]
  if { $Dove != -1 } {
    set NomeFile $Dove/$qualescambdin
  } else {
    return -1
  }
  if [file exist $NomeFile] {
    set fId [open $NomeFile r]
    set Tutto [read $fId]
    close $fId
    set IndiceVersione [lsearch -exact $Tutto "Versione"]
    if { $IndiceVersione != -1 } {
      return [lindex $Tutto [expr $IndiceVersione +1]]
    } else {
      return -1
    }
  } else {
    return -1
  }
}

####################################################################
# Procedura AggiornaVersione
# Funzione che aggiorna la versione
# Input: 
#   WdgLabel   Widget della label
# Ritorna :
#   Niente
####################################################################
proc AggiornaVersione { WdgLabel args } {

  set Versione [EstraiVersione]
  if { $Versione != -1 } {
    $WdgLabel config -text "[GetString prt_stam_tvers] $Versione"
  } else {
    $WdgLabel config -text ""
  }
}

####################################################################
# Procedura CompileOptionWindow
# Apre la finestra per la gestione di scambdin
# Input: 
#   t             Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc CompileOptionWindow { t } {
global OptCompile EditPrefs tcl_platform
global qualescambdin PercorsoRemoto

  set old_focus [focus]
  set MainW [winfo toplevel [focus]]
  grab release [grab current]

  toplevel .compwin
  wm title .compwin [GetString CmdCompileOption]
  wm protocol .compwin WM_DELETE_WINDOW procx

  label .compwin.version -font $EditPrefs(windowfont)

  tixOptionMenu .compwin.scambdin -label "[GetString Scambdin]   " \
	-variable qualescambdin -command "AggiornaVersione .compwin.version" \
	-options "
	    label.anchor e
            label.font $EditPrefs(windowfont)
	    menubutton.width 15
	    menubutton.font $EditPrefs(messagefont)
	    menu.font $EditPrefs(messagefont)
	"

  set Dove [TrovaFile scambdin]
  if { $Dove != -1 } {
    set VecchioPercorso [pwd]
    cd $Dove
    foreach opt [lsort [glob -nocomplain scambdin*]] {
      .compwin.scambdin add command $opt -label $opt
    }
    cd $VecchioPercorso
  }

  set qualescambdin $EditPrefs(qualescambdin)

  set PercorsoRemoto $EditPrefs(binpathremoto)

  VariableEntry .compwin.rempath -label [GetString PercorsoRemoto] \
    -labelwidth 25 -entrywidth 25 -variable PercorsoRemoto   

  tixButtonBox .compwin.b -orientation horizontal

  .compwin.b add ok -image ok -font $EditPrefs(windowfont) \
    -command " SetNewCompileOption .compwin"
  if { !$EditPrefs(plclocale) } {
    .compwin.b add default -image aggscamb -font $EditPrefs(windowfont) \
      -command " SetRemoteCompileOption .compwin.version"
  }
  .compwin.b add cancel -image cancel -font $EditPrefs(windowfont) \
    -command { destroy .compwin }

  CniBalloon [.compwin.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  if { !$EditPrefs(plclocale) } {
    CniBalloon [.compwin.b subwidget default] [GetString GetScambdin] \
      $EditPrefs(windowfont) 500
  }
  CniBalloon [.compwin.b subwidget cancel] [GetString CANCEL] \
      $EditPrefs(windowfont) 500

  if { $EditPrefs(plclocale) } {
    pack .compwin.version .compwin.scambdin -side top -pady 10
  } else {
    pack .compwin.version .compwin.scambdin .compwin.rempath -side top -pady 10
  }       

  pack .compwin.b -side top -fill x -pady 5

  set Versione [EstraiVersione]
  if { $Versione != -1 } {
    .compwin.version config -text "[GetString prt_stam_tvers] $Versione"
  }

  wm transient .compwin $MainW
  PosizionaFinestre $MainW .compwin c

  bind .compwin <Key-Escape> {destroy .compwin}
  bind .compwin <Control-d> "set EditPrefs(DebugColloquio) .debug"
  bind .compwin <Control-q> "set EditPrefs(DebugColloquio) 0"

  DefaultButton [.compwin.b subwidget ok] .compwin.rempath.e 

  grab .compwin
  tkwait window .compwin

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }
}

####################################################################
# Procedura RicreaLibrerie
# Comando che ricrea gli eventuali *.o
# di libreria necessari per la compilazione
# Input: 
#   listobj       Lista delle librerie necessarie
# Ritorna :
#   Niente
####################################################################
proc RicreaLibrerie { listobj } {
global Home EditPrefs ModiFile input2 lista_librerie FunctionBlockList
global tcl_platform QualeScambio QualeScambdin

  if { $listobj == "" } {
    return 0
  } 

  ScriviDebugColloquio "Ricompilazione librerie"
  ScriviDebugColloquio $QualeScambio
  ScriviDebugColloquio $QualeScambdin

  set segnali $QualeScambio
  set function_block_code $Home/util/illib.ilc

  # Caso della libreria
  foreach librerie $listobj {
    if [file exist $librerie] {
      continue
    }
    set filename [file root $librerie].plc
    if ![file exists $filename] {
      AlertBox -text "[GetString MsgNoLibrarySource] \n $filename" 
      return 1
    }
    set libraryName [file root $filename]
    set ScambdinFile $QualeScambdin
    ScriviDebugColloquio "$Home/ilc -il -g -w 0 -header $segnali $function_block_code \
             $ScambdinFile -code $filename -os $filename.s"
    catch {exec $Home/ilc -il -g -w 0 -header $segnali $function_block_code \
             $ScambdinFile -code $filename -os $filename.s} Risultato
    if {$Risultato == ""} { 
      if {$tcl_platform(platform) == "unix"} {
        ScriviDebugColloquio "as $filename.s -o $libraryName.o"
        catch {exec as $filename.s -o $libraryName.o} Risultato 
      } else {
        ScriviDebugColloquio "$Home/as $filename.s -o $libraryName.o"
        catch {exec $Home/as $filename.s -o $libraryName.o} Risultato 
      }
    } else {
      VisualizzazioneErroriCompilazione $EditPrefs(nonintrepretaerrori) $t $Risultato
      return 1
    }
    if {$Risultato == ""} {
      file delete $filename.s
    }
  }
   return 0
}
