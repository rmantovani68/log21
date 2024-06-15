#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <ep-common.h>
#include <picking.h>

#include <gtk-support.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainfun.h"
#include "mainwin.h"


G_DEFINE_TYPE_WITH_PRIVATE(MainApp, main_app, GTK_TYPE_APPLICATION);

MainAppPrivate *get_main_app_private_instance (gpointer app)
{
    return  main_app_get_instance_private (MAIN_APP(app));
}

static gboolean init_db_connection(MainApp *app)
{
    MainAppPrivate *priv = get_main_app_private_instance (app);
    gboolean rc;

    /*
    * connect to db
    */
    rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword);

    return rc;
}

static void close_db_connection(void)
{
    DBDisconnect();
}

static void init_trace(MainApp *app)
{
#ifdef TRACE
    MainAppPrivate *priv = get_main_app_private_instance (app);

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


static void init_app(MainApp *app)
{
    MainAppPrivate *priv = get_main_app_private_instance (app);

    /* read configuration from cfg file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(app);

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
            dlg_msg(NULL, "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            main_app_quit (app);
        }

        /* Database */
        if(init_db_connection(app)){
            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, app);

        } else {
            // dlg_msg(NULL, "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
            g_printf("Application error : error in database connection\n");
            // main_app_quit (app);
        }
    } else {
        // dlg_msg(NULL, "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
        main_app_quit (app);
    }
}

static void main_app_init (MainApp *app)
{
    g_printf("%s_init\n", APP_NAME);

    MainAppPrivate *priv = get_main_app_private_instance (app);

    priv->nPID             = APP_PID;
    priv->pszAppName       = g_strdup (APP_NAME);
    priv->pszAppClass      = g_strdup (APP_CLASS);
    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);
    priv->pszAppTitle      = g_strdup (APP_TITLE);

    priv->pszDocumentsPath = xdg_user_dir_lookup ("DOCUMENTS");
    priv->pszDownloadPath  = xdg_user_dir_lookup ("DOWNLOAD");
    priv->pszPicturesPath  = xdg_user_dir_lookup ("PICTURES");
    priv->pszDesktopPath   = xdg_user_dir_lookup ("DESKTOP");

    g_printf("Documents [%s]\n", priv->pszDocumentsPath);
    g_printf("Download  [%s]\n", priv->pszDownloadPath);
    g_printf("Pictures  [%s]\n", priv->pszPicturesPath);
    g_printf("Desktop   [%s]\n", priv->pszDesktopPath);

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    init_app(app);
}

static void main_app_startup (GApplication *app)
{
    g_printf("%s_startup\n", APP_NAME);

    MainAppPrivate *priv = get_main_app_private_instance (app);

    G_APPLICATION_CLASS (main_app_parent_class)->startup (app);
}

static void main_app_activate (GApplication *app)
{
    g_printf("%s_activate\n", APP_NAME);

    MainAppPrivate *priv = get_main_app_private_instance (app);
    priv->main_window = GTK_APPLICATION_WINDOW(main_window_new (GTK_APPLICATION(app)));
    gtk_application_add_window (GTK_APPLICATION(app), GTK_WINDOW(priv->main_window));
    gtk_window_present (GTK_WINDOW (priv->main_window));
}

static void main_app_class_init (MainAppClass *class)
{
    g_printf("%s_class_init\n", APP_NAME);

    G_APPLICATION_CLASS (class)->startup = main_app_startup;
    G_APPLICATION_CLASS (class)->activate = main_app_activate;
}


MainApp *main_app_new (void)
{
    MainApp *app;

    g_printf("%s_new\n", APP_NAME);

    gchar *application_id;
    application_id = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    return g_object_new (MAIN_APP_TYPE, "application-id", application_id, "flags", ep_get_glib_default_flags(), NULL);
    g_free(application_id);

    return app;
}

void main_app_quit (MainApp  *app)
{
    g_printf("%s_quit\n", APP_NAME);
    MainAppPrivate *priv = get_main_app_private_instance (app);

    close_db_connection();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer)
        g_source_remove(Cfg.nMainTimer);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);

    close_trace ();

    if(priv->settings){          
        g_clear_object (&(priv->settings)); 
        priv->settings=NULL;    
    }

    if(priv->pszAppName){        g_free(priv->pszAppName);       priv->pszAppName = NULL;    }
    if(priv->pszAppClass){       g_free(priv->pszAppClass);      priv->pszAppClass = NULL;    }
    if(priv->pszCfgFileName){    g_free(priv->pszCfgFileName);   priv->pszCfgFileName = NULL;    }

    /* xdg directories */
    if(priv->pszDocumentsPath){  g_free(priv->pszDocumentsPath); priv->pszDocumentsPath = NULL;    }
    if(priv->pszDownloadPath){   g_free(priv->pszDownloadPath);  priv->pszDownloadPath = NULL;    }
    if(priv->pszPicturesPath){   g_free(priv->pszPicturesPath);  priv->pszPicturesPath = NULL;    }
    if(priv->pszDesktopPath){    g_free(priv->pszDesktopPath);   priv->pszDesktopPath = NULL;    }

    if(priv->pszAppTitle){       g_free(priv->pszAppTitle);      priv->pszAppTitle = NULL;    }

    g_application_quit (G_APPLICATION (app));
}
