/*
* InsertProdottoWindow declaration
*/
#pragma once

#ifndef __INSERT_PRODOTTO_H__
#define __INSERT_PRODOTTO_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define INSERT_PRODOTTO_WINDOW_TYPE (insert_prodotto_window_get_type ())
G_DECLARE_FINAL_TYPE (InsertProdottoWindow, insert_prodotto_window, INSERT_PRODOTTO, WINDOW, GtkDialog)

InsertProdottoWindow * insert_prodotto_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __INSERT_PRODOTTO_H__ */
