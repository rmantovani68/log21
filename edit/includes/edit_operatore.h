#ifndef __EDIT_OPERATORE_H
#define __EDIT_OPERATORE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_OPERATORE_WINDOW_TYPE (edit_operatore_window_get_type ())
G_DECLARE_FINAL_TYPE (EditOperatoreWindow, edit_operatore_window, EDIT_OPERATORE, WINDOW, GtkDialog)


EditOperatoreWindow *edit_operatore_window_new (MainWindow *win);

#endif /* __EDIT_OPERATORE_H */
