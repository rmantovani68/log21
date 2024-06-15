/*
* main.c
* Bartolini - main module 
* Easy Picking 3.0
* Copyright CNI srl 1999-2001
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <gtk/gtk.h>
#include <string.h>
#include <signal.h>
#include <libpq-fe.h>


#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <ca_file.h>
#include <shared.h>
#include <pmx_msq.h>
#include <dbfun.h>

#include "interface.h"
#include "support.h"
#include "picking.h"
#include "callbacks.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __authors__[];
extern char __copyright__[];

int main (int argc, char *argv[])
{	
	gchar *szTitle;
#ifdef TRACE
	char szBufTrace[80];
#endif

	/*
  gtk_set_locale ();
	gtk_rc_parse ("../data/edit.rc");
  gtk_init (&argc, &argv);
	*/
	gnome_program_init("test", VERSION, LIBGNOMEUI_MODULE, argc, argv, GNOME_PARAM_NONE);

	/* Predispongo l'utilizzo dell'uscita di sicurezza */
	signal(SIGTERM, ExitApp);
	signal(SIGINT,ExitApp);
	signal(SIGQUIT,ExitApp);
	signal(SIGILL,ExitApp);
	signal(SIGKILL,ExitApp);
	signal(SIGSEGV, SIG_IGN);

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

	trace_out_vstr_date(1,"Started");
	trace_out_vstr(1,"Module Name     : %s",argv[0]);
#endif

	/*
	* Lettura configurazione
	*/
	ReadConfiguration(TRUE);

	/*
	* Gestione Database PostgreSQL
	*/
	if(!ConnectDataBase(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase)){
		trace_debug(&cRED,"Connection to database %s on %s failed.\n",Cfg.szPGDataBase,Cfg.szPGHost);
	}

	/* Lettura dei parametri relativi ai displays */
	Cfg.nNumeroDisplays=ReadDisplaysInfo(Cfg.Displays);

	/*
	* alloco la shared memory dei settori
	*/
	if((pSettori=(PSETTORE)SetupShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
		trace_debug(&cRED,"Errore in allocazione Shared Memory [%d] (Settori)", Cfg.nSettoriShmKey);
	}
	memset(pSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));

	ReadSettoriInfo(Cfg.nNumeroSettori);

	/* 
	* Creo la coda messaggi locale 
	* per comodita' faccio finta di essere [settori]
	*/
	if((ProcList[PROC_SETTORI].nQNumber = CreateMsgQ(ProcList[PROC_SETTORI].nQKey,1))<0){
		trace_debug(&cRED,"Creazione coda messaggi locale fallita");
	}

	RunSimpleProcess(PROC_IOS);

	main_window = create_main_window();

	ReadUbicazioni(&Ubicazioni);
	SortUbicazioni(&Ubicazioni);

	do_lista_ubicazioni(get_widget(main_window,"sw_main"));

	szTitle=g_strdup_printf("Easy Picking %s-%s - Test %s", __version__, __customer__, rcsid);
	gtk_window_set_title (GTK_WINDOW (main_window), szTitle);
	g_free(szTitle);
	gtk_widget_show (main_window);

	txt_msgs=get_widget(main_window,"txt_msgs");

	/*
	* Timer per gestione eventi
	*/
	Cfg.nTimer = gtk_timeout_add(Cfg.nMainDelay,ProcessMsgs,NULL);

  gtk_main ();
  return 0;
}
