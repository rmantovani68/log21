/*
* EditProdottoWindow declaration
*/
#pragma once

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define EDIT_PRODOTTO_WINDOW_TYPE (edit_prodotto_window_get_type ())
G_DECLARE_FINAL_TYPE (EditProdottoWindow, edit_prodotto_window, EDIT_PRODOTTO, WINDOW, GtkDialog)

EditProdottoWindow * edit_prodotto_window_new (GtkWindow *parent, gchar *cdpro, gboolean storico);

G_END_DECLS
