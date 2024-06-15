#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <msg-box.h>

#define WINDOW_NAME "msg-box"

GtkMessageType _type;
GtkButtonsType _buttons;

struct _MsgBox
{
    GtkDialog parent;
};

typedef struct _MsgBoxPrivate MsgBoxPrivate;

struct _MsgBoxPrivate
{
    GtkMessageType type;

    GtkButtonsType buttons;

    GtkWidget *label;
    GtkWidget *image_box;
    GtkWidget *image;
    GtkWidget *box;
    GtkWidget *frame;
    GtkWidget *pb_ok;
    GtkWidget *pb_close;
    GtkWidget *pb_yes;
    GtkWidget *pb_no;
    GtkWidget *pb_cancel;
};

G_DEFINE_TYPE_WITH_PRIVATE(MsgBox, msg_box, GTK_TYPE_DIALOG)

void msg_box_set_markup (MsgBox *msgbox, const gchar *str)
{
    MsgBoxPrivate *priv = msg_box_get_instance_private (msgbox);

    gtk_label_set_markup (GTK_LABEL (priv->label), str);
}

void msg_box_set_image (MsgBox *msgbox, GtkWidget *image)
{
    MsgBoxPrivate *priv = msg_box_get_instance_private (msgbox);

    g_return_if_fail (image == NULL || GTK_IS_WIDGET (image));

    if (priv->image)
        gtk_widget_destroy (priv->image);

    priv->image = image;

    if (priv->image) { 
        gtk_widget_set_halign (priv->image, GTK_ALIGN_CENTER);
        gtk_widget_set_valign (priv->image, GTK_ALIGN_START);
        gtk_container_add (GTK_CONTAINER (priv->image_box), priv->image);
        gtk_box_reorder_child (GTK_BOX (priv->image_box), priv->image, 0);
    }

    priv->type = GTK_MESSAGE_OTHER;
}

GtkWidget *msg_box_get_image (MsgBox *msgbox)
{
    MsgBoxPrivate *priv = msg_box_get_instance_private (msgbox);

    return priv->image;
}

GtkWidget *msg_box_get_message_area (MsgBox *msgbox)
{
    MsgBoxPrivate *priv = msg_box_get_instance_private (msgbox);

    return priv->box;
}

static void cancel_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_dialog_response (GTK_DIALOG (win), GTK_RESPONSE_CANCEL);
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_dialog_response (GTK_DIALOG (win), GTK_RESPONSE_OK);
}


static void no_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_dialog_response (GTK_DIALOG (win), GTK_RESPONSE_NO);
}


static void yes_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_dialog_response (GTK_DIALOG (win), GTK_RESPONSE_YES);
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{ 
    gtk_dialog_response (GTK_DIALOG (win), GTK_RESPONSE_CLOSE);
}

static GActionEntry entries[] = {
  {"close",  close_activated,  NULL, NULL, NULL},
  {"ok",     ok_activated,     NULL, NULL, NULL},
  {"yes",    yes_activated,    NULL, NULL, NULL},
  {"no",     no_activated,     NULL, NULL, NULL},
  {"cancel", cancel_activated, NULL, NULL, NULL}
};

static void msg_box_init (MsgBox *win)
{
    MsgBoxPrivate *priv = msg_box_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->type    = _type;
    priv->buttons = _buttons;
    
    switch(priv->type){
        case GTK_MESSAGE_INFO:     gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "gtk-info",          GTK_ICON_SIZE_DIALOG); break;
        case GTK_MESSAGE_WARNING:  gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "gtk-warning",       GTK_ICON_SIZE_DIALOG); break;
        case GTK_MESSAGE_QUESTION: gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "gtk-question",      GTK_ICON_SIZE_DIALOG); break;
        case GTK_MESSAGE_ERROR:    gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "gtk-error",         GTK_ICON_SIZE_DIALOG); break;
        case GTK_MESSAGE_OTHER:    gtk_image_set_from_icon_name(GTK_IMAGE(priv->image), "gtk-missing-image", GTK_ICON_SIZE_DIALOG); break;
    }

    gtk_widget_hide(priv->pb_ok    );
    gtk_widget_hide(priv->pb_close );
    gtk_widget_hide(priv->pb_yes   );
    gtk_widget_hide(priv->pb_no    );
    gtk_widget_hide(priv->pb_cancel);

    switch(priv->buttons){
        case GTK_BUTTONS_NONE:
        break;
        case GTK_BUTTONS_OK:
            gtk_widget_show(priv->pb_ok);
        break;
        case GTK_BUTTONS_CLOSE:
            gtk_widget_show(priv->pb_close);
        break;
        case GTK_BUTTONS_CANCEL:
            gtk_widget_show(priv->pb_cancel);
        break;
        case GTK_BUTTONS_YES_NO:
            gtk_widget_show(priv->pb_yes);
            gtk_widget_show(priv->pb_no);
        break;
        case GTK_BUTTONS_OK_CANCEL:
            gtk_widget_show(priv->pb_ok);
            gtk_widget_show(priv->pb_cancel);
        break;
    }

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void msg_box_dispose (GObject *object)
{

    G_OBJECT_CLASS (msg_box_parent_class)->dispose (object);
}

static void msg_box_class_init (MsgBoxClass *class)
{
    G_OBJECT_CLASS (class)->dispose = msg_box_dispose;

    /* register resources */
    _ep_ensure_resources ();

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_UTILITIES, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, label    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, image_box);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, image    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, box      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, frame    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, pb_ok    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, pb_close );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, pb_yes   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, pb_no    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MsgBox, pb_cancel);
}

GtkWidget *msg_box_new (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...)
{
    GtkWidget *widget;
    GtkDialog *dialog;
    gchar* msg = NULL;
    va_list args;

    g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

    _type    = type;
    _buttons = buttons;
    
    widget = g_object_new (MSG_BOX_TYPE, "use-header-bar", TRUE, NULL);

    MsgBoxPrivate *priv = msg_box_get_instance_private ((MsgBox *)widget);

    dialog = GTK_DIALOG (widget);

    gtk_label_set_use_markup (GTK_LABEL (priv->label), TRUE);

    if (message_format) {
        va_start (args, message_format);
        msg = g_strdup_vprintf (message_format, args);
        va_end (args);

        gtk_label_set_text (GTK_LABEL (priv->label), msg);

        g_free (msg);
    }

    if (parent != NULL)
        gtk_window_set_transient_for (GTK_WINDOW (widget), GTK_WINDOW (parent));
  
    if (flags & GTK_DIALOG_MODAL)
        gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);

    if (flags & GTK_DIALOG_DESTROY_WITH_PARENT)
        gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);

    return widget;
}

GtkWidget *msg_box_new_with_markup (GtkWindow *parent, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const gchar *message_format, ...)
{
    GtkWidget *widget;
    va_list args;
    gchar *msg = NULL;

    g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), NULL);

    widget = msg_box_new (parent, flags, type, buttons, NULL);

    if (message_format) {
        va_start (args, message_format);
        msg = g_markup_vprintf_escaped (message_format, args);
        va_end (args);

        msg_box_set_markup ((MsgBox *)widget, msg);

        g_free (msg);
    }

    return widget;
}


int dlg_msg_with_table(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, gchar *sqlcmd, int nSelectionMode, GList *PSList, char *fmt,...)
{
    GtkWidget *dlg;
    va_list args;
    gchar *msg = NULL;

    va_start (args, fmt);
    msg = g_markup_vprintf_escaped (fmt, args);
    va_end (args);

    dlg = msg_box_new_with_markup (parent, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, type, buttons_type, msg);

    g_free (msg);

    if(title) gtk_window_set_title (GTK_WINDOW (dlg), title);

    GtkWidget *box = msg_box_get_message_area((MsgBox *)dlg);
    GtkWidget *sw = gtk_scrolled_window_new(NULL, NULL);

    gtk_box_pack_start (GTK_BOX(box), sw, TRUE, TRUE, 5);
    gtk_widget_show(sw);
    gtk_window_set_resizable (GTK_WINDOW(dlg), TRUE);
    gtk_window_set_default_size (GTK_WINDOW(dlg), 800, 600);

    RefreshTable(sw,"lst",sqlcmd,nSelectionMode,PSList, NULL, NULL);

    int rc = gtk_dialog_run (GTK_DIALOG (dlg));
    gtk_widget_destroy(dlg);

    return rc;
}

GtkWidget *dlg_msg_with_child(GtkWindow *parent, const gchar *title, GtkMessageType type, GtkButtonsType buttons_type, GtkWidget *w, gboolean expand, char *fmt,...)
{
    GtkWidget *dlg;
    va_list args;
    gchar *msg = NULL;

    va_start (args, fmt);
    msg = g_markup_vprintf_escaped (fmt, args);
    va_end (args);

    dlg = msg_box_new_with_markup (parent, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, type, buttons_type, msg);

    g_free (msg);

    if(title) gtk_window_set_title (GTK_WINDOW (dlg), title);

    GtkWidget *box = msg_box_get_message_area((MsgBox *)dlg);

    if(w){
        gtk_box_pack_start (GTK_BOX(box), w, expand, TRUE, 5);
        gtk_widget_show(w);
    }

    return dlg;
}

