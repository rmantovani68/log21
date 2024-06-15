/*
* ImportSpedizioniWindow declaration
*/
#pragma once

#ifndef __IMPORT_SPEDIZIONI_H__
#define __IMPORT_SPEDIZIONI_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define IMPORT_SPEDIZIONI_WINDOW_TYPE (import_spedizioni_window_get_type ())
G_DECLARE_FINAL_TYPE (ImportSpedizioniWindow, import_spedizioni_window, IMPORT_SPEDIZIONI, WINDOW, GtkDialog)

ImportSpedizioniWindow * import_spedizioni_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __IMPORT_SPEDIZIONI_H__ */
