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
#include "mainappstoricizzazionedati.h"
#include "mainfun.h"

struct _MainAppStoricizzazioneDati
{
    GtkDialog parent;
};

typedef struct _MainAppStoricizzazioneDatiPrivate MainAppStoricizzazioneDatiPrivate;

struct _MainAppStoricizzazioneDatiPrivate
{
    MainAppWindow *main_app_window;
    gchar *pszCfgFileName;
    GtkWidget *lb_msg;
    GtkWidget *lb_label;
    GtkWidget *pr_progress;
    GtkWidget *pb_ok;
    GtkWidget *pb_close;
	int nValuesOrdini;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppStoricizzazioneDati, main_app_storicizzazione_dati, GTK_TYPE_DIALOG)

static void do_storicizzazione_dati(gpointer win)
{
	DBresult *DBRes;
	DBresult *DBResDelete;
	DBresult *DBResRighe;
	DBresult *DBResColli;
	DBresult *DBResOrdini;
	DBresult *DBResArticoli;
	int nNumeroRecords;
	int nIndex;
    MainAppStoricizzazioneDatiPrivate *priv;

    priv = main_app_storicizzazione_dati_get_instance_private (MAIN_APP_STORICIZZAZIONE_DATI (win));

	gtk_label_printf(priv->lb_msg,"Attendere la fine della procedura di storicizzazione");
	gtk_widget_set_sensitive(priv->pb_ok,FALSE);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"BEGIN WORK;");
	DBFreeQueryResult(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini);
	nNumeroRecords=DBntuples(DBRes);

	for(nIndex=0;nIndex<nNumeroRecords;nIndex++){
	
		/*
		* LG 18-10-2007
		* controllo che tutti i colli sia in stato H prima della storicizzazione
		* se no salto l'ordine.
		*/
		if (!OrdineProduzione(DBgetvalue(DBRes,nIndex,0))) {
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione [ordine %s]. Ordine senza colli di produzione\n",DBgetvalue(DBRes,nIndex,0));
			continue;
		}
	
		/*
		* controllo che sia avvenuto l'inserimento nelle tabelle di storico
		* prima di cancellare la tuple dagli archivi di produzione
		*/

		/*
		* Righe
		*/
		DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"insert into rig_prod_stor select * from rig_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResRighe) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione righe [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* righe storicizzate : cancello le righe di produzione */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from rig_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResRighe);

		/*
		* Colli
		*/
		DBResColli=DBExecQuery(Cfg.nDebugLevel>1,"insert into col_prod_stor select * from col_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResColli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione colli [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* colli storicizzati : cancello i colli di produzione */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from col_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResColli);

		/*
		* Ordini / Articoli Ricevuti / Note
		*/
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_ord_stor select * from ric_ord where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResOrdini) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione ordini ricevuti [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* ordini ricevuti storicizzati : cancello gli ordini ricevuti */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_ord where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResOrdini);

		DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_art_stor select * from ric_art where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResArticoli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione articoli ricevuti [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* articoli ricevuti storicizzati : cancello gli articoli ricevuti */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_art where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResArticoli);

		DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_note_stor select * from ric_note where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResArticoli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs"),"Fallita storicizzazione note ricevute [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* note ricevute storicizzate : cancello le note ricevute */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_note where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResArticoli);

	    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(priv->pr_progress), (gdouble)nIndex / (gdouble)nNumeroRecords);
	}

	DBFreeQueryResult(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"COMMIT WORK;");
	DBFreeQueryResult(DBRes);
	
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord;");
	nNumeroRecords=DBntuples(DBRes);
	DBFreeQueryResult(DBRes);
	if (nNumeroRecords==0) {
		/*
		* Se non trovo ordini allora storicizzo anche le cedole presenti in cedole
		*/
		DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into cedole_stor select * from cedole;");
		if(DBresultStatus(DBResArticoli) != DBRES_COMMAND_OK){
#ifdef TRACE
			trace_debug(TRUE, TRUE, "Fallita storicizzazione cedole");
#endif
		} else {
			/* articoli ricevuti storicizzati : cancello gli articoli ricevuti */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"truncate cedole;");
			DBFreeQueryResult(DBResDelete);
		}
		DBFreeQueryResult(DBResArticoli);
	}
	
    refresh_table(priv->main_app_window, (gchar *)"sw_list_ordini", (gchar *)"lst", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Ordini", "lista_ordini", Cfg.nTipoSelezione );

	gtk_label_printf(priv->lb_msg,"Procedura di storicizzazione dati terminata\nPremere Cancel per uscire");
	gtk_widget_set_sensitive(priv->pb_close,TRUE);
	gtk_window_set_focus(GTK_WINDOW (win), priv->pb_close);
	gtk_update();
}

static gboolean init_storicizzazione(gpointer win)
{
	DBresult *DBRes;
	int nNumeroRecords;
	int nNumeroRecordsKO;
	ep_bool_t bStorOK=TRUE;
    MainAppStoricizzazioneDatiPrivate *priv;

    priv = main_app_storicizzazione_dati_get_instance_private (MAIN_APP_STORICIZZAZIONE_DATI (win));

    gtk_widget_set_sensitive(priv->pb_ok,TRUE);


	DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini);
	nNumeroRecords=DBntuples(DBRes);
	DBFreeQueryResult(DBRes);

	if(nNumeroRecords){
		DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and roflimc='%c';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini, CARICHI_ORDINE_DA_IMPORTARE);
		nNumeroRecordsKO=DBntuples(DBRes);
		DBFreeQueryResult(DBRes);
		if(nNumeroRecordsKO){
			bStorOK=FALSE;
			/*
			* Visualizza messaggio di errore
			*/
            dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,
                "Tra gli ordini da storicizzare ce ne sono alcuni ancora non importati nel programma carichi\nSe il programma carichi non e' gia' attivo, farlo partire\nQuindi aspettare 5 minuti e riprovare a storicizzare.");

		} else {
			DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and rofliex not in ('%c','%c');",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini,EMAIL_SPEDITA_XAB,EMAIL_NON_SPEDITA_XAB);
			nNumeroRecordsKO=DBntuples(DBRes);
			DBFreeQueryResult(DBRes);
			if(nNumeroRecordsKO){
				bStorOK=FALSE;
				/*
				* Visualizza messaggio di errore
				*/
                dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,
                    "Tra gli ordini da storicizzare ce ne sono alcuni per i quali non è stata prodotta l'XAB elettronica\nChiamare il supporto tecnico per verificare il problema");
			} else {
                gtk_label_printf(priv->lb_msg,"%d Ordini da storicizzare\nPremere Ok per confermare la storicizzazione",nNumeroRecords);
                gtk_widget_set_sensitive(priv->pb_ok,TRUE);
            }
		}

	} else {
		/*
		* Visualizza messaggio di errore
		*/
        dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Non sono presenti Ordini da storicizzare");
	}
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_storicizzazione_dati(win);
}

static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
  {"close", close_activated, NULL, NULL, NULL},
  {"ok",    ok_activated,    NULL, NULL, NULL}
};

static void main_app_storicizzazione_dati_init (MainAppStoricizzazioneDati *win)
{
    MainAppStoricizzazioneDatiPrivate *priv;

    priv = main_app_storicizzazione_dati_get_instance_private (win);

    priv->pszCfgFileName = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    gtk_widget_init_template (GTK_WIDGET (win));

    init_storicizzazione(win);

    
    init_actions(win, entries, G_N_ELEMENTS(entries), "storicizzazione_dati");
}

static void main_app_storicizzazione_dati_dispose (GObject *object)
{
    MainAppStoricizzazioneDatiPrivate *priv;

    priv = main_app_storicizzazione_dati_get_instance_private (MAIN_APP_STORICIZZAZIONE_DATI (object));

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (main_app_storicizzazione_dati_parent_class)->dispose (object);
}

static void main_app_storicizzazione_dati_class_init (MainAppStoricizzazioneDatiClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_storicizzazione_dati_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/storicizzazione_dati.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStoricizzazioneDati, pb_ok);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStoricizzazioneDati, pb_close);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStoricizzazioneDati, lb_msg);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStoricizzazioneDati, lb_label);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppStoricizzazioneDati, pr_progress);
}

MainAppStoricizzazioneDati * main_app_storicizzazione_dati_new (MainAppWindow *win)
{
    MainAppStoricizzazioneDati *w = g_object_new (MAIN_APP_STORICIZZAZIONE_DATI_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppStoricizzazioneDatiPrivate *priv = main_app_storicizzazione_dati_get_instance_private (MAIN_APP_STORICIZZAZIONE_DATI (w));

    priv->main_app_window = win;

    return w;
}
