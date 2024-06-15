#ifndef __MAINAPP_H
#define __MAINAPP_H

#include <gtk/gtk.h>


#define APP_NAME "display"
#define APP_TITLE "Display"
#define APP_PID PROC_DISPLAY_00
#define MAIN_APP_TYPE (main_app_get_type ())
G_DECLARE_FINAL_TYPE (MainApp, main_app, MAIN, APP, GtkApplication)


MainApp *main_app_new (void);


extern gint settore;
extern gboolean maximized;
extern gboolean fullscreen;
#endif /* __MAINAPP_H */
