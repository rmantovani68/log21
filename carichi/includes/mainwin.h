#ifndef __MAINWIN_H
#define __MAINWIN_H

#include <gtk/gtk.h>

#include <proc_list.h>
#include "mainstruct.h"
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

    GSettings *settings;
    WindowSizeInfo *wsi;

    GdkScreen *screen;

    GtkCssProvider *provider;

    /* serve per alcune funzioni di accesso al database */
    MACCHINA Macchina;

    gint nPID;

    gchar *pszAppName;
    gchar *pszAppTitle;
    gchar *pszAppClass;
    gchar *pszCfgFileName;

    GtkWidget *pb_crea_nuovi_udc;
    GtkWidget *pb_stampa_etichette;

    GtkWidget *txt_msgs;
    GtkWidget *search_bar;
    GtkWidget *search_entry;

    GtkWidget *rb_giacenze;
    GtkWidget *rb_movimenti;
    GtkWidget *rb_catalogo;
    GtkWidget *rb_reports;

    GtkWidget *stack_main;
    GtkWidget *box_giacenze;
    GtkWidget *box_movimenti;
    GtkWidget *box_catalogo;
    GtkWidget *box_reports;

    GtkWidget *stack_giacenze;
    GtkWidget *box_giacenze_totali;
    GtkWidget *box_giacenze_prodotti_udc;
    GtkWidget *box_giacenze_udc;
    GtkWidget *box_giacenze_distinte;

    GtkWidget *stack_reports;
    GtkWidget *box_reports_storico_giacenze;
    GtkWidget *box_reports_storico_movimenti;
    GtkWidget *box_reports_carichi_importati;

    GtkWidget *stack_report_giacenze;
    GtkWidget *box_report_giacenze_totali;
    GtkWidget *box_report_giacenze_prodotti_udc;
    GtkWidget *box_report_giacenze_udc;
    GtkWidget *box_report_giacenze_distinte;

    GtkWidget *giacenze_ricerca_entry_udc;
    GtkWidget *giacenze_ricerca_entry_prodotto;
    GtkWidget *giacenze_ricerca_entry_titolo;
    GtkWidget *movimenti_ricerca_entry_udc;
    GtkWidget *movimenti_ricerca_entry_prodotto;
    GtkWidget *movimenti_ricerca_entry_titolo;
    GtkWidget *giacenze_stor_ricerca_entry_udc;
    GtkWidget *giacenze_stor_ricerca_entry_prodotto;
    GtkWidget *giacenze_stor_ricerca_entry_titolo;
    GtkWidget *reports_movimenti_entry_udc;
    GtkWidget *reports_movimenti_entry_prodotto;
    GtkWidget *reports_movimenti_entry_titolo;
    GtkWidget *carichi_ricerca_entry_doc;
    GtkWidget *carichi_ricerca_entry_cdpro;
    GtkWidget *carichi_ricerca_entry_dstit;
    GtkWidget *movimenti_ricerca_cb_causale;
    GtkWidget *reports_movimenti_cb_causale;
    GtkWidget *movimenti_ricerca_pb_from_date;
    GtkWidget *movimenti_ricerca_pb_to_date;
    GtkWidget *reports_movimenti_pb_from_date;
    GtkWidget *reports_movimenti_pb_to_date;
    GtkWidget *giacenze_ricerca_pb_refresh;
    GtkWidget *giacenze_ricerca_pb_clear;
    GtkWidget *pb_giacenze_totali_ingresso;
    GtkWidget *pb_giacenze_totali_spedizione;
    GtkWidget *pb_giacenze_totali_conteggia;
    GtkWidget *pb_giacenze_totali_restituzione;
    GtkWidget *pb_giacenze_totali_differenza_inventariale;
    GtkWidget *pb_giacenze_totali_storicizza;
    GtkWidget *pb_giacenze_totali_edit_prodotto;
    GtkWidget *pb_giacenze_totali_stampa;
    GtkWidget *pb_prodotti_bancalati_stampa;
    GtkWidget *pb_prodotti_bancalati_elimina;
    GtkWidget *pb_prodotti_bancalati_edit_prodotto;
    GtkWidget *pb_bancali_edit_bancale;
    GtkWidget *pb_bancali_set_distinta;
    GtkWidget *pb_bancali_stampa_packing_list;
    GtkWidget *pb_bancali_elimina;
    GtkWidget *pb_distinte_crea;
    GtkWidget *pb_distinte_stampa;
    GtkWidget *pb_distinte_crea_file_dati;
    GtkWidget *pb_distinte_restituzione;
    GtkWidget *pb_distinte_storicizza;
    GtkWidget *pb_distinte_elimina;
    GtkWidget *pb_distinte_edit_distinta;
    GtkWidget *movimenti_ricerca_pb_refresh;
    GtkWidget *movimenti_ricerca_pb_clear;
    GtkWidget *pb_movimentazioni_ingresso;
    GtkWidget *pb_movimentazioni_spedizione;
    GtkWidget *pb_movimentazioni_conteggia;
    GtkWidget *pb_movimentazioni_restituzione;
    GtkWidget *pb_movimentazioni_differenza_inventariale;
    GtkWidget *pb_movimentazioni_annulla_movimento;
    GtkWidget *pb_movimentazioni_stampa;
    GtkWidget *pb_giacenze_stor_ricerca_refresh;
    GtkWidget *pb_giacenze_stor_ricerca_clear;
    GtkWidget *pb_prodotti_bancalati_stor_edit_prodotto;
    GtkWidget *pb_prodotti_bancalati_stor_satmpa;
    GtkWidget *pb_bancali_stor_edit_bancale;
    GtkWidget *pb_bancali_stor_stampa_packing_list;
    GtkWidget *pb_distinte_stor_crea_file_dati;
    GtkWidget *pb_distinte_stor_edit_distinta;
    GtkWidget *pb_distinte_stor_stampa_distinte;
    GtkWidget *reports_movimenti_pb_refresh;
    GtkWidget *reports_movimenti_pb_clear;
    GtkWidget *pb_carichi_ricerca_trova;
    GtkWidget *pb_carichi_ricerca_pulisci;
    GtkWidget *sb_column;
    GtkWidget *giacenze_ricerca_sb_distinta;
    GtkWidget *movimenti_ricerca_sb_distinta;
    GtkWidget *giacenze_stor_ricerca_sb_distinta;
    GtkWidget *reports_movimenti_sb_distinta;

    GtkWidget *sw_list_giacenze_totali;
    GtkWidget *sw_list_giacenze_prodotti_udc;
    GtkWidget *sw_list_giacenze_udc;
    GtkWidget *sw_list_giacenze_distinte;
    GtkWidget *sw_list_main_movimentazioni;
    GtkWidget *sw_list_main_catalogo;
    GtkWidget *sw_list_giacenze_stor_totali;
    GtkWidget *sw_list_giacenze_stor_prodotti_udc;
    GtkWidget *sw_list_giacenze_stor_udc;
    GtkWidget *sw_list_giacenze_stor_distinte;
    GtkWidget *sw_list_stor_movimenti;
    GtkWidget *sw_list_carichi_importati;

    GtkWidget *lb_giacenze_distinta ;
    GtkWidget *lb_giacenze_udc      ;
    GtkWidget *lb_giacenze_prodotto ;
    GtkWidget *lb_giacenze_dstit   ;

    GIACENZE_DATI_RICERCA_STRUCT GiacenzeDatiRicerca;
    GIACENZE_DATI_RICERCA_STRUCT GiacenzeStorDatiRicerca; 
    GIACENZE_DATI_RICERCA_STRUCT MovimentiDatiRicerca;
    GIACENZE_DATI_RICERCA_STRUCT CarichiDatiRicerca;


};

typedef struct _MainWindowPrivate MainWindowPrivate;

void do_aggiorna_db(gpointer win);
MainWindow *main_window_new (MainApp *app);
MainWindowPrivate *get_main_window_private_instance (gpointer win);

#define TREE_VIEW_LST(widget)       (GTK_TREE_VIEW(widget))
#define TREE_SELECTION_LST(widget)  (GTK_TREE_SELECTION((gtk_tree_view_get_selection(TREE_VIEW_LST(widget)))))
#define TREE_MODEL_LST(widget)      (GTK_TREE_MODEL((gtk_tree_view_get_model(TREE_VIEW_LST(widget)))))

#endif /* __MAINWIN_H */
