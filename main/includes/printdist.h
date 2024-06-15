#ifndef __PRINTDIST_H
#define __PRINTDIST_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define PRINT_DIST_WINDOW_TYPE (print_dist_get_type ())
G_DECLARE_FINAL_TYPE (PrintDistWindow, print_dist, PRINT_DIST, WINDOW, GtkDialog)


PrintDistWindow *print_dist_new          (MainWindow *win);


#endif /* __PRINTDIST_H */
