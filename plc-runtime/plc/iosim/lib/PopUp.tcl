
##############################################################################
# Procedura che crea il popup menu' per la TKTABLE
##############################################################################
proc PopUpTkTable { w } {
global Matrice

    set Matrice(Popup) [tixPopupMenu $w.p -postcmd  { PostCmd }]
    $w.p bind $w
    set menu [$w.p subwidget menu]
    $menu add command -label Delete -under 0 -command {
			DeleteVar [lindex $Matrice(selezione) 0] } 
#    $menu add command -label Find    -under 0
}

proc PostCmd { x y } {
global	Matrice Nomi Start Congela

if {$Congela == "True"} {
        return 0
        }

if [ info exist Start ] {
	if { $Start == "1" } {
		return 0
		}
	}
if [info exist Nomi([lindex $Matrice(selezione) 0]) ] {
	$Matrice(Popup) configure -title "I/O [lindex $Matrice(selezione) 0]"
	return 1
	}
return 0
}
