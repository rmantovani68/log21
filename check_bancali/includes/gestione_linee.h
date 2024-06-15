#ifndef __GESTIONE_LINEE_H
#define __GESTIONE_LINEE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define GESTIONE_LINEE_WINDOW_TYPE (gestione_linee_window_get_type ())
G_DECLARE_FINAL_TYPE (GestioneLineeWindow, gestione_linee_window, GESTIONE_LINEE, WINDOW, GtkDialog)


GestioneLineeWindow *gestione_linee_window_new (MainWindow *win);


#endif /* __GESTIONE_LINEE_H */
