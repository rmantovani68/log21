#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_distinta.h"
#include "edit_bancale.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-distinta-window"

static GtkWindow *_parent;
static gchar *_nmdis=NULL;
static gboolean _storico;

struct _EditDistintaWindow
{
    GtkDialog parent;
};

typedef struct _EditDistintaWindowPrivate EditDistintaWindowPrivate;

struct _EditDistintaWindowPrivate
{
    GtkWindow *parent;

    gchar *pszCfgFileName;

    GSettings *settings;

    WindowSizeInfo *wsi;

    gchar *nmdis;
    gboolean storico;

    GtkWidget *lb_nmdis;
    GtkWidget *lb_stato;
    GtkWidget *lb_stspe;
    GtkWidget *lb_msg  ;
    GtkWidget *sw_list ;


};

G_DEFINE_TYPE_WITH_PRIVATE(EditDistintaWindow, edit_distinta_window, GTK_TYPE_DIALOG)





void refresh_lista_bancali(gpointer win)
{
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);

    GList *PS=NULL;

    PS=add_item_to_parse(&PS,"%NMDIS%",priv->nmdis,FALSE);
    char szSelectCmd[4096];
    GetFileString("Edit Distinta", priv->storico ? "lista_udc_in_distinta_stor" : "lista_udc_in_distinta", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(priv->sw_list,"lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);
}

static void refresh(gpointer win)
{
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);
    DBresult *DBRes;
    char szSTATO[64];
    char szSTSPE[64];

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cdstato, cdstspe, cdtmspe from %s where cdnmdis=%s;",priv->storico ? "carichi_distinte_stor" : "carichi_distinte", priv->nmdis);
    if(DBresultStatus(DBRes)==DBRES_TUPLES_OK &&  DBntuples(DBRes)==1){
        strcpy(szSTATO,DBgetvalue(DBRes,0,0));
        strcpy(szSTSPE,DBgetvalue(DBRes,0,1));
        gtk_label_printf(priv->lb_nmdis,"<span size='15000'><b>%s</b></span>",priv->nmdis);
        gtk_label_printf(priv->lb_stato,"<span size='15000'><b>%s</b></span>",szSTATO);
        gtk_label_printf(priv->lb_stspe,"<span size='15000'><b>%s</b></span>",szSTSPE);

        refresh_lista_bancali(win);

    } else {
        gtk_label_printf(priv->lb_msg,"<span>Distinta %s non presente</span>",priv->nmdis);
    }
    DBclear(DBRes);
}



static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void edit_bancale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    gchar *cdudc = get_field_from_list_selected_row(win, "lst",0);

    w = (GtkWidget *)edit_bancale_window_new (GTK_WINDOW(win), cdudc, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(cdudc)
        g_free(cdudc);
}

static GActionEntry entries[] = {
    {"close",        close_activated,           NULL, NULL, NULL },
    {"modifica",     action_activated,          NULL, NULL, NULL },
    {"edit-bancale", edit_bancale_activated,    NULL, NULL, NULL }

};

static void edit_distinta_window_init (EditDistintaWindow *win)
{
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);

    g_printf("%s_window_init\n", WINDOW_NAME);

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

    priv->nmdis=g_strdup(_nmdis);
    priv->storico = _storico;
    refresh(win);
}

static void edit_distinta_window_dispose (GObject *object)
{
    EditDistintaWindow *win = EDIT_DISTINTA_WINDOW (object);
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);

    g_printf("%s_dispose\n", WINDOW_NAME);

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

    if(priv->nmdis) {
        g_free(priv->nmdis);
        priv->nmdis = NULL;
    }


    G_OBJECT_CLASS (edit_distinta_window_parent_class)->dispose (object);
}

static void edit_distinta_window_finalize (GObject *object)
{
    EditDistintaWindow *win = EDIT_DISTINTA_WINDOW (object);
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);

    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (edit_distinta_window_parent_class)->finalize (object);
}

static void edit_distinta_window_class_init (EditDistintaWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_distinta_window_dispose;
    G_OBJECT_CLASS (class)->finalize = edit_distinta_window_finalize;

    g_printf("%s_class_init\n", WINDOW_NAME);

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditDistintaWindow, lb_nmdis);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditDistintaWindow, lb_stato);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditDistintaWindow, lb_stspe);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditDistintaWindow, lb_msg  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditDistintaWindow, sw_list );
}

EditDistintaWindow * edit_distinta_window_new (GtkWindow *parent, gchar *nmdis, gboolean storico)
{
    _parent  = parent;
    _nmdis   = nmdis;
    _storico = storico;
    g_printf("%s_new\n", WINDOW_NAME);

    EditDistintaWindow *win = g_object_new (EDIT_DISTINTA_WINDOW_TYPE, "transient-for", parent, "use-header-bar", TRUE, NULL);
    EditDistintaWindowPrivate *priv = edit_distinta_window_get_instance_private (win);

    priv->parent = _parent;

    return win;
}
