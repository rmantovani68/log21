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
#include "mainappresetsettore.h"
#include "mainfun.h"


void do_reset_settore(int isola, int settore)
{
    char szBuffer[128];

	sprintf(szBuffer,"%d,%d",isola,settore);
	SendMessage(PROC_SETTORI, PROC_MAIN, RESET_SETTORE, szBuffer);
}

struct _MainAppResetSettore
{
    GtkDialog parent;
};

typedef struct _MainAppResetSettorePrivate MainAppResetSettorePrivate;

struct _MainAppResetSettorePrivate
{
    MainAppWindow *main_app_window;
    GtkWidget *sb_isola;
    GtkWidget *sb_settore;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppResetSettore, main_app_reset_settore, GTK_TYPE_DIALOG)


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void reset_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppResetSettorePrivate *priv;

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

static void main_app_reset_settore_init (MainAppResetSettore *win)
{
    gtk_widget_init_template (GTK_WIDGET (win));

    init_actions(win, entries, G_N_ELEMENTS(entries), "reset_settore");
}

static void main_app_reset_settore_dispose (GObject *object)
{
    MainAppResetSettorePrivate *priv;

    priv = main_app_reset_settore_get_instance_private (MAIN_APP_RESET_SETTORE (object));

    G_OBJECT_CLASS (main_app_reset_settore_parent_class)->dispose (object);
}

static void main_app_reset_settore_class_init (MainAppResetSettoreClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_reset_settore_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/reset_settore.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppResetSettore, sb_settore       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppResetSettore, sb_isola);
}

MainAppResetSettore * main_app_reset_settore_new (MainAppWindow *win)
{
    MainAppResetSettore *w = g_object_new (MAIN_APP_RESET_SETTORE_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppResetSettorePrivate *priv = main_app_reset_settore_get_instance_private (MAIN_APP_RESET_SETTORE (w));

    priv->main_app_window = win;

    return w;
}
