#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <gtk-support.h>

#include <ep-common.h>
#include <proc_list.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"

static gchar *_pszCfgFileName;
static PLINEA_STRUCT _pDatiLinea;

struct _MainApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(MainApp, main_app, GTK_TYPE_APPLICATION);

static void main_app_init (MainApp *app)
{
    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s", __FILE__, __FUNCTION__);

    _pszCfgFileName = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    /* read configuration from cfg file */
    if(!ReadCfg(_pszCfgFileName, CFGItems, TRUE)){
        trace_debug(TRUE, TRUE, "Configuration file [%s] not present", _pszCfgFileName);
        fprintf(stderr, "Configuration file [%s] not present\n", _pszCfgFileName);
        g_application_quit (G_APPLICATION (app));
    }

    /*
    * coda messaggi non presente - tutto ok
    * creo la coda locale per l'applicazione
    */
    if(CreateProcessMsgQ(APP_PID, 1)<0){
        trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", APP_NAME, APP_PID);
        fprintf(stderr, "Creazione coda msg di %s (%d) fallita\n", APP_NAME, APP_PID);
        // g_application_quit (G_APPLICATION (app));
    }
    /*
    * Creo la shared memory per la gestione della linea
    */
    if((_pDatiLinea=(PLINEA_STRUCT)CreateSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
        trace_debug(TRUE, TRUE, "Cannot Allocate Shared Memory [%d] !", Cfg.nShmKey);
        fprintf(stderr, "Cannot Allocate Shared Memory [%d] !\n", Cfg.nShmKey);
        // g_application_quit (G_APPLICATION (app));
    }

    /*
    * connect to db
    */
    ep_bool_t rc;
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){
        trace_debug(TRUE, TRUE, "Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        fprintf(stderr, "Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        // g_application_quit (G_APPLICATION (app));
    }

}

static void main_app_startup (GApplication *app)
{
    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s", __FILE__, __FUNCTION__);

    G_APPLICATION_CLASS (main_app_parent_class)->startup (app);

}

static void main_app_activate (GApplication *app)
{
    MainWindow *win;

    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s", __FILE__, __FUNCTION__);
    win = main_window_new (MAIN_APP (app));
    gtk_window_present (GTK_WINDOW (win));
}

static void main_app_class_init (MainAppClass *class)
{
    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s", __FILE__, __FUNCTION__);
    G_APPLICATION_CLASS (class)->startup = main_app_startup;
    G_APPLICATION_CLASS (class)->activate = main_app_activate;
}

MainApp *main_app_new (void)
{
    g_log (NULL,G_LOG_LEVEL_DEBUG, "%s - %s", __FILE__, __FUNCTION__);

    gchar *application_id = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    return g_object_new (MAIN_APP_TYPE, "application-id", application_id, "flags", ep_get_glib_default_flags(), NULL);

    g_free(application_id);
}
