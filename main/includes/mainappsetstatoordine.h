#ifndef __MAINAPPSETSTATOORDINE_H
#define __MAINAPPSETSTATOORDINE_H

#define MAIN_APP_SET_STATO_ORDINE_TYPE (main_app_set_stato_ordine_get_type ())
G_DECLARE_FINAL_TYPE (MainAppSetStatoOrdine, main_app_set_stato_ordine, MAIN, APP_SET_STATO_ORDINE, GtkDialog)


MainAppSetStatoOrdine *main_app_set_stato_ordine_new          (MainAppWindow *win);


#endif /* __MAINAPPSET_STATO_ORDINE_H */
