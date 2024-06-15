#ifndef __MAINAPPORDINIFILTRATI_H
#define __MAINAPPORDINIFILTRATI_H

#define MAIN_APP_ORDINI_FILTRATI_TYPE (main_app_ordini_filtrati_get_type ())
G_DECLARE_FINAL_TYPE (MainAppOrdiniFiltrati, main_app_ordini_filtrati, MAIN, APP_ORDINI_FILTRATI, GtkDialog)


MainAppOrdiniFiltrati * main_app_ordini_filtrati_new (MainAppWindow *win);


#endif /* __MAINAPPORDINIFILTRATI_H */
