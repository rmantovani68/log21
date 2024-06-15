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

gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) ;
void on_mb_main_activate_current (GtkMenuShell *menushell, gboolean force_hide, gpointer user_data);
void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data);

void do_about(void);
void do_aggiorna_lista(void);
void do_carico_settori(GtkWidget *dlg,int nTipoEquilibratura);
void do_stampa(gpointer user_data, GtkButton *button);
void do_conferma_equilibratura(gpointer user_data, GtkButton *button);
void do_create_carico_settori (GtkWidget *parent,GtkWidget *container);
void do_dlg_equilibratura(void);
void do_dlg_edit_articolo(GtkWidget *dlg,char *szKey);
void do_exit(void);
void do_lista_articoli(void);
void do_lista_articoli_cedola(gpointer user_data, GtkButton *button);
void do_lista_cedole(void);
void on_dlg_app_error_pb_ok_clicked(gpointer user_data, GtkButton *button);
void on_dlg_equilibratura_pb_conferma_clicked (gpointer user_data, GtkButton *button);
void on_dlg_equilibratura_pb_equilibra_clicked (gpointer user_data, GtkButton *button);
void on_dlg_equilibratura_pb_refresh_clicked (gpointer user_data, GtkButton *button);
void on_dlg_equilibratura_rb_pezzi_toggled (gpointer user_data, GtkToggleButton *togglebutton);
void on_dlg_equilibratura_rb_righe_toggled (gpointer user_data, GtkToggleButton *togglebutton);
void on_dlg_config_pb_apply_clicked(gpointer user_data, GtkButton *button);
void on_dlg_config_pb_ok_clicked(gpointer user_data, GtkButton *button);
void on_dlg_edit_articolo_pb_assegna_ubicazione_clicked (gpointer user_data, GtkButton *button);
void on_dlg_edit_articolo_pb_calcolo_colli_pcf_clicked (gpointer user_data, GtkButton *button);
void on_dlg_edit_articolo_pb_next_clicked(gpointer user_data, GtkButton *button); 
void on_dlg_edit_articolo_pb_ok_clicked(gpointer user_data, GtkButton *button); 
void on_dlg_edit_articolo_pb_prev_clicked(gpointer user_data, GtkButton *button); 
void on_dlg_edit_articolo_pb_record_clicked(gpointer user_data, GtkButton *button); 
void on_dlg_edit_articolo_pb_vuota_ubicazione_clicked (gpointer user_data, GtkButton *button);
void on_dlg_exit_pb_ok_clicked(gpointer user_data, GtkButton *button);
void on_dlg_message_pb_ok_clicked (gpointer user_data, GtkButton *button);
void on_dlg_stampa_pb_stampa_clicked (gpointer user_data, GtkButton *button);
void on_main_pb_articoli_clicked(gpointer user_data, GtkButton *button); 
void on_main_pb_cedole_clicked(gpointer user_data, GtkButton *button); 
void on_main_pb_conferma_equilibratura_clicked (gpointer user_data, GtkButton *button);
void on_main_pb_config_clicked(gpointer user_data, GtkButton *button);
void on_main_pb_edit_clicked(gpointer user_data, GtkButton *button); 
void on_main_pb_lav_clicked(gpointer user_data, GtkButton *button);
void on_main_pb_equilibra_clicked(gpointer user_data, GtkButton *button); 
void on_main_pb_exit_clicked(gpointer user_data, GtkButton *button);
void on_main_pb_help_clicked(gpointer user_data, GtkButton *button);
void on_pbm_about_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_aggiorna_lista_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_carico_settori_activate         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_configurazione_activate(GtkMenuItem * menuitem, gpointer user_data) ;
void on_pbm_exit_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_help_activate(GtkMenuItem * menuitem, gpointer user_data);
void on_pbm_lista_articoli_activate(GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_lista_cedole_activate(GtkMenuItem     *menuitem, gpointer         user_data);
void on_dlg_edit_articolo_pb_cerca_ubicazione_clicked (gpointer user_data, GtkButton *button); 
void on_dlg_ubicazione_pb_ok_clicked        (gpointer user_data, GtkButton *button); 
void on_main_pb_stampa_clicked              (gpointer user_data, GtkButton *button); 
void on_dlg_edit_articolo_pb_calcolo_colli_pcf_clicked (gpointer user_data, GtkButton *button);
void on_dlg_equilibratura_cb_tutti_settori_magazzino_toggled (gpointer user_data, GtkToggleButton *togglebutton);

int CambiaFlagLavorazione(char *szCodProd);
