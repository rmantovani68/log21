#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <gtk-support.h>

#include <picking.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainappwin.h"
#include "mainappprefs.h"
#include "mainfun.h"

struct _MainAppPrefs
{
    GtkDialog parent;
};

typedef struct _MainAppPrefsPrivate MainAppPrefsPrivate;

struct _MainAppPrefsPrivate
{
    gchar *pszCfgFileName;

    GSettings *settings;
    GtkWidget *font;
    GtkWidget *transition;

    GtkWidget *pb_ok;
    GtkWidget *pb_apply;
    GtkWidget *pb_cancel;

    GtkWidget *sb_debug_level;
    GtkWidget *sb_incremento_copie_pallet;
    GtkWidget *sb_soglia_pallet;
    GtkWidget *sb_soglia_prelievo;

    GtkWidget *combo_imballo_medio;

    GtkWidget *cb_reindex_periodico;
    GtkWidget *cb_invio_periodico;
    GtkWidget *cb_a2ps;
    GtkWidget *cb_reindex;

    GtkWidget *rb_imballi_ubicati;
    GtkWidget *rb_imballi_non_ubicati;
    GtkWidget *rb_gestione_collo;
    GtkWidget *rb_gestione_ordine;
    GtkWidget *rb_all_rac;
    GtkWidget *rb_settori;
    GtkWidget *rb_ordine_evaso;
    GtkWidget *rb_ordine_stampato_xab;
    GtkWidget *rb_selezione_singola;
    GtkWidget *rb_selezione_multipla;

    GtkWidget *entry_path_data;
    GtkWidget *entry_path_exe;
    GtkWidget *entry_path_export;
    GtkWidget *entry_path_stampe;
    GtkWidget *entry_path_storico;
    GtkWidget *entry_import_monitor;
    GtkWidget *entry_import_spedizioni;
    GtkWidget *entry_import_catalogo;
    GtkWidget *entry_import_tabelle;
    GtkWidget *entry_export_monitor;
    GtkWidget *entry_export_spedizioni;
    GtkWidget *entry_export_corriere;
    GtkWidget *entry_export_corr_agg;
    GtkWidget *entry_export_corr_sda;
    GtkWidget *entry_export_monitor_storico;
    GtkWidget *entry_export_spedizioni_storico;
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

G_DEFINE_TYPE_WITH_PRIVATE(MainAppPrefs, main_app_prefs, GTK_TYPE_DIALOG)

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppPrefsPrivate *priv = main_app_prefs_get_instance_private (win);

    ApplyCfgItems(GTK_WIDGET(win),CFGItems);
    WriteCfgItems(CFGItems, priv->pszCfgFileName);
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

static void main_app_prefs_init (MainAppPrefs *win)
{
    MainAppPrefsPrivate *priv = main_app_prefs_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));
    priv->settings = g_settings_new ("org.easy-picking.mainapp");

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    SetCfgWidgets(GTK_WIDGET (win), CFGItems);

    switch(Cfg.nImballiUbicati){
        case 0:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_imballi_non_ubicati) , TRUE);
        break;
        case 1:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_imballi_ubicati) , TRUE);
        break;
    }

    switch(Cfg.nChiusuraCollo){
        case 0:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_gestione_ordine) , TRUE);
        break;
        case 1:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_gestione_collo) , TRUE);
        break;
    }

    switch(Cfg.nStampaRAC){
        case DOPO_IL_LANCIO:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_all_rac) , TRUE);
        break;
        case INIZIO_PRELIEVO:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_settori) , TRUE);
        break;
    }

    switch(Cfg.szTipoOrdinePreDistinta[0]){
        case 'X':
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_ordine_stampato_xab) , TRUE);
        break;
        case 'E':
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_ordine_evaso) , TRUE);
        break;
    }

    /* tipo selezione */
    switch(Cfg.nTipoSelezione){
        GTK_SELECTION_NONE:
        break;
        GTK_SELECTION_SINGLE:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_selezione_singola) , TRUE);
        break;
        GTK_SELECTION_BROWSE:
        break;
        GTK_SELECTION_MULTIPLE:
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(priv->rb_selezione_multipla) , TRUE);
        break;
    }


    init_actions(win, entries, G_N_ELEMENTS(entries), "preferences");
}

static void main_app_prefs_dispose (GObject *object)
{
    MainAppPrefsPrivate *priv;

    priv = main_app_prefs_get_instance_private (MAIN_APP_PREFS (object));
    g_clear_object (&priv->settings);

    G_OBJECT_CLASS (main_app_prefs_parent_class)->dispose (object);
}

static void main_app_prefs_class_init (MainAppPrefsClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_prefs_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/preferences.ui");

    /*
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, font);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, transition);
    */
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, sb_debug_level                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, sb_incremento_copie_pallet     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, sb_soglia_pallet               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, sb_soglia_prelievo             );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, combo_imballo_medio            );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, cb_reindex_periodico           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, cb_invio_periodico             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, cb_a2ps                        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, cb_reindex                     );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_imballi_ubicati             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_imballi_non_ubicati         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_gestione_collo              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_gestione_ordine             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_all_rac                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_settori                     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_ordine_evaso                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_ordine_stampato_xab         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_selezione_singola           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, rb_selezione_multipla          );

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_path_data                );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_path_exe                 );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_path_export              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_path_stampe              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_path_storico             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_import_monitor           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_import_spedizioni        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_import_catalogo          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_import_tabelle           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_monitor           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_spedizioni        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_corriere          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_corr_agg          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_corr_sda          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_monitor_storico   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_export_spedizioni_storico);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_pghost                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_pgport                   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_pgdatabase               );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_printer_console          );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_label_printer_console    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_xab_printer              );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_dist_printer             );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_printer_bilancia         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrefs, entry_label_printer_bilancia   );
}

MainAppPrefs * main_app_prefs_new (MainAppWindow *win)
{
    MainAppPrefs *w = g_object_new (MAIN_APP_PREFS_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppPrefsPrivate *priv = main_app_prefs_get_instance_private (MAIN_APP_PREFS (w));

    return w;
}
