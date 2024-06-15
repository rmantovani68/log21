#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>
#include <msg-box.h>
#include <ep-about.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_ordine.h"
#include "edit_imballo.h"
#include "edit_prodotto.h"
#include "edit_ubicazione.h"
#include "edit_operatore.h"
#include "edit_settore.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

void on_row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    g_printf("row-activated\n");
}

gboolean on_button_pressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
        g_print ("Single right click on the tree view.\n");

        /* optional: select row if no row is selected or only
        *  one other row is selected (will only do something
        *  if you set a tree selection mode as described later
        *  in the tutorial) */
        if (1) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

            /* Note: gtk_tree_selection_count_selected_rows() does not
            *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
            if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {
                GtkTreePath *path;

                /* Get tree path for row that was clicked */
                if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
                    gtk_tree_selection_unselect_all(selection);
                    gtk_tree_selection_select_path(selection, path);
                    gtk_tree_path_free(path);
                }
            }
        } /* end of optional bit */

        /* 
        * TODO: view_popup_menu(treeview, event, userdata); 
        */

        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;

}




static void edit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    
    char *pszKey;
    char szKey[256];
    int nRowSelected=0;
    gboolean ok = FALSE;


    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
        if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
            /* ho ottenuto l'iter */
            gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,0, &pszKey);
            strcpy(szKey,pszKey);
            g_free(pszKey);
            ok  = TRUE;
        }
    }

    if(ok){

        if(!strcmp(priv->szTableName,"ordini")){
            EditOrdineWindow *w;

            w = edit_ordine_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        } else if(!strcmp(priv->szTableName,"imballi")){
            EditImballoWindow *w;

            w = edit_imballo_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        } else if(!strcmp(priv->szTableName,"catalogo")){
            EditProdottoWindow *w;

            w = edit_prodotto_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        } else if(!strcmp(priv->szTableName,"ubicazioni")){
            EditUbicazioneWindow *w;

            w = edit_ubicazione_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        } else if(!strcmp(priv->szTableName,"operatori")){
            EditOperatoreWindow *w;

            w = edit_operatore_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        } else if(!strcmp(priv->szTableName,"settori")){
            EditSettoreWindow *w;

            w = edit_settore_window_new (MAIN_WINDOW (win));
            gtk_window_present (GTK_WINDOW (w));
        }

    }
    
#ifdef TODO
    if(!strcmp(priv->szTableName,"ordini")){

        GtkWidget *dlg_edit_ordine = create_dlg_edit_ordine();
        edit_dlg_ordini(dlg_edit_ordine,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_edit_ordine),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_edit_ordine);

    } else if(!strcmp(priv->szTableName,"ordini_storico")){

        GtkWidget *dlg_edit_ordine = create_dlg_edit_ordine();
        edit_dlg_ordini(dlg_edit_ordine,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_edit_ordine),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_edit_ordine);
    } else if(!strcmp(priv->szTableName,"opeset")){

        GtkWidget *dlg_opeset = create_dlg_opeset();
        edit_dlg_opeset(dlg_opeset,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_opeset),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_opeset);

    } else if(!strcmp(priv->szTableName,"ubicazioni")){

        GtkWidget *dlg_ubicazioni = create_dlg_ubicazioni();
        edit_dlg_ubicazioni(dlg_ubicazioni,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_ubicazioni),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_ubicazioni);

    } else if(!strcmp(priv->szTableName,"catalogo")){

        GtkWidget *dlg_catalogo = create_dlg_catalogo();
        edit_dlg_prodotto(dlg_catalogo,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_catalogo),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_catalogo);

    } else if(!strcmp(priv->szTableName,"imballi")){

        GtkWidget *dlg_imballi = create_dlg_imballi();
        edit_dlg_imballi(dlg_imballi,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_imballi),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_imballi);

    } else if(!strcmp(priv->szTableName,"operatori")){

        GtkWidget *dlg_operatori = create_dlg_operatori();
        edit_dlg_operatori(dlg_operatori,szKey);
        gtk_window_set_transient_for(GTK_WINDOW(dlg_operatori),GTK_WINDOW(main_window));
        gtk_widget_show(dlg_operatori);

    } else if(
        !strcmp(priv->szTableName,"nota_linee") ||
        !strcmp(priv->szTableName,"eventi") ||
        !strcmp(priv->szTableName,"storico_linee_spedizione") ||
        !strcmp(priv->szTableName,"storico_bancali") ||
        !strcmp(priv->szTableName,"storico_colli_bancale")
    ){
        GtkWidget *dlg = CreateDBEditDlg(priv->szTableName);
        gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
        gtk_widget_show(dlg);
        GetKeysList(priv->szTableName,clist,nRowSelected,szKey);
        UpdateDBEditDlg(dlg,priv->szTableName,szKey);
    }
#endif

}

#ifdef TODO

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
    DBResult *PGres;
    /* N.B. da fare la gestione dello storico */
    ep_bool_t bStorico=FALSE;
    if(!strcmp(priv->szTableName,"ordini_storico")){
        bStorico=TRUE;
    }

    gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_ORDPROG")),&pszCodiceOrdine);
    strcpy(szCodiceOrdine,pszCodiceOrdine);
    gtk_label_get(GTK_LABEL(get_widget(dlg,"lb_NMCOL")),&pszColloInEdit);
    nColloInEdit=atoi(pszColloInEdit);

    /* ricavo il numero di colli dell'ordine */
    if(bStorico){
        PGres=DBExecQuery(Cfg.nDebugLevel,"select ronmcll from ric_ord_stor where ordprog='%s';",szCodiceOrdine);
    } else {
        PGres=DBExecQuery(Cfg.nDebugLevel,"select ronmcll from ric_ord where ordprog='%s';",szCodiceOrdine);
    }

    if(DBntuples(PGres) && nColloInEdit<atoi(DBgetvalue(PGres,0,0))){
        edit_dlg_collo(dlg,szCodiceOrdine,++nColloInEdit);
        gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),TRUE);
        gtk_widget_set_sensitive(get_widget(dlg,"pb_prev"),TRUE);
    } else {
        gtk_widget_set_sensitive(get_widget(dlg,"pb_next"),FALSE);
    }
    DBclear(PGres);
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
    DBResult *DBRes;
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
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Modificato prodotto [%s]",szCodProd);
        } else {
            trace_debug(&cRED,"Fallita modifica su prodotto [%s]",szCodProd);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","catalogo",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

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
    DBResult *DBRes;
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

        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Modificato imballo [%s]",szCodImb);
        } else {
            trace_debug(&cRED,"Fallita modifica su imballo [%s]",szCodImb);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodImb,GTK_CLIST(get_widget(main_window,"clist")),0,0));

        edit_dlg_imballi(dlg,szCodImb);
    }
}


void do_aggiungi_imballo (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodImb[128];
    ep_bool_t bOk=TRUE;
    
    strcpy(szCodImb,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

    if(strlen(szCodImb)!=0){
        /* controllo che non sia presente un imballo avente il medesimo codice */
        DBRes=DBExecQuery(Cfg.nDebugLevel,"select codice from imballi where codice='%s';",szCodImb);
        if(DBntuples(DBRes)){
            trace_debug(&cRED,"Fallito inserimento imballo [%s] : Codice gia' presente in anagrafica imballi",szCodImb);
            bOk=FALSE;
        }
        DBclear(DBRes);

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
            
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
            if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
                trace_debug(&cGREEN,"Inserito imballo [%s]",szCodImb);
            } else {
                trace_debug(&cRED,"Fallito inserimento imballo [%s]",szCodImb);
            }
            DBclear(DBRes);

            RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

            clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodImb,GTK_CLIST(get_widget(main_window,"clist")),0,0));

            edit_dlg_imballi(dlg,szCodImb);
        }
    } else {
        trace_debug(&cRED,"Fallito inserimento : Inserire il codice");
    }
}


void do_elimina_imballo (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodImb[128];
    char *pszKey;
    int nRowSelected;
    
    strcpy(szCodImb,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

    if(strlen(szCodImb)!=0){
        sprintf(szSQLCmd,"delete from imballi where codice=trim('%s') and ordtipo='%s';",szCodImb,Cfg.szTipoOrdini);
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Eliminato imballo [%s]",szCodImb);
        } else {
            trace_debug(&cRED,"Fallita eliminazione imballo [%s]",szCodImb);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","imballi",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

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
    ep_bool_t bAbilitata;


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
    DBResult *DBRes;
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
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set cnistato='%c' where ubicazione='%s';",szStato[0]-1,szCodiceUbicazione);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Abilitata ubicazione %s [%c]",szCodiceUbicazione,szStato[0]-1);
        } else {
            trace_debug(&cRED,"Fallita abilitazione ubicazione %s [%c]",szCodiceUbicazione,szStato[0]-1);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

        edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
    }
}

void do_disabilita_ubicazione                    (gpointer user_data, GtkButton *button)
{
    char szCodiceUbicazione[40];
    char szStato[40];
    DBResult *DBRes;
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
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set cnistato='%c' where ubicazione='%s';",szStato[0]+1,szCodiceUbicazione);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Abilitata ubicazione %s [%c]",szCodiceUbicazione,szStato[0]+1);
        } else {
            trace_debug(&cRED,"Fallita abilitazione ubicazione %s [%c]",szCodiceUbicazione,szStato[0]+1);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

        edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
    }
}

void do_vuota_ubicazione                    (gpointer user_data, GtkButton *button)
{
    char szCodiceUbicazione[40];
    char szCodiceProdotto[40];
    DBResult *DBRes;
    GtkWidget *dlg =GTK_WIDGET(user_data);
    
    /* ricavo il codice ubicazione ed il prodotto */
    get_ubicazione_in_edit(dlg,szCodiceUbicazione);

    if(strlen(szCodiceUbicazione)!=0){
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set codprod='' where ubicazione='%s';",szCodiceUbicazione);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Liberata ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
        } else {
            trace_debug(&cRED,"Fallita vuota ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

        edit_dlg_ubicazioni(dlg,szCodiceUbicazione);
    }
}

void do_assegna_ubicazione (gpointer user_data, GtkButton *button)
{
    char szSQLCmd[1024];
    char szCodiceUbicazione[40];
    char szCodiceProdotto[40];
    DBResult *DBRes;
    GtkWidget *dlg =GTK_WIDGET(user_data);
    
    /* ricavo il codice ubicazione ed il prodotto */
    strcpy(szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_prodotto"))));
    get_ubicazione_in_edit(dlg,szCodiceUbicazione);

    if(strlen(szCodiceUbicazione)!=0){
        sprintf(szSQLCmd,"update ubicazioni set codprod='%s' where ubicazione='%s';",szCodiceProdotto,szCodiceUbicazione);
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Assegnata ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
        } else {
            trace_debug(&cRED,"Fallito assegnamento ubicazione %s [prodotto %s]",szCodiceUbicazione,szCodiceProdotto);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","ubicazioni",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

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
    DBResult *DBRes;
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
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Modificato operatore [%s]",szCodOpe);
        } else {
            trace_debug(&cRED,"Fallita modifica operatore [%s]",szCodOpe);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")), GetKeyIndex(szCodOpe,GTK_CLIST(get_widget(main_window,"clist")),0,0));

        edit_dlg_operatori(dlg,szCodOpe);
    }
}

void do_insert_operatore                           (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodOpe[128];
    ep_bool_t bOk=TRUE;
    
    strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

    if(strlen(szCodOpe)!=0){
        /* controllo che non sia presente un imballo avente il medesimo codice */
        DBRes=DBExecQuery(Cfg.nDebugLevel,"select opcdope from operatori where opcdope='%s';",szCodOpe);
        if(DBntuples(DBRes)){
            trace_debug(&cRED,"Fallito inserimento operatore [%s] : Codice gia' presente in anagrafica operatori",szCodOpe);
            bOk=FALSE;
        }
        DBclear(DBRes);

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
            
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
            if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
                trace_debug(&cGREEN,"Inserito operatore [%s]",szCodOpe);
            } else {
                trace_debug(&cRED,"Fallito inserimento operatore [%s]",szCodOpe);
            }
            DBclear(DBRes);

            RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

            clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")), GetKeyIndex(szCodOpe,GTK_CLIST(get_widget(main_window,"clist")),0,0));

            edit_dlg_operatori(dlg,szCodOpe);
        }
    } else {
        trace_debug(&cRED,"Fallito inserimento : Inserire il codice");
    }
}

void do_elimina_operatore                          (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodOpe[128];
    
    strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_codice"))));

    if(strlen(szCodOpe)!=0){
        sprintf(szSQLCmd,"delete from operatori where opcdope=trim('%s');",szCodOpe);
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Eliminato operatore [%s]",szCodOpe);
        } else {
            trace_debug(&cRED,"Fallita cancellazione operatore [%s]",szCodOpe);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","operatori",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),0);

        edit_dlg_operatori(dlg,szCodOpe);
    }
}


/********************************************************************
************************** DLG_OPESET *******************************
********************************************************************/

void on_dlg_opeset_pb_vuota_clicked         (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodSet[64];
    
    strcpy(szCodSet,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));

    if(strlen(szCodSet)!=0){
        sprintf(szSQLCmd,"update settori set operatore='' where settore='%s';",szCodSet);
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Vuotato settore [%s]",szCodSet);
        } else {
            trace_debug(&cRED,"Fallita vuota settore [%s]",szCodSet);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","opeset",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

        clist_update_selected_row(GTK_CLIST(get_widget(main_window,"clist")),GetKeyIndex(szCodSet,GTK_CLIST(get_widget(main_window,"clist")),0,0));

        edit_dlg_opeset(dlg,szCodSet);
    }
}

void on_dlg_opeset_pb_assegna_clicked       (gpointer user_data, GtkButton *button)
{
    DBResult *DBRes;
    char szSQLCmd[1024];
    GtkWidget *dlg =GTK_WIDGET(user_data);
    char szCodSet[64];
    char szCodOpe[64];
    
    strcpy(szCodSet,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_settore"))));
    strcpy(szCodOpe,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_operatore"))));

    if(strlen(szCodOpe)!=0){
        sprintf(szSQLCmd,"update settori set operatore='%s' where settore='%s';",szCodOpe,szCodSet);
        
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK && atoi(DBcmdTuples(DBRes))==1){
            trace_debug(&cGREEN,"Assegnato operatore %s al settore %s",szCodOpe,szCodSet);
        } else {
            trace_debug(&cRED,"Fallito Assegnamento operatore %s al settore %s",szCodOpe,szCodSet);
        }
        DBclear(DBRes);

        RefreshTable(main_window,"sw_list","clist","lb_table_name","lb_table_description","lb_table_rows","edit","opeset",GTK_SELECTION_SINGLE,szActualKey, NULL, NULL);

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

#endif

static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){
        trace_debug(TRUE, TRUE, "Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        dlg_msg(GTK_WINDOW(win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Connection to database [%s:%s] failed.", Cfg.szDBHost, Cfg.szDBName);
    }

    return rc;
}

static void close_db_connection(void)
{
    DBDisconnect();
}

static void init_trace(MainWindow *win)
{
#ifdef TRACE
    MainWindowPrivate *priv = get_main_window_private_instance (win);


    char szBufTrace[80];

    strcpy(szBufTrace, Cfg.szPathTrace);
    strcat(szBufTrace,"/");
    strcat(szBufTrace, priv->pszAppName);
    strcat(szBufTrace, TRACE_FILE_SUFFIX);

    open_trace(priv->pszAppName, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Module Name     : %s",priv->pszAppName);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
    trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(TRUE, TRUE, "Started");
#endif
}

static void Exit(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    close_db_connection();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer)
        g_source_remove(Cfg.nMainTimer);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);


    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}

static void search_changed_callback (GtkSearchEntry *entry, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    int nIndex;
    int nStartRow=0;
    char szKey[64];
    char *pszString;
    int nColumn=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_column));
    /* ricavo le opzioni di ricerca */
    ep_bool_t bCaseSensitive=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->cb_casesensitive));
    ep_bool_t bRestart=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(priv->cb_start));
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    
    /* ricavo la chiave e la colonna su cui cercare */
    const gchar *pSearch = gtk_entry_get_text (GTK_ENTRY(entry));
    int n = strlen(pSearch);

    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {

        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,nColumn, &pszString);
        
        if(!g_ascii_strcasecmp (pszString, pSearch)){
            /* found */
            gtk_tree_view_set_cursor(TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, FALSE);
            gtk_tree_view_scroll_to_cell (TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, TRUE, 0.5, 0.0);
            break;
        }

        g_free(pszString);

    } while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst), &iter));

#ifdef TODO

    if(nColumn>(GTK_CLIST(clist)->columns)) return;


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
#endif
}

static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    /* read configuration from cfg file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * Verifico la presenza di una istanza attiva del processo MAIN
    * controllando la presenza della coda MSG
    */
    if( TRUE || OpenProcessMsgQ(priv->nPID)<0){
        /*
        * coda messaggi non presente - tutto ok
        * creo la coda locale per l'applicazione
        */
        if(CreateProcessMsgQ(priv->nPID, 1)<0){
            trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        }

        gchar *pszTitle=g_strdup_printf("%s %s-%s - %s %s", __application_name__, __version__, __customer__, priv->pszAppTitle, rcsid);
        gtk_window_set_title (GTK_WINDOW (win), pszTitle);
        g_free(pszTitle);


        /* Database */
        if(init_db_connection(win)){

            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

            gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
            g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);


        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
            gtk_widget_destroy (GTK_WIDGET (win));
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
        gtk_widget_destroy (GTK_WIDGET (win));
        g_application_quit (G_APPLICATION (_app));
    }
}

static void find_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

}

static void cerca_activated    (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gboolean mode;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
}

void lista_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    g_printf("Lista\n");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->box_lista);
}

void messaggi_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    g_printf("Messaggi\n");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw_msgs);
}


static void setup_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{

}

static void about_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    w = about_window_new (GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);

    gtk_window_present (GTK_WINDOW (w));
}


static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    int rc=dlg_msg( GTK_WINDOW(win), "Exit", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Uscire dall'applicazione ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            Exit(win);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        break;
    }

}

static void stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}
static void ordini_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "ordini","TRUE", G_CALLBACK(on_selection_changed));
}
static void colli_scartati_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "colli_scartati","TRUE", G_CALLBACK(on_selection_changed));
}
static void settori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "settori","TRUE", G_CALLBACK(on_selection_changed));
}
static void operatori_settori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "opeset","TRUE", G_CALLBACK(on_selection_changed));
}
static void linee_spedizione_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "storico_linee_spedizione","TRUE", G_CALLBACK(on_selection_changed));
}
static void bancali_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "storico_bancali","TRUE", G_CALLBACK(on_selection_changed));
}
static void colli_bancali_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "storico_colli_bancale","TRUE", G_CALLBACK(on_selection_changed));
}
static void eventi_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "eventi","TRUE", G_CALLBACK(on_selection_changed));
}
static void ubicazioni_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "ubicazioni","TRUE", G_CALLBACK(on_selection_changed));
}
static void prodotti_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "catalogo","TRUE", G_CALLBACK(on_selection_changed));
}
static void imballi_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "imballi","TRUE", G_CALLBACK(on_selection_changed));
}
static void operatori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "operatori","TRUE", G_CALLBACK(on_selection_changed));
}
static void storico_ordini_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "ordini_storico","TRUE", G_CALLBACK(on_selection_changed));
}
static void nota_linee_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    load_table(win, "nota_linee","TRUE", G_CALLBACK(on_selection_changed));
}
static void aggiorna_lista_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}
static void help_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}
static void select_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}






static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}


static GActionEntry entries[] =
{
    { "cerca",                cerca_activated,              NULL, NULL, NULL },
    { "setup",                setup_activated,              NULL, NULL, NULL },
    { "about",                about_activated,              NULL, NULL, NULL },
    { "uscita",               quit_activated,               NULL, NULL, NULL },
    { "stampa",               stampa_activated,             NULL, NULL, NULL },
    { "ordini",               ordini_activated,             NULL, NULL, NULL },
    { "colli_scartati",       colli_scartati_activated,     NULL, NULL, NULL },
    { "operatori_settori",    operatori_settori_activated,  NULL, NULL, NULL },
    { "settori",              settori_activated,            NULL, NULL, NULL },
    { "linee_spedizione ",    linee_spedizione_activated,   NULL, NULL, NULL },
    { "bancali",              bancali_activated,            NULL, NULL, NULL },
    { "colli_bancali",        colli_bancali_activated,      NULL, NULL, NULL },
    { "eventi",               eventi_activated,             NULL, NULL, NULL },
    { "ubicazioni",           ubicazioni_activated,         NULL, NULL, NULL },
    { "prodotti",             prodotti_activated,           NULL, NULL, NULL },
    { "imballi",              imballi_activated,            NULL, NULL, NULL },
    { "operatori",            operatori_activated,          NULL, NULL, NULL },
    { "storico_ordini",       storico_ordini_activated,     NULL, NULL, NULL },
    { "nota_linee",           nota_linee_activated,         NULL, NULL, NULL },
    { "aggiorna_lista",       aggiorna_lista_activated,     NULL, NULL, NULL },
    { "help",                 help_activated,               NULL, NULL, NULL },
    { "edit",                 edit_activated,               NULL, NULL, NULL },
    { "stampa",               stampa_activated,             NULL, NULL, NULL },
    { "select",               select_activated,             NULL, NULL, NULL },
    { "find",                 find_activated,               NULL, NULL, NULL },

    { "lista",                lista_activated,              NULL, NULL, NULL },
    { "messaggi",             messaggi_activated,           NULL, NULL, NULL }

};

MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW (win));
}


static void main_window_init (MainWindow *win)
{
    g_printf("%s init\n", WINDOW_NAME);

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    priv->pszAppName          = g_strdup (APP_NAME);
    priv->pszAppTitle         = g_strdup (APP_TITLE);
    priv->pszAppClass         = g_strdup (APP_CLASS);
    priv->nPID                = APP_PID;
    priv->pszCfgFileName      = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    priv->pszEditCfgFileName  = g_strdup_printf("%s/%s", g_get_current_dir (), "edit.cfg");

    priv->screen = gdk_screen_get_default ();
    priv->provider = gtk_css_provider_new ();

    gchar *css_filename = g_strdup_printf("/org/%s/%s/%s.css", priv->pszAppClass, priv->pszAppName, priv->pszAppName);
    gtk_css_provider_load_from_resource (priv->provider, css_filename);
    g_free(css_filename);

    gtk_style_context_add_provider_for_screen (priv->screen, GTK_STYLE_PROVIDER(priv->provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);


    init_app(win);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void main_window_dispose (GObject *object)
{
    g_printf("%s dispose\n", WINDOW_NAME);

    MainWindowPrivate *priv = get_main_window_private_instance (object);
    MainWindow *win = MAIN_WINDOW (object);

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

    if(priv->pszAppTitle){           g_free(priv->pszAppTitle);          priv->pszAppTitle = NULL;    }
    if(priv->pszAppName){            g_free(priv->pszAppName);           priv->pszAppName = NULL;    }
    if(priv->pszAppClass){           g_free(priv->pszAppClass);          priv->pszAppClass = NULL;    }
    if(priv->pszCfgFileName){        g_free(priv->pszCfgFileName);       priv->pszCfgFileName = NULL;    }
    if(priv->pszEditCfgFileName){    g_free(priv->pszEditCfgFileName);   priv->pszEditCfgFileName = NULL;    }


    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("%s class init\n", WINDOW_NAME);

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, search_entry);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_table_name       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_table_description);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, lb_table_rows       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, txt_msgs            );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_main);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_msgs);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_lista);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, cb_casesensitive);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, cb_start        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, sb_column        );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_edit  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_print );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_select);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_search);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_config);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pb_exit  );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_print                      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_exit                       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_ordini                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_colli_scartati             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_storico_linee_di_spedizione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_storico_bancali            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_storico_colli_su_bancale   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_eventi                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_ubicazioni                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_prodotti                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_imballi                    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_operatori                  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_storico_ordini             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_nota_linee                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_edit                       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_about                      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow, pbm_help                       );

}                                                                                                             
                                                                                                              
MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    g_printf("main_window_new\n");
    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    priv->app = app;

    return win;
}
