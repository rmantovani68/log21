#ifndef __PREFERENCES_H
#define __PREFERENCES_H

#include <gtk/gtk.h>
#include "mainwin.h"


#define PREFERENCES_WINDOW_TYPE (preferences_window_get_type ())
G_DECLARE_FINAL_TYPE (PreferencesWindow, preferences_window, PREFERENCES, WINDOW, GtkDialog)


PreferencesWindow        *preferences_window_new          (MainWindow *win);


#endif /* __PREFERENCES_H */
