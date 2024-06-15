/*
* InsertDistintaWindow declaration
*/
#pragma once

#ifndef __INSERT_DISTINTA_H__
#define __INSERT_DISTINTA_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define INSERT_DISTINTA_WINDOW_TYPE (insert_distinta_window_get_type ())
G_DECLARE_FINAL_TYPE (InsertDistintaWindow, insert_distinta_window, INSERT_DISTINTA, WINDOW, GtkDialog)

InsertDistintaWindow * insert_distinta_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __INSERT_DISTINTA_H__ */
