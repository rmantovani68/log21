#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainappwin.h"
#include "mainappprintdist.h"
#include "mainfun.h"

struct _MainAppPrintDist
{
    GtkDialog parent;
};

typedef struct _MainAppPrintDistPrivate MainAppPrintDistPrivate;

struct _MainAppPrintDistPrivate
{
    gchar *pszCfgFileName;

    GtkWidget *entry_tpspe;
    GtkWidget *entry_cdlin;
    GtkWidget *entry_ordprog;
    GtkWidget *sb_cedola;
    GtkWidget *cb_storico;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppPrintDist, main_app_printdist, GTK_TYPE_DIALOG)

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
  {"close",                  close_activated,  NULL, NULL, NULL} , 
  {"stampa_xab",             action_activated, NULL, NULL, NULL} , 
  {"stampa_distribuzione",   action_activated, NULL, NULL, NULL} , 
  {"stampa_distinta",        action_activated, NULL, NULL, NULL} , 
  {"dischetto_corriere",     action_activated, NULL, NULL, NULL} , 
  {"dischetto_corr_agg",     action_activated, NULL, NULL, NULL} , 
  {"dischetto_corr_sda",     action_activated, NULL, NULL, NULL} ,
  {"spedizione_host",        action_activated, NULL, NULL, NULL}
    
};

static void main_app_printdist_init (MainAppPrintDist *win)
{
    MainAppPrintDistPrivate *priv;

     g_logv (NULL,G_LOG_LEVEL_DEBUG, "main_app_printdist_init");

    gtk_widget_init_template (GTK_WIDGET (win));

    priv = main_app_printdist_get_instance_private (win);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    init_actions(win, entries, G_N_ELEMENTS(entries), "printdist");
}

static void main_app_printdist_dispose (GObject *object)
{
    MainAppPrintDistPrivate *priv;

    priv = main_app_printdist_get_instance_private (MAIN_APP_PRINTDIST (object));

    g_free(priv->pszCfgFileName);
    priv->pszCfgFileName = NULL;

    G_OBJECT_CLASS (main_app_printdist_parent_class)->dispose (object);
}

static void main_app_printdist_class_init (MainAppPrintDistClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_printdist_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/print_distribution.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrintDist, entry_tpspe  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrintDist, entry_cdlin  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrintDist, entry_ordprog);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrintDist, sb_cedola    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppPrintDist, cb_storico   );
}

MainAppPrintDist * main_app_printdist_new (MainAppWindow *win)
{
    return g_object_new (MAIN_APP_PRINTDIST_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
