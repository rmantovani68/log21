#ifndef __MAINAPP_H
#define __MAINAPP_H

#include <gtk/gtk.h>
#include <mainwin.h>

#define APP_NAME "template"
#define APP_TITLE "Template"
#define APP_PID PROC_TEMPLATE
#define MAIN_APP_TYPE (main_app_get_type ())
G_DECLARE_FINAL_TYPE (MainApp, main_app, MAIN, APP, GtkApplication)

struct _MainApp
{
    GtkApplication parent;
};

struct _MainAppPrivate
{
    MainApp *app;
    GtkApplicationWindow *main_window;

    GSettings *settings;
    gint nPID;

    gchar *pszAppName;
    gchar *pszAppClass;
    gchar *pszCfgFileName;
    gchar *pszAppTitle;

    gchar *pszDocumentsPath;
    gchar *pszDownloadPath;
    gchar *pszPicturesPath;
    gchar *pszDesktopPath;
};

typedef struct _MainAppPrivate MainAppPrivate;



MainApp *main_app_new (void);
void main_app_quit (MainApp *app);
MainAppPrivate *get_main_app_private_instance (gpointer app);


#endif /* __MAINAPP_H */
