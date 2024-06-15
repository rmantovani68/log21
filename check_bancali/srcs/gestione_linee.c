#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "gestione_linee.h"
#include "mainfun.h"

#define WINDOW_NAME "gestione-linee-window"

struct _GestioneLineeWindow
{
    GtkDialog parent;
};

typedef struct _GestioneLineeWindowPrivate GestioneLineeWindowPrivate;

struct _GestioneLineeWindowPrivate
{
    GtkWidget *lb_titolo     ;
    GtkWidget *lb_key_linea  ;
    GtkWidget *lb_key_bancale;
    GtkWidget *lb_num_colli  ;
    GtkWidget *lb_msg        ;
    GtkWidget *sw_list       ;
    GtkWidget *sb_anno       ;
    GtkWidget *entry_ordprog ;
    GtkWidget *entry_nmced   ;
    GtkWidget *entry_tpspe   ;
    GtkWidget *entry_cdlin   ;

    gchar *pszCfgFileName;

    GSettings *settings;
    WindowSizeInfo *wsi;

    gchar lista_attuale[128];
};

G_DEFINE_TYPE_WITH_PRIVATE(GestioneLineeWindow, gestione_linee_window, GTK_TYPE_DIALOG)



void on_selection_changed (GtkTreeSelection *treeselection, gpointer win)
{
    g_printf("selection_changed\n");
}

static void apply_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GestioneLineeWindowPrivate *priv = gestione_linee_window_get_instance_private (win);

	char szBuffer[128];
	char szNMCED[128];
	char szTPSPE[128];
	char szCDLIN[128];
	char szORDPROG[128];
	char szOrdProg[128];
	int nAnno;
	DBresult *DBRes;
	int nKeyLinea;

	strcpy(szORDPROG, gtk_entry_get_text(GTK_ENTRY(priv->entry_ordprog)));
	strcpy(szNMCED, gtk_entry_get_text(GTK_ENTRY(priv->entry_nmced)));
	strcpy(szTPSPE, gtk_entry_get_text(GTK_ENTRY(priv->entry_tpspe)));
	strcpy(szCDLIN, gtk_entry_get_text(GTK_ENTRY(priv->entry_cdlin)));
	nAnno = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->sb_anno));

	StrTrimAll(szORDPROG);

	if(strlen(szORDPROG)){

		sprintf(szOrdProg,"AM0000015%02d%s",nAnno-2000,szORDPROG);

		DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select key_collo,key_bancale,key_linea from storico_colli_bancale where cpnmced = %s and ordprog='%s' order by cpnmcol;", szNMCED, szOrdProg);
		if((DBntuples(DBRes))){
			nKeyLinea=atoi(DBgetvalue(DBRes,0,2));
			gtk_label_printf(priv->lb_key_linea,"%d",nKeyLinea);
			sprintf(szBuffer,"c.cpnmced=%s and c.ordprog='%s'",szNMCED,szOrdProg);
			strcpy(priv->lista_attuale,"lista_colli_ordine");
			// RefreshTable(dlg,"sw_list","lst","lb_titolo","check_bancali",priv->lista_attuale,GTK_SELECTION_SINGLE,szBuffer, NULL, NULL);
            refresh_table(win, "sw_list","lst", priv->pszCfgFileName, "check_bancali",priv->lista_attuale,GTK_SELECTION_SINGLE,szBuffer);
            g_signal_connect(TREE_SELECTION_LST(find_child(GTK_WIDGET(win), "lst")), "change", G_CALLBACK (on_selection_changed),  win);
		} else {
			gtk_label_printf(priv->lb_msg,"Ordine non trovato !");
		}
		DBclear(DBRes);
	} else {
		DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select key_linea from storico_linee_spedizione where lnnmced = %s and lntpspe='%s' and lncdlin='%s' order by tminit desc;", szNMCED, szTPSPE, szCDLIN);
		if((DBntuples(DBRes))){
			nKeyLinea=atoi(DBgetvalue(DBRes,0,0));
			gtk_label_printf(priv->lb_key_linea,"%d",nKeyLinea);
			sprintf(szBuffer,"b.key_linea=%d and c.key_bancale=b.key_bancale",nKeyLinea);
			strcpy(priv->lista_attuale,"lista_bancali_cedola_linea");

			// RefreshTable(dlg,"sw_list","lst","lb_titolo","check_bancali",priv->lista_attuale,GTK_SELECTION_SINGLE,szBuffer, NULL, NULL);
            refresh_table(win, "sw_list","lst", priv->pszCfgFileName, "check_bancali",priv->lista_attuale,GTK_SELECTION_SINGLE,szBuffer);
            g_signal_connect(TREE_SELECTION_LST(find_child(GTK_WIDGET(win), "lst")), "change", G_CALLBACK (on_selection_changed),  win);
		} else {
			gtk_label_printf(priv->lb_msg,"Linea non trovata !");
		}
		DBclear(DBRes);
	}
}

static void stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GestioneLineeWindowPrivate *priv = gestione_linee_window_get_instance_private (win);
	int nKeyLinea;
	int nKeyBancale;
	char *pPtr;

	nKeyLinea   = atoi(gtk_label_get_text(GTK_LABEL(priv->lb_key_linea)));
	nKeyBancale = atoi(gtk_label_get_text(GTK_LABEL(priv->lb_key_bancale)));

    int rc=dlg_msg( GTK_WINDOW(win), "Stampa", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Stampa la lista attuale ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            if(!strcmp(priv->lista_attuale,"lista_bancali_cedola_linea")){
                stampa_linea(win, nKeyLinea);
            } else if(!strcmp(priv->lista_attuale,"lista_colli_bancale")){
                stampa_bancale(win, nKeyBancale,TRUE,FALSE);
            }
        break;
    }
}


static void colli_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GestioneLineeWindowPrivate *priv = gestione_linee_window_get_instance_private (win);
	char szBuffer[128];
	int nKeyLinea;
	int nKeyBancale;
	gchar *pPtr;

	nKeyLinea  = atoi(gtk_label_get_text(GTK_LABEL(priv->lb_key_linea)));   
	nKeyBancale= atoi(gtk_label_get_text(GTK_LABEL(priv->lb_key_bancale))); 

	sprintf(szBuffer,"key_bancale=%d",nKeyBancale);

	refresh_table(win, "sw_list","lst", priv->pszCfgFileName, "check_bancali","lista_colli_bancale",GTK_SELECTION_SINGLE,szBuffer);
}



static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action activated\n");
}

static GActionEntry entries[] = {
  {"apply",  apply_activated,  NULL, NULL, NULL},
  {"colli",  colli_activated, NULL, NULL, NULL},
  {"stampa", stampa_activated, NULL, NULL, NULL},
  {"close",  close_activated,  NULL, NULL, NULL}
};

static void gestione_linee_window_init (GestioneLineeWindow *win)
{
    GestioneLineeWindowPrivate *priv = gestione_linee_window_get_instance_private (win);

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

static void gestione_linee_window_dispose (GObject *object)
{
    GestioneLineeWindow *win = GESTIONE_LINEE_WINDOW (object);
    GestioneLineeWindowPrivate *priv = gestione_linee_window_get_instance_private (win);

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

    G_OBJECT_CLASS (gestione_linee_window_parent_class)->dispose (object);
}

static void gestione_linee_window_class_init (GestioneLineeWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = gestione_linee_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, lb_titolo     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, lb_key_linea  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, lb_key_bancale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, lb_num_colli  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, lb_msg        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, sw_list       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, sb_anno       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, entry_ordprog );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, entry_nmced   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, entry_tpspe   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), GestioneLineeWindow, entry_cdlin   );

}

GestioneLineeWindow * gestione_linee_window_new (MainWindow *win)
{
    return g_object_new (GESTIONE_LINEE_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);
}
