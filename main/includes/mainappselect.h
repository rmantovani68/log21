#ifndef __MAINAPPSELECT_H
#define __MAINAPPSELECT_H

#define MAIN_APP_SELECT_TYPE (main_app_select_get_type ())
G_DECLARE_FINAL_TYPE (MainAppSelect, main_app_select, MAIN, APP_SELECT, GtkDialog)

MainAppSelect * main_app_select_new (gpointer win);


#endif /* __MAINAPPSELECT_H */
