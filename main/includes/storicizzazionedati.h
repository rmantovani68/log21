#ifndef __STORICIZZAZIONEDATI_H
#define __STORICIZZAZIONEDATI_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define STORICIZZAZIONE_DATI_WINDOW_TYPE (storicizzazione_dati_get_type ())
G_DECLARE_FINAL_TYPE (StoricizzazionedatiWindow, storicizzazione_dati, STORICIZZAZIONE_DATI, WINDOW, GtkDialog)


StoricizzazionedatiWindow *storicizzazione_dati_new          (MainWindow *win);


#endif /* __STORICIZZAZIONEDATI_H */
