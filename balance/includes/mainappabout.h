#ifndef __MAINAPPABOUT_H
#define __MAINAPPABOUT_H

#include <gtk/gtk.h>
#include "mainappwin.h"


#define MAIN_APP_ABOUT_TYPE (main_app_about_get_type ())
G_DECLARE_FINAL_TYPE (MainAppAbout, main_app_about, MAIN, APP_ABOUT, GtkDialog)


MainAppAbout *main_app_about_new          (MainAppWindow *win);


#endif /* __MAINAPPABOUT_H */
