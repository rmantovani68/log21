#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "template_source.h"
#include "mainfun.h"

#define WINDOW_NAME "template-source-window"

static GtkWindow *_parent;

struct _TemplateSourceWindow
{
    GtkDialog parent;
};

typedef struct _TemplateSourceWindowPrivate TemplateSourceWindowPrivate;

struct _TemplateSourceWindowPrivate
{
    GtkApplication *app;
    GtkWindow *parent;

    WindowSizeInfo *wsi;
};

G_DEFINE_TYPE_WITH_PRIVATE(TemplateSourceWindow, template_source_window, GTK_TYPE_DIALOG)


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

static void template_source_window_init (TemplateSourceWindow *win)
{
    g_printf("%s_window_init\n", WINDOW_NAME);

}

static void template_source_window_dispose (GObject *object)
{
    g_printf("%s_dispose\n", WINDOW_NAME);

    TemplateSourceWindow *win = TEMPLATE_SOURCE_WINDOW (object);
    TemplateSourceWindowPrivate *priv = template_source_window_get_instance_private (win);

    if(priv->parent){
        MainWindowPrivate *main_win_priv = get_main_window_private_instance (priv->parent);
        MainAppPrivate *main_app_priv = get_main_app_private_instance (main_win_priv->app);

        if(priv->wsi){          
            g_settings_sync ();
            window_save_state (win,  priv->wsi, main_app_priv->settings);
            window_size_info_free (priv->wsi);
            priv->wsi = NULL;
        }
    }

    G_OBJECT_CLASS (template_source_window_parent_class)->dispose (object);
}

static void template_source_window_finalize (GObject *object)
{
    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (template_source_window_parent_class)->finalize (object);
}

static void template_source_window_class_init (TemplateSourceWindowClass *class)
{
    g_printf("%s_class_init\n", WINDOW_NAME);

    G_OBJECT_CLASS (class)->dispose = template_source_window_dispose;
    G_OBJECT_CLASS (class)->finalize = template_source_window_finalize;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

}

static void template_source_window_setup (TemplateSourceWindow *win)
{
    TemplateSourceWindowPrivate *priv = template_source_window_get_instance_private (win);

    if(priv->parent){
        MainWindowPrivate *main_win_priv = get_main_window_private_instance (priv->parent);
        priv->app = main_win_priv->app;
        MainAppPrivate *main_app_priv = get_main_app_private_instance (priv->app);

        gtk_widget_init_template (GTK_WIDGET (win));

        priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

        window_load_state (win, priv->wsi, main_app_priv->settings);

        g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
        g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);
    }

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

TemplateSourceWindow * template_source_window_new (GtkWindow *parent)
{
    g_printf("%s_new\n", WINDOW_NAME);

    TemplateSourceWindow *win = g_object_new (TEMPLATE_SOURCE_WINDOW_TYPE, "transient-for", parent, "use-header-bar", TRUE, NULL);
    TemplateSourceWindowPrivate *priv = template_source_window_get_instance_private (win);
    priv->parent = parent;
    template_source_window_setup (win);

    return win;
}
