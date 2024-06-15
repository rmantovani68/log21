#ifndef __MAINAPPSPEDIZIONE_DATI_H
#define __MAINAPPSPEDIZIONE_DATI_H

#define MAIN_APP_SPEDIZIONE_DATI_TYPE (main_app_spedizione_dati_get_type ())
G_DECLARE_FINAL_TYPE (MainAppSpedizioneDati, main_app_spedizione_dati, MAIN, APP_SPEDIZIONE_DATI, GtkDialog)

typedef enum {
    SPEDIZIONE_DATI,
    SPEDIZIONE_SDA,
    SPEDIZIONE_CORRIERE,
    SPEDIZIONE_CORR_AGG
} SpedizioneType;

MainAppSpedizioneDati * main_app_spedizione_dati_new (gpointer win, SpedizioneType type);


#endif /* __MAINAPPSPEDIZIONE_DATI_H */
