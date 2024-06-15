/*
* Callbacks.h
* Modulo: Bilancia
* Easy Picking 3.0 - Wella Rebuild
* Copyright A&L srl 2003-2021
* Autori : Roberto Mantovani 
*          Stefano Tarroni
* Data Creazione : 16-10-2000 
*/

#include <gtk/gtk.h>

/*
* MAIN WINDOW
*/
gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data);

/*
* TOOLBAR BUTTONS
*/
void on_main_window_pb_exit_clicked            (gpointer user_data, GtkButton *button);
void on_main_window_pb_edit_clicked            (gpointer user_data, GtkButton *button);
void on_main_window_pb_settings_clicked        (gpointer user_data, GtkButton *button);
void on_main_window_pb_help_clicked            (gpointer user_data, GtkButton *button);

/*
* MENUBAR OPTIONS
*/
void on_pbm_rettifica_collo_activate     (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_aggiungi_collo_activate      (GtkMenuItem *menuitem, gpointer user_data); 
void on_pbm_elimina_collo_activate       (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_exit_activate                (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_configurazione_activate      (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_about_activate               (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_debug_variabili_plc_activate (GtkMenuItem *menuitem, gpointer user_data);

/*
* DLG_EXIT
*/
void on_dlg_exit_pb_ok_clicked           (gpointer user_data, GtkButton *button);
void on_dlg_exit_pb_cancel_clicked       (gpointer user_data, GtkButton *button);

/*
* DLG_MESSAGE
*/ 
void on_dlg_message_pb_ok_clicked        (gpointer user_data, GtkButton *button);
void on_dlg_message_pb_cancel_clicked    (gpointer user_data, GtkButton *button);

/*
* DLG_APP_ERROR
*/
void on_dlg_app_error_pb_ok_clicked      (gpointer user_data, GtkButton *button);

/*
* DLG_CONFIG
*/
void on_dlg_config_pb_ok_clicked                     (gpointer user_data, GtkButton *button);
void on_dlg_config_pb_apply_clicked                  (gpointer user_data, GtkButton *button);


/*
* ORDINAMENTO COLONNE
*/
void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data);


/*
* Altre callbacks ...
*/


void do_about(void);
void do_exit(void);
void do_config(void);
void do_scarti(void);
void do_num_copie(void);
void do_storico_scarti(void);
void do_trace(void);
void do_reindex(GtkButton *button, gpointer user_data);
void do_delete_barcode(GtkButton *button, gpointer user_data);
void do_add_barcode(GtkButton *button, gpointer user_data);
void do_modify_barcode(GtkButton *button, gpointer user_data);
void do_print_storico_scarti(GtkButton *button, gpointer user_data);
void do_print_scarti(GtkButton *button, gpointer user_data);
int do_check_barcode(void);
void do_message(GtkWidget *parent,char *szTitle,char *szMessage,GtkSignalFunc func);
void on_button_pressed_destroy(gpointer user_data, GtkButton *button);
void do_refresh_table_scarti(GtkWidget *dlg);
void do_barcode(void);
void do_flags(void);

void on_pbm_trace_activate                   (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_reindex_activate                 (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_tabella_scarti_activate          (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_storico_scarti_activate          (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_barcode_activate                 (GtkMenuItem *menuitem, gpointer user_data);;
void on_pbm_tabella_flags_activate           (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_cancella_tabella_scarti_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_pbm_cancella_storico_scarti_activate (GtkMenuItem *menuitem, gpointer user_data);

void on_main_window_pb_scarti_clicked  (gpointer user_data, GtkButton *button);
void on_main_window_pb_num_copie_clicked    (gpointer user_data, GtkButton *button);

void on_dlg_scarti_pb_add_clicked      (gpointer user_data, GtkButton *button);
void on_dlg_scarti_pb_modify_clicked   (gpointer user_data, GtkButton *button);
void on_dlg_scarti_pb_delete_clicked   (gpointer user_data, GtkButton *button);
void on_dlg_scarti_pb_close_clicked    (gpointer user_data, GtkButton *button);
gboolean dlg_scarti_do_close_event     (GtkWidget *widget, GdkEvent *event, gpointer user_data);
void on_dlg_scarti_pb_print_clicked                      (gpointer user_data, GtkButton *button); 
void on_dlg_storico_scarti_pb_print_clicked              (gpointer user_data, GtkButton *button); 
void on_dlg_storico_scarti_pb_select_date_range_clicked  (gpointer user_data, GtkButton *button);
void on_dlg_storico_scarti_pb_refresh_clicked            (gpointer user_data, GtkButton *button);
void on_dlg_select_date_range_cal_from_date_day_selected (GtkCalendar *calendar, gpointer user_data);
void on_dlg_select_date_range_cal_to_date_day_selected   (GtkCalendar *calendar, gpointer user_data); 
void on_dlg_select_date_range_pb_ok_clicked              (gpointer user_data, GtkButton *button); 
void on_dlg_storico_scarti_pb_refresh_clicked            (gpointer user_data, GtkButton *button);

void on_dlg_config_cb_postscript_toggled    (gpointer user_data, GtkToggleButton *togglebutton);
void on_dlg_scarti_pb_close_clicked         (gpointer user_data, GtkButton *button);
void on_main_window_pb_barcode_clicked      (gpointer user_data, GtkButton *button);
void on_dlg_barcode_pb_ok_clicked           (gpointer user_data, GtkButton *button);


void on_dlg_scarti_entry_barcode_activate   (GtkEditable     *editable, gpointer         user_data);

void on_dlg_flags_pb_add_clicked            (gpointer user_data, GtkButton *button);

void on_dlg_flags_pb_modify_clicked         (gpointer user_data, GtkButton *button);

void on_dlg_flags_pb_delete_clicked         (gpointer user_data, GtkButton *button);

void on_dlg_flags_pb_print_clicked          (gpointer user_data, GtkButton *button);

void on_dlg_flags_pb_close_clicked          (gpointer user_data, GtkButton *button);

void do_print_scarti_flags(GtkButton *button, gpointer user_data);
void do_add_scarti_flags(GtkButton *button, gpointer user_data);
void do_mod_scarti_flags(GtkButton *button, gpointer user_data);
void do_del_scarti_flags(GtkButton *button, gpointer user_data);

gboolean dlg_flags_do_close_event               (GtkWidget       *widget, GdkEvent        *event, gpointer         user_data);

void on_combo_colore_changed                (GtkEditable     *editable, gpointer         user_data);
void on_lst_scarti_flags_select_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);
void on_lst_scarti_flags_unselect_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);


void do_delete_tabella_storico_scarti(GtkButton *button, gpointer user_data);
void do_delete_tabella_scarti(GtkButton *button, gpointer user_data);

gboolean dlg_work_do_close_event                (GtkWidget       *widget, GdkEvent        *event, gpointer         user_data); 
void on_combo_file_entry_changed            (GtkEditable     *editable, gpointer         user_data); 
void on_dlg_work_pb_ok_clicked              (gpointer user_data, GtkButton *button); 
void on_dlg_work_pb_close_clicked           (gpointer user_data, GtkButton *button); 
void on_pbm_import_dati_activate            (GtkMenuItem     *menuitem, gpointer         user_data); 
void on_pbm_export_dati_activate            (GtkMenuItem     *menuitem, gpointer         user_data);
void do_import(void);
void do_export(void);

void on_rb_posizioni_fisse_toggled          (gpointer user_data, GtkToggleButton *togglebutton);
void on_rb_campi_delimitati_toggled         (gpointer user_data, GtkToggleButton *togglebutton);
void do_import_dati(GtkButton *button, gpointer user_data);
void do_export_dati(GtkButton *button, gpointer user_data);
void do_confirm_import_dati(GtkButton *button, gpointer user_data);
void do_confirm_export_dati(GtkButton *button, gpointer user_data);
void do_import_dati(GtkButton *button, gpointer user_data);
void do_export_dati(GtkButton *button, gpointer user_data);


void on_dlg_num_copie_pb_ok_clicked           (gpointer user_data, GtkButton *button);
