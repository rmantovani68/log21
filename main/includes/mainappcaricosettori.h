#ifndef __MAINAPPCARICOSETTORI_H
#define __MAINAPPCARICOSETTORI_H

#define MAIN_APP_CARICO_SETTORI_TYPE (main_app_carico_settori_get_type ())
G_DECLARE_FINAL_TYPE (MainAppCaricoSettori, main_app_carico_settori, MAIN, APP_CARICO_SETTORI, GtkDialog)


MainAppCaricoSettori *main_app_carico_settori_new          (MainAppWindow *win);


#endif /* __MAINAPPCARICOSETTORI_H */
