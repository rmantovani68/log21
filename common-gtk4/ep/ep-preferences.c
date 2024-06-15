#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>
#include <ep-preferences.h>

#include <proc_list.h>

#define WINDOW_NAME "preferences-window"

struct _PreferencesWindow
{
    GtkDialog parent;
};

typedef struct _PreferencesWindowPrivate PreferencesWindowPrivate;

struct _PreferencesWindowPrivate
{
    GSettings *settings;
    WindowSizeInfo *wsi;

    GtkWidget *pb_ok;
    GtkWidget *pb_apply;
    GtkWidget *pb_cancel;

    GtkWidget *sb_debug_level;

    GtkWidget *entry_path_data;
    GtkWidget *entry_path_exe;
    GtkWidget *entry_path_export;
    GtkWidget *entry_path_stampe;
    GtkWidget *entry_path_storico;
    GtkWidget *entry_pghost;
    GtkWidget *entry_pgport;
    GtkWidget *entry_pgdatabase;
    GtkWidget *entry_printer_console;
    GtkWidget *entry_label_printer_console;
    GtkWidget *entry_xab_printer;
    GtkWidget *entry_dist_printer;
    GtkWidget *entry_printer_bilancia;
    GtkWidget *entry_label_printer_bilancia;

};

G_DEFINE_TYPE_WITH_PRIVATE(PreferencesWindow, preferences_window, GTK_TYPE_DIALOG)

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ApplyCfgItems(GTK_WIDGET(win),CFGItems);
    g_printf("OK Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}
static void apply_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ApplyCfgItems(GTK_WIDGET(win),CFGItems);
    g_printf("Apply Action : activated\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}


static GActionEntry entries[] = {
    {"verifica_linea_spedizione",  action_activated, NULL, NULL, NULL},
    {"aggiorna_linea_spedizione",  action_activated, NULL, NULL, NULL},
    {"cancella_linea_spedizione",  action_activated, NULL, NULL, NULL},
    {"modifica_vettore",           action_activated, NULL, NULL, NULL},
    {"ok",                         ok_activated, NULL, NULL, NULL},
    {"apply",                      apply_activated, NULL, NULL, NULL},
    {"close",                      close_activated,  NULL, NULL, NULL}
};

static void preferences_window_init (PreferencesWindow *win)
{
    PreferencesWindowPrivate *priv;

    priv = preferences_window_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    SetCfgWidgets(GTK_WIDGET (win), CFGItems);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void preferences_window_dispose (GObject *object)
{
    PreferencesWindow *win = PREFERENCES_WINDOW (object);
    PreferencesWindowPrivate *priv = preferences_window_get_instance_private (win);

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

    G_OBJECT_CLASS (preferences_window_parent_class)->dispose (object);
}

static void preferences_window_class_init (PreferencesWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = preferences_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_GENERIC, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, sb_debug_level                 );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_path_data                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_path_exe                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_path_export              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_path_stampe              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_path_storico             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_pghost                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_pgport                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_pgdatabase               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_printer_console          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_label_printer_console    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_xab_printer              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_dist_printer             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_printer_bilancia         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PreferencesWindow, entry_label_printer_bilancia   );
}

PreferencesWindow * preferences_window_new (GtkApplicationWindow *win)
{
    g_printf("%s new\n", WINDOW_NAME);

    return g_object_new (PREFERENCES_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
