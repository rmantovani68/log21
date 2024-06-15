#ifndef __MAINAPPSTORICIZZAZIONE_DATI_H
#define __MAINAPPSTORICIZZAZIONE_DATI_H

#define MAIN_APP_STORICIZZAZIONE_DATI_TYPE (main_app_storicizzazione_dati_get_type ())
G_DECLARE_FINAL_TYPE (MainAppStoricizzazioneDati, main_app_storicizzazione_dati, MAIN, APP_STORICIZZAZIONE_DATI, GtkDialog)


MainAppStoricizzazioneDati *main_app_storicizzazione_dati_new          (MainAppWindow *win);


#endif /* __MAINAPPSTORICIZZAZIONE_DATI_H */
