#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>


#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "editarticolo.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-articolo-window"

static MainWindow *_main_window;

struct _EditArticoloWindow
{
    GtkDialog parent;
};

typedef struct _EditArticoloWindowPrivate EditArticoloWindowPrivate;

struct _EditArticoloWindowPrivate
{
    MainWindow *main_window;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gchar *pszCfgFileName;
    gchar szActualTable[128];

    GtkWidget *lb_cdpro;
    GtkWidget *lb_dstit;
    GtkWidget *lb_dsaut;
    GtkWidget *lb_dimensioni;
    GtkWidget *lb_pesgr;
    GtkWidget *lb_msg;
    GtkWidget *pb_cerca_ubicazione;
    GtkWidget *pb_assegna_ubicazione;
    GtkWidget *pb_vuota_ubicazione;
    GtkWidget *pb_calcolo_colli_pcf;
    GtkWidget *pb_ok;
    GtkWidget *pb_record;
    GtkWidget *pb_prev;
    GtkWidget *pb_next;
    GtkWidget *pb_close;
    GtkWidget *sb_qtffo;
    GtkWidget *sb_ctffo;
    GtkWidget *sb_qtpcf;
    GtkWidget *sb_qtpcp;
    GtkWidget *rb_pallet;
    GtkWidget *rb_scaffale;
    GtkWidget *rb_automatico;
    GtkWidget *rb_manuale;
    GtkWidget *rb_equilibratura_automatica;
    GtkWidget *rb_equilibratura_manuale;
    GtkWidget *rb_normale;
    GtkWidget *rb_fuori_formato;
    GtkWidget *rb_preconfezionato;

    GtkWidget *combo_imballo_pcf;

    GtkWidget *entry_fila;
    GtkWidget *entry_montante;
    GtkWidget *entry_colonna;
    GtkWidget *entry_piano;
};

G_DEFINE_TYPE_WITH_PRIVATE(EditArticoloWindow, edit_articolo_window, GTK_TYPE_DIALOG)


void UpdateEditArticolo(gpointer win, gchar *product_code)
{
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
	char szCDPRO[128];
	char szCDUB1[128];
	char szCDUB2[128];
	char szCDUB3[128];
	char szCDUB4[128];
	char szDSTIT[128];
	char szDSAUT[128];
	char szCDFLG[128];
	char szSTATO[128];
	char szTPUBI[128];
	char szFCPCF[128];
	int nIndex;
	int nSWFFO;
	int nQTFFO;
	int nCTFFO;
	int nQTPCF;
	int nQTPCP;
	int nALTEZ;
	int nLARGH;
	int nLUNGH;
	int nPESGR;
	GList *lista_imballi=NULL;
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

#ifdef EIMINATO
	GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(priv->main_window),"lst");
    gchar *pszString;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)){
        if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
            /* ho ottenuto l'iter */
            gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,Cfg.nArticoliKeyField, &pszString); 

            strcpy(product_code,pszString);

            g_free(pszString);
        }
    }
#endif

	/*
	* riempio il combo contenente i codici imballo PCF
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select codice from imballi where ordtipo='%s' order by codice;",Cfg.szTipoOrdini);
	for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
		lista_imballi=g_list_append(lista_imballi,DBgetvalue(DBRes,nIndex,0));
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->combo_imballo_pcf), DBgetvalue(DBRes,nIndex,0));
	}

	g_list_free(lista_imballi);
	DBclear(DBRes);
	/* accetto solo valori presenti in lista e non vuoto */
    /* deprecated */
	/* gtk_combo_set_value_in_list(GTK_COMBO(priv->combo_imballo_pcf")),TRUE,FALSE); */

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select prdstit,prdsaut,praltez,prlargh,prlungh,prpesgr,prctffo from catalogo where prcdpro='%s';",product_code);
	if(DBntuples(DBRes)){
		strcpy(szDSTIT,DBgetvalue(DBRes,0,0));
		strcpy(szDSAUT,DBgetvalue(DBRes,0,1));
		nALTEZ = atoi( DBgetvalue(DBRes,0,2));
		nLARGH = atoi( DBgetvalue(DBRes,0,3));
		nLUNGH = atoi( DBgetvalue(DBRes,0,4));
		nPESGR = atoi( DBgetvalue(DBRes,0,5));
		nCTFFO = atoi( DBgetvalue(DBRes,0,6));
	} else {
		gtk_label_printf(priv->lb_msg,"Codice [%s] non presente in catalogo",product_code);
		bOK=FALSE;
	}
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select fila(bacdubi),montante(bacdubi),colonna(bacdubi),piano(bacdubi),bacdflg,bastato,batpubi,baswffo,baqtffo,baqtpcf,baqtpcp,bafcpcf from bal_art_tmp_%s_%s where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,product_code);
	if(DBntuples(DBRes)){
		strcpy(szCDUB1,DBgetvalue(DBRes,0,0));
		strcpy(szCDUB2,DBgetvalue(DBRes,0,1));
		strcpy(szCDUB3,DBgetvalue(DBRes,0,2));
		strcpy(szCDUB4,DBgetvalue(DBRes,0,3));
		strcpy(szCDFLG,DBgetvalue(DBRes,0,4));
		strcpy(szSTATO,DBgetvalue(DBRes,0,5));
		strcpy(szTPUBI,DBgetvalue(DBRes,0,6));
		nSWFFO = atoi( DBgetvalue(DBRes,0,7));
		nQTFFO = atoi( DBgetvalue(DBRes,0,8));
		nQTPCF = atoi( DBgetvalue(DBRes,0,9));
		nQTPCP = atoi( DBgetvalue(DBRes,0,10));
		strcpy(szFCPCF,DBgetvalue(DBRes,0,11));
	} else {
		gtk_label_printf(priv->lb_msg,"Codice [%s] non presente",product_code);
		bOK=FALSE;
	}
	DBclear(DBRes);

	if(bOK==FALSE){
		return;
	}


	/*
	sprintf(szCDUBI,"%-3.3s%3.3s%3.3s%3.3s",szCDUB1,szCDUB2,szCDUB3,szCDUB4);
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione from ubicazioni where ubicazione='%s';",szCDUBI);
	if(DBntuples(DBRes)){
		gtk_widget_set_sensitive(priv->rb_automatico,TRUE);
		gtk_widget_set_sensitive(priv->rb_manuale,TRUE);
		gtk_widget_set_sensitive(priv->rb_pallet,TRUE);
		gtk_widget_set_sensitive(priv->rb_scaffale,TRUE);
		gtk_widget_set_sensitive(priv->rb_equilibratura_automatica,TRUE);
		gtk_widget_set_sensitive(priv->rb_equilibratura_manuale,TRUE);
	} else {
		gtk_widget_set_sensitive(priv->rb_automatico,FALSE);
		gtk_widget_set_sensitive(priv->rb_manuale,FALSE);
		gtk_widget_set_sensitive(priv->rb_pallet,FALSE);
		gtk_widget_set_sensitive(priv->rb_scaffale,FALSE);
		gtk_widget_set_sensitive(priv->rb_equilibratura_automatica,FALSE);
		gtk_widget_set_sensitive(priv->rb_equilibratura_manuale,FALSE);
	}
	DBclear(DBRes);
	*/

	gtk_label_set_text(GTK_LABEL(priv->lb_cdpro),product_code);
	gtk_label_set_text(GTK_LABEL(priv->lb_dstit),szDSTIT);
	gtk_label_set_text(GTK_LABEL(priv->lb_dsaut),szDSAUT);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_fila),     szCDUB1);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_montante), szCDUB2);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_colonna),  szCDUB3);
	gtk_entry_set_text(GTK_ENTRY(priv->entry_piano),    szCDUB4);

	// gtk_entry_set_text(GTK_ENTRY(GTK_COMBO_BOX_TEXT(priv->combo_imballo_pcf)->entry), szFCPCF);


	gtk_label_printf(priv->lb_dimensioni,"%d X %d X %d",nLUNGH,nLARGH,nALTEZ);
	gtk_label_printf(priv->lb_pesgr,"%4d",nPESGR);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->sb_qtffo),nQTFFO);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->sb_ctffo),nCTFFO);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->sb_qtpcf),nQTPCF);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->sb_qtpcp),nQTPCP);

	if(nQTPCF){
        gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_preconfezionato), TRUE);
	} else {
		switch(nSWFFO){
			/* no fuori formato */
			default:
			case 0: 
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_normale), TRUE);
            break;
			/* fuori formato classico */
			case 1: 
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_fuori_formato), TRUE);
            break;
		}
	}

	switch(szCDFLG[0]){
		case UBICAZIONE_AUTOMATICA:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_automatico), TRUE);
		break;
		case UBICAZIONE_MANUALE:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_manuale), TRUE);
		break;
	}

	/* stato ubicazione */
	switch(szSTATO[0]){
		default:
		case UBICAZIONE_PALLET:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_pallet), TRUE);
		break;
		case UBICAZIONE_SCAFFALE:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_scaffale), TRUE);
		break;
	}
	/* tipo di equilibratura */
	switch(szTPUBI[0]){
		case UBICAZIONE_AUTOMATICA:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_equilibratura_automatica), TRUE);
		break;
		default:
		case UBICAZIONE_MANUALE:
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(priv->rb_equilibratura_manuale), TRUE);
		break;
	}
}


/* TODO - da usare nella applicazione */
static void RegisterEditArticolo(gpointer win)
{
	char *pszString;
	char szCDPRO[128];
	char szFCPCF[128];
	char szSWCOL[128];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
	int nTuples;
	int nIndex;
	int nQTFFO=0;
	int nCTFFO=0;
	int nQTPCF=0;
	int nQTPCP=0;
	ep_bool_t bSWFFO=FALSE;
	ep_bool_t bSWPCF=FALSE;
	char cSTATO;
	char cTPUBI;
	char cCDFLG;
	int nNMCPE;
	int nCopieTotali;
	int nCopieUbicate;
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

	strcpy(szCDPRO, gtk_label_get_text(GTK_LABEL(priv->lb_cdpro)));
	bSWFFO=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_fuori_formato));
	bSWPCF=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_preconfezionato));
	if(bSWFFO){

		nQTFFO=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_qtffo));
		nCTFFO=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_ctffo));

		if(nQTFFO<=0){
			bOK=FALSE;
			gtk_label_printf(priv->lb_msg,"Quantita fuori formato minore o uguale a 0");
		}

		if(nCTFFO<0){
			bOK=FALSE;
			gtk_label_printf(priv->lb_msg,"Categoria fuori formato minore di 0");
		}
	}
	if(bSWPCF){
		nQTPCF=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_qtpcf));
		if(nQTPCF<=0){
			bOK=FALSE;
			gtk_label_printf(priv->lb_msg,"Quantita preconfezionato minore o uguale a 0");
		} else {
			/* 
			* preconfezionato 
			* calcolo la qt in ubicazione
			*/
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.raqtord,o.roswcol from ric_art r,ric_ord o where r.racdpro='%s' and r.ordprog=o.ordprog and o.ordtipo='%s';",szCDPRO,Cfg.szTipoOrdini); 
			nTuples=DBntuples(DBRes);
			/* copie totali articolo */
			nCopieTotali=0;
			nCopieUbicate=0;
			for(nIndex=0;nIndex<nTuples;nIndex++){

				nNMCPE=atoi(DBgetvalue(DBRes,nIndex,0));

				nCopieTotali+=nNMCPE;

				strcpy(szSWCOL,DBgetvalue(DBRes,nIndex,1));
				if(szSWCOL[0]!='1'){
					/* normale - non pallet */
					/* calcolo la qta non in pcf (totale-resto pcf) */
					nCopieUbicate+=(nNMCPE%nQTPCF);
				}
			}
			DBclear(DBRes);
			/* aggiorno le qta nell'articolo */
			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set banmcpt=%d, banmcpe=%d where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,nCopieTotali,nCopieUbicate,szCDPRO);
			DBclear(DBRes);
		}
	}

	/* 
	* preconfezionato pallet
	* lo gestisco comunque 
	*/
	nQTPCP=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_qtpcp));
	if(nQTPCP<0){
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Quantità preconfezionato Pallet minore di 0");
	}


	/* flag ubicazione */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_automatico))){
		cCDFLG=UBICAZIONE_AUTOMATICA;
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_manuale))){
		cCDFLG=UBICAZIONE_MANUALE;
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Errore in tipo ubicazione (automatica/manuale)");
	}

	/* stato ubicazione */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_pallet))){
		cSTATO=UBICAZIONE_PALLET;
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_scaffale))){
		cSTATO=UBICAZIONE_SCAFFALE;
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Errore in tipo ubicazione (Pallet/Scaffale)");
	}

	/* tipo di equilibratura */
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_equilibratura_automatica))){
		cTPUBI=UBICAZIONE_AUTOMATICA;
	} else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->rb_equilibratura_manuale))){
		cTPUBI=UBICAZIONE_MANUALE;
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Errore in tipo equilibratura (Automatica/Manuale)");
	}

    /*
	strcpy(szFCPCF,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(priv->combo_imballo_pcf)->entry)));
	g_strstrip(szFCPCF);
    */

	if(!bOK){
		return;
	}
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set bacdflg='%c', bastato='%c', batpubi='%c', baswffo=%d, baqtffo=%d, baqtpcf=%d, baqtpcp=%d, bafcpcf='%s' where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,cCDFLG,cSTATO,cTPUBI,bSWFFO,nQTFFO,nQTPCF,nQTPCP,szFCPCF,szCDPRO);
	if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))) {
		gtk_label_printf(priv->lb_msg,"Codice [%s] dati registrati ",szCDPRO);
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Codice [%s] Errore in registrazione dati",szCDPRO);
	}
	DBclear(DBRes);
	if(bOK){
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update catalogo set prswffo=%d, prqtffo=%d, prctffo=%d, prqtpcf=%d, prqtpcp=%d, prfcpcf='%s' where prcdpro='%s';",bSWFFO,nQTFFO,nCTFFO,nQTPCF,nQTPCP,szFCPCF,szCDPRO);
		if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))) {
			gtk_label_printf(priv->lb_msg,"Catalogo : Codice [%s] dati registrati ",szCDPRO);
		} else {
			bOK=FALSE;
			gtk_label_printf(priv->lb_msg,"Catalogo : Codice [%s] Errore in registrazione dati",szCDPRO);
		}
		DBclear(DBRes);
	}

	if(bOK){
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        ep_update_table(win, "sw", "lst",priv->pszCfgFileName, "Equilibratura", priv->szActualTable,szCDPRO, TRUE);
	}
}

/* TODO - da usare nella applicazione */
static void EditArticoloAssegnaUbicazione (gpointer win)
{
	char *pszString;
	char szCDUB1[128];
	char szCDUB2[128];
	char szCDUB3[128];
	char szCDUB4[128];
	char szCDUBI[128];
	char szCDPRO[128];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

	strcpy(szCDPRO, gtk_label_get_text(GTK_LABEL(priv->lb_cdpro)));

	strcpy(szCDUB1,gtk_entry_get_text(GTK_ENTRY(priv->entry_fila)));      g_strstrip(szCDUB1);
	strcpy(szCDUB2,gtk_entry_get_text(GTK_ENTRY(priv->entry_montante)));  g_strstrip(szCDUB2);
	strcpy(szCDUB3,gtk_entry_get_text(GTK_ENTRY(priv->entry_colonna)));   g_strstrip(szCDUB3);
	strcpy(szCDUB4,gtk_entry_get_text(GTK_ENTRY(priv->entry_piano)));     g_strstrip(szCDUB4);

	sprintf(szCDUBI,"%-3.3s%3.3s%3.3s%3.3s",szCDUB1,szCDUB2,szCDUB3,szCDUB4);

	/* verifico esistenza ubicazione */
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione from ubicazioni where ubicazione='%s'",szCDUBI);
	if(DBntuples(DBRes)==0){
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Ubicazione [%s] non presente",szCDUBI);
	}
	DBclear(DBRes);

	if(!bOK){
		return;
	}
	/* 
	* assegno l'ubicazione e il flag di equilibratura manuale (batpubi)
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set bacdubi='%s',batpubi='%c' where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,szCDUBI,UBICAZIONE_MANUALE,szCDPRO);
	if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))) {
		gtk_label_printf(priv->lb_msg,"Codice [%s] Ubicazione [%s] assegnata ",szCDPRO,szCDUBI);
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Codice [%s] Errore in assegnazione Ubicazione [%s]",szCDPRO,szCDUBI);
	}
	DBclear(DBRes);

	if(bOK){
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        ep_update_table(win, "sw", "lst",priv->pszCfgFileName, "Equilibratura", priv->szActualTable,szCDPRO, TRUE);
		UpdateEditArticolo(win,szCDPRO);
	}
}

/* TODO - da usare nella applicazione */
static void EditArticoloVuotaUbicazione (gpointer win)
{
	char *pszString;
	char szCDPRO[128];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

	strcpy(szCDPRO, gtk_label_get_text(GTK_LABEL(priv->lb_cdpro)));

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set bacdubi='' where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,szCDPRO);
	if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))) {
		gtk_label_printf(priv->lb_msg,"Codice [%s] Ubicazione vuotata ",szCDPRO);
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Codice [%s] Errore in vuotamento Ubicazione",szCDPRO);
	}
	DBclear(DBRes);

	if(bOK){
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        ep_update_table(win, "sw", "lst",priv->pszCfgFileName, "Equilibratura", priv->szActualTable,szCDPRO, TRUE);
		UpdateEditArticolo(win, szCDPRO);
	}

}

/* TODO - da usare nella applicazione */
static void EditArticoloCalcoloColliPCF (gpointer win)
{
	char *pszString;
	char szCDPRO[128];
	ep_bool_t bOK=TRUE;
	DBresult *DBRes;
	int nIndex;
	int nTuples;
	int nColli=0;
	int nResto=0;
	int nQTPCF=0;
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

	strcpy(szCDPRO, gtk_label_get_text(GTK_LABEL(priv->lb_cdpro)));
	nQTPCF=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_qtpcf));

	if(nQTPCF<=0){
		gtk_label_printf(priv->lb_msg,"Il numero copie per collo preconfezionato e' minore o uguale a 0");
		return;
	}

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select raqtord from ric_art r, ric_ord o where r.racdpro='%s' and r.ordprog=o.ordprog and o.ordtipo='%s' and o.roswcol!='1';",szCDPRO,Cfg.szTipoOrdini);
	if((nTuples=DBntuples(DBRes))) {
		nColli=0;
		nResto=0;
		for(nIndex=0;nIndex<nTuples;nIndex++){
			nColli+=atoi(DBgetvalue(DBRes,nIndex,0))/nQTPCF;
			nResto+=atoi(DBgetvalue(DBRes,nIndex,0))%nQTPCF;
		}
	} else {
		bOK=FALSE;
		gtk_label_printf(priv->lb_msg,"Errore in calcolo colli di preconfezionato");
	}
	DBclear(DBRes);

	if(bOK){
#ifdef TODO
		GtkWidget *dlg = create_dlg_message();
		GtkWidget *lb_msg = priv->lb_msg;
		GtkWidget *pb_ok = priv->pb_ok;

		gtk_window_set_title (GTK_WINDOW (dlg), "Calcolo Colli Preconfezionato");
		gtk_window_set_focus (GTK_WINDOW (dlg), pb_ok);

		/*
		* nessun ordine della lista e' selezionato
		*/
		gtk_label_printf(lb_msg,"%4d - Righe contenenti l'articolo\n%4d - Colli preconfezionati\n%4d - Copie rimanenti",nTuples,nColli,nResto);
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (on_dlg_message_pb_ok_clicked), dlg);
		gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
		gtk_widget_show(dlg);
#endif
	}

}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {
  {"close", close_activated, NULL, NULL, NULL}
};

static void edit_articolo_window_init (EditArticoloWindow *win)
{
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    strcpy(priv->szActualTable, "articoli");

    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);


    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);
    
    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);

	GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(_main_window),"lst");
    gchar *pszString;
	char szCDPRO[128];
	gboolean found = FALSE;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)){
        if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
            /* ho ottenuto l'iter */
            gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,Cfg.nArticoliKeyField, &pszString); 

            strcpy(szCDPRO,pszString);
			found = TRUE;

            g_free(pszString);
        }
    }
	if (found) {
		UpdateEditArticolo(win,szCDPRO);
	}

}

static void edit_articolo_window_dispose (GObject *object)
{
	EditArticoloWindow *win = EDIT_ARTICOLO_WINDOW (object);
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (win);

    g_printf("%s dispose\n", WINDOW_NAME);

    if(priv->wsi){          
        g_settings_sync ();
        window_save_state (win,  priv->wsi, priv->settings);
        window_size_info_free (priv->wsi);
        priv->wsi = NULL;
    }

    if(priv->settings){          
        g_clear_object (&(priv->settings)); 
        priv->settings=NULL;    
    }

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (edit_articolo_window_parent_class)->dispose (object);
}

static void edit_articolo_window_class_init (EditArticoloWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_articolo_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_cdpro);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_dstit);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_dsaut);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_dimensioni);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_pesgr);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, lb_msg);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_cerca_ubicazione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_assegna_ubicazione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_vuota_ubicazione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_calcolo_colli_pcf);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_ok);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_record);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_prev);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_next);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, pb_close);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, sb_qtffo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, sb_ctffo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, sb_qtpcf);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, sb_qtpcp);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_pallet);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_scaffale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_automatico);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_manuale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_equilibratura_automatica);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_equilibratura_manuale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_normale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_fuori_formato);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, rb_preconfezionato);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, combo_imballo_pcf);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, entry_fila);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, entry_montante);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, entry_colonna);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditArticoloWindow, entry_piano);
}

EditArticoloWindow * edit_articolo_window_new (MainWindow *win)
{
    g_printf("%s new\n", WINDOW_NAME);
	_main_window = win;
    EditArticoloWindow *w =  g_object_new (EDIT_ARTICOLO_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
    EditArticoloWindowPrivate *priv = edit_articolo_window_get_instance_private (w);
    priv->main_window = win;

    return w;
}
