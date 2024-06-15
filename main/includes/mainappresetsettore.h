#ifndef __MAINAPPRESETSETTORE_H
#define __MAINAPPRESETSETTORE_H

#define MAIN_APP_RESET_SETTORE_TYPE (main_app_reset_settore_get_type ())
G_DECLARE_FINAL_TYPE (MainAppResetSettore, main_app_reset_settore, MAIN, APP_RESET_SETTORE, GtkDialog)


MainAppResetSettore * main_app_reset_settore_new (MainAppWindow *win);


#endif /* __MAINAPPRESETSETTORE_H */
