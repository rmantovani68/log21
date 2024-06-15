/*
* InsertUdcWindow declaration
*/
#pragma once

#ifndef __INSERT_UDC_H__
#define __INSERT_UDC_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define INSERT_UDC_WINDOW_TYPE (insert_udc_window_get_type ())
G_DECLARE_FINAL_TYPE (InsertUdcWindow, insert_udc_window, INSERT_UDC, WINDOW, GtkDialog)

InsertUdcWindow * insert_udc_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __INSERT_UDC_H__ */
