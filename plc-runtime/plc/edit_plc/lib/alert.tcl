
            ######################################
            #                                    #
            #     File di gestione Box           #
            #     di comunicazione con utente    #
            #                                    #
            ######################################

####################################################################
# Procedura PromptBox
# Definizione di un box di input dati
# Possono essere assegnate le seguenti opzioni:
#   -text (default "Enter a value:")
#   -default (default "")
#   -text2 (default "Enter a value:")
#   -default2 (default "")
#   -cancelvalue (default "")
#   -title (default "Prompt")
#   -numentry (default 1)
#   -uppercase (default 0)
# Input: 
#   args    Opzioni
# Ritorna :
#   Niente
####################################################################
proc PromptBox { args } {
global EditPrefs TmpReturn MsdData

  ParseArgs {
    {text "Enter a value:"}
    {default ""}
    {text2 "Enter a value:"}
    {default2 ""}
    {cancelvalue ""}
    {title Prompt}
    {numentry 1}
    {uppercase 0}
  }
  
  set text [GetString $text]
  set text2 [GetString $text2]
  set title [GetString $title]
  catch {unset TmpReturn}
  
  set old_focus [focus]
  if { $old_focus == "" } {
    set old_focus .jedit0.main.t
  }
  set MainW [winfo toplevel $old_focus]

  if [winfo exists .pr] {
    return
  }

  grab release [grab current]

  toplevel .pr
  wm title .pr $title
  wm protocol .pr WM_DELETE_WINDOW procx
  
  message .pr.msg -width 300 -anchor w -text $text
  entry .pr.field -width 40 -textvariable TmpReturn(result)
  if { $numentry == 2 } {
    message .pr.msg2 -width 300 -anchor w -text $text2
    entry .pr.field2 -width 40 -textvariable TmpReturn(result2)
  }
  tixButtonBox .pr.b -orientation horizontal
  .pr.b add ok  -image ok -font $EditPrefs(windowfont)  -command "destroy .pr" 
  .pr.b add cancel -image cancel -font $EditPrefs(windowfont) -command "
                                            set TmpReturn(result) $cancelvalue
                                            destroy .pr" 

  if [ info exists EditPrefs(windowfont) ] {
    .pr.msg configure -font $EditPrefs(messagefont)
    .pr.field configure -font $EditPrefs(windowfont)
    if { $numentry == 2 } {
      .pr.msg2 configure -font $EditPrefs(messagefont)
      .pr.field2 configure -font $EditPrefs(windowfont)
    }
  }

  CniBalloon [.pr.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.pr.b subwidget cancel] [GetString CANCEL] \
        $EditPrefs(windowfont) 500

  pack .pr.msg -side top -fill both -expand yes -padx 10
  pack .pr.field -side top -padx 10 -pady 10
  if { $numentry == 2 } {
    pack .pr.msg2 -side top -fill both -expand yes -padx 10
    pack .pr.field2 -side top -padx 10 -pady 10
  }
  pack .pr.b -side bottom -fill x

  .pr.field delete 0 end
  .pr.field insert end $default
  if { $numentry == 2 } {
    .pr.field2 delete 0 end
    .pr.field2 insert end $default2
  }
  
  wm transient .pr $MainW
  PosizionaFinestre $MainW .pr c

  if { $numentry == 1 } {
    DefaultButton [.pr.b subwidget ok] .pr.field
  } else {
    DefaultButton [.pr.b subwidget ok] .pr.field .pr.field2
  }

  focus .pr.field

  $old_focus config -cursor pirate
  [winfo toplevel $old_focus] config -cursor pirate

  if { $uppercase } {
    # faccio in modo che la scrittura della variabile nel campo di editing sia 
    # maiuscolo
    bind .pr.field <Any-KeyRelease> {
      set TmpReturn(result) [string toupper $TmpReturn(result)] 
      if {[string length $TmpReturn(result)] > $MsdData(lunvar)} {
        %W delete $MsdData(lunvar) end
      }
    }
  }

  bind .pr <Enter> {
    .pr config -cursor left_ptr
  }

  bind .pr <Leave> [format {
    %s config -cursor pirate
    [winfo toplevel %s] config -cursor pirate
  } $old_focus $old_focus] 

  bind .pr <Key-Escape> { [.pr.b subwidget cancel] invoke}

  grab .pr
  tkwait window .pr

  if [winfo exists $old_focus] {
    focus -force $old_focus 
  }

  $old_focus config -cursor left_ptr
  [winfo toplevel $old_focus] config -cursor left_ptr
  if { $numentry == 1 } {
    return $TmpReturn(result)
  } else {
    if { $TmpReturn(result) == "" } {
      return $TmpReturn(result)
    } else {
      return [list $TmpReturn(result) $TmpReturn(result2)]
    }
  }
}

####################################################################
# Procedura AlertBox
# Definizione di un box di avverimento
# Possono essere assegnate le seguenti opzioni:
#   -title (default "Alert")
#   -text (default "Alert!" - not really optional")
#
# Input: 
#   args    Opzioni
# Ritorna :
#   Niente
####################################################################
proc AlertBox { args } {
global EditPrefs 

  ParseArgs {
    {title "Alert"}
    {text "Alert!"}
  }

  set title [GetString $title]
  set text [GetString $text]
  
  set old_focus [focus]
  if { $old_focus == "" } {
    set old_focus .jedit0.main.t
  }
  set MainW [winfo toplevel $old_focus]
  
  if [winfo exists .alert] {
    return
  }

  grab release [grab current]

  toplevel .alert
  
  wm title .alert $title
  wm protocol .alert WM_DELETE_WINDOW procx  ;# dis.tasto x
  
  frame .alert.frm
  label .alert.frm.lbim -justify center -image info4 
  label .alert.frm.lbt -justify center -text $text -font $EditPrefs(messagefont)

  tixButtonBox .alert.frm1 -orientation horizontal
  .alert.frm1 add ok -image ok -font $EditPrefs(windowfont)  -command "
                                                            destroy .alert"

  CniBalloon [.alert.frm1 subwidget ok] [GetString OK] \
       $EditPrefs(windowfont) 500

  pack .alert.frm.lbim -side left -padx 20 -anchor w
  pack .alert.frm.lbt -side left -padx 20 -anchor w
  pack .alert.frm -pady 15 -anchor c -side top
  pack .alert.frm1 -pady 5 -fill x

  wm transient .alert $MainW
  PosizionaFinestre $MainW .alert c
  
  DefaultButton [.alert.frm1 subwidget ok] .alert

  $old_focus config -cursor pirate
  [winfo toplevel $old_focus] config -cursor pirate

  focus .alert

  bind .alert <Enter> {
    %W config -cursor left_ptr
  }

  bind .alert <Leave> [format {
    %s config -cursor pirate
    [winfo toplevel %s] config -cursor pirate
  } $old_focus $old_focus] 

  bind .alert <Key-Escape> { [.alert.frm1 subwidget ok] invoke}

  grab .alert
  tkwait window .alert

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }

  $old_focus config -cursor left_ptr
  [winfo toplevel $old_focus] config -cursor left_ptr
}

####################################################################
# Procedura ConfirmBox
# Definizione di un box di richiesta conferma
# Possono essere assegnate le seguenti opzioni:
#   -title (default "Confirm")
#   -text (default "Are you sure?")
#   -yesbutton (default "Print")
#   -nobutton (default "CANCEL")
#   -thirdbutton (default "")
#
# Input: 
#   args    Opzioni
# Ritorna :
#   Vero (1) su OK; Falso (0) su CANCEL; (2) su THIRD_BUTTON (eventuale)
####################################################################
proc ConfirmBox { args } {
  global EditPrefs TmpReturn

  ParseArgs {
    {title Confirm}
    {text MsgSure}
    {yesbutton OK}
    {nobutton CANCEL}
    {thirdbutton ""}
  }
  
  if { $nobutton == "CANCEL" } {
    set CancelImage cancel
  } elseif { $nobutton == "OKALL" } {
    set CancelImage okall
  }

  set title [GetString $title]
  set text [GetString $text]
  set yesbutton [GetString $yesbutton]
  set nobutton [GetString $nobutton]
  if { $thirdbutton != "" } {
    set thirdbutton [GetString $thirdbutton]
  }
  catch {unset TmpReturn}
  
  set old_focus [focus]
  if { $old_focus == "" } {
    set old_focus .jedit0.main.t
  }
  set MainW [winfo toplevel $old_focus]

  if [winfo exists .confirm] {
    return
  }

  grab release [grab current]

  toplevel .confirm
  wm title .confirm $title
  wm protocol .confirm WM_DELETE_WINDOW procx  ;# dis.tasto x
  
  frame .confirm.frm
  frame .confirm.sep -class Rule
  label .confirm.frm.lbim -justify center -image confirm 
  label .confirm.frm.lbt -justify center -text $text \
             -font $EditPrefs(messagefont)

  tixButtonBox .confirm.frm1 -orientation horizontal
  .confirm.frm1 add ok -image ok -font $EditPrefs(windowfont)  -command "
                                                       set TmpReturn 1
                                                       destroy .confirm" 
  .confirm.frm1 add cancel -image $CancelImage -font $EditPrefs(windowfont) -command "
                                                         set TmpReturn 0
                                                         destroy .confirm"
  if { $thirdbutton != "" } {
    .confirm.frm1 add source -image zip -font $EditPrefs(windowfont) -command "
                                                         set TmpReturn 2
                                                         destroy .confirm"
  }

  CniBalloon [.confirm.frm1 subwidget ok] $yesbutton $EditPrefs(windowfont) 500
  CniBalloon [.confirm.frm1 subwidget cancel] $nobutton $EditPrefs(windowfont) 500

  if { $thirdbutton != "" } {
    CniBalloon [.confirm.frm1 subwidget source] $thirdbutton $EditPrefs(windowfont) 500
  }

  DefaultButton [.confirm.frm1 subwidget ok] .confirm

  pack .confirm.frm.lbim -side left -padx 20 -anchor w
  pack .confirm.frm.lbt -side left -padx 20 -anchor w
  pack .confirm.frm -pady 15 -anchor c -side top

  pack .confirm.frm1 -side top -fill x


  wm transient .confirm $MainW
  PosizionaFinestre $MainW .confirm c

  $old_focus config -cursor pirate
  [winfo toplevel $old_focus] config -cursor pirate

  focus .confirm

  bind .confirm <Enter> {
    .confirm config -cursor left_ptr
  }

  bind .confirm <Leave> [format {
    %s config -cursor pirate
    [winfo toplevel %s] config -cursor pirate
  } $old_focus $old_focus] 

  bind .confirm <Key-Escape> { [.confirm.frm1 subwidget cancel] invoke }

  grab .confirm
  tkwait window .confirm

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }

  $old_focus config -cursor left_ptr
  [winfo toplevel $old_focus] config -cursor left_ptr
  return $TmpReturn
}

####################################################################
# Procedura CommentBox
# Definizione di un box per l'input di un commento
# Possono essere assegnate le seguenti opzioni:
#   -text (default "Comment :"
#   -default (default "")
#   -cancelvalue (default "")
#   -title (default "COMMENT")
#
# Input: 
#   args    Opzioni
# Ritorna :
#   Il commento
####################################################################
proc CommentBox { args } {
global EditPrefs TmpReturn

  ParseArgs {
    {text "Comment :"}
    {default ""}
    {cancelvalue ""}
    {title Prompt}
  }
  

  set text [GetString $text]
  set title [GetString $title]
  catch {unset TmpReturn}
  
  set old_focus [focus]
  if { $old_focus == "" } {
    set old_focus .jedit0.main.t
  }
  set MainW [winfo toplevel $old_focus]

  if [winfo exists .pr] {
    return
  }

  grab release [grab current]

  toplevel .pr
  wm title .pr $title
  wm protocol .pr WM_DELETE_WINDOW procx
  
  message .pr.msg -width 300 -anchor w -text $text
  text .pr.field -width $EditPrefs(textwidth) -height 5
  tixButtonBox .pr.b -orientation horizontal
  .pr.b add ok -image ok -font $EditPrefs(windowfont)  -command {
                                   global TmpReturn
                                   set TmpReturn [.pr.field get 1.0 end]
                                   destroy .pr} 
  .pr.b add cancel -image cancel -font $EditPrefs(windowfont) -command {
                                   global TmpReturn
                                   set TmpReturn {}
                                   destroy .pr}

  if [ info exists EditPrefs(windowfont) ] {
    .pr.msg configure -font $EditPrefs(messagefont)
    .pr.field configure -font $EditPrefs(textfont)
  }

  pack .pr.msg -side top -fill both -expand yes -padx 10
  pack .pr.field -side top -padx 10 -pady 10
  pack .pr.b -side bottom -fill x

  .pr.field delete 1.0 end
  .pr.field insert end $default
  
  CniBalloon [.pr.b subwidget ok] [GetString OK] $EditPrefs(windowfont) 500
  CniBalloon [.pr.b subwidget cancel] [GetString CANCEL] \
        $EditPrefs(windowfont) 500

  wm transient .pr $MainW
  PosizionaFinestre $MainW .pr c

  focus .pr.field

  $old_focus config -cursor pirate
  [winfo toplevel $old_focus] config -cursor pirate

  bind .pr <Enter> {
    .pr config -cursor left_ptr
  }

  bind .pr <Leave> [format {
    %s config -cursor pirate
    [winfo toplevel %s] config -cursor pirate
  } $old_focus $old_focus] 

  bind .pr <Key-Escape> { [.pr.b subwidget cancel] invoke}

  grab .pr
  tkwait window .pr

  if [winfo exists $old_focus] {
    focus -force $old_focus
  }

  $old_focus config -cursor left_ptr
  [winfo toplevel $old_focus] config -cursor left_ptr
  set commento ""
  foreach line [split $TmpReturn "\n"] {
    if { $line != "" } {
      if { [string first "(*" $line ] == -1 } {
        append commento "(* $line"
      } else {
        append commento $line
      }
      if { [string first "*)" $line ] == -1 } {
        append commento " *)" \n
      } else {
        append commento \n
      }
    }
  } 
  set TmpReturn $commento
  return $TmpReturn
}

