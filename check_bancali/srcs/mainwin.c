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
#include <msg-box.h>
#include <ep-about.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "gestione_linee.h"
#include "fine_linea.h"
#include "chiusura_bancale.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"


G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

MainWindowPrivate *internal_main_window_get_instance_private (MainWindow *win)
{
    return main_window_get_instance_private (win);
}


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

        // TODO view_popup_menu(treeview, event, userdata);

        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;

}

static void entry_barcode_activated_cb (GtkEntry *entry, gpointer *win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));
    char szBarcodeCollo[128];

    strcpy(szBarcodeCollo, gtk_entry_get_text(GTK_ENTRY(entry)));
    g_printf("entry_barcode_activated_cb(%s) %d %d %s\n", szBarcodeCollo, priv->nFase, priv->nPID, priv->pszCfgFileName);

    /* inserito barcode collo da tastiera */
    do_barcode_collo(win, szBarcodeCollo);
    gtk_entry_set_text(GTK_ENTRY(entry),"");
}



static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){
        trace_debug_gtk("RED",TRUE,TRUE,NULL,"Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        g_printf("Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        // dlg_msg(GTK_WINDOW(win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Connection to database [%s:%s] failed.", Cfg.szDBHost, Cfg.szDBName);
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
    MainWindowPrivate *priv = main_window_get_instance_private (win);


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
    MainWindowPrivate *priv = main_window_get_instance_private (win);

    close_db_connection();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer)
        g_source_remove(Cfg.nMainTimer);

    TerminateProcess(PROC_BARCODE_BANCALI);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);


    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}


static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (win);

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

        gchar *pszTitle=g_strdup_printf("%s %s-%s - Controllo Bancali %s", __application_name__, __version__, __customer__, rcsid);
        gtk_window_set_title (GTK_WINDOW (win), pszTitle);
        g_free(pszTitle);

        /* Database */
        if(init_db_connection(win)){
            /*
            * Apertura del canale di comunicazione con il PLC server (QPLC)
            */
            if(Cfg.nPLC){
                if(!(plc_connect(Cfg.szPLCHost, Cfg.nPLCService))){
                    dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in PLC connection");
                } 
            }
			g_signal_connect (priv->entry_barcode, "activate" , G_CALLBACK (entry_barcode_activated_cb), win);

            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

            RunSimpleProcess(PROC_BARCODE_BANCALI, Cfg.szPathExe );


        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
            // gtk_widget_destroy (GTK_WIDGET (win));
            // g_application_quit (G_APPLICATION (_app));
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
        gtk_widget_destroy (GTK_WIDGET (win));
        g_application_quit (G_APPLICATION (_app));
    }
}

static void gestione_linee_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GestioneLineeWindow *w;

    w = gestione_linee_window_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (w));
}

static void preferences_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
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
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));

    int rc=dlg_msg( GTK_WINDOW(win), "Exit", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Uscire dall'applicazione ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            Exit(win);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        break;
    }

}

static void inizio_linea_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg( GTK_WINDOW(win), "Inizio Linea", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Inizio Linea");
    switch(rc){
        case GTK_RESPONSE_OK:
            inizio_linea(win);
        break;
    }
}

static void seleziona_linea_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_name(sw, "sw_linee");

    char szSelectCmd[4096];

    GetFileString("check_bancali","lista_linee_attive", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName, NULL); 

    GList *PS=NULL;

    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);

    RefreshTable(sw, "lst_linee", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);

    GtkWidget *dlg = dlg_msg_with_child( GTK_WINDOW(win), "Seleziona Linea", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL, sw, TRUE, "Seleziona Linea");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_OK:
            attiva_linea(win, sw);
        break;
    }
    gtk_widget_destroy(dlg);
}

static void fine_linea_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    FineLineaWindow *w;

    w = fine_linea_window_new (MAIN_WINDOW (win));
    gtk_window_present (GTK_WINDOW (w));
}

static void apertura_bancale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    int rc=dlg_msg( GTK_WINDOW(win), "Apertura Bancale", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Apertura Bancale");
    switch(rc){
        case GTK_RESPONSE_OK:
            do_apertura_bancale(win);
        break;
    }
}

static void chiusura_bancale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));
    ChiusuraBancaleWindow *w;

    w = chiusura_bancale_window_new (MAIN_WINDOW (win), priv->Bancale.key);
    gtk_window_present (GTK_WINDOW (w));
}

void stampa_linea_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));


    GtkWidget *w = gtk_spin_button_new_with_range (0, 999999, 1);
    gtk_widget_set_name(w, "sb_linea");

    GtkWidget *dlg = dlg_msg_with_child(GTK_WINDOW(win), "Stampa Linea", GTK_MESSAGE_OTHER, GTK_BUTTONS_OK_CANCEL, w, FALSE, "Inserire il codice progressivo\ndella linea da stampare");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_OK:
        {
            int nKey = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));

            stampa_linea(win, nKey);
        }
        break;
    }
    gtk_widget_destroy(dlg);
}

void stampa_bancale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (MAIN_WINDOW(win));


    GtkWidget *w = gtk_spin_button_new_with_range (0, 999999, 1);
    gtk_widget_set_name(w, "sb_bancale");

    GtkWidget *dlg = dlg_msg_with_child(GTK_WINDOW(win), "Stampa Bancale", GTK_MESSAGE_OTHER, GTK_BUTTONS_OK_CANCEL, w, FALSE, "Inserire il codice progressivo\ndel bancale da stampare");
    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    switch(rc){
        case GTK_RESPONSE_OK:
        {
            int nKey = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(w));

            stampa_bancale(win, nKey,TRUE,FALSE);
        }
        break;
    }
    gtk_widget_destroy(dlg);
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static GActionEntry entries[] =
{
    { "about",                         about_activated,                       NULL, NULL, NULL },
    { "help",                          action_activated,                      NULL, NULL, NULL },
    { "setup",                         preferences_activated,                 NULL, NULL, NULL },
    { "exit",                          quit_activated,                        NULL, NULL, NULL },
    { "inizio_linea",                  inizio_linea_activated,                NULL, NULL, NULL },
    { "fine_linea",                    fine_linea_activated,                  NULL, NULL, NULL },
    { "apertura_bancale",              apertura_bancale_activated,            NULL, NULL, NULL },
    { "chiusura_bancale",              chiusura_bancale_activated,            NULL, NULL, NULL },
    { "stampa_linea",                  stampa_linea_activated,                NULL, NULL, NULL },
    { "stampa_bancale",                stampa_bancale_activated,              NULL, NULL, NULL },
    { "seleziona_linea",               seleziona_linea_activated,             NULL, NULL, NULL },
    { "gestione_linee",                gestione_linee_activated,              NULL, NULL, NULL }
};

static void main_window_init (MainWindow *win)
{
    MainWindowPrivate *priv = main_window_get_instance_private (win);

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
    MainWindow *win = MAIN_WINDOW (object);
    MainWindowPrivate *priv = main_window_get_instance_private (win);

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

    g_printf("main_window_dispose\n");

    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("main_window_class_init\n");

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  txt_msgs             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_msg               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_linee_titolo      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bancali_titolo    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_colli_titolo      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lntpspe           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lncdlin           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_key_linea         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lnnmced           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lnnmbnc           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lnnmcll           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lnpsrea           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_lnpspre           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bntpspe           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bncdlin           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cdspe_tit         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_key_bancale       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_nmcll_tit         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_psnet_tit         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bnnmcll           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bnpsrea           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_bnpspre           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_barcode           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cpnmcol           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cptpfor           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cpnmrgh           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cpnmcpe           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cppsrea           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cppspre           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cptpspe           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_cpcdlin           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_ordprog           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_key_collo         );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  entry_barcode        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_linee             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_bancali           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_colli             );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_inizio_linea      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_fine_linea        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_apertura_bancale  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_chiusura_bancale  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_stampa_linea      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_stampa_bancale    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_config            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_select_linea      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_linee             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_exit              );
}                                                                                                             
                                                                                                              
MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    g_printf("main_window_new\n");
    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = main_window_get_instance_private (win);
    priv->app = app;

    return win;
}
