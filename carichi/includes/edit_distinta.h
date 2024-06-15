/*
* EditDistintaWindow declaration
*/
#pragma once

#ifndef __EDIT_DISTINTA_H__
#define __EDIT_DISTINTA_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define EDIT_DISTINTA_WINDOW_TYPE (edit_distinta_window_get_type ())
G_DECLARE_FINAL_TYPE (EditDistintaWindow, edit_distinta_window, EDIT_DISTINTA, WINDOW, GtkDialog)

EditDistintaWindow * edit_distinta_window_new (GtkWindow *parent, gchar *nmdis, gboolean storico);

G_END_DECLS

#endif /* __EDIT_DISTINTA_H__ */
