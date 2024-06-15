#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "mainapp.h"
#include "mainappwin.h"
#include "mainappabout.h"
#include "mainfun.h"

struct _MainAppAbout
{
    GtkDialog parent;
};

typedef struct _MainAppAboutPrivate MainAppAboutPrivate;

struct _MainAppAboutPrivate
{
    GtkWidget *lb_version;
    GtkWidget *lb_customer;
    GtkWidget *lb_authors;
    GtkWidget *lb_copyright;
    GtkWidget *lb_rcsid;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppAbout, main_app_about, GTK_TYPE_DIALOG)

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
  {"close", close_activated, NULL, NULL, NULL}
};

static void main_app_about_init (MainAppAbout *win)
{
    MainAppAboutPrivate *priv;

    priv = main_app_about_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));
    
    extern char rcsid[];
    extern char __version__[];
    extern char __customer__[];
    extern char __copyright__[];
    extern char __authors__[];
    
    gtk_label_set_text(GTK_LABEL(priv->lb_version),   __version__);
    gtk_label_set_text(GTK_LABEL(priv->lb_customer),  __customer__);
    gtk_label_set_text(GTK_LABEL(priv->lb_authors),   __authors__);
    gtk_label_set_text(GTK_LABEL(priv->lb_copyright), __copyright__);
    gtk_label_set_text(GTK_LABEL(priv->lb_rcsid ),    rcsid );

    init_actions(win, entries, G_N_ELEMENTS(entries), "about");
}

static void main_app_about_dispose (GObject *object)
{
    MainAppAboutPrivate *priv;

    priv = main_app_about_get_instance_private (MAIN_APP_ABOUT (object));

    G_OBJECT_CLASS (main_app_about_parent_class)->dispose (object);
}

static void main_app_about_class_init (MainAppAboutClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_about_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/about.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppAbout, lb_version);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppAbout, lb_customer);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppAbout, lb_authors);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppAbout, lb_copyright);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppAbout, lb_rcsid);
}

MainAppAbout * main_app_about_new (MainAppWindow *win)
{
    return g_object_new (MAIN_APP_ABOUT_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
