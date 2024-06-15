#ifndef __MAINAPPDISTR_H
#define __MAINAPPDISTR_H

#define MAIN_APP_DISTR_TYPE (main_app_distr_get_type ())
G_DECLARE_FINAL_TYPE (MainAppDistr, main_app_distr, MAIN, APP_DISTR, GtkDialog)


MainAppDistr *main_app_distr_new          (MainAppWindow *win);


#endif /* __MAINAPPDISTR_H */
