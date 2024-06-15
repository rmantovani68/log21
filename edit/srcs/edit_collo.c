#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_collo.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-collo-window"

struct _EditColloWindow
{
    GtkDialog parent;
};

typedef struct _EditColloWindowPrivate EditColloWindowPrivate;

struct _EditColloWindowPrivate
{
    GtkWidget *lb_msg;
    GtkWidget *sw_list;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
};

G_DEFINE_TYPE_WITH_PRIVATE(EditColloWindow, edit_collo_window, GTK_TYPE_DIALOG)


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
    EditColloWindowPrivate *priv = edit_collo_window_get_instance_private (win);

}

static void next_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditColloWindowPrivate *priv = edit_collo_window_get_instance_private (win);

}

static GActionEntry entries[] = {
  {"close",      close_activated,      NULL, NULL, NULL},
  {"prev",       prev_activated,       NULL, NULL, NULL},
  {"next",       next_activated,       NULL, NULL, NULL}
                
                
};

static void edit_collo_window_init (EditColloWindow *win)
{
    EditColloWindowPrivate *priv = edit_collo_window_get_instance_private (win);

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

static void edit_collo_window_dispose (GObject *object)
{
    EditColloWindow *win = EDIT_COLLO_WINDOW (object);
    EditColloWindowPrivate *priv = edit_collo_window_get_instance_private (win);

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


    G_OBJECT_CLASS (edit_collo_window_parent_class)->dispose (object);
}

static void edit_collo_window_class_init (EditColloWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_collo_window_dispose;

    gchar *pszBuffer;
    pszBuffer = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), pszBuffer);
    g_free(pszBuffer);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditColloWindow, sw_list      );
}

EditColloWindow * edit_collo_window_new (EditOrdineWindow *win)
{
    return g_object_new (EDIT_COLLO_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
