#ifndef __RESETSETTORE_H
#define __RESETSETTORE_H

#include <gtk/gtk.h>
#include "mainwin.h"

#define RESET_SETTORE_WINDOW_TYPE (main_app_reset_settore_get_type ())
G_DECLARE_FINAL_TYPE (SettoreWindow, main_app_reset_settore, RESET_SETTORE, WINDOW, GtkDialog)


SettoreWindow * main_app_reset_settore_new (MainWindow *win);


#endif /* __RESETSETTORE_H */
