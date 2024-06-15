#ifndef __MAINAPPPREFS_H
#define __MAINAPPPREFS_H

#include <gtk/gtk.h>
#include "mainappwin.h"


#define MAIN_APP_PREFS_TYPE (main_app_prefs_get_type ())
G_DECLARE_FINAL_TYPE (MainAppPrefs, main_app_prefs, MAIN, APP_PREFS, GtkDialog)


MainAppPrefs        *main_app_prefs_new          (MainAppWindow *win);


#endif /* __MAINAPPPREFS_H */
