#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>

#include <sql.h>
#include <sqlext.h>

#include <ep-common.h>
#include <ep-db.h>
#include <ep-odbc.h>

#include <picking.h>
#include <trace.h>
#include <pmx_msq.h>
#include <ep-about.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>
#include <msg-box.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
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

        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

void set_window_title(MainWindow *win)
{
    //gchar *pszTitle=g_strdup_printf("%s %s-%s - Fine Linea %s", __application_name__, __version__, __customer__, rcsid);
    gchar *pszTitle=g_strdup_printf("%s", __module_name__);
    gtk_window_set_title (GTK_WINDOW (win), pszTitle);
    g_free(pszTitle);
}

static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnect(Cfg.szPGHost, Cfg.szPGPort, Cfg.szPGDataBase))){
        trace_debug_gtk("RED",TRUE,TRUE,NULL,"Connection to database [%s:%s] failed.\n", Cfg.szPGHost, Cfg.szPGDataBase);
        dlg_msg(GTK_WINDOW(win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Connection to database [%s:%s] failed.", Cfg.szPGHost, Cfg.szPGDataBase);
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

    odbc_disconnect();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer){
        g_source_remove(Cfg.nMainTimer);
        Cfg.nMainTimer=0;
    }

    TerminateProcess(PROC_SOCKET_SERVER_SCANNER);

    TerminateProcess(PROC_SOCKET_SERVER_BILANCIA);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);

    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}


static void search_changed_callback (GtkSearchEntry *entry, gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");
    char szParagraph[128];
    char szBuffer[128];
    char szData[128];
    int nRowIndex=0;
    int nRiga=-1;
    int nKeyIndex=-1;
    char *pszString;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
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

    gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
    g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);

    /* read configuration from ini file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * creo la coda locale per l'applicazione
    */
    if(CreateProcessMsgQ(priv->nPID, 1)<0){
        trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
    }

    set_window_title(win);


    /* Database */
    if(init_db_connection(win)){
        ep_set_tipo_ordini(Cfg.szTipoOrdini);
        ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
        ep_refresh_table(win, (gchar *)"sw_list", (gchar *)"lst", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Colli", (gchar *)"lista_colli", Cfg.nTipoSelezione, NULL);

        /*
        * Timer per gestione eventi
        */
        Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

        /* lancio processi */
        /*
        RunSimpleProcess(PROC_BARCODE_BILANCIA, Cfg.szPathExe );
        RunSimpleProcess(PROC_BILANCIA,         Cfg.szPathExe );
        */

    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
    }
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

static void select_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_select_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst")));
}

static void unselect_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_tree_selection_unselect_all (TREE_SELECTION_LST(find_child(GTK_WIDGET(win),"lst")));
}

static void refresh_activated       (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_refresh_table(win, (gchar *)"sw_list", (gchar *)"lst", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Colli", (gchar *)"lista_colli", Cfg.nTipoSelezione, NULL);
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

void search_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gboolean mode;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
}

static GActionEntry entries[] =
{
    { "select-all",                    select_all_activated,                  NULL, NULL, NULL },
    { "unselect-all",                  unselect_all_activated,                NULL, NULL, NULL },
    { "setup",                         preferences_activated,                 NULL, NULL, NULL },
    { "refresh-lista-colli",           refresh_activated,                     NULL, NULL, NULL },
    { "about",                         about_activated,                       NULL, NULL, NULL },
    { "help",                          action_activated,                      NULL, NULL, NULL },
    { "uscita",                        quit_activated,                        NULL, NULL, NULL },

    { "search",                        search_activated,                      NULL, NULL, NULL },
    { "quit",                          quit_activated,                        NULL, NULL, NULL }
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

    MainWindow *win = MAIN_WINDOW (object);
    MainWindowPrivate *priv = get_main_window_private_instance (win);

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
    g_printf("main_window_class_init\n");

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_entry);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_plc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_plc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_barcode_bilancia);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_peso_rea);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_host_result);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pixmap_plc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  txt_msgs);
}

MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    priv->app = app;

    return win;
}
