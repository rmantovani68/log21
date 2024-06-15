#ifndef __EDIT_SETTORE_H
#define __EDIT_SETTORE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_SETTORE_WINDOW_TYPE (edit_settore_window_get_type ())
G_DECLARE_FINAL_TYPE (EditSettoreWindow, edit_settore_window, EDIT_SETTORE, WINDOW, GtkDialog)


EditSettoreWindow *edit_settore_window_new (MainWindow *win);

#endif /* __EDIT_SETTORE_H */
