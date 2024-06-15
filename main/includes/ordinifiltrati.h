#ifndef __ORDINIFILTRATI_H
#define __ORDINIFILTRATI_H

#include <gtk/gtk.h>
#include "mainwin.h"

#define ORDINI_FILTRATI_WINDOW_TYPE (ordini_filtrati_get_type ())
G_DECLARE_FINAL_TYPE (OrdiniFiltratiWindow, ordini_filtrati, ORDINI_FILTRATI, WINDOW, GtkDialog)


OrdiniFiltratiWindow * ordini_filtrati_new (MainWindow *win);


#endif /* __ORDINIFILTRATI_H */
