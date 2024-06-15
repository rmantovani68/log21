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

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

gint     _settore;
gboolean _maximized;
gboolean _fullscreen;
MainWindow *_win;

struct _MainWindow
{
    GtkApplicationWindow parent;
};


G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);


static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){
        trace_debug(TRUE, TRUE, "Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        g_printf("Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
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
    char szBuffer[128];


    char szBufTrace[80];

    strcpy(szBufTrace, Cfg.szPathTrace);
    strcat(szBufTrace,"/");
    sprintf(szBuffer,"%s_%02d%s", priv->pszAppName, priv->pid-PROC_DISPLAY_00, TRACE_FILE_SUFFIX);
    strcat(szBufTrace,szBuffer);

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
    if(Cfg.nMainTimer){
        g_printf("remove timer\n");
        g_source_remove(Cfg.nMainTimer);
        Cfg.nMainTimer = 0;

        g_printf("delete queue\n");
        /* cancello la coda messaggi principale */
        DeleteProcessMsgQ(priv->pid);
    }


    trace_debug(TRUE, TRUE, "Stopped");

    g_printf("close trace\n");
    close_trace ();
    g_printf("exit\n");
    exit(0);
}

static void ExitApp(int received_signal)
{
    /* vengono ignorati altri arrivi del segnale */
    signal(received_signal, SIG_IGN);

    Exit(_win);

    /* ripristino del signal handler */
    signal(received_signal, ExitApp);
}


static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    /* read configuration from ini file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);
    priv->pszDisplayCfgFileName = g_strdup_printf("%s/%s", Cfg.szPathData, "display.cfg");

    init_trace(win);

    trace_debug(TRUE, TRUE, "Settore         : %d",priv->settore);
    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->pid);

    /*
    * Verifico la presenza di una istanza attiva del processo 
    * controllando la presenza della coda MSG
    */
    if( TRUE || OpenProcessMsgQ(priv->pid)<0){
        /*
        * coda messaggi non presente - tutto ok
        * creo la coda locale per l'applicazione
        */
        if(CreateProcessMsgQ(priv->pid, 1)<0){
            trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->pid);
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->pid);
        }

        gchar *pszTitle=g_strdup_printf("%s %s-%s - Display  %s - PID [%d] Settore [%d]", __application_name__, __version__, __customer__, rcsid, priv->pid, priv->settore);
        gtk_window_set_title (GTK_WINDOW (win), pszTitle);
        g_free(pszTitle);

        /* Database */
        if(init_db_connection(win)){

            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
    }
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

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static GActionEntry entries[] =
{
    { "quit",         quit_activated,      NULL, NULL, NULL }
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
    priv->nPID             = APP_PID+priv->settore;;
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
    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    g_print("main_window_class_init\n");

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_ordine_collo       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_ordine_collo              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_settore            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_settore                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stato_linea               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_operatore          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_codice_operatore          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_dati_operatore            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_operazione                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_copie                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_ubicazione         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_ubicazione                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_codice             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_codice                    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo_titolo             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_titolo                    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_tabella      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_ordini       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_colli        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_righe        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_copie        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_da_prelevare );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_ordini_da_prelevare );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_colli_da_prelevare  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_righe_da_prelevare  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_copie_da_prelevare  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_titolo_prelevati    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_ordini_prelevati    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_colli_prelevati     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_righe_prelevati     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_stats_copie_prelevati     );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  eb_stato_linea               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  eb_operazione                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  eb_copie                     );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_dati_in_alto           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_operazione             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_variabile              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_dati_prodotto          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_statistiche            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  frame_lista_colli            );
}

MainWindow *main_window_new (MainApp *app, gint settore, gboolean maximize, gboolean fullscreen)
{
    g_print("main_window_new\n");

    _settore    = settore;
    _maximized  = maximized;
    _fullscreen = fullscreen;

    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    priv->app   = app;
    
    _win = win;
    
    return win;
}
