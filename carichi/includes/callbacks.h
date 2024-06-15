/*
* callbacks.h
* Callbacks functions declaration
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autori : Roberto Mantovani
*          Stefano Tarroni
*/

#include <gtk/gtk.h>
#include "picking.h"

/* callbcaks.c */
void UserGetCfgItems(GtkWidget *dlg);
void UserSetCfgItems(GtkWidget *dlg);
void do_about(void);
void do_aggiorna_db (void);
void do_config(void);
void do_dlg_spedizione_dati(GtkWidget *parent);
void do_exit(void);
ep_bool_t do_ricezione_dati(void);
void do_spedizione_dati(GtkButton *button, gpointer user_data);

void on_main_pb_exit_clicked(GtkButton * button, gpointer user_data);

gboolean on_main_window_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) ;
gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) ;

void on_pbm_about_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_aggiorna_archivi_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_aggiorna_lista_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_configurazione_activate(GtkMenuItem * menuitem, gpointer user_data) ;
void on_pbm_deseleziona_tutto_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_distribuzione_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_exit_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_help_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_ricezione_dati_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_seleziona_tutto_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_spedizione_dati_activate(GtkMenuItem * menuitem, gpointer user_data);
/* mainsel.c */
void do_dlg_select(void);
void do_start_selezione(GtkWidget *dlg);

void
on_pbm_gestione_a_bancale_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_item1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pbm_gestione_in_linea_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_pbm_ricezione_catalogo_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_nb_main_switch_page                 (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
on_pb_movimentazioni_ingresso_clicked  (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_spedizione_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_restituzione_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);


void
on_dlg_ingresso_merce_pb_genera_codice_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_dlg_inserimento_distinta_pb_genera_distinta_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_catalogo_edit_prodotto_clicked   (GtkButton       *button,
                                        gpointer         user_data);


void
on_pbm_crea_nuovi_udc_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pb_movimentazioni_ricezione_spedizioni_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void on_dlg_importazione_spedizioni_pb_importa_clicked (gpointer         user_data, GtkButton       *button);


void
on_dlg_importazione_catalogo_pb_importa_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_dlg_importazione_catalogo_pb_set_codice_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_catalogo_importa_clicked         (GtkButton       *button,
                                        gpointer         user_data);


void
on_dlg_importazione_catalogo_pb_ricezione_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

//void
//on_dlg_importazione_cedola_pb_importa_clicked
//                                        (GtkButton       *button,
//                                        gpointer         user_data);


void
on_dlg_importazione_spedizioni_pb_aggiungi_cedola_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_dlg_importazione_spedizioni_pb_cancella_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_dlg_importazione_spedizioni_pb_edit_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);


void
on_nb_giacenze_switch_page             (GtkNotebook     *notebook,
                                        GtkNotebookPage *page,
                                        guint            page_num,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_conteggia_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_restituzione_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_bancali_set_distinta_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_crea_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_stampa_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_bancali_stampa_packing_list_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_ricerca_trova_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_ricerca_pulisci_clicked (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_differenza_inventariale_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_conteggia_clicked (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_ingresso_clicked (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_spedizione_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_ricezione_spedizioni_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);


void
on_pb_giacenze_totali_differenza_inventariale_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_ricerca_trova_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_movimentazioni_ricerca_pulisci_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);


void
on_pb_distinte_crea_file_dati_clicked  (GtkButton       *button,
                                        gpointer         user_data);


void
on_pb_catalogo_converti_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);


void
on_pb_movimentazioni_annulla_movimento_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_storicizza_clicked      (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_storicizza_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_giacenze_ricerca_cb_distinte_storico_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_catalogo_aggiungi_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_catalogo_storicizza_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_elimina_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_pbm_stampa_etichetta_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pb_movimentazioni_stampa_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_stampa_clicked   (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_prodotti_bancalati_stampa_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_prodotti_bancalati_elimina_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_bancali_elimina_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void on_pb_edit_prodotto_modifica_identificativi_clicked (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_modifica_dati_clicked (gpointer         user_data,GtkButton       *button);

void
on_pb_edit_prodotto_visualizza_movimentazioni_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void on_pb_edit_prodotto_precedente_clicked (gpointer         user_data,GtkButton       *button);
void on_pb_edit_prodotto_successivo_clicked (gpointer         user_data, GtkButton       *button );

void
on_pb_giacenze_totali_edit_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);
void onIterChanged(gpointer data);
void onTreeViewChanged(gpointer data);
void onParentDlgAssociatoChanged(gpointer data);

void on_pb_edit_bancale_modifica_dati_clicked (GtkButton       *button, gpointer         user_data);



void on_pb_edit_bancale_precedente_clicked  (gpointer         user_data,GtkButton       *button);
void on_pb_edit_bancale_successivo_clicked  (gpointer         user_data,GtkButton       *button);
void on_pb_bancali_edit_bancale_clicked     (GtkButton       *button, gpointer         user_data);

void on_pb_edit_bancale_edit_prodotto_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_distinta_edit_bancale_clicked (gpointer         user_data,GtkButton       *button);
void on_pb_edit_distinta_precedente_clicked  (gpointer         user_data,GtkButton       *button);
void on_pb_edit_distinta_successivo_clicked  (gpointer         user_data,GtkButton       *button);
void on_pb_distinte_edit_distinta_clicked   (GtkButton       *button, gpointer         user_data);

void on_main_pb_edit_clicked                (GtkToolButton   *toolbutton, gpointer         user_data);

void on_pb_prodotti_bancalati_edit_prodotto_clicked (GtkButton       *button, gpointer         user_data);

void on_pb_catalogo_allinea_clicked         (GtkButton       *button, gpointer         user_data);


void on_pb_edit_prodotto_ingresso_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_spedizione_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_conteggia_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_differenza_inventariale_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_annulla_movimento_clicked  (gpointer         user_data,GtkButton       *button);

void on_pb_edit_prodotto_stampa_clicked  (gpointer         user_data,GtkButton       *button);

void
on_pb_giacenze_stor_ricerca_trova_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_stor_ricerca_pulisci_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_stor_edit_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_giacenze_totali_stor_stampa_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_prodotti_bancalati_stor_edit_prodotto_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_prodotti_bancalati_stor_stampa_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_bancali_stor_edit_bancale_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_bancali_stor_stampa_packing_list_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_stor_crea_file_dati_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_stor_edit_distinta_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_distinte_stor_stampa_clicked     (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_crea_udc_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_stampa_etichette_clicked         (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_about_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_exit_clicked                     (GtkButton       *button,
                                        gpointer         user_data);
ep_bool_t do_crea_udc(void);
ep_bool_t do_stampa_etichette(void);

void
on_pb_carichi_ricerca_trova_clicked    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pb_carichi_ricerca_pulisci_clicked  (GtkButton       *button,
                                        gpointer         user_data);
