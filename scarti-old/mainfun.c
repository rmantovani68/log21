/*
* mainfun.c
* funzioni
* Gestione Scarti
* Easy Picking 3.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <signal.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <shared.h>
#include <time.h>

#include "interface.h"
#include "support.h"

#ifdef TRACE
	#include <trace.h>
#endif

#include <pmx_msq.h>
#include <dbfun.h>
#include <proc_list.h>
#include <picking.h>

#include "sockets.h"

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

#include "callbacks.h"


/*
* GTK-Error handling functions
*/
void gtk_error_handler(gchar *str)
{
	trace_debug(NULL,txt_trace,str);
}

void gtk_warning_handler(gchar *str)
{
	trace_debug(NULL,txt_trace,str);
}

void gtk_message_handler(gchar *str)
{
	trace_debug(NULL,txt_trace,str);
}

/*
* ReadConfiguration()
* lettura della configurazione dal file cni.cfg
*/
BOOL ReadConfiguration(void)
{
	char szParagraph[128];
	char szCurrentDirectory[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	strcpy(Cfg.szCniCfg,szCurrentDirectory);
	strcat(Cfg.szCniCfg,"/cni.cfg");

	if(!FileExists(Cfg.szCniCfg)){
#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration : file cni.cfg not present");
#endif
		return FALSE;
	}

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	GetFileString(szParagraph,"PathData",        "../data",      Cfg.szPathData,         80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathExe",         "./",           Cfg.szPathExe,          80,Cfg.szCniCfg,NULL);
	GetFileString(szParagraph,"PathStampe",      "../stampe",    Cfg.szPathStampe,       80,Cfg.szCniCfg,NULL);

	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"PGHost",    "localhost",Cfg.szPGHost,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGPort",    "5432",     Cfg.szPGPort,     80,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"PGDataBase","picking",  Cfg.szPGDataBase, 80,Cfg.szCniCfg,NULL); 

	/*
	* Lettura dei parametri relativi ai processi 
	*/
	ReadProcInfo(Cfg.szCniCfg);

	/* 
	* configurazioni specifiche di peso 
	*/
	Cfg.nDelay           = GetFileInt(ProcList[PROC_SCARTI].szProcName,"Delay",           100, Cfg.szCniCfg,NULL);
	Cfg.nDebugVersion    = GetFileInt(ProcList[PROC_SCARTI].szProcName,"DebugVersion",      0, Cfg.szCniCfg,NULL);
	Cfg.nProgramID       = GetFileInt(ProcList[PROC_SCARTI].szProcName,"ProgramID",         0, Cfg.szCniCfg,NULL);
	Cfg.nColumnCharWidth = GetFileInt(ProcList[PROC_SCARTI].szProcName,"ColumnCharWidth",  10, Cfg.szCniCfg,NULL);
	Cfg.nA2ps            = GetFileInt(ProcList[PROC_SCARTI].szProcName,"A2ps",              0, Cfg.szCniCfg,NULL);
	Cfg.nHtml2ps         = GetFileInt(ProcList[PROC_SCARTI].szProcName,"Html2ps",           0, Cfg.szCniCfg,NULL);
	GetFileString(ProcList[PROC_SCARTI].szProcName,"PrinterScarti",    "lp",Cfg.szPrinter, 80,Cfg.szCniCfg,NULL);

#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration");
	trace_out_vstr(1,"-------------------------------------------");
	trace_out_vstr(1,"PathData           : [%s]", Cfg.szPathData);
	trace_out_vstr(1,"PathExe            : [%s]", Cfg.szPathExe);
	trace_out_vstr(1,"PLC                : [%d]", Cfg.nPLC);
	trace_out_vstr(1,"PLCService         : [%d]", Cfg.nPLCService);
	trace_out_vstr(1,"DebugVersion       : [%d]", Cfg.nDebugVersion);
	trace_out_vstr(1,"-------------------------------------------");
#endif
	return TRUE;
}

/*
* ReadProcInfo()
* Lettura dei parametri relativi ai processi 
*/
void ReadProcInfo(char *szFileConfig)
{
	char szProcIndex[128];
	char szProcBuffer[128];
	int nIndex;

	for (nIndex=0; nIndex<NUM_PROC; nIndex++) {
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].nPriority,
				&ProcList[nIndex].bExecute);

		/* 
		* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
		*                 problemi con la RunSimpleProcess()  e la lettura della condifgurazione
		*/
		StrTrimAll(ProcList[nIndex].szProcName);

		ProcList[nIndex].nGId=nIndex+1;

		/*
		* resetto il numero coda messaggi del processo
		*/
		ProcList[nIndex].nQNumber=0;
	}
}


void ExitApp()
{
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif

	/* rimuovo il timer per gestione eventi*/
  gtk_timeout_remove(Cfg.nTimer);

	TerminateProcess(PROC_BARCODE_SCARTI);

	/* cancello la coda messaggi principale */
	DeleteMsgQ(ProcList[PROC_SCARTI].nQNumber);

	DisconnectDataBase();

	/* chiudo la GUI */
	gtk_widget_destroy(main_window);
	gtk_main_quit();
}




/*
* void EditConfigurazione(void)
* visualizza i dati di configurazione
*/
void EditConfigurazione(GtkWidget *dlg)
{
	char szDevice[64];
	char szParams[64];
	char szParagraph[128];

	/*
	* GENERALE
	*/ 
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_delay")),Cfg.nDelay);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_debug_version")),Cfg.nDebugVersion);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prgid")),Cfg.nProgramID);

	if(Cfg.nA2ps || Cfg.nHtml2ps){
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"cb_postscript")),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_a2ps"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_html2ps"),TRUE);
		if(Cfg.nA2ps){
			gtk_widget_activate(get_widget(dlg,"rb_a2ps"));
		}
		if(Cfg.nHtml2ps){
			gtk_widget_activate(get_widget(dlg,"rb_html2ps"));
		}
	} else {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"cb_postscript")),FALSE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_a2ps"),FALSE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_html2ps"),FALSE);
	}
	/*
	* DIRECTORY
	*/
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_path_data")),   Cfg.szPathData);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_path_exe")),    Cfg.szPathExe);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_path_stampe")), Cfg.szPathStampe);

	/*
	* DATABASE
	*/
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pghost")),     Cfg.szPGHost);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pgport")),     Cfg.szPGPort);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pgdatabase")), Cfg.szPGDataBase);

	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_printer")),        Cfg.szPrinter);

	/*
	* BARCODE SCARTI
	*/
	sprintf(szParagraph,"%s %d",ProcList[PROC_BARCODE_SCARTI].szProcName,PROC_BARCODE_SCARTI);
	GetFileString(szParagraph,"CommDevice", "/dev/ttyS1", szDevice,    80 ,Cfg.szCniCfg,NULL); 
	GetFileString(szParagraph,"CommParams", "9600,n,8,1", szParams, 80 ,Cfg.szCniCfg,NULL); 
	gtk_entry_set_text (GTK_ENTRY (get_widget(dlg,"entry_barcode_device")), szDevice);
	gtk_entry_set_text (GTK_ENTRY (get_widget(dlg,"entry_barcode_params")), szParams);

}



/*
* void ApplyConfigurazione(void)
* visualizza i dati di configurazione
*/
void ApplyConfigurazione(GtkWidget *dlg)
{
	char szDevice[64];
	char szParams[64];
	char szParagraph[64];
	
	/*
	* memorizzo le informazioni relative ai TIMEOUT
	*/ 
	Cfg.nDelay           = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_delay")));
	Cfg.nDebugVersion    = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_debug_version")));
	Cfg.nProgramID       = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prgid")));

	PutFileInt(ProcList[PROC_SCARTI].szProcName, "Delay",           Cfg.nDelay,Cfg.szCniCfg,NULL);
	PutFileInt(ProcList[PROC_SCARTI].szProcName, "DebugVersion",    Cfg.nDebugVersion,Cfg.szCniCfg,NULL);
	PutFileInt(ProcList[PROC_SCARTI].szProcName, "ProgramID",       Cfg.nProgramID,Cfg.szCniCfg,NULL);

	Cfg.nA2ps    = GTK_TOGGLE_BUTTON(get_widget(dlg,"rb_a2ps"))->active;
	Cfg.nHtml2ps = GTK_TOGGLE_BUTTON(get_widget(dlg,"rb_html2ps"))->active;

	PutFileInt(ProcList[PROC_SCARTI].szProcName, "A2ps",    Cfg.nA2ps,Cfg.szCniCfg,NULL);
	PutFileInt(ProcList[PROC_SCARTI].szProcName, "Html2ps", Cfg.nHtml2ps,Cfg.szCniCfg,NULL);

	/*
	* DIRECTORY
	*/
	strcpy(Cfg.szPathData,          gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_path_data"))));
	strcpy(Cfg.szPathExe,           gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_path_exe"))));
	strcpy(Cfg.szPathStampe,        gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_path_stampe"))));
	strcpy(Cfg.szPrinter,       gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_printer"))));

	PutFileString("General Settings", "PathData",   Cfg.szPathData,   Cfg.szCniCfg, NULL); 
	PutFileString("General Settings", "PathStampe", Cfg.szPathExport, Cfg.szCniCfg, NULL); 
	PutFileString("General Settings", "PathExe",    Cfg.szPathExe,    Cfg.szCniCfg, NULL); 
	PutFileString("General Settings", "PrinterScarti", Cfg.szPrinter,       Cfg.szCniCfg, NULL); 

	/*
	* DATABASE
	*/
	strcpy(Cfg.szPGHost,        gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pghost"))));
	strcpy(Cfg.szPGPort,        gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pgport"))));
	strcpy(Cfg.szPGDataBase,    gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pgdatabase"))));

	PutFileString("DataBase Settings", "PGHost",    Cfg.szPGHost,       Cfg.szCniCfg, NULL); 
	PutFileString("DataBase Settings", "PGPort",    Cfg.szPGPort,       Cfg.szCniCfg, NULL); 
	PutFileString("DataBase Settings", "PGDataBase",Cfg.szPGDataBase,   Cfg.szCniCfg, NULL); 

	/*
	* barcode scarto
	*/
	sprintf(szDevice, gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_barcode_device"))));
	sprintf(szParams, gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_barcode_params"))));
	sprintf(szParagraph,"%s %d",ProcList[PROC_BARCODE_SCARTI].szProcName,ProcList[PROC_BARCODE_SCARTI].nGId-1);

	PutFileString(szParagraph,"CommDevice", szDevice,Cfg.szCniCfg,NULL); 
	PutFileString(szParagraph,"CommParams", szParams,Cfg.szCniCfg,NULL); 
}

void trace_debug(GdkColor *Color,GtkWidget *txt,...)
{
  va_list args;
  char szMessage[1024];
  char *fmt;

  va_start(args,txt);
  fmt=va_arg(args,char *);
  (void)vsprintf(szMessage,fmt,args);
  va_end(args);
#ifdef TRACE
  trace_out_vstr_date(1,szMessage);
#endif
	if(txt){
		strcat(szMessage,"\n");
		gtk_text_printf(NULL,txt,szMessage);
	}
}

int GetColorIndex(PCOLORSTRUCT pColors,char *szColorName)
{
	int nIndex=0;

	while(strlen(pColors[nIndex].szColorName)){
		if(!strcasecmp(pColors[nIndex].szColorName,szColorName)){
			return nIndex;
		}
		nIndex++;
	}
	return -1;
}

GdkColor *GetColorValue(PCOLORSTRUCT pColors,int nColorIndex)
{
	int nIndex=0;

	while(strlen(pColors[nIndex].szColorName)){
		if(nIndex==nColorIndex){
			return &(pColors[nIndex].cColor);
		}
		nIndex++;
	}
	return &cGREY;
}
