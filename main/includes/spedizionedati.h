#ifndef __SPEDIZIONEDATI_H
#define __SPEDIZIONEDATI_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define SPEDIZIONE_DATI_WINDOW_TYPE (spedizione_dati_get_type ())
G_DECLARE_FINAL_TYPE (SpedizioneDatiWindow, spedizione_dati, SPEDIZIONE_DATI, WINDOW, GtkDialog)

typedef enum {
    SPEDIZIONE_DATI,
    SPEDIZIONE_SDA,
    SPEDIZIONE_CORRIERE,
    SPEDIZIONE_CORR_AGG
} SpedizioneType;

SpedizioneDatiWindow * spedizione_dati_new (gpointer win, SpedizioneType type);


#endif /* __SPEDIZIONEDATI_H */
