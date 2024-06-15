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
    gchar *pszEditCfgFileName;
    gchar szTableName[128];

    GtkWidget *lb_table_name       ;
    GtkWidget *lb_table_description;
    GtkWidget *lb_table_rows       ;
    GtkWidget *txt_msgs            ;
    GtkWidget *cb_casesensitive;
    GtkWidget *cb_start        ;
    GtkWidget *sb_column       ;

    GtkWidget *stack_main;
    GtkWidget *sw_list;
    GtkWidget *box_lista;
    GtkWidget *sw_msgs;

    GtkWidget *pb_edit  ;
    GtkWidget *pb_print ;
    GtkWidget *pb_select;
    GtkWidget *pb_search;
    GtkWidget *pb_config;
    GtkWidget *pb_exit  ;
                                   
    GtkWidget *pbm_print                      ;
    GtkWidget *pbm_exit                       ;
    GtkWidget *pbm_ordini                     ;
    GtkWidget *pbm_colli_scartati             ;
    GtkWidget *pbm_storico_linee_di_spedizione;
    GtkWidget *pbm_storico_bancali            ;
    GtkWidget *pbm_storico_colli_su_bancale   ;
    GtkWidget *pbm_eventi                     ;
    GtkWidget *pbm_ubicazioni                 ;
    GtkWidget *pbm_prodotti                   ;
    GtkWidget *pbm_imballi                    ;
    GtkWidget *pbm_operatori                  ;
    GtkWidget *pbm_storico_ordini             ;
    GtkWidget *pbm_nota_linee                 ;
    GtkWidget *pbm_edit                       ;
    GtkWidget *pbm_about                      ;
    GtkWidget *pbm_help                       ;

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
