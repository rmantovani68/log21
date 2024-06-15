#ifndef __CALCIMBALLI_H
#define __CALCIMBALLI_H

#define CALC_IMBALLI_WINDOW_TYPE (calc_imballi_window_get_type ())
G_DECLARE_FINAL_TYPE (CalcImballiWindow, calc_imballi_window, CALC_IMBALLI, WINDOW, GtkDialog)


CalcImballiWindow *calc_imballi_window_new          (MainWindow *win);


#endif /* __CALCIMBALLI_H */
