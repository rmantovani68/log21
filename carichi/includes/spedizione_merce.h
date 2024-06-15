/*
* SpedizioneMerceWindow declaration
*/
#pragma once

#ifndef __SPEDIZIONE_MERCE_H__
#define __SPEDIZIONE_MERCE_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define SPEDIZIONE_MERCE_WINDOW_TYPE (spedizione_merce_window_get_type ())
G_DECLARE_FINAL_TYPE (SpedizioneMerceWindow, spedizione_merce_window, SPEDIZIONE_MERCE, WINDOW, GtkDialog)

SpedizioneMerceWindow * spedizione_merce_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __SPEDIZIONE_MERCE_H__ */
