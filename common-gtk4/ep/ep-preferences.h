#ifndef __EP_PREFERENCES_H_
#define __EP_PREFERENCES_H_

#include <gtk/gtk.h>

#define PREFERENCES_WINDOW_TYPE (preferences_window_get_type ())
G_DECLARE_FINAL_TYPE (PreferencesWindow, preferences_window, PREFERENCES, WINDOW, GtkDialog)

PreferencesWindow        *preferences_window_new          (GtkApplicationWindow *win);

#endif /* __PREFERENCES_H */
