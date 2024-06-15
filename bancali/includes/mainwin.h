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

    GtkWidget *lb_titolo;
    GtkWidget *lb_BOLLA;
    GtkWidget *lb_COLLO;
    GtkWidget *lb_risultato;
    GtkWidget *lb_PESOCALC_TIT;
    GtkWidget *lb_PESOCALC;
    GtkWidget *lb_PESOREA_TIT;
    GtkWidget *lb_PESOREA;
    GtkWidget *lb_PESODIFF_TIT;
    GtkWidget *lb_PESODIFF;
    GtkWidget *lb_volume_imb;
    GtkWidget *lb_volume_uti;
    GtkWidget *lb_VOLUME;
    GtkWidget *lb_VOL_UTI;
    GtkWidget *lb_VOLUME_P;
    GtkWidget *lb_VOL_P_UTI;
    GtkWidget *lb_perc_imb;
    GtkWidget *lb_perc_uti;
    GtkWidget *lb_peso;
    GtkWidget *lb_tara;
    GtkWidget *lb_PSPRE;
    GtkWidget *lb_TARA;
    GtkWidget *lb_ordine;
    GtkWidget *lb_collo;
    GtkWidget *lb_copie;
    GtkWidget *lb_cliente;
    GtkWidget *lb_formato;
    GtkWidget *lb_righe;
    GtkWidget *lb_PRDOC;
    GtkWidget *lb_NMCOL;
    GtkWidget *lb_NMCPE;
    GtkWidget *lb_CDRID;
    GtkWidget *lb_SWCOL;
    GtkWidget *lb_NMRGH;
    GtkWidget *lb_evadi;
    GtkWidget *entry_BOLLA;
    GtkWidget *entry_COLLO;

    GtkWidget *pb_cerca;
    GtkWidget *pb_evadi;
    GtkWidget *pb_stampa;
    GtkWidget *pb_bilancia;
    GtkWidget *pb_conf;
    GtkWidget *pb_info;
    GtkWidget *pb_clear;
    GtkWidget *pb_exit;

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
