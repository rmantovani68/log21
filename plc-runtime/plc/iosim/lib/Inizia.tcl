#
#
#               #################################
#               #                               #
#               #       ReadDefaults            #
#               #                               #
#               #################################
#

proc ReadDefaults {} {
global HOME

set appoggio [ReadReg Applicazioni_Lae PATH]
if {$appoggio == "unix" } {
	set HOME /home/lae
	} elseif { $appoggio == "errore_registro" } {
		  tk_messageBox -icon info -message "Fallita lettura registro" -type ok
		  exit
		 } else {
		  set HOME $appoggio
		 }
}
#
#
#
#		#################################
#		#				#
#		#	get_dtm			#
#		#				#
#		#################################
#
proc	get_dtm { w } {
}


#               #################################
#               #                               #
#               #       get_pixmaps             #
#               #                               #
#               #################################
#
proc    get_pixmaps { w } {

  global HOME FrecciaDestra IconaExit IconaStart IconaStop IconaReload
  global file_sel1_image file_sel2_image FileLingua IconaInput IconaOutput



#################################
#	I N S E R I S C I	#

if [catch {set imagefile [open "$HOME/icone/insert.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] insert.xpm"  yes
    return
  }
set FrecciaDestra [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile



#################################
#	U S C I T A 		#

if [catch {set imagefile [open "$HOME/icone/exit.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] exit.xpm"  yes
    return
  }
set IconaExit [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile


#################################
#	  S T A R T		#

if [catch {set imagefile [open "$HOME/icone/Qsstart.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] Qsstart.xpm"  yes
    return
  }
set IconaStart [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile



#################################

#################################
#	  S T O P 		#

if [catch {set imagefile [open "$HOME/icone/Qssospe.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] Qssospe.xpm"  yes
    return
  }
set IconaStop [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile



#################################
#	  R E L O A D		#

if [catch {set imagefile [open "$HOME/icone/reload.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] reload.xpm"  yes
    return
  }
set IconaReload [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile



#################################
#	Icone INPUT/OUTPUT	#

if [catch {set imagefile [open "$HOME/icone/input.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] input.xpm"  yes
    return
  }
set IconaInput [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile
if [catch {set imagefile [open "$HOME/icone/output.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] output.xpm"  yes
    return
  }
set IconaOutput [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile


#################################
#	  Per file selector

if [catch {set imagefile [open "$HOME/icone/xncPROG.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] xncPROG.xpm"  yes
    return
  }
set file_sel1_image [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile


#################################
#	  Per file selector

if [catch {set imagefile [open "$HOME/icone/xncLOCAL.xpm" r] } out] {
    MessageBox $w "[CaricaLingua $FileLingua ErroreFile] xncLOCAL.xpm"  yes
    return
  }
set file_sel2_image [ list [image create pixmap -data [read $imagefile]] ]
close $imagefile


}

