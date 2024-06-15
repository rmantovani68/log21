#ifndef __TEST_H__
#define __TEST_H__

#include <gtk/gtk.h>

/* 2 sec */
#define PLC_TIMEOUT 2000     

#define TEST_WINDOW_TYPE (test_window_get_type ())
G_DECLARE_FINAL_TYPE (TestWindow, test_window, TEST, WINDOW, GtkDialog)


TestWindow *test_window_new (MainWindow *win);


void ResetSettore(gpointer win, int nIsola,int nSettore);
void ResetSettori(gpointer win);
void SetStatoSettore(gpointer win, int settore_index, StatiSettore stato_settore);
StatiSettore GetStatoSettore(gpointer win, int settore_index);
void SetStatoLinea(gpointer win, StatiLinea stato_linea);
StatiLinea GetStatoLinea(gpointer win);
#endif /* __TEST_H__ */
