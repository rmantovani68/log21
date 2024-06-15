/*
* gtk-support.h
*/

#ifndef _gtk_support_h_
#define _gtk_support_h_

#include <gtk/gtk.h>
#include <trace.h>
#include <ep-resources.h>
#include <ep-common.h>
#include <ep-db.h>

typedef	struct _WindowSizeInfo {
    gchar *window_name;
    gint current_x;
    gint current_y;
    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
} WindowSizeInfo;



void gtk_label_printf(GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,2,3)));
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...) __attribute__ ((format(printf,3,4)));
void trace_debug_gtk(char *szColor,gboolean bDate,gboolean bNewLine,GtkWidget *txt,char *fmt,...) __attribute__ ((format(printf,5,6)));
void gtk_update(void);
void gtk_widget_set_foreground(GtkWidget *w,GdkRGBA *Color);
void gtk_widget_set_background(GtkWidget *w,GdkRGBA *Color);
void ApplyCfgItems(GtkWidget *dlg,PCFGITEM pItems);
void SetCfgWidgets(GtkWidget *dlg,PCFGITEM pItems);
void gtk_label_pango_printf(GtkLabel *label,char *pszCfgFileName,char *fmt,...) __attribute__ ((format(printf,3,4)));;

void gtk_tree_model_get_text(GtkTreeModel *tree_model, GtkTreeIter *iter, gint column, char **value);

GtkWidget *find_child(GtkWidget *parent, const gchar *name);
GtkWidget *get_ui_widget(GtkBuilder *builder, const gchar *name);
int dlg_msg(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, char *fmt,...) __attribute__ ((format(printf,5,6)));

WindowSizeInfo *window_size_info_new (WindowSizeInfo **wsi, gchar *window_name);
void window_size_info_free (WindowSizeInfo *wsi);
gboolean window_save_state (gpointer win, WindowSizeInfo *wsi, GSettings *settings);
void window_load_state (gpointer win, WindowSizeInfo *wsi, GSettings *settings);
gboolean on_window_size_allocate (gpointer win, GtkAllocation *allocation, gpointer user_data);
gboolean on_window_state_event (gpointer win, GdkEventWindowState *event, gpointer user_data);

void _ep_ensure_resources (void);

void init_actions(gpointer win, GActionEntry *entries, int n_entries, gchar *group_name);
#endif /* _gtk_support_h_ */
