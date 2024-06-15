#ifndef __EDIT_UBICAZIONE_H
#define __EDIT_UBICAZIONE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define EDIT_UBICAZIONE_WINDOW_TYPE (edit_ubicazione_window_get_type ())
G_DECLARE_FINAL_TYPE (EditUbicazioneWindow, edit_ubicazione_window, EDIT_UBICAZIONE, WINDOW, GtkDialog)


EditUbicazioneWindow *edit_ubicazione_window_new (MainWindow *win);

#endif /* __EDIT_UBICAZIONE_H */
