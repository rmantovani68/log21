/*
* callbacks.h
* Callbacks functions declaration
* Easy Picking 3.0
* Copyright A&L srl 1999-2000
* Autori : Roberto Mantovani
*          Stefano Tarroni
*/

#include <gtk/gtk.h>

/* callback associate alla main_window */
gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) ;
void on_mb_main_activate_current (GtkMenuShell *menushell, gboolean force_hide, gpointer user_data);

/************************************
****** pulsanti di main_window ******
************************************/
void on_main_pb_config_clicked              (gpointer user_data, GtkButton *button);
void on_main_pb_exit_clicked                (gpointer user_data, GtkButton *button);
void on_main_pb_edit_clicked                (gpointer user_data, GtkButton *button);
void on_main_pb_select_clicked              (gpointer user_data, GtkButton *button);
void on_main_pb_search_clicked              (gpointer user_data, GtkButton *button);
void on_main_pb_print_clicked               (gpointer user_data, GtkButton *button);

/************************************
********** main menu' ***************
************************************/
void on_pbm_print_activate                        (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_exit_activate                         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_about_activate                        (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_aggiorna_lista_activate               (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_configurazione_activate               (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_edit_activate                         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_help_activate                         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_ordini_activate                       (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_opeset_activate                       (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_ubicazioni_activate                   (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_prodotti_activate                     (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_imballi_activate                      (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_operatori_activate                    (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_causali_di_trasporto_activate         (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_forzature_clienti_activate            (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_forme_di_pagamento_activate           (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_gruppi_cap_activate                   (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_linee_di_spedizione_activate          (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_linee_vettori_activate                (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_raggruppamenti_cap_activate           (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_tipi_di_spedizione_activate           (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_anagrafica_vettori_activate           (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_descrizione_linee_activate            (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_descrizione_linee_per_cedola_activate (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_storico_ordini_activate         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_storico_linee_di_spedizione_activate (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_storico_bancali_activate        (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_storico_colli_su_bancale_activate (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_colli_scartati_activate         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_eventi_activate                 (GtkMenuItem     *menuitem, gpointer         user_data);

/* callback associate alla CList di main_window */
void on_clist_select_row          (GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);
void on_clist_unselect_row        (GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);
void on_lst_sort_column_clicked   (GtkCList *clist, gint column, gpointer user_data);
                                  
/* funzioni di riutilizzo */
void show_exit_dlg(void);
void edit_clist_row(GtkCList *clist);

/* dlg_exit */
void on_dlg_exit_pb_ok_clicked(gpointer user_data, GtkButton *button);

/* dlg_config */
void on_dlg_config_pb_apply_clicked                (gpointer user_data, GtkButton *button);
void on_dlg_config_pb_ok_clicked                   (gpointer user_data, GtkButton *button);

/* dlg_edit_ordine */
void on_dlg_edit_ordine_pb_prev_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_edit_ordine_pb_next_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_edit_ordine_pb_edit_collo_clicked      (gpointer user_data, GtkButton *button);

/* dlg_edit_collo */
void on_dlg_edit_collo_pb_prev_clicked             (gpointer user_data, GtkButton *button);
void on_dlg_edit_collo_pb_next_clicked             (gpointer user_data, GtkButton *button);

/* dlg_catalogo */
void on_dlg_catalogo_pb_prev_clicked               (gpointer user_data, GtkButton *button); 
void on_dlg_catalogo_pb_modifica_clicked           (gpointer user_data, GtkButton *button); 
void on_dlg_catalogo_pb_next_clicked               (gpointer user_data, GtkButton *button);
void do_modifica_prodotto                          (gpointer user_data, GtkButton *button);
                                                   
/* dlg_imballi */                                  
void on_dlg_imballi_pb_elimina_clicked             (gpointer user_data, GtkButton *button); 
void on_dlg_imballi_pb_aggiungi_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_imballi_pb_modifica_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_imballi_pb_prev_clicked                (gpointer user_data, GtkButton *button);
void on_dlg_imballi_pb_next_clicked                (gpointer user_data, GtkButton *button);
void do_modifica_imballo                           (gpointer user_data, GtkButton *button);
void do_aggiungi_imballo                           (gpointer user_data, GtkButton *button);
void do_elimina_imballo                            (gpointer user_data, GtkButton *button);
                                                   
/* dlg_ubicazioni */                               
void on_dlg_ubicazioni_pb_prev_clicked             (gpointer user_data, GtkButton *button);
void on_dlg_ubicazioni_pb_next_clicked             (gpointer user_data, GtkButton *button);
void on_dlg_ubicazioni_pb_vuota_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_ubicazioni_pb_assegna_clicked          (gpointer user_data, GtkButton *button);
void on_dlg_ubicazioni_pb_abilitazione_clicked     (gpointer user_data, GtkButton *button);
void do_vuota_ubicazione                           (gpointer user_data, GtkButton *button);
void do_assegna_ubicazione                         (gpointer user_data, GtkButton *button);
void do_abilita_ubicazione                         (gpointer user_data, GtkButton *button);
void do_disabilita_ubicazione                      (gpointer user_data, GtkButton *button);

/* dlg_operatori */
void on_dlg_operatori_pb_elimina_clicked           (gpointer user_data, GtkButton *button);
void on_dlg_operatori_pb_modifica_clicked          (gpointer user_data, GtkButton *button);
void on_dlg_operatori_pb_inserisci_clicked         (gpointer user_data, GtkButton *button);
void on_dlg_operatori_pb_prev_clicked              (gpointer user_data, GtkButton *button);
void on_dlg_operatori_pb_next_clicked              (gpointer user_data, GtkButton *button);
void do_modifica_operatore                         (gpointer user_data, GtkButton *button);
void do_insert_operatore                           (gpointer user_data, GtkButton *button);
void do_elimina_operatore                          (gpointer user_data, GtkButton *button);

/* dlg_opeset */
void on_dlg_opeset_pb_vuota_clicked                (gpointer user_data, GtkButton *button); 
void on_dlg_opeset_pb_assegna_clicked              (gpointer user_data, GtkButton *button); 
void on_dlg_opeset_pb_prev_clicked                 (gpointer user_data, GtkButton *button);
void on_dlg_opeset_pb_next_clicked                 (gpointer user_data, GtkButton *button);

/* dlg_search */
void on_dlg_search_pb_cerca_clicked                (gpointer user_data, GtkButton *button);
void do_about(void);


void on_dlg_cedola_pb_ok_clicked            (gpointer user_data, GtkButton *button);

void on_dlg_edit_pb_prev_clicked        (gpointer user_data, GtkButton *button);
void on_dlg_edit_pb_next_clicked        (gpointer user_data, GtkButton *button);

void on_dlg_edit_pb_add_clicked        (gpointer user_data, GtkButton *button);
void on_dlg_edit_pb_del_clicked        (gpointer user_data, GtkButton *button);
void on_dlg_edit_pb_mod_clicked        (gpointer user_data, GtkButton *button);
void do_edit_add_record (gpointer user_data, GtkButton *button);
void do_edit_delete_record (gpointer user_data, GtkButton *button);
void do_edit_update_record (gpointer user_data, GtkButton *button);

/* dlg_select */
void do_dlg_select(void);
void on_dlg_expand_pb_ok_clicked (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_and_clicked           (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_expand_clicked        (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_ok_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_or_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_reset_sel_clicked     (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_select_all_clicked    (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_start_sel_clicked     (gpointer user_data, GtkButton *button);
void on_dlg_select_pb_stampa_clicked            (gpointer user_data, GtkButton *button);

void do_print(gpointer user_data, GtkButton *button);





void
on_pbm_descrizione_linee_generali_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pbm_nota_linee_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
