#ifndef __FINE_LINEA_H
#define __FINE_LINEA_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define FINE_LINEA_WINDOW_TYPE (fine_linea_window_get_type ())
G_DECLARE_FINAL_TYPE (FineLineaWindow, fine_linea_window, FINE_LINEA, WINDOW, GtkDialog)


FineLineaWindow *fine_linea_window_new (MainWindow *win);


#endif /* __FINE_LINEA_H */
