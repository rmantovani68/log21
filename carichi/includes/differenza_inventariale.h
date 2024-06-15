/*
* DifferenzaInventarialeWindow declaration
*/
#pragma once

#ifndef __DIFFERENZA_INVENTARIALE_H__
#define __DIFFERENZA_INVENTARIALE_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define DIFFERENZA_INVENTARIALE_WINDOW_TYPE (differenza_inventariale_window_get_type ())
G_DECLARE_FINAL_TYPE (DifferenzaInventarialeWindow, differenza_inventariale_window, DIFFERENZA_INVENTARIALE, WINDOW, GtkDialog)

DifferenzaInventarialeWindow * differenza_inventariale_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __DIFFERENZA_INVENTARIALE_H__ */
