#ifndef __DISTR_H
#define __DISTR_H

#define DISTR_WINDOW_TYPE (distr_get_type ())
G_DECLARE_FINAL_TYPE (DistrWindow, distr, DISTR, WINDOW, GtkDialog)


DistrWindow *distr_new          (MainWindow *win);


#endif /* __DISTR_H */
