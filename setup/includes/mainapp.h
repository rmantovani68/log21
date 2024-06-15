#ifndef __MAINAPP_H
#define __MAINAPP_H

#include <gtk/gtk.h>


#define APP_NAME "setup"
#define APP_TITLE "Setup"
#define APP_PID PROC_SETUP
#define MAIN_APP_TYPE (main_app_get_type ())
G_DECLARE_FINAL_TYPE (MainApp, main_app, MAIN, APP, GtkApplication)


MainApp     *main_app_new         (void);

#endif /* __MAINAPP_H */
