#ifndef __MAINWIN_H
#define __MAINWIN_H

#define MAIN_WINDOW_TYPE (main_window_get_type ())
G_DECLARE_FINAL_TYPE (MainWindow, main_window, MAIN, WINDOW, GtkApplicationWindow)


struct _MainWindow
{
    GtkApplicationWindow parent;
};


struct _MainWindowPrivate
{
    MainApp *app;

    GSettings *settings;

    WindowSizeInfo *wsi;

    GdkScreen *screen;

    GtkCssProvider *provider;

    gint nPID;
    gchar *pszAppName;
    gchar *pszAppTitle;
    gchar *pszAppClass;
    gchar *pszCfgFileName;


    PLINEA_STRUCT pDatiLinea;

    GtkWidget *pb_start;
    GtkWidget *pb_stop;
    GtkWidget *pb_search;
    GtkWidget *impianto_menu;
    GtkWidget *pbm_carica_stato;
    GtkWidget *pbm_salva_stato;
    GtkWidget *pbm_carica_ultimo_stato_salvato;
    GtkWidget *pixmap_master;
    GtkWidget *pixmap_stato_linea;
    GtkWidget *lb_stato_linea;
    GtkWidget *separator_1;
    GtkWidget *i0;
    GtkWidget *i1;

    GtkWidget *pbm_pulizia_dati_ricezione;
    GtkWidget *pbm_pulizia_dati_produzione;
    GtkWidget *pb_ricezione_dati;
    GtkWidget *pbm_spedizione_dati;
    GtkWidget *pbm_storicizzazione_dati;
    GtkWidget *pbm_configurazione;
    GtkWidget *pbm_setta_stato_ordine;
    GtkWidget *pixmap_rx;
    GtkWidget *lb_rx;
    GtkWidget *pixmap_tx;
    GtkWidget *lb_tx;
    GtkWidget *i2;
    GtkWidget *i5;

    GtkWidget *revealer_main_0;
    GtkWidget *revealer_main_1;
    GtkWidget *revealer_main_2;
    GtkWidget *revealer_main_3;
    GtkWidget *search_bar;
    GtkWidget *search_entry;

    GtkWidget *stack_main;
    GtkWidget *sw_list_ordini;
    GtkWidget *sw_msgs;
    GtkWidget *sw_impianto;
    GtkWidget *txt_msgs;

    GtkWidget *rb_lista_ordini;
    GtkWidget *rb_messaggi;
    GtkWidget *rb_impianto;

#ifdef SPOSTARE_IN_FUNZIONE
    DBSTRUCT *ptCorriere;        /* Export Dati Corriere */
    DBSTRUCT *ptCorrAgg;         /* Export Dati Corriere - Aggiuntivo */
    DBSTRUCT *ptCorrSDA;         /* Export Dati Corriere - SDA */

    DBSTRUCT *ptOrdProd;         /* Ordini di Produzione */
    DBSTRUCT *ptColProd;         /* Colli di Produzione */
    DBSTRUCT *ptRigProd;         /* Righe di Produzione */  
    DBSTRUCT *ptEvasi;           /* evasi */
    DBSTRUCT *ptContenutoColli;  /* contenuto colli */
    DBSTRUCT *ptFlussoCedola;    /* flusso cedola */
#endif
};

typedef struct _MainWindowPrivate MainWindowPrivate;

void do_aggiorna_db(gpointer win);
MainWindow *main_window_new (MainApp *app);
MainWindowPrivate *get_main_window_private_instance (gpointer win);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINWIN_H */
