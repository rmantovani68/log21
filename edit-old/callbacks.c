/*
* callbacks.c
* Callbacks functions
* Easy Picking 3.0
* Copyright A&L-HSD srl 1999-2002
* Autori : Roberto Mantovani
*          Stefano Tarroni
*/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef TRACE
#	include <trace.h>
#endif

#include <gnome.h>
#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <time.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include <proc_list.h>
#include <pmx_msq.h>
#include <ca_file.h>
#include <dbfun.h>

#include "picking.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"



/********************************************************************
********************* Pulsanti di main_window ***********************
********************************************************************/

void on_main_pb_config_clicked (gpointer user_data, GtkButton *button)
{
	EditConfigurazione();
}

void on_main_pb_print_clicked               (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg= create_dlg_message();
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	if(GTK_CLIST(clist)==NULL) return;


	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));

	gtk_label_printf(get_widget(dlg,"lb_msg"),"Conferma la stampa della lista attuale");
	g_signal_connect_swapped (GTK_OBJECT (get_widget(dlg,"pb_ok")), "clicked", G_CALLBACK (do_print), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg), "Stampa");
	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_widget_show(dlg);

}
	

void on_main_pb_edit_clicked (gpointer user_data, GtkButton *button)
{
	edit_clist_row(GTK_CLIST(get_widget(main_window,"clist")));
}

void on_main_pb_search_clicked              (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg_search = create_dlg_search();
	gtk_clist_unselect_all(GTK_CLIST(get_widget(main_window,"clist")));
	gtk_window_set_transient_for(GTK_WINDOW(dlg_search),GTK_WINDOW(main_window));
	gtk_widget_show(dlg_search);
}

void on_main_pb_select_clicked              (gpointer user_data, GtkButton *button)
{
	do_dlg_select();
}

void on_main_pb_exit_clicked (gpointer user_data, GtkButton *button)
{
	show_exit_dlg();
}

/********************************************************************
**************** Fine Pulsanti di main_window ***********************
********************************************************************/




/********************************************************************
******************************  MENU' *******************************
********************************************************************/

void on_pbm_print_activate                  (GtkMenuItem     *menuitem, gpointer         user_data)
{
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	if(GTK_CLIST(clist)==NULL) return;

	PrintTable("edit",szTableName,Cfg.szPrinterConsole,szActualKey,NULL);
}

void on_pbm_aggiorna_lista_activate(GtkMenuItem *menuitem, gpointer user_data)
{
	RefreshTable(main_window,"sw_list","clist",szTableName,"lb_table_description","lb_table_rows","edit",szTableName,GTK_SELECTION_SINGLE,szActualKey);
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),FALSE);
}

void on_pbm_exit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	show_exit_dlg();
}


void on_pbm_about_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	do_about();
}


void on_pbm_edit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	edit_clist_row(GTK_CLIST(get_widget(main_window,"clist")));
}

void on_pbm_help_activate(GtkMenuItem * menuitem, gpointer user_data)
{

}

void on_pbm_configurazione_activate(GtkMenuItem * menuitem, gpointer user_data) 
{
	EditConfigurazione();
}

/********************************************************************
********** Pulsanti di edit archivi presenti nel main menu' *********
********************************************************************/

void on_pbm_ordini_activate                 (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ordini"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_opeset_activate                 (GtkMenuItem     *menuitem, gpointer         user_data)
{ 
	load_table(strcpy(szTableName,"opeset"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_ubicazioni_activate             (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ubicazioni"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_prodotti_activate               (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"catalogo"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_imballi_activate                (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"imballi"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_operatori_activate              (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"operatori"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_causali_di_trasporto_activate   (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttcs"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_forzature_clienti_activate      (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttfc"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_forme_di_pagamento_activate     (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttfp"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_gruppi_cap_activate             (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttgc"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_linee_di_spedizione_activate    (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttls"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_linee_vettori_activate          (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttlv"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_raggruppamenti_cap_activate     (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttrc"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_tipi_di_spedizione_activate     (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttts"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_anagrafica_vettori_activate     (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttve"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_descrizione_linee_activate      (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"sddslin"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_descrizione_linee_per_cedola_activate (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttds"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_descrizione_linee_generali_activate (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"ttds_gen"),strcpy(szActualKey,"TRUE"));

}



void on_pbm_storico_linee_di_spedizione_activate    (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"storico_linee_spedizione"),strcpy(szActualKey,"TRUE"));
}


void on_pbm_storico_bancali_activate                (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"storico_bancali"),strcpy(szActualKey,"TRUE"));
}


void on_pbm_storico_colli_su_bancale_activate       (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"storico_colli_bancale"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_storico_ordini_activate         (GtkMenuItem     *menuitem, gpointer         user_data)
{
	GtkWidget *dlg= create_dlg_cedola();
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");

	gtk_window_set_focus (GTK_WINDOW (dlg), pb_ok);
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));

}

void on_pbm_colli_scartati_activate         (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"colli_scartati"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_eventi_activate                 (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"eventi"),strcpy(szActualKey,"TRUE"));
}

void on_pbm_nota_linee_activate             (GtkMenuItem     *menuitem, gpointer         user_data)
{
	load_table(strcpy(szTableName,"nota_linee"),strcpy(szActualKey,"TRUE"));
}

/********************************************************************
***** Fine Pulsanti di edit archivi presenti nel main menu' *********
********************************************************************/

/********************************************************************
************************** FINE MENU' *******************************
********************************************************************/





void on_mb_main_activate_current (GtkMenuShell *menushell, gboolean force_hide, gpointer user_data)
{
}


void on_dlg_exit_pb_ok_clicked(gpointer user_data, GtkButton *button)
{
	ExitApp();
}


gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) 
{
	show_exit_dlg();
  return TRUE;
}


/*
* void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data)
*
* Effettua un ordinamento sulla clist : utilizza la colonna (nColumnNumber) selezionata come discriminante;
* se la stessa colonna viene selezionanata di seguito l'ordinamento varia da ascendente a discentente.
*/
void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data)
{
	static int nOldColumnNumber=-1;
	static int nOrd;

	if (nColumnNumber!=nOldColumnNumber){
		nOrd=0;
		nOldColumnNumber=nColumnNumber;
	} else {
		nOrd++;
	}
	gtk_clist_set_sort_type(GTK_CLIST(clist),(nOrd%2==0)?GTK_SORT_ASCENDING:GTK_SORT_DESCENDING);
	gtk_clist_set_sort_column(GTK_CLIST(clist), nColumnNumber);
	gtk_clist_sort(GTK_CLIST(clist));
}


void on_clist_select_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),TRUE);
	gtk_widget_set_sensitive(get_widget(main_window,"pbm_edit"),TRUE);
}

void on_clist_unselect_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),FALSE);
	gtk_widget_set_sensitive(get_widget(main_window,"pbm_edit"),FALSE);
}

void show_exit_dlg(void)
{
	GtkWidget *dlg_exit = create_dlg_exit();
	gtk_window_set_focus (GTK_WINDOW (dlg_exit), get_widget(dlg_exit,"pb_ok"));
	gtk_widget_show(dlg_exit);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_exit),GTK_WINDOW(main_window));
}

void edit_clist_row(GtkCList *clist)
{
	char *pszKey;
	char szKey[256];
	int nRowSelected=0;

	if(GTK_CLIST(clist)==NULL) return;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	/* ricavo il codice nella clist a video */
	gtk_clist_get_text(GTK_CLIST(clist),nRowSelected,0,&pszKey);
	strcpy(szKey,pszKey);

	if(!strcmp(szTableName,"ordini")){
		/* completare l'edit dei colli */
		/* rivedere causa variazioni alle relazioni DB */

		GtkWidget *dlg_edit_ordine = create_dlg_edit_ordine();
		edit_dlg_ordini(dlg_edit_ordine,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_edit_ordine),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_edit_ordine);

	} else if(!strcmp(szTableName,"ordini_storico")){
		/* completare l'edit dei colli */

		GtkWidget *dlg_edit_ordine = create_dlg_edit_ordine();
		edit_dlg_ordini(dlg_edit_ordine,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_edit_ordine),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_edit_ordine);
	} else if(!strcmp(szTableName,"opeset")){

		GtkWidget *dlg_opeset = create_dlg_opeset();
		edit_dlg_opeset(dlg_opeset,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_opeset),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_opeset);

	} else if(!strcmp(szTableName,"ubicazioni")){

		GtkWidget *dlg_ubicazioni = create_dlg_ubicazioni();
		edit_dlg_ubicazioni(dlg_ubicazioni,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_ubicazioni),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_ubicazioni);

	} else if(!strcmp(szTableName,"catalogo")){

		GtkWidget *dlg_catalogo = create_dlg_catalogo();
		edit_dlg_prodotto(dlg_catalogo,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_catalogo),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_catalogo);

	} else if(!strcmp(szTableName,"imballi")){

		GtkWidget *dlg_imballi = create_dlg_imballi();
		edit_dlg_imballi(dlg_imballi,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_imballi),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_imballi);

	} else if(!strcmp(szTableName,"operatori")){

		GtkWidget *dlg_operatori = create_dlg_operatori();
		edit_dlg_operatori(dlg_operatori,szKey);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_operatori),GTK_WINDOW(main_window));
		gtk_widget_show(dlg_operatori);

	} else if(
		!strcmp(szTableName,"nota_linee") ||
		!strcmp(szTableName,"ttcs") ||
		!strcmp(szTableName,"ttfc") ||
		!strcmp(szTableName,"ttfp") ||
		!strcmp(szTableName,"ttgc") ||
		!strcmp(szTableName,"ttls") ||
		!strcmp(szTableName,"ttlv") ||
		!strcmp(szTableName,"ttrc") ||
		!strcmp(szTableName,"ttts") ||
		!strcmp(szTableName,"ttve") ||
		!strcmp(szTableName,"sddslin") ||
		!strcmp(szTableName,"ttds") ||
		!strcmp(szTableName,"ttds_gen") ||
		!strcmp(szTableName,"eventi") ||
		!strcmp(szTableName,"storico_linee_spedizione") ||
		!strcmp(szTableName,"storico_bancali") ||
		!strcmp(szTableName,"storico_colli_bancale")
	){
		GtkWidget *dlg = CreateDBEditDlg(szTableName);
		gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
		gtk_widget_show(dlg);
		GetKeysList(szTableName,clist,nRowSelected,szKey);
		UpdateDBEditDlg(dlg,szTableName,szKey);
	}

	gtk_window_set_focus (GTK_WINDOW(main_window), GTK_WIDGET(clist));
}

/********************************************************************
*********************** DLG_EDIT_ORDINE *****************************
********************************************************************/

void on_dlg_edit_ordine_pb_prev_clicked        (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char *pszCodiceOrdine;
	char szOrdineInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *lst=GTK_CLIST(get_widget(main_window,"clist"));

	/* ricavo il codice dell'ordine attualmente in edit */
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_ORDPROG")),&pszCodiceOrdine);
	strcpy(szOrdineInEdit,pszCodiceOrdine);

	nIndex=GetKeyIndex(szOrdineInEdit,lst,0,0);

	if(nIndex>0){
		gtk_clist_get_text(lst,--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_ordini(dlg,szKey);
		clist_update_selected_row(lst,-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_edit_ordine_pb_next_clicked        (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char *pszCodiceOrdine;
	char szOrdineInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *lst=GTK_CLIST(get_widget(main_window,"clist"));

	/* ricavo il codice dell'ordine attualmente in edit */
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_ORDPROG")),&pszCodiceOrdine);
	strcpy(szOrdineInEdit,pszCodiceOrdine);

	nIndex=GetKeyIndex(szOrdineInEdit,lst,0,0);

	if(nIndex<lst->rows-1){
		gtk_clist_get_text(lst,++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_ordini(dlg,szKey);
		clist_update_selected_row(lst,1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}

void on_dlg_edit_ordine_pb_edit_collo_clicked  (gpointer user_data, GtkButton *button)
{
	char *pszKey;
	char szCodiceOrdine[128];
	int nRowSelected=0;
	int nCollo;
	GtkWidget *dlg = create_dlg_edit_collo();
	GtkCList *lst=GTK_CLIST(get_widget(GTK_WIDGET(user_data),"lista_righe_ordine"));

	gtk_label_get(GTK_LABEL(get_widget(user_data,"lb_ORDPROG")),&pszKey);
	strcpy(szCodiceOrdine,pszKey);

	if(lst->selection){
		nRowSelected=GPOINTER_TO_INT(lst->selection->data);
	}

	/* ricavo il numero del collo */
	gtk_clist_get_text(lst,nRowSelected,0,&pszKey);
	nCollo=atoi(pszKey);

	edit_dlg_collo(dlg,szCodiceOrdine,nCollo);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(user_data));
	gtk_widget_show(dlg);
}

/********************************************************************
************************ DLG_EDIT_COLLO *****************************
********************************************************************/
void on_dlg_edit_collo_pb_prev_clicked   (gpointer user_data, GtkButton *button)
{
	int nColloInEdit;
	char *pszColloInEdit;
	char *pszCodiceOrdine;
	char szCodiceOrdine[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_ORDPROG")),&pszCodiceOrdine);
	strcpy(szCodiceOrdine,pszCodiceOrdine);
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_NMCOL")),&pszColloInEdit);
	nColloInEdit=atoi(pszColloInEdit);

	if(nColloInEdit>1){
		edit_dlg_collo(dlg,szCodiceOrdine,--nColloInEdit);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_edit_collo_pb_next_clicked   (gpointer user_data, GtkButton *button)
{
	int nColloInEdit;
	char *pszColloInEdit;
	char *pszCodiceOrdine;
	char szCodiceOrdine[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	PGresult *PGres;
	/* N.B. da fare la gestione dello storico */
	BOOL bStorico=FALSE;
	if(!strcmp(szTableName,"ordini_storico")){
		bStorico=TRUE;
	}

	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_ORDPROG")),&pszCodiceOrdine);
	strcpy(szCodiceOrdine,pszCodiceOrdine);
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_NMCOL")),&pszColloInEdit);
	nColloInEdit=atoi(pszColloInEdit);

	/* ricavo il numero di colli dell'ordine */
	if(bStorico){
		PGres=PGExecSQL(Cfg.nDebugVersion,"select ronmcll from ric_ord_stor where ordprog='%s';",szCodiceOrdine);
	} else {
		PGres=PGExecSQL(Cfg.nDebugVersion,"select ronmcll from ric_ord where ordprog='%s';",szCodiceOrdine);
	}

	if(PQntuples(PGres) && nColloInEdit<atoi(PQgetvalue(PGres,0,0))){
		edit_dlg_collo(dlg,szCodiceOrdine,++nColloInEdit);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
	PQclear(PGres);
}


/********************************************************************
************************* DLG_CATALOGO ******************************
********************************************************************/

void on_dlg_catalogo_pb_prev_clicked        (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char *pszCodiceProdotto;
	char szProdottoInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice del prodotto attualmente in edit */
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_prcdpro")),&pszCodiceProdotto);
	strcpy(szProdottoInEdit,pszCodiceProdotto);

	nIndex=GetKeyIndex(szProdottoInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex>0){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_prodotto(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_catalogo_pb_modifica_clicked    (gpointer user_data, GtkButton *button)
{ 
	GtkWidget *lb_msg=(GtkWidget *)NULL;
	GtkWidget *pb_ok=(GtkWidget *)NULL;
	char szCodiceProdotto[40];
	char *pszCodiceProdotto;
	GtkWidget *dlg =GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_prcdpro")),&pszCodiceProdotto);
	strcpy(szCodiceProdotto,pszCodiceProdotto);

	gtk_label_printf(lb_msg,"CONFERMI LA MODIFICA DEL PRODOTTO %s",szCodiceProdotto);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_modifica_prodotto), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Modifica Dati Prodotto");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_catalogo_pb_next_clicked        (gpointer user_data, GtkButton *button)
{ 
	int nIndex;
	char *pszCodiceProdotto;
	char szProdottoInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice del prodotto attualmente in edit */
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_prcdpro")),&pszCodiceProdotto);
	strcpy(szProdottoInEdit,pszCodiceProdotto);

	nIndex=GetKeyIndex(szProdottoInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex<(GTK_CLIST(get_widget(main_window,"clist"))->rows)-1){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_prodotto(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),+1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}

void do_modifica_prodotto (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodProd[128];
	char *pszCodProd;
	char szFCPCF[128];
	
	gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_prcdpro")),&pszCodProd);
	strcpy(szCodProd,pszCodProd);
	strcpy(szFCPCF,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_imballo_pcf"))->entry)));
	g_strstrip(szFCPCF);

	if(strlen(szCodProd)!=0){
		sprintf(szSQLCmd,"update catalogo set prlungh=%d , prlargh=%d , praltez=%d , prpesgr=%d , prswffo=%d , prctffo=%d, prqtffo=%d , prfcpcf='%s', prqtpcf=%d , prqtpcp=%d where prcdpro='%s';",
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_peso"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_swffo"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_ctffo"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtffo"))),
			szFCPCF,
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcf"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_qtpcp"))),
			szCodProd);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Modificato prodotto [%s]",szCodProd);
		} else {
			trace_debug(&cRED,"Fallita modifica su prodotto [%s]",szCodProd);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","catalogo",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodProd,GTK_CLIST(get_widget(main_window,"clist")),0,0));

		edit_dlg_prodotto(dlg,szCodProd);
	}
}

/********************************************************************
************************* DLG_IMBALLI *******************************
********************************************************************/

void on_dlg_imballi_pb_elimina_clicked      (gpointer user_data, GtkButton *button)
{ 
	char szCodiceImballo[40];
	GtkWidget *lb_msg = (GtkWidget *)NULL;
	GtkWidget *pb_ok = (GtkWidget *)NULL;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");
	strcpy(szCodiceImballo,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI L'ELIMINAZIONE DELL'IMBALLO %s ?",szCodiceImballo);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_elimina_imballo), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Cancellazione imballi");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_imballi_pb_aggiungi_clicked     (gpointer user_data, GtkButton *button)
{ 
	char szCodiceImballo[40];
	GtkWidget *lb_msg = (GtkWidget *)NULL;
	GtkWidget *pb_ok = (GtkWidget *)NULL;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");
	strcpy(szCodiceImballo,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI L'INSERIMENTO DELL'IMBALLO %s ?",szCodiceImballo);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_aggiungi_imballo), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Inserimento imballi");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_imballi_pb_modifica_clicked     (gpointer user_data, GtkButton *button)
{ 
	char szCodiceImballo[40];
	GtkWidget *lb_msg = (GtkWidget *)NULL;
	GtkWidget *pb_ok = (GtkWidget *)NULL;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");
	strcpy(szCodiceImballo,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI LA MODIFICA DELL'IMBALLO %s ?",szCodiceImballo);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_modifica_imballo), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Modifica dati imballo");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_imballi_pb_prev_clicked         (gpointer user_data, GtkButton *button)
{ 
	int nIndex;
	char szImballoInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice dell'imballo attualmente in edit */
	strcpy(szImballoInEdit,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	nIndex=GetKeyIndex(szImballoInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex>0){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_imballi(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_imballi_pb_next_clicked         (gpointer user_data, GtkButton *button)
{ 
	int nIndex;
	char szImballoInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice dell'imballo attualmente in edit */
	strcpy(szImballoInEdit,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	nIndex=GetKeyIndex(szImballoInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex<(GTK_CLIST(get_widget(main_window,"clist"))->rows)-1){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_imballi(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),+1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}


void do_modifica_imballo (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodImb[128];
	
	strcpy(szCodImb,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodImb)!=0){
		sprintf(szSQLCmd,"update imballi set descriz='%s', lunghezza=%d, altezza=%d, larghezza=%d, tara=%d, pesomax=%d, volperc=%d, tplavor=%d, categoria=%d where codice=trim('%s') and ordtipo='%s';",
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_descrizione"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tara"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_pesomax"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_volperc"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tplavor"))),
			gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_categoria"))),
			szCodImb, Cfg.szTipoOrdini);

		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Modificato imballo [%s]",szCodImb);
		} else {
			trace_debug(&cRED,"Fallita modifica su imballo [%s]",szCodImb);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodImb,GTK_CLIST(get_widget(main_window,"clist")),0,0));

		edit_dlg_imballi(dlg,szCodImb);
	}
}


void do_aggiungi_imballo (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodImb[128];
	BOOL bOk=TRUE;
	
	strcpy(szCodImb,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodImb)!=0){
		/* controllo che non sia presente un imballo avente il medesimo codice */
		PGRes=PGExecSQL(Cfg.nDebugVersion,"select codice from imballi where codice='%s';",szCodImb);
		if(PQntuples(PGRes)){
			trace_debug(&cRED,"Fallito inserimento imballo [%s] : Codice gia' presente in anagrafica imballi",szCodImb);
			bOk=FALSE;
		}
		PQclear(PGRes);

		if(bOk){
			sprintf(szSQLCmd,"insert into imballi (codice,descriz,lunghezza,altezza,larghezza,tara,pesomax,volperc,tplavor,categoria,ordtipo) values ('%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,'%s');" ,
				szCodImb,
				gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_descrizione"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_lunghezza"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_altezza"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_larghezza"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tara"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_pesomax"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_volperc"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_tplavor"))),
				gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_categoria"))),
				Cfg.szTipoOrdini);
			
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
			if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
				trace_debug(&cGREEN,"Inserito imballo [%s]",szCodImb);
			} else {
				trace_debug(&cRED,"Fallito inserimento imballo [%s]",szCodImb);
			}
			PQclear(PGRes);

			RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey);

			clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodImb,GTK_CLIST(get_widget(main_window,"clist")),0,0));

			edit_dlg_imballi(dlg,szCodImb);
		}
	} else {
		trace_debug(&cRED,"Fallito inserimento : Inserire il codice");
	}
}


void do_elimina_imballo (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodImb[128];
	char *pszKey;
	int nRowSelected;
	
	strcpy(szCodImb,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodImb)!=0){
		sprintf(szSQLCmd,"delete from imballi where codice=trim('%s') and ordtipo='%s';",szCodImb,Cfg.szTipoOrdini);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Eliminato imballo [%s]",szCodImb);
		} else {
			trace_debug(&cRED,"Fallita eliminazione imballo [%s]",szCodImb);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		if((GTK_CLIST(get_widget(main_window,"clist"))->selection)){
			nRowSelected=GPOINTER_TO_INT(GTK_CLIST(get_widget(main_window,"clist"))->selection->data);
		}
		/* ricavo il codice nella clist a video */
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),nRowSelected,0,&pszKey);
		strcpy(szCodImb,pszKey);
		edit_dlg_imballi(dlg,szCodImb);
	}
}


/********************************************************************
*********************** DLG_UBICAZIONI ******************************
********************************************************************/

void on_dlg_ubicazioni_pb_prev_clicked      (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szUbicazioneInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice dell'ubicazione attualmente in edit */
	nIndex=GetKeyIndex(get_ubicazione_in_edit(dlg,szUbicazioneInEdit),GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex>0){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_ubicazioni(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_ubicazioni_pb_next_clicked      (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szUbicazioneInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	/* ricavo il codice dell'ubicazione attualmente in edit */
	nIndex=GetKeyIndex(get_ubicazione_in_edit(dlg,szUbicazioneInEdit),GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex<(GTK_CLIST(get_widget(main_window,"clist"))->rows)-1){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_ubicazioni(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),+1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}

void on_dlg_ubicazioni_pb_vuota_clicked     (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szCodiceProdotto[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_prodotto"))));
	get_ubicazione_in_edit(dlg,szCodiceUbicazione);

	gtk_label_printf(lb_msg,"CONFERMI L'ELIMINAZIONE DEL PRODOTTO %s DALL'UBICAZIONE %s ?",szCodiceProdotto,szCodiceUbicazione);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_vuota_ubicazione), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Vuota ubicazione");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_ubicazioni_pb_assegna_clicked   (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szCodiceProdotto[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_prodotto"))));
	get_ubicazione_in_edit(dlg,szCodiceUbicazione);

	gtk_label_printf(lb_msg,"CONFERMI L'INSERIMENTO DEL PRODOTTO %s NELL'UBICAZIONE %s ?",szCodiceProdotto,szCodiceUbicazione);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_assegna_ubicazione), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Assegna ubicazione");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_ubicazioni_pb_abilitazione_clicked   (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szStato[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message;
	BOOL bAbilitata;


	/* ricavo il codice ubicazione ed il prodotto */
	get_ubicazione_in_edit(dlg,szCodiceUbicazione);

	strcpy(szStato,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_stato"))));

	switch(szStato[0]){
		case 'P':
		case 'S':
			bAbilitata=TRUE;
		break;
		case 'Q':
		case 'T':
			bAbilitata=FALSE;
		break;
		default:
			trace_debug(&cRED,"Stato ubicazione [%s] non corretto [%c]",szCodiceUbicazione,szStato[0]);
			return;
		break;
	}

	dlg_message = create_dlg_message();
	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	if(bAbilitata){
		gtk_window_set_title (GTK_WINDOW (dlg_message), "Disabilita Ubicazione");
		gtk_label_printf(lb_msg,"CONFERMI LA DISABILITAZIONE DELLA UBICAZIONE %s ?",szCodiceUbicazione);
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_disabilita_ubicazione), dlg);
	} else {
		gtk_window_set_title (GTK_WINDOW (dlg_message), "Abilita Ubicazione");
		gtk_label_printf(lb_msg,"CONFERMI LA ABILITAZIONE DELLA UBICAZIONE %s ?",szCodiceUbicazione);
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_abilita_ubicazione), dlg);
	}
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void do_abilita_ubicazione                    (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szStato[40];
	PGresult *PGRes;
	GtkWidget *dlg =GTK_WIDGET(user_data);
	
	/* ricavo il codice ubicazione e lo stato */
	/* 
	* la abilitazione consiste nell'assegnare
	* allo stato ubicazione la lettera precedente
	* es. : T->S Q->P
	*/

	get_ubicazione_in_edit(dlg,szCodiceUbicazione);
	strcpy(szStato,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_stato"))));

	if(strlen(szCodiceUbicazione)!=0){
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update ubicazioni set cnistato='%c' where ubicazione='%s';",szStato[0]-1,szCodiceUbicazione);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Abilitata ubicazione %s [%c]",szCodiceUbicazione,szStato[0]-1);
		} else {
			trace_debug(&cRED,"Fallita abilitazione ubicazione %s [%c]",szCodiceUbicazione,szStato[0]-1);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
	}
}

void do_disabilita_ubicazione                    (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szStato[40];
	PGresult *PGRes;
	GtkWidget *dlg =GTK_WIDGET(user_data);
	
	/* ricavo il codice ubicazione e lo stato */
	/* 
	* la disabilitazione consiste nell'assegnare
	* allo stato ubicazione la lettera successiva
	* es. : S->T P->Q
	*/

	get_ubicazione_in_edit(dlg,szCodiceUbicazione);
	strcpy(szStato,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_stato"))));

	if(strlen(szCodiceUbicazione)!=0){
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update ubicazioni set cnistato='%c' where ubicazione='%s';",szStato[0]+1,szCodiceUbicazione);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Abilitata ubicazione %s [%c]",szCodiceUbicazione,szStato[0]+1);
		} else {
			trace_debug(&cRED,"Fallita abilitazione ubicazione %s [%c]",szCodiceUbicazione,szStato[0]+1);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
	}
}

void do_vuota_ubicazione                    (gpointer user_data, GtkButton *button)
{
	char szCodiceUbicazione[40];
	char szCodiceProdotto[40];
	PGresult *PGRes;
	GtkWidget *dlg =GTK_WIDGET(user_data);
	
	/* ricavo il codice ubicazione ed il prodotto */
	get_ubicazione_in_edit(dlg,szCodiceUbicazione);

	if(strlen(szCodiceUbicazione)!=0){
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update ubicazioni set codprod='' where ubicazione='%s';",szCodiceUbicazione);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Liberata ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
		} else {
			trace_debug(&cRED,"Fallita vuota ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
	}
}

void do_assegna_ubicazione (gpointer user_data, GtkButton *button)
{
	char szSQLCmd[1024];
	char szCodiceUbicazione[40];
	char szCodiceProdotto[40];
	PGresult *PGRes;
	GtkWidget *dlg =GTK_WIDGET(user_data);
	
	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_prodotto"))));
	get_ubicazione_in_edit(dlg,szCodiceUbicazione);

	if(strlen(szCodiceUbicazione)!=0){
		sprintf(szSQLCmd,"update ubicazioni set codprod='%s' where ubicazione='%s';",szCodiceProdotto,szCodiceUbicazione);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Assegnata ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
		} else {
			trace_debug(&cRED,"Fallito assegnamento ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
	}
}

/********************************************************************
************************ DLG_OPERATORI ******************************
********************************************************************/
void on_dlg_operatori_pb_elimina_clicked    (gpointer user_data, GtkButton *button)
{
	char szCodiceOperatore[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceOperatore,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI LA CANCELLAZIONE DELL'OPERATORE %s ?",szCodiceOperatore);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_elimina_operatore), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Cancella operatore");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_operatori_pb_modifica_clicked   (gpointer user_data, GtkButton *button)
{
	char szCodiceOperatore[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceOperatore,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI LA MODIFICA AI DATI DELL'OPERATORE %s ?",szCodiceOperatore);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_modifica_operatore), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Modifica dati operatore");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));

}

void on_dlg_operatori_pb_inserisci_clicked  (gpointer user_data, GtkButton *button)
{
	char szCodiceOperatore[40];
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	/* ricavo il codice ubicazione ed il prodotto */
	strcpy(szCodiceOperatore,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	gtk_label_printf(lb_msg,"CONFERMI L'INSERIMENTO DELL'OPERATORE %s ?",szCodiceOperatore);
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_insert_operatore), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg_message), "Inserimento operatore");
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
}

void on_dlg_operatori_pb_prev_clicked       (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szOperatoreInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	strcpy(szOperatoreInEdit,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	nIndex=GetKeyIndex(szOperatoreInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex>0){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_operatori(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_operatori_pb_next_clicked       (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szOperatoreInEdit[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	strcpy(szOperatoreInEdit,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	nIndex=GetKeyIndex(szOperatoreInEdit,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex<(GTK_CLIST(get_widget(main_window,"clist"))->rows)-1){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_operatori(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),+1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}

void do_modifica_operatore                         (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodOpe[128];
	
	strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodOpe)!=0){
		sprintf(szSQLCmd,"update operatori set opnmope='%s', opcgope='%s', opinope='%s', oploope='%s', opprope='%s', opcpope='%s', opnmtel='%s' where opcdope='%s';",
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_nome"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cognome"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_indirizzo"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_localita"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_provincia"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cap"))),
			gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_telefono"))),
			szCodOpe);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Modificato operatore [%s]",szCodOpe);
		} else {
			trace_debug(&cRED,"Fallita modifica operatore [%s]",szCodOpe);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")), GetKeyIndex(szCodOpe,GTK_CLIST(get_widget(main_window,"clist")),0,0));

		edit_dlg_operatori(dlg,szCodOpe);
	}
}

void do_insert_operatore                           (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodOpe[128];
	BOOL bOk=TRUE;
	
	strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodOpe)!=0){
		/* controllo che non sia presente un imballo avente il medesimo codice */
		PGRes=PGExecSQL(Cfg.nDebugVersion,"select opcdope from operatori where opcdope='%s';",szCodOpe);
		if(PQntuples(PGRes)){
			trace_debug(&cRED,"Fallito inserimento operatore [%s] : Codice gia' presente in anagrafica operatori",szCodOpe);
			bOk=FALSE;
		}
		PQclear(PGRes);

		if(bOk){
				sprintf(szSQLCmd,"insert into operatori (opcdope,opnmope,opcgope,opinope,oploope,opprope,opcpope,opnmtel) values ('%s','%s','%s','%s','%s','%s','%s','%s');"
				,szCodOpe
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_nome")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cognome")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_indirizzo")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_localita")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_provincia")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cap")))
				,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_telefono"))));
			
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
			if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
				trace_debug(&cGREEN,"Inserito operatore [%s]",szCodOpe);
			} else {
				trace_debug(&cRED,"Fallito inserimento operatore [%s]",szCodOpe);
			}
			PQclear(PGRes);

			RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey);

			clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")), GetKeyIndex(szCodOpe,GTK_CLIST(get_widget(main_window,"clist")),0,0));

			edit_dlg_operatori(dlg,szCodOpe);
		}
	} else {
		trace_debug(&cRED,"Fallito inserimento : Inserire il codice");
	}
}

void do_elimina_operatore                          (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodOpe[128];
	
	strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

	if(strlen(szCodOpe)!=0){
		sprintf(szSQLCmd,"delete from operatori where opcdope=trim('%s');",szCodOpe);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Eliminato operatore [%s]",szCodOpe);
		} else {
			trace_debug(&cRED,"Fallita cancellazione operatore [%s]",szCodOpe);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

		edit_dlg_operatori(dlg,szCodOpe);
	}
}


/********************************************************************
************************** DLG_OPESET *******************************
********************************************************************/

void on_dlg_opeset_pb_vuota_clicked         (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodSet[64];
	
	strcpy(szCodSet,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));

	if(strlen(szCodSet)!=0){
		sprintf(szSQLCmd,"update settori set operatore='' where settore='%s';",szCodSet);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Vuotato settore [%s]",szCodSet);
		} else {
			trace_debug(&cRED,"Fallita vuota settore [%s]",szCodSet);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","opeset",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodSet,GTK_CLIST(get_widget(main_window,"clist")),0,0));

		edit_dlg_opeset(dlg,szCodSet);
	}
}

void on_dlg_opeset_pb_assegna_clicked       (gpointer user_data, GtkButton *button)
{
	PGresult *PGRes;
	char szSQLCmd[1024];
	GtkWidget *dlg =GTK_WIDGET(user_data);
	char szCodSet[64];
	char szCodOpe[64];
	
	strcpy(szCodSet,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));
	strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_operatore"))));

	if(strlen(szCodOpe)!=0){
		sprintf(szSQLCmd,"update settori set operatore='%s' where settore='%s';",szCodOpe,szCodSet);
		
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			trace_debug(&cGREEN,"Assegnato operatore %s al settore %s",szCodOpe,szCodSet);
		} else {
			trace_debug(&cRED,"Fallito Assegnamento operatore %s al settore %s",szCodOpe,szCodSet);
		}
		PQclear(PGRes);

		RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","opeset",GTK_SELECTION_SINGLE,szActualKey);

		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodSet,GTK_CLIST(get_widget(main_window,"clist")),0,0));

		edit_dlg_opeset(dlg,szCodSet);
	}
}

void on_dlg_opeset_pb_prev_clicked          (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szCodiceSettore[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	strcpy(szCodiceSettore,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));

	nIndex=GetKeyIndex(szCodiceSettore,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex>0){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),--nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_opeset(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_opeset_pb_next_clicked          (gpointer user_data, GtkButton *button)
{
	int nIndex;
	char szCodiceSettore[64];
	char *pszKey;
	char szKey[64];
	GtkWidget *dlg=GTK_WIDGET(user_data);

	strcpy(szCodiceSettore,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));

	nIndex=GetKeyIndex(szCodiceSettore,GTK_CLIST(get_widget(main_window,"clist")),0,0);

	if(nIndex<(GTK_CLIST(get_widget(main_window,"clist"))->rows)-1){
		gtk_clist_get_text(GTK_CLIST(get_widget(main_window,"clist")),++nIndex,0,&pszKey);
		strcpy(szKey,pszKey);
		edit_dlg_opeset(dlg,szKey);
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),+1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}


/********************************************************************
************************** DLG_CONFIG *******************************
********************************************************************/

void on_dlg_config_pb_ok_clicked(gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));
	gtk_widget_destroy(GTK_WIDGET(user_data));
}


void on_dlg_config_pb_apply_clicked(gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));
}


/********************************************************************
************************** DLG_SEARCH *******************************
********************************************************************/

void on_dlg_search_pb_cerca_clicked         (gpointer user_data, GtkButton *button)
{
	int nIndex;
	int nColumn;
	int nStartRow=0;
	char szKey[64];
	BOOL bCaseSensitive;
	BOOL bRestart;
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	
	/* ricavo la chiave e la colonna su cui cercare */
	strcpy(szKey,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_key"))));
	nColumn=atoi(gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"sb_column"))));

	if(nColumn>(GTK_CLIST(clist)->columns)) return;

	/* ricavo le opzioni di ricerca */
	bCaseSensitive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"cb_casesensitive")));
	bRestart=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"cb_start")));

	/* determino la posizione nella clist da cui riprendere a cercare */
	if(!bRestart && (GTK_CLIST(clist)->selection)){
		/* riprendo la ricerca dalla riga successiva */
		nStartRow=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data)+1;
	}

	/*
	* se riparto dalla 1a riga disattivo il toggle_button per evitare di 
	* ciclare sempre sulla prima riga che incontro nella lista
	*/
	if (bRestart) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"cb_start")),FALSE);
	
	if((nIndex=search_key(szKey,GTK_CLIST(clist),nColumn-1,nStartRow,bCaseSensitive))>=0){
		clist_move_to_row(GTK_CLIST(clist),nIndex);
		gtk_label_printf(get_widget(dlg,"lb_msg"),"riga %d",nIndex+1);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"scansione terminata");
	}
}

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __copyright__[];
extern char __authors__[];

void do_about(void)
{
	GtkWidget *dlg = create_dlg_about();

	gtk_label_printf(get_widget(dlg,"lb_version"), __version__);
	gtk_label_printf(get_widget(dlg,"lb_customer"), __customer__);
	gtk_label_printf(get_widget(dlg,"lb_rcsid"), rcsid);
	gtk_label_printf(get_widget(dlg,"lb_authors"), __authors__);
	gtk_label_printf(get_widget(dlg,"lb_copyright"), __copyright__);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_widget_show(dlg);
}



void on_dlg_cedola_pb_ok_clicked            (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *sb_cedola=get_widget(dlg,"sb_cedola");
	int nCedola = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sb_cedola));

	gtk_widget_destroy(dlg);

	sprintf(szActualKey,"ronmced='%d'",nCedola);

	load_table(strcpy(szTableName,"ordini_storico"),szActualKey);

}

void on_dlg_edit_pb_next_clicked        (gpointer user_data, GtkButton *button)
{
	char szKey[256];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	int nRowSelected;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(clist->selection->data);
	}

	if(GetKeysList(szTableName,clist,nRowSelected+1,szKey)){
		UpdateDBEditDlg(dlg,szTableName,szKey);
		clist_update_selected_row(clist,1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
	}
}

void on_dlg_edit_pb_prev_clicked        (gpointer user_data, GtkButton *button)
{
	char szKey[256];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	int nRowSelected;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(GetKeysList(szTableName,clist,nRowSelected-1,szKey)){
		UpdateDBEditDlg(dlg,szTableName,szKey);
		clist_update_selected_row(clist,-1);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),FALSE);
	}
}

void on_dlg_edit_pb_add_clicked        (gpointer user_data, GtkButton *button)
{
	char szKey[256];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	int nRowSelected;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(GetKeysList(szTableName,clist,nRowSelected,szKey)){
		GtkWidget *dlg_message=create_dlg_message();
		GtkWidget *lb_msg=get_widget(dlg_message,"lb_msg");
		GtkWidget *pb_ok=get_widget(dlg_message,"pb_ok");

		gtk_label_printf(lb_msg,"CONFERMI L'INSERIMENTO ?");
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_edit_add_record), dlg);
		gtk_window_set_title (GTK_WINDOW (dlg_message), "Inserimento Record");
		gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
		gtk_widget_show(dlg_message);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
	}
}

void on_dlg_edit_pb_del_clicked        (gpointer user_data, GtkButton *button)
{
	char szKey[256];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	int nRowSelected;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(GetKeysList(szTableName,clist,nRowSelected,szKey)){
		GtkWidget *dlg_message=create_dlg_message();
		GtkWidget *lb_msg=get_widget(dlg_message,"lb_msg");
		GtkWidget *pb_ok=get_widget(dlg_message,"pb_ok");

		gtk_label_printf(lb_msg,"CONFERMI LA CANCELLAZIONE ?");
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_edit_delete_record), dlg);
		gtk_window_set_title (GTK_WINDOW (dlg_message), "Cancellazione Record");
		gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
		gtk_widget_show(dlg_message);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
	}
}

void on_dlg_edit_pb_mod_clicked        (gpointer user_data, GtkButton *button)
{
	char szKey[256];
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	int nRowSelected;

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(GetKeysList(szTableName,clist,nRowSelected,szKey)){
		GtkWidget *dlg_message=create_dlg_message();
		GtkWidget *lb_msg=get_widget(dlg_message,"lb_msg");
		GtkWidget *pb_ok=get_widget(dlg_message,"pb_ok");

		gtk_label_printf(lb_msg,"CONFERMI LA MODIFICA ?");
		g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_edit_update_record), dlg);
		gtk_window_set_title (GTK_WINDOW (dlg_message), "Modifica Record");
		gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
		gtk_widget_show(dlg_message);
		gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(dlg));
	}
}

void do_edit_add_record (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *lb_msg=get_widget(dlg,"lb_msg");
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	GtkWidget *entry_edit;
	int nRowSelected=0;
	int nIndex;
	int nKeyFields;
	int nDisplayFields;
	int nFieldCount;
	int nTuples;
	char szKey[1024];
	char szBuffer[1024];
	char szSQLCmd[1024];
	char szDBName[128];
	DBSTRUCT DB;
	PFIELD pField;
	BOOL bOK=TRUE;
	PGresult *PGRes = NULL;

	strcpy(szDBName,szTableName);

	if(GTK_CLIST(clist)==NULL){
		gtk_label_printf(lb_msg,"do_edit_add_record() : Errore di sistema");
		return ;
	}

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		gtk_label_printf(lb_msg,"Errore in lettura configurazione tabella [%s]",szDBName);
		return ;
	}
	nKeyFields=0;
	nDisplayFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & KEY_MASK){
			nKeyFields++;
		}
		if(pField->nFieldFlag & DISPLAY_MASK){
			nDisplayFields++;
		}
	}

	/* ricavo la chiave */
	nFieldCount=0;
	szKey[0]='\0';
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];

		if(pField->nFieldFlag & KEY_MASK){
			if(nFieldCount!=0){
				strcat(szKey," AND ");
			}
			sprintf(szBuffer,"entry_%s",pField->szFieldName);
			if((entry_edit = get_widget(dlg,szBuffer))){
				strcat(szKey,pField->szFieldName);
				strcat(szKey,"=");
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
				strcat(szKey,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
			}
			nFieldCount++;
		}
	}

	sprintf(szSQLCmd,"select * from %s where %s;",szDBName,szKey);


	/* cerco il record */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
	if (PQresultStatus(PGRes) != PGRES_TUPLES_OK){
		/* Errore nella select */
		gtk_label_printf(lb_msg,"Errore : Valore dei campi chiave non corretto");
		bOK=FALSE;
	}
	if((nTuples=PQntuples(PGRes))){
		/* trovato : c'e' qualcosa che non va */
		gtk_label_printf(lb_msg,"Errore : record con chiave non univoca");
		bOK=FALSE;
	}
	PQclear(PGRes);

	if(bOK){

		/* costruisco il comando di insert */
		sprintf(szSQLCmd,"insert into %s (",szDBName);

		nFieldCount=0;
		for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

			pField=DB.pFields[nIndex];


			if(pField->nFieldFlag & DISPLAY_MASK){
				if(nFieldCount!=0){
					strcat(szSQLCmd," ,");
				}
				strcat(szSQLCmd,pField->szFieldName);
				nFieldCount++;
			}
		}

		strcat(szSQLCmd,") values (");

		nFieldCount=0;
		for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){

			pField=DB.pFields[nIndex];

			if(pField->nFieldFlag & DISPLAY_MASK){
				if(nFieldCount!=0){
					strcat(szSQLCmd," ,");
				}
				sprintf(szBuffer,"entry_%s",pField->szFieldName);
				if((entry_edit = get_widget(dlg,szBuffer))){
					if(pField->cFieldType=='C' || pField->cFieldType=='D'){
						strcat(szSQLCmd,"'");
						strcat(szSQLCmd,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
						strcat(szSQLCmd,"'");
						NormalizeString(szSQLCmd);
					} else {
						strcat(szSQLCmd,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
					}
				}
				nFieldCount++;
			}
		}
		strcat(szSQLCmd,");");

		PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
		if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
			gtk_label_printf(lb_msg,"Inserito nuovo record");
		} else {
			gtk_label_printf(lb_msg,"Errore in inserimento");
			bOK=FALSE;
		}
		PQclear(PGRes);

	}
#ifndef ELIMINATO
	if(bOK){
		/* la refreshtable manda tutto in crash !!!!!! PERCHEEEE!? */
		RefreshTable(main_window,"sw_list","clist",szTableName,"lb_table_description","lb_table_rows","edit",szTableName,GTK_SELECTION_SINGLE,szActualKey);
		gtk_update();
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),nRowSelected);
		if(GetKeysList(szTableName,GTK_CLIST(get_widget(main_window,"clist")),nRowSelected,szKey)){
			UpdateDBEditDlg(dlg,szTableName,szKey);
		}
	}
#endif
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}
}

void do_edit_delete_record (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *lb_msg=get_widget(dlg,"lb_msg");
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	GtkWidget *entry_edit;
	int nRowSelected=0;
	int nRows;
	int nIndex;
	int nKeyFields;
	int nDisplayFields;
	int nFieldCount;
	char szKey[1024];
	char szBuffer[1024];
	char szSQLCmd[1024];
	char szDBName[128];
	DBSTRUCT DB;
	PFIELD pField;
	BOOL bOK=TRUE;
	PGresult *PGRes = NULL;

	strcpy(szDBName,szTableName);

	if(GTK_CLIST(clist)==NULL){
		gtk_label_printf(lb_msg,"do_edit_delete_record() : Errore di sistema");
		return ;
	}

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
		nRows=GTK_CLIST(clist)->rows;
	}

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		gtk_label_printf(lb_msg,"Errore in lettura configurazione tabella [%s]",szDBName);
		return ;
	}
	nKeyFields=0;
	nDisplayFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & KEY_MASK){
			nKeyFields++;
		}
		if(pField->nFieldFlag & DISPLAY_MASK){
			nDisplayFields++;
		}
	}

	/* ricavo la chiave */
	nFieldCount=0;
	szKey[0]='\0';
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];

		if(pField->nFieldFlag & KEY_MASK){
			if(nFieldCount!=0){
				strcat(szKey," AND ");
			}
			sprintf(szBuffer,"entry_%s",pField->szFieldName);
			if((entry_edit = get_widget(dlg,szBuffer))){
				strcat(szKey,pField->szFieldName);
				strcat(szKey,"=");
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
				strcat(szKey,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
			}
			nFieldCount++;
		}
	}

	sprintf(szSQLCmd,"delete from %s where %s;",szDBName,szKey);


	/* cancello il record */
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
	if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
		gtk_label_printf(lb_msg,"Cancellato record");
	} else {
		gtk_label_printf(lb_msg,"Errore in cancellazione");
		bOK=FALSE;
	}
	PQclear(PGRes);

#ifndef ELIMINATO
	if(bOK){
		/* cancello la riga */
		gtk_clist_remove(clist,nRowSelected);
		/* normalizzo nRowSelected */
		nRows=clist->rows;
		nRowSelected=min(nRowSelected,nRows-1);
		clist_update_selected_row(clist,nRowSelected);
		if(GetKeysList(szTableName,clist,nRowSelected,szKey)){
			UpdateDBEditDlg(dlg,szTableName,szKey);
		}
	}
#endif
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}
}

void do_edit_update_record (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *lb_msg=get_widget(dlg,"lb_msg");
	GtkCList *clist=GTK_CLIST(get_widget(main_window,"clist"));
	GtkWidget *entry_edit;
	int nRowSelected=0;
	int nIndex;
	int nKeyFields;
	int nDisplayFields;
	int nFieldCount;
	char szKey[1024];
	char szBuffer[1024];
	char szSQLCmd[1024];
	char szDBName[128];
	DBSTRUCT DB;
	PFIELD pField;
	BOOL bOK=TRUE;
	PGresult *PGRes = NULL;

	strcpy(szDBName,szTableName);

	if(GTK_CLIST(clist)==NULL){
		gtk_label_printf(lb_msg,"do_edit_update_record() : Errore di sistema");
		return ;
	}

	if((GTK_CLIST(clist)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(clist)->selection->data);
	}

	if(!ReadTableInfo(&DB,Cfg.szPathData,szDBName)){
		gtk_label_printf(lb_msg,"Errore in lettura configurazione tabella [%s]",szDBName);
		return ;
	}
	nKeyFields=0;
	nDisplayFields=0;
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];
		if(pField->nFieldFlag & KEY_MASK){
			nKeyFields++;
		}
		if(pField->nFieldFlag & DISPLAY_MASK){
			nDisplayFields++;
		}
	}

	/* ricavo la chiave */
	nFieldCount=0;
	szKey[0]='\0';
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		pField=DB.pFields[nIndex];

		if(pField->nFieldFlag & KEY_MASK){
			if(nFieldCount!=0){
				strcat(szKey," AND ");
			}
			sprintf(szBuffer,"entry_%s",pField->szFieldName);
			if((entry_edit = get_widget(dlg,szBuffer))){
				strcat(szKey,pField->szFieldName);
				strcat(szKey,"=");
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
				strcat(szKey,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
				if(pField->cFieldType=='C'){
					strcat(szKey,"'");
				}
			}
			nFieldCount++;
		}
	}

	sprintf(szSQLCmd,"update %s set ",szDBName);


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
				strcat(szSQLCmd,"=");
				if(pField->cFieldType=='C'){
					strcat(szSQLCmd,"'");
					strcat(szSQLCmd,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
					strcat(szSQLCmd,"'");
					NormalizeString(szSQLCmd);
				} else {
					strcat(szSQLCmd,gtk_entry_get_text(GTK_ENTRY(entry_edit)));
				}
			}
			nFieldCount++;
		}
	}

	sprintf(szBuffer," where %s;",szKey);
	strcat(szSQLCmd,szBuffer);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSQLCmd);
	if (PQresultStatus(PGRes) == PGRES_COMMAND_OK && atoi(PQcmdTuples(PGRes))==1){
		gtk_label_printf(lb_msg,"Aggiornato record");
	} else {
		gtk_label_printf(lb_msg,"Errore di aggiornamento");
		bOK=FALSE;
	}
	PQclear(PGRes);

#ifndef ELIMINATO
	if(bOK){
		/* la refreshtable manda tutto in crash !!!!!! PERCHEEEE!? */
		RefreshTable(main_window,"sw_list","clist",szTableName,"lb_table_description","lb_table_rows","edit",szTableName,GTK_SELECTION_SINGLE,szActualKey);
		gtk_update();
		clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),nRowSelected);
		if(GetKeysList(szTableName,GTK_CLIST(get_widget(main_window,"clist")),nRowSelected,szKey)){
			UpdateDBEditDlg(dlg,szTableName,szKey);
		}
	}
#endif
	for(nIndex=0;nIndex<DB.nFieldsNumber;nIndex++){
		free(DB.pFields[nIndex]);
	}
}

void do_print (gpointer user_data, GtkButton *button)
{
	PrintTable("edit",szTableName,Cfg.szPrinterConsole,szActualKey,NULL);
	gtk_widget_destroy(GTK_WIDGET(user_data));
}


/*------------*/
/* DLG_SELECT */
/*------------*/

char szSelectTable[256];
char szSelectCondition[256];

void do_dlg_select(void)
{
	GtkWidget *cb_field;
	GtkWidget *cb_expression;
	GtkWidget *txf_select;
	GList *items = NULL;
	PGresult *PGRes = NULL;
	int nFields;
	int nFieldIndex;

	dlg_select = create_dlg_select();

	txf_select=get_widget(dlg_select,"txf_select");

	
	/*
	* Reset Select 
	*/
	strcpy(szSelectCondition," ");
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);

	cb_expression=get_widget(dlg_select,"cb_expression");

	items = g_list_append (items, "Uguale a");
	items = g_list_append (items, "Maggiore di");
	items = g_list_append (items, "Maggiore o uguale di");
	items = g_list_append (items, "Minore di");
	items = g_list_append (items, "Minore o uguale di");
	items = g_list_append (items, "Diverso da");

	gtk_combo_set_popdown_strings (GTK_COMBO (cb_expression), items);

	g_list_free(items); items=NULL;

	cb_field=get_widget(dlg_select,"cb_field");

	PGRes=PGExecSQL(FALSE,"select * from %s where false;",szSelectTable);
	nFields=PQnfields(PGRes);
	for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
		items = g_list_append (items, PQfname(PGRes,nFieldIndex));
	}
	gtk_combo_set_popdown_strings (GTK_COMBO (cb_field), items);

	PQclear(PGRes);

	g_list_free(items); items=NULL;

	gtk_window_set_focus (GTK_WINDOW (dlg_select), get_widget(dlg_select,"pb_select_all"));
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_and"),FALSE);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_or"),FALSE);
	gtk_widget_show(dlg_select);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_select),GTK_WINDOW(main_window));
}



void on_dlg_select_pb_ok_clicked            (gpointer user_data, GtkButton *button)
{
	strcpy(szActualKey,szSelectCondition);
	RefreshTable(main_window,"sw_list","clist",szTableName,"lb_table_description","lb_table_rows","edit",szTableName,GTK_SELECTION_SINGLE,szActualKey);
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),FALSE);

	gtk_text_printf(NULL,txt_msgs,"Attivata selezione [%s]\n",szActualKey);
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

void on_dlg_select_pb_expand_clicked        (gpointer user_data, GtkButton *button)
{
	char szField[128];
	GtkWidget *cb_field=get_widget(dlg_select,"cb_field");
	GtkWidget *dlg_expand;
	GtkWidget *sw_expand;
	GtkWidget *lst;
	char szSelectCmd[256];

	dlg_expand = create_dlg_expand();

	gtk_window_set_focus (GTK_WINDOW (dlg_expand), get_widget(dlg_expand,"pb_ok"));
	gtk_widget_show(dlg_expand);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_expand),GTK_WINDOW(dlg_select));

	sw_expand=get_widget(dlg_expand,"sw_expand");

	strcpy(szField,gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(cb_field)->entry)));

	sprintf(szSelectCmd,"select %s,count(%s) from %s group by %s order by %s",szField,szField,szSelectTable,szField,szField);

	CreateListFromSelect(dlg_expand,sw_expand,&lst,"lst_expand",GTK_SELECTION_SINGLE,szSelectCmd);
}

void on_dlg_select_pb_and_clicked           (gpointer user_data, GtkButton *button)
{

	strcat(szSelectCondition," AND ");
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_and"),FALSE);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_or"),FALSE);
}

void on_dlg_select_pb_start_sel_clicked     (gpointer user_data, GtkButton *button)
{
	int nTuples;

	nTuples=CreateTable(dlg_select,"sw_select","lst_select",NULL,NULL,NULL,"edit",szTableName,GTK_SELECTION_SINGLE,szSelectCondition);
	if(nTuples){
		gtk_label_printf(get_widget(dlg_select,"lb_selection"),"Selezionate %d righe",nTuples);
	} else {
		gtk_label_printf(get_widget(dlg_select,"lb_selection"),"SELECT ERRATA");
	}
	/*
	* Reset Select Ordini e Lancio Ordini
	*/
	/* strcpy(szSelectCondition," "); */
}

void on_dlg_select_pb_reset_sel_clicked     (gpointer user_data, GtkButton *button)
{
	strcpy(szSelectCondition," ");
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);
}

void on_dlg_select_pb_select_all_clicked    (gpointer user_data, GtkButton *button)
{
	strcat(szSelectCondition,"TRUE");
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);
}

void on_dlg_select_pb_or_clicked            (gpointer user_data, GtkButton *button)
{
	strcat(szSelectCondition," OR ");
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_and"),FALSE);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_or"),FALSE);
}

void on_dlg_select_pb_stampa_clicked            (gpointer user_data, GtkButton *button)
{
	PrintTable("edit",szTableName,Cfg.szPrinterConsole,szSelectCondition,NULL);
}


void on_dlg_expand_pb_ok_clicked (gpointer user_data, GtkButton *button)
{
	char szField[128];
	char szExpression[128];
	char *szValue;
	GtkWidget *cb_field=get_widget(dlg_select,"cb_field");
	GtkWidget *cb_expression=get_widget(dlg_select,"cb_expression");
	GtkWidget *dlg_expand=(GtkWidget *)user_data;
	GtkWidget *lst_expand=get_widget(dlg_expand,"lst_expand");
	int nRowSelected;
	int nType;
	PGresult *PGRes = NULL;

	strcpy(szField,gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(cb_field)->entry)));
	strcpy(szExpression,gtk_entry_get_text (GTK_ENTRY (GTK_COMBO(cb_expression)->entry)));
	if(!strcmp(szExpression,"Uguale a")){
		strcpy(szExpression," = ");
	} else if(!strcmp(szExpression,"Maggiore di")){
		strcpy(szExpression," > ");
	} else if(!strcmp(szExpression,"Maggiore o uguale di")){
		strcpy(szExpression," >= ");
	} else if(!strcmp(szExpression,"Minore di")){
		strcpy(szExpression," < ");
	} else if(!strcmp(szExpression,"Minore o uguale di")){
		strcpy(szExpression," <= ");
	} else if(!strcmp(szExpression,"Diverso da")){
		strcpy(szExpression," != ");
	}

	/*
	* Ricavo il tipo del campo selezionato
	*/
	PGRes=PGExecSQL(FALSE,"select %s from %s limit 1;",szField,szSelectTable);
	nType=PQftype(PGRes,0);
	/*
	gtk_text_printf(NULL,txt_msgs,"Tipo : %d",nType);
	*/
	PQclear(PGRes);

	if((GTK_CLIST(lst_expand)->selection)){
		nRowSelected=GPOINTER_TO_INT(GTK_CLIST(lst_expand)->selection->data);
		/*
		* Costruzione Select
		*/
		strcat(szSelectCondition,szField);
		strcat(szSelectCondition,szExpression);
		strcat(szSelectCondition,"'");
		gtk_clist_get_text(GTK_CLIST(lst_expand),nRowSelected,0,&szValue);
		strcat(szSelectCondition,szValue);
		strcat(szSelectCondition,"'");
	}
	gtk_label_printf(get_widget(dlg_select,"lb_selection"),szSelectCondition);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_and"),TRUE);
	gtk_widget_set_sensitive(get_widget(dlg_select,"pb_or"),TRUE);

	gtk_widget_destroy(dlg_expand);
}

void load_table(char *pszTableName,char *szKey)
{
	char szMsg[256];
	char szBuffer[256];

	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),FALSE);
	gtk_widget_set_sensitive(get_widget(main_window,"pbm_edit"),FALSE);

	sprintf(szMsg,"Loading %s ...",pszTableName);
	gnome_appbar_set_status((GnomeAppBar *)get_widget(main_window,"ab_main"),szMsg);
	gtk_update();

	CreateTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit",pszTableName,GTK_SELECTION_SINGLE,szKey);

	gnome_appbar_set_status((GnomeAppBar *)get_widget(main_window,"ab_main"),"Ready");

	strcpy(szBuffer,pszTableName);
	strcat(szBuffer,"_name");
	xncGetFileString("edit",szBuffer, "", szSelectTable, sizeof(szSelectTable),Cfg.szCfgFile,NULL); 
}




