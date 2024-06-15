#ifndef __EDIT_COLLO_H
#define __EDIT_COLLO_H

#include <gtk/gtk.h>
#include "mainwin.h"
#include "edit_ordine.h"


#define EDIT_COLLO_WINDOW_TYPE (edit_collo_window_get_type ())
G_DECLARE_FINAL_TYPE (EditColloWindow, edit_collo_window, EDIT_COLLO, WINDOW, GtkDialog)


EditColloWindow *edit_collo_window_new (EditOrdineWindow *win);

#endif /* __EDIT_COLLO_H */
