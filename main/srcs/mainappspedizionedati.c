#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainappspedizionedati.h"
#include "mainfun.h"

SpedizioneType _type;

struct _MainAppSpedizioneDati
{
    GtkDialog parent;
};

typedef struct _MainAppSpedizioneDatiPrivate MainAppSpedizioneDatiPrivate;

struct _MainAppSpedizioneDatiPrivate
{
    SpedizioneType type;
    gpointer main_app_window;
    GtkWidget *lb_1;
    GtkWidget *lb_2;
    GtkWidget *lb_ordini;
    GtkWidget *lb_colli;
    GtkWidget *lb_righe;
    GtkWidget *lb_linee;
    GtkWidget *pr_ordini;
    GtkWidget *pr_colli;
    GtkWidget *pr_righe;
    GtkWidget *pr_linee;
    GtkWidget *pb_ok;
    GtkWidget *pb_append;
    GtkWidget *pb_overwrite;
    GtkWidget *pb_close;
	int nValuesOrdini;
	int nValuesColli;
	int nValuesRighe;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppSpedizioneDati, main_app_spedizione_dati, GTK_TYPE_DIALOG)

/*
* Invio dati a corriere - SDA (scrittura file)
*/
void SpedizioneDatiSDA(gpointer win, gboolean storico, gboolean overwrite)
{
    DBSTRUCT tCorrSDA;    /* Export Dati Corriere - SDA */
    gboolean ok = TRUE;
	FILE *fpExport;
	FILE *fp;
    char szDateBuffer[128];
    char szTimeBuffer[128];
	char szCommand[256];
	char szExportFileName[128];
	char szMonitor[128];
	char szAperturaFile[128];
	int nTuples=0;
	int nIndex;
	DBresult *DBRes;
	DBresult *DBResOrdini;
	DBresult *DBResCorriere;
	DBresult *DBResNote;
	ep_bool_t bFound=TRUE;
	char szORDPROG[128];
	char szRODSCLI[128];
	char szROINCLI[128];
	char szROLOCLI[128];
	char szROCPCLI[128];
	char szROPRCLI[128];
	char szRONMCED[128];
	char szROTPSPE[128];
	char szROCDLIN[128];
	char szRONMCLL[128];
	char szROPSREA[128];
	char szRONMCPE[128];
	char szROVLCOE[128];
	char szCPBRCDE[128];
	char szDSTSP[256];
	char szDSNOT[256];
	int nROPSREA;
	int nROPSPRE;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");

    ReadTableInfo(&tCorrSDA,          Cfg.szPathData,"corr_sda");

	sprintf(szExportFileName,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorrSDA));
	sprintf(szMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportMonitor));

    strcpy(szAperturaFile, overwrite?"w":"a");

	if((fpExport=fopen(szExportFileName,szAperturaFile))==NULL){
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in apertura file dati [%s]",szExportFileName);
		return;
	}

    gtk_label_printf(priv->lb_1,"Spedizione Dati Corriere SDA in corso");
    gtk_label_printf(priv->lb_2,"Attendere la fine della procedura");

    gtk_widget_set_sensitive(priv->pb_ok,        FALSE);
    gtk_widget_set_sensitive(priv->pb_append,    FALSE);
    gtk_widget_set_sensitive(priv->pb_overwrite, FALSE);
    gtk_widget_set_sensitive(priv->pb_close,     FALSE);


    /* Tabella temporanea per invio dati corriere - SDA
    *
    *   -------  - --- ---------------------
    *   CRCDCLI  C  11 Codice cliente mittente           '769971007  '
    *   CRTPSRV  C   1 Tipo servizio                     'N'
    *   CRFILLR  C   1 Filler                            ' '
    *   CRGSTOP  C   1 Gestione operativa                'N'
    *   CRCDFLG  C   1 Flag                              '0'
    *   CRFRMDP  C   1 Fermo deposito                    '0'
    *   CRRSDST  C  40 Ragione sociale destinatario     
    *   CRINDST  C  35 Indirizzo destinatario          
    *   CRTLDST  C  15 Telefono destinatario          
    *   CRCPDST  C   5 Cap destinatario              
    *   CRLCDST  C  30 Localita destinatario        
    *   CRPVDST  C   2 Provincia destinatario      
    *   CRTPMRC  C   1 Tipo merce                        'C'
    *   CRCNMRC  C  20 Contenuto merce                   '<copie>LIBRI '
    *   CRNMCLL  C   5 Numero dei colli                  
    *   CRPSSPD  C   8 Peso dei colli in kg             
    *   CRLUNGH  C   3 Dimensione lunghezza cm        
    *   CRALTEZ  C   3 Dimensione altezza cm    
    *   CRLARGH  C   3 Dimensione larghezza cm 
    *   CRRFMTT  C  10 Riferimento mittente DDT          '<spedizione>'
    *   CRISTRZ  C  80 Istruzioni particolari            '<note corriere>'
    *   CRVLCNT  C   3 Valuta contrassegno (EUR o LIT)   'EUR'
    *   CRIMCNT  C  12 Importo contrassegno              <valore contrassegno in euri/00>
    *   CRVLASS  C   3 Valuta assicurazione (EUR o LIT)  'EUR'
    *   CRIMASS  C  12 Importo assicurazione             
    *   CRCMCNT  C   1 Commissioni contrassegno (flag)   
    *   CRVOLUM  C   7 Volume in metri cubi              
    *   CRCDRGR  C  16 Codice raggruppamento spedizione  '<spedizione>'
    *   CRRIFCL  C  26 Riferimento collo                 '<barcode collo>'
    *   CRRIFDF  C   5 Riferimento F.M. divisione fattura 
    *   CRRIFAF  C  15 Riferimento F.M. allegato fattura 
    *   CRRIFUS  C   8 Riferimento F.M. unificazione spediz.
    *   CRRIFNO  C   5 Riferimento F.M. numero offerta    
    *   CRTPPRT  C   1 Tipo porto                        
    *   CRRIFLV  C  11 Riferimento TNT: lettera di vettura  
    *   CRRIFFS  C   9 Riferimento TNT: Filiale, Segnacollo 
    *   CRTPMEZ  C   1 Tipo mezzo                        'C'
    *   -------  - --- ---------------------
    */

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table corr_sda_%s_%s;", Cfg.szTmpSuffix,Cfg.szTipoOrdini);
	DBFreeQueryResult(DBRes);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table corr_sda_%s_%s (\
		CRCDCLI  char(11 ), CRTPSRV  char( 1 ), CRFILLR  char( 1 ), CRGSTOP  char( 1 ), CRCDFLG  char( 1 ), CRFRMDP  char( 1 ), CRRSDST  char(40 ), CRINDST  char(35 ), CRTLDST  char(15 ),\
		CRCPDST  char( 5 ), CRLCDST  char(30 ), CRPVDST  char( 2 ), CRTPMRC  char( 1 ), CRCNMRC  char(20 ), CRNMCLL  char( 5 ), CRPSSPD  char( 8 ), CRLUNGH  char( 3 ), CRALTEZ  char( 3 ),\
		CRLARGH  char( 3 ), CRRFMTT  char(10 ), CRISTRZ  char(80 ), CRVLCNT  char( 3 ), CRIMCNT  char(12 ), CRVLASS  char( 3 ), CRIMASS  char(12 ), CRCMCNT  char( 1 ), CRVOLUM  char( 7 ),\
		CRCDRGR  char(16 ), CRRIFCL  char(26 ), CRRIFDF  char( 5 ), CRRIFAF  char(15 ), CRRIFUS  char( 8 ), CRRIFNO  char( 5 ), CRTPPRT  char( 1 ), CRRIFLV  char(11 ), CRRIFFS  char( 9 ),\
		CRTPMEZ  char( 1 ));",Cfg.szTmpSuffix,Cfg.szTipoOrdini);

	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in creazione tabella dati corriere SDA");
        ok=FALSE;
	}

	DBFreeQueryResult(DBRes);

	/*
	* seleziono gli ordini 
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select \
			ro.ordprog,\
			ro.rodscli,\
			ro.roincli,\
			ro.rocpcli,\
			ro.rolocli,\
			ro.roprcli,\
			ro.ronmcll,\
			ro.ropsrea,\
			ro.ropspre,\
			ro.ronmcpe,\
			ro.rovlcoe,\
			ro.ronmced,\
			ro.rotpspe,\
			ro.rocdlin,\
			cp.cpbrcde \
			from %s ro,sel_ord_tmp_%s_%s so,%s cp where ro.ordprog=so.ordprog and ordtipo='%s' and cp.ordprog=so.ordprog order by ro.ordprog,cp.cpnmcol;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,Cfg.szTipoOrdini,
		storico?"col_prod_stor":"col_prod",
		Cfg.szTipoOrdini);


	if((nTuples=DBntuples(DBResOrdini))){

        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"DatiCorrSDA() : (%d:Colli)",nTuples);

		for (nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szORDPROG,DBgetvalue(DBResOrdini,nIndex,0));
			strcpy(szRODSCLI,DBgetvalue(DBResOrdini,nIndex,1));
			strcpy(szROINCLI,DBgetvalue(DBResOrdini,nIndex,2));
			strcpy(szROCPCLI,DBgetvalue(DBResOrdini,nIndex,3));
			strcpy(szROLOCLI,DBgetvalue(DBResOrdini,nIndex,4));
			strcpy(szROPRCLI,DBgetvalue(DBResOrdini,nIndex,5));
			sprintf(szRONMCLL,"%d",atoi(DBgetvalue(DBResOrdini,nIndex,6)));

			nROPSREA=atoi(DBgetvalue(DBResOrdini,nIndex,7));
			nROPSPRE=atoi(DBgetvalue(DBResOrdini,nIndex,8));

			sprintf(szROPSREA,"%.2f",(float)(nROPSREA?nROPSREA:nROPSPRE)/(float)1000);

			sprintf(szRONMCPE,"%d LIBRI",atoi(DBgetvalue(DBResOrdini,nIndex,9)));
			sprintf(szROVLCOE,"%.2f",(float)atoi(DBgetvalue(DBResOrdini,nIndex,10))/(float)100);
			strcpy(szRONMCED,DBgetvalue(DBResOrdini,nIndex,11));
			StrTrimAll(szRONMCED);
			strcpy(szROTPSPE,DBgetvalue(DBResOrdini,nIndex,12));
			strcpy(szROCDLIN,DBgetvalue(DBResOrdini,nIndex,13));
			strcpy(szCPBRCDE,DBgetvalue(DBResOrdini,nIndex,14));

			strcpy(szDSNOT,"");


			strcpy(szORDPROG,RightStr(szORDPROG,6));
			/*
			* rm 27-09-2004 : aggiunto campo cdlin in ricerca su ttds
			*               
			*/
			DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='%s';",
				Cfg.szCDSOC, Cfg.szCDMAG, szRONMCED,szROTPSPE,szROCDLIN);
			if(DBntuples(DBResNote)){
				strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
				strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
				bFound=TRUE;
			} else {
				bFound=FALSE;
			}
			DBFreeQueryResult(DBResNote);

			if(!bFound){
				/* rm 27-09-2004 : primo giro bis: cerco con cdlin = '' */
				DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s' and elccdlin='';",
					Cfg.szCDSOC, Cfg.szCDMAG, szRONMCED,szROTPSPE);
				if(DBntuples(DBResNote)){
					strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
					strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
					bFound=TRUE;
				} else {
					bFound=FALSE;
				}
				DBFreeQueryResult(DBResNote);
			}

			if(!bFound){
				/* secondo giro : cerco con tpspe = ** */
				DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elccdsoc='%s' and elccdmag='%s' and elcnmced='%s' and elctpspe='%s';",
					Cfg.szCDSOC, Cfg.szCDMAG, szRONMCED,"**");
				if(DBntuples(DBResNote)){
					strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
					strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
					bFound=TRUE;
				} else {
					bFound=FALSE;
					strcpy(szDSTSP,"");
					strcpy(szDSNOT,"");
                    trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Ordine [%s] manca campo note corriere [%s-%s]",szORDPROG,szRONMCED,szROTPSPE);
				}
				DBFreeQueryResult(DBResNote);
			}


			strcpy(szRODSCLI,NormalizeString(szRODSCLI));
			strcpy(szROINCLI,NormalizeString(szROINCLI));
			strcpy(szROCPCLI,NormalizeString(szROCPCLI));
			strcpy(szROLOCLI,NormalizeString(szROLOCLI));
			strcpy(szROPRCLI,NormalizeString(szROPRCLI));
			strcpy(szDSNOT,NormalizeString(szDSNOT));

			strcpy(szRODSCLI,Substitute(szRODSCLI,'\"',' '));
			strcpy(szROINCLI,Substitute(szROINCLI,'\"',' '));
			strcpy(szROLOCLI,Substitute(szROLOCLI,'\"',' '));
			strcpy(szDSNOT,  Substitute(szDSNOT,'\"',' '));

			DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"insert into corr_sda_%s_%s ( \
				CRCDCLI, CRTPSRV, CRGSTOP, CRCDFLG, CRFRMDP, CRRSDST, CRINDST, CRCPDST,\
				CRLCDST, CRPVDST, CRTPMRC, CRCNMRC, CRNMCLL, CRPSSPD, CRRFMTT, CRISTRZ,\
				CRVLCNT, CRIMCNT, CRVLASS, CRCDRGR, CRRIFCL, CRTPMEZ) \
                values ( '%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s' );",
				Cfg.szTmpSuffix,Cfg.szTipoOrdini,
				"769971007",    /* codice cliente mitt. - fisso */
				"N",            /* Tipo Servizio */
				"N",            /* Gestione Operativa */
				"0",            /* Flag */
				"0",            /* Fermo Deposito */
				szRODSCLI,
				szROINCLI,
				szROCPCLI,
				szROLOCLI,
				szROPRCLI,
				"C",            /* Tipo Merce */
				szRONMCPE,      /* Contenuto Merce : '<copie> LIBRI' */
				szRONMCLL,
				szROPSREA,
				szORDPROG,
				szDSNOT,        /* Istruzioni - note corriere */
				"EUR",          /* Valuta contrassegno */
				szROVLCOE,
				"EUR",          /* Valuta Assicurazione */
				szORDPROG,
				szCPBRCDE,
				"C");


			if (DBresultStatus(DBResCorriere) != DBRES_COMMAND_OK ) {
                trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Ordine [%s] Errore in insert dati corriere SDA",szORDPROG);
			} 
			DBFreeQueryResult(DBResCorriere);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}
	}
	DBFreeQueryResult(DBResOrdini);

	DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"select \
			CRCDCLI, CRTPSRV, CRFILLR, CRGSTOP, CRCDFLG, CRFRMDP, CRRSDST, CRINDST, CRTLDST, CRCPDST, CRLCDST, \
            CRPVDST, CRTPMRC, CRCNMRC, CRNMCLL, CRPSSPD, CRLUNGH, CRALTEZ, CRLARGH, CRRFMTT, CRISTRZ, CRVLCNT, \
            CRIMCNT, CRVLASS, CRIMASS, CRCMCNT, CRVOLUM, CRCDRGR, CRRIFCL, CRRIFDF, CRRIFAF, CRRIFUS, CRRIFNO, \
            CRTPPRT, CRRIFLV, CRRIFFS, CRTPMEZ \
            from corr_sda_%s_%s order by CRRFMTT,CRRIFCL;", Cfg.szTmpSuffix,Cfg.szTipoOrdini);

	if((nTuples=DBntuples(DBResCorriere))){

        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs, "DatiCorrSDA() : (%d:Colli)",nTuples);

		for (nIndex=0;nIndex<nTuples;nIndex++){

			/*
			* spedizione dell'ordine a Host
			*/
			WriteAsciiRecord(fpExport, &tCorrSDA,DBResCorriere,nIndex,TRUE,FALSE);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}
        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs, "Registrati %d colli",nTuples);
	}
	DBFreeQueryResult(DBResCorriere);

	if(fpExport) fclose(fpExport); 

	/* 
	* 23-03-2001 rm : copia di sicurezza 
	*/	
	sprintf(szCommand,"cp %s %s.old",szExportFileName,szExportFileName);
	system(szCommand);

	gtk_label_printf(priv->lb_1,"Procedura di spedizione dati terminata");
	gtk_label_printf(priv->lb_2,"Attendere. invio dati a host in corso");

	/*
	* creo il monitor
	*/
	if((fp=fopen(szMonitor,"w"))){
		fprintf(fp,"%s : %s %s",
			szMonitor, 
			GetDate(time((long *)0),szDateBuffer), 
			GetTime(time((long *)0),szTimeBuffer));

			fclose(fp);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Fallita creazione monitor di import dati");
	}

	gtk_label_printf(priv->lb_1,"Procedura di invio dati a Host terminata");
	gtk_label_printf(priv->lb_2,"Premere il tasto Cancel per uscire");
    gtk_widget_set_sensitive(priv->pb_close,TRUE);
}

/*
* Invio dati a corriere (scrittura file)
*/
void SpedizioneDatiCorriere(gpointer win, gboolean storico, gboolean overwrite)
{
    DBSTRUCT tCorriere;    /* Export Dati Corriere */
	FILE *fpExport;
	FILE *fp;
    char szDateBuffer[128];
    char szTimeBuffer[128];
	char szAperturaFile[256];
	char szCommand[256];
	char szExportFile[128];
	char szMonitor[128];
	char szOrdProg[128];
	char szAADOC[128];
	char szPRDOC[128];
	char szDSCLI[128];
	char szINCLI[128];
	char szLOCLI[128];
	char szCPCLI[128];
	char szPRCLI[128];
	char szNMCED[128];
	char szTPSPE[128];
	char szCDLIN[128];
	char szCDVET[128];
	char szDSTSP[128];
	char szDSNOT[256];
	char szFLNOT[256];
	int nNMCLL;
	int nVLCON;
	int nSWCOL;
	int nPSPRE;
	int nPSREA;
	int nTuples=0;
	int nIndex;
	DBresult *DBRes;
	DBresult *DBResOrdini;
	DBresult *DBResCorriere;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");

    ReadTableInfo(&tCorriere, Cfg.szPathData,"corriere");

	sprintf(szExportFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorriere));
	sprintf(szMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportMonitor));

    strcpy(szAperturaFile, overwrite?"w":"a");

	if((fpExport=fopen(szExportFile,szAperturaFile))==NULL){
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "SpedizioneDatiCorriere() : Errore in apertura file [%s]",szExportFile);
		return;
	}

	gtk_label_printf(priv->lb_1,"Spedizione Dati Corriere in corso");
	gtk_label_printf(priv->lb_2,"Attendere la fine della procedura");
    gtk_widget_set_sensitive(priv->pb_ok,        FALSE);
    gtk_widget_set_sensitive(priv->pb_append,    FALSE);
    gtk_widget_set_sensitive(priv->pb_overwrite, FALSE);
    gtk_widget_set_sensitive(priv->pb_close,     FALSE);

    /* Tabella temporanea per invio dati corriere
    *
    *   -------  - --- ---------------------
            crcdcli  C  11 Codice Cliente 
            crflr01  N   5 Filler Field 01 
            crdscli  C  40 Descrizione Cliente 
            crincli  C  50 Indirizzo Cliente 
            crcpcli  C   5 CAP Cliente 
            crlocli  C  30 Localita' Cliente
            crprcli  C   2 Provincia Cliente 
            crtpimb  C   1 Tipo Imballo 
            crcntsp  C  20 Contenuto Spedizione 
            crnmcll  N   5 Numero Colli Spedizione 
            crpsspe  N   8 Peso Spedizione 
            crflr02  N   9 Filler Field 02 
            craadoc  C   2 Anno Spedizione 
            crprdoc  C   6 Codice Spedizione 
            crflr03  N   2 Filler Field 03 
            crdsnot  C  80 Note Spedizione 
            crvlcon  N  29 Valore Contrassegno 
    *		crflr04  N  24 Filler Field 04 
    *   -------  - --- ---------------------
    */

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table corriere_%s_%s;", Cfg.szTmpSuffix,Cfg.szTipoOrdini);
	DBFreeQueryResult(DBRes);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table corriere_%s_%s (\
		crcdcli char(11),\
		crflr01 int4,\
		crdscli char(40),\
		crincli char(50),\
		crcpcli char(5),\
		crlocli char(30),\
		crprcli char(2),\
		crtpimb char(1),\
		crcntsp char(20),\
		crnmcll int4,\
		crpsspe int4,\
		crflr02 int4,\
		craadoc char(2),\
		crprdoc char(6),\
		crflr03 int4,\
		crdsnot char(80),\
		crvlcon int4,\
		crflr04 char(24)\
	);",Cfg.szTmpSuffix,Cfg.szTipoOrdini);

	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Errore in creazione tabella dati corriere");
	}

	DBFreeQueryResult(DBRes);

	/*
	* seleziono gli ordini 
	* LG 28-12-2011 aggiungo il corriere rocdve2
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select \
		ro.ordprog, ro.rodscli, ro.roincli, \
		ro.rocpcli, ro.rolocli, ro.roprcli, ro.ronmcll, \
		ro.rovlcoe, ro.roswcol, ro.ropspre, ro.ropsrea, \
		ro.ronmced,ro.rotpspe,ro.rocdlin, ro.rocdve2, \
		ro.rodsnot, ro.rodstsp, ro.roflnot \
		from %s ro,sel_ord_tmp_%s_%s so where ro.ordprog=so.ordprog and ordtipo='%s' order by ro.ordprog;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,
		Cfg.szTipoOrdini,
		Cfg.szTipoOrdini);

	if((nTuples=DBntuples(DBResOrdini))){

#ifdef TRACE
		trace_debug(TRUE, TRUE, "SpedizioneDatiCorriere() : (%d:Ordini)",nTuples);
#endif

		for (nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szOrdProg,DBgetvalue(DBResOrdini,nIndex,0));
			strcpy(szAADOC,SubStr(szOrdProg,9,2));
			strcpy(szPRDOC,SubStr(szOrdProg,11,6));
			strcpy(szDSCLI,DBgetvalue(DBResOrdini,nIndex,1));
			strcpy(szINCLI,DBgetvalue(DBResOrdini,nIndex,2));
			strcpy(szCPCLI,DBgetvalue(DBResOrdini,nIndex,3));
			strcpy(szLOCLI,DBgetvalue(DBResOrdini,nIndex,4));
			strcpy(szPRCLI,DBgetvalue(DBResOrdini,nIndex,5));
			nNMCLL=atoi(DBgetvalue(DBResOrdini,nIndex,6));
			nVLCON=atoi(DBgetvalue(DBResOrdini,nIndex,7));
			nSWCOL=atoi(DBgetvalue(DBResOrdini,nIndex,8));
			nPSPRE=atoi(DBgetvalue(DBResOrdini,nIndex,9));
			nPSREA=atoi(DBgetvalue(DBResOrdini,nIndex,10));
			strcpy(szNMCED,DBgetvalue(DBResOrdini,nIndex,11));
			StrTrimAll(szNMCED);
			strcpy(szTPSPE,DBgetvalue(DBResOrdini,nIndex,12));
			strcpy(szCDLIN,DBgetvalue(DBResOrdini,nIndex,13));
			strcpy(szCDVET,DBgetvalue(DBResOrdini,nIndex,14));
			strcpy(szFLNOT,DBgetvalue(DBResOrdini,nIndex,17));

			strcpy(szDSNOT,Cfg.szNotaCorriere);

			
			/*
			* lg 27-02-2012 : ora il dato deve essere preso da ric_ord. Il dato deve essere precedentemente settato
			* 								in fase di lancio ordine o stampa etichette
			* 								altrimenti lo ricalcolo?
			*/
			if (szFLNOT[0]==NOTA_CORRIERE_SETTATA) {
				strcpy(szDSNOT,DBgetvalue(DBResOrdini,nIndex,15));
				strcpy(szDSTSP,DBgetvalue(DBResOrdini,nIndex,16));
			} else {
                trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Ordine [%s] manca campo note corriere [%s-%s]",szOrdProg,szNMCED,szTPSPE);
			}

			strcpy(szDSCLI,NormalizeString(szDSCLI));
			strcpy(szINCLI,NormalizeString(szINCLI));
			strcpy(szCPCLI,NormalizeString(szCPCLI));
			strcpy(szLOCLI,NormalizeString(szLOCLI));
			strcpy(szPRCLI,NormalizeString(szPRCLI));
			strcpy(szDSNOT,NormalizeString(szDSNOT));

			DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"insert into corriere_%s_%s ( crcdcli, crflr01, crdscli, crincli, crcpcli, crlocli, crprcli, crtpimb, crcntsp, crnmcll, crpsspe, crflr02, craadoc, crprdoc, crflr03, crdsnot, crvlcon, crflr04 ) values ( '%s',%d,'%s','%s', '%s','%s','%s','%c', '%s',%d,%d,%d, '%s','%s',%d,'%s', %d,'%s');",
				Cfg.szTmpSuffix,
				Cfg.szTipoOrdini,
				"00769971002",        /* Codice cliente - fisso */
				0,
				szDSCLI,
				szINCLI,
				szCPCLI,
				szLOCLI,
				szPRCLI,
				nSWCOL==4?'P':'C',    /* Spedizione a pallet : P / colli : C */
				"LIBRI",              /* Contenuto spedizione - fisso */
				nNMCLL,
				nPSREA?nPSREA:nPSPRE, /* Peso spedizione - se non esiste il peso reale associo il peso calcolato */
				0,
				szAADOC,
				szPRDOC,
				0,
				szDSNOT,
				nVLCON,"");
			if (DBresultStatus(DBResCorriere) != DBRES_COMMAND_OK ) {
                trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Ordine [%s] Errore in insert dati corriere",szOrdProg);
			} 
			DBFreeQueryResult(DBResCorriere);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}
	}
	DBFreeQueryResult(DBResOrdini);


	DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"select \
		crcdcli, crflr01, crdscli, crincli, crcpcli, crlocli,\
		crprcli, crtpimb, crcntsp, crnmcll, crpsspe, crflr02,\
		craadoc, crprdoc, crflr03, crdsnot, crvlcon, crflr04 from corriere_%s_%s order by crprdoc;",
		Cfg.szTmpSuffix,
		Cfg.szTipoOrdini);

	if((nTuples=DBntuples(DBResCorriere))){

        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs, "SpedizioneDatiCorriere() : (%d:Ordini)",nTuples);

		for (nIndex=0;nIndex<nTuples;nIndex++){

			/*
			* spedizione dell'ordine a Host
			*/
			WriteAsciiRecord(fpExport, &tCorriere,DBResCorriere,nIndex,TRUE,FALSE);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}

        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs, "Registrati %d ordini",nTuples);
	}
	DBFreeQueryResult(DBResCorriere);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,
        "update ric_ord set rostato='%c',rofliem='%c',roprgem=%d where ordprog in (select ordprog from sel_ord_tmp_%s_%s) ;",
		ORDINE_FILE_INVIATO, EMAIL_VETTORE_SPEDITA_MANUALE , 0, Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ){
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Errore in aggiornamento ordini spediti",nTuples);
	}
	DBFreeQueryResult(DBRes);

	if(fpExport) fclose(fpExport); 

	/* 
	* 23-03-2001 rm : copia di sicurezza 
	*/	
	sprintf(szCommand,"cp %s %s.old",szExportFile,szExportFile);
	system(szCommand);

	gtk_label_printf(priv->lb_1,"Procedura di spedizione dati corriere terminata");
	gtk_label_printf(priv->lb_2,"Attendere. invio dati a host in corso");


	/*
	* creo il monitor
	*/
	if((fp=fopen(szMonitor,"w"))){
		fprintf(fp,"%s : %s %s",
			szMonitor, 
			GetDate(time((long *)0),szDateBuffer), 
			GetTime(time((long *)0),szTimeBuffer));

			fclose(fp);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs, "Fallita creazione monitor di import dati");
	}

	gtk_label_printf(priv->lb_1,"Procedura di invio dati a Host terminata");
	gtk_label_printf(priv->lb_2,"Premere il tasto close per uscire");
    gtk_widget_set_sensitive(priv->pb_close, TRUE);
}

/*
* Invio dati a corriere - aggiuntivo (scrittura file)
*/
void SpedizioneDatiCorrAgg(gpointer win, gboolean storico, gboolean overwrite)
{
    DBSTRUCT tCorrAgg;    /* Export Dati Corriere Aggiuntivi */
	FILE *fpExport;
	FILE *fp;
    char szDateBuffer[128];
    char szTimeBuffer[128];
	char szAperturaFile[256];
	char szCommand[256];
	char szExportFile[128];
	char szMonitor[128];
	char szOrdProg[128];
	char szAADOC[128];
	char szPRDOC[128];
	char szBRCDE[128];
	int nTuples=0;
	int nIndex;
	DBresult *DBRes;
	DBresult *DBResOrdini;
	DBresult *DBResCorriere;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");

    ReadTableInfo(&tCorrAgg, Cfg.szPathData,"corr_agg");

	sprintf(szExportFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorrAgg));
	sprintf(szMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportMonitor));

    strcpy(szAperturaFile, overwrite?"w":"a");

	if((fpExport=fopen(szExportFile,szAperturaFile))==NULL){
#ifdef TRACE
		trace_debug(TRUE, TRUE, "DatiCorrAgg() : Errore in apertura file [%s]",szExportFile);
#endif
		gtk_text_printf("CYAN",txt_msgs,"DatiCorrAgg() : Errore in apertura file export [%s]\n",szExportFile);
		return;
	}

	gtk_label_printf(priv->lb_1,"Spedizione Dati Corriere Aggiuntivi in corso");
	gtk_label_printf(priv->lb_2,"Attendere la fine della procedura");
    gtk_widget_set_sensitive(priv->pb_ok,        FALSE);
    gtk_widget_set_sensitive(priv->pb_append,    FALSE);
    gtk_widget_set_sensitive(priv->pb_overwrite, FALSE);
    gtk_widget_set_sensitive(priv->pb_close,     FALSE);

    /* Tabella temporanea per invio dati corriere - Aggiuntivo
    *
    *   -------  - --- ---------------------
            cacdcli  C  11 Codice Cliente Mittente + POP
            caanspe  C   4 Anno di spedizione
            canrspe  C   2 Numero serie spedizione
            caspedz  C   7 Numero Spedizione
            catprec  C   1 Tipo Record
            cabrcde  C  35 Barcode A&L
    *   -------  - --- ---------------------
    */

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table corr_agg_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBFreeQueryResult(DBRes);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table corr_agg_%s_%s (\
		cacdcli  char(11),\
		caanspe  char(4),\
		canrspe  char(2),\
		caspedz  char(7),\
		catprec  char(1),\
		cabrcde  char(35)\
	);", Cfg.szTmpSuffix, Cfg.szTipoOrdini);

	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in creazione tabella dati corriere aggiuntivi");
        return;
	}

	DBFreeQueryResult(DBRes);

	/*
	* seleziono gli ordini 
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select ro.ordprog, cp.cpbrcde from %s ro,sel_ord_tmp_%s_%s so,%s cp where ro.ordprog=so.ordprog and ordtipo='%s' and cp.ordprog=ro.ordprog order by ro.ordprog,cp.cpnmcol;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,
		Cfg.szTipoOrdini,
		storico?"col_prod_stor":"col_prod",
		Cfg.szTipoOrdini);

	if((nTuples=DBntuples(DBResOrdini))){

        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"DatiCorrAgg() : (%d:Colli)",nTuples);

		for (nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szOrdProg,DBgetvalue(DBResOrdini,nIndex,0));
			sprintf(szAADOC,"%04d",2000+atoi(SubStr(szOrdProg,9,2)));
			strcpy(szPRDOC,SubStr(szOrdProg,11,6));
			strcpy(szBRCDE,DBgetvalue(DBResOrdini,nIndex,1));

			DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"insert into corr_agg_%s_%s ( cacdcli, caanspe, canrspe, caspedz, catprec, cabrcde ) values ( '%s','%s','%s','%s','%s','%s' );",
				Cfg.szTmpSuffix,
				Cfg.szTipoOrdini,
				" 0430561043",        /* codice cliente mitt. - fisso */
				szAADOC,              /* anno della spedizione */
				"  ",                 /* numero di serie della sped. - fisso */
				szPRDOC,              /* numero spedizione (PRDOC) */
				"E",                  /* tipo record - fisso */
				szBRCDE);             /* barcode A&L collo */

			if (DBresultStatus(DBResCorriere) != DBRES_COMMAND_OK ) {
                trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Ordine [%s] Errore in insert dati corriere aggiuntivo",szOrdProg);
			} 
			DBFreeQueryResult(DBResCorriere);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}
	}
	DBFreeQueryResult(DBResOrdini);

	DBResCorriere=DBExecQuery(Cfg.nDebugLevel>1,"select cacdcli, caanspe, canrspe, caspedz, catprec, cabrcde from corr_agg_%s_%s order by caspedz,cabrcde;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);

	if((nTuples=DBntuples(DBResCorriere))){

        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"DatiCorrAgg() : (%d:Colli)",nTuples);

		for (nIndex=0;nIndex<nTuples;nIndex++){

			/*
			* spedizione dell'ordine a Host
			*/
			WriteAsciiRecord(fpExport, &tCorrAgg,DBResCorriere,nIndex,TRUE,FALSE);
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndex+1/(gdouble)nTuples);
		}
        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"Registrati %d colli",nTuples);
	}
	DBFreeQueryResult(DBResCorriere);

	if(fpExport) fclose(fpExport); 

	/* 
	* 23-03-2001 rm : copia di sicurezza 
	*/	
	sprintf(szCommand,"cp %s %s.old",szExportFile,szExportFile);
	system(szCommand);

	gtk_label_printf(priv->lb_1,"Procedura di spedizione dati corriere aggiuntivi terminata");
	gtk_label_printf(priv->lb_2,"Attendere. invio dati a host in corso");

	/*
	* creo il monitor
	*/
	if((fp=fopen(szMonitor,"w"))){
		fprintf(fp,"%s : %s %s", szMonitor, 
			GetDate(time((long *)0),szDateBuffer), 
			GetTime(time((long *)0),szTimeBuffer));

			fclose(fp);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Fallita creazione monitor di import dati");
	}

	gtk_label_printf(priv->lb_1,"Procedura di invio dati corriere aggiuntivi a Host terminata");
	gtk_label_printf(priv->lb_2,"Premere il tasto close per uscire");
    gtk_widget_set_sensitive(priv->pb_close, TRUE);
}


/*
* Spedizione dati a HOST
* 11-12-2017 rm : spedizione dati RCS
*/
void SpedizioneDati(gpointer win,gboolean overwrite, char *szExportFileName,char *szExportRigheFileName,ep_bool_t bCambiaStato, gboolean  storico)
{
    DBSTRUCT tEvasi;           /* evasi */
    DBSTRUCT tContenutoColli;
    char szDateBuffer[128];
    char szTimeBuffer[128];
    ep_bool_t bERRORE=FALSE;

    char szCommand[256];
    char szMonitor[128];
    char szOrdProg[128];
    char szAperturaFile[128];

    int nNumeroOrdini=0;
    int nNumeroRighe=0;
    int nNumeroColli=0;
    int nIndexOrdini;
    int nIndexRighe;
    int nTotaleRighe=0;
    int nTotaleColli=0;

    DBresult *DBRes;
    DBresult *DBResOrdini;
    DBresult *DBResRighe;
    DBresult *DBResUpdate;

    FILE *fpExport=NULL;
    FILE *fpExportRighe=NULL;

    char szTmpExportFileName[256];
    char szTmpExportRigheFileName[256];

    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

    ReadTableInfo(&tEvasi,          Cfg.szPathData,"evasi");
    ReadTableInfo(&tContenutoColli, Cfg.szPathData,"contenuto_colli");

    sprintf(szMonitor,"%s/%s",Cfg.szPathExport,g_strstrip(Cfg.szExportMonitor));

    sprintf(szTmpExportFileName,"%s.tmp",szExportFileName);
    sprintf(szTmpExportRigheFileName,"%s.tmp",szExportRigheFileName);

    strcpy(szAperturaFile, overwrite?"w":"a");
    if((fpExport=fopen(szTmpExportFileName,szAperturaFile))==NULL){
        trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Errore in apertura file dati [%s]",szExportFileName);
        return;
    }

    if((fpExportRighe=fopen(szTmpExportRigheFileName,szAperturaFile))==NULL){
        trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Errore in apertura file dati [%s]",szExportRigheFileName);
        return;
    }

    gtk_label_printf(priv->lb_1,"Spedizione Dati in corso");
    gtk_label_printf(priv->lb_2,"Attendere la fine della procedura");

    gtk_widget_set_sensitive(priv->pb_ok,        FALSE);
    gtk_widget_set_sensitive(priv->pb_append,    FALSE);
    gtk_widget_set_sensitive(priv->pb_overwrite, FALSE);
    gtk_widget_set_sensitive(priv->pb_close,     FALSE);


    /*
    * seleziono gli ordini selezionati
    */
    DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select r.ronmced,r.ordprog,r.ronmcll,c.cpprgcl from sel_ord_tmp_%s_%s s,ric_ord r,col_prod c where r.ordprog=s.ordprog and r.ordprog=c.ordprog and c.cpnmcol=1 order by r.ordprog;",Cfg.szTmpSuffix,Cfg.szTipoOrdini);

    if((nNumeroOrdini=DBntuples(DBResOrdini))){

        trace_debug_gtk("BLUE",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Lanciata Spedizione Dati (%d:Ordini) a Host",nNumeroOrdini);

        for (nIndexOrdini=0;nIndexOrdini<nNumeroOrdini;nIndexOrdini++){
            strcpy(szOrdProg,DBgetvalue(DBResOrdini,nIndexOrdini,1));
            nNumeroColli = atoi(DBgetvalue(DBResOrdini,nIndexOrdini,2));
            /*
            * spedizione dell'ordine a Host
            */
            /*
            * rm 07-02-2007 : controllo segnacollo > 0 
            */
            if(atoi(DBgetvalue(DBResOrdini,nIndexOrdini,3))==0){
                trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"ERRORE IN SPEDIZIONE ORDINE [%s] SEGNACOLLO NON PRESENTE !",DBgetvalue(DBResOrdini,nIndexOrdini,1));
                bERRORE=TRUE;
            }
            if(!bERRORE){
                WriteAsciiRecord(fpExport, &tEvasi,DBResOrdini,nIndexOrdini,TRUE,FALSE);
                
                if(bCambiaStato){
                    /*
                    * cambio lo stato dell'ordine ricevuto (E -> H)
                    */
                    DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c' where ordprog='%s';", ORDINE_SPEDITO_HOST, szOrdProg);DBFreeQueryResult(DBResUpdate);

                    /*
                    * cambio lo stato dei colli relativi all'ordine(E -> H)
                    */
                    DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c' where ordprog='%s';", COLLO_SPEDITO_HOST, szOrdProg); DBFreeQueryResult(DBResUpdate);

                    /*
                    * cambio lo stato delle righe relative all'ordine(E -> H)
                    */
                    DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpstato='%c' where ordprog='%s';", RIGA_SPEDITA_HOST, szOrdProg); DBFreeQueryResult(DBResUpdate);

                    UpdateOrdine(MAIN_APP_WINDOW(priv->main_app_window), szOrdProg);

                }
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_ordini),(gdouble)nIndexOrdini+1/(gdouble)priv->nValuesOrdini);
                nTotaleColli += nNumeroColli;
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_colli), (gdouble)nTotaleColli/(gdouble)priv->nValuesColli);
                /*
                * RIGHE - seleziono le righe dell'ordine 
                */
                DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"select r.RONMCED, r.ORDPROG, p.RPNMCOL, p.RPCDUBI, p.RPCDPRO, p.RPPRIOR, p.RPPRRIG, p.RPSWFFO, p.RPSWPCF, p.RPQTSPE, to_char(p.RPTMPRE,'YYYYMMDDHH24MISS'), p.RPCDOPR, to_char(c.CPTMEVA,'YYYYMMDDHH24MISS'), c.CPPSPRE, c.CPPSREA, c.CPTPFOR from sel_ord_tmp_%s_%s s,ric_ord r,col_prod c,rig_prod p where r.ordprog=s.ordprog and r.ordprog=c.ordprog and p.ordprog=r.ordprog and p.rpnmcol=c.cpnmcol and r.ordprog='%s' order by r.ordprog,c.cpnmcol,p.rpcdpro;",Cfg.szTmpSuffix,Cfg.szTipoOrdini,szOrdProg);
                if((nNumeroRighe=DBntuples(DBResRighe))){
                    for (nIndexRighe=0;nIndexRighe<nNumeroRighe;nIndexRighe++){
                        /*
                        * spedizione del contenuto ordine a Host (RIGHE)
                        */
                        WriteAsciiRecord(fpExportRighe, &tContenutoColli,DBResRighe,nIndexRighe,TRUE,FALSE);
                    }
                    nTotaleRighe+=nNumeroRighe;
                    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_righe),(gdouble)nTotaleRighe/(gdouble)priv->nValuesRighe);
                }
                DBFreeQueryResult(DBResRighe);
            }
        }

        if(!bERRORE){
            trace_debug_gtk("BLUE",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Spediti a Host %d ordini",nNumeroOrdini);
        } else {
            trace_debug_gtk("RED", TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"SPEDIZIONE ANNULLATA");
        }
    }

    DBFreeQueryResult(DBResOrdini);

    if(fpExport) fclose(fpExport); 
    if(fpExportRighe) fclose(fpExportRighe); 

    /* 
    * 07-02-2007 rm : copia di sicurezza 
    */    
    sprintf(szCommand,"cp %s %s.%s-%s", szTmpExportFileName, szExportFileName,
                    GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                    GetTimeHHMMSS(time((long *)0),szTimeBuffer));
    system(szCommand);

    /* 
    * 07-02-2007 rm : copia di sicurezza 
    */    
    sprintf(szCommand,"cp %s %s.%s-%s", szTmpExportRigheFileName, szExportRigheFileName,
                    GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                    GetTimeHHMMSS(time((long *)0),szTimeBuffer));
    system(szCommand);

    /* rinomina in file definitivo */
    sprintf(szCommand,"mv %s %s", szTmpExportFileName, szExportFileName);
    system(szCommand);

    /* rinomina in file definitivo */
    sprintf(szCommand,"mv %s %s", szTmpExportRigheFileName, szExportRigheFileName);
    system(szCommand);


    if(bERRORE){
        /* 
        * 07-02-2007 rm : copia di sicurezza 
        */    
        sprintf(szCommand,"mv %s %s.%s-%s-KO", szExportFileName, szExportFileName,
                        GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                        GetTimeHHMMSS(time((long *)0),szTimeBuffer));
        system(szCommand);

        sprintf(szCommand,"mv %s %s.%s-%s-KO", szExportRigheFileName, szExportRigheFileName,
                        GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                        GetTimeHHMMSS(time((long *)0),szTimeBuffer));
        system(szCommand);

        gtk_label_printf(priv->lb_1,"Si Sono verificati errori in fase di spedizione");
        gtk_label_printf(priv->lb_2,"La spedizione e' stata annullata");
        gtk_widget_set_sensitive(priv->pb_close,TRUE);
    } else {

        gtk_label_printf(priv->lb_1,"Procedura di spedizione dati terminata");
        gtk_widget_set_sensitive(priv->pb_close,TRUE);
    }
}

/*
* Spedizione dati a Aladino
*/
void SpedizioneDatiAladino(gpointer win, gboolean overwrite, char *szExportFileName)
{
    DBSTRUCT tFlussoCedola;
    char szDateBuffer[128];
    char szTimeBuffer[128];
    ep_bool_t bERRORE=FALSE;
    char szCommand[256];
    char szAperturaFile[256];
    int nLinee;
    int nLineaIndex;
    int nProgressivo;
    DBresult *DBRes;
    DBresult *DBResCedola;
    DBresult *DBResUpdateCedola;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

    FILE *fpExport=NULL;

    ReadTableInfo(&tFlussoCedola, Cfg.szPathData,"flusso_cedola");

    strcpy(szAperturaFile, overwrite?"w":"a");

    if((fpExport=fopen(szExportFileName,szAperturaFile))==NULL){
        trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Errore in apertura file dati [%s]",szExportFileName);
        return;
    }



    DBResCedola=DBExecQuery(Cfg.nDebugLevel,
        "insert into flusso_cedola \
        ( FCNMCED, FCTPSPE, FCCDLIN, FCDSLIN, FCNMCPE, FCNMCLL, FCNMORD, FCPSTOT, FCSTATO) \
        select \
        ronmced,'2',rocdlin,rodslin,sum(ronmcpe),sum(ronmcll),count(ordprog),sum(ropspre),' ' \
        from ric_ord ro \
        where \
        ordprog in (select ordprog from sel_ord_tmp_%s_%s)  \
        group by ronmced, rocdlin,rodslin;",Cfg.szTmpSuffix,Cfg.szTipoOrdini);
    DBFreeQueryResult(DBResCedola);

    DBResCedola=DBExecQuery(Cfg.nDebugLevel,
        "select FCPROGR, FCNMCED, FCTPSPE, FCCDLIN, FCDSLIN, FCNMCPE, FCNMCLL, FCNMORD, FCPSTOT, FCSTATO from flusso_cedola where fcstato=' ';");

    if((nLinee=DBntuples(DBResCedola))){
        for(nLineaIndex=0;nLineaIndex<nLinee;nLineaIndex++){

            nProgressivo=atoi(DBgetvalue(DBResCedola,nLineaIndex,0));

            WriteAsciiRecord(fpExport, &tFlussoCedola,DBResCedola,nLineaIndex,TRUE,FALSE);

            DBResUpdateCedola=DBExecQuery(Cfg.nDebugLevel,"update flusso_cedola set fcstato='S' where FCPROGR=%d;",nProgressivo);
            DBFreeQueryResult(DBResUpdateCedola);

            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(priv->pr_linee),(gfloat)nLineaIndex+1/(gfloat)nLinee);

        }
    }


    DBFreeQueryResult(DBResCedola);


    if(fpExport) fclose(fpExport); 

    /* 
    * 07-02-2007 rm : copia di sicurezza 
    */    
    sprintf(szCommand,"cp %s %s.%s-%s", szExportFileName, szExportFileName,
                    GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                    GetTimeHHMMSS(time((long *)0),szTimeBuffer));
    system(szCommand);

    if(bERRORE){
        /* 
        * 07-02-2007 rm : copia di sicurezza 
        */    
        sprintf(szCommand,"mv %s %s.%s-%s-KO", szExportFileName, szExportFileName,
                        GetDateYYYYMMDD(time((long *)0),szDateBuffer),
                        GetTimeHHMMSS(time((long *)0),szTimeBuffer));
        system(szCommand);
    }
}

void do_spedizione_dati (gpointer win, gboolean storico, gboolean overwrite)
{
    char szExportFile[256];
    char szExportRigheFile[256];
    char szExportFlussoCedola[256];
    MainAppSpedizioneDatiPrivate *priv;

    priv = main_app_spedizione_dati_get_instance_private (win);

    sprintf(szExportFile,"%s/%s",Cfg.szPathExport,g_strstrip(Cfg.szExportAnticipato));
    sprintf(szExportRigheFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportRighe));
    sprintf(szExportFlussoCedola,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportFlussoCedola));

    SpedizioneDati        (win, overwrite , szExportFile, szExportRigheFile, TRUE, storico);
    SpedizioneDatiAladino (win, overwrite , szExportFlussoCedola);
}

void do_spedizione_SDA (gpointer win, gboolean storico, gboolean overwrite)
{
    SpedizioneDatiSDA(win, storico, overwrite);
}

void do_spedizione_corriere (gpointer win, gboolean storico, gboolean overwrite)
{
    SpedizioneDatiCorriere(win, storico, overwrite);
}

void do_spedizione_corr_agg (gpointer win, gboolean storico, gboolean overwrite)
{
    SpedizioneDatiCorrAgg(win, storico, overwrite);
}

void init_spedizione_dati(MainAppSpedizioneDati *win, gboolean storico)
{
	char szExportFile[128];
	DBresult *DBResOrdini;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (win);

    gtk_window_set_title (GTK_WINDOW (win), "Spedizionadati a Host");

	sprintf(szExportFile,"%s/%s",Cfg.szPathStorico,StrTrimAll(Cfg.szExportSpedizioni));


	priv->nValuesOrdini=0;
	priv->nValuesColli=0;
	priv->nValuesRighe=0;

	/*
	* seleziono il numero di ORDINI e COLLI EVASI
	*/
	DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select count(ro.ordprog), sum(ro.ronmcll), sum(ro.ronmrgh) from %s ro,sel_ord_tmp_%s_%s so where ro.ordprog=so.ordprog;",
        storico ? "ric_ord_stor": "ric_ord", 
        Cfg.szTmpSuffix, 
        Cfg.szTipoOrdini);

	if(DBntuples(DBResOrdini)){
		if ((priv->nValuesOrdini=atoi(DBgetvalue(DBResOrdini,0,0)))!=0){
            gtk_label_printf(priv->lb_ordini,"%d Ordini", priv->nValuesOrdini);
		}

		if ((priv->nValuesColli=atoi(DBgetvalue(DBResOrdini,0,1)))!=0){
            gtk_label_printf(priv->lb_colli,"%d Colli", priv->nValuesColli);
		}

		if ((priv->nValuesRighe=atoi(DBgetvalue(DBResOrdini,0,2)))!=0){
            gtk_label_printf(priv->lb_righe,"%d Righe", priv->nValuesRighe);
		}
	}
	DBFreeQueryResult(DBResOrdini);

	if ((priv->nValuesOrdini==0)){
		/*
		* Non sono presenti dati da inviare a host
		*/

		gtk_label_printf(priv->lb_1,"Non sono presenti dati da trasmettere");
		gtk_widget_set_sensitive(priv->pb_ok,FALSE);
        gtk_widget_hide(priv->pb_append);
        gtk_widget_hide(priv->pb_overwrite);
	} else {

		if (FileExists(szExportFile)){
			gtk_label_printf(priv->lb_1,"Sono già presenti dati trasmessi");
			gtk_label_printf(priv->lb_1,"Sovrascrivere o Aggiungere ?");
            gtk_widget_hide(priv->pb_ok);
            gtk_widget_show(priv->pb_append);
            gtk_widget_show(priv->pb_overwrite);
		} else {
			gtk_label_printf(priv->lb_1,"Sono presenti dati da trasmettere");
			gtk_label_printf(priv->lb_2,"Premere Ok per iniziare la spedizione");
            gtk_widget_hide(priv->pb_append);
            gtk_widget_hide(priv->pb_overwrite);
		}
	}
}

void init_spedizione_SDA(MainAppSpedizioneDati *win, gboolean storico)
{
	char szExportFile[128];
	int nTuples=0;
	DBresult *DBRes;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

	sprintf(szExportFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorrSDA));

	/*
	* controllo gli ordini 
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select \
			ro.ordprog, ro.rodscli, ro.roincli, ro.rocpcli, ro.rolocli, ro.roprcli, ro.ronmcll, ro.ropsrea,\
			ro.ropspre, ro.ronmcpe, ro.rovlcoe, ro.ronmced, ro.rotpspe, ro.rocdlin, cp.cpbrcde \
			from %s ro,sel_ord_tmp_%s_%s so,%s cp where ro.ordprog=so.ordprog and ordtipo='%s' and cp.ordprog=so.ordprog order by ro.ordprog,cp.cpnmcol;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,Cfg.szTipoOrdini,
		storico?"col_prod_stor":"col_prod",
		Cfg.szTipoOrdini);


	if((nTuples=DBntuples(DBRes))){
        gtk_label_printf(priv->lb_ordini,"%d Ordini", nTuples);
    } else {
        gtk_label_printf(priv->lb_ordini,"Ordini non presenti");
    }
	DBFreeQueryResult(DBRes);
	if (nTuples){
		/*
		* Non sono presenti dati da inviare a host
		*/

		gtk_label_printf(priv->lb_1,"Non sono presenti dati da trasmettere");
		gtk_widget_set_sensitive(priv->pb_ok,FALSE);
        gtk_widget_hide(priv->pb_append);
        gtk_widget_hide(priv->pb_overwrite);

	} else {

		if (FileExists(szExportFile)){
			gtk_label_printf(priv->lb_1,"Sono già presenti dati trasmessi");
			gtk_label_printf(priv->lb_1,"Sovrascrivere o Aggiungere ?");
            gtk_widget_hide(priv->pb_ok);
            gtk_widget_show(priv->pb_append);
            gtk_widget_show(priv->pb_overwrite);
		} else {
			gtk_label_printf(priv->lb_1,"Sono presenti dati da trasmettere");
			gtk_label_printf(priv->lb_2,"Premere Ok per iniziare la spedizione");
            gtk_widget_hide(priv->pb_append);
            gtk_widget_hide(priv->pb_overwrite);
		}
	}
}

void init_spedizione_corriere(MainAppSpedizioneDati *win, gboolean storico)
{
	char szExportFile[128];
	int nTuples=0;
	DBresult *DBRes;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

	sprintf(szExportFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorrSDA));

	/*
	* controllo gli ordini 
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select \
			ro.ordprog, ro.rodscli, ro.roincli, ro.rocpcli, ro.rolocli, ro.roprcli, ro.ronmcll, ro.ropsrea,\
			ro.ropspre, ro.ronmcpe, ro.rovlcoe, ro.ronmced, ro.rotpspe, ro.rocdlin, cp.cpbrcde \
			from %s ro,sel_ord_tmp_%s_%s so,%s cp where ro.ordprog=so.ordprog and ordtipo='%s' and cp.ordprog=so.ordprog order by ro.ordprog,cp.cpnmcol;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,Cfg.szTipoOrdini,
		storico?"col_prod_stor":"col_prod",
		Cfg.szTipoOrdini);


	if((nTuples=DBntuples(DBRes))){
        gtk_label_printf(priv->lb_ordini,"%d Ordini", nTuples);
    } else {
        gtk_label_printf(priv->lb_ordini,"Ordini non presenti");
    }
	DBFreeQueryResult(DBRes);
	if (nTuples){
		/*
		* Non sono presenti dati da inviare a host
		*/

		gtk_label_printf(priv->lb_1,"Non sono presenti dati da trasmettere");
		gtk_widget_set_sensitive(priv->pb_ok,FALSE);
        gtk_widget_hide(priv->pb_append);
        gtk_widget_hide(priv->pb_overwrite);

	} else {

		if (FileExists(szExportFile)){
			gtk_label_printf(priv->lb_1,"Sono già presenti dati trasmessi");
			gtk_label_printf(priv->lb_1,"Sovrascrivere o Aggiungere ?");
            gtk_widget_hide(priv->pb_ok);
            gtk_widget_show(priv->pb_append);
            gtk_widget_show(priv->pb_overwrite);
		} else {
			gtk_label_printf(priv->lb_1,"Sono presenti dati da trasmettere");
			gtk_label_printf(priv->lb_2,"Premere Ok per iniziare la spedizione");
            gtk_widget_hide(priv->pb_append);
            gtk_widget_hide(priv->pb_overwrite);
		}
	}
}

void init_spedizione_corr_agg(MainAppSpedizioneDati *win, gboolean storico)
{
	char szExportFile[128];
	int nTuples=0;
	DBresult *DBRes;
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

	sprintf(szExportFile,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorrSDA));

	/*
	* controllo gli ordini 
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select \
			ro.ordprog, ro.rodscli, ro.roincli, ro.rocpcli, ro.rolocli, ro.roprcli, ro.ronmcll, ro.ropsrea,\
			ro.ropspre, ro.ronmcpe, ro.rovlcoe, ro.ronmced, ro.rotpspe, ro.rocdlin, cp.cpbrcde \
			from %s ro,sel_ord_tmp_%s_%s so,%s cp where ro.ordprog=so.ordprog and ordtipo='%s' and cp.ordprog=so.ordprog order by ro.ordprog,cp.cpnmcol;",
		storico?"ric_ord_stor":"ric_ord",
		Cfg.szTmpSuffix,Cfg.szTipoOrdini,
		storico?"col_prod_stor":"col_prod",
		Cfg.szTipoOrdini);


	if((nTuples=DBntuples(DBRes))){
        gtk_label_printf(priv->lb_ordini,"%d Ordini", nTuples);
    } else {
        gtk_label_printf(priv->lb_ordini,"Ordini non presenti");
    }
	DBFreeQueryResult(DBRes);
	if (nTuples){
		/*
		* Non sono presenti dati da inviare a host
		*/

		gtk_label_printf(priv->lb_1,"Non sono presenti dati da trasmettere");
		gtk_widget_set_sensitive(priv->pb_ok,FALSE);
        gtk_widget_hide(priv->pb_append);
        gtk_widget_hide(priv->pb_overwrite);

	} else {

		if (FileExists(szExportFile)){
			gtk_label_printf(priv->lb_1,"Sono già presenti dati trasmessi");
			gtk_label_printf(priv->lb_1,"Sovrascrivere o Aggiungere ?");
            gtk_widget_hide(priv->pb_ok);
            gtk_widget_show(priv->pb_append);
            gtk_widget_show(priv->pb_overwrite);
		} else {
			gtk_label_printf(priv->lb_1,"Sono presenti dati da trasmettere");
			gtk_label_printf(priv->lb_2,"Premere Ok per iniziare la spedizione");
            gtk_widget_hide(priv->pb_append);
            gtk_widget_hide(priv->pb_overwrite);
		}
	}
}


static void append_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

    switch(priv->type){
        case SPEDIZIONE_DATI:
            do_spedizione_dati (win, FALSE, FALSE);
        break;
        case SPEDIZIONE_SDA:
            do_spedizione_SDA (win, FALSE, FALSE);
        break;
        case SPEDIZIONE_CORRIERE:
            do_spedizione_corriere (win, FALSE, FALSE);
        break;
        case SPEDIZIONE_CORR_AGG:
            do_spedizione_corr_agg (win, FALSE, FALSE);
        break;
    }
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));

    switch(priv->type){
        case SPEDIZIONE_DATI:
            do_spedizione_dati (win, FALSE, TRUE);
        break;
        case SPEDIZIONE_SDA:
            do_spedizione_SDA (win, FALSE, TRUE);
        break;
        case SPEDIZIONE_CORRIERE:
            do_spedizione_corriere (win, FALSE, TRUE);
        break;
        case SPEDIZIONE_CORR_AGG:
            do_spedizione_corr_agg (win, FALSE, TRUE);
        break;
    }
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
  {"close",     close_activated,     NULL, NULL, NULL},
  {"append",    append_activated,    NULL, NULL, NULL},
  {"overwrite", ok_activated,        NULL, NULL, NULL},
  {"ok",        ok_activated,        NULL, NULL, NULL}
};

static void main_app_spedizione_dati_init (MainAppSpedizioneDati *win)
{
    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (win));
    gtk_widget_init_template (GTK_WIDGET (win));
    
    priv->type = _type;

    switch(priv->type){
        case SPEDIZIONE_DATI:
            init_spedizione_dati(win, FALSE);
        break;
        case SPEDIZIONE_SDA:
            init_spedizione_SDA(win, FALSE);
        break;
        case SPEDIZIONE_CORRIERE:
            init_spedizione_corriere(win, FALSE);
        break;
        case SPEDIZIONE_CORR_AGG:
            init_spedizione_corr_agg(win, FALSE);
        break;
    }
    
    init_actions(win, entries, G_N_ELEMENTS(entries), "spedizione_dati");
}

static void main_app_spedizione_dati_dispose (GObject *object)
{
    MainAppSpedizioneDatiPrivate *priv;

    priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (object));

    G_OBJECT_CLASS (main_app_spedizione_dati_parent_class)->dispose (object);
}

static void main_app_spedizione_dati_class_init (MainAppSpedizioneDatiClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_spedizione_dati_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/spedizione_dati.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pb_ok        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pb_close     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pb_append    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pb_overwrite );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_1         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_2         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_ordini    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_colli     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_righe     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, lb_linee     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pr_ordini    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pr_colli     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pr_righe     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSpedizioneDati, pr_linee     );
}

MainAppSpedizioneDati * main_app_spedizione_dati_new (gpointer win, SpedizioneType type)
{
    _type = type;
    MainAppSpedizioneDati *w = g_object_new (MAIN_APP_SPEDIZIONE_DATI_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppSpedizioneDatiPrivate *priv = main_app_spedizione_dati_get_instance_private (MAIN_APP_SPEDIZIONE_DATI (w));

    priv->main_app_window = win;

    return w;
}
