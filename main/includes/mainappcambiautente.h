#ifndef __MAINAPPCAMBIAUTENTE_H
#define __MAINAPPCAMBIAUTENTE_H

typedef enum {
    CAMBIA_UTENTE,
    CAMBIA_PASSWORD
} CambiaUtenteType;

#define MAIN_APP_CAMBIA_UTENTE_TYPE (main_app_cambia_utente_get_type ())
G_DECLARE_FINAL_TYPE (MainAppCambiaUtente, main_app_cambia_utente, MAIN, APP_CAMBIA_UTENTE, GtkDialog)


MainAppCambiaUtente *main_app_cambia_utente_new (MainAppWindow *win, CambiaUtenteType type);


#endif /* __MAINAPPCAMBIAUTENTE_H */
