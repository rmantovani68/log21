#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <ep-popover.h>

#define WINDOW_NAME "ep-popover"


EpPopoverType _type;
GtkButtonsType _buttons;

enum {
  RESPONSE,
  CLOSE,
  LAST_SIGNAL
};

static guint ep_signals[LAST_SIGNAL];

typedef struct _ResponseData ResponseData;

struct _ResponseData
{
  gint response_id;
};

struct _EpPopover
{
    GtkPopover parent;

    /*< public >*/

    void (* response) (EpPopover *ep, gint response_id);
};

typedef struct _EpPopoverPrivate EpPopoverPrivate;

struct _EpPopoverPrivate
{
    GtkWidget *relative;
    EpPopoverType type;
    GtkButtonsType buttons;

    GtkWidget *lb_cdudc;
    GtkWidget *lb_cdpro;
    GtkWidget *lb_dstit;
    GtkWidget *lb_qty;
    GtkWidget *entry_cdudc;
    GtkWidget *entry_cdpro;
    GtkWidget *entry_dstit;
    GtkWidget *sb_qty;
    GtkWidget *calendar;
    GtkWidget *pb_ok;
    GtkWidget *pb_close;
    GtkWidget *pb_yes;
    GtkWidget *pb_no;
    GtkWidget *pb_cancel;
};

G_DEFINE_TYPE_WITH_PRIVATE(EpPopover, ep_popover, GTK_TYPE_POPOVER)

void ep_popover_response (EpPopover *ep, gint response_id);

void ep_popover_set_type (EpPopover *ep, EpPopoverType type)
{
    EpPopoverPrivate *priv = ep_popover_get_instance_private (ep);

    priv->type = type;
}

static void cancel_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_CANCEL);
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_OK);
}


static void no_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_NO);
}


static void yes_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_YES);
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{ 
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_CLOSE);
}

static void calendar_activated (GtkCalendar* self, gpointer win)
{ 
    ep_popover_response (EP_POPOVER (win), GTK_RESPONSE_OK);
}


static GActionEntry entries[] = {
  {"close",  close_activated,  NULL, NULL, NULL},
  {"ok",     ok_activated,     NULL, NULL, NULL},
  {"yes",    yes_activated,    NULL, NULL, NULL},
  {"no",     no_activated,     NULL, NULL, NULL},
  {"cancel", cancel_activated, NULL, NULL, NULL}
};

static void ep_popover_init (EpPopover *win)
{
    EpPopoverPrivate *priv = ep_popover_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->type    = _type;
    priv->buttons = _buttons;
    
    gtk_widget_hide(priv->entry_cdudc);
    gtk_widget_hide(priv->entry_cdpro);
    gtk_widget_hide(priv->entry_dstit);
    gtk_widget_hide(priv->sb_qty);
    gtk_widget_hide(priv->lb_cdudc);
    gtk_widget_hide(priv->lb_cdpro);
    gtk_widget_hide(priv->lb_dstit);
    gtk_widget_hide(priv->lb_qty);
    gtk_widget_hide(priv->calendar);

    switch(priv->type){
        case EP_TYPE_UDC_PRODUCT_QUANTITY:
            gtk_widget_show(priv->entry_cdudc);
            gtk_widget_show(priv->lb_cdudc);
            gtk_widget_show(priv->entry_cdpro);
            gtk_widget_show(priv->lb_cdpro);
            gtk_widget_show(priv->sb_qty);
            gtk_widget_show(priv->lb_qty);
        break;
        case EP_TYPE_PRODUCT_QUANTITY:
            gtk_widget_show(priv->entry_cdpro);
            gtk_widget_show(priv->lb_cdpro);
            gtk_widget_show(priv->sb_qty);
            gtk_widget_show(priv->lb_qty);
        break;
        case EP_TYPE_QUANTITY:
            gtk_widget_show(priv->sb_qty);
            gtk_widget_show(priv->lb_qty);
        break;
        case EP_TYPE_PRODUCT:
            gtk_widget_show(priv->entry_cdpro);
            gtk_widget_show(priv->lb_cdpro);
        break;
        case EP_TYPE_PRODUCT_TITLE_QUANTITY:
            gtk_widget_show(priv->entry_cdpro);
            gtk_widget_show(priv->lb_cdpro);
            gtk_widget_show(priv->sb_qty);
            gtk_widget_show(priv->lb_qty);
            gtk_widget_show(priv->entry_dstit);
            gtk_widget_show(priv->lb_dstit);
        break;
        case EP_TYPE_UDC:
            gtk_widget_show(priv->entry_cdudc);
            gtk_widget_show(priv->lb_cdudc);
        break;
        case EP_TYPE_CALENDAR:
            gtk_widget_show(priv->calendar);
            g_signal_connect (priv->calendar, "day-selected-double-click", G_CALLBACK (calendar_activated), win);
        break;
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

static void ep_popover_dispose (GObject *object)
{
    G_OBJECT_CLASS (ep_popover_parent_class)->dispose (object);
}

static void ep_popover_class_init (EpPopoverClass *class)
{
    G_OBJECT_CLASS (class)->dispose = ep_popover_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_UTILITIES, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, entry_cdudc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, entry_cdpro);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, entry_dstit);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, sb_qty     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, lb_cdudc   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, lb_cdpro   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, lb_dstit   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, lb_qty     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, calendar   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, pb_ok      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, pb_close   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, pb_yes     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, pb_no      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EpPopover, pb_cancel  );

    /**
    * EpPopover::response:
    * @ep: the object on which the signal is emitted
    * @response_id: the response ID
    *
    * Emitted when an action widget is clicked, the ep receives a
    * delete event, or the application programmer calls ep_popover_response().
    * On a delete event, the response ID is #GTK_RESPONSE_DELETE_EVENT.
    * Otherwise, it depends on which action widget was clicked.
    */
    ep_signals[RESPONSE] = g_signal_new ("response", G_OBJECT_CLASS_TYPE (class), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT);

}

GtkWidget *ep_popover_new (GtkWidget *relative, EpPopoverType type, GtkButtonsType buttons)
{
    GtkWidget *w;

    g_return_val_if_fail (relative == NULL || GTK_IS_WIDGET (relative), NULL);

    _type    = type;
    _buttons = buttons;
    
    w = g_object_new (EP_TYPE_POPOVER, NULL);

    EpPopoverPrivate *priv = ep_popover_get_instance_private ((EpPopover *)w);

    priv->type = type;
    priv->buttons = buttons;
    priv->relative = relative;

    if (relative != NULL)
        gtk_popover_set_relative_to(GTK_POPOVER (w),  GTK_WIDGET (relative));
  
    // if (flags & GTK_DIALOG_MODAL)
    //    gtk_window_set_modal (GTK_WINDOW (popover), TRUE);

    // if (flags & GTK_DIALOG_DESTROY_WITH_PARENT)
    //    gtk_window_set_destroy_with_parent (GTK_WINDOW (popover), TRUE);

    return w;
}

/**
 * ep_popover_response:
 * @ep: a #EpPopover
 * @response_id: response ID
 *
 * Emits the #EpPopover::response signal with the given response ID.
 * Used to indicate that the user has responded to the ep in some way;
 * typically either you or ep_popover_run() will be monitoring the
 * ::response signal and take appropriate action.  **/
void ep_popover_response (EpPopover *ep, gint response_id)
{
    g_return_if_fail (EP_IS_POPOVER (ep));

    g_signal_emit (ep, ep_signals[RESPONSE], 0, response_id);
}

typedef struct
{
    EpPopover *ep;
    gint response_id;
    GMainLoop *loop;
    gboolean destroyed;
} RunInfo;

static void shutdown_loop (RunInfo *ri)
{
    if (g_main_loop_is_running (ri->loop))
        g_main_loop_quit (ri->loop);
}

static void run_unmap_handler (EpPopover *ep, gpointer data)
{
    RunInfo *ri = data;

    shutdown_loop (ri);
}

static void run_response_handler (EpPopover *ep, gint response_id, gpointer data)
{
    RunInfo *ri;

    ri = data;

    ri->response_id = response_id;

    shutdown_loop (ri);
}

static gint run_delete_handler (EpPopover *ep, GdkEventAny *event, gpointer data)
{
    RunInfo *ri = data;

    shutdown_loop (ri);

    return TRUE; /* Do not destroy */
}

static void run_destroy_handler (EpPopover *ep, gpointer data)
{
    RunInfo *ri = data;

    /* shutdown_loop will be called by run_unmap_handler */

    ri->destroyed = TRUE;
}

/**
 * ep_popover_run:
 * @ep: a #EpPopover
 *
 * Blocks in a recursive main loop until the @ep either emits the
 * #EpPopover::response signal, or is destroyed. If the ep is
 * destroyed during the call to ep_popover_run(), gtk_ep_run() returns
 * #GTK_RESPONSE_NONE. Otherwise, it returns the response ID from the
 * ::response signal emission.
 *
 * Before entering the recursive main loop, ep_popover_run() calls
 * gtk_widget_show() on the ep for you. Note that you still
 * need to show any children of the ep yourself.
 *
 * During ep_popover_run(), the default behavior of #GtkWidget::delete-event
 * is disabled; if the ep receives ::delete_event, it will not be
 * destroyed as windows usually are, and ep_popover_run() will return
 * #GTK_RESPONSE_DELETE_EVENT. Also, during ep_popover_run() the ep
 * will be modal. You can force ep_popover_run() to return at any time by
 * calling ep_popover_response() to emit the ::response signal. Destroying
 * the ep during ep_popover_run() is a very bad idea, because your
 * post-run code won’t know whether the ep was destroyed or not.
 *
 * After ep_popover_run() returns, you are responsible for hiding or
 * destroying the ep if you wish to do so.
 *
 * Typical usage of this function might be:
 * |[<!-- language="C" -->
 *   GtkWidget *ep = ep_popover_new ();
 *   // Set up ep...
 *
 *   int result = ep_popover_run (GTK_DIALOG (ep));
 *   switch (result)
 *     {
 *       case GTK_RESPONSE_ACCEPT:
 *          // do_application_specific_something ();
 *          break;
 *       default:
 *          // do_nothing_since_ep_was_cancelled ();
 *          break;
 *     }
 *   gtk_widget_destroy (ep);
 * ]|
 *
 * Note that even though the recursive main loop gives the effect of a
 * modal ep (it prevents the user from interacting with other
 * windows in the same window group while the ep is run), callbacks
 * such as timeouts, IO channel watches, DND drops, etc, will
 * be triggered during a ep_popover_run() call.
 *
 * Returns: response ID
 **/
gint ep_popover_run (EpPopover *ep)
{
    RunInfo ri = { NULL, GTK_RESPONSE_NONE, NULL, FALSE };
    gboolean was_modal;
    gulong response_handler;
    gulong unmap_handler;
    gulong destroy_handler;
    gulong delete_handler;

    g_return_val_if_fail (GTK_IS_POPOVER (ep), -1);

    g_object_ref (ep);

    was_modal = gtk_popover_get_modal ( GTK_POPOVER(ep) );

    if (!was_modal)
        gtk_popover_set_modal (GTK_POPOVER (ep), TRUE);

    if (!gtk_widget_get_visible (GTK_WIDGET (ep)))
        gtk_widget_show (GTK_WIDGET (ep));

    /* TODO response signal ... */
    response_handler = g_signal_connect (ep, "response", G_CALLBACK (run_response_handler), &ri);

    /* GtkWidget signals */
    unmap_handler   = g_signal_connect (ep, "unmap",        G_CALLBACK (run_unmap_handler),   &ri);
    delete_handler  = g_signal_connect (ep, "delete-event", G_CALLBACK (run_delete_handler),  &ri);
    destroy_handler = g_signal_connect (ep, "destroy",      G_CALLBACK (run_destroy_handler), &ri);

    ri.loop = g_main_loop_new (NULL, FALSE);

    g_main_loop_run (ri.loop);

    g_main_loop_unref (ri.loop);

    ri.loop = NULL;

    if (!ri.destroyed)
    {
        if (!was_modal)
            gtk_popover_set_modal (GTK_POPOVER(ep), FALSE);

        /* TODO response signal ... */
        g_signal_handler_disconnect (ep, response_handler);
        g_signal_handler_disconnect (ep, unmap_handler);
        g_signal_handler_disconnect (ep, delete_handler);
        g_signal_handler_disconnect (ep, destroy_handler);
    }

    g_object_unref (ep);

    return ri.response_id;
}

GtkWidget *ep_popover_get_field(EpPopover *ep, EpPopoverField field)
{
    EpPopoverPrivate *priv = ep_popover_get_instance_private (ep);
    GtkWidget *w=NULL;

    switch(field){
        case EP_FIELD_NONE:     w = NULL; break;
        case EP_FIELD_QUANTITY: w = priv->sb_qty; break;
        case EP_FIELD_PRODUCT:  w = priv->entry_cdpro; break;
        case EP_FIELD_TITLE:    w = priv->entry_dstit; break;
        case EP_FIELD_UDC:      w = priv->entry_cdudc; break;
        case EP_FIELD_CALENDAR: w = priv->calendar; break;
    }
    return w;
}
