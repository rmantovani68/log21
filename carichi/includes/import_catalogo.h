/*
* ImportCatalogoWindow declaration
*/
#pragma once

#ifndef __IMPORT_CATALOGO_H__
#define __IMPORT_CATALOGO_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define IMPORT_CATALOGO_WINDOW_TYPE (import_catalogo_window_get_type ())
G_DECLARE_FINAL_TYPE (ImportCatalogoWindow, import_catalogo_window, IMPORT_CATALOGO, WINDOW, GtkDialog)

ImportCatalogoWindow * import_catalogo_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __IMPORT_CATALOGO_H__ */
