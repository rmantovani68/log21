#ifndef __CARICOSETTORI_H
#define __CARICOSETTORI_H

#define CARICO_SETTORI_WINDOW_TYPE (carico_settori_get_type ())
G_DECLARE_FINAL_TYPE (CaricoSettoriWindow, carico_settori, CARICO_SETTORI, WINDOW, GtkDialog)


CaricoSettoriWindow *carico_settori_new          (MainWindow *win);


#endif /* __CARICOSETTORI_H */
