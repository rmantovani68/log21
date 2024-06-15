#ifndef __MAINWIN_H
#define __MAINWIN_H

#include <gtk/gtk.h>

#include <proc_list.h>
#include "mainapp.h"


#define MAIN_WINDOW_TYPE (main_window_get_type ())
G_DECLARE_FINAL_TYPE (MainWindow, main_window, MAIN, WINDOW, GtkApplicationWindow)


struct _MainWindow
{
    GtkApplicationWindow parent;
};


struct _MainWindowPrivate
{
    MainApp *app;

    gint nFaseOld;
    gint nFase;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;

    GdkScreen *screen;
    GtkCssProvider *provider;

    gint nPID;
    gchar *pszAppName;
    gchar *pszAppTitle;
    gchar *pszAppClass;
    gchar *pszCfgFileName;

    GtkWidget *lb_numero_operatori   ;
    GtkWidget *lb_media_colli        ;
    GtkWidget *lb_media_righe        ;
    GtkWidget *lb_media_copie        ;
    GtkWidget *lb_media_scarti       ;
    GtkWidget *lb_media_operatori    ;
    GtkWidget *pb_settori            ;
    GtkWidget *pb_statistica_evasione;
    GtkWidget *pb_operatori          ;
    GtkWidget *pb_imballi            ;
    GtkWidget *pb_config             ;
    GtkWidget *pb_exit               ;

    GtkWidget *search_bar;
    GtkWidget *search_entry;
};

typedef struct _MainWindowPrivate MainWindowPrivate;

MainWindow *main_window_new (MainApp *app);
MainWindowPrivate *get_main_window_private_instance (gpointer win);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINWIN_H */
