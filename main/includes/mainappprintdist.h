#ifndef __MAINAPPPRINTDIST_H
#define __MAINAPPPRINTDIST_H

#define MAIN_APP_PRINTDIST_TYPE (main_app_printdist_get_type ())
G_DECLARE_FINAL_TYPE (MainAppPrintDist, main_app_printdist, MAIN, APP_PRINTDIST, GtkDialog)


MainAppPrintDist *main_app_printdist_new          (MainAppWindow *win);


#endif /* __MAINAPPPRINTDIST_H */
