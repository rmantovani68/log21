#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_prodotto.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-prodotto-window"

struct _EditProdottoWindow
{
    GtkDialog parent;
};

typedef struct _EditProdottoWindowPrivate EditProdottoWindowPrivate;

struct _EditProdottoWindowPrivate
{
    GtkWidget *lb_msg;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
};

G_DEFINE_TYPE_WITH_PRIVATE(EditProdottoWindow, edit_prodotto_window, GTK_TYPE_DIALOG)


static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void prev_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
}

static void next_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
}

static void insert_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
}

static void record_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
}

static void delete_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
}

static GActionEntry entries[] = {
  {"close",      close_activated,      NULL, NULL, NULL},
  {"prev",       prev_activated,       NULL, NULL, NULL},
  {"next",       next_activated,       NULL, NULL, NULL},
  {"insert",     insert_activated,     NULL, NULL, NULL},
  {"record",     record_activated,     NULL, NULL, NULL},
  {"delete",     delete_activated,     NULL, NULL, NULL}
                
};

static void edit_prodotto_window_init (EditProdottoWindow *win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

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

static void edit_prodotto_window_dispose (GObject *object)
{
    EditProdottoWindow *win = EDIT_PRODOTTO_WINDOW (object);
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

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


    G_OBJECT_CLASS (edit_prodotto_window_parent_class)->dispose (object);
}

static void edit_prodotto_window_class_init (EditProdottoWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_prodotto_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

}

EditProdottoWindow * edit_prodotto_window_new (MainWindow *win)
{
    return g_object_new (EDIT_PRODOTTO_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
