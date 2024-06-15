#ifndef __SETSTATOORDINE_H
#define __SETSTATOORDINE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define SET_STATO_ORDINE_WINDOW_TYPE (set_stato_ordine_window_get_type ())
G_DECLARE_FINAL_TYPE (SetStatoOrdineWindow, set_stato_ordine_window, SET_STATO_ORDINE, WINDOW, GtkDialog)


SetStatoOrdineWindow *set_stato_ordine_window_new          (MainWindow *win);


#endif /* __MAINAPPSET_STATO_ORDINE_H */
