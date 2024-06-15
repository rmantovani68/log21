/*
* InsertQuantityWindow declaration
*/
#pragma once

#ifndef __INSERT_QUANTITY_H__
#define __INSERT_QUANTITY_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define INSERT_QUANTITY_WINDOW_TYPE (insert_quantity_window_get_type ())
G_DECLARE_FINAL_TYPE (InsertQuantityWindow, insert_quantity_window, INSERT_QUANTITY, WINDOW, GtkDialog)

InsertQuantityWindow * insert_quantity_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __INSERT_QUANTITY_H__ */
