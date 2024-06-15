#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <gtk-support.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "conteggia_prodotto.h"
#include "mainfun.h"

#define WINDOW_NAME "conteggia-prodotto-window"

static GtkWindow *_parent;

struct _ConteggiaProdottoWindow
{
    GtkDialog parent;
};

typedef struct _ConteggiaProdottoWindowPrivate ConteggiaProdottoWindowPrivate;

struct _ConteggiaProdottoWindowPrivate
{
    GtkWindow *parent;

    gchar *pszCfgFileName;

    GSettings *settings;

    WindowSizeInfo *wsi;
};

G_DEFINE_TYPE_WITH_PRIVATE(ConteggiaProdottoWindow, conteggia_prodotto_window, GTK_TYPE_DIALOG)


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
    {"close",      close_activated,      NULL, NULL, NULL },
    {"action",     action_activated,     NULL, NULL, NULL }
};

static void conteggia_prodotto_window_init (ConteggiaProdottoWindow *win)
{
    ConteggiaProdottoWindowPrivate *priv = conteggia_prodotto_window_get_instance_private (win);

    g_printf("%s_window_init\n", WINDOW_NAME);

    gtk_widget_init_template (GTK_WIDGET (win));


    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);


    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void conteggia_prodotto_window_dispose (GObject *object)
{
    ConteggiaProdottoWindow *win = CONTEGGIA_PRODOTTO_WINDOW (object);
    ConteggiaProdottoWindowPrivate *priv = conteggia_prodotto_window_get_instance_private (win);

    g_printf("%s_dispose\n", WINDOW_NAME);

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


    G_OBJECT_CLASS (conteggia_prodotto_window_parent_class)->dispose (object);
}

static void conteggia_prodotto_window_finalize (GObject *object)
{
    ConteggiaProdottoWindow *win = CONTEGGIA_PRODOTTO_WINDOW (object);
    ConteggiaProdottoWindowPrivate *priv = conteggia_prodotto_window_get_instance_private (win);

    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (conteggia_prodotto_window_parent_class)->finalize (object);
}

static void conteggia_prodotto_window_class_init (ConteggiaProdottoWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = conteggia_prodotto_window_dispose;
    G_OBJECT_CLASS (class)->finalize = conteggia_prodotto_window_finalize;

    g_printf("%s_class_init\n", WINDOW_NAME);

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

}

ConteggiaProdottoWindow * conteggia_prodotto_window_new (GtkWindow *parent)
{
    _parent = parent;
    g_printf("%s_new\n", WINDOW_NAME);

    ConteggiaProdottoWindow *win = g_object_new (CONTEGGIA_PRODOTTO_WINDOW_TYPE, "transient-for", parent, "use-header-bar", TRUE, NULL);
    ConteggiaProdottoWindowPrivate *priv = conteggia_prodotto_window_get_instance_private (win);

    priv->parent = _parent;

    return win;
}
