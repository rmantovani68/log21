#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "chiusura_bancale.h"
#include "mainfun.h"

#define WINDOW_NAME "chiusura-bancale-window"

static MainWindow *_parent;
static int _nmbnc;

struct _ChiusuraBancaleWindow
{
    GtkDialog parent;
};

typedef struct _ChiusuraBancaleWindowPrivate ChiusuraBancaleWindowPrivate;

struct _ChiusuraBancaleWindowPrivate
{

	MainWindow *parent;
	int nmbnc;

    GtkWidget *lb_key_bancale;
    GtkWidget *lb_bntpspe;
    GtkWidget *lb_bncdlin;
    GtkWidget *lb_bnnmcll;
    GtkWidget *lb_bnpsrea;
    GtkWidget *lb_bnpspre;
    GtkWidget *lb_msg;
    GtkWidget *sw_colli;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gint current_width;
    gint current_height;
    gboolean is_maximized;
    gboolean is_fullscreen;
};

G_DEFINE_TYPE_WITH_PRIVATE(ChiusuraBancaleWindow, chiusura_bancale_window, GTK_TYPE_DIALOG)


static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void chiusura_bancale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    ChiusuraBancaleWindowPrivate *priv = chiusura_bancale_window_get_instance_private (win);

	if(do_chiusura_bancale(priv->parent)){
		gtk_widget_destroy(GTK_WIDGET(win));
	}
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static GActionEntry entries[] = {
  {"ok",     chiusura_bancale_activated, NULL, NULL, NULL},
  {"cancel", close_activated, NULL, NULL, NULL},
  {"stampa", action_activated, NULL, NULL, NULL}
};

static void chiusura_bancale_window_init (ChiusuraBancaleWindow *win)
{
    ChiusuraBancaleWindowPrivate *priv = chiusura_bancale_window_get_instance_private (win);

    gtk_widget_init_template (GTK_WIDGET (win));


    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);


    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void chiusura_bancale_window_dispose (GObject *object)
{
    ChiusuraBancaleWindow *win = CHIUSURA_BANCALE_WINDOW (object);
    ChiusuraBancaleWindowPrivate *priv = chiusura_bancale_window_get_instance_private (win);

    g_printf("%s dispose\n", WINDOW_NAME);

    if(priv->wsi){          
        g_settings_sync ();
        window_save_state (win,  priv->wsi, priv->settings);
        window_size_info_free (priv->wsi);
        priv->wsi = NULL;
    }

    if(priv->settings){          
        g_clear_object (&(priv->settings)); 
        priv->settings=NULL;    
    }

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }


    G_OBJECT_CLASS (chiusura_bancale_window_parent_class)->dispose (object);
}

static void chiusura_bancale_window_class_init (ChiusuraBancaleWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = chiusura_bancale_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_key_bancale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_bntpspe    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_bncdlin    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_bnnmcll    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_bnpsrea    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_bnpspre    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, lb_msg        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ChiusuraBancaleWindow, sw_colli      );
}

ChiusuraBancaleWindow * chiusura_bancale_window_new (MainWindow *win, int nmbnc)
{
	_parent = win;
	_nmbnc = nmbnc;

    ChiusuraBancaleWindow *w = g_object_new (CHIUSURA_BANCALE_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
    ChiusuraBancaleWindowPrivate *priv = chiusura_bancale_window_get_instance_private (w);

    priv->parent = _parent;
    priv->nmbnc = _nmbnc;

	return w;
}
