#ifndef __ABOUT_H
#define __ABOUT_H

#define ABOUT_WINDOW_TYPE (about_window_get_type ())
G_DECLARE_FINAL_TYPE (AboutWindow, about_window, ABOUT, WINDOW, GtkDialog)


AboutWindow *about_window_new          (MainWindow *win);


#endif /* __ABOUT_H */
