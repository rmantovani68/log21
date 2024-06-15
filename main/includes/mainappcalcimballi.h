#ifndef __MAINAPPCALC_IMBALLI_H
#define __MAINAPPCALC_IMBALLI_H

#define MAIN_APP_CALC_IMBALLI_TYPE (main_app_calcimballi_get_type ())
G_DECLARE_FINAL_TYPE (MainAppCalcImballi, main_app_calcimballi, MAIN, APP_CALC_IMBALLI, GtkDialog)


MainAppCalcImballi *main_app_calcimballi_new          (MainAppWindow *win);


#endif /* __MAINAPPCALC_IMBALLI_H */
