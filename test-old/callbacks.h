/*
* callbacks.h
* Callbacks functions declaration
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autori : Roberto Mantovani
*          Stefano Tarroni
*/

#include <gtk/gtk.h>
#include "picking.h"

/* callback associate alla main_window */
gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) ;
void on_mb_main_activate_current (GtkMenuShell *menushell, gboolean force_hide, gpointer user_data);

/************************************
****** pulsanti di main_window ******
************************************/
void on_main_pb_config_clicked              (gpointer user_data, GtkButton *button);
void on_main_pb_exit_clicked                (gpointer user_data, GtkButton *button);
void on_main_pb_print_clicked               (gpointer user_data, GtkButton *button);
void on_main_pb_settori_clicked             (gpointer user_data, GtkButton *button);
void on_main_pb_test_clicked                (gpointer user_data, GtkButton *button); 

/************************************
********** main menu' ***************
************************************/
void on_pbm_print_activate                  (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_exit_activate                   (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_about_activate                  (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_aggiorna_activate               (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_configurazione_activate         (GtkMenuItem     *menuitem, gpointer         user_data);
void on_pbm_help_activate                   (GtkMenuItem     *menuitem, gpointer         user_data);

/* callback associate alla CList di main_window */
void on_clist_select_row          (GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);
void on_clist_unselect_row        (GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data);
void on_lst_sort_column_clicked   (GtkCList *clist, gint column, gpointer user_data);
                                  
/* funzioni di riutilizzo */
void do_about(void);
void do_exit(void);
void do_settori(GtkWidget *parent);
void create_table_settori(GtkWidget *dlg,GtkWidget *parent,int nRows,int nCols);
void refresh_table_settori(GtkWidget *dlg,BOOL bRefreshDB);
void do_lista_ubicazioni(GtkWidget *parent);
void do_test(void);

/* dlg_exit */
void on_dlg_exit_pb_ok_clicked(gpointer user_data, GtkButton *button);

/* dlg_config */
void on_dlg_config_pb_apply_clicked                (gpointer user_data, GtkButton *button);
void on_dlg_config_pb_ok_clicked                   (gpointer user_data, GtkButton *button);

/* dlg_test */
void on_dlg_test_rb_singolo_settore_toggled (GtkToggleButton *togglebutton, gpointer         user_data);
void on_dlg_test_pb_start_clicked              (gpointer user_data, GtkButton *button);
void on_dlg_test_pb_stop_clicked          (gpointer user_data, GtkButton *button);
void do_fine_test(gpointer user_data, GtkButton *button);

void on_dlg_test_pb_start_clicked           (gpointer user_data, GtkButton *button);

void on_dlg_test_pb_stop_clicked            (gpointer user_data, GtkButton *button); 
void on_dlg_test_pb_reset_clicked           (gpointer user_data, GtkButton *button); 
void on_dlg_test_rb_pulsante_simulato_toggled (GtkToggleButton *togglebutton, gpointer         user_data);
