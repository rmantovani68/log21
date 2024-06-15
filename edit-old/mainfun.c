/*
* mainfun.c
* funzioni 
* Easy Picking 3.0
* Copyright A&L srl 1999-2000
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

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	strcpy(Cfg.szCfgFile,szCurrentDirectory);
	strcat(Cfg.szCfgFile,"/edit.cfg");
	
	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"General Settings");

	xncGetFileString(szParagraph,"TipoOrdini",      "N",          Cfg.szTipoOrdini,      80, Cfg.szCfgFile, NULL);
	xncGetFileString(szParagraph,"PathData",        "../data",    Cfg.szPathData,        80, Cfg.szCfgFile, NULL);
	xncGetFileString(szParagraph,"PathExe",         "./",         Cfg.szPathExe,         80, Cfg.szCfgFile, NULL);
	xncGetFileString(szParagraph,"PathStampe",      "../stampe",  Cfg.szPathStampe,      80, Cfg.szCfgFile, NULL);
	xncGetFileString(szParagraph,"PrinterConsole",  "lp",         Cfg.szPrinterConsole,  80, Cfg.szCfgFile, NULL);

	Cfg.nDebugVersion      = xncGetFileInt(szParagraph,      "DebugVersion",      0, Cfg.szCfgFile,NULL);
	Cfg.nCharWidth         = xncGetFileInt(szParagraph,      "CharWidth",         8, Cfg.szCfgFile,NULL);
	Cfg.nA2ps              = xncGetFileInt(szParagraph,      "a2ps",              0, Cfg.szCfgFile,NULL);

	/*
	* lettura del file di configurazione del DataBase (PostgreSQL)
	*/
	strcpy(szParagraph,"DataBase Settings");

	xncGetFileString(szParagraph,"PGHost",    "localhost",Cfg.szPGHost,     80,Cfg.szCfgFile,NULL); 
	xncGetFileString(szParagraph,"PGPort",    "5432",     Cfg.szPGPort,     80,Cfg.szCfgFile,NULL); 
	xncGetFileString(szParagraph,"PGDataBase","picking",  Cfg.szPGDataBase, 80,Cfg.szCfgFile,NULL); 


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

void ExitApp()
{
#ifdef TRACE
	trace_out_vstr_date(1,"Stopped");
#endif

	DisconnectDataBase();

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
	strcpy(szCmd,szSqlCmd);

	/*
	* Sostituisco i valori parametrici
	* %ORDPROG%   : Chiave Ordine
	*/
	if((pPtr=strstr(szSqlTmp,"%KEY%"))){
		*pPtr='\0';
		strcpy(szCmd,szSqlTmp);
		strcat(szCmd,szKey);
		strcat(szCmd,pPtr+5);
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
	xncPutFileInt("General Settings",    "DebugVersion",     Cfg.nDebugVersion,     Cfg.szCfgFile, NULL);
	xncPutFileInt("General Settings",    "CharWidth",        Cfg.nCharWidth,        Cfg.szCfgFile, NULL);
	xncPutFileString("General Settings", "PrinterConsole",   Cfg.szPrinterConsole,  Cfg.szCfgFile, NULL); 

	xncPutFileString("General Settings", "PathData",   Cfg.szPathData,   Cfg.szCfgFile, NULL); 
	xncPutFileString("General Settings", "PathStampe", Cfg.szPathStampe, Cfg.szCfgFile, NULL); 
	xncPutFileString("General Settings", "PathExe",    Cfg.szPathExe,    Cfg.szCfgFile, NULL); 

	xncPutFileString("DataBase Settings", "PGHost",        Cfg.szPGHost,       Cfg.szCfgFile, NULL); 
	xncPutFileString("DataBase Settings", "PGPort",        Cfg.szPGPort,       Cfg.szCfgFile, NULL); 
	xncPutFileString("DataBase Settings", "PGDataBase",    Cfg.szPGDataBase,   Cfg.szCfgFile, NULL); 
}

void edit_dlg_ordini(GtkWidget *dlg, char *pszCodiceOrdine)
{
	PGresult *PGRes = NULL;
	char szItem[128];
	char szStato[128];
	char szBuffer[128];
	int nTuples;
	int nColli=0;
	int nRighe=0;
	int nCopie=0;
	/* N.B. da fare la gestione dello storico */
	BOOL bStorico=FALSE;
	if(!strcmp(szTableName,"ordini_storico")){
		bStorico=TRUE;
	}

	/*
	* Cerco l'ordine specificato nella tabella ordini ricevuti
	*/
	if (!bStorico){
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select rostato,ronmcll,ronmrgh,ronmcpe,ordprog,rodscli,rotmeva,ropspre,ropsrea,rocpcli,rolocli,rocdrid from ric_ord where ordprog='%s';",pszCodiceOrdine);
	} else {
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select rostato,ronmcll,ronmrgh,ronmcpe,ordprog,rodscli,rotmeva,ropspre,ropsrea,rocpcli,rolocli,rocdrid from ric_ord_stor where ordprog='%s';",pszCodiceOrdine);
	}
	nTuples=PQntuples(PGRes);

	szStato[0]='\0';

	/*
	* Trovato l'ordine che cercavo nella tabella ordini ricevuti
	*/
	if(nTuples){
		PGresult *PGResElab = NULL;

		/*
		* assegno i valori dell'ordine alle label
		*/
		strcpy(szStato,PQgetvalue(PGRes,0,0));
		nColli=atoi(PQgetvalue(PGRes,0,1));
		nRighe=atoi(PQgetvalue(PGRes,0,2));
		nCopie=atoi(PQgetvalue(PGRes,0,3));
		gtk_label_printf(get_widget(dlg,"lb_NMCLL"),"%4d",nColli);
		gtk_label_printf(get_widget(dlg,"lb_NMRGH"),"%4d",nRighe);
		gtk_label_printf(get_widget(dlg,"lb_NMCPE"),"%4d",nCopie);
		gtk_label_printf(get_widget(dlg,"lb_ORDPROG"),PQgetvalue(PGRes,0,4));
		gtk_label_printf(get_widget(dlg,"lb_DSCLI1"),PQgetvalue(PGRes,0,5));
		gtk_label_printf(get_widget(dlg,"lb_CDCLI"),PQgetvalue(PGRes,0,11));
		gtk_label_printf(get_widget(dlg,"lb_STATO"), GetStatoOrdine(szStato[0],szBuffer));
		gtk_label_printf(get_widget(dlg,"lb_DATAEVA"), PQgetvalue(PGRes,0,6),szDateBuffer);
		gtk_label_printf(get_widget(dlg,"lb_PSPRE"), "%7.2fKg",(float)((float)atoi(PQgetvalue(PGRes,0,7))/(float)1000));
		gtk_label_printf(get_widget(dlg,"lb_PSREA"), "%7.2fKg",(float)((float)atoi(PQgetvalue(PGRes,0,8))/(float)1000));
		gtk_label_printf(get_widget(dlg,"lb_CAP"), PQgetvalue(PGRes,0,9),szDateBuffer);
		gtk_label_printf(get_widget(dlg,"lb_LOC"), PQgetvalue(PGRes,0,10),szDateBuffer);

		/*
		* controllo se l'ordine e' gia stato elaborato e quindi formati i colli
		*/
		if (!bStorico){
			PGResElab=PGExecSQL(Cfg.nDebugVersion>1,"select ordprog from col_prod where ordprog='%s';",pszCodiceOrdine);
		} else {
			PGResElab=PGExecSQL(Cfg.nDebugVersion>1,"select ordprog from col_prod_stor where ordprog='%s';",pszCodiceOrdine);
		}

		if(PQntuples(PGResElab)==0){
			/*
			* Ordine ricevuto ma non ancora elaborato -> visualizzo la lista delle righe
			*/
			if (!bStorico){
				strcpy(szItem,"righe_ordine");
			} else {
				strcpy(szItem,"righe_ordine_storico");
			}
			/*
			* disattivo il pulsante di edit dei colli
			*/
			gtk_widget_set_sensitive(get_widget(dlg,"pb_edit_collo"),FALSE);
		} else {
			/*
			* Ordine prodotto -> visualizzo ls lista dei colli
			*/
			if (!bStorico){
				strcpy(szItem,"colli_ordine");
			} else {
				strcpy(szItem,"colli_ordine_storico");
			}
			gtk_widget_set_sensitive(get_widget(dlg,"pb_edit_collo"),TRUE);
		}
		PQclear(PGResElab);
	}

	PQclear(PGRes);

	/*
	* lettura della select dal file di configurazione
	*/
	CreateTable(dlg,"sw_list","lista_righe_ordine",NULL,NULL,NULL,"edit",szItem,GTK_SELECTION_SINGLE,pszCodiceOrdine);
}


char *GetStatoOrdine(char cStato,char *pszBuffer)
{
	switch(cStato){
		case ORDINE_RICEVUTO:
			strcpy(pszBuffer,"RICEVUTO");
		break;
		case ORDINE_SPEDITO:
			strcpy(pszBuffer,"SPEDITO");
		break;
		case ORDINE_ELABORATO:
			strcpy(pszBuffer,"ELABORATO");
		break;
		case ORDINE_IN_ANALISI:
			strcpy(pszBuffer,"IN ANALISI");
		break;
		case ORDINE_STAMPATA_RAC:
			strcpy(pszBuffer,"STAMPATO");
		break;
		case ORDINE_IN_PRELIEVO:
			strcpy(pszBuffer,"IN PRELIEVO");
		break;
		case ORDINE_PRELEVATO:
			strcpy(pszBuffer,"PRELEVATO");
		break;
		case ORDINE_EVASO:
			strcpy(pszBuffer,"EVASO");
		break;
		case ORDINE_STAMPATA_XAB:
			strcpy(pszBuffer,"STAMPATO XAB");
		break;
		case ORDINE_STAMPATA_DIST:
			strcpy(pszBuffer,"STAMPATO DIST");
		break;
		case ORDINE_SPEDITO_HOST:
			strcpy(pszBuffer,"SPEDITO A HOST");
		break;
		default:
			strcpy(pszBuffer,"NON DEFINITO");
		break;
	}
	return pszBuffer;
}


/*
* int GetKeyIndex(char *pszKey,GtkCList *clist,int nColumn,int nStartRow);
*
* effettua, nella GtkClist passatagli, la ricerca
* per il campo pszKey presente in colonna nColumn
* a partire dalla riga nStartRow
*/
int GetKeyIndex(char *pszKey,GtkCList *clist,int nColumn, int nStartRow)
{
	int nIndex=nStartRow;
	char *pszText;
	char szKey[128];

	strcpy(szKey,pszKey);

	/* la chiave della lista e' costituita sempre dal parametro nColumn */
	while(gtk_clist_get_text(GTK_CLIST(clist),nIndex,nColumn,&pszText)){
		if(!strcasecmp(szKey,pszText)){
			return(nIndex);
		}
		nIndex++;
	}
	return(-1);
}


void edit_dlg_prodotto(GtkWidget *dlg, char *szCodProd)
{
	PGresult *PGRes;
	BOOL bOK=TRUE;
	GList *lista_imballi=NULL;
	int nIndex;

	/*
	* riempio il combo contenente i codici imballo PCF
	*/
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select codice from imballi where ordtipo='%s' order by codice;",Cfg.szTipoOrdini);
	for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
		lista_imballi=g_list_append(lista_imballi,PQgetvalue(PGRes,nIndex,0));
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(get_widget(dlg,"combo_imballo_pcf")),lista_imballi);
	g_list_free(lista_imballi);
	PQclear(PGRes);
	/* accetto solo valori presenti in lista e non vuoto */
	gtk_combo_set_value_in_list(GTK_COMBO(get_widget(dlg,"combo_imballo_pcf")),TRUE,FALSE);

	gtk_label_printf(get_widget(dlg,"lb_prcdpro"),szCodProd);
	gtk_label_printf(get_widget(dlg,"lb_msg")," ");
	
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select prdstit,prpesgr,prlungh,prlargh,praltez,prswffo,prctffo,prqtffo,prfcpcf,prqtpcf,prqtpcp from catalogo where prcdpro='%s';",szCodProd);
	if (PQntuples(PGRes)){
		gtk_label_set(GTK_LABEL(get_widget(dlg,"lb_prdstit")),PQgetvalue(PGRes,0,0));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_peso")),atoi(PQgetvalue(PGRes,0,1)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza")),atoi(PQgetvalue(PGRes,0,2)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza")),atoi(PQgetvalue(PGRes,0,3)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza")),atoi(PQgetvalue(PGRes,0,4)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_swffo")),atoi(PQgetvalue(PGRes,0,5)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_ctffo")),atoi(PQgetvalue(PGRes,0,6)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtffo")),atoi(PQgetvalue(PGRes,0,7)));
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_imballo_pcf"))->entry), PQgetvalue(PGRes,0,8));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcf")),atoi(PQgetvalue(PGRes,0,9)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcp")),atoi(PQgetvalue(PGRes,0,10)));
	} else {
		bOK=FALSE;
	}
	PQclear(PGRes);

	if(bOK){
		/*
		* cerca prodotto in ubicazioni
		*/
		PGRes=PGExecSQL(Cfg.nDebugVersion,"select fila(ubicazione),montante(ubicazione),colonna(ubicazione),piano(ubicazione) from ubicazioni where codprod='%s' order by priorita;",szCodProd);
		if(PQresultStatus(PGRes)==PGRES_TUPLES_OK && PQntuples(PGRes)){
			gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_fila")),    PQgetvalue(PGRes,0,0));
			gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_montante")),PQgetvalue(PGRes,0,1));
			gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_colonna")), PQgetvalue(PGRes,0,2));
			gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_piano")),   PQgetvalue(PGRes,0,3));

			if (PQntuples(PGRes)>1){
				gtk_label_printf(get_widget(dlg,"lb_msg"),"Prodotto %s pluriubicato!",szCodProd);
			}
		}
		PQclear(PGRes);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Prodotto %s non presente in catalogo!",szCodProd);
		gtk_label_set(GTK_LABEL(get_widget(dlg,"lb_prdstit")),"");
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_peso")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_swffo")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_ctffo")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtffo")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_swpcf")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcf")),0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcp")),0);
	}
}

void edit_dlg_imballi(GtkWidget *dlg, char *pszCodiceImballo)
{
	PGresult *PGRes;

	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_codice")),pszCodiceImballo);
	
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select trim(DESCRIZ),LUNGHEZZA,ALTEZZA,LARGHEZZA,TARA,PESOMAX,VOLPERC,TPLAVOR,CATEGORIA from imballi where codice='%s';",pszCodiceImballo);

	if (PQntuples(PGRes)){
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_descrizione")),              PQgetvalue(PGRes,0,0));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza")), atoi(PQgetvalue(PGRes,0,1)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza")),   atoi(PQgetvalue(PGRes,0,2)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza")), atoi(PQgetvalue(PGRes,0,3)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tara")),      atoi(PQgetvalue(PGRes,0,4)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_pesomax")),   atoi(PQgetvalue(PGRes,0,5)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_volperc")),   atoi(PQgetvalue(PGRes,0,6)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tplavor")),   atoi(PQgetvalue(PGRes,0,7)));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_categoria")), atoi(PQgetvalue(PGRes,0,8)));
	}

	PQclear(PGRes);
}

void edit_dlg_ubicazioni(GtkWidget *dlg, char *szCodUbi)
{
	PGresult *PGRes;
	char szStato[40];

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select fila(UBICAZIONE),montante(UBICAZIONE),colonna(UBICAZIONE),piano(UBICAZIONE),CODPROD,ISOLA,SETTORE,DISPLAY,RIGA,COLONNA,PRIORITA,UBCDFLG,UBITIPO,CNISTATO from ubicazioni where ubicazione='%s';",szCodUbi);

	if (PQntuples(PGRes)){
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_fila")),        PQgetvalue(PGRes,0,0));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_montante")),    PQgetvalue(PGRes,0,1));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_colonna")),     PQgetvalue(PGRes,0,2));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_piano")),       PQgetvalue(PGRes,0,3));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_prodotto")),    PQgetvalue(PGRes,0,4));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_isola")),       PQgetvalue(PGRes,0,5));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_settore")),     PQgetvalue(PGRes,0,6));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_display")),     PQgetvalue(PGRes,0,7));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_riga_ios")),    PQgetvalue(PGRes,0,8));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_colonna_ios")), PQgetvalue(PGRes,0,9));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_priorita")),    PQgetvalue(PGRes,0,10));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_ubcdflg")),     PQgetvalue(PGRes,0,11));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_ubtipo")),      PQgetvalue(PGRes,0,12));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_stato")),       PQgetvalue(PGRes,0,13));
	}
	PQclear(PGRes);
	strcpy(szStato,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_stato"))));

	switch(szStato[0]){
		case 'P':
		case 'S':
			gtk_label_printf(get_widget(dlg,"lb_msg")," ");
		break;
		case 'Q':
		case 'T':
			gtk_label_printf(get_widget(dlg,"lb_msg"),"Ubicazione disabilitata");
		break;
		default:
			trace_debug(&cRED,"Stato ubicazione [%s] non corretto [%c]",szCodUbi,szStato[0]);
		break;
	}
}


char *get_ubicazione_in_edit(GtkWidget *dlg, char *pszUbicazione)
{
	PGresult *PGres;
	char szFilaUbi[64];
	char szMontanteUbi[64];
	char szColonnaUbi[64];
	char szPianoUbi[64];

	/* ricavo il codice dell'ubicazione attualmente in edit */
	strcpy(szFilaUbi,     gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_fila"))));
	strcpy(szMontanteUbi, gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_montante"))));
	strcpy(szColonnaUbi,  gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_colonna"))));
	strcpy(szPianoUbi,    gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_piano"))));

	PGres=PGExecSQL(Cfg.nDebugVersion>1,"select ubicazione from ubicazioni where fila(ubicazione)='%s' and montante(ubicazione)='%s' and colonna(ubicazione)='%s' and piano(ubicazione)='%s';",szFilaUbi,szMontanteUbi,szColonnaUbi,szPianoUbi);

	*pszUbicazione='\0';

	if(PQntuples(PGres)!=0){
		strcpy(pszUbicazione,PQgetvalue(PGres,0,0));
	} else {
		trace_debug(&cRED,"Errore in ricerca Ubicazione [%s%s%s%s]",szFilaUbi,szMontanteUbi,szColonnaUbi,szPianoUbi);
	}

	PQclear(PGres);

	return pszUbicazione;
}

void edit_dlg_operatori(GtkWidget *dlg, char *pszCodiceOperatore)
{
	PGresult *PGRes;

	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_codice")),pszCodiceOperatore);
	
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select OPNMOPE,OPCGOPE,OPINOPE,OPLOOPE,OPPROPE,OPCPOPE,OPNMTEL from operatori where opcdope='%s';",pszCodiceOperatore);

	if (PQntuples(PGRes)){
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_nome")),       PQgetvalue(PGRes,0,0));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cognome")),    PQgetvalue(PGRes,0,1));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_indirizzo")),  PQgetvalue(PGRes,0,2));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_localita")),   PQgetvalue(PGRes,0,3));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_provincia")),  PQgetvalue(PGRes,0,4));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cap")),        PQgetvalue(PGRes,0,5));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_telefono")),   PQgetvalue(PGRes,0,6));
	}

	PQclear(PGRes);
}

void edit_dlg_opeset     (GtkWidget *dlg, char *pszCodiceSet)
{
	PGresult *PGRes;
	GtkWidget *ComboOperatore = get_widget(dlg,"combo_operatore");
	GList *listaOperatori=NULL;
	int nIndex;

	/*
	* riempio il combo contenente i codici operatori
	*/
	PGRes=PGExecSQL(Cfg.nDebugVersion,"select opcdope from operatori order by opcdope;");
	for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
		listaOperatori=g_list_append(listaOperatori,PQgetvalue(PGRes,nIndex,0));
	}
	gtk_combo_set_popdown_strings(GTK_COMBO(ComboOperatore),listaOperatori);
	g_list_free(listaOperatori);
	PQclear(PGRes);

	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_settore")),pszCodiceSet);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select trim(operatore) from settori where settore='%s';",pszCodiceSet);

	if (PQntuples(PGRes)){
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_operatore")),  PQgetvalue(PGRes,0,0));
	}

	PQclear(PGRes);
}

void edit_dlg_collo(GtkWidget *dlg, char *pszOrdine, int nCollo)
{
	PGresult *PGRes = NULL;
	char szStato[128];
	char szBuffer[128];
	char szKey[128];
	int nTuples;
	/* N.B. da fare la gestione dello storico */
	BOOL bStorico=FALSE;
	if(!strcmp(szTableName,"ordini_storico")){
		bStorico=TRUE;
	}

	/* Cerco il collo */
	if (!bStorico){
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select cptpfor,cpstato,data(cptmeva),ora(cptmeva),cppspre,cppsrea from col_prod where ordprog='%s' and cpnmcol=%d;",pszOrdine,nCollo);
	} else {
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select cptpfor,cpstato,data(cptmeva),ora(cptmeva),cppspre,cppsrea from col_prod_stor where ordprog='%s' and cpnmcol=%d;",pszOrdine,nCollo);
	}
	nTuples=PQntuples(PGRes);

	szStato[0]='\0';

	if(nTuples){

		/*
		* assegno i valori alle label
		*/
		strcpy(szStato,PQgetvalue(PGRes,0,1));
		gtk_label_printf(get_widget(dlg,"lb_ORDPROG"),pszOrdine);
		gtk_label_printf(get_widget(dlg,"lb_NMCOL"),"%d",nCollo);
		gtk_label_printf(get_widget(dlg,"lb_TPFOR"),PQgetvalue(PGRes,0,0));
		gtk_label_printf(get_widget(dlg,"lb_STATO"), GetStatoOrdine(szStato[0],szBuffer));
		gtk_label_printf(get_widget(dlg,"lb_DATAEVA"), PQgetvalue(PGRes,0,2));
		gtk_label_printf(get_widget(dlg,"lb_ORAEVA"), PQgetvalue(PGRes,0,3));
		gtk_label_printf(get_widget(dlg,"lb_PSPRE"), "%7.2fKg",(float)((float)atoi(PQgetvalue(PGRes,0,4))/(float)1000));
		gtk_label_printf(get_widget(dlg,"lb_PSREA"), "%7.2fKg",(float)((float)atoi(PQgetvalue(PGRes,0,5))/(float)1000));

	}
	PQclear(PGRes);

	sprintf(szKey,"r.ordprog='%s' AND r.rpnmcol=%d",pszOrdine,nCollo);

	/*
	* lettura della select dal file di configurazione
	*/
	if (!bStorico){
		CreateTable(dlg,"sw_list","lista_righe_collo",NULL,NULL,NULL,"edit","righe_collo",GTK_SELECTION_SINGLE,szKey);
	} else {
		CreateTable(dlg,"sw_list","lista_righe_collo",NULL,NULL,NULL,"edit","righe_collo_storico",GTK_SELECTION_SINGLE,szKey);
	}
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
	char szKey[256];

	strcpy(szKey,pszKey);

	/* ricavo la lunghezza della chiave da cercare */
	nKeyLenght=strlen(szKey);

	while(gtk_clist_get_text(GTK_CLIST(clist),nIndex,nColumn,&pszText)){
		if(bCaseSensitive){
			if(!strncmp(szKey,pszText,nKeyLenght)){
				return(nIndex);
			}
		} else {
			if(!strncasecmp(pszKey,pszText,nKeyLenght)){
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

BOOL GetKeysList(char *szDBName,GtkCList *clist,int nRow,char *pszKey)
{
	int nIndex;
	int nKeyFields;
	int nFieldCount;
	DBSTRUCT DB;
	PFIELD pField;
	BOOL bOK=TRUE;
	char *pszText;
	char szText[1024];

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		trace_debug(&cRED,"Errore in lettura configurazione tabella [%s]",szDBName);
		return FALSE;
	}

	nKeyFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & KEY_MASK){
			nKeyFields++;
		}
	}

	strcpy(pszKey,"");
	nFieldCount=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & KEY_MASK){
			if(!gtk_clist_get_text(clist,nRow,nFieldCount,&pszText)){
				bOK=FALSE;
			} else {
				strcpy(szText,pszText);
				
				if(nFieldCount!=0){
					strcat(pszKey," AND ");
				}
				strcat(pszKey,pField->szFieldName);
				strcat(pszKey,"=");
				if(pField->cFieldType=='C' || pField->cFieldType=='D' ){
					strcat(pszKey,"'");
					strcat(pszKey,NormalizeString(szText));
					strcat(pszKey,"'");
				} else {
					strcat(pszKey,szText);
				}
			}
			nFieldCount++;
		}
	}
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}
	return bOK;
}

GtkWidget *CreateDBEditDlg(char *szDBName)
{
	GtkWidget *dlg=create_dlg_edit();
	GtkWidget *table_edit;
	GtkWidget *label_edit;
	GtkWidget *entry_edit;
	int nIndex;
	int nDisplayFields;
	int nFieldCount;
	char szBuffer[128];
	DBSTRUCT DB;
	PFIELD pField;

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		trace_debug(&cRED,"Errore in lettura configurazione tabella [%s]",szDBName);
		return NULL;
	}

	/* 
	* ricavo il numero di righe della tabella
	* (campi con bitmask DISPLAY a 1 )
	*/
	nDisplayFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & DISPLAY_MASK){
			nDisplayFields++;
		}
	}

	table_edit = gtk_table_new (nDisplayFields, 2, FALSE);
	gtk_widget_set_name (table_edit, "table_edit");
	gtk_widget_ref (table_edit);
	gtk_object_set_data_full (GTK_OBJECT (dlg), "table_edit", table_edit, (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table_edit);
	gtk_container_add (GTK_CONTAINER (get_widget(dlg,"frame_edit")), table_edit);
	gtk_container_set_border_width (GTK_CONTAINER (table_edit), 3);
	gtk_table_set_row_spacings (GTK_TABLE (table_edit), 2);
	gtk_table_set_col_spacings (GTK_TABLE (table_edit), 2);

	nFieldCount=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

		pField=DB.pFields[nIndex];

		/* visualizzare o non visualizzare  */
		if(pField->nFieldFlag & DISPLAY_MASK){
			if(pField->nFieldFlag & KEY_MASK){
				sprintf(szBuffer,"K - %s",pField->szFieldDescr);
			} else {
				sprintf(szBuffer,"%s",pField->szFieldDescr);
			}
			label_edit = gtk_label_new (szBuffer);

			sprintf(szBuffer,"lb_%s",pField->szFieldName);
			gtk_widget_set_name (label_edit, szBuffer);
			gtk_widget_ref (label_edit);
			gtk_object_set_data_full (GTK_OBJECT (dlg), szBuffer, label_edit, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (label_edit);
			gtk_table_attach (GTK_TABLE (table_edit), label_edit, 0, 1, nFieldCount, nFieldCount+1, (GtkAttachOptions) (GTK_FILL), (GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (label_edit), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (label_edit), 5, 0);


			switch(pField->cFieldType){
				case 'N':
					entry_edit = gtk_entry_new ();
					sprintf(szBuffer,"entry_%s",pField->szFieldName);
					gtk_widget_set_name (entry_edit, szBuffer);
					gtk_widget_ref (entry_edit);
					gtk_object_set_data_full (GTK_OBJECT (dlg), szBuffer, entry_edit, (GtkDestroyNotify) gtk_widget_unref);
					gtk_widget_show (entry_edit);
					gtk_table_attach (GTK_TABLE (table_edit), entry_edit, 1, 2, nFieldCount, nFieldCount+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

				break;
				default:
					entry_edit = gtk_entry_new ();
					sprintf(szBuffer,"entry_%s",pField->szFieldName);
					gtk_widget_set_name (entry_edit, szBuffer);
					gtk_widget_ref (entry_edit);
					gtk_object_set_data_full (GTK_OBJECT (dlg), szBuffer, entry_edit, (GtkDestroyNotify) gtk_widget_unref);
					gtk_widget_show (entry_edit);
					gtk_table_attach (GTK_TABLE (table_edit), entry_edit, 1, 2, nFieldCount, nFieldCount+1, (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), (GtkAttachOptions) (0), 0, 0);

				break;
			}
			gtk_entry_set_max_length (GTK_ENTRY(entry_edit),pField->nFieldLen);
			if((pField->nFieldFlag & KEY_MASK)){
				gtk_widget_set_foreground(entry_edit,&cGREEN);
				gtk_widget_set_background(entry_edit,&cBLACK);
			}
			if(!(pField->nFieldFlag & EDIT_MASK)){
				/* non editabile */
				gtk_editable_set_editable(GTK_EDITABLE(entry_edit),FALSE);
				gtk_widget_set_foreground(entry_edit,&cRED);
				gtk_widget_set_background(entry_edit,&cBLACK);
			}
			nFieldCount++;
		}
	}

	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}
	
	if(!(DB.nTableMask & INSERT_MASK)){
		gtk_widget_set_sensitive(get_widget(dlg,"pb_add"),FALSE);
	} else {
		gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_add")), "clicked", GTK_SIGNAL_FUNC (on_dlg_edit_pb_add_clicked), dlg);
	}
	if(!(DB.nTableMask & DELETE_MASK)){
		gtk_widget_set_sensitive(get_widget(dlg,"pb_del"),FALSE);
	} else {
		gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_del")), "clicked", GTK_SIGNAL_FUNC (on_dlg_edit_pb_del_clicked), dlg);
	}
	if(!(DB.nTableMask & CHANGE_MASK)){
		gtk_widget_set_sensitive(get_widget(dlg,"pb_mod"),FALSE);
	} else {
		gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_mod")), "clicked", GTK_SIGNAL_FUNC (on_dlg_edit_pb_mod_clicked), dlg);
	}

	gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_prev")), "clicked", GTK_SIGNAL_FUNC (on_dlg_edit_pb_prev_clicked), dlg);
	gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_next")), "clicked", GTK_SIGNAL_FUNC (on_dlg_edit_pb_next_clicked), dlg);

  return dlg;
}

BOOL UpdateDBEditDlg(GtkWidget *dlg,char *szDBName,char *szKey)
{
	GtkWidget *entry_edit;
	int nIndex;
	int nTuples;
	int nDisplayFields;
	int nFieldCount;
	char szBuffer[1024];
	char szSQLCmd[1024];
	DBSTRUCT DB;
	PFIELD pField;
	BOOL bOK=TRUE;
	PGresult *PGRes = NULL;

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		trace_debug(&cRED,"Errore in lettura configurazione tabella [%s]",szDBName);
		return FALSE;
	}

	nDisplayFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & DISPLAY_MASK){
			nDisplayFields++;
		}
	}

	strcpy(szSQLCmd,"select ");

	nFieldCount=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

		pField=DB.pFields[nIndex];


		if(pField->nFieldFlag & DISPLAY_MASK){

			if(nFieldCount!=0){
				strcat(szSQLCmd," ,");
			}

			sprintf(szBuffer,"entry_%s",pField->szFieldName);
			if((entry_edit = get_widget(dlg,szBuffer))){
				strcat(szSQLCmd,pField->szFieldName);
			}
			nFieldCount++;
		}
	}
	sprintf(szBuffer," from %s where %s;",szDBName,szKey);
	strcat(szSQLCmd,szBuffer);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
	if((nTuples=PQntuples(PGRes))){
		nFieldCount=0;
		for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

			pField=DB.pFields[nIndex];

			if(pField->nFieldFlag & DISPLAY_MASK){
				sprintf(szBuffer,"entry_%s",pField->szFieldName);
				if((entry_edit = get_widget(dlg,szBuffer))){
					switch(pField->cFieldType){
						case 'N':
							gtk_entry_set_text(GTK_ENTRY(entry_edit),PQgetvalue(PGRes,0,nFieldCount));
						break;
						default:
							gtk_entry_set_text(GTK_ENTRY(entry_edit),PQgetvalue(PGRes,0,nFieldCount));
						break;
					}
				}
				nFieldCount++;
			}
		}
	} else {
		bOK=FALSE;
		for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

			pField=DB.pFields[nIndex];

			if(pField->nFieldFlag & DISPLAY_MASK){
				sprintf(szBuffer,"entry_%s",pField->szFieldName);
				if((entry_edit = get_widget(dlg,szBuffer))){
					gtk_entry_set_text(GTK_ENTRY(entry_edit),"");
				}
				nFieldCount++;
			}
		}
	}
	PQclear(PGRes);

	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}

	return bOK;
}
