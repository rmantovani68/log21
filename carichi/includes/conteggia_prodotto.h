/*
* ConteggiaProdottoWindow declaration
*/
#pragma once

#ifndef __CONTEGGIA_PRODOTTO_H__
#define __CONTEGGIA_PRODOTTO_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define CONTEGGIA_PRODOTTO_WINDOW_TYPE (conteggia_prodotto_window_get_type ())
G_DECLARE_FINAL_TYPE (ConteggiaProdottoWindow, conteggia_prodotto_window, CONTEGGIA_PRODOTTO, WINDOW, GtkDialog)

ConteggiaProdottoWindow * conteggia_prodotto_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __CONTEGGIA_PRODOTTO_H__ */
