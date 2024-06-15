/*
* main.c
* modulo principale
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <libpq-fe.h>


#ifdef TRACE
	#include <trace.h>
#endif
#include <shared.h>
#include <pmx_msq.h>
#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include "interface.h"
#include "support.h"

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

extern char rcsid[];
extern char __version__[];
extern char __customer__[];

int main (int argc, char *argv[])
{
	gchar *szTitle;
	GtkWidget *dlg;
#ifdef TRACE
	char szBufTrace[80];
#endif

	/*
  gtk_set_locale ();
	gtk_rc_parse ("../data/scarti.rc");
  gtk_init (&argc, &argv);
	*/
	gnome_program_init("main", VERSION, LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_NONE);

	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, ExitApp);
	signal(SIGINT,ExitApp);
	signal(SIGQUIT,ExitApp);
	signal(SIGILL,ExitApp);
	signal(SIGKILL,ExitApp);

	/* 
	* ignoro il segnale di broken pipe 
	*/
	signal(SIGPIPE, SIG_IGN);

	/* 
	* ignoro il segnale di uscita applicazione child 
	* (stampe in background)
	*/
	signal(SIGCHLD, SIG_IGN);

	/*
	* GTK-Error handling functions
	*/
	/*
	g_set_error_handler((GErrorFunc)gtk_error_handler);
	g_set_warning_handler((GWarningFunc)gtk_warning_handler);
	g_set_message_handler((GPrintFunc)gtk_message_handler);
	*/

	add_pixmap_directory (PACKAGE_BASE_DIR "pixmaps");

#ifdef TRACE
	/* Apro il file trace */
	sprintf(szBufTrace,"%s%s",argv[0],TRACE_FILE_SUFFIX);
	open_trace(argv[0], szBufTrace, TRACE_FILE_MAXSIZE);

	trace_out_vstr(1,"Module Name     : %s",argv[0]);
	trace_out_vstr(1,"Version         : %s",__version__);
	trace_out_vstr(1,"Customer Name   : %s",__customer__);
	trace_out_vstr(1,"RCSID String    : %s",rcsid);
	trace_out_vstr_date(1,"Started");
#endif

	/*
	* Lettura configurazione
	*/
	if(ReadConfiguration()){

#ifdef TRACE
		trace_out_vstr(1,"Local Module ID : %d",PROC_SCARTI);
		trace_out_vstr(1,"Local Queue ID  : %d",ProcList[PROC_SCARTI].nQKey);
#endif

		main_window = create_main_window ();

		szTitle=g_strdup_printf("%s %s-%s - Gestione Scarti %s", __application_name__, __version__, __customer__, rcsid);
		gtk_window_set_title (GTK_WINDOW (main_window), szTitle);
		g_free(szTitle);

		dlg_trace = create_dlg_trace ();
		txt_trace = get_widget(dlg_trace,"txt_trace");
		gtk_widget_show (main_window);
		/*
		* Verifico la presenza di una istanza attiva del processo SCARTI
		* controllando la presenza della coda MSG
		*/
		if((ProcList[PROC_SCARTI].nQNumber=OpenMsgQ(ProcList[PROC_SCARTI].nQKey))<0){
			/*
			* creo la coda locale per SCARTI
			*/
			if((ProcList[PROC_SCARTI].nQNumber= CreateMsgQ(ProcList[PROC_SCARTI].nQKey, 1))<0){
#ifdef TRACE
				trace_out_vstr_date(1,"Creazione coda msg SCARTI (%d) fallita", ProcList[PROC_SCARTI].nQKey);
#endif
				exit(FALSE);
			}

			/*
			* Gestione Database PostgreSQL
			*/
			if(!ConnectDataBase(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase)){
				trace_debug(&cRED, txt_trace, "Connection to database [%s:%s] failed.\n", Cfg.szPGHost,Cfg.szPGDataBase);
			}

			RunSimpleProcess(PROC_BARCODE_SCARTI);

			/*
			* Timer per gestione eventi
			*/
			Cfg.nTimer = gtk_timeout_add(Cfg.nDelay,ProcessMsgs,NULL);
		} else {
			GtkWidget *dlg;

			dlg =create_dlg_app_error();
			gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
			gtk_widget_show (dlg);
		}
	} else {
		dlg =create_dlg_app_error();
		gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
		gtk_label_printf(get_widget(dlg,"lb_msg"),"File di configurazione cni.cfg non presente\n\nPremere OK per terminare");
		gtk_widget_show (dlg);
	}

  gtk_main ();

  return 0;
}
