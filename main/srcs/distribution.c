#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <picking.h>

#include <gtk-support.h>
#include <msg-box.h>
#include <dbfun-gtk.h>
#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "distribution.h"
#include "spedizionedati.h"
#include "mainfun.h"

#define WINDOW_NAME "distribution-window"

struct _DistrWindow
{
    GtkDialog parent;
};

typedef struct _DistrWindowPrivate DistrWindowPrivate;

struct _DistrWindowPrivate
{
    MainWindow *main_window;

    gchar *pszCfgFileName;

    GtkWidget *sw;
    GtkWidget *pb_lancia;
    GtkWidget *pb_spedizione_dati;
    GtkWidget *pb_corriere;
    GtkWidget *pb_corr_agg;
    GtkWidget *pb_stampa_rac;
    GtkWidget *pb_stampa_xab;
    GtkWidget *pb_lista_articoli;
    GtkWidget *pb_stampa_dist;
    GtkWidget *pb_stampa_dc;
    GtkWidget *pb_stampa;
    GtkWidget *pb_sda;
    GtkWidget *pb_aggiorna;
};

G_DEFINE_TYPE_WITH_PRIVATE(DistrWindow, distr, GTK_TYPE_DIALOG)


void on_lst_distr_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    DistrWindowPrivate *priv = distr_get_instance_private (win);
    gboolean ok = FALSE;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)) {
        ok = TRUE;
    }
	gtk_widget_set_sensitive(priv->pb_lancia,          ok);
	gtk_widget_set_sensitive(priv->pb_spedizione_dati, ok);
	gtk_widget_set_sensitive(priv->pb_corriere,        ok);
	gtk_widget_set_sensitive(priv->pb_corr_agg,        ok);
	gtk_widget_set_sensitive(priv->pb_stampa_rac,      ok);
	gtk_widget_set_sensitive(priv->pb_stampa_xab,      ok);
	gtk_widget_set_sensitive(priv->pb_lista_articoli,  ok);
	gtk_widget_set_sensitive(priv->pb_stampa_dist,     ok);
	gtk_widget_set_sensitive(priv->pb_stampa_dc,       ok);
	gtk_widget_set_sensitive(priv->pb_stampa,          ok);
	gtk_widget_set_sensitive(priv->pb_sda,             ok);
	gtk_widget_set_sensitive(priv->pb_aggiorna,        ok);
}


static ep_bool_t StampaRACLinea(char *szNMCED,char *szTPSPE,char *szCDLIN, char *szCDVET)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s,%s,%s,%s", szNMCED,szTPSPE,szCDLIN,szCDVET,Cfg.szLabelPrinterConsole);
	return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC_LINEA,szBuffer);
}

static ep_bool_t StampaXABLinea(char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s,%s,%s,%s", szNMCED,szTPSPE,szCDLIN,szCDVET,Cfg.szXABPrinterConsole);
	if(bStorico){
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB_LINEA_STORICO,szBuffer);
	} else {
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB_LINEA,szBuffer);
	}
}

static ep_bool_t StampaDCLinea(char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,ep_bool_t bStorico)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s,%s,%s,%s", szNMCED,szTPSPE,szCDLIN,szCDVET,Cfg.szXABPrinterConsole);
	if(bStorico){
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DC_LINEA_STORICO,szBuffer);
	} else {
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DC_LINEA,szBuffer);
	}
}

static ep_bool_t StampaDistinta(char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVET,char *szAnno,ep_bool_t bStorico)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s,%s,%s,%s,%s", szNMCED,szTPSPE,szCDLIN,szCDVET,szAnno,Cfg.szDISTPrinterConsole);

	if(bStorico){
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DISTINTA_STORICO,szBuffer);
	} else {
		return SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DISTINTA,szBuffer);
	}
}


static void do_lista_articoli (gpointer win, gchar *cfg_file_name, gpointer title)
{
	DBresult *DBRes;
    char szSelectCmd[4096];
    DistrWindowPrivate *priv = distr_get_instance_private (win);

	/*
	* Tabella sel_art_tmp_[TIPO_ORDINI] : 
	* sacdpro
	* sanmrgh
	* sanmcpe
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_art_tmp_%s_%s as select r.racdpro as sacdpro,count(r.racdpro) as sanmrgh,sum(r.raqtord) as sanmcpe from ric_art r,sel_ord_tmp_%s_%s s where r.ordprog=s.ordprog group by r.racdpro;",
		Cfg.szTmpSuffix, Cfg.szTipoOrdini, Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBclear(DBRes);

    GetFileString("distribuzione","lista_articoli", "", szSelectCmd, sizeof(szSelectCmd), cfg_file_name,NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                    "Lista Articoli", 
                    GTK_MESSAGE_INFO, 
                    GTK_BUTTONS_CLOSE, 
                    szSelectCmd, 
                    GTK_SELECTION_SINGLE, 
                    NULL, 
                    title?title:"");
}

static void do_file_SDA_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
    SpedizioneDatiWindow *dlg;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

    dlg = spedizione_dati_new (MAIN_WINDOW (priv->main_window), SPEDIZIONE_SDA);
    gtk_window_present (GTK_WINDOW (dlg));
}

static void do_file_corr_agg_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
    SpedizioneDatiWindow *dlg;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

    dlg = spedizione_dati_new (MAIN_WINDOW (priv->main_window), SPEDIZIONE_CORR_AGG);
    gtk_window_present (GTK_WINDOW (dlg));
}

static void do_file_corriere_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
    SpedizioneDatiWindow *dlg;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

    dlg = spedizione_dati_new (MAIN_WINDOW (priv->main_window), SPEDIZIONE_CORRIERE);
    gtk_window_present (GTK_WINDOW (dlg));
}

static void do_stampa_dc_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
	char szNMCED[64];
	char szTPSPE[64];
	char szCDLIN[64];
	char szCDVET[64];
	int nLinee=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

	DBRes=DBExecQuery(FALSE,"select r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2,count(s.ordprog) from sel_ord_tmp_%s_%s s,ric_ord r where s.ordprog=r.ordprog group by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2 order by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nLinee=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0));
		strcpy(szTPSPE,DBgetvalue(DBRes,nIndex,1));
		strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,2));
		strcpy(szCDVET,DBgetvalue(DBRes,nIndex,3));
		if(!StampaDCLinea(szNMCED,szTPSPE,szCDLIN,szCDVET,FALSE)){
            trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore in stampa DC linea [%s,%s,%s,%s]",szNMCED,szTPSPE,szCDLIN,szCDVET);
		} else {
            trace_debug_gtk("GREEN",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Linea [%s-%s:%s:%s] - Stampati %d Ordini",szNMCED,szTPSPE,szCDLIN,szCDVET,atoi(DBgetvalue(DBRes,nIndex,4)));
			nLinee++;
		}
	}
	DBclear(DBRes);
    trace_debug_gtk("GREEN",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Stampate %d Linee",nLinee);
}

/*
* Stampa Distinta - lavora su sel_ord_tmp_[SUFFIX]_[ORDTIPO]
*/
static void do_stampa_distinta_distribuzioni(gpointer win, gchar *cfg_file_name, gpointer data)
{
	char szNMCED[64];
	char szTPSPE[128];
	char szCDLIN[128];
	char szCDVET[64];
	char szAnno[128];
	int nLinee=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

	/* seleziono gli ordini da stampare raggruppati per tipo spedizione e linea */
	DBRes=DBExecQuery(FALSE,"select r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2,anno(r.rotmrcz) from sel_ord_tmp_%s_%s s,ric_ord r where s.ordprog=r.ordprog group by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2,anno(r.rotmrcz) order by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nLinee=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0));
		strcpy(szTPSPE,DBgetvalue(DBRes,nIndex,1));
		strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,2));
		strcpy(szCDVET,DBgetvalue(DBRes,nIndex,3));
		strcpy(szAnno,DBgetvalue(DBRes,nIndex,4));
		if(!StampaDistinta(szNMCED,szTPSPE,szCDLIN,szCDVET,szAnno,FALSE)){
            trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore in stampa distinta Linea [%s-%s %s-%s-%s]\n",szNMCED,szTPSPE,szCDLIN,szCDVET,szAnno);
		} else {
			nLinee++;
		}
	}
	DBclear(DBRes);
    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Stampate %d linee",nLinee);
}

static void do_spedizioni_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
    g_printf("do_spedizioni_distribuzioni,\n");
}

static void do_lancio_distribuzioni (gpointer win, gchar *cfg_file_name, gpointer data)
{
    DistrWindowPrivate *priv = distr_get_instance_private (win);
	/*
	* Lancio gli ordini delle distribuzioni selezionate
	*/
	char szOrdine[64];
	int nOrdini=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;


	DBRes=DBExecQuery(FALSE,"select s.ordprog from sel_ord_tmp_%s_%s s,ric_ord r where s.ordprog=r.ordprog order by r.rotpspe,r.rocdlin,r.rocdve2,s.ordprog;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nOrdini=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szOrdine,DBgetvalue(DBRes,nIndex,0));
		if(!LanciaOrdine(win, szOrdine,FALSE)){
            trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore in lancio ordine [%s]\n",szOrdine);
		} else {
			nOrdini++;
		}
	}
	DBclear(DBRes);

    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Lanciati %d ordini\n",nOrdini);

	SendMessage(PROC_VOLUM, PROC_MAIN, START_ANALISI, NULL);
}

/*
* Stampa RAC - lavora su sel_ord_tmp_[SUFFIX]_[ORDTIPO]
*/
static void do_stampa_rac_distribuzioni(gpointer win, gchar *cfg_file_name, gpointer data)
{
	char szNMCED[64];
	char szTPSPE[64];
	char szCDLIN[64];
	char szCDVET[64];
	int nLinee=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;
    DistrWindowPrivate *priv = distr_get_instance_private (win);

	DBRes=DBExecQuery(1,"select r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2,count(s.ordprog) from sel_ord_tmp_%s_%s s,ric_ord r where s.ordprog=r.ordprog group by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2 order by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nLinee=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0));
		strcpy(szTPSPE,DBgetvalue(DBRes,nIndex,1));
		strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,2));
		strcpy(szCDVET,DBgetvalue(DBRes,nIndex,3));
		if(!StampaRACLinea(szNMCED,szTPSPE,szCDLIN,szCDVET)){
            trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore in stampa RAC linea [%s,%s,%s,%s]\n",szNMCED,szTPSPE,szCDLIN,szCDVET);
		} else {
            trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Linea [%s-%s:%s:%s] - Stampati %d Ordini\n",szNMCED,szTPSPE,szCDLIN,szCDVET,atoi(DBgetvalue(DBRes,nIndex,4)));
			nLinee++;
		}
	}
	DBclear(DBRes);
    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Stampate %d Linee\n",nLinee);
}

/*
* Stampa XAB - lavora su sel_ord_tmp_[SUFFIX]_[ORDTIPO]
*/
static void do_stampa_xab_distribuzioni(gpointer win, gchar *cfg_file_name, gpointer data)
{
	char szNMCED[64];
	char szTPSPE[64];
	char szCDLIN[64];
	char szCDVET[64];
	int nLinee=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;
    DistrWindowPrivate *priv = distr_get_instance_private (win);


	DBRes=DBExecQuery(FALSE,"select r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2,count(s.ordprog) from sel_ord_tmp_%s_%s s,ric_ord r where s.ordprog=r.ordprog group by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2 order by r.ronmced,r.rotpspe,r.rocdlin,r.rocdve2;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nLinee=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szNMCED,DBgetvalue(DBRes,nIndex,0));
		strcpy(szTPSPE,DBgetvalue(DBRes,nIndex,1));
		strcpy(szCDLIN,DBgetvalue(DBRes,nIndex,2));
		strcpy(szCDVET,DBgetvalue(DBRes,nIndex,3));
		if(!StampaXABLinea(szNMCED,szTPSPE,szCDLIN,szCDVET,FALSE)){
            trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore in stampa XAB linea [%s,%s,%s,%s]\n",szNMCED,szTPSPE,szCDLIN,szCDVET);
		} else {
            trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Linea [%s-%s:%s:%s] - Stampati %d Ordini\n",szNMCED,szTPSPE,szCDLIN,szCDVET,atoi(DBgetvalue(DBRes,nIndex,4)));
			nLinee++;
		}
	}
	DBclear(DBRes);
    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Stampate %d Linee\n",nLinee);
}

typedef void (*distr_callback)(gpointer win, gchar *, gpointer);

static void on_selected_distribuzioni_do (gpointer win, gchar *title, gchar *confirmation_msg, distr_callback fn, gpointer cb_data, char *lst_stati_ordine)
{
    GList *lista;
    GtkTreeIter iter;
    char *pszOrdine;
    char szOrdine[256];
    char szSelectCmd[4096];
    DistrWindowPrivate *priv = distr_get_instance_private (win);
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
		char *pszString;
		char szCDLIN[128];
		char szNMCED[128];
		int nRowIndex=-1;
		DBresult *DBRes = NULL;
        ep_bool_t bOK=TRUE;
        int nTuples;

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where false;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);

        do {
            if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
                /* ho ottenuto l'iter */
                gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,0, &pszString); strcpy(szNMCED,pszString); StrTrimAll(szNMCED); g_free(pszString);
                gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,1, &pszString); strcpy(szCDLIN,pszString); StrTrimAll(szCDLIN); g_free(pszString);
                trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"linea -> [%s-%s]", szNMCED, szCDLIN);
                char szCondition[128];

                if(lst_stati_ordine)
                    sprintf(szCondition, " rostato in (%s) and ", lst_stati_ordine);
                else
                    szCondition[0]='\0';

                DBRes=DBExecQuery(Cfg.nDebugLevel>1,
                    "insert into sel_ord_tmp_%s_%s select ordprog from ric_ord where %s ronmced='%s' and trim(rocdlin) = '%s' and ordtipo='%s';",
                    Cfg.szTmpSuffix,
                    Cfg.szTipoOrdini,
                    szCondition,
                    szNMCED,
                    szCDLIN,
                    Cfg.szTipoOrdini);
                if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
                    trace_debug_gtk("RED",TRUE,1,find_child(GTK_WIDGET(priv->main_window),"txt_msgs"),"Errore nella selezione ordini (%s,%s)",szNMCED,szCDLIN);
                    bOK=FALSE;
                }
                DBclear(DBRes);

            }
        } while((lista=g_list_next(lista)));

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select * from sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
		nTuples=DBntuples(DBRes);
		DBclear(DBRes);

		if (nTuples && bOK){
            GetFileString("distribuzione","lista_ordini", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 

            GList *PS=NULL;
            add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
            int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                            title?title:"", 
                            GTK_MESSAGE_QUESTION, 
                            GTK_BUTTONS_OK_CANCEL, 
                            szSelectCmd, 
                            GTK_SELECTION_SINGLE, 
                            PS, 
                            confirmation_msg?confirmation_msg:"");
            switch (rc) {
                case GTK_RESPONSE_OK:
                    if(fn)
                        fn(win, priv->pszCfgFileName, cb_data);
                break;
            }
		} else {
            dlg_msg( GTK_WINDOW(win), title?title:"", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE, "Nessun ordine selezionato !");
		}
    } else {
        dlg_msg( GTK_WINDOW(win), title?title:"", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE, "Nessuna distribuzione selezionata !");
    }
    
    g_list_free(lista);
}

static void do_stampa_distribuzione (gpointer win)
{
    DistrWindowPrivate *priv = distr_get_instance_private (win);
    int rc = dlg_msg( GTK_WINDOW(win), "Stampa Distribuzione", GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL, "Stampa la lista ditribuzioni ?");
    switch (rc) {
        case GTK_RESPONSE_OK:
            PrintTable(priv->pszCfgFileName, "distribuzione","distribuzione_stampa",Cfg.szPrinterConsole,Cfg.szPathStampe, NULL, NULL, Cfg.nTipoStampe);
        break;
    }
}



/**************************************************************************************
*   FINE DLG_DISTRIBUZIONE
****************************************************************************************/

/*
* do_lista_distribuzione(DistrWindow *win)
*/
static void do_lista_distribuzione(DistrWindow *win)
{
    DBresult *DBRes;
    DBresult *DBResOrdini;
    DBresult *DBResInsert;
    DBresult *DBResUpdate;
    char szNMCED[80],szCDLIN[80],szTPSPE[80],szDSLIN[80],szCDVE2[80],szStato[80];
    int nOrdiniSpeditiHOST = 0,nColliSpeditiHOST = 0;
    int nOrdiniStampatiDIST = 0, nColliStampatiDIST = 0;
    int nOrdiniStampatiXAB = 0, nColliStampatiXAB = 0;
    int nOrdiniEvasi = 0, nColliEvasi = 0;
    int nOrdiniInEvasione = 0, nColliInEvasione = 0;
    int nOrdiniTotali = 0, nColliTotali = 0, nRigheTotali = 0, nCopieTotali = 0,nPesoTotale = 0;
    int nTuples;
    int nIndex;

    DistrWindowPrivate *priv = distr_get_instance_private (win);

    /*
    * LG nov 2011: aggiungo scidvet per creare una lista in cui la discriminante non
    * sia solo tpspe e tplin ma anche il vettore.
    */
    /*
    * Tabella sel_dist_tmp_[TIPO_ORDINI]
    *
    sdnmced : Numero Cedola
    sdtpspe : Tipo Spedizione
    sdcdlin : Codice Linea
    sddslin : Descrizione Linea (vedi ricerca)
    sdidvet : ID vettore (vedi tabella ttlv)
    sdpspre : Peso totale ordini
    sdnmrgh : Righe totali ricevute
    sdnmcpe : Copie totali ricevute
    sdnmort : Numero Ordini nella linea
    sdnmclt : Numero Colli
    sdnmore : Ordini Evasi
    sdnmcle : Colli Evasi
    sdnmors : Ordini Spediti a Host
    sdnmcls : Colli Spediti a host
    sdnmorx : Ordini stampati xab
    sdnmclx : Colli stampata xab
    sdnmord : Ordini stampati dist
    sdnmcld : Colli stampata dist
    */

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_dist_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
    DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_dist_tmp_%s_%s (\
        sdnmced text,\
        sdtpspe text,\
        sdcdlin text,\
        sdcdve2 text,\
        sddslin text,\
        sddsvet text,\
        sdpspre int4 default 0,\
        sdnmrgh int4 default 0,\
        sdnmcpe int4 default 0,\
        sdnmort int4 default 0,\
        sdnmclt int4 default 0,\
        sdnmori int4 default 0,\
        sdnmcli int4 default 0,\
        sdnmore int4 default 0,\
        sdnmcle int4 default 0,\
        sdnmors int4 default 0,\
        sdnmcls int4 default 0,\
        sdnmorx int4 default 0,\
        sdnmclx int4 default 0,\
        sdnmord int4 default 0,\
        sdnmcld int4 default 0\
    );", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
    DBclear(DBRes);
     /*
    * raggruppo gli ordini con ronmced,rotpspe,rocdlin,rocdve2 (new) e li inserisco nella tabella
    */
    DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_dist_tmp_%s_%s (sdnmced,sdtpspe,sdcdlin,sddslin,sdcdve2,sdnmort,sdnmclt,sdpspre,sdnmrgh,sdnmcpe) select ronmced,rotpspe,rocdlin,rodslin,rocdve2,count(ordprog),sum(ronmcll),sum(ropspre),sum(ronmrgh),sum(ronmcpe) from ric_ord where ordtipo='%s' group by ronmced,rotpspe,rocdlin,rodslin, rocdve2 order by rotpspe,rocdlin,rocdve2;", Cfg.szTmpSuffix, Cfg.szTipoOrdini, Cfg.szTipoOrdini); 
    DBclear(DBResInsert);
    /*
    * calcolo i totali per distribuzione
    */
    DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"select rostato,ronmced,rotpspe,rocdlin,rocdve2,count(ordprog),sum(ronmcll),sum(ronmrgh),sum(ronmcpe),sum(ropspre),rodslin from ric_ord where ordtipo='%s' group by ronmced,rostato,rotpspe,rocdlin,rocdve2,rodslin order by ronmced,rotpspe,rocdlin,rocdve2;", Cfg.szTipoOrdini);
    nTuples=DBntuples(DBResOrdini);
    strcpy(szNMCED,"");
    strcpy(szTPSPE,"");
    strcpy(szCDLIN,"");
    strcpy(szDSLIN,"");
    strcpy(szCDVE2,"");
    for(nIndex=0;nIndex<nTuples;nIndex++){
        strcpy(szStato,DBgetvalue(DBResOrdini,nIndex,0));
        if(strcmp(szNMCED,DBgetvalue(DBResOrdini,nIndex,1)) || strcmp(szCDLIN,DBgetvalue(DBResOrdini,nIndex,3))){
            strcpy(szNMCED,DBgetvalue(DBResOrdini,nIndex,1));
            strcpy(szTPSPE,DBgetvalue(DBResOrdini,nIndex,2));
            strcpy(szCDLIN,DBgetvalue(DBResOrdini,nIndex,3));
            strcpy(szCDVE2,DBgetvalue(DBResOrdini,nIndex,4));
            strcpy(szDSLIN,DBgetvalue(DBResOrdini,nIndex,10));

            nOrdiniSpeditiHOST = 0;nColliSpeditiHOST = 0;
            nOrdiniStampatiDIST = 0; nColliStampatiDIST = 0;
            nOrdiniStampatiXAB = 0; nColliStampatiXAB = 0;
            nOrdiniEvasi = 0; nColliEvasi = 0;
            nOrdiniInEvasione = 0; nColliInEvasione = 0;
            nOrdiniTotali = 0; nColliTotali = 0; nRigheTotali = 0; nCopieTotali = 0;nPesoTotale = 0;
        }
        switch(szStato[0]){
            case ORDINE_SPEDITO_HOST:
                nOrdiniSpeditiHOST+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliSpeditiHOST+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_STAMPATA_DIST:
                nOrdiniStampatiDIST+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliStampatiDIST+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_STAMPATA_XAB:
                nOrdiniStampatiXAB+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliStampatiXAB+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_EVASO:
                nOrdiniEvasi+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliEvasi+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_ELABORATO:
            case ORDINE_STAMPATA_RAC:
            case ORDINE_FILE_INVIATO:
            case ORDINE_IN_PRELIEVO:
            case ORDINE_PRELEVATO:
                nOrdiniInEvasione+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliInEvasione+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            default:
                nOrdiniTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            break;
        }
        nRigheTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,7));
        nCopieTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,8));
        nPesoTotale+=atoi(DBgetvalue(DBResOrdini,nIndex,9));
        /*
        * Update dei totali per distribuzione
        */
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select sdnmced,sdtpspe,sdcdlin,sdcdve2 from sel_dist_tmp_%s_%s where sdnmced='%s' and sdcdlin='%s';",
             Cfg.szTmpSuffix, Cfg.szTipoOrdini, szNMCED, szCDLIN);
        if(DBresultStatus(DBRes) == DBRES_TUPLES_OK){
            if(DBntuples(DBRes)){
                /* esiste gia' */
                DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update sel_dist_tmp_%s_%s set \
                    sdnmors=%d,sdnmcls=%d,\
                    sdnmord=%d,sdnmcld=%d,\
                    sdnmorx=%d,sdnmclx=%d,\
                    sdnmore=%d,sdnmcle=%d,\
                    sdnmori=%d,sdnmcli=%d where sdnmced='%s' and sdcdlin='%s';" , Cfg.szTmpSuffix, Cfg.szTipoOrdini,
                nOrdiniSpeditiHOST, nColliSpeditiHOST,
                nOrdiniStampatiDIST, nColliStampatiDIST,
                nOrdiniStampatiXAB, nColliStampatiXAB,
                nOrdiniEvasi, nColliEvasi,
                nOrdiniInEvasione, nColliInEvasione,
                szNMCED, szCDLIN);

                DBclear(DBResUpdate);
            } else {
                /* non esiste -- impossibile */
                DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_dist_tmp_%s_%s (sdnmced,sdtpspe,sdcdlin,sdcdve2, sddslin, sdnmrgh,sdnmcpe,sdpspre,sdnmort,sdnmclt, sdnmors,sdnmcls, sdnmord,sdnmcld, sdnmorx,sdnmclx, sdnmore,sdnmcle, sdnmori,sdnmcli) values ( '%s','%s','%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d );", Cfg.szTmpSuffix, Cfg.szTipoOrdini,
                    szNMCED,szTPSPE,szCDLIN, szCDVE2,szDSLIN,
                    nRigheTotali, nCopieTotali, nPesoTotale,
                    nOrdiniTotali, nColliTotali,
                    nOrdiniSpeditiHOST, nColliSpeditiHOST,
                    nOrdiniStampatiDIST, nColliStampatiDIST,
                    nOrdiniStampatiXAB, nColliStampatiXAB,
                    nOrdiniEvasi, nColliEvasi,
                    nOrdiniInEvasione, nColliInEvasione);

                DBclear(DBResInsert);
            }
            DBclear(DBRes);
        }
    }
    /*
    * calcolo totali 
    */
    nOrdiniSpeditiHOST = 0;nColliSpeditiHOST = 0;
    nOrdiniStampatiDIST = 0; nColliStampatiDIST = 0;
    nOrdiniStampatiXAB = 0; nColliStampatiXAB = 0;
    nOrdiniEvasi = 0; nColliEvasi = 0;
    nOrdiniInEvasione = 0; nColliInEvasione = 0;
    nOrdiniTotali = 0; nColliTotali = 0; nRigheTotali = 0; nCopieTotali = 0;nPesoTotale = 0;
    for(nIndex=0;nIndex<nTuples;nIndex++){
        strcpy(szStato,DBgetvalue(DBResOrdini,nIndex,0));
        switch(szStato[0]){
            case ORDINE_SPEDITO_HOST:
                nOrdiniSpeditiHOST+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliSpeditiHOST+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_STAMPATA_DIST:
                nOrdiniStampatiDIST+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliStampatiDIST+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_STAMPATA_XAB:
                nOrdiniStampatiXAB+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliStampatiXAB+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_EVASO:
                nOrdiniEvasi+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliEvasi+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            case ORDINE_ELABORATO:
            case ORDINE_STAMPATA_RAC:
            case ORDINE_FILE_INVIATO:
            case ORDINE_IN_PRELIEVO:
            case ORDINE_PRELEVATO:
                nOrdiniInEvasione+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliInEvasione+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            default:
                nOrdiniTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,5));
                nColliTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,6));
            break;
        }
        nRigheTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,7));
        nCopieTotali+=atoi(DBgetvalue(DBResOrdini,nIndex,8));
        nPesoTotale+=atoi(DBgetvalue(DBResOrdini,nIndex,9));
    }
    DBclear(DBResOrdini);
    /*
    * inserimento totali 
    */
    DBResInsert=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_dist_tmp_%s_%s (sdnmced,sdtpspe,sdcdlin,sdcdve2,sddslin,sddsvet,sdnmrgh,sdnmcpe,sdpspre,sdnmort,sdnmclt, sdnmors,sdnmcls, sdnmord,sdnmcld, sdnmorx,sdnmclx, sdnmore,sdnmcle, sdnmori,sdnmcli) values ( '%s','%s','%s','%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d );",
        Cfg.szTmpSuffix, Cfg.szTipoOrdini,
        "T","T","T","T","T","TOTALI",
        nRigheTotali, nCopieTotali, nPesoTotale,
        nOrdiniTotali, nColliTotali,
        nOrdiniSpeditiHOST, nColliSpeditiHOST,
        nOrdiniStampatiDIST, nColliStampatiDIST,
        nOrdiniStampatiXAB, nColliStampatiXAB,
        nOrdiniEvasi, nColliEvasi,
        nOrdiniInEvasione, nColliInEvasione);

    DBclear(DBResInsert);

}


static void lancia_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c'", ORDINE_SPEDITO, ORDINE_RICEVUTO);
    on_selected_distribuzioni_do (win, "Lancio Distribuzioni", "Confermi il lancio delle distribuzioni selezionate ?", do_lancio_distribuzioni, NULL, szStati);
}

static void print_rac_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c'", ORDINE_ELABORATO, ORDINE_IN_PRELIEVO);
    on_selected_distribuzioni_do (win, "Stampa RAC", "Confermi la stampa RAC delle distribuzioni selezionate ?", do_stampa_rac_distribuzioni, NULL, szStati);
}

static void print_xab_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c','%c','%c'", ORDINE_EVASO, ORDINE_STAMPATA_XAB, ORDINE_STAMPATA_DISTINTA, ORDINE_SPEDITO_HOST);

    on_selected_distribuzioni_do (win, "Stampa XAB", "Confermi la stampa XAB  delle distribuzioni selezionate ?", do_stampa_xab_distribuzioni, NULL, szStati);
}

static void send_host_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c'", ORDINE_EVASO);
    on_selected_distribuzioni_do (win, "Spedizione Distribuzioni", "Confermi la spedizione a host delle distribuzioni selezionate ?", do_spedizioni_distribuzioni, NULL, szStati);
}


static void print_dc_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c','%c','%c','%c'", ORDINE_EVASO, ORDINE_STAMPATA_XAB, ORDINE_STAMPATA_DISTINTA, ORDINE_FILE_INVIATO, ORDINE_SPEDITO_HOST);
    on_selected_distribuzioni_do (win, "Documento Corriere", "Confermi la stampa del documento corriere delle distribuzioni selezionate ?", do_stampa_dc_distribuzioni, NULL, szStati);
}

static void print_dist_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c'", ORDINE_STAMPATA_DIST, ORDINE_FILE_INVIATO);
    on_selected_distribuzioni_do (win, "Stampa Distinta", "Confermi la stampa della distinta delle distribuzioni selezionate ?", do_stampa_distinta_distribuzioni, NULL, szStati);
}

static void corriere_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c'", EMAIL_VETTORE_NON_SPEDITA, ORDINE_STAMPATA_DISTINTA);
    on_selected_distribuzioni_do (win, "Dati Corriere", "Confermi la spedizione dei dati al corriere delle distribuzioni selezionate ?", do_file_corriere_distribuzioni, NULL, szStati);
}

static void corr_agg_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c','%c','%c'", ORDINE_EVASO, ORDINE_STAMPATA_XAB, ORDINE_STAMPATA_DISTINTA, ORDINE_FILE_INVIATO);
    on_selected_distribuzioni_do (win, "Dati Corriere Aggiuntivi", "Confermi la spedizione dei dati al aggiuntivi al corriere delle distribuzioni selezionate ?", do_file_corr_agg_distribuzioni, NULL, szStati);
}

static void sda_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szStati[256];

    sprintf(szStati,"'%c','%c','%c','%c'", ORDINE_EVASO, ORDINE_STAMPATA_XAB, ORDINE_STAMPATA_DISTINTA, ORDINE_SPEDITO_HOST);
    on_selected_distribuzioni_do (win, "Spedizione dati SDA", "Confermi la spedizione dei dati al corriere SDA delle distribuzioni selezionate ?", do_file_SDA_distribuzioni, NULL, szStati);
}

static void lista_articoli_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    on_selected_distribuzioni_do (win, "Lista Articoli", "Premere OK per visualizzare la lista degli articoli contenuti nelle distribuzioni selezionate", do_lista_articoli, "Lista Articoli relativi alle distribuzioni selezionate" , NULL);
}

static void print_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_distribuzione(win);
}

static void refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_lista_distribuzione(win);
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {

    {"lancia",          lancia_activated,         NULL, NULL, NULL} ,
    {"send_host",       send_host_activated,      NULL, NULL, NULL} ,
    {"corriere",        corriere_activated,       NULL, NULL, NULL} ,
    {"corr_agg",        corr_agg_activated,       NULL, NULL, NULL} ,
    {"print_rac",       print_rac_activated,      NULL, NULL, NULL} ,
    {"print_xab",       print_xab_activated,      NULL, NULL, NULL} ,
    {"lista_articoli",  lista_articoli_activated, NULL, NULL, NULL} ,
    {"print_dist",      print_dist_activated,     NULL, NULL, NULL} ,
    {"print_dc",        print_dc_activated,       NULL, NULL, NULL} ,
    {"print",           print_activated,          NULL, NULL, NULL} ,
    {"sda",             sda_activated,            NULL, NULL, NULL} ,
    {"refresh",         refresh_activated,        NULL, NULL, NULL} ,
    {"close",           close_activated,          NULL, NULL, NULL}

};

static void distr_init (DistrWindow *win)
{
    char szSelectCmd[4096];
    DistrWindowPrivate *priv; 

    gtk_widget_init_template (GTK_WIDGET (win));

    priv = distr_get_instance_private (win);
    
    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    do_lista_distribuzione(win);

    GetFileString("distribuzione","distribuzione", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 

    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    RefreshTable(priv->sw,"lst",szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);

    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    GtkTreeSelection *selection = TREE_SELECTION_LST(lst);

    g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK (on_lst_distr_selection_changed), win);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);

	gtk_widget_set_sensitive(priv->pb_lancia,          FALSE);
	gtk_widget_set_sensitive(priv->pb_spedizione_dati, FALSE);
	gtk_widget_set_sensitive(priv->pb_corriere,        FALSE);
	gtk_widget_set_sensitive(priv->pb_corr_agg,        FALSE);
	gtk_widget_set_sensitive(priv->pb_stampa_rac,      FALSE);
	gtk_widget_set_sensitive(priv->pb_stampa_xab,      FALSE);
	gtk_widget_set_sensitive(priv->pb_lista_articoli,  FALSE);
	gtk_widget_set_sensitive(priv->pb_stampa_dist,     FALSE);
	gtk_widget_set_sensitive(priv->pb_stampa_dc,       FALSE);
	gtk_widget_set_sensitive(priv->pb_stampa,          FALSE);
	gtk_widget_set_sensitive(priv->pb_sda,             FALSE);
	gtk_widget_set_sensitive(priv->pb_aggiorna,        FALSE);

}

static void distr_dispose (GObject *object)
{
    DistrWindowPrivate *priv = distr_get_instance_private (DISTR_WINDOW (object));

    if(priv->pszCfgFileName) {
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }


    G_OBJECT_CLASS (distr_parent_class)->dispose (object);
}

static void distr_class_init (DistrWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = distr_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, sw );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_lancia );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_spedizione_dati );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_corriere );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_corr_agg );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_stampa_rac );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_stampa_xab );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_lista_articoli );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_stampa_dist );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_stampa_dc );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_stampa );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_sda );
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), DistrWindow, pb_aggiorna );

}

DistrWindow * distr_new (MainWindow *win)
{
    DistrWindow *w = g_object_new (DISTR_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    DistrWindowPrivate *priv = distr_get_instance_private (DISTR_WINDOW (w));

    priv->main_window = win;

    return w;
}
