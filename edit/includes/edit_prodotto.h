#ifndef __EDIT_PRODOTTO_H
#define __EDIT_PRODOTTO_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_PRODOTTO_WINDOW_TYPE (edit_prodotto_window_get_type ())
G_DECLARE_FINAL_TYPE (EditProdottoWindow, edit_prodotto_window, EDIT_PRODOTTO, WINDOW, GtkDialog)


EditProdottoWindow *edit_prodotto_window_new (MainWindow *win);

#endif /* __EDIT_PRODOTTO_H */
