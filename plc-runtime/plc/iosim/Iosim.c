
/* This file is created by the "tcl2c" utility, which is included in
 * most "plus"-patches (e.g. for Tcl7.6 and Tcl8.0). Standalone
 * executables can be made by providing alternative initialization
 * functions which don't read files any more. Sometimes, small
 * adaptations to the original libraries are needed to get the
 * application truly standalone. The "plus"-patches contain these
 * adaptations for Tcl and Tk. If you just create your own
 * Xxx_InitStandAlone() function for your package, you can
 * easyly extend the "tcl2c" utility to your own requirements.
 *
 *	Jan Nijtmans
 *	CMG (Computer Management Group) Arnhem B.V.
 *	email: nijtmans@worldaccess.nl (private)
 *	       Jan.Nijtmans@cmg.nl (work)
 *	url:   http://www.worldaccess.nl/~nijtmans/
 */
#include "tcl.h"
#ifdef __WIN32__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#include <malloc.h>
#include <locale.h>

static int setargv _ANSI_ARGS_((char ***argvPtr));
static void TclshPanic _ANSI_ARGS_(TCL_VARARGS(char *,format));
#ifndef CNIDLL
extern void TclWinSetTclInstance(HINSTANCE instance);
#endif

#endif

/*
 * Defines to replace the standard Xxx_Init calls to Xxx_InitStandAlone.
 * If you don't have this function, just delete the corresponding
 * define such that the normal initialization function is used.
 * Similar: If SafeInit functions exists, you can use these
 * by commenting out the corresponding lines below.
 */

#if defined(TCL_ACTIVE) && !defined(SHARED)
#define Tcl_Init Tcl_InitStandAlone
#define Tk_Init Tk_InitStandAlone
#define Tix_Init Tix_InitStandAlone
#define Tix_InitStandAlone Tixsam_Init
#else
#define Tix_Init Tixsam_Init
#endif

#if TCL_MAJOR_VERSION < 8
#define Tk_SafeInit (Tcl_PackageInitProc *) NULL
#define Tix_SafeInit (Tcl_PackageInitProc *) NULL
#endif

/*
 * Prototypes of all initialization functions and the free() function.
 * So, only "tcl.h" needs to be included now.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USE_TCLALLOC
#   define USE_TCLALLOC 0
#endif
#if USE_TCLALLOC == 0
extern void free _ANSI_ARGS_((void *));
#endif
extern int  Tcl_Init _ANSI_ARGS_((Tcl_Interp *interp));


extern void Tk_MainLoop _ANSI_ARGS_((void));
#define HAS_TK
#ifdef __WIN32__
extern void TkWinXInit _ANSI_ARGS_((HINSTANCE hinstance));
extern void TkWinXCleanup _ANSI_ARGS_((HINSTANCE hinstance));
#endif

extern int  Tk_Init _ANSI_ARGS_((Tcl_Interp *interp));
#ifndef Tk_SafeInit
extern int  Tk_SafeInit _ANSI_ARGS_((Tcl_Interp *interp));
#endif
extern int  Tix_Init _ANSI_ARGS_((Tcl_Interp *interp));
#ifndef Tix_SafeInit
extern int  Tix_SafeInit _ANSI_ARGS_((Tcl_Interp *interp));
#endif


#ifdef __cplusplus
}
#endif

/*
 * The array "script" contains the script that is compiled in.
 * It will be executed in tclAppInit() after the other initializations.
 */

static char *lineformat = "%.0s%d";
static int line = (__LINE__ + 1);
static char *script[] = {
"proc CancellaFile { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set FileCanc [prompt $CalWdg [CaricaLingua $FileLingua Cancella_file]] \n\
  if {[catch \"file delete [pwd]/$FileCanc\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreFileDel] $FileCanc\"  yes\n\
  }\n\
}",
"proc CreaDirettorio { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set DirCreate [prompt $CalWdg [CaricaLingua $FileLingua Crea_direttorio]] \n\
  if {[catch \"file mkdir [pwd]/$DirCreate\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreDirCrea] $DirCreate\"  yes\n\
  }\n\
}",
"proc CancellaDirettorio { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set DirDel [prompt $CalWdg [CaricaLingua $FileLingua Cancella_direttorio]] \n\
  if {[catch \"file delete -force [pwd]/$DirDel\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreDirDel] $DirDel\"  yes\n\
  }\n\
}",
"proc LoadFile { PathLab DataLab ListD ListF } {\n\
\n\
  global ProgPath NomeFile\n\
\n\
  $ListD delete 0 end\n\
  $ListF delete 0 end\n\
\n\
  update idletasks\n\
\n\
  $PathLab config -text \"/[string trimleft [pwd] $ProgPath]\"\n\
  if { [pwd] != $ProgPath } {\n\
    $ListD insert end \"..\"\n\
  }\n\
\n\
  # add any directory\n\
  foreach i [lsort [glob -nocomplain *]] {\n\
    if {$i != \".\" && $i != \"..\"} {\n\
      if {[file isdirectory ./$i]} {\n\
        $ListD insert end \"$i\"\n\
      } else {\n\
        $ListF insert end \"$i\"\n\
      }\n\
    }\n\
  }\n\
\n\
  # add any dot-files:\n\
  foreach i [lsort \"[glob -nocomplain *.plc]\"] {\n\
    if {$i != \".\" && $i != \"..\"} {\n\
      if {[file isdirectory ./$i]} {\n\
        $ListD insert end \"$i\"\n\
      } else {\n\
        $ListF insert end \"$i\"\n\
      }\n\
    }\n\
  }\n\
}",
"proc DefinisciData { w } {\n\
  global NomeFile\n\
\n\
  $w config -text \"$NomeFile - [file size ./$NomeFile] - [clock format [file mtime ./$NomeFile] -format {%T %d/%m/%y}]\"\n\
}",
"proc EscPress {} {\n\
  global NomeFile\n\
set NomeFile {}\n\
destroy .specchio\n\
}",
"proc FileSelection { MainW title} {\n\
  global Lingua font_usato FileLingua ProgPath\n\
  global NomeFile\n\
  global file_sel1_image file_sel2_image\n\
\n\
  #sporco trucco per il Doppio click su file\n\
  catch {destroy .specchio}\n\
  toplevel .specchio\n\
  wm withdraw .specchio\n\
\n\
  set w .fs\n\
  if [winfo exist $w] {\n\
    return\n\
  }\n\
  toplevel $w \n\
\n\
  wm title $w [CaricaLingua $FileLingua $title]\n\
  \n\
\n\
  set NomeFile {}\n\
  #\n\
  #  Creazione della MENUBAR principale\n\
  #\n\
  frame $w.top \n\
  frame $w.top.menubar -relief flat -bd 2 \n\
\n\
  menubutton $w.top.menubar.mb1 -text [CaricaLingua $FileLingua Servizi] -anchor w -menu $w.top.menubar.mb1.file \\\n\
                              -font $font_usato -underline 0 -relief raised \n\
  set file [menu $w.top.menubar.mb1.file -tearoff 0 ]\n\
  $file add command -label [CaricaLingua $FileLingua Cancella_file] -command \"CancellaFile $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
  $file add separator\n\
  $file add command -label [CaricaLingua $FileLingua Crea_direttorio] -command \"CreaDirettorio $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
  $file add command -label [CaricaLingua $FileLingua Cancella_direttorio] -command \"CancellaDirettorio $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
\n\
  label $w.top.menubar.label1 -image $file_sel1_image -relief flat \n\
  label $w.top.menubar.label2 -image $file_sel2_image -relief flat \n\
  pack $w.top.menubar.label1 -in $w.top.menubar -side left -anchor nw -padx 3\n\
  pack $w.top.menubar.mb1 -in $w.top.menubar -side left -expand yes -fill x \n\
  pack $w.top.menubar.label2 -in $w.top.menubar -side right -anchor ne -padx 3\n\
\n\
  #\n\
  #  Creazione delle due scrolled list\n\
  #\n\
\n\
  label $w.top.path -text \"/\" -font $font_usato \n\
\n\
  frame $w.top.scrolled \n\
  tixScrolledListBox $w.top.scrolled.dir -scrollbar y -options [format {\n\
	listbox.font %s\n\
    } $font_usato]\n\
\n\
  tixScrolledListBox $w.top.scrolled.file -scrollbar y -options [format {\n\
	listbox.font %s\n\
    } $font_usato]\n\
  pack $w.top.scrolled.dir -in $w.top.scrolled -side left -expand true -fill both \n\
  pack $w.top.scrolled.file -in $w.top.scrolled -side right -expand true -fill both\n\
\n\
\n\
  entry $w.top.filename -font $font_usato -textvariable NomeFile \n\
  label $w.top.filedata -font $font_usato \n\
\n\
  tixButtonBox $w.box -orientation horizontal\n\
  $w.box add ok     -text [CaricaLingua $FileLingua OK]  -font $font_usato  -command \"destroy .specchio\" \n\
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command \"\n\
                                                             set NomeFile {}\n\
                                                             destroy .specchio\" \n\
\n\
  pack $w.top.menubar -in $w.top -side top -fill x -expand true -anchor n\n\
  pack $w.top.path -in $w.top -side top -anchor nw -pady 2 \n\
  pack $w.top.scrolled -in $w.top -side top -fill both -expand true \n\
  pack $w.top.filename -in $w.top -side top -anchor w -pady 5 \n\
  pack $w.top.filedata -in $w.top -side top -pady 5 \n\
\n\
  pack $w.top -side top -padx 10 -pady 10 -expand true -fill both -anchor n \n\
  pack $w.box -side bottom -fill x -anchor n\n\
\n\
  wm transient $w\n\
\n\
  posiziona:finestre $MainW $w\n\
\n\
  set oldPwd [pwd]\n\
  cd $ProgPath\n\
  LoadFile $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]\n\
\n\
  bind [$w.top.scrolled.file subwidget listbox] <ButtonRelease-1> [format {\n\
    set NomeFile [selection get]\n\
    DefinisciData %s\n\
  } $w.top.filedata]\n\
\n\
  bind [$w.top.scrolled.file subwidget listbox] <KeyRelease> [format {\n\
    set SelezioneCorrente [%s curselection]\n\
    if { $SelezioneCorrente != \"\" } {\n\
      set NomeFile [%s get $SelezioneCorrente]\n\
      DefinisciData %s\n\
    }\n\
  } [$w.top.scrolled.file subwidget listbox] [$w.top.scrolled.file subwidget listbox]  $w.top.filedata]\n\
\n\
\n\
  bind [$w.top.scrolled.dir subwidget listbox] <Double-1> [format {\n\
    set Direttorio [selection get]\n\
    if { $Direttorio != \"..\" } {\n\
      cd $Direttorio\n\
      LoadFile %s %s %s %s\n\
    } elseif { [pwd] != $ProgPath } {\n\
      cd ..\n\
      LoadFile %s %s %s %s\n\
    } \n\
  } $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox] \\\n\
    $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]] \n\
\n\
  bind $w.top.filename <Return> [format {\n\
    destroy %s\n\
  } .specchio] \n\
\n\
  bind $w.top.filename <KeyRelease> {\n\
    set NomeFile [string toupper $NomeFile]\n\
  }\n\
\n\
  bind $w.top.filename <FocusIn> {\n\
      %W select range 0 end\n\
  }\n\
\n\
  bind $w <Escape> \" set NomeFile {}\n\
                   destroy .specchio\" \n\
\n\
  wm protocol $w WM_DELETE_WINDOW EscPress\n\
  grab $w\n\
  focus $w.top.filename\n\
  tkwait window .specchio\n\
  destroy $w\n\
\n\
  set NomeFileCompleto \"[pwd]/$NomeFile\"\n\
  cd $oldPwd\n\
  if { $NomeFile != \"\" } {\n\
    return $NomeFileCompleto\n\
  } else {\n\
   return 0\n\
  }\n\
}",
"proc    Reload { w } {\n\
global Matrice font_usato listbox Nomi TipoIO Lunghezza\n\
global FileLingua IconaInput IconaOutput Congela\n\
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
global BottoneStart BottoneStop BottoneReload BottoneAdd\n\
\n\
if {$Congela == \"True\"} {\n\
	return\n\
	}\n\
\n\
$BottoneStart configure -state disabled\n\
$BottoneStop configure -state disabled\n\
$BottoneAdd configure -state disabled\n\
$BottoneReload configure -state disabled\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled\n\
$BottoneSalvaCome(widget) entryconfigure\\\n\
			$BottoneSalvaCome(index) -state disabled\n\
\n\
\n\
$w config -cursor watch\n\
update idletasks\n\
\n\
if [array exists Nomi] {\n\
	unset Nomi \n\
	}\n\
GetLista\n\
\n\
$listbox delete 0 end\n\
update idletasks\n\
\n\
foreach var [lsort [array names Nomi]] {\n\
	if { $var != \"\" && [info exist TipoIO($var) ] } {\n\
		if { $TipoIO($var) == 1 } {\n\
			# INPUT\n\
			$listbox insert end -itemtype imagetext -text \"$var\" -image $IconaInput\n\
			} elseif { $TipoIO($var) == 2 } {\n\
			# OUTPUT\n\
			$listbox insert end -itemtype imagetext -text \"$var\" -image $IconaOutput\n\
			}\n\
		}\n\
	}\n\
$w config -cursor arrow\n\
$BottoneStart configure -state active\n\
$BottoneStop configure -state disabled\n\
$BottoneAdd configure -state active\n\
$BottoneReload configure -state active\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state active\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state active\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state active\n\
$BottoneSalvaCome(widget) entryconfigure\\\n\
			$BottoneSalvaCome(index) -state active\n\
}",
"proc    Aggiungi { w } {\n\
global Matrice font_usato listbox\n\
global FileLingua Congela\n\
\n\
if {$Congela == \"True\"} {\n\
	return\n\
	}\n\
\n\
if { [ catch { set selezionato [$listbox info selection ] } out ] != 0 } {\n\
	return\n\
	}\n\
foreach linea [split $selezionato \" \"] {\n\
	if { $linea != \"\" } {\n\
     		InserisceVar [$listbox entrycget $linea -text] toggle\n\
		}\n\
	}\n\
}",
"proc    Stop { w } {\n\
global Matrice font_usato listbox BottoneAdd BottoneReload\n\
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
global FileLingua Start BottoneStart BottoneStop IconaStart\n\
\n\
set Start 0\n\
$BottoneStart configure -state active\n\
$BottoneStop configure -state disabled\n\
$BottoneAdd configure -state active\n\
$BottoneReload configure -state active\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state active\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state active\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state active\n\
$BottoneSalvaCome(widget) entryconfigure $BottoneSalvaCome(index)\\\n\
								 -state active\n\
}",
"proc    InserisceVar { NomeVariabile tred } {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global FileLingua ArrayDebugLinee\n\
global HOME\n\
\n\
if [info exist ArrayDebugLinee($NomeVariabile) ] {\n\
	return\n\
	}\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
upvar \\#0 $Matrice(copypage) copyary\n\
\n\
set linee [expr [lindex [$Matrice(widget) configure -rows] 4] -1]\n\
\n\
set ArrayDebugLinee($NomeVariabile) [expr $Matrice(linee) + 1]\n\
if { $linee <= $Matrice(linee) } {\n\
	$Matrice(widget) insert rows $Matrice(linee) 1\n\
	update idletasks\n\
	}\n\
\n\
set ary([expr $Matrice(linee) +1],0) $NomeVariabile\n\
set copyary([expr $Matrice(linee) +1],0) $NomeVariabile\n\
set copyary([expr $Matrice(linee) +1],1) \"\"\n\
InserisceValore [expr $Matrice(linee) +1] $tred\n\
\n\
incr Matrice(linee)\n\
$Matrice(widget) see $Matrice(linee),0\n\
update idletasks\n\
$Matrice(widget) tag col sinistro-ro 0\n\
$Matrice(widget) tag col centro-ro 1\n\
}",
"proc    InserisceVareVal { NomeVariabile Valore tred } {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global FileLingua ArrayDebugLinee\n\
global HOME\n\
\n\
if [info exist ArrayDebugLinee($NomeVariabile) ] {\n\
	return\n\
	}\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
upvar \\#0 $Matrice(copypage) copyary\n\
\n\
set linee [expr [lindex [$Matrice(widget) configure -rows] 4] -1]\n\
\n\
set ArrayDebugLinee($NomeVariabile) [expr $Matrice(linee) + 1]\n\
if { $linee <= $Matrice(linee) } {\n\
	$Matrice(widget) insert rows $Matrice(linee) 1\n\
	update idletasks\n\
	}\n\
\n\
set copyary([expr $Matrice(linee) +1],0) $NomeVariabile\n\
set copyary([expr $Matrice(linee) +1],1) \"\"\n\
set ary([expr $Matrice(linee) +1],0) $NomeVariabile\n\
SetMatrice $NomeVariabile $TipoIO($NomeVariabile) $Lunghezza($NomeVariabile)\\\n\
		[expr $Matrice(linee) +1] $Valore\n\
\n\
incr Matrice(linee)\n\
$Matrice(widget) see $Matrice(linee),0\n\
update idletasks\n\
$Matrice(widget) tag col sinistro-ro 0\n\
$Matrice(widget) tag col centro-ro 1\n\
}",
"proc    InserisceValore { linea tred } {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global FileLingua\n\
global HOME\n\
\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
set NomeVariabile $ary($linea,0)\n\
\n\
if { $TipoIO($NomeVariabile) != 1 && $TipoIO($NomeVariabile) != 2 } {\n\
	MessageBox $Matrice(toplevel)\\\n\
	 \"[CaricaLingua $FileLingua ErroreTipoSconosciuto] $NomeVariabile\" yes\n\
	return\n\
	}\n\
SetMatrice $NomeVariabile $TipoIO($NomeVariabile) $Lunghezza($NomeVariabile)\\\n\
	$linea [Getstate $NomeVariabile $tred ]\n\
\n\
}",
"proc    StartAggiornamento { w } {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
global FileLingua BottoneStart BottoneStop BottoneAdd BottoneReload\n\
global HOME Start IconaStop Congela\n\
\n\
if {$Congela == \"True\"} {\n\
	return\n\
	}\n\
\n\
$BottoneStart configure -state disabled\n\
$BottoneStop configure -state active\n\
$BottoneAdd configure -state disabled\n\
$BottoneReload configure -state disabled\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled\n\
$BottoneSalvaCome(widget) entryconfigure $BottoneSalvaCome(index)\\\n\
								 -state disabled\n\
\n\
\n\
for {set i 1} { $i <= $Matrice(linee) } { incr i } {\n\
	InserisceValore $i delay\n\
	}\n\
set Start 1\n\
Aggiornamento\n\
}",
"proc    Aggiornamento {} {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global FileLingua\n\
global HOME Start\n\
set y1 \\\n\
   [expr round([lindex [$Matrice(widget) yview] 0] * $Matrice(linee) + 1) ] \n\
set y2 \\\n\
   [expr round([lindex [$Matrice(widget) yview] 1] * $Matrice(linee) ) ] \n\
\n\
if { $Start == 1 } {\n\
	for {set i $y1} { $i <= $y2  } { incr i } {\n\
		InserisceValore $i delay\n\
		}\n\
	update idletasks\n\
	after 500 Aggiornamento\n\
	}\n\
}",
"proc    ToggleValore { line } {\n\
\n\
global Matrice TipoIO Lunghezza\n\
global HOME FileLingua\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
set NomeVariabile $ary($line,0)\n\
\n\
if { [Getstate $NomeVariabile toggle] == 0 } {\n\
	# e' zero ... lo setto setin <key> <val>\n\
	if { [Setstate $NomeVariabile 1 toggle] != \"OK\" } {\n\
		MessageBox $Matrice(toplevel)\\\n\
		 \"[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile\"  yes\n\
		return\n\
		}\n\
	while { [Getstate $NomeVariabile toggle] != 1 } {\n\
		}\n\
   } else {	\n\
	# e' uno ... lo resetto\n\
	if { [Setstate $NomeVariabile 0 toggle] != \"OK\" } {\n\
		MessageBox $Matrice(toplevel)\\\n\
		 \"[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile\"  yes\n\
		return\n\
		}\n\
	while { [Getstate $NomeVariabile toggle] != 0 } {\n\
		}\n\
   }\n\
InserisceValore $line toggle\n\
}",
"proc    DeleteVar { NomeVariabile } {\n\
\n\
global Matrice font_usato listbox TipoIO Lunghezza\n\
global FileLingua ArrayDebugLinee\n\
global HOME\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
\n\
if { ! [info exist ArrayDebugLinee($NomeVariabile) ] } {\n\
	return\n\
	}\n\
\n\
$Matrice(widget) delete rows $ArrayDebugLinee($NomeVariabile) 1\n\
set Matrice(linee) [expr $Matrice(linee) - 1]\n\
\n\
update idletasks\n\
$Matrice(widget) flush\n\
update idletasks\n\
\n\
for {set i [expr $ArrayDebugLinee($NomeVariabile) ]} \\\n\
		 		{$i <= $Matrice(linee)} { incr i } {\n\
	set ArrayDebugLinee($ary($i,0)) $i\n\
	}\n\
\n\
unset ArrayDebugLinee($NomeVariabile)\n\
return\n\
}",
"proc    Cancella { w } {\n\
global Matrice font_usato\n\
global FileLingua ArrayDebugLinee\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
for {set i 1} {$i <= $Matrice(linee)} { incr i } {\n\
	unset ArrayDebugLinee($ary($i,0))\n\
        }\n\
\n\
$Matrice(widget) delete rows 1 $Matrice(linee)\n\
set Matrice(linee) [expr $Matrice(linee) - 1]\n\
\n\
set Matrice(linee) 0\n\
$Matrice(widget) flush\n\
update idletasks\n\
}",
"proc    SwitchHost { padre } {\n\
\n\
wm withdraw $padre\n\
global FileLingua font_usato font_fisso tcl_platform Host okHost\n\
\n\
set w [toplevel .host]\n\
wm title $w \"[CaricaLingua $FileLingua SelezionaHost]\"\n\
\n\
frame $w.top -border 1 -relief raised\n\
\n\
set Host localhost\n\
tixLabelEntry $w.top.a -label \"Host: \" \\\n\
	-options {\n\
	    entry.width 30\n\
	    entry.textVariable Host\n\
	    label.anchor e\n\
	    entry.anchor o\n\
	}\n\
bind [$w.top.a subwidget entry]  <Return>  \"[format \"okcmd %s\" $w]\"\n\
\n\
pack $w.top.a -side top -fill x -expand yes\n\
tixButtonBox $w.box -orientation horizontal\n\
$w.box add ok -text Ok -underline 0 -command \"[format \"okcmd %s\" $w]\" -width 6\n\
$w.box add exit     -text Exit     -underline 0 -command \"exit\" -width 6\n\
\n\
pack $w.box -side bottom -fill x\n\
pack $w.top -side top -fill both -expand yes\n\
\n\
focus $w.top.a\n\
\n\
tkwait variable okHost\n\
destroy $w\n\
update idletasks\n\
wm deiconify $padre\n\
return \"OK\"\n\
}",
"proc okcmd { w } {\n\
global	okHost Host FileLingua\n\
if { [VerificaEsistenzaPlc] == 0 } {\n\
	set okHost yes\n\
	} else {\n\
	MessageBox $w  \"$Host: [CaricaLingua $FileLingua PlcAssente]\"  yes\n\
	}\n\
}",
"proc    windows { w } {\n\
\n\
global FileLingua font_usato Matrice font_fisso SAVEPATH\n\
global tcl_platform listbox BottoneStart BottoneStop BottoneReload BottoneAdd\n\
global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
\n\
global HOME FrecciaDestra IconaExit IconaStart IconaStop IconaReload\n\
\n\
frame $w.top\n\
frame $w.top.menubar -relief raised -bd 0\n\
\n\
menubutton $w.top.menubar.mb1 -text [CaricaLingua $FileLingua File]\\\n\
		-menu $w.top.menubar.mb1.file -font $font_usato -underline 0 \n\
\n\
set file [menu $w.top.menubar.mb1.file -tearoff 0 ]\n\
\n\
$file add command -label [CaricaLingua $FileLingua Apri]\\\n\
		-command [format {Apri %s} $w] -font $font_usato\\\n\
		-underline 0\n\
set BottoneApri(widget) $file\n\
set BottoneApri(index)  0\n\
$file add command -label [CaricaLingua $FileLingua Salva]\\\n\
		-command [format {Salva %s} $w] -font $font_usato\\\n\
		-underline 0\n\
set BottoneSalva(widget) $file\n\
set BottoneSalva(index)  1\n\
$file add command -label [CaricaLingua $FileLingua Salvacome]\\\n\
		-command [format {SaveCome %s} $w] -font $font_usato\\\n\
		-underline 0\n\
set BottoneSalvaCome(widget) $file\n\
set BottoneSalvaCome(index)  2\n\
$file add separator\n\
$file add command -label [CaricaLingua $FileLingua Uscita]\\\n\
 		-command [format {Uscita %s} $w] -font $font_usato\\\n\
		-underline 2\n\
\n\
menubutton $w.top.menubar.mb2 -text [CaricaLingua $FileLingua Edit]\\\n\
		-menu $w.top.menubar.mb2.edit -font $font_usato -underline 0\n\
\n\
set edit [menu $w.top.menubar.mb2.edit -tearoff 0 ]\n\
$edit add command -label \"Cancella tutto\"\\\n\
 		-command [format {Cancella %s} $w] -font $font_usato\\\n\
		-underline 2\n\
set BottoneCancella(widget) $edit\n\
set BottoneCancella(index)  0\n\
\n\
pack $w.top.menubar -in $w.top -side left -fill x -expand true -anchor n\n\
pack $w.top.menubar.mb1 -side left -fill x\n\
pack $w.top.menubar.mb2 -side left -fill x\n\
pack $w.top -side top -fill x \n\
\n\
\n\
\n\
label $w.statusbar\n\
pack $w.statusbar -side bottom -fill x -padx 3 -pady 1\n\
tixBalloon $w.ballon -statusbar $w.statusbar\n\
\n\
set Matrice(NomeFile) \"$SAVEPATH/NONAME.SIM\"\n\
set BoxNomeConf [frame $w.frame2 -bd 1 -relief ridge]\n\
label $BoxNomeConf.lab -anchor w \\\n\
	 -text \"[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)\"\n\
set Matrice(WidgetNomeFile) $BoxNomeConf.lab\n\
pack $BoxNomeConf -side top -fill x -padx 2 -pady 2\n\
pack $BoxNomeConf.lab -fill x -side top -padx 2 -pady 2\n\
\n\
set ToolBox [frame $w.frame1 -bd 1 -relief ridge]\n\
\n\
set BottoneReload [button $ToolBox.iconareload -image $IconaReload\\\n\
				-command [format {Reload %s} $w] ]\n\
pack $ToolBox.iconareload -in $ToolBox -side left\n\
\n\
set BottoneAdd [button $ToolBox.frecciadestra -image $FrecciaDestra\\\n\
				-command [format {Aggiungi %s} $w]]\n\
pack $ToolBox.frecciadestra -in $ToolBox -side left\n\
\n\
set BottoneStart [button $ToolBox.iconastart -image $IconaStart\\\n\
				-command [format {StartAggiornamento %s} $w] ]\n\
pack $ToolBox.iconastart -in $ToolBox -side left\n\
\n\
set BottoneStop [button $ToolBox.iconastop -image $IconaStop\\\n\
		-command [format {Stop %s} $w] -state disabled ]\n\
pack $ToolBox.iconastop -in $ToolBox -side left\n\
\n\
pack $ToolBox -fill x -side top -padx 2 -pady 2\n\
\n\
\n\
\n\
\n\
set pane [tixPanedWindow $w.pane -orient horizontal]\n\
pack $pane -expand yes -fill both -side right\n\
set f1 [$pane add 1 -min 100 -expand 2]\n\
set f2 [$pane add 2 -min 100 -expand 1]\n\
$f1 config -relief flat\n\
$f2 config -relief flat\n\
\n\
set ListaWindows [frame $f1.frame -border 1 -relief raised]\n\
set DebugWindow [frame $f2.frame -border 1 -relief raised]\n\
\n\
\n\
\n\
set ListaVariabili [frame $ListaWindows.frame]\n\
set listboxm [tixScrolledTList $ListaVariabili.lb -width 370\\\n\
		-height 130 -options {\\\n\
		tlist.orient horizontal\\\n\
		tlist.selectMode extended\\\n\
		} ]\n\
set listbox $listboxm.tlist\n\
pack $ListaVariabili -side top -fill both -expand yes\n\
pack $listboxm -side left -fill both -expand yes\n\
pack $ListaWindows -fill both -expand yes\n\
\n\
bind $listbox <Double-1> [format {Aggiungi %s} $w]\n\
\n\
\n\
\n\
set Matrice(page) AA\n\
set Matrice(copypage) A \n\
set Matrice(selezione) \"\"\n\
\n\
set FrameDebug [frame $DebugWindow.frame]\n\
set Matrice(widget) [table $FrameDebug.grid \\\n\
		-autoclear 1 \\\n\
		-variable $Matrice(page)\\\n\
		-titlerows 1 \\\n\
		-titlecols 0\\\n\
		-colstretch unset\\\n\
		-drawmode fast\\\n\
		-cols 2\\\n\
		-rows 1\\\n\
                -validate yes \\\n\
                -validatecommand { validate %c %S } \\\n\
                -browsecmd { browse %s %S } \\\n\
		-width 13 -height 10 ]\n\
set Matrice(linee) 0\n\
set Matrice(browse) 1\n\
$Matrice(widget) activate 1,1\n\
set Matrice(browse) 0\n\
$Matrice(widget) tag config destro-ro -anchor e -state disabled\n\
$Matrice(widget) tag config sinistro-ro -anchor w -state disabled\n\
$Matrice(widget) tag config centro-ro -anchor c -state disabled\n\
$Matrice(widget) tag configure INP-OFF -fg white -bg DarkGreen -relief sunken\n\
$Matrice(widget) tag configure INP-ON  -fg black -bg green -relief raised\n\
$Matrice(widget) tag configure OUT-OFF -fg white -bg red4 -relief sunken\n\
$Matrice(widget) tag configure OUT-ON  -fg white -bg red -relief raised\n\
$Matrice(widget) tag configure NOPLC  -bg gray -relief sunken\n\
\n\
$Matrice(widget) tag row centro-ro 0\n\
$Matrice(widget) tag col sinistro-ro 0 \n\
$Matrice(widget) tag col destro-ro 1\n\
$Matrice(widget) width 1 6\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
set ary(0,0) [CaricaLingua $FileLingua LabelIO]\n\
set ary(0,1) [CaricaLingua $FileLingua Valore]\n\
\n\
scrollbar $FrameDebug.sy -command [list $Matrice(widget) yview]\n\
scrollbar $FrameDebug.sx -command [list $Matrice(widget) xview]\\\n\
					 -orient horizontal\n\
$Matrice(widget) configure  -yscrollcommand [format { %s set } $FrameDebug.sy ]\\\n\
                 -xscrollcommand  [format { %s set } $FrameDebug.sx ] \n\
\n\
bind $FrameDebug.sy <FocusIn> \"focus $Matrice(widget)\"\n\
bind $FrameDebug.sx <FocusIn> \"focus $Matrice(widget)\"\n\
\n\
pack $FrameDebug.sx -in $FrameDebug -side bottom -fill x\n\
pack $FrameDebug.sy -in $FrameDebug -side right -fill y\n\
pack $Matrice(widget) -in $FrameDebug -side left -expand yes -fill both\n\
pack $FrameDebug -side left -expand yes -fill both  -padx 3 -pady 3\n\
pack $DebugWindow -fill both -expand yes\n\
\n\
bind $Matrice(widget) <Alt-f> { }\n\
bind $Matrice(widget) <Alt-F> { }\n\
\n\
bind $Matrice(widget) <ButtonPress-3> {\n\
    	set Matrice(selezione) [%W get @%x,%y]\n\
	}\n\
bind $Matrice(widget) <ButtonRelease-2> {\n\
	break\n\
	}\n\
bind $Matrice(widget) <1> {\n\
    ClickSuMatrice %W %x %y\n\
    break\n\
}\n\
bind $Matrice(widget) <Motion> {\n\
    break\n\
}\n\
\n\
bind $Matrice(widget) <Key-Up> {\n\
	break\n\
	}\n\
bind $Matrice(widget) <Key-Down> {\n\
	break\n\
	}\n\
bind $Matrice(widget) <Key-Left> {\n\
	break\n\
	}\n\
bind $Matrice(widget) <Key-Right> {\n\
	break\n\
	}\n\
\n\
PopUpTkTable $Matrice(widget)\n\
\n\
set Matrice(toplevel) $w\n\
\n\
\n\
\n\
\n\
$w.ballon bind $ToolBox.iconareload\\\n\
	-balloonmsg \"Ricarica\"\\\n\
	-statusmsg \"Ricarica la lista variabili dal PLC\" \n\
$w.ballon bind $ToolBox.frecciadestra -balloonmsg \"Aggiungi\" \\\n\
	-statusmsg \"Aggiunge la variabile dalla lista variabili\"\n\
$w.ballon bind $ToolBox.iconastart -balloonmsg \"Start\" \\\n\
	-statusmsg \"Avvia l'aggiornamento delle variabili\"\n\
$w.ballon bind $ToolBox.iconastop -balloonmsg \"Stop\" \\\n\
	-statusmsg \"Ferma l'aggiornamento delle variabili\"\n\
\n\
}",
"proc ReadDefaults {} {\n\
global HOME\n\
\n\
set appoggio [ReadReg Applicazioni_Lae PATH]\n\
if {$appoggio == \"unix\" } {\n\
	set HOME /home/lae\n\
	} elseif { $appoggio == \"errore_registro\" } {\n\
		  tk_messageBox -icon info -message \"Fallita lettura registro\" -type ok\n\
		  exit\n\
		 } else {\n\
		  set HOME $appoggio\n\
		 }\n\
}",
"proc	get_dtm { w } {\n\
}",
"proc    get_pixmaps { w } {\n\
\n\
  global HOME FrecciaDestra IconaExit IconaStart IconaStop IconaReload\n\
  global file_sel1_image file_sel2_image FileLingua IconaInput IconaOutput\n\
\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/insert.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] insert.xpm\"  yes\n\
    return\n\
  }\n\
set FrecciaDestra [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/exit.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] exit.xpm\"  yes\n\
    return\n\
  }\n\
set IconaExit [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/Qsstart.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] Qsstart.xpm\"  yes\n\
    return\n\
  }\n\
set IconaStart [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/Qssospe.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] Qssospe.xpm\"  yes\n\
    return\n\
  }\n\
set IconaStop [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/reload.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] reload.xpm\"  yes\n\
    return\n\
  }\n\
set IconaReload [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/input.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] input.xpm\"  yes\n\
    return\n\
  }\n\
set IconaInput [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
if [catch {set imagefile [open \"$HOME/icone/output.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] output.xpm\"  yes\n\
    return\n\
  }\n\
set IconaOutput [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/xncPROG.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] xncPROG.xpm\"  yes\n\
    return\n\
  }\n\
set file_sel1_image [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
\n\
if [catch {set imagefile [open \"$HOME/icone/xncLOCAL.xpm\" r] } out] {\n\
    MessageBox $w \"[CaricaLingua $FileLingua ErroreFile] xncLOCAL.xpm\"  yes\n\
    return\n\
  }\n\
set file_sel2_image [ list [image create pixmap -data [read $imagefile]] ]\n\
close $imagefile\n\
\n\
\n\
}",
"proc CancellaFile { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set FileCanc [prompt $CalWdg [CaricaLingua $FileLingua Cancella_file]] \n\
  if {[catch \"file delete [pwd]/$FileCanc\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreFileDel] $FileCanc\"  yes\n\
  }\n\
}",
"proc CreaDirettorio { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set DirCreate [prompt $CalWdg [CaricaLingua $FileLingua Crea_direttorio]] \n\
  if {[catch \"file mkdir [pwd]/$DirCreate\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreDirCrea] $DirCreate\"  yes\n\
  }\n\
}",
"proc CancellaDirettorio { CalWdg PathLab DataLab ListD ListF} {\n\
\n\
  global FileLingua\n\
\n\
  set DirDel [prompt $CalWdg [CaricaLingua $FileLingua Cancella_direttorio]] \n\
  if {[catch \"file delete -force [pwd]/$DirDel\" out] == 0} {\n\
    LoadFile $PathLab $DataLab [$ListD subwidget listbox] [$ListF subwidget listbox]\n\
  } else {\n\
    MessageBox $CalWdg \"[CaricaLingua $FileLingua ErroreDirDel] $DirDel\"  yes\n\
  }\n\
}",
"proc LoadFile { PathLab DataLab ListD ListF } {\n\
\n\
  global ProgPath NomeFile\n\
\n\
  $ListD delete 0 end\n\
  $ListF delete 0 end\n\
\n\
  update idletasks\n\
\n\
  $PathLab config -text \"/[string trimleft [pwd] $ProgPath]\"\n\
  if { [pwd] != $ProgPath } {\n\
    $ListD insert end \"..\"\n\
  }\n\
\n\
  # add any directory\n\
  foreach i [lsort [glob -nocomplain *]] {\n\
    if {$i != \".\" && $i != \"..\"} {\n\
      if {[file isdirectory ./$i]} {\n\
        $ListD insert end \"$i\"\n\
      } else {\n\
        $ListF insert end \"$i\"\n\
      }\n\
    }\n\
  }\n\
\n\
  # add any dot-files:\n\
  foreach i [lsort \"[glob -nocomplain *.plc]\"] {\n\
    if {$i != \".\" && $i != \"..\"} {\n\
      if {[file isdirectory ./$i]} {\n\
        $ListD insert end \"$i\"\n\
      } else {\n\
        $ListF insert end \"$i\"\n\
      }\n\
    }\n\
  }\n\
}",
"proc DefinisciData { w } {\n\
  global NomeFile\n\
\n\
  $w config -text \"$NomeFile - [file size ./$NomeFile] - [clock format [file mtime ./$NomeFile] -format {%T %d/%m/%y}]\"\n\
}",
"proc EscPress {} {\n\
  global NomeFile\n\
set NomeFile {}\n\
destroy .specchio\n\
}",
"proc FileSelection { MainW title} {\n\
  global Lingua font_usato FileLingua ProgPath\n\
  global NomeFile\n\
  global file_sel1_image file_sel2_image\n\
\n\
  #sporco trucco per il Doppio click su file\n\
  catch {destroy .specchio}\n\
  toplevel .specchio\n\
  wm withdraw .specchio\n\
\n\
  set w .fs\n\
  if [winfo exist $w] {\n\
    return\n\
  }\n\
  toplevel $w \n\
\n\
  wm title $w [CaricaLingua $FileLingua $title]\n\
  \n\
\n\
  set NomeFile {}\n\
  #\n\
  #  Creazione della MENUBAR principale\n\
  #\n\
  frame $w.top \n\
  frame $w.top.menubar -relief flat -bd 2 \n\
\n\
  menubutton $w.top.menubar.mb1 -text [CaricaLingua $FileLingua Servizi] -anchor w -menu $w.top.menubar.mb1.file \\\n\
                              -font $font_usato -underline 0 -relief raised \n\
  set file [menu $w.top.menubar.mb1.file -tearoff 0 ]\n\
  $file add command -label [CaricaLingua $FileLingua Cancella_file] -command \"CancellaFile $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
  $file add separator\n\
  $file add command -label [CaricaLingua $FileLingua Crea_direttorio] -command \"CreaDirettorio $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
  $file add command -label [CaricaLingua $FileLingua Cancella_direttorio] -command \"CancellaDirettorio $w \\\n\
                            $w.top.path $w.top.filedata $w.top.scrolled.dir $w.top.scrolled.file\" \\\n\
                              -font $font_usato \n\
\n\
  label $w.top.menubar.label1 -image $file_sel1_image -relief flat \n\
  label $w.top.menubar.label2 -image $file_sel2_image -relief flat \n\
  pack $w.top.menubar.label1 -in $w.top.menubar -side left -anchor nw -padx 3\n\
  pack $w.top.menubar.mb1 -in $w.top.menubar -side left -expand yes -fill x \n\
  pack $w.top.menubar.label2 -in $w.top.menubar -side right -anchor ne -padx 3\n\
\n\
  #\n\
  #  Creazione delle due scrolled list\n\
  #\n\
\n\
  label $w.top.path -text \"/\" -font $font_usato \n\
\n\
  frame $w.top.scrolled \n\
  tixScrolledListBox $w.top.scrolled.dir -scrollbar y -options [format {\n\
	listbox.font %s\n\
    } $font_usato]\n\
\n\
  tixScrolledListBox $w.top.scrolled.file -scrollbar y -options [format {\n\
	listbox.font %s\n\
    } $font_usato]\n\
  pack $w.top.scrolled.dir -in $w.top.scrolled -side left -expand true -fill both \n\
  pack $w.top.scrolled.file -in $w.top.scrolled -side right -expand true -fill both\n\
\n\
\n\
  entry $w.top.filename -font $font_usato -textvariable NomeFile \n\
  label $w.top.filedata -font $font_usato \n\
\n\
  tixButtonBox $w.box -orientation horizontal\n\
  $w.box add ok     -text [CaricaLingua $FileLingua OK]  -font $font_usato  -command \"destroy .specchio\" \n\
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command \"\n\
                                                             set NomeFile {}\n\
                                                             destroy .specchio\" \n\
\n\
  pack $w.top.menubar -in $w.top -side top -fill x -expand true -anchor n\n\
  pack $w.top.path -in $w.top -side top -anchor nw -pady 2 \n\
  pack $w.top.scrolled -in $w.top -side top -fill both -expand true \n\
  pack $w.top.filename -in $w.top -side top -anchor w -pady 5 \n\
  pack $w.top.filedata -in $w.top -side top -pady 5 \n\
\n\
  pack $w.top -side top -padx 10 -pady 10 -expand true -fill both -anchor n \n\
  pack $w.box -side bottom -fill x -anchor n\n\
\n\
  wm transient $w\n\
\n\
  posiziona:finestre $MainW $w\n\
\n\
  set oldPwd [pwd]\n\
  cd $ProgPath\n\
  LoadFile $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]\n\
\n\
  bind [$w.top.scrolled.file subwidget listbox] <ButtonRelease-1> [format {\n\
    set NomeFile [selection get]\n\
    DefinisciData %s\n\
  } $w.top.filedata]\n\
\n\
  bind [$w.top.scrolled.file subwidget listbox] <KeyRelease> [format {\n\
    set SelezioneCorrente [%s curselection]\n\
    if { $SelezioneCorrente != \"\" } {\n\
      set NomeFile [%s get $SelezioneCorrente]\n\
      DefinisciData %s\n\
    }\n\
  } [$w.top.scrolled.file subwidget listbox] [$w.top.scrolled.file subwidget listbox]  $w.top.filedata]\n\
\n\
\n\
  bind [$w.top.scrolled.dir subwidget listbox] <Double-1> [format {\n\
    set Direttorio [selection get]\n\
    if { $Direttorio != \"..\" } {\n\
      cd $Direttorio\n\
      LoadFile %s %s %s %s\n\
    } elseif { [pwd] != $ProgPath } {\n\
      cd ..\n\
      LoadFile %s %s %s %s\n\
    } \n\
  } $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox] \\\n\
    $w.top.path $w.top.filedata [$w.top.scrolled.dir subwidget listbox] [$w.top.scrolled.file subwidget listbox]] \n\
\n\
  bind $w.top.filename <Return> [format {\n\
    destroy %s\n\
  } .specchio] \n\
\n\
  bind $w.top.filename <KeyRelease> {\n\
    set NomeFile [string toupper $NomeFile]\n\
  }\n\
\n\
  bind $w.top.filename <FocusIn> {\n\
      %W select range 0 end\n\
  }\n\
\n\
  bind $w <Escape> \" set NomeFile {}\n\
                   destroy .specchio\" \n\
\n\
  wm protocol $w WM_DELETE_WINDOW EscPress\n\
  grab $w\n\
  focus $w.top.filename\n\
  tkwait window .specchio\n\
  destroy $w\n\
\n\
  set NomeFileCompleto \"[pwd]/$NomeFile\"\n\
  cd $oldPwd\n\
  if { $NomeFile != \"\" } {\n\
    return $NomeFileCompleto\n\
  } else {\n\
   return 0\n\
  }\n\
}",
"proc Salva { w } {\n\
global	Matrice\n\
if [info exist Matrice(NomeFile) ] {\n\
	Save\n\
	} else {\n\
	SaveCome $w\n\
	}\n\
}",
"proc Save { } {\n\
\n\
global Matrice\n\
global font_usato FileLingua\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
if {[catch \"set fId [open $Matrice(NomeFile) w]\" out] == 0} {\n\
	fconfigure $fId -translation binary\n\
	puts -nonewline $fId \"Firmoni\\n\"\n\
	puts -nonewline $fId \"#    FILE DI CONFIGURAZIONE\\n\"\n\
	puts -nonewline $fId \"#  SIMULATORE DI I/O PER QPLC\\n\"\n\
	for {set i 1} {$i <= $Matrice(linee)} { incr i } {\n\
		puts -nonewline $fId \"Riga$i=$ary($i,0)=$ary($i,1)\\n\"\n\
		}\n\
	puts -nonewline $fId \"inomriF\\n\"\n\
	close $fId\n\
   	} else {\n\
	MessageBox $w\\\n\
	      \"Problemi nell'apertura \\n$Matrice(NomeFile) per salvataggio\" yes\n\
	return 0\n\
	}\n\
return 1\n\
}",
"proc SaveCome { w } {\n\
global Matrice FileLingua NomeFile ProgPath SAVEPATH\n\
set ProgPath $SAVEPATH\n\
FileSelection $w [CaricaLingua $FileLingua Salva]\n\
if { $NomeFile != \"\" } {\n\
	set Matrice(OldNomeFile) $Matrice(NomeFile) \n\
	if { [string first \".SIM\" $NomeFile ] == \"-1\" && \\\n\
			[string first \".sim\" $NomeFile ] == \"-1\" } {\n\
		set Matrice(NomeFile)\\\n\
			 \"/home/d_xnc/plc/[string toupper $NomeFile.SIM]\"\n\
		} else {\n\
		set Matrice(NomeFile)\\\n\
			 \"/home/d_xnc/plc/[string toupper $NomeFile]\"\n\
		}\n\
	if [ Save ] {\n\
		$Matrice(WidgetNomeFile) configure\\\n\
	      -text \"[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)\"\n\
		} else {\n\
		set Matrice(NomeFile) $Matrice(OldNomeFile)\n\
		}\n\
		\n\
	}\n\
}",
"proc Apri { w } {\n\
global Matrice FileLingua NomeFile ProgPath SAVEPATH\n\
set ProgPath $SAVEPATH\n\
FileSelection $w [CaricaLingua $FileLingua Apri]\n\
if { $NomeFile != \"\" } {\n\
	set Matrice(OldNomeFile) $Matrice(NomeFile) \n\
	set Matrice(NomeFile) \"/home/d_xnc/plc/$NomeFile\"\n\
	if [ ApriPure ] {\n\
		$Matrice(WidgetNomeFile) configure\\\n\
	      -text \"[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)\"\n\
		} else {\n\
		set Matrice(NomeFile) $Matrice(OldNomeFile)\n\
		}\n\
	}\n\
}",
"proc ApriPure { } {\n\
\n\
global Matrice\n\
global font_usato FileLingua\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
if {[catch \"set fId [open $Matrice(NomeFile) r]\" out] != 0} {\n\
	MessageBox $w\\\n\
	      \"Problemi nell'apertura \\n$Matrice(NomeFile) per lettura\" yes\n\
	return 0\n\
	}\n\
\n\
set inputfile [split [read $fId] \\n]\n\
close $fId\n\
\n\
Cancella $Matrice(toplevel)\n\
\n\
update idletasks\n\
foreach line $inputfile {\n\
	if { $line != \"Firmoni\" && $line != \"inomriF\" && $line != \"\"  && \\\n\
				 [string index $line 0] != \"#\"  } {\n\
		InserisceVareVal [lindex [split $line =] 1]\\\n\
					[lindex [split $line =] 2] toggle\n\
		}\n\
	}\n\
RefreshInputs\n\
return 1\n\
}",
"proc validate {colonna val} {\n\
\n\
switch $colonna {\n\
	1 {\n\
	  }\n\
	2 {\n\
	  ## Nome file Alfanumerico SOLO maiuscolo Lunghezza 64\n\
	  set expr {^[a-zA-Z0-9_./]*$}\n\
	  set len 64\n\
	  }\n\
	3 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	4 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	5 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	6 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	7 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	8 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	9 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	10 {\n\
	  ## contapezzi SOLO numerico Lunghezza 4\n\
	  set expr {^[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 4\n\
	  }\n\
	11 {\n\
	  ## larghezza lamiera SOLO numerico Lunghezza 8\n\
	  set expr {^[0-9]*\\.?[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	  set len 7\n\
	  }\n\
	default { return 0 }\n\
	}\n\
 if {[regexp $expr $val]} {\n\
	if { [string length $val] <= $len } {\n\
		return 1\n\
		} else {\n\
		bell\n\
  		return 0\n\
		}\n\
	} else {\n\
	bell\n\
  	return 0\n\
	}\n\
}",
"proc browse { prev next } {\n\
    global Matrice\n\
    global FileLingua\n\
\n\
if { $Matrice(browse) == 1 } { return }	\n\
\n\
set Matrice(browse) 1\n\
\n\
if { [lindex [split $next \",\"] 0] == \"0\" || \\\n\
			[lindex [split $next \",\"] 0] > $Matrice(linee) } {\n\
	$Matrice(widget) activate $prev\n\
	set next $prev\n\
	}\n\
set Matrice(browse) 0\n\
}",
"proc Uscita { w } {\n\
global FileLingua \n\
\n\
exit\n\
}",
"proc ClickSuMatrice { w r c } {\n\
global FileLingua Matrice Nomi TipoIO Lunghezza ArrayDebugLinee Congela\n\
\n\
if {$Congela == \"True\"} {\n\
        return\n\
        }\n\
\n\
set label [$w get @0,$c]\n\
\n\
if { $label == \"\" } {\n\
	return\n\
	}\n\
\n\
if { $label == [$w get @$r,$c] } {\n\
	# si tratta della prima colonna (LABELS)\n\
	return\n\
	}\n\
upvar \\#0 $Matrice(page) ary\n\
if { $ary(0,1) == [$w get @$r,$c] } {\n\
	# si tratta della prima riga \n\
	return\n\
	}\n\
\n\
set y $ArrayDebugLinee($label)\n\
\n\
if { ! [info exist ary($y,0) ] } {\n\
	return\n\
	}\n\
\n\
if { ! [info exist Nomi($ary($y,0)) ] } {\n\
	return\n\
	}\n\
\n\
if { $TipoIO($ary($y,0)) != 1 } {\n\
	# posso forzare solo gli inputs\n\
	return\n\
	}\n\
\n\
if { $Lunghezza($ary($y,0)) == 1 } {\n\
	# trattasi  di variabile a 1 bit\n\
	ToggleValore $y\n\
	} else {\n\
	set ret [AdcInput $Matrice(toplevel) $ary($y,0) $ary($y,1) 1024 -1023]\n\
	if { $ret != \"\" } {\n\
		Setstate $ary($y,0) $ret toggle\n\
		set ary($y,1) $ret\n\
		}\n\
	}\n\
}",
"proc WaitRiconnetti { } {\n\
  global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
  global BottoneStart BottoneStop BottoneReload BottoneAdd\n\
\n\
$BottoneStart configure -state disabled\n\
$BottoneStop configure -state disabled\n\
$BottoneAdd configure -state disabled\n\
$BottoneReload configure -state disabled\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state disabled\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state disabled\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state disabled\n\
$BottoneSalvaCome(widget) entryconfigure\\\n\
				$BottoneSalvaCome(index) -state disabled\n\
}",
"proc OkRiconnetti { } {\n\
  global BottoneCancella BottoneApri BottoneSalva BottoneSalvaCome\n\
  global BottoneStart BottoneStop BottoneReload BottoneAdd Congela Matrice\n\
\n\
set Congela False\n\
Reload $Matrice(toplevel)\n\
RefreshInputs\n\
MessageBox $Matrice(toplevel) \"CONNECTED !!!!\" yes\n\
$BottoneStart configure -state active\n\
$BottoneStop configure -state disabled\n\
$BottoneAdd configure -state active\n\
$BottoneReload configure -state active\n\
$BottoneCancella(widget) entryconfigure $BottoneCancella(index) -state active\n\
$BottoneApri(widget) entryconfigure $BottoneApri(index) -state active\n\
$BottoneSalva(widget) entryconfigure $BottoneSalva(index) -state active\n\
$BottoneSalvaCome(widget) entryconfigure\\\n\
				$BottoneSalvaCome(index) -state active\n\
}",
"proc Riconnetti { } {\n\
\n\
  global sId_delay sId_toggle sId_listp\n\
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
  global SocketPlc Host Congela Matrice\n\
\n\
if { $Congela == \"True\" } {\n\
	WaitRiconnetti\n\
	}\n\
if { [VerificaEsistenzaPlc] == 0 } {\n\
	OkRiconnetti\n\
	return\n\
	}\n\
after 3000 Riconnetti\n\
}",
"proc VerificaEsistenzaPlc { } {\n\
\n\
  global sId_delay sId_toggle sId_listp\n\
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
  global SocketPlc Host Congela\n\
\n\
if [catch {set sId_delay [socket $Host $SocketPlc]} Result] {\n\
	return 1\n\
	}\n\
fileevent $sId_delay readable {AspettaRispostaPLC delay}\n\
vwait RispostaPLC_delay\n\
if [catch {set sId_toggle [socket $Host $SocketPlc]} Result] {\n\
	return 1\n\
	}\n\
fileevent $sId_toggle readable {AspettaRispostaPLC toggle}\n\
vwait RispostaPLC_toggle\n\
if [catch {set sId_listp [socket $Host $SocketPlc]} Result] {\n\
	return 1\n\
	}\n\
fileevent $sId_listp readable {AspettaRispostaPLC listp}\n\
vwait RispostaPLC_listp\n\
set Congela False\n\
return 0\n\
}",
"proc Getstate { NomeVar tred } {\n\
  global TcpHostPlcRemoto SocketPlc AddressBase TipoIO Nomi Lunghezza\n\
  global sId_delay sId_toggle sId_listp \n\
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
  global	Congela\n\
\n\
if { $Congela == \"True\" } {\n\
	return\n\
	}\n\
\n\
switch -- $tred {\n\
	\"delay\" {\n\
		set sId	 $sId_delay\n\
		set RispostaPLC RispostaPLC_delay\n\
		}\n\
	\"toggle\" {\n\
		set sId	 $sId_toggle\n\
		set RispostaPLC RispostaPLC_toggle\n\
		}\n\
	\"listp\" {\n\
		set sId	 $sId_listp\n\
		set RispostaPLC RispostaPLC_listp\n\
		}\n\
	}\n\
\n\
switch -- $TipoIO($NomeVar) {\n\
	\"1\" {\n\
		puts $sId \"mem [format %x $Nomi($NomeVar) ]\\\n\
			 			[Nbytes $Lunghezza($NomeVar)]\"\n\
		flush $sId\n\
		vwait $RispostaPLC\n\
		append risp $ $RispostaPLC\n\
		eval \"set rispo $risp\"\n\
		return [format %d 0x[InvertiByte [lindex $rispo 1] ] ] }\n\
	\"2\" {\n\
		puts $sId \"mem [format %x $Nomi($NomeVar) ]\\\n\
						[Nbytes $Lunghezza($NomeVar)]\"\n\
		flush $sId\n\
		vwait $RispostaPLC\n\
		append risp $ $RispostaPLC\n\
		eval \"set rispo $risp\"\n\
		return [format %d 0x[InvertiByte [lindex $rispo 1] ] ] }\n\
	}\n\
}",
"proc Setstate { NomeVar Valore tred } {\n\
  global Lunghezza Key\n\
  global sId_delay sId_toggle sId_listp \n\
  global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
  global Congela\n\
\n\
if { $Congela == \"True\" } {\n\
	return\n\
	}\n\
\n\
switch -- $tred {\n\
	\"delay\" {\n\
		set sId	 $sId_delay\n\
		set RispostaPLC RispostaPLC_delay\n\
		}\n\
	\"toggle\" {\n\
		set sId	 $sId_toggle\n\
		set RispostaPLC RispostaPLC_toggle\n\
		}\n\
	\"listp\" {\n\
		set sId	 $sId_listp\n\
		set RispostaPLC RispostaPLC_listp\n\
		}\n\
	}\n\
\n\
puts $sId \"setin $Key($NomeVar) [format %x $Valore]\"\n\
flush $sId\n\
vwait $RispostaPLC\n\
append risp $ $RispostaPLC\n\
eval \"set rispo $risp\"\n\
return $rispo\n\
}",
"proc AspettaRispostaPLC { tred } {\n\
\n\
global Variabili\n\
global sId_delay sId_toggle sId_listp \n\
global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
global	Congela\n\
\n\
switch -- $tred {\n\
	\"delay\" {\n\
		set sId	 $sId_delay\n\
		set RispostaPLC RispostaPLC_delay\n\
		}\n\
	\"toggle\" {\n\
		set sId	 $sId_toggle\n\
		set RispostaPLC RispostaPLC_toggle\n\
		}\n\
	\"listp\" {\n\
		set sId	 $sId_listp\n\
		set RispostaPLC RispostaPLC_listp\n\
		}\n\
	}\n\
\n\
set linea [GetLine $sId] \n\
\n\
set Variabili [list \"\"]\n\
\n\
\n\
if {[string range $linea 0 2] == \"OK\"} {\n\
	set $RispostaPLC $linea\n\
	return\n\
	}\n\
\n\
while { [string index $linea 1] == \"-\" } {\n\
	set var [string range $linea 2 end]\n\
	set Variabili [lappend Variabili \"$var\"]\n\
	set linea [GetLine $sId]\n\
	}\n\
set $RispostaPLC $linea\n\
}",
"proc GetOutput { Variabile tred } {\n\
\n\
global sId_delay sId_toggle sId_listp \n\
global RispostaPLC_delay RispostaPLC_toggle RispostaPLC_listp\n\
global	Congela\n\
\n\
if { $Congela == \"True\" } {\n\
	return\n\
	}\n\
\n\
switch -- $tred {\n\
	\"delay\" {\n\
		set sId	 $sId_delay\n\
		set RispostaPLC RispostaPLC_delay\n\
		}\n\
	\"toggle\" {\n\
		set sId	 $sId_toggle\n\
		set RispostaPLC RispostaPLC_toggle\n\
		}\n\
	\"listp\" {\n\
		set sId	 $sId_listp\n\
		set RispostaPLC RispostaPLC_listp\n\
		}\n\
	}\n\
\n\
\n\
puts $sId \"wreset\"\n\
flush $sId\n\
vwait $RispostaPLC\n\
\n\
if { [lindex $$RispostaPLC 0] == \"OK\" } {\n\
	return 0\n\
    	}\n\
return 1\n\
}",
"proc GetLista { } {\n\
global	Congela\n\
global sId_listp RispostaPLC_listp FileLingua\n\
global Variabili Nomi TipoIO Lunghezza Key AddressBase\n\
\n\
if { $Congela == \"True\" } {\n\
	return\n\
	}\n\
\n\
puts $sId_listp \"mod U_CODE\"\n\
flush $sId_listp\n\
vwait RispostaPLC_listp\n\
if { [lindex $RispostaPLC_listp 0] == \"E\" } {\n\
	MessageBox $w  \"[CaricaLingua $FileLingua ErroreLista]\"  yes\n\
	} elseif { [lindex $RispostaPLC_listp 0] == \"D\" } {\n\
		set AddressBase 0x[lindex $RispostaPLC_listp 1]\n\
	}\n\
\n\
puts $sId_listp \"list p\"\n\
flush $sId_listp\n\
vwait RispostaPLC_listp\n\
foreach var $Variabili {\n\
    if { [lindex $var 2] != \"\" } {\n\
	puts $sId_listp \"sym [lindex $var 3]\"\n\
	flush $sId_listp\n\
	vwait RispostaPLC_listp\n\
	if { [lindex $RispostaPLC_listp 1] != \"S\" } {\n\
		set mnemonico [lindex $RispostaPLC_listp 2]\n\
		} else {\n\
		set mnemonico \"[lindex $RispostaPLC_listp 2]\\[[format \"%d\" 0x[lindex $RispostaPLC_listp 3]]\\]\"\n\
		}\n\
	set address [lindex $var 3]\n\
	set key [lindex $var 4]\n\
	set inpoout [lindex $var 1]\n\
	set lunghezza [format \"%d\" \"0x[lindex $var 2]\" ]\n\
	set Nomi($mnemonico) \"0x$address\"\n\
	set TipoIO($mnemonico) $inpoout\n\
	set Lunghezza($mnemonico) $lunghezza\n\
	set Key($mnemonico) $key\n\
	}\n\
     }\n\
}",
"proc GetLine { sId } {\n\
global	Congela Matrice FileLingua\n\
set carattere [read $sId 1]\n\
append Risultato $carattere\n\
if { $carattere == \"\\0\" } {\n\
	Stop $Matrice(toplevel)\n\
	set Congela True\n\
	close $sId\n\
	set risp [MessageBox $Matrice(toplevel) \"RECONNECT ?\" yesno]\n\
	if { $risp == \"no\" } {\n\
		exit\n\
		}\n\
	if { $risp == \"busy\" } {\n\
		return\n\
		}\n\
	if { $risp == \"yes\" } {\n\
		Riconnetti\n\
		return\n\
		}\n\
	}\n\
while { $carattere != \"\\n\" && $carattere != \"\\0\" } {\n\
	set carattere [read $sId 1]\n\
	if { $carattere != \"\\n\" && $carattere != \"\\0\" } {\n\
  		append Risultato $carattere\n\
		}\n\
	}\n\
if { $Risultato == \"A 1\" } {\n\
	if { $Congela == \"True\" } {\n\
		return\n\
		}\n\
	set Congela True\n\
        Stop $Matrice(toplevel)\n\
	WaitRiconnetti\n\
	return\n\
	}\n\
if { $Risultato == \"A 2\" } {\n\
	if { $Congela == \"False\" } {\n\
		return\n\
		}\n\
	set Congela False\n\
	OkRiconnetti\n\
	return\n\
	}\n\
if { $carattere == \"\\0\" } {\n\
        set Congela True\n\
	Stop $Matrice(toplevel)\n\
	close $sId\n\
	if { [MessageBox $Matrice(toplevel) \"RECONNECT ?\" yesno] == \"no\" } {\n\
		exit\n\
		} else {\n\
		Riconnetti\n\
		return\n\
		}\n\
        }\n\
return $Risultato\n\
}",
"proc RefreshInputs { } {\n\
\n\
global Congela Matrice TipoIO FileLingua\n\
\n\
$Matrice(widget) flush\n\
upvar \\#0 $Matrice(page) ary\n\
\n\
for {set line 1} { $line <= $Matrice(linee) } { incr line } {\n\
   # Loop su tutti gli inputs sorvegliati\n\
   set NomeVariabile $ary($line,0)\n\
   if { $TipoIO($NomeVariabile) == 1 } {\n\
\n\
	if { $ary($line,1) == \"ON\" } {\n\
	    if { [Setstate $NomeVariabile 1 toggle] != \"OK\" } {\n\
		MessageBox $Matrice(toplevel)\\\n\
		 \"[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile\"  yes\n\
		continue\n\
		}\n\
	     while { [Getstate $NomeVariabile toggle] != 1 } {\n\
                }\n\
	     }\n\
        } else {\n\
	if { [Setstate $NomeVariabile $ary($line,1) toggle] != \"OK\" } {\n\
                MessageBox $Matrice(toplevel)\\\n\
                 \"[CaricaLingua $FileLingua ErroreSetting] $NomeVariabile\"  yes\n\
                continue\n\
                }\n\
	}\n\
    }\n\
}",
"proc CaricaLingua { FileLingua NomeEtichetta } {\n\
\n\
  global Lingua\n\
\n\
  if { $FileLingua != \"\" } { \n\
    set IndiceEtichetta [string first \"$NomeEtichetta.$Lingua\" $FileLingua]\n\
    if { $IndiceEtichetta != -1 } {\n\
      set FileLingua [string range $FileLingua $IndiceEtichetta end]\n\
      set Linea [string range $FileLingua 0 [string first \";\" $FileLingua]] \n\
      return [string range $Linea [expr [string first \"=\" $Linea] + 1] [expr [string length $Linea] -2]]\n\
    } else {\n\
      return $NomeEtichetta\n\
    }\n\
  } else {\n\
    return $NomeEtichetta\n\
  }\n\
} ",
"proc parse_args { arglist } {\n\
  upvar args args\n\
  \n\
  foreach pair $arglist {\n\
    set option [lindex $pair 0]\n\
    set default [lindex $pair 1]                ;# will be null if not supplied\n\
    set index [lsearch -exact $args \"-$option\"]\n\
    if {$index != -1} {\n\
      set index1 [expr {$index + 1}]\n\
      set value [lindex $args $index1]\n\
      uplevel 1 [list set $option $value]       ;# caller's variable \"$option\"\n\
    } else {\n\
      uplevel 1 [list set $option $default]     ;# caller's variable \"$option\"\n\
    }\n\
  }\n\
  return 0\n\
}",
"proc button_entry {args} {\n\
  parse_args {\n\
    {label {Value:} }\n\
    {variable value }\n\
    {buttonwidth {15} }\n\
    {entrywidth {8} }\n\
    {command {} }\n\
  }\n\
  global $variable font_usato \n\
  \n\
  set newframe [lindex $args 0]\n\
  \n\
  frame $newframe\n\
  set b $newframe.b\n\
  set e $newframe.e\n\
  button $b -text $label -width $buttonwidth -anchor w -font $font_usato -padx 0 -pady 0 -bd 1  \\\n\
            -command \"$command $newframe\"\n\
  entry $e -width $entrywidth -textvariable $variable \n\
  \n\
  pack $b -in $newframe -side left -fill x \n\
  pack $e -in $newframe -side left -fill x -expand yes\n\
  \n\
  bind $e <FocusIn> {\n\
      %W select range 0 end\n\
  }\n\
  bind $e <Return> [format { %s invoke } $b]\n\
  return $newframe\n\
}",
"proc posiziona:finestre {MainW w } {\n\
    set top_default .lae\n\
    if {$MainW  == \"\" } {\n\
      set main $top_default\n\
    } else {\n\
      set main $MainW\n\
    }\n\
    # first, display off-screen:\n\
    wm withdraw $w              ;# hide the window\n\
    wm protocol $w WM_DELETE_WINDOW GestioneUscita\n\
    update idletasks            ;# force geometry managers to run\n\
    # calculate position:\n\
    # il 7  ed il 25 sono relative ai bordi delle finestre\n\
    if { [winfo exist $w ] == 0 } {\n\
      return\n\
    }\n\
\n\
    set x [expr [winfo rootx $main] - 7 + [winfo width $main]/2 \\\n\
            - [winfo reqwidth $w]/2]\n\
    set y [expr [winfo rooty $main] - 25 + [winfo height $main]/2 \\\n\
            - [winfo reqheight $w]/2]\n\
\n\
    wm geom $w +$x+$y\n\
    update idletasks\n\
    wm deiconify $w\n\
    update idletasks\n\
    wm focus $w\n\
}",
"proc PrmEsc {} {\n\
  global NamePrompt\n\
set NamePrompt {}\n\
destroy .pr\n\
}",
"proc prompt { CalWdg title } {\n\
  global HOME font_usato FileLingua\n\
  global NamePrompt\n\
\n\
  set NamePrompt \"\"\n\
  set w .pr\n\
  if [winfo exists $w] {\n\
    destroy $w \n\
  }\n\
\n\
  toplevel $w\n\
\n\
  wm title $w $title\n\
  \n\
  entry $w.field -width 40 -font $font_usato -textvariable NamePrompt\n\
\n\
  tixButtonBox $w.box -orientation horizontal\n\
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato  -command \"destroy $w\" \n\
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command \"\n\
                                                    set NamePrompt {}\n\
                                                    destroy $w\" \n\
\n\
  pack $w.field -side top -padx 10 -pady 10\n\
  pack $w.box -side bottom -fill x\n\
\n\
  $w.field delete 0 end\n\
  $w.field insert end \"\"\n\
  \n\
  wm transient $w [winfo toplevel $CalWdg]\n\
\n\
  posiziona:finestre [winfo toplevel $CalWdg] $w          \n\
\n\
  focus $w.field\n\
\n\
  bind $w.field  <KeyRelease> {\n\
    set NamePrompt [string toupper $NamePrompt]\n\
  }\n\
\n\
  bind $w.field  <Return> \"\n\
    destroy $w\n\
  \"\n\
\n\
  grab $w\n\
  wm protocol $w WM_DELETE_WINDOW PrmEsc\n\
\n\
  tkwait window $w\n\
  return $NamePrompt\n\
}",
"proc MsgEsc {} {\n\
  global retMess\n\
set retMess esc\n\
destroy .message\n\
}",
"proc MessageBox { CalWdg Message type } {\n\
\n\
  global FileLingua font_usato\n\
  global retMess\n\
\n\
  set w .message\n\
  if [winfo exist $w] {\n\
    return busy\n\
  }\n\
\n\
\n\
  toplevel $w\n\
  \n\
  label $w.top -padx 20 -pady 10 -border 1 -relief raised -anchor c\\\n\
		 -text $Message -font $font_usato \n\
\n\
  tixButtonBox $w.box -orientation horizontal\n\
\n\
if { $type == \"yesnoesc\" } {\n\
	$w.box add ok -text [CaricaLingua $FileLingua SI] -font $font_usato\\\n\
		 -command \" 	set retMess yes\n\
				destroy $w\" \n\
	$w.box add close -text [CaricaLingua $FileLingua NO] -font $font_usato\\\n\
		 -command \"	set retMess no\n\
				destroy $w\" \n\
	$w.box add esc -text [CaricaLingua $FileLingua ANNULLA]\\\n\
		 -font $font_usato -command \"	set retMess esc\n\
						destroy $w\" \n\
	bind [$w.box subwidget ok] 	<Right> [format {\n\
				 focus %s } [$w.box subwidget close] ]\n\
	bind [$w.box subwidget close] 	<Right> [format {\n\
				 focus %s } [$w.box subwidget esc] ]\n\
	bind [$w.box subwidget esc] 	<Right> [format {\n\
				 focus %s } [$w.box subwidget ok] ]\n\
\n\
	bind [$w.box subwidget ok] 	<Left> [format {\n\
				 focus %s } [$w.box subwidget esc] ]\n\
	bind [$w.box subwidget close] 	<Left> [format {\n\
				 focus %s } [$w.box subwidget ok] ]\n\
	bind [$w.box subwidget esc] 	<Left> [format {\n\
				 focus %s } [$w.box subwidget close] ]\n\
}\n\
if { $type == \"yesno\" } {\n\
	$w.box add ok -text [CaricaLingua $FileLingua SI] -font $font_usato\\\n\
		-command \" 	set retMess yes\n\
				destroy $w\" \n\
	$w.box add close -text [CaricaLingua $FileLingua NO] -font $font_usato\\\n\
		-command \"	set retMess no\n\
				destroy $w\" \n\
	bind [$w.box subwidget ok] 	<Right> [format {\n\
				 focus %s } [$w.box subwidget close] ]\n\
	bind [$w.box subwidget close] 	<Right> [format {\n\
				 focus %s } [$w.box subwidget ok] ]\n\
\n\
	bind [$w.box subwidget ok] 	<Left> [format {\n\
				 focus %s } [$w.box subwidget close] ]\n\
	bind [$w.box subwidget close] 	<Left> [format {\n\
				 focus %s } [$w.box subwidget ok] ]\n\
}\n\
if { $type == \"yes\" } {\n\
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato \\\n\
	 -command \" 	set retMess yes\n\
			destroy $w\" \n\
}\n\
  pack $w.box -side bottom -fill x\n\
  pack $w.top -side top -fill both -expand yes\n\
\n\
  focus [$w.box subwidget ok] \n\
\n\
  wm transient $w $CalWdg \n\
\n\
  posiziona:finestre $CalWdg $w\n\
\n\
  grab $w\n\
  wm protocol $w WM_DELETE_WINDOW MsgEsc\n\
  tkwait window $w\n\
  return $retMess\n\
}",
"proc SetModificata { } {\n\
\n\
  global Lingua IconaModificata nosaved_image editgrid\n\
\n\
set editgrid(Modificata) 1\n\
$IconaModificata configure -image $nosaved_image\n\
}",
"proc UnSetModificata { } {\n\
\n\
  global Lingua IconaModificata saved_image editgrid\n\
\n\
if [info exist editgrid(NomeFile)] {\n\
	if [info exist editgrid(Modificata) ] {\n\
		unset editgrid(Modificata) \n\
		}\n\
	}\n\
$IconaModificata configure -image $saved_image\n\
}",
"proc SelezionaHost { } {\n\
\n\
  global Lingua IconaModificata saved_image FileLingua WSelectHost Host\n\
\n\
  catch {destroy .specchio}\n\
  toplevel .specchio\n\
  wm withdraw .specchio\n\
\n\
\n\
set w .selezionahost\n\
if [winfo exist $w] {\n\
    return\n\
}\n\
\n\
toplevel $w\n\
wm title $w [CaricaLingua $FileLingua SelezionaHost]\n\
\n\
\n\
frame $w.top \n\
frame $w.top.fr -relief ridge -bd 2\n\
\n\
button $w.top.ok -text [CaricaLingua $FileLingua OK]\\\n\
	 -command \"okhost\" -state disabled\n\
button $w.top.annulla -text [CaricaLingua $FileLingua ANNULLA] \\\n\
	 -command \"annullahost\"\n\
\n\
set WSelectHost(okbutton) $w.top.ok\n\
\n\
\n\
tixComboBox $w.top.fr.a -label \"[CaricaLingua $FileLingua Host]: \"\\\n\
	-dropdown true -prunehistory true \\\n\
	-editable true -grab none -validatecmd CommandHost \\\n\
	-options {\n\
	    listbox.height 6\n\
	    label.width 10\n\
	    label.anchor e\n\
	}\n\
\n\
if [info exist Host] {\n\
	set WSelectHost(old) $Host\n\
	} else {\n\
	set WSelectHost(old) \"\"\n\
	}\n\
  \n\
set WSelectHost(padre) $w.top.fr.a\n\
set WSelectHost(entry) [$w.top.fr.a subwidget entry]\n\
set WSelectHost(pulsante) [$w.top.fr.a subwidget arrow]\n\
set WSelectHost(storia) [$w.top.fr.a subwidget listbox]\n\
\n\
\n\
pack $w.top\n\
pack $w.top.fr -pady 2 -padx 2\n\
pack $w.top.fr.a -pady 5 -padx 3\n\
pack $w.top.ok -pady 5 -padx 3 -side left\n\
pack $w.top.annulla -pady 5 -padx 3 -side right\n\
\n\
foreach valore [ ReadHistory ]  {\n\
	if { [string index $valore 0 ] != \" \" && [string length $valore] > 0 } {\n\
		$WSelectHost(padre) addhistory $valore\n\
		}\n\
	}\n\
posiziona:finestre [winfo toplevel .lae] $w\n\
focus $WSelectHost(entry)\n\
\n\
grab $w\n\
wm protocol $w WM_DELETE_WINDOW annullahost\n\
tkwait window .specchio\n\
destroy $w\n\
\n\
}",
"proc CommandHost { valore } {\n\
\n\
  global Lingua IconaModificata saved_image FileLingua tcl_platform WSelectHost\n\
\n\
if { $valore == \"\" } { return \"\" }\n\
if { $tcl_platform(platform) == \"unix\" } {\n\
	# UNIX\n\
	if { [ catch { exec /bin/ping $valore 56 1 } out ] ==0 } {\n\
		# l'host esiste\n\
		$WSelectHost(padre) addhistory $valore\n\
		$WSelectHost(okbutton) configure -state active\n\
		update idletasks\n\
		focus $WSelectHost(okbutton)\n\
		return $valore\n\
		} else {\n\
		# l'host non esiste\n\
		update idletasks\n\
		focus $WSelectHost(entry)\n\
		return \"\"\n\
		}\n\
	} else {\n\
	# WINDOWS\n\
	if { [file exist //$valore/shared ]} {\n\
		$WSelectHost(padre) insert end $valore\n\
		$WSelectHost(okbutton) configure -state active\n\
		return $valore\n\
		} else {\n\
		# l'host non esiste\n\
		update idletasks\n\
		focus $WSelectHost(entry)\n\
		return \"\"\n\
		}\n\
	}\n\
}",
"proc okhost { } {\n\
\n\
  global FileLingua WSelectHost Host editgrid\n\
\n\
set Host [$WSelectHost(padre) cget -value]\n\
$editgrid(host) configure -text \"[CaricaLingua $FileLingua Host]: $Host\"\n\
WriteHistory [$WSelectHost(storia) get 0 end]\n\
destroy .specchio\n\
}",
"proc annullahost { } {\n\
\n\
  global FileLingua WSelectHost Host\n\
\n\
set Host $WSelectHost(old)\n\
WriteHistory [$WSelectHost(storia) get 0 end]\n\
destroy .specchio\n\
}",
"proc MostraVersione {w} {\n\
global FileLingua VERSIONE\n\
\n\
MessageBox $w \"[CaricaLingua $FileLingua Versione] $VERSIONE\"  yes\n\
}",
"proc ReadHistory {} {\n\
	global HOME\n\
	global tcl_platform\n\
\n\
set lista \"\"\n\
\n\
if [file exist $HOME/history] {\n\
	set fId [open $HOME/history r]\n\
	set lista [split [read $fId] \\n]\n\
	close $fId\n\
	}\n\
return $lista\n\
}",
"proc WriteHistory { lista } {\n\
	global HOME\n\
	global tcl_platform\n\
\n\
if {[catch { set fId [open $HOME/history w] } out] == 0} {	\n\
	foreach linea $lista {\n\
		puts $fId $linea\n\
	}\n\
	close $fId\n\
	}\n\
return\n\
}",
"proc validate { widget Parametro Carattere } {\n\
	global	ValoreParametri\n\
	global	TipoParametri\n\
	global	RangeParametri\n\
\n\
set fuorirange 0\n\
set comando \"catch {if {! ($ValoreParametri($Parametro) $RangeParametri($Parametro)) } {\n\
		set fuorirange 1\n\
		} } out \"\n\
\n\
if { $TipoParametri($Parametro) == \"I\" } {\n\
	#INTERO .... solo numeri\n\
	if { $ValoreParametri($Parametro) != \"\" } {\n\
		eval $comando\n\
	}\n\
	if { $RangeParametri($Parametro) == \">=0\" } {\n\
		# INTERO SOLO POSITIVO\n\
		set expr {^[1-9]?[0-9]*([0-9]?e[0-9]?[0-9]*)?$}\n\
	} else {\n\
		# INTERO ANCHE NEGATIVO\n\
		set expr {^[-+]?[0-9]*([0-9]?e[-+]?[0-9]*)?$}\n\
	}\n\
}\n\
\n\
if { $TipoParametri($Parametro) == \"R\" } {\n\
	#REALE\n\
	if { $ValoreParametri($Parametro) != \"\" } {\n\
		eval $comando\n\
	}\n\
	if { $RangeParametri($Parametro) == \">=0\" } {\n\
		# REALE SOLO POSITIVO\n\
		set expr {^[0-9]?[0-9]*\\.?[0-9]*([0-9]\\.?e[0-9]?[0-9]*)?$}\n\
	} else {\n\
		# REALE ANCHE NEGATIVO\n\
		set expr {^[-+]?[0-9]*\\.?[0-9]*([0-9]\\.?e[-+]?[0-9]*)?$}\n\
	}\n\
}\n\
\n\
\n\
if { ! [regexp $expr $ValoreParametri($Parametro) ] || $fuorirange } {\n\
	set indice [string last $Carattere $ValoreParametri($Parametro)]\n\
	set tmp1 [string range $ValoreParametri($Parametro) 0 [expr $indice -1]]\n\
	append tmp1 [string range $ValoreParametri($Parametro)\\\n\
							 [expr $indice + 1] end]\n\
	set ValoreParametri($Parametro) $tmp1\n\
	$widget icursor $indice\n\
	}\n\
}",
"proc InvertiByte { Stringa } {\n\
\n\
  set LunghezzaStringa [string length $Stringa]\n\
  incr LunghezzaStringa -2\n\
  \n\
  for {set indice $LunghezzaStringa} { $indice >= 0 } {incr indice -2} {\n\
    append StringaInvertita [string range $Stringa $indice [expr $indice + 1]]\n\
  }\n\
  return $StringaInvertita\n\
}",
"proc Nbytes { bits } {\n\
  return [expr (($bits -1) / 8) +1]\n\
}",
"proc SetMatrice { NomeVariabile TipoIO Lunghezza linea valore } {\n\
global Matrice\n\
\n\
upvar \\#0 $Matrice(page) ary\n\
upvar \\#0 $Matrice(copypage) copyary\n\
\n\
if { ! [info exist copyary($linea,1)] } {\n\
	set copyary($linea,1) \"\"\n\
	} \n\
\n\
switch -- $TipoIO {\n\
	\"1\" {\n\
		if { $Lunghezza == 1 } {\n\
			if { $valore == \"1\" } {\n\
				set valore \"ON\"\n\
				} elseif { $valore == \"0\" } {\n\
					set valore \"OFF\"\n\
					}\n\
			if { $valore == \"OFF\" && $copyary($linea,1) != 0 } {\n\
				set ary($linea,1) \"OFF\"\n\
				set copyary($linea,1) 0\n\
				$Matrice(widget) tag cell INP-OFF $linea,1\n\
				}\n\
			if { $valore == \"ON\" && $copyary($linea,1) != 1 } {\n\
				set ary($linea,1) \"ON\"\n\
				set copyary($linea,1) 1\n\
				$Matrice(widget) tag cell INP-ON $linea,1\n\
				}\n\
			} else {\n\
			set ary($linea,1) $valore\n\
			set copyary($linea,1) $valore\n\
			$Matrice(widget) tag cell INP-OFF $linea,1\n\
			}\n\
		}\n\
	\"2\" {\n\
		if { $Lunghezza == 1 } {\n\
			if { $valore == \"1\" } {\n\
				set valore \"ON\"\n\
				} elseif { $valore == \"0\" } {\n\
					set valore \"OFF\"\n\
					}\n\
			if { $valore == \"OFF\" && $copyary($linea,1) != 0 } {\n\
				set ary($linea,1) \"OFF\"\n\
				set copyary($linea,1) 0\n\
				$Matrice(widget) tag cell OUT-OFF $linea,1\n\
				}\n\
			if { $valore == \"ON\" && $copyary($linea,1) != 1 } {\n\
				set ary($linea,1) \"ON\"\n\
				set copyary($linea,1) 1\n\
				$Matrice(widget) tag cell OUT-ON $linea,1\n\
				}\n\
			} else {\n\
			set ary($linea,1) $valore\n\
			set copyary($linea,1) $valore\n\
			$Matrice(widget) tag cell OUT-OFF $linea,1\n\
			}\n\
		}\n\
	defaut {\n\
		}\n\
	}\n\
}",
"proc InterpretaOpzioni { w argv argc } {\n\
global	NomeFile AutoCaricamento SAVEPATH\n\
set AutoCaricamento \"False\"\n\
\n\
for { set i 0} {$i < [llength $argv] } {incr i} {\n\
	switch -- [lindex $argv $i] {\n\
		\"-f\" {\n\
			if { $i < [ expr [llength $argv] -1] } {\n\
				incr i\n\
				puts [lindex $argv $i]\n\
				set NomeFile \"$SAVEPATH/[lindex $argv $i]\"\n\
				if [ file readable $NomeFile] {\n\
					set AutoCaricamento \"True\"\n\
					} else {\n\
					MessageBox $w\\\n\
						\"$NomeFile NOT FOUND\" yes\n\
					}\n\
				} else {\n\
				MessageBox $w\\\n\
				 \"USO: Iosim \\[-f <nomefile configurazione>\\]\"\\\n\
							 yes\n\
				exit\n\
				}\n\
			}\n\
		}\n\
	}\n\
}",
"proc AdcInput { CalWdg title valoreOld max min } {\n\
  global font_usato FileLingua\n\
  global value\n\
\n\
  set NamePrompt \"\"\n\
  set w .adcInput\n\
  if [winfo exists $w] {\n\
	return\n\
  }\n\
\n\
  toplevel $w\n\
\n\
  wm title $w $title\n\
\n\
  set value $valoreOld\n\
  \n\
  tixControl $w.field -variable value -min $min -max $max -value $value\n\
\n\
  tixButtonBox $w.box -orientation horizontal\n\
  $w.box add ok -text [CaricaLingua $FileLingua OK] -font $font_usato  -command \"destroy $w\" \n\
  $w.box add cancel -text [CaricaLingua $FileLingua ANNULLA] -font $font_usato -command \"\n\
							set value $valoreOld\n\
                                                    	destroy $w\" \n\
\n\
  pack $w.field -side top -padx 10 -pady 10\n\
  pack $w.box -side bottom -fill x\n\
\n\
  wm transient $w [winfo toplevel $CalWdg]\n\
\n\
  posiziona:finestre [winfo toplevel $CalWdg] $w          \n\
\n\
  focus $w.field\n\
\n\
  grab $w\n\
  wm protocol $w WM_DELETE_WINDOW PrmEsc\n\
\n\
  tkwait window $w\n\
  return $value\n\
}",
"proc PopUpTkTable { w } {\n\
global Matrice\n\
\n\
    set Matrice(Popup) [tixPopupMenu $w.p -postcmd  { PostCmd }]\n\
    $w.p bind $w\n\
    set menu [$w.p subwidget menu]\n\
    $menu add command -label Delete -under 0 -command {\n\
			DeleteVar [lindex $Matrice(selezione) 0] } \n\
}",
"proc PostCmd { x y } {\n\
global	Matrice Nomi Start Congela\n\
\n\
if {$Congela == \"True\"} {\n\
        return 0\n\
        }\n\
\n\
if [ info exist Start ] {\n\
	if { $Start == \"1\" } {\n\
		return 0\n\
		}\n\
	}\n\
if [info exist Nomi([lindex $Matrice(selezione) 0]) ] {\n\
	$Matrice(Popup) configure -title \"I/O [lindex $Matrice(selezione) 0]\"\n\
	return 1\n\
	}\n\
return 0\n\
}",
"global tcl_platform VERSIONE HOME Congela NomeFile AutoCaricamento",
"global Lingua font_usato font_fisso FileLingua SAVEPATH",
"set VERSIONE \"1.2.0.1\"",
"	#########",
"	#       #",
"	# MAIN	#",
"	#	#",
"	#########",
"if { $tcl_platform(platform) == \"unix\" } {\n\
	set HOME /home/iosim\n\
	set SAVEPATH /home/d_xnc/plc\n\
  	set font_usato -adobe-helvetica-medium-r-normal--*-100-*-*-*-*-iso8859-*\n\
	set font_fisso -misc-fixed-medium-r-normal-*-*-*-*-*-*-*-*-*\n\
     } else {\n\
	set HOME C:\\home\\iosim\n\
	set SAVEPATH C:\\\n\
  	set font_usato -adobe-helvetica-medium-r-normal--*-140-*-*-*-*-iso8859-*\n\
	set font_fisso -misc-fixed-medium-r-normal-*-*-*-*-*-*-*-*-*\n\
\n\
	#####################################\n\
	# carico dal registro di windows 95 #\n\
	# il path di installazione	    #\n\
	#####################################\n\
	ReadDefaults\n\
}",
"if [file exist $HOME/default] {\n\
  # Caricamento default\n\
  source $HOME/default\n\
} ",
"set Ios_tools_library \"$HOME/lib\"",
"set auto_path [concat [list $Ios_tools_library] $auto_path]",
"if [file exist $HOME/lin_iosim] {\n\
  set fId [open $HOME/lin_iosim r]\n\
  set FileLingua [read $fId]\n\
  close $fId\n\
} else {\n\
 puts \"errore caricamento lingue\"\n\
}",
"bind Button <Return> {\n\
    %W invoke\n\
}",
"set Congela True",
"wm withdraw .",
"set w .iosim",
"toplevel $w ",
"bind $w <Destroy> {exit}",
"wm title $w \"[CaricaLingua $FileLingua Titolo]                   \\\n\
			(iosim [CaricaLingua $FileLingua Versione] $VERSIONE)\"",
"InterpretaOpzioni $w $argv $argc",
"set IconBitmap $HOME/icone/iosim.xbm",
"if [file exist $IconBitmap] {\n\
  wm iconbitmap $w \"@$IconBitmap\"\n\
}",
"get_pixmaps $w",
"if { [SwitchHost $w ] != \"OK\" } {\n\
	exit\n\
	}",
"windows $w ",
"wm minsize $w 400 200",
"Reload $w",
"if { $AutoCaricamento == \"True\" } {\n\
	set Matrice(NomeFile) $NomeFile\n\
	$Matrice(WidgetNomeFile) configure\\\n\
              -text \"[CaricaLingua $FileLingua Configuraz]: $Matrice(NomeFile)\"\n\
	update idletasks\n\
	ApriPure\n\
	}",
(char *) NULL
};

/*
 *----------------------------------------------------------------------
 *
 * main --
 *
 *	This is the main program for the application.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Whatever the application does.
 *
 *----------------------------------------------------------------------
 */

#if defined(__WIN32__) && defined(HAS_TK)
int APIENTRY
WinMain(hInstance, hPrevInstance, lpszCmdLine, nCmdShow)
    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    LPSTR lpszCmdLine;
    int nCmdShow;
{
    char **argv;
    int argc;
#else
int
#ifdef _USING_PROTOTYPES_
main (int    argc,		/* Number of command-line arguments. */
      char **argv)		/* Values of command-line arguments. */
#else
main(argc, argv)
    int argc;			/* Number of command-line arguments. */
    char **argv;		/* Values of command-line arguments. */
#endif
{
#endif
    Tcl_Interp *interp;
    char **p = script;
    char *q, buffer[256];
    Tcl_DString data;
    Tcl_Channel inChannel, outChannel, errChannel;

#ifdef __WIN32__
#if defined(TCL_ACTIVE) && !defined(SHARED) && defined(HAS_TK)
#ifndef CNIDLL
    TclWinSetTclInstance(hInstance);
#endif
    TkWinXInit(hInstance);
    Tcl_CreateExitHandler((Tcl_ExitProc *) TkWinXCleanup, (ClientData) hInstance);
#endif

    Tcl_SetPanicProc(TclshPanic);


    /*
     * Set up the default locale to be standard "C" locale so parsing
     * is performed correctly.
     */

    setlocale(LC_ALL, "C");

    /*
     * Increase the application queue size from default value of 8.
     * At the default value, cross application SendMessage of WM_KILLFOCUS
     * will fail because the handler will not be able to do a PostMessage!
     * This is only needed for Windows 3.x, since NT dynamically expands
     * the queue.
     */
    SetMessageQueue(64);

    argc = setargv(&argv);

    /*
     * Replace argv[0] with full pathname of executable, and forward
     * slashes substituted for backslashes.
     */

    GetModuleFileName(NULL, buffer, sizeof(buffer));
    argv[0] = buffer;
    for (q = buffer; *q != '\0'; q++) {
	if (*q == '\\') {
	    *q = '/';
	}
    }

#endif
    Tcl_FindExecutable(argv[0]);
    interp = Tcl_CreateInterp();

    q = Tcl_Merge(argc-1, argv+1);
    Tcl_SetVar(interp, "argv", q, TCL_GLOBAL_ONLY);
    ckfree(q);
    sprintf(buffer, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buffer, TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "argv0", argv[0],TCL_GLOBAL_ONLY);
    Tcl_SetVar(interp, "tcl_interactive","0", TCL_GLOBAL_ONLY);

    if (Tcl_Init(interp) != TCL_OK) {
        goto error;
    }
    if (Tk_Init(interp) != TCL_OK) {
        goto error;
    }
    Tcl_StaticPackage(interp, "Tk", Tk_Init, Tk_SafeInit);
    if (Tix_Init(interp) != TCL_OK) {
        goto error;
    }
    Tcl_StaticPackage(interp, "Tix", Tix_Init, Tix_SafeInit);

#ifdef TKTABLE 
    if (Tktable_Init(interp) != TCL_OK) {
        goto error;
    }
    Tcl_StaticPackage(interp, "Tktable", Tktable_Init, (Tcl_PackageInitProc *) NULL);
#endif 
#ifdef CNI 
  RegistraComandi(interp);
#endif 


    /*
     * Execute the script that is compiled in.
     */

    inChannel = Tcl_GetStdChannel(TCL_STDIN);
    outChannel = Tcl_GetStdChannel(TCL_STDOUT);
    Tcl_DStringInit(&data);
    while(*p) {
	Tcl_DStringSetLength(&data,0);
	Tcl_DStringAppend(&data,*p++,-1);
	if (Tcl_Eval(interp,Tcl_DStringValue(&data)) != TCL_OK) {
	    Tcl_DStringFree(&data);
	    while (p-- != script) {
		for (q = *p;*q; q++) {
		    if (*q=='\n') line++;
		}
		line++;
	    }
	    sprintf(buffer,lineformat,"script",line);
	    Tcl_AddErrorInfo(interp,"\n	( Error in file: \"");
	    Tcl_AddErrorInfo(interp,__FILE__);
	    Tcl_AddErrorInfo(interp,"\", line: ");
	    Tcl_AddErrorInfo(interp,buffer);
	    Tcl_AddErrorInfo(interp,")");
	    errChannel = Tcl_GetStdChannel(TCL_STDERR);
	    if (errChannel) {
		Tcl_Write(errChannel,
			Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY), -1);
		Tcl_Write(errChannel, "\n", 1);
	    }
#ifdef __WIN32__
	    TclshPanic(Tcl_GetVar(interp, "errorInfo", TCL_GLOBAL_ONLY));
#endif
	    sprintf(buffer, "exit %d", 1);
	    Tcl_Eval(interp, buffer);
	}
    }
    Tcl_DStringFree(&data);
    Tk_MainLoop();
    sprintf(buffer, "exit %d", 0);
    Tcl_Eval(interp, buffer);

error:
    errChannel = Tcl_GetStdChannel(TCL_STDERR);
    if (errChannel) {
	Tcl_Write(errChannel,
		"application-specific initialization failed: ", -1);
	Tcl_Write(errChannel, interp->result, -1);
	Tcl_Write(errChannel, "\n", 1);
    }
#ifdef __WIN32__
    TclshPanic(interp->result);
#endif
    sprintf(buffer, "exit %d", 1);
    Tcl_Eval(interp, buffer);
    return 0;
}

#ifdef __WIN32__
/*
 *----------------------------------------------------------------------
 *
 * TclshPanic --
 *
 *	Display a message and exit.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Exits the program.
 *
 *----------------------------------------------------------------------
 */

void
TclshPanic TCL_VARARGS_DEF(char *,arg1)
{
    va_list argList;
    char buf[1024];
    char *format;

    format = TCL_VARARGS_START(char *,arg1,argList);
    vsprintf(buf, format, argList);

    MessageBeep(MB_ICONEXCLAMATION);
    MessageBox(NULL, buf, "Fatal Error in Tclsh",
	    MB_ICONSTOP | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
#ifdef _MSC_VER
    _asm {
        int 3
    }
#endif
    ExitProcess(1);
}

/*
 *-------------------------------------------------------------------------
 *
 * setargv --
 *
 *	Parse the Windows command line string into argc/argv.  Done here
 *	because we don't trust the builtin argument parser in crt0.  
 *	Windows applications are responsible for breaking their command
 *	line into arguments.
 *
 *	2N backslashes + quote -> N backslashes + begin quoted string
 *	2N + 1 backslashes + quote -> literal
 *	N backslashes + non-quote -> literal
 *	quote + quote in a quoted string -> single quote
 *	quote + quote not in quoted string -> empty string
 *	quote -> begin quoted string
 *
 * Results:
 *	returns the number of arguments and fill argvPtr with the
 *	array of arguments.
 *
 * Side effects:
 *	Memory allocated.
 *
 *--------------------------------------------------------------------------
 */

static int
setargv(argvPtr)
    char ***argvPtr;		/* Filled with argument strings (malloc'd). */
{
    char *cmdLine, *p, *arg, *argSpace;
    char **argv;
    int argc, size, inquote, copy, slashes;

    cmdLine = GetCommandLine();

    /*
     * Precompute an overly pessimistic guess at the number of arguments
     * in the command line by counting non-space spans.
     */

    size = 2;
    for (p = cmdLine; *p != '\0'; p++) {
	if (isspace(*p)) {
	    size++;
	    while (isspace(*p)) {
		p++;
	    }
	    if (*p == '\0') {
		break;
	    }
	}
    }
    argSpace = (char *) ckalloc((unsigned) (size * sizeof(char *)
	    + strlen(cmdLine) + 1));
    argv = (char **) argSpace;
    argSpace += size * sizeof(char *);
    size--;

    p = cmdLine;
    for (argc = 0; argc < size; argc++) {
	argv[argc] = arg = argSpace;
	while (isspace(*p)) {
	    p++;
	}
	if (*p == '\0') {
	    break;
	}

	inquote = 0;
	slashes = 0;
	while (1) {
	    copy = 1;
	    while (*p == '\\') {
		slashes++;
		p++;
	    }
	    if (*p == '"') {
		if ((slashes & 1) == 0) {
		    copy = 0;
		    if ((inquote) && (p[1] == '"')) {
			p++;
			copy = 1;
		    } else {
			inquote = !inquote;
		    }
                }
                slashes >>= 1;
            }

            while (slashes) {
		*arg = '\\';
		arg++;
		slashes--;
	    }

	    if ((*p == '\0') || (!inquote && isspace(*p))) {
		break;
	    }
	    if (copy != 0) {
		*arg = *p;
		arg++;
	    }
	    p++;
        }
	*arg = '\0';
	argSpace = arg + 1;
    }
    argv[argc] = NULL;

    *argvPtr = argv;
    return argc;
}
#endif /* __WIN32__ */
