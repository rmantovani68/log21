#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "printdist.h"
#include "mainfun.h"

#define WINDOW_NAME "print-distribution-window"

struct _PrintDistWindow
{
    GtkDialog parent;
};

typedef struct _PrintDistWindowPrivate PrintDistWindowPrivate;

struct _PrintDistWindowPrivate
{
    gchar *pszCfgFileName;

    GtkWidget *entry_tpspe;
    GtkWidget *entry_cdlin;
    GtkWidget *entry_ordprog;
    GtkWidget *sb_cedola;
    GtkWidget *cb_storico;
};

G_DEFINE_TYPE_WITH_PRIVATE(PrintDistWindow, print_dist, GTK_TYPE_DIALOG)

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

static void print_dist_init (PrintDistWindow *win)
{
    PrintDistWindowPrivate *priv;

    g_printf("print_dist_init\n");

    gtk_widget_init_template (GTK_WIDGET (win));

    priv = print_dist_get_instance_private (win);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void print_dist_dispose (GObject *object)
{
    PrintDistWindowPrivate *priv;

    priv = print_dist_get_instance_private (PRINT_DIST_WINDOW (object));

    g_free(priv->pszCfgFileName);
    priv->pszCfgFileName = NULL;

    G_OBJECT_CLASS (print_dist_parent_class)->dispose (object);
}

static void print_dist_class_init (PrintDistWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = print_dist_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PrintDistWindow, entry_tpspe  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PrintDistWindow, entry_cdlin  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PrintDistWindow, entry_ordprog);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PrintDistWindow, sb_cedola    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), PrintDistWindow, cb_storico   );
}

PrintDistWindow * print_dist_new (MainWindow *win)
{
    return g_object_new (PRINT_DIST_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
