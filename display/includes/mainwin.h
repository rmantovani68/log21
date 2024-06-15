#ifndef __MAINWIN_H
#define __MAINWIN_H

#include <gtk/gtk.h>

#include <proc_list.h>
#include "mainapp.h"


#define MAIN_WINDOW_TYPE (main_window_get_type ())
G_DECLARE_FINAL_TYPE (MainWindow, main_window, MAIN, WINDOW, GtkApplicationWindow)

struct _MainWindowPrivate
{
    MainApp *app;
    int pid;
    int settore;
    gboolean maximized;
    gboolean fullscreen;

    GdkScreen *screen;
    GtkCssProvider *provider;

    gint nPID;
    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;

    gchar *pszAppName;
    gchar *pszAppTitle;
    gchar *pszAppClass;
    gchar *pszCfgFileName;
    gchar *pszDisplayCfgFileName;

    GtkWidget *lb_titolo_ordine_collo       ;
    GtkWidget *lb_ordine_collo              ;
    GtkWidget *lb_titolo_settore            ;
    GtkWidget *lb_settore                   ;
    GtkWidget *lb_stato_linea               ;
    GtkWidget *lb_titolo_operatore          ;
    GtkWidget *lb_codice_operatore          ;
    GtkWidget *lb_dati_operatore            ;
    GtkWidget *lb_operazione                ;
    GtkWidget *lb_copie                     ;
    GtkWidget *lb_titolo_ubicazione         ;
    GtkWidget *lb_ubicazione                ;
    GtkWidget *lb_titolo_codice             ;
    GtkWidget *lb_codice                    ;
    GtkWidget *lb_titolo_titolo             ;
    GtkWidget *lb_titolo                    ;
    GtkWidget *lb_stats_titolo_tabella      ;
    GtkWidget *lb_stats_titolo_ordini       ;
    GtkWidget *lb_stats_titolo_colli        ;
    GtkWidget *lb_stats_titolo_righe        ;
    GtkWidget *lb_stats_titolo_copie        ;
    GtkWidget *lb_stats_titolo_da_prelevare ;
    GtkWidget *lb_stats_ordini_da_prelevare ;
    GtkWidget *lb_stats_colli_da_prelevare  ;
    GtkWidget *lb_stats_righe_da_prelevare  ;
    GtkWidget *lb_stats_copie_da_prelevare  ;
    GtkWidget *lb_stats_titolo_prelevati    ;
    GtkWidget *lb_stats_ordini_prelevati    ;
    GtkWidget *lb_stats_colli_prelevati     ;
    GtkWidget *lb_stats_righe_prelevati     ;
    GtkWidget *lb_stats_copie_prelevati     ;

    GtkWidget *eb_stato_linea               ;
    GtkWidget *eb_operazione                ;
    GtkWidget *eb_copie                     ;

    GtkWidget *frame_dati_in_alto           ;
    GtkWidget *frame_operazione             ;
    GtkWidget *frame_variabile              ;
    GtkWidget *frame_dati_prodotto          ;
    GtkWidget *frame_statistiche            ;
    GtkWidget *frame_lista_colli            ;

};

typedef struct _MainWindowPrivate MainWindowPrivate;

MainWindowPrivate *get_main_window_private_instance (gpointer win);


MainWindow *main_window_new (MainApp *app, gint settore, gboolean maximize, gboolean fullscreen);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINWIN_H */
