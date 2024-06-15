#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <ep-common.h>
#include <picking.h>

#include <gtk-support.h>

#include "mainstruct.h"
#include "mainapp.h"
#include "mainwin.h"



struct _MainApp
{
    GtkApplication parent;
};

G_DEFINE_TYPE(MainApp, main_app, GTK_TYPE_APPLICATION);

static void main_app_init (MainApp *app)
{
    g_printf("main_app_init\n");
}

static void main_app_startup (GApplication *app)
{
    g_printf("main_app_startup\n");

    G_APPLICATION_CLASS (main_app_parent_class)->startup (app);

}

static void main_app_activate (GApplication *app)
{
    MainWindow *win;

    g_printf("main_app_activate\n");
    win = main_window_new (MAIN_APP (app));
    gtk_window_present (GTK_WINDOW (win));
}

static void main_app_class_init (MainAppClass *class)
{
    g_printf("main_app_class_init\n");
    G_APPLICATION_CLASS (class)->startup = main_app_startup;
    G_APPLICATION_CLASS (class)->activate = main_app_activate;
}

MainApp *main_app_new (void)
{
    g_printf("main_app_new\n");
    char szBuffer[256];
    sprintf(szBuffer, "org.%s.%s", APP_CLASS, APP_NAME);
    return g_object_new (MAIN_APP_TYPE, "application-id", szBuffer, "flags", ep_get_glib_default_flags(), NULL);
}
