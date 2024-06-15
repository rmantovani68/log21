#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>

#include "ep-about.h"

#define WINDOW_NAME "about-window"

struct _AboutWindow
{
    GtkDialog parent;
};

typedef struct _AboutWindowPrivate AboutWindowPrivate;

struct _AboutWindowPrivate
{
    GtkWidget *lb_module_name;
    GtkWidget *lb_version;
    GtkWidget *lb_customer;
    GtkWidget *lb_authors;
    GtkWidget *lb_copyright;
    GtkWidget *lb_rcsid;
};

G_DEFINE_TYPE_WITH_PRIVATE(AboutWindow, about_window, GTK_TYPE_DIALOG)

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {
  {"close", close_activated, NULL, NULL, NULL}
};

static void about_window_init (AboutWindow *win)
{
    AboutWindowPrivate *priv = about_window_get_instance_private (ABOUT_WINDOW (win));

    gtk_widget_init_template (GTK_WIDGET (win));
    
    extern char rcsid[];
    extern char __module_name__[];
    extern char __version__[];
    extern char __customer__[];
    extern char __copyright__[];
    extern char __authors__[];
    
    gtk_label_set_text(GTK_LABEL(priv->lb_module_name), __module_name__);
    gtk_label_set_text(GTK_LABEL(priv->lb_version),   __version__);
    gtk_label_set_text(GTK_LABEL(priv->lb_customer),  __customer__);
    gtk_label_set_text(GTK_LABEL(priv->lb_authors),   __authors__);
    gtk_label_set_text(GTK_LABEL(priv->lb_copyright), __copyright__);
    gtk_label_set_text(GTK_LABEL(priv->lb_rcsid ),    rcsid );

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void about_window_dispose (GObject *object)
{
    AboutWindow *win = ABOUT_WINDOW (object);
    AboutWindowPrivate *priv = about_window_get_instance_private (win);

    g_printf("%s dispose\n", WINDOW_NAME);

    G_OBJECT_CLASS (about_window_parent_class)->dispose (object);
}

static void about_window_class_init (AboutWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = about_window_dispose;

    /* register resources */
    _ep_ensure_resources ();

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_UTILITIES, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_module_name);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_version);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_customer);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_authors);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_copyright);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), AboutWindow, lb_rcsid);
}

GtkWidget *about_window_new (GtkWindow *parent, GtkDialogFlags flags)
{
    GtkWidget *widget;
    GtkDialog *dialog;

    g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

    g_printf("%s new\n", WINDOW_NAME);
    
    widget = g_object_new (ABOUT_WINDOW_TYPE, "use-header-bar", TRUE, NULL);

    dialog = GTK_DIALOG (widget);

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (widget), GTK_WINDOW (parent));
  
    if (flags & GTK_DIALOG_MODAL)
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    if (flags & GTK_DIALOG_DESTROY_WITH_PARENT)
        gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

    return widget;
}
