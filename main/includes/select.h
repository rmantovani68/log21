#ifndef __SELECT_H
#define __SELECT_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define SELECT_WINDOW_TYPE (main_app_select_get_type ())
G_DECLARE_FINAL_TYPE (SelectWindow, main_app_select, SELECT, WINDOW, GtkDialog)

SelectWindow * main_app_select_new (gpointer win);


#endif /* __SELECT_H */
