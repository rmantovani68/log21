#ifndef __MSGBOX_H
#define __MSGBOX_H

#include <gtk/gtk.h>


#define MSG_BOX_TYPE (msg_box_get_type ())
G_DECLARE_FINAL_TYPE (MsgBox, msg_box, MSG, BOX, GtkDialog)


void msg_box_set_markup (MsgBox *msgbox, const gchar *str);
void msg_box_set_image (MsgBox *msgbox, GtkWidget *image);
GtkWidget *msg_box_get_image (MsgBox *msgbox);
GtkWidget *msg_box_get_message_area (MsgBox *msgbox);
GtkWidget *msg_box_new (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...) G_GNUC_PRINTF (5, 6);
GtkWidget *msg_box_new_with_markup (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...) G_GNUC_PRINTF (5, 6);
int dlg_msg_with_table(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, gchar *sqlcmd, int nSelectionMode, GList *PSList, char *fmt,...);
GtkWidget *dlg_msg_with_child(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, GtkWidget *w, gboolean expand, char *fmt,...);

#endif /* __MSGBOX_H */
