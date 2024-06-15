#ifndef __MAINWIN_H
#define __MAINWIN_H

#include <gtk/gtk.h>

#include <gtk-support.h>
#include <proc_list.h>


#define MAIN_WINDOW_TYPE (main_window_get_type ())
G_DECLARE_FINAL_TYPE (MainWindow, main_window, MAIN, WINDOW, GtkApplicationWindow)


struct _MainWindow
{
    GtkApplicationWindow parent;
};


struct _MainWindowPrivate
{
    GtkApplication *app;

    WindowSizeInfo *wsi;

    GdkScreen *screen;
    GtkCssProvider *provider;

    GtkWidget *search_bar;
    GtkWidget *search_entry;
    GtkWidget *main_menu;
};

typedef struct _MainWindowPrivate MainWindowPrivate;

void do_aggiorna_db(gpointer win);
MainWindow *main_window_new (GtkApplication *app);
MainWindowPrivate *get_main_window_private_instance (gpointer win);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINWIN_H */
