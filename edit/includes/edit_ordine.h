#ifndef __EDIT_ORDINE_H
#define __EDIT_ORDINE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_ORDINE_WINDOW_TYPE (edit_ordine_window_get_type ())
G_DECLARE_FINAL_TYPE (EditOrdineWindow, edit_ordine_window, EDIT_ORDINE, WINDOW, GtkDialog)


EditOrdineWindow *edit_ordine_window_new (MainWindow *win);

#endif /* __EDIT_ORDINE_H */
