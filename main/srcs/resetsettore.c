#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "resetsettore.h"
#include "mainfun.h"

#define WINDOW_NAME "reset-settore-window"

struct _SettoreWindow
{
    GtkDialog parent;
};

typedef struct _SettoreWindowPrivate SettoreWindowPrivate;

struct _SettoreWindowPrivate
{
    MainWindow *main_window;
    GtkWidget *sb_isola;
    GtkWidget *sb_settore;
};

G_DEFINE_TYPE_WITH_PRIVATE(SettoreWindow, main_app_reset_settore, GTK_TYPE_DIALOG)

void do_reset_settore(int isola, int settore)
{
    char szBuffer[128];

	sprintf(szBuffer,"%d,%d",isola,settore);
	SendMessage(PROC_SETTORI, PROC_MAIN, RESET_SETTORE, szBuffer);
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void reset_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    SettoreWindowPrivate *priv;

    priv = main_app_reset_settore_get_instance_private (win);
	int isola   = gtk_spin_button_get_value_as_int((GtkSpinButton *)priv->sb_isola);
	int settore = gtk_spin_button_get_value_as_int((GtkSpinButton *)priv->sb_settore);

    int rc=dlg_msg( GTK_WINDOW(win), "Reset Settore", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Reset Settore %d Isola %d ?", settore, isola);
    switch(rc){
        case GTK_RESPONSE_YES:
			do_reset_settore(isola, settore);
        break;
    }
}

static GActionEntry entries[] = {
  {"reset", reset_activated, NULL, NULL, NULL},
  {"close", close_activated, NULL, NULL, NULL}
};

static void main_app_reset_settore_init (SettoreWindow *win)
{
    gtk_widget_init_template (GTK_WIDGET (win));

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void main_app_reset_settore_dispose (GObject *object)
{
    SettoreWindowPrivate *priv;

    priv = main_app_reset_settore_get_instance_private (RESET_SETTORE_WINDOW (object));

    G_OBJECT_CLASS (main_app_reset_settore_parent_class)->dispose (object);
}

static void main_app_reset_settore_class_init (SettoreWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_reset_settore_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), SettoreWindow, sb_settore       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), SettoreWindow, sb_isola);
}

SettoreWindow * main_app_reset_settore_new (MainWindow *win)
{
    SettoreWindow *w = g_object_new (RESET_SETTORE_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    SettoreWindowPrivate *priv = main_app_reset_settore_get_instance_private (RESET_SETTORE_WINDOW (w));

    priv->main_window = win;

    return w;
}
