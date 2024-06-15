
            ######################################
            #                                    #
            #     File di creazione della        #
            #     menu bar                       #
            #                                    #
            ######################################

####################################################################
# Procedura MenuMenubutton
# Procedura di creazione dei bottoni della menubar
# Input: 
#   mb    Nome della menu bar
#   menu  Nome menu button
#   key   Testo da aseegnare al menubutton
# Ritorna :
#   Niente
####################################################################
proc MenuMenubutton { mb menu key } {

  global EditPrefs 

  menubutton $mb -menu $menu \
    -text [GetString $key] 
  menu $menu -tearoff 0
  
  if [ info exists EditPrefs(windowfont) ] {
    $mb configure -font $EditPrefs(windowfont)
  }

  return $mb
}

####################################################################
# Procedura MenuCommands
# Procedura di creazione degli entry di ciascuna voce
# della menubar.
# Gli argmomenti dati in args servono per disabilitare le voci dei menu
# a seconda del tipo di finestra, nell'ordine  LADDER,NEW WINDOW,FB
# Input: 
#   menu  Nome del menu button
#   w     Widget di testo
#   cmds  Comando associato al bottone
#   args  Flags di presenza o meno della voce a seconda della
#         finestra chiamante
#         la prima lista riguarda le voci da disabilitare nel caso di ladder
#         la seconda lista riguarda le voci da disabilitare nel caso di NEW WINDOW
#         la terza lista riguarda le voci da disabilitare nel caso di function block
#         la quarta lista riguarda le voci da disabilitare nel caso di debug
#         la quinta lista riguarda le voci da disabilitare nel caso di editor ladder
# Ritorna :
#   Niente
####################################################################
proc MenuCommands { menu w cmds args } {
global EditPrefs DebugNoWidget LadderWidget
global EditLadderWidget 

  # Il parametro args contiene 4 liste di flag:
  set i 0
  foreach cmd $cmds {
    #voci del menu da disabilitare in caso di LADDER
    if { [llength $args] > 0 } {
      if { [lindex [lindex $args 0] $i] == 0 && "x$cmd" != "x-"} {
        if { [info exist LadderWidget] == 0 } {
          set LadderWidget [lappend LadderWidget $menu:[GetString $cmd]] 
        } elseif { [lsearch -exact $LadderWidget $menu:[GetString $cmd]] == -1 } {
          set LadderWidget [lappend LadderWidget $menu:[GetString $cmd]] 
        }
      }
      if { [lindex [lindex $args 4] $i] == 0 && "x$cmd" != "x-"} {
        if { [info exist EditLadderWidget] == 0 } {
          set EditLadderWidget [lappend EditLadderWidget $menu:[GetString $cmd]] 
        } elseif { [lsearch -exact $EditLadderWidget $menu:[GetString $cmd]] == -1 && \
           $cmd != "CmdGoToLine"} {
          set EditLadderWidget [lappend EditLadderWidget $menu:[GetString $cmd]] 
        }
      }
    } 

    set tipo [TestTipoFinestra $w]
    set W [TextToTop $w]

    #voci del menu da disabilitare in caso di NEW WINDOW (.jedit>0)
    if { [llength $args] > 1 && $tipo=="main" && $W!=".jedit0"} {
      set stato1 [lindex [lindex $args 1] $i]
    } else {
      set stato1 1
    }

    #voci del menu da disabilitare in caso di FUNCTION BLOCK
    if { [llength $args] > 2  && $tipo=="fb" } {
      set stato2 [lindex [lindex $args 2] $i]
    } else {
      set stato2 1
    }

    if { $stato1==0 || $stato2==0 } {
      set state disabled
    } else {
      set state normal
    }

    # metto in DebugNoWidget i widget da nascondere in caso di debug
    if { [llength $args] > 3 } {
      if { [lindex [lindex $args 3] $i] == 0 && "x$cmd" != "x-" && \
           $state == "normal"} {
        if { [info exist DebugNoWidget] == 0 } {
          set DebugNoWidget [lappend DebugNoWidget $menu:[GetString $cmd]] 
        } elseif { [lsearch -exact $DebugNoWidget $menu:[GetString $cmd]] == -1 } {
          set DebugNoWidget [lappend DebugNoWidget $menu:[GetString $cmd]] 
        }
      }
    }

    if { $state == "normal" } {
      if {"x$cmd" == "x-"} {
        $menu add separator
      } else {
        $menu add command \
          -label [GetString $cmd] \
          -accelerator [GetAccelerator $cmd] \
          -command [list $cmd $w] \
          -state $state
      }
      if [ info exists EditPrefs(windowfont) ] {
       $menu configure -font $EditPrefs(windowfont)
      }
      SetBind $w [MenuToTop $menu] $cmd 
    }
    incr i
  }
}

####################################################################
# Procedura MenuCheckbuttons
# Procedura di creazione di checkButton in menubar.
# Input: 
#   args  Argomenti vari
# Ritorna :
#   Niente
####################################################################
proc MenuCheckbuttons { args } {
  global EditPrefs

  ParseArgs {
    {command {}}
    {font {}}
    {variable VAR}
  }

  set menu [lindex $args 0]
  set list [lindex $args 1]
  
  foreach sublist $list {
    if {"x$sublist" == "x-"} {
      $menu add separator
      continue
    }
    
    set key [lindex $sublist 0]
    set var_value [lindex $sublist 1]
    set var_name [lindex $sublist 2]
    
    if {"x$var_name" == "x"} {
      set var_name $variable
    }
    
    if {"x$command" == "x"} {
      $menu add checkbutton \
        -label [GetString $key] \
        -accelerator [GetAccelerator $key] \
        -variable $var_name \
        -selectcolor $EditPrefs(checkcolor) \
    } else {
      $menu add checkbutton \
        -command $command \
        -font $font \
        -label [GetString $key] \
        -accelerator [GetAccelerator $key] \
        -variable $var_name \
        -selectcolor $EditPrefs(checkcolor) \
    }
  }
}

####################################################################
# Procedura MenuRadiobuttons
# Procedura di creazione di radioButton in menubar.
# Input: 
#   args  Argomenti vari
# Ritorna :
#   Niente
####################################################################
proc MenuRadiobuttons { args } {

  global EditPrefs

  ParseArgs {
    {command {}}
    {font {}}
    {variable VAR}
  }

  set menu [lindex $args 0]
  set list [lindex $args 1]
  
  foreach sublist $list {
    if {"x$sublist" == "x-"} {
      $menu add separator
      continue
    }
    
    set key [lindex $sublist 0]
    set var_value [lindex $sublist 1]
    set var_name [lindex $sublist 2]
    
    if {"x$var_name" == "x"} {
      set var_name $variable
    }
    
    if {"x$command" == "x"} {
      $menu add radiobutton \
        -label [GetString $key] \
        -accelerator [GetAccelerator $key] \
        -variable $var_name \
        -selectcolor $EditPrefs(checkcolor) \
        -value $var_value
    } else {
      $menu add radiobutton \
        -command $command \
        -font $font \
        -label [GetString $key] \
        -accelerator [GetAccelerator $key] \
        -variable $var_name \
        -selectcolor $EditPrefs(checkcolor) \
        -value $var_value
    }
  }
}

####################################################################
# Procedura MenusCreate
# Procedura di creazione della menu bar
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
#   tipo  IL o LADDER
# Ritorna :
#   Niente
####################################################################
proc MenusCreate {mb t tipo} {
global EditPrefs 
  
  if { $tipo == "IL" } {
    set all_menus {file edit user print}
    if [winfo exists $mb.ladder] {
      destroy $mb.ladder
    }
  } elseif { $tipo == "LADDER" } {
    set all_menus {file edit user print ladder}
  }
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
# Procedura MkMenufile
# Procedura di creazione del sottomenu file
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenufile {menu t} {
  global EditPrefs
  
  MenuMenubutton $menu $menu.m MenuFile
  
  MenuCommands $menu.m $t {
      CmdLoad
      CmdLoadEsecuzione
      CmdSave
      CmdSaveas
      -
      CmdLoadLibrary
      CmdSaveLibrary
      CmdSaveasLibrary
      -
      CmdBackup
      -
      CmdNewWindow
      CmdClose
      CmdQuit
  } {1 1 1 1 1 0 0 0 1 1 1 0 1 1} \
    {1 1 0 0 0 0 0 0 0 0 1 0 1 0} \
    {0 0 1 0 0 0 0 0 0 0 1 0 0 1} \
    {0 0 0 0 0 0 0 0 0 0 0 0 0 1} \
    {0 0 0 0 0 1 1 1 0 0 0 1 0 1}
}

####################################################################
# Procedura MkMenuedit
# Procedura di creazione del sottomenu edit
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenuedit {menu t} {
  global EditPrefs
  
  MenuMenubutton $menu $menu.m MenuEdit
  
  MenuCommands $menu.m $t {
    CmdCut
    CmdCopy
    CmdPaste
    CmdSelectAll
    -
    CmdFind
    CmdFindAgain
    -
    CmdGoToLine
    -
    CmdUndo
    CmdRedo
    -
    CmdWrapline
    DeleteLine
  } {0 0 0 0 0 1 1 1 1 0 0 0 0 0 1} \
    {1 1 1 1 1 1 1 1 1 1 1 1 1 1 1} \
    {1 1 1 1 1 1 1 1 1 1 1 1 1 1 1} \
    {0 0 0 0 0 1 1 1 1 0 0 0 0 0 0} \
    {1 1 1 1 1 0 0 0 0 1 1 1 1 1 1} 
}

####################################################################
# Procedura MkMenuuser
# Procedura di creazione del sottomenu user
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenuuser {menu t} {
global DebugState EditPrefs DebugNoWidget tcl_platform
global EditLadderWidget

  MenuMenubutton $menu $menu.m MenuUser

  MenuCommands $menu.m $t {
    CmdIoConnect
    -
    CmdLoadFB
    -
    CmdCalcolaEquazioni
    -
    StartStopDebug
    CmdDebugOption
    CmdCompileOption
    CmdLingua
    -
    DefinedFunctionBlockList
    DefinedVariableList
    StartCompileProgram
    LoadPlc
    -
  } {1 1 0 0 0 1 1 1 1 1 1 1 1 1 1 1} \
    {0 0 1 1 1 1 0 0 0 0 0 1 1 0 0 1} \
    {0 0 0 0 1 1 0 0 0 0 0 1 1 0 0 1} \
    {0 0 0 0 0 1 1 0 0 0 1 0 0 0 0 1} \
    {0 0 1 1 1 0 0 0 0 0 0 0 0 0 0 1} 

  set w [TextToTop $t]
  if { [lsearch -exact $DebugNoWidget $menu.m:[GetString TextLadder]] == -1 } {
    set DebugNoWidget [lappend DebugNoWidget $menu.m:[GetString TextLadder]]
    set EditLadderWidget [lappend EditLadderWidget $menu.m:[GetString TextLadder]]
  }
  if { [lsearch -exact $DebugNoWidget $menu.m:[GetString TextAwl]] == -1 } {
    set DebugNoWidget [lappend DebugNoWidget $menu.m:[GetString TextAwl]]
    set EditLadderWidget [lappend EditLadderWidget $menu.m:[GetString TextAwl]]
  }
  MenuRadiobuttons $menu.m \
    -command "IlLadderConvert $w" \
    -font $EditPrefs(windowfont) \
    -variable EditPrefs(awl) \
    { {TextLadder 0} {TextAwl 1} }
}

####################################################################
# Procedura MkMenuprint
# Procedura di creazione del sottomenu print
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenuprint {menu t} {

  MenuMenubutton $menu $menu.m MenuPrint

  MenuCommands $menu.m $t {
    CmdPrintPlci
    CmdPrintPlcl
    CmdPrintMsd
    CmdPrintCross
    -
    CmdPrintDocal
    -
    CmdPrintPopz
  } {1 1 1 1 1 1 1 1} \
    {0 0 0 0 0 0 0 0} \
    {0 0 0 0 0 0 0 0} \
    {0 0 0 0 0 0 0 0} \
    {0 0 0 0 0 0 0 0}
}

####################################################################
# Procedura MkMenuladder
# Procedura di creazione del sottomenu ladder
# Input: 
#   mb    Nome della menu bar
#   t     Nome del widget di testo
# Ritorna :
#   Niente
####################################################################
proc MkMenuladder {menu t} {
  
  MenuMenubutton $menu $menu.m MenuLadder
  
  MenuCommands $menu.m $t {
      CmdNewSegment
      CmdNewFunctioBlock
      CmdNewMathEq
      CmdOldSegment
      -
      CmdLabel
      CmdShowLadder
      -
      CmdInsertSegment
      CmdDeleteSegment
  } {1 1 1 1 1 1 1 1 1 1} \
    {1 1 1 1 1 1 1 1 1 1} \
    {1 1 1 1 1 1 1 1 1 1} \
    {0 0 0 0 0 0 0 0 0 0} \
    {0 0 0 0 0 0 1 0 1 0}
}

####################################################################
# Procedura GetAccelerator
# Procedura che ritorna l'eventuale acceleratore associato ad
# una voce di menu bar
# Input: 
#   key   Voce della menu bar
# Ritorna :
#   L'eventuale acceleratore
####################################################################
proc GetAccelerator { key } {
  global KeyAccelerator
  
  if [info exists KeyAccelerator($key)] {
    return $KeyAccelerator($key)
  }
}

####################################################################
# Procedura SetBind
# Procedura che associa alla voce della menu bar un bind
# Input: 
#   w     Nome della toplevel
#   w2    Nome del widget di testo
#   cmd   Comando da chiamare
# Ritorna :
#   L'eventuale acceleratore
####################################################################
proc SetBind { w w2 cmd} {

  global EditPrefs DebugState

  set acceleratore [GetAccelerator $cmd]
  if { [string index $acceleratore 0] == "^" } {
    set acceleratore [string trimleft $acceleratore "^"]
    if { "$cmd" == "CmdCut" || "$cmd" == "CmdCopy" || "$cmd" == "CmdPaste" } {
      bind [winfo class $w] <Control-$acceleratore> { }
      bind [winfo class $w] <Control-[string toupper $acceleratore]> { }
    }
    bind $w2 <Control-$acceleratore> "$cmd $w"
    bind $w2 <Control-[string toupper $acceleratore]> "$cmd $w"
  }
}

####################################################################
# Procedura GetString
# Procedura che data una etichetta ritorna il testo in
# lingua corrispondente
# Input: 
#   NomeEtichetta    Etichetta associata alla stringa
# Ritorna :
#   Niente
####################################################################
proc GetString { NomeEtichetta } {

  global EditPrefs Home LanguageFile

  set Lingua $EditPrefs(lingua)

  set IndiceEtichetta [string first "$NomeEtichetta.$Lingua" $LanguageFile]
  if { $IndiceEtichetta != -1 } {
    set FileLinguaTmp [string range $LanguageFile $IndiceEtichetta end]
    set Linea [string range $FileLinguaTmp 0 [string first ";" $FileLinguaTmp]] 
    while {[string first "\\n" $Linea] != -1 } {
      set Acapo [string first "\\n" $Linea]
      set Linea "[string range $Linea 0 [expr $Acapo -1]]\n[string range $Linea \
         [expr $Acapo +2] end]" 
    }
    return [string range $Linea [expr [string first "=" $Linea] + 1] \
         [expr [string length $Linea] -2]]
  } else {
    return $NomeEtichetta
  }
} 

######################################################################
# SetMnemonicMenuBar Widget Menubar - 
# Setta gli mnemonici della menu bar
######################################################################
proc SetMnemonicMenuBar {MBWidget} {

  set ListaFigli [winfo children $MBWidget]

  set ElencoMnemonici ""

  foreach Figlio $ListaFigli {
    set Stringa [$Figlio cget -text]
    for {set i 0} {$i < [string length $Stringa]} {incr i} {
      if { [string first [string index $Stringa $i] $ElencoMnemonici] == -1 } {
        $Figlio config -underline $i
        append ElencoMnemonici [string index $Stringa $i]
        break
      }
    }
  }
}
