#ifndef __MAINAPPSTAMPAORDINECOLLO_H
#define __MAINAPPSTAMPAORDINECOLLO_H

#include <gtk/gtk.h>
#include "mainwin.h"

typedef enum {
    STAMPA_ETICHETTE_ORDINE,
    STAMPA_PACKING_LIST_ORDINE,
    STAMPA_XAB_ORDINE,
    STAMPA_DETTAGLIO_ORDINE,
    STAMPA_DATI_ORDINE
} STAMPA_ORDINE_COLLO_TYPE_ENUM;

#define STAMPA_ORDINE_COLLO_WINDOW_TYPE (stampa_ordine_collo_window_get_type ())
G_DECLARE_FINAL_TYPE (StampaOrdineColloWindow, stampa_ordine_collo_window, STAMPA_ORDINE_COLLO, WINDOW, GtkDialog)


StampaOrdineColloWindow * stampa_ordine_collo_window_new (MainWindow *win, STAMPA_ORDINE_COLLO_TYPE_ENUM t, gchar *o);


#endif /* __MAINAPPSTAMPAORDINECOLLO_H */
