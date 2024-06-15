/*
* gtk-support.h
*/
#include <gtk/gtk.h>
/*
* rm : include per gestione configurazione
*/
#include <stdio.h>
#include <ep-common.h>
#include <ep-db.h>

void gtk_label_printf(GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,2,3)));
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,3,4)));
void trace_debug(char *szColor,gboolean bDate,gboolean bNewLine,GtkWidget *txt,char *fmt,...) __attribute__ ((format(printf,5,6)));
void gtk_update(void);
void gtk_widget_set_foreground(GtkWidget *w,GdkColor *Color);
void gtk_widget_set_background(GtkWidget *w,GdkColor *Color);
void ApplyCfgItems(GtkWidget *dlg,PCFGITEM pItems);
void SetCfgWidgets(GtkWidget *dlg,PCFGITEM pItems);
void gtk_label_pango_printf(GtkLabel *label,char *pszCfgFileName,char *fmt,...) __attribute__ ((format(printf,3,4)));;

void gtk_tree_model_get_text(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, char **value);

GtkWidget *find_child(GtkWidget *parent, const gchar *name);
GtkWidget *get_ui_widget(GtkBuilder *builder, const gchar *name);
int dlg_msg(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, char *fmt,...) __attribute__ ((format(printf,5,6)));
