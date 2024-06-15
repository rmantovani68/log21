#ifndef __EDIT_IMBALLO_H
#define __EDIT_IMBALLO_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_IMBALLO_WINDOW_TYPE (edit_imballo_window_get_type ())
G_DECLARE_FINAL_TYPE (EditImballoWindow, edit_imballo_window, EDIT_IMBALLO, WINDOW, GtkDialog)


EditImballoWindow *edit_imballo_window_new (MainWindow *win);

#endif /* __EDIT_IMBALLO_H */
