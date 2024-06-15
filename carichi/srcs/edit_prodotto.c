#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "edit_prodotto.h"
#include "mainfun.h"

#define WINDOW_NAME "edit-prodotto-window"

static GtkWindow *_parent;
static gchar *_cdpro=NULL;
static gboolean _storico;

struct _EditProdottoWindow
{
    GtkDialog parent;
};

typedef struct _EditProdottoWindowPrivate EditProdottoWindowPrivate;

struct _EditProdottoWindowPrivate
{
    GtkWindow *parent;

    gchar *pszCfgFileName;

    gchar *cdpro;
    gboolean storico;

    GSettings *settings;

    WindowSizeInfo *wsi;

    GtkWidget *pb_edit_prodotto_chiudi;
    GtkWidget *pb_edit_prodotto_modifica_identificativi;
    GtkWidget *pb_edit_prodotto_modifica_dati;
    GtkWidget *pb_edit_prodotto_visualizza_movimentazioni;
    GtkWidget *pb_edit_prodotto_ingresso;
    GtkWidget *pb_edit_prodotto_spedizione;
    GtkWidget *pb_edit_prodotto_conteggia;
    GtkWidget *pb_edit_prodotto_differenza_inventariale;
    GtkWidget *pb_edit_prodotto_annulla_movimento;
    GtkWidget *pb_edit_prodotto_stampa;
                                                 
    GtkWidget *entry_brcde;
    GtkWidget *entry_brcdf;
    GtkWidget *entry_dstit;
    GtkWidget *entry_dsaut;
    GtkWidget *sb_pzpro;
    GtkWidget *sb_nmtir;
    GtkWidget *lb_cdpro;
    GtkWidget *lb_msg;
    GtkWidget *sw_list;
};

G_DEFINE_TYPE_WITH_PRIVATE(EditProdottoWindow, edit_prodotto_window, GTK_TYPE_DIALOG)

void on_lista_movimenti_selection_changed (GtkTreeSelection *selection, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    if(gtk_tree_selection_count_selected_rows ( selection)) {
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_annulla_movimento,TRUE);
    } else {
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_annulla_movimento,FALSE);
    }

}

void refresh_lista_movimenti(gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    GList *PS=NULL;
    PS=add_item_to_parse(&PS,"%CDPRO%",priv->cdpro,FALSE);
    if(priv->storico){
        char szSelectCmd[4096];
        GetFileString("Edit Prodotto","lista_movimentazioni_prodotto_stor", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
        RefreshTable(priv->sw_list,"lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, G_CALLBACK(on_lista_movimenti_selection_changed), win);
    } else {
        char szSelectCmd[4096];
        GetFileString("Edit Prodotto","lista_movimentazioni_prodotto", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
        RefreshTable(priv->sw_list,"lst", szSelectCmd, GTK_SELECTION_SINGLE, PS, G_CALLBACK(on_lista_movimenti_selection_changed), win);
    }
    gtk_widget_set_sensitive(priv->pb_edit_prodotto_annulla_movimento,FALSE);

}

static void refresh(gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);
    DBresult *DBRes;
    char szDSTIT[128];
    char szDSAUT[128];
    ep_bool_t bProdottoInCatalogo=FALSE;

    if(priv->storico){
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_ingresso,FALSE);
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_spedizione,FALSE);
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_conteggia,FALSE);
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_differenza_inventariale,FALSE);
        gtk_widget_set_sensitive(priv->pb_edit_prodotto_modifica_identificativi,FALSE);
    }


    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select prcdpro,prdstit,prdsaut from catalogo  where prcdpro='%s';",priv->cdpro);
    if(DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)==1){
        strcpy(szDSTIT,DBgetvalue(DBRes,0,1));
        strcpy(szDSAUT,DBgetvalue(DBRes,0,2));
        bProdottoInCatalogo=TRUE;
    } else {
        bProdottoInCatalogo=FALSE;
    }
    DBclear(DBRes);

    if (bProdottoInCatalogo) {
        gtk_label_printf(priv->lb_cdpro,"<span size='15000'><b>%s</b></span>",priv->cdpro);
        gtk_entry_set_text(GTK_ENTRY(priv->entry_dstit),szDSTIT);
        gtk_entry_set_text(GTK_ENTRY(priv->entry_dsaut),szDSAUT);

        if (Cfg.bShowListaMovimentazioniInDlgEditProdotto) {
            refresh_lista_movimenti(win);
        }

    } else {
        gtk_label_printf(priv->lb_msg,"<span>Prodotto %s non in catalogo</span>",priv->cdpro);
    }
    
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

static void edit_id_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void edit_dati_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static void movimenti_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    refresh_lista_movimenti(win);
}

static void ingresso_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    EpPopoverField fields[2]; gchar *values[2];

    int i=0;
    fields[i] = EP_FIELD_PRODUCT; values[i++]=priv->cdpro; 
    fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_edit_prodotto_ingresso), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_ingresso);
}

static void spedizione_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    EpPopoverField fields[2]; gchar *values[2];

    int i=0;
    fields[i] = EP_FIELD_PRODUCT; values[i++]=priv->cdpro; 
    fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_edit_prodotto_spedizione), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_spedizione);
}

static void conteggia_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    EpPopoverField fields[2]; gchar *values[2];

    int i=0;
    fields[i] = EP_FIELD_PRODUCT; values[i++]=priv->cdpro; 
    fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_edit_prodotto_conteggia), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_conteggia);
}

static void differenza_inventariale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    EpPopoverField fields[2]; gchar *values[2];

    int i=0;
    fields[i] = EP_FIELD_PRODUCT; values[i++]=priv->cdpro; 
    fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_edit_prodotto_differenza_inventariale), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_differenza_inventariale);
}

static void annulla_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gchar *progressivo = get_field_from_list_selected_row(win, "lst",7);
    gchar *msg = NULL;
    if(!do_annulla_movimento(win, progressivo, &msg)){
        dlg_msg( GTK_WINDOW(win), "Errore", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Errore in operazione:\n%s", msg?msg:"NON SPECIFICATO");
    }
    if(msg) g_free(msg);
    if(progressivo) g_free(progressivo);
}

static void stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static GActionEntry entries[] = {
    {"close",                   close_activated,                     NULL, NULL, NULL },
    {"edit-id",                 edit_id_activated,                   NULL, NULL, NULL },
    {"edit-dati",               edit_dati_activated,                 NULL, NULL, NULL },
    {"movimenti",               movimenti_activated,                 NULL, NULL, NULL },
    {"ingresso",                ingresso_activated,                  NULL, NULL, NULL },
    {"spedizione",              spedizione_activated,                NULL, NULL, NULL },
    {"conteggia",               conteggia_activated,                 NULL, NULL, NULL },
    {"differenza-inventariale", differenza_inventariale_activated,   NULL, NULL, NULL },
    {"annulla",                 annulla_activated,                   NULL, NULL, NULL },
    {"stampa",                  stampa_activated,                    NULL, NULL, NULL }
};



static void edit_prodotto_window_init (EditProdottoWindow *win)
{
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

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

    priv->storico = _storico;
    priv->cdpro = g_strdup(_cdpro);

    refresh(win);
}

static void edit_prodotto_window_dispose (GObject *object)
{
    EditProdottoWindow *win = EDIT_PRODOTTO_WINDOW (object);
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

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

    if(priv->cdpro) {
        g_strdup(priv->cdpro);
        priv->cdpro=NULL;
    }


    G_OBJECT_CLASS (edit_prodotto_window_parent_class)->dispose (object);
}

static void edit_prodotto_window_finalize (GObject *object)
{
    EditProdottoWindow *win = EDIT_PRODOTTO_WINDOW (object);
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    g_printf("%s_finalize\n", WINDOW_NAME);

    G_OBJECT_CLASS (edit_prodotto_window_parent_class)->finalize (object);
}

static void edit_prodotto_window_class_init (EditProdottoWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = edit_prodotto_window_dispose;
    G_OBJECT_CLASS (class)->finalize = edit_prodotto_window_finalize;

    g_printf("%s_class_init\n", WINDOW_NAME);

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_chiudi);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_modifica_identificativi);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_modifica_dati);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_visualizza_movimentazioni);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_ingresso);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_spedizione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_conteggia);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_differenza_inventariale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_annulla_movimento);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, pb_edit_prodotto_stampa);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, entry_brcde);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, entry_brcdf);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, entry_dstit);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, entry_dsaut);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, sb_pzpro);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, sb_nmtir);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, lb_cdpro);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, lb_msg);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), EditProdottoWindow, sw_list);

}

EditProdottoWindow * edit_prodotto_window_new (GtkWindow *parent, gchar *cdpro, gboolean storico)
{
    _parent = parent;
    _cdpro = cdpro;
    _storico = storico;
    g_printf("%s_new\n", WINDOW_NAME);

    EditProdottoWindow *win = g_object_new (EDIT_PRODOTTO_WINDOW_TYPE, "transient-for", parent, "use-header-bar", TRUE, NULL);
    EditProdottoWindowPrivate *priv = edit_prodotto_window_get_instance_private (win);

    priv->parent = _parent;

    return win;
}

