#ifndef __MAINAPPSTAMPAORDINECOLLO_H
#define __MAINAPPSTAMPAORDINECOLLO_H

typedef enum {
    STAMPA_ETICHETTE_ORDINE,
    STAMPA_PACKING_LIST_ORDINE,
    STAMPA_XAB_ORDINE,
    STAMPA_DETTAGLIO_ORDINE,
    STAMPA_DATI_ORDINE
} STAMPA_ORDINE_COLLO_TYPE_ENUM;

#define MAIN_APP_STAMPA_ORDINE_COLLO_TYPE (main_app_stampa_ordine_collo_get_type ())
G_DECLARE_FINAL_TYPE (MainAppStampaOrdineCollo, main_app_stampa_ordine_collo, MAIN, APP_STAMPA_ORDINE_COLLO, GtkDialog)


MainAppStampaOrdineCollo * main_app_stampa_ordine_collo_new (MainAppWindow *win, STAMPA_ORDINE_COLLO_TYPE_ENUM t, gchar *o);


#endif /* __MAINAPPSTAMPAORDINECOLLO_H */
