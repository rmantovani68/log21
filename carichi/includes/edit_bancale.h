/*
* EditBancaleWindow declaration
*/
#pragma once

#ifndef __EDIT_BANCALE_H__
#define __EDIT_BANCALE_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define EDIT_BANCALE_WINDOW_TYPE (edit_bancale_window_get_type ())
G_DECLARE_FINAL_TYPE (EditBancaleWindow, edit_bancale_window, EDIT_BANCALE, WINDOW, GtkDialog)

EditBancaleWindow * edit_bancale_window_new (GtkWindow *parent, gchar *cdudc, gboolean storico);

G_END_DECLS

#endif /* __EDIT_BANCALE_H__ */
