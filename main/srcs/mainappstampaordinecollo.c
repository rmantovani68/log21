#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>
#include <picking.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainappwin.h"
#include "mainappstampaordinecollo.h"
#include "mainfun.h"

/* workaround */
STAMPA_ORDINE_COLLO_TYPE_ENUM _type;
gchar *_ordine=NULL;



struct _MainAppStampaOrdineCollo
{
    GtkDialog parent;
};

typedef struct _MainAppStampaOrdineColloPrivate MainAppStampaOrdineColloPrivate;

struct _MainAppStampaOrdineColloPrivate
{
    MainAppWindow *main_app_window;
    STAMPA_ORDINE_COLLO_TYPE_ENUM type;
    gchar *ordine;
    GtkWidget *cb_storico;
    GtkWidget *cb_stampa_su_file;
    GtkWidget *lb_msg;
    GtkWidget *lb_ordine;
    GtkWidget *lb_collo;
    GtkWidget *entry_ordine;
    GtkWidget *sb_collo;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppStampaOrdineCollo, main_app_stampa_ordine_collo, GTK_TYPE_DIALOG)

void do_stampa_etichette_ordine (gchar *ordine)
{
    char szBuffer[256];

    sprintf(szBuffer,"%s,%s", ordine,Cfg.szLabelPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC,szBuffer);
}

void do_stampa_dati_ordine (gchar *ordine)
{
    char szBuffer[256];

    sprintf(szBuffer,"%s,%s", ordine,Cfg.szPrinterConsole);
    SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_DATI_ORDINE,szBuffer);
}

void do_stampa_dettagli_spedizione (gchar *ordine)
{
	char szBuffer[128];
	DBresult *DBRes;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into stampe (CODICE_TIPO_STAMPA, NOME_STAMPANTE, STRINGA_PARAMETRI, PRIORITA, STATO) values ('%d', '%s', '%s', 10, 'R');",
		COD_STAMPA_SPEDIZIONE,
		Cfg.szPrinterConsole,
		ordine);

	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
        trace_debug(TRUE, 1,  "Fallita insert in stampe [%d-%s-%s]",COD_STAMPA_SPEDIZIONE, Cfg.szPrinterConsole, ordine);
	}
	DBFreeQueryResult(DBRes);

}

void do_stampa_etichetta_collo (gchar *ordine, int collo)
{
    char szBuffer[256];

	if(collo){
		sprintf(szBuffer,"%s,%d,%s", ordine, collo, Cfg.szLabelPrinterConsole);
		SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC_COLLO,szBuffer);
	} else {
		sprintf(szBuffer,"%s,%s", ordine, Cfg.szLabelPrinterConsole);
		SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC,szBuffer);
	}
}

void do_stampa_xab_ordine (gchar *ordine, gboolean storico)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s", ordine, Cfg.szXABPrinterConsole);
	if(storico){
		SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB_STORICO,szBuffer);
	} else {
		SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_XAB,szBuffer);
	}
}

void do_stampa_packing_list_collo (gchar ordine, int collo, gboolean storico)
{
	char szBuffer[128];

	if(collo){
		sprintf(szBuffer,"%s,%d,%s", ordine, collo, Cfg.szPrinterConsole);
		if(storico){
			SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST_COLLO_STORICO,szBuffer);
		} else {
			SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST_COLLO,szBuffer);
		}
	} else {
		sprintf(szBuffer,"%s,%s", ordine, Cfg.szPrinterConsole);
		SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST,szBuffer);
	}
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    // do_stampa_ordine_collo(win);
}

static GActionEntry entries[] = {
  {"ok", ok_activated, NULL, NULL, NULL},
  {"close", close_activated, NULL, NULL, NULL}
};

static void main_app_stampa_ordine_collo_init (MainAppStampaOrdineCollo *win)
{
    gchar *titolo, *question;
    MainAppStampaOrdineColloPrivate *priv;

    priv = main_app_stampa_ordine_collo_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->type   = _type;
    priv->ordine = _ordine;

    switch(priv->type){
        case STAMPA_ETICHETTE_ORDINE:
            titolo = g_strdup("Stampa Etichette Ordine");
            question = g_strdup("Inserire il codice ordine e il numero collo\n(0) stampa tutte le etichette");
        break;
        case STAMPA_XAB_ORDINE:
            titolo = g_strdup("Stampa XAB Ordine");
            question = g_strdup("Inserire il codice ordine");
            gtk_widget_hide(priv->sb_collo);
            gtk_widget_hide(priv->lb_collo);
        break;
        case STAMPA_DETTAGLIO_ORDINE:
            titolo = g_strdup("Stampa Dettaglio Ordine");
            question = g_strdup("Inserire il codice ordine");
            gtk_widget_hide(priv->sb_collo);
            gtk_widget_hide(priv->lb_collo);
        break;
        case STAMPA_DATI_ORDINE:
            titolo = g_strdup("Stampa Dati Ordine");
            question = g_strdup("Inserire il codice ordine");
            gtk_widget_hide(priv->sb_collo);
            gtk_widget_hide(priv->lb_collo);
        break;
        case STAMPA_PACKING_LIST_ORDINE:
            titolo = g_strdup("Stampa Packing List Ordine");
            question = g_strdup("Inserire il codice ordine e il numero collo\n(0) stampa tutte le packing list");
        break;
        default:
            titolo = g_strdup("NON SUPPORTATA");
            question = g_strdup("NON SUPPORTATA");
            gtk_widget_hide(priv->sb_collo);
            gtk_widget_hide(priv->lb_collo);
        break;
    }

    gtk_window_set_title (GTK_WINDOW (win), titolo);
    gtk_label_printf(priv->lb_msg , question);
    g_free(titolo);
    g_free(question);

    if(priv->ordine)
        gtk_entry_set_text(GTK_ENTRY(priv->entry_ordine), priv->ordine);

    init_actions(win, entries, G_N_ELEMENTS(entries), "stampa_ordine_collo");
}

static void main_app_stampa_ordine_collo_dispose (GObject *object)
{
    MainAppStampaOrdineColloPrivate *priv;

    priv = main_app_stampa_ordine_collo_get_instance_private (MAIN_APP_STAMPA_ORDINE_COLLO (object));

    if(priv->ordine){
        g_free(priv->ordine);
        priv->ordine=NULL;
    }

    G_OBJECT_CLASS (main_app_stampa_ordine_collo_parent_class)->dispose (object);
}

static void main_app_stampa_ordine_collo_class_init (MainAppStampaOrdineColloClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_stampa_ordine_collo_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/stampa_ordine_collo.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, cb_storico       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, cb_stampa_su_file);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, lb_msg           );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, lb_ordine        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, lb_collo         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, entry_ordine     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStampaOrdineCollo, sb_collo         );
}

/*
* attenzione ! la dispose viene chiamata più volte quindi evitare chiamate multiple alla free ...
*/
MainAppStampaOrdineCollo * main_app_stampa_ordine_collo_new (MainAppWindow *win, STAMPA_ORDINE_COLLO_TYPE_ENUM t, gchar *o)
{
    _type = t;
    _ordine = NULL;
    if(o!=NULL){
        _ordine = g_strdup(o);
    }

    MainAppStampaOrdineCollo *w = g_object_new (MAIN_APP_STAMPA_ORDINE_COLLO_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppStampaOrdineColloPrivate *priv = main_app_stampa_ordine_collo_get_instance_private (MAIN_APP_STAMPA_ORDINE_COLLO (w));

    priv->main_app_window = win;

    return w;
}
