#ifndef __BILANCIA_H
#define __BILANCIA_H

#include <gtk/gtk.h>

#define BILANCIA_WINDOW_TYPE (bilancia_window_get_type ())
G_DECLARE_FINAL_TYPE (BilanciaWindow, bilancia_window, BILANCIA, WINDOW, GtkDialog)


BilanciaWindow *bilancia_window_new (MainWindow *win);


#endif /* __BILANCIA_H */
