#ifndef __MAINAPPWIN_H
#define __MAINAPPWIN_H

#include <gtk/gtk.h>

#include <proc_list.h>
#include "mainapp.h"


#define MAIN_APP_WINDOW_TYPE (main_app_window_get_type ())
G_DECLARE_FINAL_TYPE (MainAppWindow, main_app_window, MAIN, APP_WINDOW, GtkApplicationWindow)


struct _MainAppWindow
{
    GtkApplicationWindow parent;
};


struct _MainAppWindowPrivate
{
    MainApp *app;

    GSettings *settings;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;

    gint nPID;
    gchar *pszAppName;
    gchar *pszCfgFileName;


};

typedef struct _MainAppWindowPrivate MainAppWindowPrivate;

void do_aggiorna_db(gpointer win);
MainAppWindow *main_app_window_new (MainApp *app);
MainAppWindowPrivate *get_main_window_private_instance (gpointer win);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINAPPWIN_H */
