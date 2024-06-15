#ifndef __EDITARTICOLO_H
#define __EDITARTICOLO_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_ARTICOLO_WINDOW_TYPE (edit_articolo_window_get_type ())
G_DECLARE_FINAL_TYPE (EditArticoloWindow, edit_articolo_window, EDIT_ARTICOLO, WINDOW, GtkDialog)


EditArticoloWindow *edit_articolo_window_new (MainWindow *win);


#endif /* __EDITARTICOLO_H */
