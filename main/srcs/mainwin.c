#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>
#include <picking.h>

#include <gtk-support.h>
#include <msg-box.h>
#include <ep-about.h>
#include <dbfun-gtk.h>

#include <proc_list.h>


#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "distribution.h"
#include "printdist.h"
#include "calcimballi.h"
#include "setstatoordine.h"
#include "spedizionedati.h"
#include "storicizzazionedati.h"
#include "caricosettori.h"
#include "stampaordinecollo.h"
#include "resetsettore.h"
#include "ordinifiltrati.h"
#include "cambiautente.h"
#include "select.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);


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

    /* rimuovo il timer per refresh statistica*/
    if(Cfg.nStatisticaTimer)
        g_source_remove(Cfg.nStatisticaTimer);

    /* 
    * Se la linea e' in start
    * rimuovo il timer per salvataggio stato settori 
    */
    if(priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_START && Cfg.nSaveStateTimer){
        g_source_remove(Cfg.nSaveStateTimer);
        Cfg.nSaveStateTimer = 0;
    }
#ifdef SPOSTARE_IN_FUNZIONE
    FreeTablesInfo();
#endif

    TerminateProcess(PROC_SETTORI);
    TerminateProcess(PROC_IOS);
    TerminateProcess(PROC_VOLUM);
    TerminateProcess(PROC_PRINTLABEL);
    TerminateProcess(PROC_RECEIVE);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);

    if(priv->pDatiLinea){
        DetachSharedMemory((char *)priv->pDatiLinea);

        if(!DeleteSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
            trace_debug(TRUE, TRUE, "Error Removing Shared Memory (%d)", Cfg.nShmKey);
        }
        priv->pDatiLinea=(PLINEA_STRUCT)NULL;
    }

    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}


static ep_bool_t do_vacuum_table(char *szTable,  GtkWidget *lb,GtkWidget *txt)
{
    ep_bool_t bOK=TRUE;
    DBresult *DBRes;

    trace_debug(TRUE, TRUE, "Aggiornamento %s in corso",szTable); 
    if(lb!=(GtkWidget *)NULL) {
        gtk_label_printf(lb,"Aggiornamento %s in corso",szTable); 
        gtk_update();
    }
    if(txt!=(GtkWidget *)NULL) {
        gtk_text_printf("GREEN",txt,"%-12.12s",szTable); 
        gtk_update();
    }
    DBRes=DBExecQuery(FALSE,"vacuum analyze %s;",szTable);
    if (DBresultStatus(DBRes) == DBRES_COMMAND_OK ) {
        if(lb!=(GtkWidget *)NULL) gtk_label_printf(lb,"Aggiornamento %s terminato",szTable);
        if(txt!=(GtkWidget *)NULL) gtk_text_printf("GREEN",txt,"... Terminato\n");
        trace_debug(TRUE, TRUE, "Aggiornamento %s terminato",szTable); 
    } else {
        if(lb!=(GtkWidget *)NULL) gtk_label_printf(lb,"Aggiornamento %s fallito\n\nContattare l'assistenza",szTable);
        if(txt!=(GtkWidget *)NULL) gtk_text_printf("RED",txt,"... Fallito, contattare l'assistenza\n");
        trace_debug(TRUE, TRUE, "Aggiornamento %s fallito",szTable); 
        bOK=FALSE;
    }
    DBclear(DBRes);

    return bOK;
}

void do_aggiorna_db(gpointer win)
{
    do_vacuum_table("col_prod",  NULL, NULL);
    do_vacuum_table("rig_prod",  NULL, NULL);
    do_vacuum_table("ric_ord",   NULL, NULL);
    do_vacuum_table("ric_art",   NULL, NULL);
    do_vacuum_table("ric_note",  NULL, NULL);
    do_vacuum_table("catalogo",  NULL, NULL);
    do_vacuum_table("ubicazioni",NULL, NULL);
}

static void do_delete_ricezione(gpointer win)
{
    DBresult *DBRes;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    trace_debug_gtk(NULL, TRUE,1,priv->txt_msgs,"Inizio cancellazione tabelle di ricezione");
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"truncate table ric_ord;");DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"truncate table ric_art;");DBclear(DBRes);
    trace_debug_gtk(NULL, FALSE,0,priv->txt_msgs,"... Terminata");

    refresh_lista_ordini(win);
}

static void do_delete_produzione(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    DBresult *DBRes;

    trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs,"Inizio cancellazione tabelle di produzione");
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"truncate table col_prod;");DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"truncate table rig_prod;");DBclear(DBRes);
    trace_debug_gtk(NULL, FALSE, FALSE, priv->txt_msgs,"... Terminata");
}

static void do_ricezione_dati(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    FILE *fp;
    char szMonitor[128];
    char szDateBuffer[128];
    char szTimeBuffer[128];
    int nCedola;
    DBresult *DBRes;

    if(priv->pDatiLinea->bRxData==FALSE){
        gtk_text_printf("RED",priv->txt_msgs,"Attenzione : nessun dato da ricevere\n");
        return;
    }

    sprintf(szMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szImportMonitor));

    /*
    * creo il monitor
    */
    if((fp=fopen(szMonitor,"w"))){
        fprintf(fp,"%s : %s %s",szMonitor, GetDate(szDateBuffer), GetTime(szTimeBuffer));

        fclose(fp);

    } else {
        gtk_text_printf("RED",priv->txt_msgs,"Fallita creazione monitor di import dati\n");
    }
}

static void do_reset_settori (void)
{
	SendMessage(PROC_SETTORI, PROC_MAIN, RESET_ALL, NULL);
}

static void do_calcolo_colli(void)
{
    SendMessage(PROC_VOLUM, PROC_MAIN, START_ANALISI_TEST, NULL);

}


static void search_changed_callback (GtkSearchEntry *entry, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
    GtkWidget *txt_msgs = priv->txt_msgs;
	char szParagraph[128];
	char szBuffer[128];
	char szData[128];
	int nRowIndex=0;
	int nRiga=-1;
	int nKeyIndex=-1;
	char *pszString;
    const gchar *pSearch = gtk_entry_get_text (GTK_ENTRY(entry));
    int n = strlen(pSearch);

	strcpy(szParagraph,"Tabella Ordini");
	strcpy(szBuffer,"lista_ordini");
	strcat(szBuffer,"_key");
	nKeyIndex=GetFileInt(szParagraph,szBuffer, 0, priv->pszCfgFileName,NULL); 


    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {

        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,nKeyIndex, &pszString);
        
        if(!g_ascii_strcasecmp (pszString, pSearch)){
            /* found */
            gtk_tree_view_set_cursor(TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, FALSE);
            gtk_tree_view_scroll_to_cell (TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, TRUE, 0.5, 0.0);
            break;
        }

        g_free(pszString);

    } while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst), &iter));

}

static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    priv->app = _app;

    /* read configuration from cfg file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

#ifdef SPOSTARE_IN_FUNZIONE

    /*
    * Lettura configurazione tabelle utilizzate
    */
    InsertTableInfo(&(priv->tCorriere),        Cfg.szPathData,"corriere");
    InsertTableInfo(&(priv->tCorrAgg),         Cfg.szPathData,"corr_agg");
    InsertTableInfo(&(priv->tCorrSDA),         Cfg.szPathData,"corr_sda");

    InsertTableInfo(&(priv->tOrdProd),         Cfg.szPathData,"ord_prod");
    InsertTableInfo(&(priv->tColProd),         Cfg.szPathData,"col_prod");
    InsertTableInfo(&(priv->tRigProd),         Cfg.szPathData,"rig_prod");
    InsertTableInfo(&(priv->tEvasi),           Cfg.szPathData,"evasi");
    InsertTableInfo(&(priv->tContenutoColli),  Cfg.szPathData,"contenuto_colli");
    InsertTableInfo(&(priv->tFlussoCedola),    Cfg.szPathData,"flusso_cedola");

    ReadTablesInfo();
#endif

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * Creo la shared memory per la gestione della linea
    */
    if((priv->pDatiLinea=(PLINEA_STRUCT)OpenSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
        trace_debug(TRUE, TRUE, "Cannot Open Shared Memory [%d] !", Cfg.nShmKey);
        g_printf("Cannot Open Shared Memory [%d]\n", Cfg.nShmKey);
        // dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Cannot Open Shared Memory [%d] !", Cfg.nShmKey);
        // gtk_widget_destroy (GTK_WIDGET (win));
        // g_application_quit (G_APPLICATION (_app));
    } else {
        priv->pDatiLinea->nStatoLinea=LINEA_IN_STOP;
        Cfg.nSaveStateTimer = 0;
    }
    gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
    g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);

    SetIOS(win, Cfg.bIOSConnected);

    SetMaster(win, Cfg.bMaster);

    gchar *pszTitle=g_strdup_printf("%s %s-%s - Gestione Impianto %s", __application_name__, __version__, __customer__, rcsid);
    gtk_window_set_title (GTK_WINDOW (win), pszTitle);
    g_free(pszTitle);

    /* Database */
    refresh_lista_ordini(win);

    /*
    * Refresh statistica 
    */
    StatisticaImpianto(win);

    /* 
    * TODO:
    * ------------------- 
    * spostare in mainapp 
    * ------------------- 
    */

    /*
    * Timer per gestione eventi
    */
    Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

    /*
    * Timer per statistica impianto
    */
    Cfg.nStatisticaTimer=g_timeout_add(Cfg.nStatisticaDelay, StatisticaImpianto, win);

    /*
    * Verifica cambio anno per gestione sequenze progressive ad azzeramento annuale
    */
    CheckAnno(Cfg.nDebugLevel);

    /* lancio processi */
    RunSimpleProcess(PROC_VOLUM,      Cfg.szPathExe );
    RunSimpleProcess(PROC_SETTORI,    Cfg.szPathExe );
    RunSimpleProcess(PROC_IOS,        Cfg.szPathExe );
    RunSimpleProcess(PROC_PRINTLABEL, Cfg.szPathExe );
    RunSimpleProcess(PROC_RECEIVE,    Cfg.szPathExe );
}

static void preferences_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{

}

static void distribution_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    DistrWindow *dlg;

    dlg = distr_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

static void calcolo_imballi_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CalcImballiWindow *dlg;

    dlg = calc_imballi_window_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

static void about_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;
    
    w = about_window_new (GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);

    gtk_window_present (GTK_WINDOW (w));
}


static void printdist_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    PrintDistWindow *dlg;

    dlg = print_dist_new(MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
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

static void calcolo_colli_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg( GTK_WINDOW(win), "Calcolo NUmero Colli", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Calcolo dei colli relativi agli ordini ricevuti");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_calcolo_colli();
        break;
    }
}

static void start_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    StartLinea(MAIN_WINDOW(win));
}

static void stop_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    StopLinea(MAIN_WINDOW(win));
}

static void select_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_select_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst_ordini")));
}

static void unselect_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_unselect_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst_ordini")));
}

static void reset_ordine_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GList *lista;
    GtkTreeIter iter;
    char *pszOrdine;
    char szOrdine[256];
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");

    int rc;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
        rc = dlg_msg(GTK_WINDOW(win), "Reset Ordini", GTK_MESSAGE_QUESTION ,GTK_BUTTONS_OK_CANCEL,"Confermi il reset degli ordini selezionati ?");
        switch (rc) {
            case GTK_RESPONSE_OK:
            {
                do {
                    if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
                        /* ho ottenuto l'iter */
                        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,Cfg.nOrdiniKeyField, &pszOrdine);
                        strcpy(szOrdine,pszOrdine);
                        g_free(pszOrdine);
                        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs,"ordine -> [%s]", szOrdine);
                        if(ResetOrdine(win, szOrdine)){
                            UpdateOrdine(win, szOrdine);
                        }
                    }
                } while((lista=g_list_next(lista)));
            }
            break;
        }
    }
    
    g_list_free(lista);
    
}
static void set_stato_ordine_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    char *pszOrdine;
    char szOrdine[256];
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");

    int rc;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
        SetStatoOrdineWindow *dlg;

        dlg = set_stato_ordine_window_new (MAIN_WINDOW (win));
        gtk_window_present (GTK_WINDOW (dlg));
    }
    
    g_list_free(lista);
}

static void carica_stato_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szFileName[128];
    struct stat filestat;

    sprintf(szFileName,"%s/settori.stato",Cfg.szPathData);

    if( FileExists(szFileName)){
        stat(szFileName,&filestat);

        /* newline included in ctime() output */
        printf(" File [%s] \naccess  time %s\nmodify  time %s\nchanged time  %s\n", szFileName, ctime(&filestat.st_atime) , ctime(&filestat.st_mtime) , ctime(&filestat.st_ctime) );

        int rc=dlg_msg( GTK_WINDOW(win), "Carica stato settori", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi Caricamento dello stato settori \nData : %s ?", ctime(&filestat.st_mtime));
        switch(rc){
            case GTK_RESPONSE_OK:
                SendMessage(PROC_SETTORI, PROC_MAIN, RESTORE_STATE, szFileName);
            break;
        }
    } else {
        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "file [%s]  di stato settori non presente", szFileName);
    }

}

static void salva_stato_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    char szFileName[128];
    struct stat filestat;

    sprintf(szFileName,"%s/settori.stato",Cfg.szPathData);

    stat(szFileName,&filestat);

    int rc=dlg_msg( GTK_WINDOW(win), "Salvataggio stato settori", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi il salvataggio dello stato dei settori ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            SendMessage(PROC_SETTORI, PROC_MAIN, SAVE_STATE, szFileName);
        break;
    }
}

static void carica_ultimo_stato_salvato_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szFileName[128];
    struct stat filestat;

    sprintf(szFileName,"%s/settori.stato.save",Cfg.szPathData);

    if( FileExists(szFileName)){
        stat(szFileName,&filestat);

        /* newline included in ctime() output */
        printf(" File [%s] access  time %s", szFileName, ctime(&filestat.st_atime) );
        printf(" File [%s] modify  time %s", szFileName, ctime(&filestat.st_mtime) );
        printf(" File [%s] changed time %s", szFileName, ctime(&filestat.st_ctime) );

        int rc=dlg_msg( GTK_WINDOW(win), "Caricamento ultimo stato salvato settori", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi caricamento ultimo stato salvato settori \nData : %s ?", ctime(&filestat.st_mtime));
        switch(rc){
            case GTK_RESPONSE_OK:
                SendMessage(PROC_SETTORI, PROC_MAIN, RESTORE_STATE, szFileName);
            break;
        }
    } else {
        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "file [%s]  di stato settori non presente", szFileName);
    }
}

static void pulizia_dati_ricezione_activated      (GSimpleAction *action, GVariant *parameter, gpointer win) 
{
    int rc=dlg_msg(GTK_WINDOW(win), "Cancellazione Tabelle di Ricezione", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi la cancellazione\ndelle tabelle di ricezione ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_delete_ricezione(win);
        break;
    }
}


static void pulizia_dati_produzione_activated     (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg(GTK_WINDOW(win), "Cancellazione Tabelle di Produzione", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi la cancellazione\ndelle tabelle di produzione ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_delete_produzione(win);
        break;
    }
}


static void aggiorna_archivi_activated            (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    int rc=dlg_msg(GTK_WINDOW(win), "Aggiornamento Tabelle", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL, "Confermi l'aggiornamento \ndelle tabelle di produzione ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabelle in corso...");
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella colli");                do_vacuum_table("col_prod",  NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella righe di produzione");  do_vacuum_table("rig_prod",  NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella ordini");               do_vacuum_table("ric_ord",   NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella righe ricevute");       do_vacuum_table("ric_art",   NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella note ricevute");        do_vacuum_table("ric_note",  NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella catalogo");             do_vacuum_table("catalogo",  NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabella ubicazioni");           do_vacuum_table("ubicazioni",NULL, NULL);
            trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "Aggiornamento tabelle terminato");
        break;
    }

}

static void refresh_activated       (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    refresh_lista_ordini(win);
}

static void ricezione_dati_activated       (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg(GTK_WINDOW(win), "Ricezione Dati", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi la ricezione dati  ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_ricezione_dati(win);
        break;
    }
}

static void spedizione_dati_activated             (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    SpedizioneDatiWindow *dlg;

    dlg = spedizione_dati_new (MAIN_WINDOW (win), SPEDIZIONE_DATI);
    gtk_window_present (GTK_WINDOW (dlg));
}

static void invia_file_ftp_activated              (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szCommand[256];
    sprintf(szCommand,"spedisci_consuntivo.sh");

    gboolean result;
    gchar *standard_out;
    gchar *standard_err;
    gint exit_state;
    GError *err;
    
    int rc=dlg_msg(GTK_WINDOW(win), "Invio file a server", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi l'invio FTP dei dati  ?");
    switch(rc){
        case GTK_RESPONSE_OK:
            if ((result = g_spawn_command_line_sync(szCommand, &standard_out, &standard_err, &exit_state, &err))==FALSE) {
                trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, "errore in esecuzione comando [%s]", szCommand);
            } else {
                trace_debug_gtk(NULL, TRUE, 1, priv->txt_msgs, standard_out);
            }
        break;
    }
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_log (NULL,G_LOG_LEVEL_DEBUG, "Action : activated");
}

static gboolean check_storicizzazione(gpointer win)
{
	DBresult *DBRes;
	int nNumeroRecords;
	int nNumeroRecordsKO;
    gboolean ret_value=FALSE;

	DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini);
	nNumeroRecords=DBntuples(DBRes);
	DBclear(DBRes);

	if(nNumeroRecords){
		DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and roflimc='%c';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini, CARICHI_ORDINE_DA_IMPORTARE);
		nNumeroRecordsKO=DBntuples(DBRes);
		DBclear(DBRes);
		if(nNumeroRecordsKO){
			/*
			* Visualizza messaggio di errore
			*/
            dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,
                "Tra gli ordini da storicizzare ce ne sono alcuni ancora non importati nel programma carichi\nSe il programma carichi non e' gia' attivo, farlo partire\nQuindi aspettare 5 minuti e riprovare a storicizzare.");

		} else {
			DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and rofliex not in ('%c','%c');",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini,EMAIL_SPEDITA_XAB,EMAIL_NON_SPEDITA_XAB);
			nNumeroRecordsKO=DBntuples(DBRes);
			DBclear(DBRes);
			if(nNumeroRecordsKO){
				/*
				* Visualizza messaggio di errore
				*/
                dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,
                    "Tra gli ordini da storicizzare ce ne sono alcuni per i quali non è stata prodotta l'XAB elettronica\nChiamare il supporto tecnico per verificare il problema");
			} else {
                ret_value = TRUE;
                // gtk_label_printf(priv->lb_msg,"%d Ordini da storicizzare\nPremere Ok per confermare la storicizzazione",nNumeroRecords);
            }
		}

	} else {
		/*
		* Visualizza messaggio di errore
		*/
        dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Non sono presenti Ordini da storicizzare");
	}
    return ret_value;
}

static void storicizzazione_dati_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    if(check_storicizzazione(win)){
        StoricizzazionedatiWindow *dlg;

        dlg = storicizzazione_dati_new (MAIN_WINDOW (win));
        gtk_window_present (GTK_WINDOW (dlg));
    }
}

static void carico_settori_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CaricoSettoriWindow *dlg;

    dlg = carico_settori_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

static void ordini_filtrati_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    OrdiniFiltratiWindow *dlg;

    dlg = ordini_filtrati_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}









static void do_stampa_ordine_collo(gpointer win,STAMPA_ORDINE_COLLO_TYPE_ENUM type, GtkTreeSelectionForeachFunc func)
{
	GList *lista;
	GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
	int rc;
    char szOrdine[128];
    gboolean show_dlg_stampa_ordine_collo = FALSE;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gchar *titolo, *question;

    szOrdine[0]='\0';

	lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
	if(!lista){
        show_dlg_stampa_ordine_collo = TRUE;
		/* nessun ordine selezionato */
	} else {
		if(lista && g_list_length(lista)==0){
			/* nessun ordine selezionato */
            show_dlg_stampa_ordine_collo = TRUE;
		}
		if(lista && g_list_length(lista)==1){
			/* un ordine selezionato */
            show_dlg_stampa_ordine_collo = TRUE;
            // Cerco l'iter nella lista
            if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
                char *pPtr;
                gtk_tree_model_get_text(TREE_MODEL_LST(lst), &iter, Cfg.nOrdiniKeyField, &pPtr);
                trace_debug_gtk(NULL, TRUE,1,priv->txt_msgs,"ordine -> [%s]", pPtr);
                strcpy(szOrdine, pPtr);
                g_free(pPtr);
            }
		} else {
			/* più ordini selezionati */
		}
	}
    if(show_dlg_stampa_ordine_collo){
        StampaOrdineColloWindow *dlg;

        dlg = stampa_ordine_collo_window_new (MAIN_WINDOW (win), type, szOrdine);
        gtk_window_present (GTK_WINDOW (dlg));
    } else {
        switch(type){
            case STAMPA_ETICHETTE_ORDINE:
                titolo = g_strdup("Stampa Etichette");
                question = g_strdup("Confermi la stampa delle etichette degli ordini selezionati ?");
            break;
            case STAMPA_XAB_ORDINE:
                titolo = g_strdup("Stampa XAB Ordini");
                question = g_strdup("Confermi la stampa delle etichette degli ordini selezionati ?");
            break;
            case STAMPA_DETTAGLIO_ORDINE:
                titolo = g_strdup("Stampa Dettaglio Ordini");
                question = g_strdup("Confermi la stampa delle etichette degli ordini selezionati ?");
            break;
            case STAMPA_DATI_ORDINE:
                titolo = g_strdup("Stampa Dati Ordini");
                question = g_strdup("Confermi la stampa delle etichette degli ordini selezionati ?");
            break;
            case STAMPA_PACKING_LIST_ORDINE:
                titolo = g_strdup("Stampa Packing List Ordini");
                question = g_strdup("Confermi la stampa delle packing list degli ordini selezionati ?");
            break;
            default:
                titolo = g_strdup("NON SUPPORTATA");
                question = g_strdup("NON SUPPORTATA");
            break;
        }
        rc = dlg_msg(GTK_WINDOW(win), titolo, GTK_MESSAGE_ERROR,GTK_BUTTONS_OK_CANCEL,question);
        switch (rc) {
            case GTK_RESPONSE_OK:
            {
                gtk_tree_selection_selected_foreach (TREE_SELECTION_LST(lst), func, (gpointer)win );
            }
            break;
        }
        g_free(titolo);
        g_free(question);
    }
	
	g_list_free(lista);
}


static void lancia_ordine_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gchar *ptr;
    char szBuffer[128];
    int nOrdini=0;

    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);
    if(!LanciaOrdine(win, ptr,FALSE)){
        trace_debug_gtk("RED", TRUE, TRUE, priv->txt_msgs, "Errore in lancio ordine [%s]",ptr);
    } else {
        nOrdini++;
        SendMessage(PROC_VOLUM, PROC_MAIN, ANALISI_ORDINE, ptr);
    }

    g_free(ptr);
}

static void stampa_etichette_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    gchar *ptr;
    char szBuffer[128];
    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);
    sprintf(szBuffer,"%s,%s", ptr,Cfg.szLabelPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC,szBuffer);
    g_free(ptr);
}

void stampa_xab_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    gchar *ptr;
    char szBuffer[128];
    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);
    sprintf(szBuffer,"%s,%s", ptr,Cfg.szXABPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB,szBuffer);
    g_free(ptr);
}

void stampa_dati_ordine_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    gchar *ptr;
    char szBuffer[128];
    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);
    sprintf(szBuffer,"%s,%s", ptr,Cfg.szPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DATI_ORDINE,szBuffer);
    g_free(ptr);
}

void stampa_dettagli_spedizione_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    gchar *ptr;
    char szBuffer[128];
	DBresult *DBRes;

    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into stampe (CODICE_TIPO_STAMPA, NOME_STAMPANTE, STRINGA_PARAMETRI, PRIORITA, STATO) values ('%d', '%s', '%s', 10, 'R');",
		COD_STAMPA_SPEDIZIONE,
		Cfg.szPrinterConsole,
		ptr);

	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
        trace_debug(TRUE, TRUE,  "Fallita insert in stampe [%d-%s-%s]",COD_STAMPA_SPEDIZIONE, Cfg.szPrinterConsole, ptr);
	}
	DBclear(DBRes);

    g_free(ptr);
}

void stampa_packing_list_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
    gchar *ptr;
    char szBuffer[128];
    gtk_tree_model_get_text(model, iter, Cfg.nOrdiniKeyField, &ptr);
    sprintf(szBuffer,"%s,%s", ptr, Cfg.szPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB_A4,szBuffer);
    g_free(ptr);
}

static void stampa_etichette_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_ordine_collo(win,STAMPA_ETICHETTE_ORDINE, stampa_etichette_callback);
}

void stampa_xab_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_ordine_collo(win,STAMPA_XAB_ORDINE, stampa_xab_callback);
}

void stampa_dati_ordine_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_ordine_collo(win,STAMPA_DATI_ORDINE, stampa_xab_callback);
}

void stampa_dettagli_spedizione_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_ordine_collo(win,STAMPA_DETTAGLIO_ORDINE, stampa_dettagli_spedizione_callback);
}

void stampa_packing_list_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_stampa_ordine_collo(win,STAMPA_PACKING_LIST_ORDINE, stampa_packing_list_callback);
}

void reset_settori_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg( GTK_WINDOW(win), "Reset Settori", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Reset Settori ?");
    switch(rc){
        case GTK_RESPONSE_YES:
			do_reset_settori();
        break;
    }
}

void reset_settore_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    SettoreWindow *dlg;

    dlg = main_app_reset_settore_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

void lancio_ordini_selezionati_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
	GList *lista;
	GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
	int rc;
    char szOrdine[128];
    gboolean show_dlg_stampa_ordine_collo = FALSE;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gchar *titolo, *question;

    szOrdine[0]='\0';

	lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
	if(lista && g_list_length(lista)){
        /* almeno un ordine selezionato */
        rc = dlg_msg(GTK_WINDOW(win), "Lancia Ordini", GTK_MESSAGE_ERROR,GTK_BUTTONS_OK_CANCEL, "Confermi lancio ordini selezionati ?");
        switch (rc) {
            case GTK_RESPONSE_OK:
            {
                gtk_tree_selection_selected_foreach (TREE_SELECTION_LST(lst), lancia_ordine_callback, (gpointer)win );
            }
            break;
        }
    }
	
	g_list_free(lista);
}


void rimanenza_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szSelectCmd[4096];
    GetFileString("rimanenza","rimanenza", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 
    int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                    "Rimanenza", 
                    GTK_MESSAGE_INFO, 
                    GTK_BUTTONS_CLOSE, 
                    szSelectCmd, 
                    GTK_SELECTION_SINGLE, 
                    NULL, 
                    "Rimanenza per articolo");
}

void cambio_utente_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CambiaUtenteWindow *dlg;

    dlg = cambia_utente_new (MAIN_WINDOW (win), CAMBIA_UTENTE);
    gtk_window_present (GTK_WINDOW (dlg));
}

void select_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    SelectWindow *dlg;

    dlg = main_app_select_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (dlg));
}

void cambio_password_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    CambiaUtenteWindow *dlg;

    dlg = cambia_utente_new (MAIN_WINDOW (win), CAMBIA_PASSWORD);
    gtk_window_present (GTK_WINDOW (dlg));
}

void search_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean mode;
#ifdef TODO
    /* esperimento con overlay e revealer */
    static int count=0;
    GtkRevealer *revealer;


    switch(count){
        case 0: revealer = GTK_REVEALER(priv->revealer_main_0); break;
        case 1: revealer = GTK_REVEALER(priv->revealer_main_1); break;
        case 2: revealer = GTK_REVEALER(priv->revealer_main_2); break;
        case 3: revealer = GTK_REVEALER(priv->revealer_main_3); break;
    }
    if(!mode){
        gtk_widget_hide (GTK_WIDGET(revealer));
    } else {
        gtk_widget_show (GTK_WIDGET(revealer));
    }
    gtk_revealer_set_reveal_child (revealer, !mode);

    if(!mode)
        count = (count+1) % 4;
#endif
    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);

}

void lista_ordini_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
     g_log (NULL,G_LOG_LEVEL_DEBUG, "Lista Ordini");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw_list_ordini);
}

void messaggi_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
     g_log (NULL,G_LOG_LEVEL_DEBUG, "Messaggi");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw_msgs);
}

void impianto_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
     g_log (NULL,G_LOG_LEVEL_DEBUG, "Impianto");

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->sw_impianto);
}




static GActionEntry entries[] =
{
    { "carica-stato",                  carica_stato_activated,                NULL, NULL, NULL },
    { "salva-stato",                   salva_stato_activated,                 NULL, NULL, NULL },
    { "carica-ultimo-stato-salvato",   carica_ultimo_stato_salvato_activated, NULL, NULL, NULL },
    { "pulizia-dati-ricezione",        pulizia_dati_ricezione_activated,      NULL, NULL, NULL },
    { "pulizia-dati-produzione",       pulizia_dati_produzione_activated,     NULL, NULL, NULL },
    { "aggiorna-archivi",              aggiorna_archivi_activated,            NULL, NULL, NULL },
    { "ricezione-dati",                ricezione_dati_activated,              NULL, NULL, NULL },
    { "spedizione-dati",               spedizione_dati_activated,             NULL, NULL, NULL },
    { "invia-file-ftp",                invia_file_ftp_activated,              NULL, NULL, NULL },
    { "storicizzazione-dati",          storicizzazione_dati_activated,        NULL, NULL, NULL },
    { "uscita",                        quit_activated,                        NULL, NULL, NULL },
    { "select-all",                    select_all_activated,                  NULL, NULL, NULL },
    { "unselect-all",                  unselect_all_activated,                NULL, NULL, NULL },
    { "reset-stato-ordine",            reset_ordine_activated,                NULL, NULL, NULL },
    { "set-stato-ordine",              set_stato_ordine_activated,            NULL, NULL, NULL },
    { "preferences",                   preferences_activated,                 NULL, NULL, NULL },
    { "refresh-lista-ordini",          refresh_activated,                     NULL, NULL, NULL },
    { "carico-settori",                carico_settori_activated,              NULL, NULL, NULL },
    { "stampa-etichette-ordine",       stampa_etichette_activated,            NULL, NULL, NULL },
    { "stampa-packing-list-ordine",    stampa_packing_list_activated,         NULL, NULL, NULL },
    { "stampa-xab",                    stampa_xab_activated,                  NULL, NULL, NULL },
    { "stampa-dati-ordine",            stampa_dati_ordine_activated,          NULL, NULL, NULL },
    { "stampa-dettagli-spedizione",    stampa_dettagli_spedizione_activated,  NULL, NULL, NULL },
    { "stampa-distribuzione",          printdist_activated,                   NULL, NULL, NULL },
    { "start-impianto",                start_activated,                       NULL, NULL, NULL },
    { "stop-impianto",                 stop_activated,                        NULL, NULL, NULL },
    { "reset-settori",                 reset_settori_activated,               NULL, NULL, NULL },
    { "reset-settore",                 reset_settore_activated,               NULL, NULL, NULL },
    { "test-ios",                      action_activated,                      NULL, NULL, NULL },
    { "lancio-ordini-selezionati",     lancio_ordini_selezionati_activated,   NULL, NULL, NULL },
    { "cambia-priorita",               action_activated,                      NULL, NULL, NULL },
    { "distribuzione",                 distribution_activated,                NULL, NULL, NULL },
    { "ordini-filtrati",               ordini_filtrati_activated,             NULL, NULL, NULL },
    { "calcolo-colli",                 calcolo_colli_activated,               NULL, NULL, NULL },
    { "consumo-imballi",               calcolo_imballi_activated,             NULL, NULL, NULL },
    { "rimanenza",                     rimanenza_activated,                   NULL, NULL, NULL },
    { "cambio-utente",                 cambio_utente_activated,               NULL, NULL, NULL },
    { "cambio-password",               cambio_password_activated,             NULL, NULL, NULL },
    { "about",                         about_activated,                       NULL, NULL, NULL },
    { "help",                          action_activated,                      NULL, NULL, NULL },

    { "start",                         start_activated,                       NULL, NULL, NULL },
    { "stop",                          stop_activated,                        NULL, NULL, NULL },
    { "select",                        select_activated,                      NULL, NULL, NULL },
    { "setup",                         preferences_activated,                 NULL, NULL, NULL },
    { "search",                        search_activated,                      NULL, NULL, NULL },
    { "distribuzione",                 distribution_activated,                NULL, NULL, NULL },
    { "filter",                        ordini_filtrati_activated,             NULL, NULL, NULL },
    { "exit",                          quit_activated,                        NULL, NULL, NULL },

    { "lista_ordini",                  lista_ordini_activated,                NULL, NULL, NULL },
    { "messaggi",                      messaggi_activated,                    NULL, NULL, NULL },
    { "impianto",                      impianto_activated,                    NULL, NULL, NULL }



};

MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW(win));
}

static void main_window_init (MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
     g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s - %s", __FILE__, __FUNCTION__, WINDOW_NAME);

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

     g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s - %s", __FILE__, __FUNCTION__, WINDOW_NAME);

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
     g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s - %s", __FILE__, __FUNCTION__, WINDOW_NAME);

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_start                        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_stop                         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_search                       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  impianto_menu                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_carica_stato                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_salva_stato                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_carica_ultimo_stato_salvato );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_master                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_stato_linea              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stato_linea                  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  separator_1                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  i0                              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  i1                              );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_pulizia_dati_ricezione      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_pulizia_dati_produzione     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_ricezione_dati               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_spedizione_dati             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_storicizzazione_dati        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_configurazione              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pbm_setta_stato_ordine          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_rx                       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_rx                           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_tx                       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_tx                           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  i2                              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  i5                              );

#ifdef TODO
    /* esperimento con overlay e revealer */
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  revealer_main_0  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  revealer_main_1  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  revealer_main_2  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  revealer_main_3  );
#endif

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_entry);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_main);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_ordini);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_msgs);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_impianto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  txt_msgs);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_lista_ordini);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_messaggi);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_impianto);
}

MainWindow *main_window_new (MainApp *app)
{
    _app = app;

    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s - %s", __FILE__, __FUNCTION__, WINDOW_NAME);

    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);

    return win;
}
