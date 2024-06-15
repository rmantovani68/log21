/*
* mainfun.c
* funzioni 
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
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
#include <time.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <shared.h>

#include "interface.h"
#include "support.h"

#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <ca_file.h>
#include <pmx_msq.h>
#include <dbfun.h>

#include "../common/picking.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

#include "callbacks.h"


/*
* GTK-Error handling functions
*/
void gtk_error_handler(gchar *str)
{
	gtk_text_printf(NULL,txt_msgs,str);
}

void gtk_warning_handler(gchar *str)
{
	gtk_text_printf(NULL,txt_msgs,str);
}

void gtk_message_handler(gchar *str)
{
	gtk_text_printf(NULL,txt_msgs,str);
}


/*
* ReadConfiguration()
* lettura della configurazione dal file edit.cfg
*/
void ReadConfiguration(BOOL bReadProcInfo)
{
	char szParagraph[128];
	char szCurrentDirectory[128];
	int nIndex;
	char szFieldName[128];
	char szBuffer[128];

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	strcpy(Cfg.szCniCfg,szCurrentDirectory);
	strcat(Cfg.szCniCfg,"/cni.cfg");

	ReadProcInfo(Cfg.szCniCfg);
	
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	xncGetFileString(szParagraph,"TipoOrdini",      "C",          Cfg.szTipoOrdini,      80, Cfg.szCniCfg, NULL);
	xncGetFileString(szParagraph,"TipoUbicazioni",  "C",          Cfg.szTipoUbicazioni,  80, Cfg.szCniCfg, NULL);
	xncGetFileString(szParagraph,"PathData",        "../data",    Cfg.szPathData,        80, Cfg.szCniCfg, NULL);
	xncGetFileString(szParagraph,"PathExe",         "./",         Cfg.szPathExe,         80, Cfg.szCniCfg, NULL);
	xncGetFileString(szParagraph,"PathStampe",      "../stampe",  Cfg.szPathStampe,      80, Cfg.szCniCfg, NULL);
	xncGetFileString(szParagraph,"PrinterConsole",  "lp",         Cfg.szPrinterConsole,  80, Cfg.szCniCfg, NULL);

	Cfg.nDebugVersion      = xncGetFileInt(szParagraph,      "DebugVersion",      0, Cfg.szCniCfg,NULL);
	Cfg.nCharWidth         = xncGetFileInt(szParagraph,      "CharWidth",         8, Cfg.szCniCfg,NULL);
	Cfg.nA2ps              = xncGetFileInt(szParagraph,      "a2ps",              0, Cfg.szCniCfg,NULL);
	Cfg.nMainDelay         = xncGetFileInt(ProcList[PROC_MAIN].szProcName,"Delay",100,Cfg.szCniCfg,NULL);

	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	xncGetFileString(szParagraph,"PGHost",    "localhost",Cfg.szPGHost,     80,Cfg.szCniCfg,NULL); 
	xncGetFileString(szParagraph,"PGPort",    "5432",     Cfg.szPGPort,     80,Cfg.szCniCfg,NULL); 
	xncGetFileString(szParagraph,"PGDataBase","picking",  Cfg.szPGDataBase, 80,Cfg.szCniCfg,NULL); 

	/*
	* Numero settori componenti il magazzino
	*/
	Cfg.nNumeroSettori=xncGetFileInt(ProcList[PROC_SETTORI].szProcName,"NumeroSettori",15,Cfg.szCniCfg,NULL);

	/*
	* parametri specifici dell'applicazione
	*/
	/*
	* Delay in millisecondi
	*/
	Cfg.nDelay          = xncGetFileInt(ProcList[PROC_SETTORI].szProcName,"Delay",100,Cfg.szCniCfg,NULL);
	Cfg.nMaxDamages     = xncGetFileInt(ProcList[PROC_IOS].szProcName,"MaxDamages",5,Cfg.szCniCfg,NULL);

	/*
	* Numero settori componenti il magazzino
	*/
	Cfg.nNumeroSettori  = xncGetFileInt(ProcList[PROC_SETTORI].szProcName,"NumeroSettori",7,Cfg.szCniCfg,NULL);
	Cfg.nNumeroIsole    = xncGetFileInt(ProcList[PROC_SETTORI].szProcName,"NumeroIsole",1,Cfg.szCniCfg,NULL);
	/*
	* Numero displays componenti l'impianto
	*/
	Cfg.nNumeroDisplays = xncGetFileInt(ProcList[PROC_SETTORI].szProcName,"NumeroDisplays",7,Cfg.szCniCfg,NULL);
	/*
	* dati relativi ai display
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
		sprintf(szFieldName,"Display_%02d",nIndex);
		xncGetFileString(ProcList[PROC_SETTORI].szProcName,szFieldName, "", szBuffer, 80,Cfg.szCniCfg,NULL);

		sscanf(szBuffer, "%d,%d,%d,%d,%d",
			&(Cfg.Displays[nIndex].nIOS),
			&(Cfg.Displays[nIndex].nCPU),
			&(Cfg.Displays[nIndex].nModulo),
			&(Cfg.Displays[nIndex].nIsola),
			&(Cfg.Displays[nIndex].nSettore));
	}
	/*
	* Numero dei moduli PKL 
	*/
	Cfg.nPKLNumber=xncGetFileInt(ProcList[PROC_IOS].szProcName,"PKLNumber",1,Cfg.szCniCfg,NULL);

	/*
	* dati relativi ai moduli PKL
	*/
	for(nIndex=0; nIndex < Cfg.nPKLNumber; nIndex++){
		sprintf(szFieldName,"PKL_%02d",nIndex);
		xncGetFileString(ProcList[PROC_IOS].szProcName,szFieldName, "", szBuffer, 80,Cfg.szCniCfg,NULL);

		sscanf(szBuffer, "%d,%d,%d",
			&(Cfg.PKL[nIndex].nIOS),
			&(Cfg.PKL[nIndex].nCPU),
			&(Cfg.PKL[nIndex].nModulo));
	}


#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration");
	trace_out_vstr(1,"-------------------------------------------");
	trace_out_vstr(1,"PathData        : [%s]", Cfg.szPathData);
	trace_out_vstr(1,"PathStampe      : [%s]", Cfg.szPathStampe);
	trace_out_vstr(1,"PathExe         : [%s]", Cfg.szPathExe);
	trace_out_vstr(1,"DebugVersion    : [%d]", Cfg.nDebugVersion);
	trace_out_vstr(1,"-------------------------------------------");
#endif
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
		xncGetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL);
		sscanf(szProcBuffer, "%[^,],%d,%d,%d",
				ProcList[nIndex].szProcName,
				&ProcList[nIndex].nQKey,
				&ProcList[nIndex].nPriority,
				&ProcList[nIndex].bExecute);

		ProcList[nIndex].nGId=nIndex+1;

		/* 
		* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
		*                 problemi con la RunSimpleProcess() 
		*/
		StrTrimAll(ProcList[nIndex].szProcName);

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

	DisconnectDataBase();

	DetachShm((char *)pSettori);

	if(!RemoveShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
#ifdef TRACE
		trace_out_vstr_date(1,"Error Removing Shared Memory (%d)", Cfg.nSettoriShmKey);
#endif
	}


	TerminateProcess(PROC_IOS);


	/* cancello la coda messaggi principale */
	DeleteMsgQ(ProcList[PROC_SETTORI].nQNumber);

	/* chiudo la GUI */
	gtk_widget_destroy(main_window);
	gtk_main_quit();
}


/*
* UpdateListFromSelect()
* Aggiorna una riga di una lista GTK da una select SQL
*/
int UpdateListFromSelect(GtkWidget *clist, char *szSqlCmd, char *szKey, int nKeyField)
{
	int nTuples;
	int nFields;
	int nIndex;
	int nFieldIndex;
	char szCmd[4096];
	char szSqlTmp[4096];
	char *pPtr;
	char *pszText;
	PGresult *PGRes = NULL;
	char szText[256];
	
	strcpy(szSqlTmp,szSqlCmd);

	/*
	* Sostituisco i valori parametrici
	* %KEY%     : Chiave 
	* %TIPOUBI% : Tipo Ubicazione 
	*/
	if((pPtr=strstr(szSqlTmp,"%KEY%"))){
		*pPtr='\0';
		strcpy(szCmd,szSqlTmp);
		strcat(szCmd,szKey);
		strcat(szCmd,pPtr+strlen("%KEY%"));
	}
	strcpy(szSqlTmp,szCmd);

	if((pPtr=strstr(szSqlTmp,"%TIPOUBI%"))){
		*pPtr='\0';
		strcpy(szCmd,szSqlTmp);
		strcat(szCmd,Cfg.szTipoUbicazioni);
		strcat(szCmd,pPtr+strlen("%TIPOUBI%"));
	}
	
	/*
	* Eseguo la select specificata
	*/
	PGRes=PGExecSQL(FALSE,szCmd);

	nTuples=PQntuples(PGRes);
	nFields=PQnfields(PGRes);

	if(nTuples==1){
		for(nIndex=0;nIndex<(GTK_CLIST(clist)->rows);nIndex++){
			gtk_clist_get_text(GTK_CLIST(clist),nIndex,nKeyField,&pszText);
			if(!strcmp(szKey,pszText)){
				for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
					int nLength;

					if(PQftype(PGRes,nFieldIndex)==1042){
						/* si tratta di una stringa */
						nLength=PQgetlength(PGRes,0,nFieldIndex);
						sprintf(szText,"%*s",nLength,PQgetvalue(PGRes,0,nFieldIndex));
					} else if(PQftype(PGRes,nFieldIndex)==701){
						/* si tratta di un float */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(szText,"%*.*f",nLength,3,atof(PQgetvalue(PGRes,0,nFieldIndex)));
					} else if(PQftype(PGRes,nFieldIndex)==23){
						/* si tratta di un int */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(szText,"%*d",nLength,atoi(PQgetvalue(PGRes,0,nFieldIndex)));
					} else {
						/* altro... lo tratto come una stringa */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(szText,"%*s",nLength,PQgetvalue(PGRes,0,nFieldIndex));
					}
					/*
					* Setto il testo del campo
					*/
					gtk_clist_set_text(GTK_CLIST(clist),nIndex,nFieldIndex,szText);
				}
			}
		}
	}
	
	PQclear(PGRes);

	return nTuples;
}


void EditConfigurazione(void)
{	
	GtkWidget *dlg=create_dlg_config();
	
	/*
	* GENERAL
	*/
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_debug_version")), Cfg.nDebugVersion);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_column_width")),  Cfg.nCharWidth);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_printer")),   Cfg.szPrinterConsole);

	/*
	* DIRECTORY
	*/
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_data_path")),   Cfg.szPathData);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_exe_path")),    Cfg.szPathExe);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_print_path")),  Cfg.szPathStampe);

	/*
	* DATABASE
	*/
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pghost")),     Cfg.szPGHost);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pgport")),     Cfg.szPGPort);
	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_pgdatabase")), Cfg.szPGDataBase);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_cancel"));
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_widget_show(dlg);
}

void ApplyConfigurazione(GtkWidget *dlg)
{
	/*
	* GENERAL
	*/
	Cfg.nDebugVersion = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_debug_version")));
	Cfg.nCharWidth    = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_column_width")));
	strcpy(Cfg.szPrinterConsole,  gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_printer"))));
	
	/*
	* DIRECTORY
	*/
	strcpy(Cfg.szPathData,          gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_data_path"))));
	strcpy(Cfg.szPathExe,           gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_exe_path"))));
	strcpy(Cfg.szPathStampe,        gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_print_path"))));

	/*
	* DATABASE
	*/
	strcpy(Cfg.szPGHost,         gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pghost"))));
	strcpy(Cfg.szPGPort,         gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pgport"))));
	strcpy(Cfg.szPGDataBase,     gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_pgdatabase"))));

	/* scrivo i dati nel file di configurazione */
	xncPutFileInt("General Settings",    "DebugVersion",     Cfg.nDebugVersion,     Cfg.szCniCfg, NULL);
	xncPutFileInt("General Settings",    "CharWidth",        Cfg.nCharWidth,        Cfg.szCniCfg, NULL);
	xncPutFileString("General Settings", "PrinterConsole",   Cfg.szPrinterConsole,  Cfg.szCniCfg, NULL); 

	xncPutFileString("General Settings", "PathData",   Cfg.szPathData,   Cfg.szCniCfg, NULL); 
	xncPutFileString("General Settings", "PathStampe", Cfg.szPathStampe, Cfg.szCniCfg, NULL); 
	xncPutFileString("General Settings", "PathExe",    Cfg.szPathExe,    Cfg.szCniCfg, NULL); 

	xncPutFileString("DataBase Settings", "PGHost",        Cfg.szPGHost,       Cfg.szCniCfg, NULL); 
	xncPutFileString("DataBase Settings", "PGPort",        Cfg.szPGPort,       Cfg.szCniCfg, NULL); 
	xncPutFileString("DataBase Settings", "PGDataBase",    Cfg.szPGDataBase,   Cfg.szCniCfg, NULL); 
}

void load_table(char *pszTableName,char *szKey)
{
	char szMsg[256];

	sprintf(szMsg,"Loading %s ...",szActualTableName);
	gtk_statusbar_push(GTK_STATUSBAR(get_widget(main_window,"sb_main")),1,szMsg);
	gtk_update();

	CreateTable(main_window,"sw_list","clist",NULL,NULL,NULL,"edit",pszTableName,GTK_SELECTION_SINGLE,szKey);

	sprintf(szMsg,"Ready.");
	gtk_statusbar_push(GTK_STATUSBAR(get_widget(main_window,"sb_main")),1,szMsg);
}


/*
* int search_key(char *pszKey,GtkCList *clist,int nColumn,int nStartRow,BOOL bCaseSensitive)
*
* ricerca nella colonna <nColumn> della <GtkCList> la stringa <pszKey> a partire dalla riga <nStartRow>
*
* parametri formali:
*
*	- char *pszKey: stringa da ricercare;
* - GtkCList *clist: gtk columned list su cui effettuare la ricerca;
* - int nColumn: colonna della clist su cui effettuare la ricerca;
* - int nStartRow: riga della GtkCList da cui partire per effettuare la ricerca;
* - BOOL bCaseSensitive: se TRUE -> ricerca casesensitive;
*
* ritorna (int) - l'indice della prima riga incontrata che soddisfa la ricerca;
*                 -1 altrimenti.
*/
int search_key(char *pszKey,GtkCList *clist,int nColumn,int nStartRow,BOOL bCaseSensitive)
{
	int nIndex=nStartRow;
	int nKeyLenght;
	char *pszText;

	/* ricavo la lunghezza della chiave da cercare */
	nKeyLenght=strlen(pszKey);

	while(gtk_clist_get_text(GTK_CLIST(clist),nIndex,nColumn,&pszText)){
		if(bCaseSensitive){
			if(!strncmp(StrTrimAll(pszKey),StrTrimAll(pszText),nKeyLenght)){
				return(nIndex);
			}
		} else {
			if(!strncasecmp(StrTrimAll(pszKey),StrTrimAll(pszText),nKeyLenght)){
				return(nIndex);
			}
		}
		nIndex++;
	}
	return(-1);
}

/*
* void clist_move_to_row(GtkCList *clist, int nRow)
*
* posiziona la <clist> passatagli alla riga <nRow>
*
* parametri formali:
*
* - GtkCList *clist: gtk columned list;
* - int nRow: riga della GtkCList su cui posizionarsi (da 0 a ...).
*
* ritorna (void)
*/
void clist_move_to_row(GtkCList *clist, int nRow)
{
	if( (GTK_CLIST(clist)==NULL) || (nRow>(GTK_CLIST(clist)->rows)) ) return;

	gtk_clist_moveto(GTK_CLIST(clist), nRow, -1, 0.5, 0);
	gtk_clist_select_row(GTK_CLIST(clist), nRow, -1);
}

/*
* void clist_update_selected_row(GtkCList *clist, int nStep)
*
* aggiorna la selezione della riga di una GtkCList in Selection Mode = Single
*
* parametri formali:
*
* - GtkCList *clist: gtk columned list;
* - int nStep: valore incrementale riferito alla Riga gia' selezionata
*              oppure a partire dall'inizio della clist se nessuna riga e' selezionata
*							
* ritorna (void)
*/
void clist_update_selected_row(GtkCList *clist, int nStep)
{
	int nRowSelected;
	int nRows;

	if(clist==NULL) return;

	nRows=clist->rows;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(clist->selection->data);
		if(nRowSelected+nStep<nRows){
			gtk_clist_moveto(clist, nRowSelected+nStep, -1, 0.5, 0);
			gtk_clist_select_row(clist, nRowSelected+nStep, -1);
		}
	} else {
		if(nStep<nRows){
			gtk_clist_moveto(clist, nStep, -1, 0.5, 0);
			gtk_clist_select_row(clist, nStep, -1);
		}
	}
}

/*
* ReadStatoSettori()
* lettura della configurazione dal file cni.cfg
*/
void ReadStatoSettori(void)
{
	int nIndex;
	char szFieldName[128];
	char szBuffer[128];

	/*
	* dati relativi ai settori (disabilitazione)
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		sprintf(szFieldName,"Settore_%02d",nIndex);

		xncGetFileString(ProcList[PROC_SETTORI].szProcName,szFieldName, "", szBuffer, 80,Cfg.szCniCfg,NULL);

		sscanf(szBuffer, "%d,%d,%d,%d",
			&(pSettori[nIndex].nIsola),
			&(pSettori[nIndex].nSettore),
			&(pSettori[nIndex].nStatoSettore),
			&(pSettori[nIndex].nTipoSettore));

		switch(pSettori[nIndex].nStatoSettore){
			case 0:
				pSettori[nIndex].nStatoSettore=DISABILITATO;
			break;
			case 1:
			default:
				pSettori[nIndex].nStatoSettore=ATTESA;
			break;
		}
	}
}

/*
* lettura delle ubicazioni in archivio
*/
int ReadUbicazioni(PUBICAZIONI pUbi)
{
	int nIndex;
	PGresult *PGRes = NULL;
	int nTuples;

	PGRes=PGExecSQL(FALSE,"select ubicazione, codprod, isola, settore, display, ios, cpu, modulo, riga, colonna, priorita, cnistato from ubicazioni where ubitipo='%s';",Cfg.szTipoUbicazioni);

	nTuples=PQntuples(PGRes);

	for(nIndex=0;nIndex<nTuples;nIndex++){

		strcpy(pUbi->Ubicazione[nIndex].szCodUbi,         PQgetvalue(PGRes,nIndex,0));
		strcpy(pUbi->Ubicazione[nIndex].szCodProd,        PQgetvalue(PGRes,nIndex,1));
		pUbi->Ubicazione[nIndex].nIsola            = atoi(PQgetvalue(PGRes,nIndex,2));
		pUbi->Ubicazione[nIndex].nSettore          = atoi(PQgetvalue(PGRes,nIndex,3));
		pUbi->Ubicazione[nIndex].nDisplay          = atoi(PQgetvalue(PGRes,nIndex,4));
		pUbi->Ubicazione[nIndex].nIOS              = atoi(PQgetvalue(PGRes,nIndex,5));
		pUbi->Ubicazione[nIndex].nCPU              = atoi(PQgetvalue(PGRes,nIndex,6));
		pUbi->Ubicazione[nIndex].nModulo           = atoi(PQgetvalue(PGRes,nIndex,7));
		pUbi->Ubicazione[nIndex].nRowIndex         = atoi(PQgetvalue(PGRes,nIndex,8));
		pUbi->Ubicazione[nIndex].nColIndex         = atoi(PQgetvalue(PGRes,nIndex,9));
		pUbi->Ubicazione[nIndex].nPriorita         = atoi(PQgetvalue(PGRes,nIndex,10));

		pUbi->Ubicazione[nIndex].nIndex=nIndex;
		
	}
	PQclear(PGRes);
	pUbi->nUbicazioni=nIndex;

	return nIndex;
}

void SortUbicazioni(PUBICAZIONI pUbi)
{
	qsort(pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

int CmpUbicazione(PUBICAZIONE pUbi1,PUBICAZIONE pUbi2)
{
	if(pUbi1->nIOS < pUbi2->nIOS){
		return -1;
	}
	if(pUbi1->nIOS > pUbi2->nIOS){
		return 1;
	}
	if(pUbi1->nCPU < pUbi2->nCPU){
		return -1;
	}
	if(pUbi1->nCPU > pUbi2->nCPU){
		return 1;
	}
	if(pUbi1->nModulo < pUbi2->nModulo){
		return -1;
	}
	if(pUbi1->nModulo > pUbi2->nModulo){
		return 1;
	}
	if(pUbi1->nRowIndex < pUbi2->nRowIndex){
		return -1;
	}
	if(pUbi1->nRowIndex > pUbi2->nRowIndex){
		return 1;
	}
	if(pUbi1->nColIndex < pUbi2->nColIndex){
		return -1;
	}
	if(pUbi1->nColIndex > pUbi2->nColIndex){
		return 1;
	}
	return 0;
}


PUBICAZIONE SearchUbicazione(PUBICAZIONI pUbi,int nIOS,int nCPU,int nModulo,int nRowIndex,int nColIndex)
{
	UBICAZIONE Ubi;

	Ubi.nIOS=nIOS;
	Ubi.nCPU=nCPU;
	Ubi.nModulo=nModulo;
	Ubi.nRowIndex=nRowIndex;
	Ubi.nColIndex=nColIndex;

	return bsearch(&Ubi,pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

/*
* Visualizza dati su display CNI PKDIS
* rm 30-11-2000 : invertite le righe per CALDARELLA (WELLA)
*/
void UpdateDisplay(PDISPLAYSTRUCT pDisplay,int nRiga)
{
	int nIOS;
	int nCPU;
	int nModulo; 
	unsigned char nDot=0; 

	if(pDisplay==(PDISPLAYSTRUCT)NULL){
		return;
	}

	nIOS=pDisplay->nIOS;
	nCPU=pDisplay->nCPU;
	nModulo=pDisplay->nModulo; 

	/*
	* Correggo al stampa della riga ordine causa puntino 
	*/
	/*
	*
	* solo per WELLA
	*
	nDot=0;
	if(isdigit(pDisplay->szRiga_1_Display[3])){
		nDot=4;
	}
	*/

	switch(nRiga){
		case TUTTO:
			/*
			* sono costretto a resettare la riga non BCD causa buco della ios PKDIS comandi 18H e 19H (Manfredi)
			*/
			DisplayWriteStringBCD( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga2, "    ",0,0);
			/* prima la riga 2 */
			DisplayWriteString( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
			DisplayWriteStringBCD( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display,nDot,0);
		break;

		case RIGA_1:
			DisplayWriteStringBCD( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display,nDot,0);
		break;

		case RIGA_2:
			/*
			* sono costretto a resettare la riga non BCD causa buco della ios PKDIS comandi 18H e 19H (Manfredi)
			*/
			DisplayWriteStringBCD( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga2, "    ",0,0);
			DisplayWriteString( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
		break;
	}
}

void DisplayWriteString(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString)
{
	char szBuffer[40];

	sprintf(szBuffer,"%d,%d,%d,%d,%d,%x,%x,%x,%x", nIOS, nCPU, nModulo, nRowIndex, nMode, szString[0], szString[1], szString[2], szString[3]);

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_DPY_STRING, szBuffer);
}

int GetBCDNibble(unsigned char cChar)
{
	switch(cChar){
		case '0': case '1': case '2': case '3': case '4': 
		case '5': case '6': case '7': case '8': case '9':
			return cChar-'0';
		break;
		case ' ': return 10; break;
		case '-': return 11; break;
		case '.': return 12; break;
		case 'X': return 13; break;	/* non gestito */
		case 'Y': return 14; break;	/* non gestito */
		case 'Z': return 15; break;	/* non gestito */
	}
	return 15;
}

/*
* DisplayWriteStringBCD(nIOS, nCPU, nModulo, nRowIndex, nMode, szString,nDot,nBlink)
* Comando di settaggio display PKDIS usando comandi BCD (11,12,13,14-18,19)
*/
void DisplayWriteStringBCD(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString,unsigned char nDot,unsigned char nBlink)
{
	char szBuffer[40];
	unsigned char DPYByte[3];

	DPYByte[0]  = GetBCDNibble(szString[0])<<4;
	DPYByte[0] |= GetBCDNibble(szString[1]);
	DPYByte[1]  = GetBCDNibble(szString[2])<<4;
	DPYByte[1] |= GetBCDNibble(szString[3]);

	/* flags */
	/* 4 bit bassi : puntino */
	/* 4 bit alti  : lampeggio */
	DPYByte[2]  = nDot | (nBlink<<4);

	sprintf(szBuffer,"%d,%d,%d,%d,%x,%x,%x", nIOS, nCPU, nModulo, nRowIndex, DPYByte[0],DPYByte[1],DPYByte[2]);

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_DPY_BCD, szBuffer);
}

void ScongelaPKL(void)
{
	int nIndex;
	char szBuffer[80];

	for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
		sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
		SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SCONGELA, szBuffer);
	}
}

void CongelaPKL(void)
{
	int nIndex;
	char szBuffer[80];

	for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
		sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
		SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_CONGELA, szBuffer);
	}
}

void SetLampada(int nIOS,int nCPU,int nModule,int nRowIndex, int nColumn)
{
	char szBuffer[40];

	sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS, nCPU, nModule, nRowIndex, nColumn);

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SET_RC, szBuffer);
}

void ResetLampada(int nIOS,int nCPU,int nModule,int nRowIndex, int nColumn)
{
	char szBuffer[40];

	sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS, nCPU, nModule, nRowIndex, nColumn); 

	SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_RC, szBuffer);
}

void ResetLuci(void)
{
	int nIndex;
	char szBuffer[80];

	for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
		sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
		SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_ALL, szBuffer);
	}
}

void ResetSettori(void)
{
	int nIndex;

	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		ResetSettore(pSettori[nIndex].nIsola,pSettori[nIndex].nSettore);
		/*
		* rm 11-07-2001 : ritardo per evitare problemi con la IOS
		*/
		usleep(1000*Cfg.nDelay);
	}
}

void ResetSettore(int nIsola,int nSettore)
{
	int nFirstDisplay=0;
	int nFirstIOS=0;
	int nFirstCPU=0;
	int nFirstMod=0;
	int nFirstCol=0;
	int nFirstRow=0;
	PGresult *PGRes = NULL;
	int nTuples;
	int nIndex;

#ifdef TRACE
	trace_out_vstr_date(1,"ResetSettore(Isola:%d,Settore:%d)",nIsola,nSettore);
#endif

	/*
	* Cerco l'indice del settore relativo ai parametri isola e settore passati alla funzione
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
		if(pSettori[nIndex].nIsola==nIsola && pSettori[nIndex].nSettore==nSettore){
			pSettori[nIndex].nIndiceRigaOrdine=0;        /* puntatore alla riga d'ordine da prelevare */
			pSettori[nIndex].nNumeroRigheOrdine=0;       /* numero delle righe d'ordine da prelevare */

			if(pSettori[nIndex].nStatoSettore!=DISABILITATO){
				pSettori[nIndex].nStatoSettore=ATTESA;       /* Stato attuale del settore */
			}
			/*
			* Determino i dati della prima ubicazione del settore in oggetto
			*/
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select display, ios, cpu, modulo, riga, colonna from ubicazioni where ubitipo='%s' AND isola='%d' AND settore='%4d' order by priorita;",Cfg.szTipoOrdini, nIsola,nSettore);

			nTuples=PQntuples(PGRes);

			if(nTuples>=1){
				nFirstDisplay = GetDisplay(atoi(PQgetvalue(PGRes,0,0)),nIsola);
				nFirstIOS=atoi(PQgetvalue(PGRes,0,1));
				nFirstCPU=atoi(PQgetvalue(PGRes,0,2));
				nFirstMod=atoi(PQgetvalue(PGRes,0,3));
				nFirstRow=atoi(PQgetvalue(PGRes,0,4));
				nFirstCol=atoi(PQgetvalue(PGRes,0,5));
			} else {
#ifdef TRACE
				trace_out_vstr_date(1,"ResetSettore(Isola:%d,Settore:%d) : Non esistono ubicazioni per il settore in oggetto",nIsola,nSettore);
#endif
			}
			pSettori[nIndex].nFirstIOS=nFirstIOS;
			pSettori[nIndex].nFirstCPU=nFirstCPU;
			pSettori[nIndex].nFirstMod=nFirstMod;
			pSettori[nIndex].nFirstRow=nFirstRow;
			pSettori[nIndex].nFirstColumn=nFirstCol;
			pSettori[nIndex].nFirstDisplay=nFirstDisplay;

			pSettori[nIndex].nIOS=nFirstIOS;             /* Nro IOS Tasto Premuto / da premere */
			pSettori[nIndex].nCPU=nFirstCPU;             /* Nro CPU Tasto Premuto / da premere */
			pSettori[nIndex].nModulo=nFirstMod;          /* Modulo Tasto Premuto / da premere */
			pSettori[nIndex].nRiga=nFirstRow;            /* Riga Tasto Premuto / da premere */
			pSettori[nIndex].nColonna=nFirstCol;         /* Colonna Tasto Premuto / da premere */
			pSettori[nIndex].nDisplay=nFirstDisplay;     /* Display */
			pSettori[nIndex].nNumCopie=0;                /* Numero Copie */
			pSettori[nIndex].nNumCollo=0;                /* Numero Collo */
			pSettori[nIndex].nStatoLampada=0;            /* Stato Lampada */
			pSettori[nIndex].szCodSped[0]='\0';          /* Codice Ordine Attivo */
			pSettori[nIndex].szCodProd[0]='\0';          /* Codice Prodotto da prelevare/prelevato*/
			pSettori[nIndex].szCodUbi[0]='\0';           /* Codice Ubicazione del prodotto da prelevare/prelevato*/

			PQclear(PGRes);
		}

	}

	/*
	* Cerco l'indice del o dei displays relativo/i ai parametri isola e settore passati alla funzione
	*/
	for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
		if(Cfg.Displays[nIndex].nIsola==nIsola && Cfg.Displays[nIndex].nSettore==nSettore){
			strcpy(Cfg.Displays[nIndex].szRiga_1_Display,"----");     /* Riga 1 del display */
			strcpy(Cfg.Displays[nIndex].szRiga_2_Display,"----");     /* Riga 2 del display */
			Cfg.Displays[nIndex].nStatoRiga1=NORMAL;
			Cfg.Displays[nIndex].nStatoRiga2=NORMAL;

			UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
		}
	}
}


/*
* int ReadDisplaysInfo(DISPLAYSTRUCT pDisplays)
*
* - Legge dalla relazione displays le informazioni relative ai displays della linea e
*   le memorizza nella struttura DISPLAYSTRUCT;
* - ritorna il numero dei displays memorizzati nella relazione 
*/
int ReadDisplaysInfo(PDISPLAYSTRUCT pDisplays)
{
	PGresult *PGres;
	int nDisplays=0;
	int nIndex=0;
	
	PGres=PGExecSQL(Cfg.nDebugVersion>1,"select display,isola,settore,ios,cpu,modulo from displays order by display;");

	if(PQresultStatus(PGres) == PGRES_TUPLES_OK && (nDisplays=PQntuples(PGres))){

		/* memorizzo le informazioni nella struttura preposta */
		for(nIndex=0; nIndex<nDisplays; nIndex++){

				pDisplays[nIndex].nDisplay = atoi(PQgetvalue(PGres,nIndex,0));
				pDisplays[nIndex].nIsola   = atoi(PQgetvalue(PGres,nIndex,1));
				pDisplays[nIndex].nSettore = atoi(PQgetvalue(PGres,nIndex,2));
				pDisplays[nIndex].nIOS     = atoi(PQgetvalue(PGres,nIndex,3));
				pDisplays[nIndex].nCPU     = atoi(PQgetvalue(PGres,nIndex,4));
				pDisplays[nIndex].nModulo  = atoi(PQgetvalue(PGres,nIndex,5));
		}

		PQclear(PGres);

	} else {
		char szMsg[256];

		/* c'e' qualcosa che non va', lo segnalo */
		sprintf(szMsg,"ATTENZIONE : fallita ReadDisplaysInfo() - Exit(1) di SETTORI");
		SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
		trace_out_vstr_date(1,szMsg);
#endif

		PQclear(PGres);

	}
	return nDisplays;
}

/*
* int ReadSettoriInfo(void)
*
* - Legge dalla relazione settori le informazioni relative ai settori della linea e
*   le memorizza nella struttura SETTORISTRUCT;
* - controlla che il numero dei settori memorizzati nella relazione del DB
*   coincida col il parametro <nNumeroSettori> passatogli
*/
void ReadSettoriInfo(int nNumeroSettori)
{
	PGresult *PGres;
	int nSettori=0;
	int nIndex=0;

	PGres=PGExecSQL(Cfg.nDebugVersion>1,"select settore,isola,flag,tipo,operatore from settori order by settore;");

	if(PQresultStatus(PGres) == PGRES_TUPLES_OK && ((nSettori=PQntuples(PGres))==nNumeroSettori) ){

#ifdef TRACE
		trace_out_vstr(1,"SETT I S T OPERATORE");
		trace_out_vstr(1,"---- - - - ---------");
#endif
		/* memorizzo le informazioni nella struttura preposta */
		for(nIndex=0; nIndex<nSettori; nIndex++){

			pSettori[nIndex].nSettore       = atoi(PQgetvalue(PGres,nIndex,0));
			pSettori[nIndex].nIsola         = atoi(PQgetvalue(PGres,nIndex,1));
			pSettori[nIndex].nStatoSettore  = atoi(PQgetvalue(PGres,nIndex,2));
			pSettori[nIndex].nTipoSettore   = atoi(PQgetvalue(PGres,nIndex,3));
			strcpy(pSettori[nIndex].szCodOperatore,PQgetvalue(PGres,nIndex,4));
#ifdef TRACE
			trace_out_vstr(1,"%4d %1d %1d %1d [%s]",
				pSettori[nIndex].nSettore,
				pSettori[nIndex].nIsola,
				pSettori[nIndex].nStatoSettore,
				pSettori[nIndex].nTipoSettore,
				pSettori[nIndex].szCodOperatore);
#endif
		}
#ifdef TRACE
		trace_out_vstr(1,"---- - - - ---------");
#endif
	
		PQclear(PGres);

	} else {
		char szMsg[256];

		/* c'e' qualcosa che non va', lo segnolo */
		sprintf(szMsg,"ATTENZIONE : fallita ReadSettoriInfo() - Exit(1) di SETTORI");
		SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
		trace_out_vstr_date(1,szMsg);
#endif

		PQclear(PGres);
		/*
		exit(1);
		*/
	}
}


int GetDisplay(int nDisplay,int nIsola)
{
	int nIndex;
	BOOL bFound=FALSE;
	
	for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
		if((Cfg.Displays[nIndex].nDisplay==nDisplay) && (Cfg.Displays[nIndex].nIsola==nIsola)){
			bFound=TRUE;
			break;
		}
	}
	if(!bFound){
		return 0;
	}
	
	return nIndex;
}
