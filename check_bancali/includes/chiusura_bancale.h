#ifndef __CHIUSURA_BANCALE_H
#define __CHIUSURA_BANCALE_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define CHIUSURA_BANCALE_WINDOW_TYPE (chiusura_bancale_window_get_type ())
G_DECLARE_FINAL_TYPE (ChiusuraBancaleWindow, chiusura_bancale_window, CHIUSURA_BANCALE, WINDOW, GtkDialog)


ChiusuraBancaleWindow * chiusura_bancale_window_new (MainWindow *win, int nmbnc);

#endif /* __CHIUSURA_BANCALE_H */
