#####################################################
# Cancellazione di un file di programma
######################################################
proc CancellaFile { CalWdg PathLab DataLab ListD ListF} {

  global FileLingua

  set FileCanc [prompt $CalWdg [CaricaLingua $FileLingua Cancella_file]] 
  if {[catch "file delete [pwd]/$FileCanc" out] == 0} {
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]
  } else {
    MessageBox $CalWdg "[CaricaLingua $FileLingua ErroreFileDel] $FileCanc"  yes
  }
}

#####################################################
# Creazione di un direttorio 
######################################################
proc CreaDirettorio { CalWdg PathLab DataLab ListD ListF} {

  global FileLingua

  set DirCreate [prompt $CalWdg [CaricaLingua $FileLingua Crea_direttorio]] 
  if {[catch "file mkdir [pwd]/$DirCreate" out] == 0} {
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]
  } else {
    MessageBox $CalWdg "[CaricaLingua $FileLingua ErroreDirCrea] $DirCreate"  yes
  }
}

#####################################################
# Creazione di un direttorio 
######################################################
proc CancellaDirettorio { CalWdg PathLab DataLab ListD ListF} {

  global FileLingua

  set DirDel [prompt $CalWdg [CaricaLingua $FileLingua Cancella_direttorio]] 
  if {[catch "file delete -force [pwd]/$DirDel" out] == 0} {
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]
  } else {
    MessageBox $CalWdg "[CaricaLingua $FileLingua ErroreDirDel] $DirDel"  yes
  }
}

######################################################################
# Caricamento di un file di programma
######################################################################
proc LoadFile { PathLab DataLab ListD ListF } {

  global ProgPath NomeFile

  $ListD delete 0 end
  $ListF delete 0 end

  update idletasks

  $PathLab config -text "/[string trimleft [pwd] $ProgPath]"
  if { [pwd] != $ProgPath } {
    $ListD insert end ".."
  }

  # add any directory
  foreach i [lsort [glob -nocomplain *]] {
    if {$i != "." && $i != ".."} {
      if {[file isdirectory ./$i]} {
        $ListD insert end "$i"
      } else {
        $ListF insert end "$i"
      }
    }
  }

  # add any dot-files:
  foreach i [lsort "[glob -nocomplain *.plc]"] {
    if {$i != "." && $i != ".."} {
      if {[file isdirectory ./$i]} {
        $ListD insert end "$i"
      } else {
        $ListF insert end "$i"
      }
    }
  }
}

######################################
#   DefinisciData
######################################
proc DefinisciData { w } {
  global NomeFile

  $w config -text "$NomeFile - [file size ./$NomeFile] - [clock format [file mtime ./$NomeFile] -format {%T %d/%m/%y}]"
}

#################################
#   Gestione Pressione escape	#
#################################
proc EscPress {} {
  global NomeFile
set NomeFile {}
destroy .specchio
}

###################################################
#    FileSelection
###################################################
proc FileSelection { MainW title} {
  global Lingua font_usato FileLingua ProgPath
  global NomeFile
  global file_sel1_image file_sel2_image

  #sporco trucco per il Doppio click su file
  catch {destroy .specchio}
  toplevel .specchio
  wm withdraw .specchio

  set w .fs
  if [winfo exist $w] {
    return
  }
  toplevel $w 

  wm title $w [CaricaLingua $FileLingua $title]
  

  set NomeFile {}
  #
  #  Creazione della MENUBAR principale
  #
  frame $w.top 
  frame $w.top.menubar -relief flat -bd 2 

  menubutton $w.top.menubar.mb1 -text [CaricaLingua $FileLingua Servizi] -anchor w -menu $w.top.menubar.mb1.file \
                              -font $font_usato -underline 0 -relief raised 
  set file [menu $w.top.menubar.mb1.file -tearoff 0 ]
  $file add command -label [CaricaLingua $FileLingua Cancella_file] -command "CancellaFile $w \
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file" \
                              -font $font_usato 
  $file add separator
  $file add command -label [CaricaLingua $FileLingua Crea_direttorio] -command "CreaDirettorio $w \
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file" \
                              -font $font_usato 
  $file add command -label [CaricaLingua $FileLingua Cancella_direttorio] -command "CancellaDirettorio $w \
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file" \
                              -font $font_usato 

  label $w.top.menubar.label1 -image $file_sel1_image -relief flat 
  label $w.top.menubar.label2 -image $file_sel2_image -relief flat 
  pack $w.top.menubar.label1 -in $w.top.menubar -side left -anchor nw -padx 3
  pack $w.top.menubar.mb1 -in $w.top.menubar -side left -expand yes -fill x 
  pack $w.top.menubar.label2 -in $w.top.menubar -side right -anchor ne -padx 3

  #
  #  Creazione delle due scrolled list
  #

  label $w.top.path -text "/" -font $font_usato 

  frame $w.top.scrolled 
  tixScrolledListBox $w.top.scrolled.dir -scrollbar y -options [format {
	listbox.font %s
    } $font_usato]

  tixScrolledListBox $w.top.scrolled.file -scrollbar y -options [format {
	listbox.font %s
    } $font_usato]
  pack $w.top.scrolled.dir -in $w.top.scrolled -side left -expand true -fill both 
  pack $w.top.scrolled.file -in $w.top.scrolled -side right -expand true -fill both


  entry $w.top.filename -font $font_usato -textvariable NomeFile 
  label $w.top.filedata -font $font_usato 

  tixButtonBox $w.box -orientation horizontal
  $w.box add ok     -text [CaricaLingua $FileLingua OK]  -font $font_usato  -command "destroy .specchio" 
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command "
                                                             set NomeFile {}
                                                             destroy .specchio" 

  pack $w.top.menubar -in $w.top -side top -fill x -expand true -anchor n
  pack $w.top.path -in $w.top -side top -anchor nw -pady 2 
  pack $w.top.scrolled -in $w.top -side top -fill both -expand true 
  pack $w.top.filename -in $w.top -side top -anchor w -pady 5 
  pack $w.top.filedata -in $w.top -side top -pady 5 

  pack $w.top -side top -padx 10 -pady 10 -expand true -fill both -anchor n 
  pack $w.box -side bottom -fill x -anchor n

  wm transient $w

  posiziona:finestre $MainW $w

  set oldPwd [pwd]
  cd $ProgPath
  LoadFile $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]

  bind [$w.top.scrolled.file subwidget listbox] <ButtonRelease-1> [format {
    set NomeFile [selection get]
    DefinisciData %s
  } $w.top.filedata]

  bind [$w.top.scrolled.file subwidget listbox] <KeyRelease> [format {
    set SelezioneCorrente [%s curselection]
    if { $SelezioneCorrente != "" } {
      set NomeFile [%s get $SelezioneCorrente]
      DefinisciData %s
    }
  } [$w.top.scrolled.file subwidget listbox] [$w.top.scrolled.file subwidget listbox]  $w.top.filedata]

# bind [$w.top.scrolled.file subwidget listbox] <Double-1> [format {
#	set NomeFile [selection get]
#	DefinisciData %s
#	destroy %s
# } $w.top.filedata .specchio] 

  bind [$w.top.scrolled.dir subwidget listbox] <Double-1> [format {
    set Direttorio [selection get]
    if { $Direttorio != ".." } {
      cd $Direttorio
      LoadFile %s %s %s %s
    } elseif { [pwd] != $ProgPath } {
      cd ..
      LoadFile %s %s %s %s
    } 
  } $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox] \
    $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]] 

  bind $w.top.filename <Return> [format {
    destroy %s
  } .specchio] 

  bind $w.top.filename <KeyRelease> {
    set NomeFile [string toupper $NomeFile]
  }

  bind $w.top.filename <FocusIn> {
      %W select range 0 end
  }

  bind $w <Escape> " set NomeFile {}
                   destroy .specchio" 

  wm protocol $w WM_DELETE_WINDOW EscPress
  grab $w
  focus $w.top.filename
  tkwait window .specchio
  destroy $w

  set NomeFileCompleto "[pwd]/$NomeFile"
  cd $oldPwd
  if { $NomeFile != "" } {
    return $NomeFileCompleto
  } else {
   return 0
  }
}
