#ifndef __CAMBIAUTENTE_H
#define __CAMBIAUTENTE_H

typedef enum {
    CAMBIA_UTENTE,
    CAMBIA_PASSWORD
} CambiaUtenteType;

#define CAMBIA_UTENTE_WINDOW_TYPE (cambia_utente_get_type ())
G_DECLARE_FINAL_TYPE (CambiaUtenteWindow, cambia_utente, CAMBIA_UTENTE, WINDOW, GtkDialog)


CambiaUtenteWindow *cambia_utente_new (MainWindow *win, CambiaUtenteType type);


#endif /* __CAMBIAUTENTE_H */
