/*
* IngressoMerceWindow declaration
*/
#pragma once

#ifndef __INGRESSO_MERCE_H__
#define __INGRESSO_MERCE_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define INGRESSO_MERCE_WINDOW_TYPE (ingresso_merce_window_get_type ())
G_DECLARE_FINAL_TYPE (IngressoMerceWindow, ingresso_merce_window, INGRESSO_MERCE, WINDOW, GtkDialog)

IngressoMerceWindow * ingresso_merce_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __INGRESSO_MERCE_H__ */
