#!/bin/bash
# The next line restarts using wishx \
exec wishx $0 ${1+"$@"}
#
# plcdeb.tcl 1.3 Sat Jul 29 22:30:15 CEST 2000
#
# Un divertissement il TCL/TK per dimostrare che qplc non scazza a mandare
# le informazioni di trace
#
# 26/07/00 GG 1.0 Prima stesura. Mancano ancora parecchie prestazioni,
#                 e anche i commenti!!!
# 27/07/00 GG 1.1 Quasi tutto funziona. Mancano ancora i commenti...
# 28/07/00 GG 1.2 Aggiunto il timeout sulla comunicazione.
# 29/07/00 GG 1.3 Tentativi di raccogliere/razionalizzare il codice.
#

global VERSION
set VERSION "1.3"

#if [ catch { infox } ] {
#	proc execl { x } {
#		set ch [ open "/tmp/plcdeb.execl" w ]
#		puts $ch {#!/bin/bash}
#		puts $ch {kill -9 $PPID}
#		puts $ch {exec $1}
#		close $ch
#		exec chmod 777 "/tmp/plcdeb.execl"
#		exec "/tmp/plcdeb.execl" "$x" <@ file0 >@ file1 2>@ file2
#	}
#}

global gl_var_refresh_time 
set gl_var_refresh_time 1000

global gl_max_var
set gl_max_var 8
global gl_max_trace_var
set gl_max_trace_var 8

global gl_fixed_font
set gl_fixed_font cnifxl

global gl_debug
set gl_debug 0
global gl_debug_reload
set gl_debug_reload 1

global gl_qplc_port
set gl_qplc_port 8039
global gl_host_name
set gl_host_name "localhost"
global gl_channel
set gl_channel -1
global gl_channel_timeout
set gl_channel_timeout 30000
global gl_plcstatus
set gl_plcstatus 0
global gl_n_task
set gl_n_task 0
global gl_max_n_task
set gl_max_n_task 10


# Ricezione di un messaggio da remoto

global gl_current_answer_data
set gl_current_answer_data ""
global gl_cmd_done
set gl_cmd_done 0

proc qplc_message {} {
global gl_channel
global handlers
global gl_current_answer_data
global gl_debug

	set eof 0
	if [ catch { set eof [ eof $gl_channel ] } ] {
		set eof 1
	}
	if {$eof} {
		do_disconnect
		tk_dialog .ok "Warning" "Connection closed" "" 0 "OK"
		return
	}
	if [ catch { gets $gl_channel line } ] {
		do_disconnect
		tk_dialog .ok "Warning" "I/O error - Connection closed" "" 0 "OK"
		return
	}
	if {$gl_debug} {
		puts "RECEIVED: $line"
	}
	set a_type [ string range "$line" 0 0 ]
	set a_data [ string range  "$line" 2 end ]
	set gl_current_answer_data "$gl_current_answer_data$a_data"
	if { [ string range "$line" 1 1 ] == "-" } {
	# Continuazione...
	} else {
		set s "$gl_current_answer_data"
		set gl_current_answer_data ""
		switch  "$a_type"  {
		"O" { $handlers(ok) "$s"; cmd_done    }
		"A" { $handlers(async_event) "$s"     }
		"D" { $handlers(data) "$s"; cmd_done  }
		"L" { $handlers(data_length) "$s"     }
		"E" { $handlers(error) "$s"; cmd_done }
		default { $handlers(unknown) "$s"; cmd_done }
		}
	}
}

proc print_handlers {} {
global handlers
puts "HANDLERS:\n ok = $handlers(ok)\n ae = $handlers(async_event)\n da = $handlers(data)\n dl = $handlers(data_length)\n er = $handlers(error)\n"
}

# Manipolatore generico di array di handler

proc install_handler_array { arr hlist } {
global $arr

	set rv {}
	set f 0
	foreach i $hlist {
		if {$f} {
			set f 0
		} else {
			set v [ array get $arr $i ]
			if {$v != {}} {
				set rv [ concat $rv $v ]
			}
			set f 1
		}
	}
	array set $arr $hlist
	return $rv
}

# Gestori dei segnali asincroni e procedure per registrarne degli alternativi.
global handlers_async
set handlers_async(error) default_handler_async_error
set handlers_async(halt) default_handler_async_halt
set handlers_async(go) default_handler_async_go
set handlers_async(fault) default_handler_async_fault
set handlers_async(trace) default_handler_async_trace
set handlers_async(trigger) default_handler_async_trigger
set handlers_async(delerr) default_handler_async_delerr
set handlers_async(message) default_handler_async_message
set handlers_async(unknown) default_handler_async_unknown

proc default_handler_async_error {x} { }
proc default_handler_async_halt {x} {
	stop_vardeb_scheduler
	update_plc_status 0
}
proc default_handler_async_go {x} {
	update_plc_status 1
}
proc default_handler_async_fault {x} {
	stop_vardeb_scheduler
	update_plc_status 2
}
proc default_handler_async_trace {x} { }
proc default_handler_async_trigger {x} { }
proc default_handler_async_delerr {x} { }
proc default_handler_async_message {x} { }
proc default_handler_async_unknown {x} { }

# Installatore dei gestori di default.
proc install_default_handler_async {} {
	return [ install_handler_array handlers_async {
		 error default_handler_async_error 
		 halt default_handler_async_halt 
		 go default_handler_async_go 
		 fault default_handler_async_fault 
		 trace default_handler_async_trace 
		 delerr default_handler_async_delerr 
		 message default_handler_async_message 
		 trigger default_handler_async_trigger 
		 unknown default_handler_async_unknown 
		} ]
}

# Installatore di gestori indicati dalla lista data come parametro.
# La lista e` composta da coppie "parola-procedura". E` restituita
# la lista dei gestori corrente

proc install_handler_async { hlist } {
	return [ install_handler_array handlers_async $hlist ]
}

# Gestori di messaggio (default)
global handlers
set handlers(ok) default_handler_ok
set handlers(async_event) default_handler_async_event
set handlers(data) default_handler_data
set handlers(data_length) default_handler_data_length
set handlers(error) default_handler_error
set handlers(unknown) default_handler_unknown

# Gestori delle risposte (dati, errori, eventi asincroni...)
# e procedure per registrarne dei nuovi.

# Installa i gestori di default
proc install_default_handlers {} {
	return [ install_handler_array handlers {
		 ok default_handler_ok 
		 async_event default_handler_async_event 
		 data default_handler_data 
		 data_length default_handler_data_length 
		 error default_handler_error 
		 unknown default_handler_unknown 
		} ]
}

# Installatore di gestori indicati dalla lista data come parametro.
# La lista e` composta da coppie "parola-procedura". E` restituita
# la lista dei gestori corrente

proc install_handlers { hlist } {
	return [ install_handler_array handlers $hlist ]
}

proc default_handler_ok {x} { }
proc default_handler_data {x} { }
proc default_handler_data_length {x} { }
proc default_handler_error {x} { }
proc default_handler_unknown {x} { }
proc default_handler_async_event {x} {
global handlers_async
global gl_debug

	set type [ lindex $x 0 ]
	set data [ lrange $x 1 end ]
	if {$gl_debug} {
		puts "ASYNC EVENT: type=$type, data=$data"
	}
	switch "$type" {
	0 { $handlers_async(error) "$data"   }
	1 { $handlers_async(halt) "$data"    }
	2 { $handlers_async(go) "$data"      }
	3 { $handlers_async(fault) "$data"   }
	4 { $handlers_async(trace) "$data"   }
	5 { $handlers_async(trigger) "$data" }
	6 { $handlers_async(delerr)  "$data" }
	7 { $handlers_async(message) "$data" }
	default { $handlers_async(unknown) "$data" }
	}
}

# Primitive per l'invio di messaggi

proc check_session {} {
global gl_channel
	if { $gl_channel == -1 } {
		error "Session terminated"
	}
}

proc check_var_and_session { x } {
	tkwait variable $x
	check_session
}

proc cmd_not_done {} {
global gl_cmd_done
	set gl_cmd_done 0
}

proc cmd_done {} {
global gl_cmd_done
	set gl_cmd_done 1
}

proc wait_cmd_done {} {
global gl_cmd_done
	check_var_and_session gl_cmd_done
}

global gl_congestion_signaled
set gl_congestion_signaled 0
global gl_congestion_abort
set gl_congestion_abort 0
global gl_command_queue
set gl_command_queue {}

proc qplc_send_start_timeout {} {
global gl_channel_timeout
global gl_channel_toid
	set gl_channel_toid [ after $gl_channel_timeout qplc_send_timeout ]
}
proc qplc_send_stop_timeout {} {
global gl_channel_timeout
global gl_channel_toid
	catch { after cancel $gl_channel_toid }
}
proc qplc_send_timeout {} {
	set rv [tk_dialog .to_dlg "Error" \
		"Communication timed out" "" \
		0 "RETRY" "DISCONNECT" ]
	if {$rv == 1} {
		do_disconnect
	} else {
		qplc_send_start_timeout
	}
}
proc qplc_send { msg  hlist } {
global gl_channel

	qplc_send_start_timeout
	cmd_not_done
	set old_hnd [ install_handlers $hlist ]
	puts $gl_channel "$msg"; flush $gl_channel
	wait_cmd_done
	qplc_send_stop_timeout
	install_handlers $old_hnd
}

proc init_queue {} {
global gl_n_task
global gl_congestion_signaled
global gl_congestion_abort
global gl_command_queue

	set gl_n_task 0
	set gl_congestion_signaled 0
	set gl_congestion_abort 0
	set gl_command_queue {}
}

proc enqueue_job { fn } {
global gl_n_task
global gl_max_n_task
global gl_congestion_signaled
global gl_congestion_abort
global gl_command_queue
global gl_debug

	incr gl_n_task
if {$gl_debug} { puts "ENQUEUE_JOB: task=$gl_n_task job=$fn"}
	if { $gl_n_task > $gl_max_n_task } {
		if {$gl_congestion_signaled} {
		# ???
		} else {
			set gl_congestion_signaled 1
			catch { destroy .err_dlg }
			set gl_congestion_abort [ \
				tk_dialog .err_dlg "Error" \
				"Channel congestion" "" \
				0 "RETRY" "GIVE UP" ]
			set gl_congestion_signaled 0
			catch { destroy .err_dlg }
		}
		if {$gl_congestion_abort} {
			incr gl_n_task -1
			return 0
		}
	}

	lappend gl_command_queue $fn

	return 1
}

proc dequeue_job {} {
global gl_n_task
global gl_command_queue
global gl_debug

	set rv [ lindex $gl_command_queue 0 ]
	set gl_command_queue [ lrange $gl_command_queue 1 end ]
	if { $rv != "" } {
		incr gl_n_task -1
	}
if {$gl_debug} {puts "DEQUEUE_JOB: task=$gl_n_task job=$rv"}
	return $rv
}

proc post_qplc_job { fn } {
global gl_doing_command
global gl_debug

	if {$gl_doing_command} {
		enqueue_job $fn
	} else {
		set gl_doing_command 1
		set cmd $fn
		while { $cmd != {} } {
if {$gl_debug} {puts "POST_QPLC_JOB: job=$fn"}
			eval $cmd
			set cmd [ dequeue_job ]
		}
		set gl_doing_command 0
	}
}

# Connessione al sistema remoto
proc do_connection {} {
global gl_qplc_port
global gl_host_name
global gl_channel
global gl_plcstatus
global gl_current_answer_data
global gl_cmd_done
global gl_doing_command

	if { $gl_channel != -1 } {
		do_disconnect
	}

	set gl_current_answer_data ""
	set gl_cmd_done 0
	set gl_doing_command 0

	install_default_handlers
	install_default_handler_async

	if [ catch {
			set gl_channel [ socket $gl_host_name $gl_qplc_port ]
		} error_code ] {
		tk_dialog .ok_cancel "Error" "$error_code" "" 0 "OK"
		return 0
	} else {
		set gl_plcstatus 0
		update_status
		fileevent $gl_channel readable qplc_message
	}

	return 1
}

# Disconnessione
proc do_disconnect {} {
global gl_channel
global gl_current_answer_data
global gl_cmd_done
global gl_n_task
global gl_congestion_signaled
global gl_command_queue
global gl_doing_command

catch {
	stop_vardeb_scheduler
	if { $gl_channel != -1 } {
		fileevent $gl_channel readable ""
		catch { puts $gl_channel "\nquit" }
		catch { close $gl_channel }
		set gl_channel -1
		set gl_current_answer_data ""
		set gl_cmd_done 0
		set gl_doing_command 0
		set gl_command_queue {}
	}
	destroy_trace_toplevel
	init_var_list
	update_status
}
}

# Aggiornamento dello stato
proc update_plc_status { x } {
global gl_plcstatus

	set gl_plcstatus $x
	update_status
}

proc update_buttons { x } {
	if {$x} {
		.buttons.debug configure -state normal
		.buttons.trace configure -state normal
	} else {
		.buttons.debug configure -text "Start Debug" \
		                         -command start_debug \
	                                 -state disabled
		.buttons.trace configure -text "Start Trace" \
		                         -command start_trace \
	                                 -state disabled
	}
}

proc update_status {} {
global gl_channel
global gl_plcstatus
global gl_host_name

	if { $gl_channel == -1 } {
		.statusbar.host configure -text "Disconnected" \
		                          -background yellow
		.statusbar.plcstatus configure -text "?" \
		                           -background gray
		.menu.connect entryconfigure 0 -label "Connect" \
		                               -command connect_qplc
		.menu entryconfigure 2 -state disabled
		.menu.options entryconfigure 1 -state disabled
		.menu.windows entryconfigure 1 -state disabled
		update_buttons 0
	} else {
		.statusbar.host configure -text "$gl_host_name" \
		                          -background cyan
		.menu entryconfigure 2 -state normal
		.menu.windows entryconfigure 1 -state normal
		.menu.connect entryconfigure 0 -label "Disconnect" \
		                               -command disconnect_qplc
		.menu.options entryconfigure 1 -state normal
		if { $gl_plcstatus == 0 } {
			.statusbar.plcstatus configure -text "HALT" \
		                           -background red
			.menu.actions entryconfigure 1 -label "GO PLC" \
					   -command { post_qplc_job do_go } \
			                   -state normal
			.menu.actions entryconfigure 0 -state normal
			update_buttons 0
		} else {
			if { $gl_plcstatus == 1 } {
				.statusbar.plcstatus configure -text "GO" \
		                           -background green
				.menu.actions entryconfigure 1 \
				           -label "HALT PLC" \
					   -command { post_qplc_job do_halt } \
				           -state normal
				.menu.actions entryconfigure 0 -state disabled
				update_buttons 1
			} else {
				.statusbar.plcstatus configure -text "FAULT" \
		                           -background yellow
				.menu.actions entryconfigure 1 \
				           -label "GO PLC" \
					   -command { post_qplc_job do_go } \
				           -state disabled
				.menu.actions entryconfigure 0 -state disabled
				update_buttons 0
			}
		}
	}
}

# Selezione del periodo ri refresh delle variabili
proc set_refresh_time {} {
	catch { destroy .selecttime }
	toplevel .selecttime
	scale .selecttime.scale -label "Refresh rate (ms): " \
		-orient horizontal -length 200 -width 40 \
		-from 100 -to 5000 -showvalue 1 -variable gl_var_refresh_time 
	button .selecttime.ok -text "OK" -command { destroy .selecttime }
	pack .selecttime.scale .selecttime.ok -fill x -pady 2m
}

# Creazione della shell per la selezione dell'host

proc make_connection {} {
global gl_qplc_port
global gl_host_name
	set gl_host_name [ .selecthost.f1.hostname get ]
	set gl_qplc_port [ .selecthost.f2.hostport get ]
	if [ do_connection ] {
		destroy .selecthost
		post_qplc_job do_initial_inquiry
	}
}
proc create_conntop {} {
global gl_qplc_port
global gl_host_name

	catch { destroy .selecthost }
	toplevel .selecthost
	frame .selecthost.f1
	label .selecthost.f1.hosttitle -text "Host name: "
	entry .selecthost.f1.hostname -width 25
	.selecthost.f1.hostname insert 0 $gl_host_name
	pack .selecthost.f1.hosttitle .selecthost.f1.hostname -side left
	frame .selecthost.f2
	label .selecthost.f2.porttitle -text "Port number: "
	entry .selecthost.f2.hostport -width 5
	.selecthost.f2.hostport insert 0 $gl_qplc_port
	pack .selecthost.f2.porttitle .selecthost.f2.hostport -side left
	frame .selecthost.f3
	button .selecthost.f3.ok -text "OK" -command make_connection
	bind .selecthost.f1.hostname <Key-Return> make_connection
	button .selecthost.f3.cancel -text "CANCEL" -command {
		destroy .selecthost
	}
	pack .selecthost.f3.ok .selecthost.f3.cancel -side left -expand 1
	pack .selecthost.f1 .selecthost.f2
	pack .selecthost.f3 -fill x -pady 2m
	focus .selecthost.f1.hostname
}

proc connect_qplc {} {
	create_conntop
}

proc disconnect_qplc {} {
	do_disconnect
}

# Array contenenti le caratteristiche delle variabili
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size

proc create_gui {} {
global gl_debug
global gl_debug_reload
global gl_fixed_font
global gl_max_var
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size
global VERSION

# Creazione dei menu`
	. configure -heigh 300 -width 400
	wm title . "Plc Debug $VERSION"
	menu .menu -tearoff 0
	menu .menu.file -tearoff 0
	.menu add cascade -label "File" -menu .menu.file -underline 0
	if {$gl_debug_reload} {
		.menu.file add command -label "Reload PlcDeb" \
			-command { execl "./plcdeb.tcl" }
		.menu.file add checkbutton -label "Log status" \
			-onvalue 1 -offvalue 0 -variable gl_debug
		.menu.file add separator
	}
	.menu.file add command -label "Load variable list" -command load_vars
	.menu.file add command -label "Save variable list" -command save_vars
	.menu.file add separator
	.menu.file add command -label "Quit" -command { exit 0 }
	menu .menu.connect -tearoff 0
	.menu add cascade -label "Connection" -menu .menu.connect -underline 0
	.menu.connect add command -label "Connect" -command connect_qplc
	menu .menu.actions -tearoff 0
	.menu add cascade -label "Commands" -menu .menu.actions -underline 0 \
	                  -state disabled
	.menu.actions add command -label "Load program" -command load_pr
	.menu.actions add command -label "Halt program" -command halt_pr
	menu .menu.options -tearoff 0
	.menu add cascade -label "Options" -menu .menu.options -underline 0
	.menu.options add command -label "Set update time" \
		-command set_refresh_time
	.menu.options add command -label "Change trace slice size" \
		-command set_trace_slice_size
	.menu.options add command -label "Change trace buffer size" \
		-command set_trace_buffer_size
	menu .menu.windows -tearoff 0
	.menu add cascade -label "Windows" -menu .menu.windows -underline 0
	.menu.windows add command -label "Trace" -command show_trace_toplevel
	.menu.windows add cascade -label "Info" \
		-menu .menu.windows.info -underline 0
	.menu.windows add command -label "Errors" -command show_err_toplevel
	.menu.windows add command -label "Messages" -command show_msg_toplevel
	.menu.windows add command -label "Statistics" -command show_stat_toplevel
	menu .menu.windows.info -tearoff 0
	.menu.windows.info add command -label "PLC setup" \
		-command {show_info setup}
	.menu.windows.info add command -label "Modules" \
		-command {show_info modules}
	.menu.windows.info add command -label "Shared variables" \
		-command {show_info shvars}
	.menu.windows.info add command -label "IPC" \
		-command {show_info ipcs}
	. configure -menu .menu

# Creazione della barra di stato
	frame .statusbar
	label .statusbar.host -justify left -text "Disconnected" \
	                      -width 30 -relief sunken
	label .statusbar.plcstatus -justify center -text "HALT" \
	                      -width 10 -relief sunken
	pack .statusbar.host -side left
	pack .statusbar.plcstatus -side right
	pack .statusbar -side top

# Creazione del sinottico delle variabili
	frame .varlist
	label .varlist.title_name -width 10 -text "Name"
	label .varlist.title_idx1 -text "Idx1"
	label .varlist.title_idx2 -text "Idx2"
	label .varlist.title_size -text "Size"
	label .varlist.title_value -text "Value"
	grid configure .varlist.title_name .varlist.title_idx1 \
	               .varlist.title_idx2 .varlist.title_size \
	               .varlist.title_value

	clear_var_list
	for {set i 1} {$i <= $gl_max_var} {incr i} {
		entry .varlist.varname$i -width 12 -state normal \
			-font $gl_fixed_font -textvariable gl_var_name($i)
		entry .varlist.idx1_$i -width 2 -state normal \
			-font $gl_fixed_font -textvariable gl_var_idx1($i)
		entry .varlist.idx2_$i -width 2 -state normal \
			-font $gl_fixed_font -textvariable gl_var_idx2($i)
		tk_optionMenu .varlist.varsize$i gl_var_size($i) 1 2 4
		label .varlist.varval$i -width 10 -relief sunken \
			-font $gl_fixed_font
		.varlist.varval$i configure -relief sunken
		grid configure .varlist.varname$i \
		               .varlist.idx1_$i \
		               .varlist.idx2_$i \
		               .varlist.varsize$i \
		               .varlist.varval$i \
		                -row $i
	}

	pack .varlist -side top

# I bottoni
	frame .buttons
	button .buttons.debug -text "Start Debug" -command start_debug \
	                      -state disabled
	button .buttons.trace -text "Start Trace" -command start_trace \
	                      -state disabled
	pack .buttons.debug .buttons.trace -side left -expand 1
	pack .buttons -side bottom -fill x -pady 2m
}

# Comando "info".
global gl_qplc_info_result

proc qplc_info_data {x} {
global gl_qplc_info_result

	set gl_qplc_info_result "$x"
}

proc qplc_info_error {x} {
global gl_qplc_info_result

	set gl_qplc_info_result ""
}

proc do_info {} {
global gl_qplc_info_result

	qplc_send "info"  {
		data qplc_info_data
		error qplc_info_error
		}
	return $gl_qplc_info_result
}

# Comando "ipcs".
global gl_qplc_ipcs_result

proc qplc_ipcs_data {x} {
global gl_qplc_ipcs_result

	set gl_qplc_ipcs_result "$x"
}

proc qplc_ipcs_error {x} {
global gl_qplc_ipcs_result

	set gl_qplc_ipcs_result ""
}

proc do_ipcs {} {
global gl_qplc_ipcs_result

	qplc_send "ipcs"  {
		data qplc_ipcs_data
		error qplc_ipcs_error
		}
	return $gl_qplc_ipcs_result
}

# Comando "free".
global gl_qplc_free_result

proc qplc_free_data {x} {
global gl_qplc_free_result

	set gl_qplc_free_result "$x"
}

proc qplc_free_error {x} {
global gl_qplc_free_result

	set gl_qplc_free_result ""
}

proc do_free {} {
global gl_qplc_free_result

	qplc_send "free"  {
		data qplc_free_data
		error qplc_free_error
		}
	return $gl_qplc_free_result
}

# Comando "mod".
global gl_qplc_mod_result

proc qplc_mod_data {x} {
global gl_qplc_mod_result

	set gl_qplc_mod_result "$x"
}

proc qplc_mod_error {x} {
global gl_qplc_mod_result

	set gl_qplc_mod_result ""
}

proc do_mod { name } {
global gl_qplc_mod_result

	qplc_send "mod $name"  {
		data qplc_mod_data
		error qplc_mod_error
		}
	return $gl_qplc_mod_result
}

# Comando "var".
global gl_qplc_var_result

proc qplc_var_data {x} {
global gl_qplc_var_result

	set gl_qplc_var_result "$x"
}

proc qplc_var_error {x} {
global gl_qplc_var_result

	set gl_qplc_var_result ""
}

proc do_var { name idx1 idx2 } {
global gl_qplc_var_result

	qplc_send "var $name $idx1 $idx2"  {
		data qplc_var_data
		error qplc_var_error
		}
	return $gl_qplc_var_result
}

# Comando "memchk".
global gl_qplc_memchk_result

proc qplc_memchk_data {x} {
global gl_qplc_memchk_result

	set gl_qplc_memchk_result "$x"
}

proc qplc_memchk_error {x} {
global gl_qplc_memchk_result

	set gl_qplc_memchk_result ""
}

proc do_memchk { mems } {
global gl_qplc_memchk_result

	qplc_send "memchk $mems"  {
		data qplc_memchk_data
		error qplc_memchk_error
		}
	return $gl_qplc_memchk_result
}

# Comando "trace a".
global gl_qplc_trace_a_result

proc qplc_trace_a_data {x} {
global gl_qplc_trace_a_result

	set gl_qplc_trace_a_result "$x"
}

proc qplc_trace_a_error {x} {
global gl_qplc_trace_a_result

	set gl_qplc_trace_a_result ""
}

proc do_trace_a { mems } {
global gl_qplc_trace_a_result

	qplc_send "trace a $mems"  {
		data qplc_trace_a_data
		error qplc_trace_a_error
		}
	return $gl_qplc_trace_a_result
}

# Comando "trace m".
proc qplc_trace_m_error {x} {
	tk_dialog .ok_cancel "Error" \
		"Error doing \"trace m\" command\nCode = $x" "" 0 "OK"
}

proc do_trace_m { val } {
	qplc_send "trace m $val"  { error qplc_trace_m_error }
}

# Comando "trace c".
proc do_trace_c {} {
	qplc_send "trace c"  {}
}

# Comando "trace e".
proc do_trace_e {} {
	qplc_send "trace e"  {}
}


# Comando "trace v".
global gl_qplc_trace_v_result

proc qplc_trace_v_data {x} {
global gl_qplc_trace_v_result

	set gl_qplc_trace_v_result "$x"
}

proc qplc_trace_v_error {x} {
global gl_qplc_trace_v_result

	set gl_qplc_trace_v_result ""
}

proc do_trace_v {} {
global gl_qplc_trace_v_result

	qplc_send "trace v"  {
		data qplc_trace_v_data
		error qplc_trace_v_error
		}
	return $gl_qplc_trace_v_result
}

# Comando "memcopy".
global gl_qplc_memcopy_result

proc qplc_memcopy_data {x} {
global gl_qplc_memcopy_result

	set gl_qplc_memcopy_result "$x"
}

proc qplc_memcopy_error {x} {
global gl_qplc_memcopy_result

	set gl_qplc_memcopy_result ""
}

proc do_memcopy { } {
global gl_qplc_memcopy_result

	qplc_send "memcopy"  {
		data qplc_memcopy_data
		error qplc_memcopy_error
		}
	return $gl_qplc_memcopy_result
}


# Interrogazione di qplc per la convalida delle variabili selezionate.

proc mark_var { n x } {
	if {$x} {
		.varlist.varval$n configure -text "" -foreground black
	} else {
		.varlist.varval$n configure -text "<ERR>" -foreground red
	}
}

global gl_var_list
global gl_var_list_nvar
set gl_var_list_nvar 0
global gl_var_list_usage
set gl_var_list_usage 0

proc init_var_list {} {
global gl_var_list_usage
global gl_var_list
global gl_var_list_nvar
global gl_max_var
	set gl_var_list_nvar 0
	set gl_var_list_usage 0
	for {set i 1} {$i <= $gl_max_var} {incr i} {
		set gl_var_list($i) {}
	}
}

proc lock_var_list {} {
global gl_var_list_usage
	set rv $gl_var_list_usage
	incr gl_var_list_usage
	return $rv
}

proc free_var_list {} {
global gl_var_list_usage
	if { $gl_var_list_usage > 0 } {
		incr gl_var_list_usage -1
		return $gl_var_list_usage
	}
	return 0
}

proc make_var_list {} {
global gl_var_list
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size
global gl_debug
global gl_max_var
global gl_var_list_nvar

	if [ lock_var_list ] {
		return $gl_var_list_nvar
	}
	set nvar 0
	for {set i 1} {$i <= $gl_max_var} {incr i} {
		set name [ string trim $gl_var_name($i) ]
		set idx1 [ string trim $gl_var_idx1($i) ]
		set idx2 [ string trim $gl_var_idx2($i) ]
		set size $gl_var_size($i)
		if { "$name" != "" } {
			set rv [ do_var "$name" "$idx1" "$idx2" ]
			if {$gl_debug} {
				puts "VAR $name $idx1 $idx2 --> rv= $rv"
			}

			if { "$rv" != "" } {
				mark_var $i 1
				if { [ lindex "$rv" 0 ] == "S" } {
				# Segnale di scambio
					set size [ lindex "$rv" 5 ] 
					set gl_var_size($i) $size
				} else {
				# Variabile interna
				}
				set gl_var_list($i) \
				    [ list [ lindex "$rv" 1 ] $size ]
				incr nvar
			} else {
				mark_var $i 0
			}

		} else {
			set gl_var_list($i) {}
		}
	}
	set gl_var_list_nvar $nvar
	return $nvar
}

#
# Visualizzazione periodica delle variabili
#

global gl_var_totmem
global gl_var_index_list
global gl_vardeb_timer
global gl_vardeb_timer_stop
set gl_vardeb_timer_stop 0

proc arm_vardeb_timer {} {
global gl_vardeb_timer
global gl_var_refresh_time
global gl_vardeb_timer_stop
	if { $gl_vardeb_timer_stop == 0 } {
		set gl_vardeb_timer [ after $gl_var_refresh_time \
			{ post_qplc_job vardeb_scheduler } ]
	}
}

proc vardeb_scheduler {} {
global gl_var_totmem
global gl_var_index_list
global gl_var_list
global gl_debug

	set rv [ do_memcopy ]
	if {$gl_debug} {
		puts "MEMCOPY  --> $rv"
	}
	if { "$rv" != "" } {
		set pos 0
		foreach i $gl_var_index_list {
			set nch [ expr [ lindex $gl_var_list($i) 1 ] * 2 ]
			set pos2 [ expr $pos + $nch ]
			set val [ string range "$rv" $pos [ expr $pos2 - 1 ] ]
			set v "0x"
			for {set j [expr $nch-2]} {$j>=0} {set j [expr $j-2]} {
				set v $v[ string range "$val" $j [expr $j+1]]
			}
			.varlist.varval$i configure -text "$v"
			set pos $pos2
		}
	}
	arm_vardeb_timer
}

proc start_vardeb_scheduler {} {
global gl_vardeb_timer_stop
	set gl_vardeb_timer_stop 0
	vardeb_scheduler
}

proc stop_vardeb_scheduler {} {
global gl_vardeb_timer
global gl_vardeb_timer_stop
	set gl_vardeb_timer_stop 1
	catch { after cancel $gl_vardeb_timer }
}

#
# Codice comune a debug periodico e trace
#

proc start_debug_or_trace_core { trace } {
global gl_var_list
global gl_var_index_list
global gl_var_totmem
global gl_debug
global gl_max_var

	if [ make_var_list ] {
		set mems ""
		for {set i 1} {$i <= $gl_max_var} {incr i} {
			if { $gl_var_list($i) != {} } {
				if {$trace} {
				# Il comando "trace" non vuole la dimensione
					set s [ lindex $gl_var_list($i) 0 ]
				} else {
					set s $gl_var_list($i)
				}
				set mems "$mems $s"
			}
		}
		if {$trace} {
			do_trace_c
			set rv [ do_trace_a "$mems" ]
			if {$gl_debug} {
				puts "TRACE A $mems --> $rv"
			}
		} else {
			set rv [ do_memchk "$mems" ]
			if {$gl_debug} {
				puts "MEMCHK $mems --> $rv"
			}
		}
		if { "$rv" == "" } {
			set rv 0
		} else {
			set ni 0
			set gl_var_totmem 0
			set gl_var_index_list {}
			for {set i 1} {$i <= $gl_max_var} {incr i} {
				if { $gl_var_list($i) != {} } {
					if { [ lindex "$rv" $ni ] == 0 } {
						mark_var $i 0
					} else {
						if {$trace} {
							set dim 1
						} else {
							set dim \
			[ lindex $gl_var_list($i) 1 ]
						}
						set gl_var_totmem \
		[ expr $gl_var_totmem +  $dim ]
						set gl_var_index_list \
		[ concat $gl_var_index_list $i ]
					}
					incr ni
				}
			}
			if {$gl_debug} {
				puts "TOTMEM = $gl_var_totmem"
				puts "INDEX LIST = $gl_var_index_list"
			}
			if { $gl_var_totmem == 0 } {
				set rv 0
			} else {
				set rv 1
			}
		}
	} else {
		set rv 0
	}
	return $rv
}

proc start_debug_core {} {
global gl_var_list
global gl_var_index_list
global gl_var_totmem
global gl_debug
global gl_max_var

	set rv [ start_debug_or_trace_core 0 ]
	if {$rv} {
		start_vardeb_scheduler
	}	
	return $rv
}

# Inizio/fine della fase di debug.
proc start_debug {} {
	.buttons.debug configure -state disabled
	post_qplc_job start_debug_step2
}
proc start_debug_step2 {} {
	if [ start_debug_core ] {
		.buttons.debug configure -text "Stop Debug" -command stop_debug
	}
	.buttons.debug configure -state normal
}
proc stop_debug {} {
	stop_vardeb_scheduler
	free_var_list
	.buttons.debug configure -text "Start Debug" -command start_debug
}

#
# Gestione del trace.
#

global gl_old_trace_handler
set gl_old_trace_handler { trace default_handler_async_trace }

global h2btab
set h2btab(0) { 0 0 0 0 }
set h2btab(1) { 1 0 0 0 }
set h2btab(2) { 0 1 0 0 }
set h2btab(3) { 1 1 0 0 }
set h2btab(4) { 0 0 1 0 }
set h2btab(5) { 1 0 1 0 }
set h2btab(6) { 0 1 1 0 }
set h2btab(7) { 1 1 1 0 }
set h2btab(8) { 0 0 0 1 }
set h2btab(9) { 1 0 0 1 }
set h2btab(A) { 0 1 0 1 }
set h2btab(B) { 1 1 0 1 }
set h2btab(C) { 0 0 1 1 }
set h2btab(D) { 1 0 1 1 }
set h2btab(E) { 0 1 1 1 }
set h2btab(F) { 1 1 1 1 }

proc make_binary_list { x } {
global h2btab
	set l [ string length $x ]
	set rv {}
	for {set i 0} {$i < $l} {incr i 7} {
		set ch1 [ string range $x $i $i ]
		incr i
		set ch2 [ string range $x $i $i ]
		lappend rv [ concat $h2btab($ch2) $h2btab($ch1) ]
	}
	return $rv
}

global gl_trace_history
global gl_max_trace_length
set gl_max_trace_length 640

proc clear_trace_history {} {
global gl_trace_history

	set gl_trace_history {}
}

proc expand_trace_history { s } {
global gl_trace_history
global gl_max_trace_length

	set bl [ make_binary_list $s ]
	set gl_trace_history [ concat $gl_trace_history $bl ]
	set len [ llength $gl_trace_history ]
	if { $len > $gl_max_trace_length } {
		set gl_trace_history [ lrange $gl_trace_history \
			[expr $len - $gl_max_trace_length] end ]
	}
}

global gl_trace_level1_height
set gl_trace_level1_height 10
global gl_trace_var_height
set gl_trace_var_height 40

proc clear_var_index {} {
global gl_var_index_list
	set gl_var_index_list {}
}

proc trace_view_core {} {
global gl_var_index_list
global gl_var_list
global gl_var_name
global gl_var_list_nvar
global gl_trace_history
global gl_trace_level1_height
global gl_trace_var_height
global gl_max_trace_var
global gl_max_trace_length
global gl_trace_label_canvas_width
global gl_trace_view_width
global gl_trace_toplevel_done

	if {! $gl_trace_toplevel_done} {
		return
	}
	clear_traces

	if {$gl_var_list_nvar > $gl_max_trace_var} {
		set limit $gl_max_trace_var
	} else {
		set limit $gl_var_list_nvar
	}
	if {$gl_trace_history != {}} {
	    for {set i 0} {$i < $limit} {incr i} {
		set lc { .trace.c.c create line }
		set x 0
		set y [ expr $i*$gl_trace_var_height+$gl_trace_var_height ]
		set of -1
		foreach j $gl_trace_history {
			set f [ lindex $j $i ]
			if { $f != $of } {
				if { $of != -1 } {
					lappend lc $x \
			 [ expr $y-$of*$gl_trace_level1_height ]
				}
				lappend lc $x \
		[ expr $y-$f*$gl_trace_level1_height ]
				set of $f
			}
			incr x
		}
		lappend lc $x [ expr $y-$f*$gl_trace_level1_height ] \
			-width 3 -fill red -tags [ list traces trace_$i ]
		eval $lc
	    }
	}
	set m [ expr  ([llength $gl_trace_history].0-$gl_trace_view_width.0)\
		/ $gl_max_trace_length.0 ]
	.trace.c.c xview moveto $m
}
proc trace_view { s } {
	expand_trace_history $s
	trace_view_core
}

proc trace_handler_ignore { x } {
}

proc trace_handler_overrun { x } {
global gl_trace_overrun_handler
	set gl_trace_overrun_handler { trace trace_handler_ignore }
	install_handler_async $gl_trace_overrun_handler
	tk_dialog .overrun "Error" "Trace overrun\nSome event will be lost" \
			 "" 0 "OK"
}

proc trace_event_handler_core { x } {
global gl_trace_overrun_handler
	set curr_hnd [ install_handler_async $gl_trace_overrun_handler ]
	set rv [ do_trace_v ]
	install_handler_async $curr_hnd
	trace_view $rv
}

proc trace_event_handler {x} {
	post_qplc_job [ list trace_event_handler_core $x ]
}

proc start_trace_scheduler {} {
global gl_old_trace_handler
global gl_trace_overrun_handler

	set gl_trace_overrun_handler { trace trace_handler_overrun }
	set gl_old_trace_handler [ install_handler_async {
		 		trace trace_event_handler
			} ]
	post_qplc_job do_trace_e
	return 1
}

proc start_trace_core {} {
	set rv [ start_debug_or_trace_core 1 ]
	if {$rv} {
	 	set rv [ start_trace_scheduler ]
	}	
	return $rv
}

proc stop_trace_core {} {
global gl_old_trace_handler
	install_handler_async $gl_old_trace_handler
	free_var_list
	post_qplc_job do_trace_c
}

global gl_traced_name_list
set gl_traced_name_list {}

proc trace_scroll_c_and_lbl { args } {
	eval .trace.c.c yview $args
	eval .trace.c.lbl yview $args
}

global gl_trace_label_canvas_width
set gl_trace_label_canvas_width 100
global gl_trace_view_width
set gl_trace_view_width 400

proc clear_traces {} {
	catch { .trace.c.c delete "traces" }
}

global gl_trace_toplevel_done
set gl_trace_toplevel_done 0

proc make_trace_toplevel {} {
global gl_var_index_list
global gl_var_list
global gl_var_name
global gl_max_trace_length
global gl_traced_name_list
global gl_trace_var_height
global gl_max_trace_var
global gl_trace_label_canvas_width
global gl_trace_view_width
global gl_var_index_list
global gl_trace_toplevel_done

#	.menu.windows entryconfigure 0 -state disabled

	if {$gl_trace_toplevel_done}  {
		clear_traces
	} else {
		toplevel .trace
		set h [ expr ([ llength $gl_var_index_list ] +1 )\
		             * $gl_trace_var_height ]
		frame .trace.buttons
		pack .trace.buttons -side bottom -fill x -pady 2m
		button .trace.buttons.dismiss -text "QUIT" -command quit_trace
		pack .trace.buttons.dismiss -expand 1
		frame .trace.c
		pack .trace.c -side top -fill both -expand yes
		canvas .trace.c.lbl -width $gl_trace_label_canvas_width \
			-height $h \
			-scrollregion \
	[ list 0 0 $gl_trace_label_canvas_width $h ] \
			-relief sunken -borderwidth 2 \
			-yscrollcommand {.trace.c.vscroll set}
		canvas .trace.c.c \
			-scrollregion [ list 0 0 $gl_max_trace_length $h ] \
			-width $gl_trace_view_width -height $h \
			-relief sunken -borderwidth 2 \
			-xscrollcommand {.trace.c.hscroll set} \
			-yscrollcommand {.trace.c.vscroll set}
		scrollbar .trace.c.vscroll \
			-command { trace_scroll_c_and_lbl }
		scrollbar .trace.c.hscroll -orient horiz \
			-command {.trace.c.c xview}

		grid .trace.c.lbl -in .trace.c \
		    -row 0 -column 0 -rowspan 1 -columnspan 1 -sticky news
		grid .trace.c.c -in .trace.c \
		    -row 0 -column 1 -rowspan 1 -columnspan 1 -sticky news
		grid .trace.c.vscroll \
		    -row 0 -column 2 -rowspan 1 -columnspan 1 -sticky news
		grid .trace.c.hscroll \
		    -row 1 -column 1 -rowspan 1 -columnspan 1 -sticky news
		grid rowconfig    .trace.c 0 -weight 1 -minsize 0
		grid columnconfig .trace.c 0 -weight 1 -minsize 0
#		bind .trace Destroy {catch { stop_trace }}
	}
	set gl_trace_toplevel_done 1
}

proc show_trace_toplevel {} {
global gl_trace_toplevel_done
	if {$gl_trace_toplevel_done}  {
		raise .trace
		focus .trace
	} else {
		repaint_trace_toplevel
	}
}

proc repaint_trace_toplevel {} {
	make_trace_toplevel
	show_trace_varlist
	trace_view_core
}

proc rebuild_trace_toplevel {} {
	destroy_trace_toplevel
	repaint_trace_toplevel
}

proc clear_trace_varlist {} {
global gl_traced_name_list
	set gl_traced_name_list {}
}

proc make_trace_varlist {} {
global gl_traced_name_list
global gl_var_index_list
global gl_var_name

	clear_trace_varlist
	foreach i $gl_var_index_list {
		lappend gl_traced_name_list [ string trim $gl_var_name($i) ]
	}
}

proc show_trace_varlist {} {
global gl_traced_name_list
global gl_var_index_list
global gl_var_name
global gl_traced_name_list
global gl_trace_var_height
global gl_max_trace_var
global gl_max_trace_length
global gl_trace_label_canvas_width
global gl_trace_toplevel_done

	if {! $gl_trace_toplevel_done}  {
		return
	}
	catch { .trace.c.lbl delete "variables" }
	set nv 1
	foreach i $gl_traced_name_list {
		.trace.c.lbl create text 10 \
			[ expr $nv*$gl_trace_var_height ] \
			-text "$i" -width \
	[ expr $gl_trace_label_canvas_width-10] \
			-justify right -anchor w \
			-tags { variables }
		incr nv
	}
}

proc destroy_trace_toplevel {} {
global gl_trace_toplevel_done
	set gl_trace_toplevel_done 0
	catch { destroy .trace }
#	.menu.windows entryconfigure 0 -state normal
}

# Inizio/fine della fase di trace.
proc start_trace {} {
	.buttons.trace configure -text "Stop Trace" -command stop_trace \
	                         -state disabled
	clear_trace_history
	post_qplc_job start_trace_step2
}
proc start_trace_step2 {} {
	if [ start_trace_core ] {
		make_trace_varlist
		rebuild_trace_toplevel
		.buttons.trace configure -text "Stop Trace" -command stop_trace
#		.menu.windows entryconfigure 0 -state disabled
	}
	.buttons.trace configure -state normal
}
proc stop_trace {} {
	.buttons.trace configure -text "Start Trace" -command start_trace \
	                         -state disabled
	post_qplc_job stop_trace_step2
}
proc stop_trace_step2 {} {
	stop_trace_core
	.buttons.trace configure -state normal
}

proc quit_trace {} {
	post_qplc_job {catch { stop_trace }; catch { destroy_trace_toplevel }}
}


proc qplc_status_data {x} {
	update_plc_status "$x"
}
proc do_status {} {
	qplc_send "status"  { data qplc_status_data }
}

proc do_go {} {
	qplc_send "go" {}
}
proc do_halt {} {
	qplc_send "halt" {}
}

proc do_initial_inquiry {} {
# Attende l'OK iniziale di qplc, fingendo che sia la risposta ad un
# comando vuoto.
	qplc_send ""  {}
# Legge lo stato corrente.
	do_status
}

global gl_trace_slice_size
set gl_trace_slice_size 640

proc set_trace_slice_size {} {
	catch { destroy .selectsize }
	toplevel .selectsize
	scale .selectsize.scale -label "Slice size: " \
		-orient horizontal -length 200 -width 40 \
		-from 50 -to 640 -showvalue 1 -variable gl_trace_slice_size 
	button .selectsize.ok -text "OK" \
		-command {
			destroy .selectsize
			set v [ format "%x" $gl_trace_slice_size ]
			post_qplc_job [ list do_trace_m $v ]
		}
	button .selectsize.cancel -text "CANCEL" \
		-command { destroy .selectsize }
	pack .selectsize.scale .selectsize.ok -fill x -pady 2m
}

global gl_max_trace_length_new
proc set_trace_buffer_size {} {
global gl_max_trace_length
global gl_max_trace_length_new
	catch { destroy .selectsize }
	toplevel .selectsize
	set gl_max_trace_length_new $gl_max_trace_length
	scale .selectsize.scale -label "Buffer size: " \
		-orient horizontal -length 200 -width 40 \
		-from 640 -to 3200 -showvalue 1 \
		-variable gl_max_trace_length_new
	button .selectsize.ok -text "OK" \
		-command {
			if {$gl_max_trace_length != $gl_max_trace_length_new} {
				set gl_max_trace_length \
					$gl_max_trace_length_new
				rebuild_trace_toplevel
			}
			destroy .selectsize
		}
	button .selectsize.cancel -text "CANCEL" \
		-command { destroy .selectsize }
	pack .selectsize.scale .selectsize.ok -fill x -pady 2m
}

# Visualizzazione di informazioni varie
proc show_info {x} {
	catch { destroy .showinfo }
	toplevel .showinfo
	label .showinfo.label -justify left -width 70 -text "?" -font fixed
	button .showinfo.ok -text "OK" \
		-command { destroy .showinfo } -state disabled
	pack .showinfo.label .showinfo.ok -pady 2m
	post_qplc_job  [list  show_info_step2 $x ]
}
proc mod_info {m s} {
	set rv [ do_mod $m ]
	scan "$rv" "%x %x %x %x %x" td dd bd cd sz
	set path [ lindex "$rv" 5 ]
	if { "$path" == "" || $sz == 0 } {
		set info " $m $s: Not loaded / Built-in\n"
	} else {
		set t [ lindex "$rv" 0 ]
		set d [ lindex "$rv" 1 ]
		set b [ lindex "$rv" 2 ]
		set c [ lindex "$rv" 3 ]
		set info " $m $s:\n  \
file=$path\n  \
size=$sz bytes\n  \
text   at 0x$t ([expr $dd-$td] bytes)\n  \
data   at 0x$d ([expr $bd-$dd] bytes * 2)\n  \
bss    at 0x$b ([expr $cd-$bd] bytes)\n  \
common at 0x$c ([expr $sz-($cd-$td)-($bd-$dd)] bytes)\n"
	}
	return "$info"
}
proc show_info_step2 {x} {
	set info "???"
	switch "$x" {
	"setup" {
		set rv [ do_info ]
		if { $rv != "" } {
			scan "$rv" "%x %x %x %x %x" \
				per maxte maxtv maxmv maxtrig
			set info "\
Execution rate: $per ms\n\
Trace buffer size: $maxte cycles ($maxtv bit/cycle)\n\
Number of locations under debug: $maxmv\n\
Available triggers: $maxtrig"
		}
	}
	"modules" {
		set info "Modules:\n"
		set rv [ mod_info U_CODE "(IL/LADDER program)" ]
		set info "$info $rv"
		set rv [ mod_info C_CODE "(RT480-style program)" ]
		set info "$info $rv"
		set rv [ mod_info HATSH_CODE "(PLC-CN communication)" ]
		set info "$info $rv"
	}
	"shvars" {
		set rv [ do_free ]
		if { $rv != "" } {
			scan "$rv" "%x %x %x %x" nvram heap keys maxkey	
			set info "Free shared resources:\n \
Non-volatile variables: $nvram bytes\n \
Shared heap: $heap bytes\n \
Free shared variable slots: $keys\n \
Biggest slot block: $maxkey keys"
		}
	}
	"ipcs" {
		set rv [ do_ipcs ]
		if { $rv != "" } {
			set info "Inter-Process Communication:\n \
Shared memory: "
			set shm [ lindex "$rv" 0 ]
			if {$shm == "V"} {
				scan "$rv" "%s %x %x %x %s" d1 km im len am
				set info "${info}SystemV interface\n  \
$len bytes\n  \
Key=$km, ID=$im, Virtual address=0x$am\n"
			} else {
				scan "$rv" "%s %s %x %x %s" d1 km im len am
				set info "${info}POSIX interface\n  \
$len bytes\n  \
Path=$km, Virtual address=0x$am\n"
			} 
			set info "${info} Semaphores: "
			set sem [ lindex "$rv" 5 ]
			if {$shm == "V"} {
				scan "$rv" "%s %s %s %s %s %s %x %x %x" \
					d1 d2 d3 d4 d5 d6 ks is num
				set info "${info}SystemV interface\n  \
$num semaphores\n  \
Key=$ks, ID=$is\n"
			} else {
				scan "$rv" "%s %s %s %s %s %s %s %x %x" \
					d1 d2 d3 d4 d5 d6 ks is num
				set info "${info}POSIX interface\n  \
$num semaphores\n  \
Path=$ks\n"
			}
			set info "${info} Non-volatile RAM regions:\n  \
Public:  name=\"[lindex $rv 9]\" address=0x[lindex $rv 10]\n  \
Private: name=\"[lindex $rv 11]\" address=0x[lindex $rv 12]"
		}
	}
	}
	.showinfo.label configure -text "$info"
	.showinfo.ok configure -state normal
}

# Lettura/salvataggio della lista delle variabili
proc load_or_save { ls action } {
	set ok 0
	if {$ls} {
		set op tk_getOpenFile
	} else {
		set op tk_getSaveFile
	}
	while {$ok == 0} {
		set f [ $op -defaultextension ".var" -filetypes {
				{{Varlist file} {.var}}
				{{All files} * }
			} ]
		if { $f != "" } {
			if [ $action $f ] {
				set ok 1
			}
		} else {
			set ok 1
		}
	}
} 

proc clear_var_list {} {
global gl_max_var
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size

	for {set i 1} {$i <= $gl_max_var } {incr i} {
		set gl_var_name($i) ""
		set gl_var_idx1($i) 0
		set gl_var_idx2($i) 0
		set gl_var_size($i) 1
	}
	clear_var_index
}

proc load_vars_core { f } {
global gl_max_var
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size

	if [ catch { set fd [ open $f r ] } error_code ] {
		tk_dialog .ok_cancel "Error" "$error_code" "" 0 "OK"
		return 0
	} else {
		clear_var_list
		catch {
		while { ! [ eof $fd ] } {
			gets $fd line
			set ord [ lindex $line 0 ]
			set name [ lindex $line 1 ]
			set idx1 [ lindex $line 2 ]
			set idx2 [ lindex $line 3 ]
			set size [ lindex $line 4 ]
			if { $ord >= 1 && $ord <= $gl_max_var } {
				set gl_var_name($ord) "$name"
				set gl_var_idx1($ord) "$idx1"
				set gl_var_idx2($ord) "$idx2"
				set gl_var_size($ord) "$size"
			}
		}
		close $fd
		}
	}
	return 1
}
proc load_vars {} {
	load_or_save 1 load_vars_core
}

proc save_vars_core { f } {
global gl_max_var
global gl_var_name
global gl_var_idx1
global gl_var_idx2
global gl_var_size

	if [ catch { set fd [ open $f w ] } error_code ] {
		tk_dialog .ok_cancel "Error" "$error_code" "" 0 "OK"
		return 0
	} else {
		catch {
		for {set i 1} {$i <= $gl_max_var} {incr i} {
			set name [ string trim $gl_var_name($i) ]
			set idx1 [ string trim $gl_var_idx1($i) ]
			set idx2 [ string trim $gl_var_idx2($i) ]
			set size $gl_var_size($i)
			if { "$name" != "" } {
				puts $fd "$i $name $idx1 $idx2 $size"
			}
		}
		close $fd
		}
	}
	return 1
}
proc save_vars {} {
	load_or_save 0 save_vars_core
}


proc reset_status {} {
global gl_channel
global gl_plcstatus
global gl_current_answer_data
global gl_cmd_done
global gl_n_task
global gl_congestion_signaled
global gl_command_queue
global gl_doing_command

	init_queue
	init_var_list
	clear_var_index
	clear_trace_history
	clear_trace_varlist
	set gl_doing_command 0
	set gl_plcstatus 0
	set gl_channel -1
	set gl_current_answer_data ""
	set gl_cmd_done 0
	set gl_n_task 0
	set gl_congestion_signaled 0
	set gl_command_queue {}
	install_default_handlers
	install_default_handler_async
	do_disconnect
}

global gl_main_loop
proc main {} {
global gl_main_loop
	reset_status
	create_gui
	update_status
#	connect_qplc
	while 1 {
		set gl_main_loop 1
		update_status
		tkwait variable gl_main_loop
		reset_status
	}
}

main

