#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <gtk-support.h>
#include <picking.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_bancale.h"
#include "edit_prodotto.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-bancale-window"

static GtkWindow *_parent;
static gchar *_cdudc=NULL;
static gboolean _storico;

struct _EditBancaleWindow
{
    GtkDialog parent;
};

typedef struct _EditBancaleWindowPrivate EditBancaleWindowPrivate;

struct _EditBancaleWindowPrivate
{
    GtkWindow *parent;

    gchar *pszCfgFileName;

    GSettings *settings;

    WindowSizeInfo *wsi;

	gchar *cdudc;
	gboolean storico;

	GtkWidget *sw_list;
	GtkWidget *lb_cdudc;
	GtkWidget *lb_msg;
	GtkWidget *sb_nmdis;

};

G_DEFINE_TYPE_WITH_PRIVATE(EditBancaleWindow, edit_bancale_window, GTK_TYPE_DIALOG)



void refresh_lista_prodotti(gpointer win)
{
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);

    GList *PS=NULL;

	PS=add_item_to_parse(&PS,"%CDUDC%",priv->cdudc,FALSE);
	if(priv->storico){
		char szSelectCmd[4096];
		GetFileString("Edit Bancale","lista_prodotti_su_udc_stor", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
		RefreshTable(priv->sw_list,"lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);
	} else {
		char szSelectCmd[4096];
		GetFileString("Edit Bancale","lista_prodotti_su_udc", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
		RefreshTable(priv->sw_list,"lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, NULL, NULL);
	}

}

static void refresh(gpointer win)
{
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);
	DBresult *DBRes;
	char szSTATO[64];
	char szCDFLG[64];
	int nNMDIS;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select udnmdis, udcdflg, udstato from %s where udcdudc='%s';", priv->storico ? "carichi_udc_stor" : "carichi_udc",priv->cdudc);
	if(DBresultStatus(DBRes)==DBRES_TUPLES_OK &&  DBntuples(DBRes)==1){
		nNMDIS=atoi(DBgetvalue(DBRes,0,0));
		strcpy(szCDFLG,DBgetvalue(DBRes,0,1));
		strcpy(szSTATO,DBgetvalue(DBRes,0,2));
		gtk_label_printf(priv->lb_cdudc,"<span size='15000'><b>%s</b></span>",priv->cdudc);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->sb_nmdis),(gdouble)nNMDIS);

		refresh_lista_prodotti(win);

	} else {
		gtk_label_printf(priv->lb_msg,"<span>Bancale %s non presente</span>",priv->cdudc);
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

static void modifica_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void edit_prodotto_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    gchar *cdpro = get_field_from_list_selected_row(win, "lst",0);

    w = (GtkWidget *)edit_prodotto_window_new (GTK_WINDOW(win), cdpro, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(cdpro)
        g_free(cdpro);
}


static GActionEntry entries[] = {
    {"close",         close_activated,         NULL, NULL, NULL },
    {"action",        action_activated,        NULL, NULL, NULL },
    {"modifica",      modifica_activated,      NULL, NULL, NULL },
    {"edit-prodotto", edit_prodotto_activated, NULL, NULL, NULL }

};

static void edit_bancale_window_init (EditBancaleWindow *win)
{
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);

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

    priv->cdudc = g_strdup(_cdudc);
    priv->storico = _storico;

	refresh(win);
}

static void edit_bancale_window_dispose (GObject *object)
{
    EditBancaleWindow *win = EDIT_BANCALE_WINDOW (object);
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);

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

    if(priv->cdudc) {
		g_free(priv->cdudc);
		priv->cdudc = NULL;
	}


    G_OBJECT_CLASS (edit_bancale_window_parent_class)->dispose (object);
}

static void edit_bancale_window_finalize (GObject *object)
{
    EditBancaleWindow *win = EDIT_BANCALE_WINDOW (object);
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);

    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (edit_bancale_window_parent_class)->finalize (object);
}

static void edit_bancale_window_class_init (EditBancaleWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_bancale_window_dispose;
    G_OBJECT_CLASS (class)->finalize = edit_bancale_window_finalize;

    g_printf("%s_class_init\n", WINDOW_NAME);

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditBancaleWindow, sw_list);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditBancaleWindow, lb_cdudc);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditBancaleWindow, lb_msg);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditBancaleWindow, sb_nmdis);
}

EditBancaleWindow * edit_bancale_window_new (GtkWindow *parent, gchar *cdudc, gboolean storico)
{
    _parent = parent;
    _cdudc = cdudc;
    _storico = storico;
    g_printf("%s_new cdudc = [%s] storico = [%d]\n", WINDOW_NAME, cdudc, storico);

    EditBancaleWindow *win = g_object_new (EDIT_BANCALE_WINDOW_TYPE, "transient-for", parent, "use-header-bar", TRUE, NULL);
    EditBancaleWindowPrivate *priv = edit_bancale_window_get_instance_private (win);

    priv->parent = _parent;

    return win;
}
