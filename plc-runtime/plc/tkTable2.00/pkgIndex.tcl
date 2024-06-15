if {[catch {package require Tcl 8.0}]} return
package ifneeded Tktable 2.00 "package require Tk 8.0; [list load [file join $dir Tktable.so.2.00] Tktable]"
