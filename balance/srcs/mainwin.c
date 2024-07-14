#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <picking.h>

#include <proc_list.h>
#include <msg-box.h>
#include <ep-about.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "editarticolo.h"
#include "balance.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

void on_lst_articoli_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    MainWindowPrivate *priv = main_window_get_instance_private (win);
    gboolean ok = FALSE;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista))
        ok = TRUE;

    gtk_widget_set_sensitive(priv->pb_edit,ok);
    gtk_widget_set_sensitive(priv->pb_lav,ok);
}

void on_lst_cedole_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    MainWindowPrivate *priv = main_window_get_instance_private (win);
    gboolean ok = FALSE;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista))
        ok = TRUE;

    gtk_widget_set_sensitive(priv->pb_articoli,ok);
}

void do_aggiorna_lista(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szSelectCmd[4096];

    GetFileString("Equilibratura", priv->szActualTable, "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName, NULL); 

    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    RefreshTable(priv->sw, "lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    GtkTreeSelection *selection = TREE_SELECTION_LST(lst);

    if(!strcmp(priv->szActualTable,"lista_cedole")){
        g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK (on_lst_cedole_selection_changed), win);
    } else if(!strcmp(priv->szActualTable,"lista_articoli")){
        g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK (on_lst_articoli_selection_changed), win);
    }
}

void do_lista_cedole(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    strcpy(priv->szActualTable,"lista_cedole");
    do_aggiorna_lista(win);

    /*
    gtk_widget_set_sensitive(priv->pb_articoli, FALSE);
    gtk_widget_set_sensitive(priv->pb_equilibra, FALSE);
    gtk_widget_set_sensitive(priv->pb_edit, FALSE);
    gtk_widget_set_sensitive(priv->pb_lav, FALSE);
    */
}

/*
* do_lista_articoli_cedola(gpointer win)
* lista degli articoli relativi agli ordini selezionati
* lavora sulla tabella sel_ord_tmp_[TIPO_ORDINI] (ordini selezionati precedentemente)
* e sel_art  
*/
void do_lista_articoli_cedola(gpointer win)
{
    DBresult *DBRes;
    DBresult *DBResArt;
    DBresult *DBResCat;
    DBresult *DBResUbi;
    DBresult *DBResUbiMan;
    DBresult *DBResUpdate;
    int nRowIndex;
    int nIndex;
    int nTuples;
    int nQTFFO;
    int nSWFFO;
    char *pszString;
    char szCedola[128];
    char szCDUBI[128];
    char szSTATO[128];
    char szCDFLG[128];
    int nCDSET;
    char szTPUBI[128];
    int nUbicazioniManuali;
    int nUbicazioniManualiUtilizzate;
    int nNMCPE;
    int nQTPCF;
    int nCPPCF;
    ep_bool_t bOK=TRUE;
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lb_msg=find_child(GTK_WIDGET(win),"lb_msg");
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");
    MainWindowPrivate *priv = main_window_get_instance_private (win);

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)){
        if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
            /* ho ottenuto l'iter */
            gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,Cfg.nCedoleKeyField, &pszString); 
            strcpy(szCedola,StrTrimAll(pszString));
            g_free(pszString);

            gtk_label_printf(priv->lb_status,"Lettura articoli cedola %s", szCedola);

            gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(priv->pr_status), (double) 0.2);
            gtk_update();

            /*
            * Seleziono le ubicazioni manuali per uso successivo
            * rm 09-10-2003 : solo quelle inutilizzate
            */
            DBResUbiMan=DBExecQuery(Cfg.nDebugLevel>1,"select ubcdubi,ubnmset,ubstato,ubcdflg from ubicazioni where ubcdflg='%c' and codprod='' order by fila(ubicazione),montante(ubicazione)::int2,colonna(ubicazione)::int2,piano(ubicazione)::int2;", UBICAZIONE_MANUALE);
            nUbicazioniManuali=DBntuples(DBResUbiMan);
            nUbicazioniManualiUtilizzate=0;


            /*
            * Tabella bal_art_tmp_[TIPO_ORDINI]
            *
            * nome    tipo          descrizione
            *
                bacdpro char(12),   - Codice Articolo
                bacdubi char(12),   - Codice Ubicazione 
                bacdset int2,       - Settore
                banmrgh int4,       - Numero Righe
                banmcpe int4,       - Numero Copie in ubicazione (escluso il preconfezionato e i bancali)
                banmcpt int4,       - Numero Copie totali
                bacdflg char(1),    - Flag Ubicazione - Manuale / Automatico 
                baswffo int2,       - Tipo FF (1/2/3) 
                baqtffo int2,       - Numero Copie per Fuori Formato
                baqtpcf int2,       - Numero Copie per Preconfezionato
                baqtpcp int2,       - Numero Copie per Preconfezionato Pallet
                bafcpcf char(1),    - Formato Collo per Preconezionato 
                bastato char(1)     - Stato ubicazione (Pallet/Scaffale Pallet_Disabilitata/Scaffale_Disabilitata)
                batpubi char(1)     - Tipo assegnamento in equilibratura  (Manuale/Automatico) 
            */

            gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(priv->pr_status), (double) 0.4);
            gtk_update();

            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table bal_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
            DBclear(DBRes);

            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table bal_art_tmp_%s_%s (\
                bacdpro text,\
                bacdubi text,\
                bacdset int2,\
                banmrgh int4,\
                banmcpe int4,\
                banmcpt int4,\
                bacdflg text,\
                baswffo int2,\
                baqtffo int2,\
                baqtpcf int2,\
                baqtpcp int2,\
                bafcpcf text,\
                bastato text,\
                batpubi char(1)\
            );", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
            if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
                bOK=FALSE;
            }
            DBclear(DBRes);

            if(bOK==FALSE){
                gtk_text_printf("RED",txt_msgs,"Errore in creazione tabella di equilibratura");
                return;
            }

            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create index bal_art_tmp_%s_%s_bacdpro on bal_art_tmp_%s_%s (bacdpro);", Cfg.szTmpSuffix, Cfg.szTipoOrdini, Cfg.szTmpSuffix, Cfg.szTipoOrdini);
            if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
                bOK=FALSE;
            }
            DBclear(DBRes);

            if(bOK==FALSE){
                gtk_text_printf("RED",txt_msgs,"Errore in creazione indice tabella di equilibratura");
                return;
            }


            /* righe totali / copie totali / copie da equilibrare */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into bal_art_tmp_%s_%s (bacdpro,banmrgh,banmcpt,banmcpe) select r.racdpro,count(r.ordprog),sum(r.raqtord),sum(r.raqtord) from ric_art r,ric_ord o where r.ordprog=o.ordprog and o.ordtipo='%s'  and o.ronmced='%s' group by racdpro;", Cfg.szTmpSuffix, Cfg.szTipoOrdini,Cfg.szTipoOrdini,szCedola); 
            if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
                bOK=FALSE;
            }
            DBclear(DBRes);

            /* copie da equilibrare - escludo pallet */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.racdpro,sum(r.raqtord) from ric_art r,ric_ord o where r.ordprog=o.ordprog and o.ordtipo='%s'  and o.ronmced='%s' and o.roswcol='1' group by racdpro;",Cfg.szTipoOrdini,szCedola); 
            nTuples=DBntuples(DBRes);
            for(nIndex=0;nIndex<nTuples;nIndex++){
                DBResArt=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set banmcpe=banmcpe-%d where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,atoi(DBgetvalue(DBRes,nIndex,1)),DBgetvalue(DBRes,nIndex,0)); 
                if (DBresultStatus(DBResArt) != DBRES_COMMAND_OK ) {
                    bOK=FALSE;
                }
                DBclear(DBResArt);
            }
            DBclear(DBRes);

            gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(priv->pr_status), (double) 0.6);
            gtk_update();
            /* 
            * copie da equilibrare - escludo pcf 
            * giro completo sulla lista righe d'ordine (esclusi gli ordini a bancale)
            */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.racdpro,r.raqtord,c.prqtpcf from ric_art r,ric_ord o,catalogo c where r.racdpro=c.prcdpro and c.prqtpcf!=0 and r.ordprog=o.ordprog and o.ordtipo='%s'  and o.roswcol!='1' and o.ronmced='%s';", Cfg.szTipoOrdini,szCedola); 
            nTuples=DBntuples(DBRes);
            for(nIndex=0;nIndex<nTuples;nIndex++){
                nNMCPE=atoi(DBgetvalue(DBRes,nIndex,1));
                nQTPCF=atoi(DBgetvalue(DBRes,nIndex,2));

                /* calcolo la qta in pcf (totale-resto pcf) */
                nCPPCF=(nNMCPE/nQTPCF)*nQTPCF;
                if(nCPPCF){
                    DBResArt=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set banmcpe=banmcpe-%d where bacdpro='%s';", Cfg.szTmpSuffix, Cfg.szTipoOrdini,nCPPCF,DBgetvalue(DBRes,nIndex,0)); 
                    if (DBresultStatus(DBResArt) != DBRES_COMMAND_OK ) {
                        bOK=FALSE;
                    }
                    DBclear(DBResArt);
                }

            }
            DBclear(DBRes);


            if(bOK==FALSE){
                gtk_text_printf("RED",txt_msgs,"Errore in inserimento dati in tabella di equilibratura");
                return;
            }


            gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(priv->pr_status), (double) 0.8);
            gtk_update();

            /* ricavo i dati da catalogo e ubicazioni */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select bacdpro from bal_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
            nTuples=DBntuples(DBRes);
            for(nIndex=0;nIndex<nTuples;nIndex++){

                DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubcdubi,ubnmset from ubicazioni where codprod='%s';", DBgetvalue(DBRes,nIndex,0));
                if(DBntuples(DBResUbi)){
                    strcpy(szCDUBI,DBgetvalue(DBResUbi,0,0));
                    nCDSET=atoi(DBgetvalue(DBResUbi,0,1));
                } else {
                    strcpy(szCDUBI,"");
                    nCDSET=0;
                }
                DBclear(DBResUbi);
                DBResCat=DBExecQuery(Cfg.nDebugLevel>1,"select prswffo,prqtffo,prqtpcf,prqtpcp,prfcpcf from catalogo where prcdpro='%s';", DBgetvalue(DBRes,nIndex,0));
                if(DBntuples(DBResCat)){
                    nSWFFO=atoi(DBgetvalue(DBResCat,0,0));
                    nQTFFO=atoi(DBgetvalue(DBResCat,0,1));
                    /* 
                    * fuori formato 
                    * metto in ubicazione manuale
                    */
                    if(nSWFFO){
                        if(nUbicazioniManualiUtilizzate+1<nUbicazioniManuali){
                            /* ubico in ubicazione manuale */
                            strcpy(szCDUBI, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,0));
                            nCDSET=atoi(DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,1));
                            strcpy(szSTATO, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,2));
                            strcpy(szCDFLG, DBgetvalue(DBResUbiMan,nUbicazioniManualiUtilizzate,3));
                            strcpy(szTPUBI, "M");

                            nUbicazioniManualiUtilizzate++;
                        } else {
                            strcpy(szCDUBI, "");
                            nCDSET=0;
                            strcpy(szSTATO, "");
                            strcpy(szCDFLG, "");
                            strcpy(szTPUBI, "");
                            gtk_text_printf("RED",txt_msgs,"Ubicazioni manuali insufficienti [%d]\n",nUbicazioniManuali);
                        }
                    } else {
                        strcpy(szSTATO, "P");
                        strcpy(szCDFLG, "A");
                        strcpy(szTPUBI, "A");
                    }


                    DBResUpdate=DBExecQuery(Cfg.nDebugLevel>1,"update bal_art_tmp_%s_%s set \
                        baswffo=%d, baqtffo=%d,\
                        baqtpcf=%d, baqtpcp=%d, bafcpcf='%s', bacdubi='%s', bacdset=%d, bastato='%s' ,\
                        bacdflg='%s', batpubi='%s' where bacdpro='%s';",

                        Cfg.szTmpSuffix,
                        Cfg.szTipoOrdini,
                        nSWFFO,                           /* SWFFO */
                        nQTFFO,                           /* QTFFO */
                        atoi(DBgetvalue(DBResCat,0,2)),   /* QTPCF */
                        atoi(DBgetvalue(DBResCat,0,3)),   /* QTPCP */
                        DBgetvalue(DBResCat,0,4),         /* FCPCF */
                        szCDUBI,                          /* CDUBI */
                        nCDSET,                           /* CDSET */
                        szSTATO,                          /* STATO */
                        szCDFLG,                          /* CDFLG */
                        szTPUBI,                          /* TPUBI */
                        DBgetvalue(DBRes,nIndex,0));
                    

                    DBclear(DBResUpdate);

                } else {
                    gtk_text_printf("RED",txt_msgs,"Articolo [%s] non presente in catalogo\n",DBgetvalue(DBRes,nIndex,0));
                }
                DBclear(DBResCat);
            }
            DBclear(DBRes);

            strcpy(priv->szActualTable,"lista_articoli");
            do_aggiorna_lista(win);

            gtk_widget_set_sensitive(priv->pb_equilibra,TRUE);
            gtk_widget_set_sensitive(priv->pb_articoli,FALSE);

            /* libero la DBResUbi */
            DBclear(DBResUbiMan);

            gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR(priv->pr_status), (double) 1.0);
            gtk_update();



        }
    }

}




void do_lista_articoli(gpointer win)
{
    GList *lista;
    MainWindowPrivate *priv = main_window_get_instance_private (win);
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)){
        int rc = dlg_msg( GTK_WINDOW(win), "Lista articoli cedola", GTK_MESSAGE_INFO,GTK_BUTTONS_OK_CANCEL, "Lista articoli cedola");
        switch(rc){
            case GTK_RESPONSE_OK:
                do_lista_articoli_cedola(win);
            break;
        }

    } else {
        dlg_msg( GTK_WINDOW(win), "Lista articoli cedola", GTK_MESSAGE_INFO,GTK_BUTTONS_OK_CANCEL, "Nessuna cedola selezionata");
    }
}


static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){        
        trace_debug_gtk("RED",TRUE,TRUE,NULL,"Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
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


static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    
    priv->app = _app;

    /* read configuration from ini file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * Verifico la presenza di una istanza attiva del processo 
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

            do_lista_cedole(win);

        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
    }
}

static void search_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean mode;

    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
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


/* TODO */
static void select_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_select_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst")));
}

/* TODO */
static void unselect_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_unselect_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst")));
}

static void refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_aggiorna_lista(win);
}

static void help_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
}

static void stampa_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance(win);

    int rc = dlg_msg(GTK_WINDOW(win), "Stampa", GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Stampa ?");

    switch (rc) {
        case GTK_RESPONSE_YES:
            PrintTable(priv->pszCfgFileName, "Equilibratura", "stampa_articoli", Cfg.szPrinterConsole, Cfg.szPathStampe, NULL, NULL, Cfg.nTipoStampe);
        break;
    }
}

static void articoli_activated  (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_lista_articoli(win);
}

static void cedole_activated    (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_lista_cedole(win);
}

static void equilibra_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    BalanceWindow *dlg;

    dlg = balance_window_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

static void conferma_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    /* TODO */
}

static void edit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditArticoloWindow *dlg;

    dlg = edit_articolo_window_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

static void lav_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    /* TODO */
}

static void lista_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    g_printf("Lista : activated\n");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw);
}

static void messaggi_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    g_printf("Messaggi : activated\n");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw_msgs);
}


static GActionEntry entries[] =
{
    { "quit",         quit_activated,      NULL, NULL, NULL },
    { "setup",        setup_activated,     NULL, NULL, NULL },
    { "search",       search_activated,    NULL, NULL, NULL },
    { "refresh",      refresh_activated,   NULL, NULL, NULL },
    { "about",        about_activated,     NULL, NULL, NULL },
    { "help",         help_activated,      NULL, NULL, NULL },
    { "stampa",       stampa_activated,    NULL, NULL, NULL },
    { "articoli",     articoli_activated,  NULL, NULL, NULL },
    { "cedole",       cedole_activated,    NULL, NULL, NULL },
    { "equilibra",    equilibra_activated, NULL, NULL, NULL },
    { "conferma",     conferma_activated,  NULL, NULL, NULL },
    { "edit",         edit_activated,      NULL, NULL, NULL },
    { "lav",          lav_activated,       NULL, NULL, NULL },

    { "lista",        lista_activated,     NULL, NULL, NULL },
    { "messaggi",     messaggi_activated,  NULL, NULL, NULL }
};









MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW(win));
}

static void main_window_init (MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    priv->pszAppName       = g_strdup (APP_NAME);
    priv->pszAppTitle      = g_strdup (APP_TITLE);
    priv->pszAppClass      = g_strdup (APP_CLASS);
    priv->nPID             = APP_PID;
    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

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
    MainWindowPrivate *priv = get_main_window_private_instance (object);
    MainWindow *win = MAIN_WINDOW (object);

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

    if(priv->pszAppTitle){       g_free(priv->pszAppTitle);      priv->pszAppTitle = NULL;    }
    if(priv->pszAppName){        g_free(priv->pszAppName);       priv->pszAppName = NULL;    }
    if(priv->pszAppClass){       g_free(priv->pszAppClass);      priv->pszAppClass = NULL;    }
    if(priv->pszCfgFileName){    g_free(priv->pszCfgFileName);   priv->pszCfgFileName = NULL;    }

    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("%s class init\n", WINDOW_NAME);

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_main);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_bar);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_msgs                  );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_status);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pr_status);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_stampa                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_articoli              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_cedole                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_equilibra             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_conferma_equilibratura);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_edit                  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_lav                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_config                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_exit                  );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_lista                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_messaggi              );

}

MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    g_printf("%s new\n", WINDOW_NAME);

    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);

    return win;
}
