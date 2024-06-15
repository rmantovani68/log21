#ifndef __ABOUT_H_
#define __ABOUT_H_

#include <gtk/gtk.h>


#define ABOUT_WINDOW_TYPE (about_window_get_type ())
G_DECLARE_FINAL_TYPE (AboutWindow, about_window, ABOUT, WINDOW, GtkDialog)


GtkWidget *about_window_new (GtkWindow *parent, GtkDialogFlags flags);

#endif /* __ABOUT_H_ */
