#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "fine_linea.h"
#include "mainfun.h"

#define WINDOW_NAME "fine-linea-window"

struct _FineLineaWindow
{
    GtkDialog parent;
};

typedef struct _FineLineaWindowPrivate FineLineaWindowPrivate;

/* workaround */
static MainWindow *_main_window;

struct _FineLineaWindowPrivate
{
    MainWindow *main_window;

    GtkWidget *lb_lntpspe;
    GtkWidget *lb_lncdlin;
    GtkWidget *lb_lnnmbnc;
    GtkWidget *lb_lnnmcll;
    GtkWidget *lb_lnpsrea;
    GtkWidget *lb_lnpspre;
    GtkWidget *lb_lnnmced;
    GtkWidget *lb_key_linea;
    GtkWidget *lb_msg;
    GtkWidget *sw_bancali;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
};

G_DEFINE_TYPE_WITH_PRIVATE(FineLineaWindow, fine_linea_window, GTK_TYPE_DIALOG)



/* 
* 'fine' di una linea di spedizione
*/
static void fine_linea_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    FineLineaWindowPrivate *priv = fine_linea_window_get_instance_private (FINE_LINEA_WINDOW(win));
    MainWindowPrivate *priv_main = internal_main_window_get_instance_private (priv->main_window);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "update storico_linee_spedizione set tmclose='now', stato='%c' where key_linea=%d;",LINEA_FINITA,priv_main->Linea.key);
    if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
    } else {
        trace_debug_gtk("RED",FALSE,TRUE,priv_main->txt_msgs,"Errore in termine Linea di spedizione [%d]",priv_main->Linea.key);
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);

    if(bOK){
        SetFase(priv->main_window, ATTESA_INIZIO_LINEA);
    }
}



static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {
  {"ok",     fine_linea_activated, NULL, NULL, NULL},
  {"cancel", close_activated, NULL, NULL, NULL},
  {"stampa", action_activated, NULL, NULL, NULL}
};

static void fine_linea_window_init (FineLineaWindow *win)
{
    FineLineaWindowPrivate *priv = fine_linea_window_get_instance_private (win);

    gtk_widget_init_template (GTK_WIDGET (win));


    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi );
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi );

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void fine_linea_window_dispose (GObject *object)
{
    FineLineaWindow *win = FINE_LINEA_WINDOW (object);
    FineLineaWindowPrivate *priv = fine_linea_window_get_instance_private (win);

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

    G_OBJECT_CLASS (fine_linea_window_parent_class)->dispose (object);
}

static void fine_linea_window_class_init (FineLineaWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = fine_linea_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lntpspe  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lncdlin  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lnnmbnc  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lnnmcll  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lnpsrea  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lnpspre  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_lnnmced  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_key_linea);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, lb_msg      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), FineLineaWindow, sw_bancali  );
}

FineLineaWindow * fine_linea_window_new (MainWindow *win)
{
    FineLineaWindow *w;

    /* valorizzo la variabile prima della new ... */
    _main_window = win;

    w =  g_object_new (FINE_LINEA_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    FineLineaWindowPrivate *priv = fine_linea_window_get_instance_private (w);

    priv->main_window = win;

    return w;
}
