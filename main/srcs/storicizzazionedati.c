#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>


#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "storicizzazionedati.h"
#include "mainfun.h"

#define WINDOW_NAME "storicizzazione-dati-window"

struct _StoricizzazionedatiWindow
{
    GtkDialog parent;
};

typedef struct _StoricizzazionedatiWindowPrivate StoricizzazionedatiWindowPrivate;

struct _StoricizzazionedatiWindowPrivate
{
    MainWindow *main_window;
    gchar *pszCfgFileName;
    GtkWidget *lb_msg;
    GtkWidget *lb_label;
    GtkWidget *pr_progress;
    GtkWidget *pb_ok;
    GtkWidget *pb_close;
	int nValuesOrdini;
};

G_DEFINE_TYPE_WITH_PRIVATE(StoricizzazionedatiWindow, storicizzazione_dati, GTK_TYPE_DIALOG)

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
    StoricizzazionedatiWindowPrivate *priv;

    priv = storicizzazione_dati_get_instance_private (STORICIZZAZIONE_DATI_WINDOW (win));

	gtk_label_printf(priv->lb_msg,"Attendere la fine della procedura di storicizzazione");
	gtk_widget_set_sensitive(priv->pb_ok,FALSE);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"BEGIN WORK;");
	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini);
	nNumeroRecords=DBntuples(DBRes);

	for(nIndex=0;nIndex<nNumeroRecords;nIndex++){
	
		/*
		* LG 18-10-2007
		* controllo che tutti i colli sia in stato H prima della storicizzazione
		* se no salto l'ordine.
		*/
		if (!OrdineProduzione(DBgetvalue(DBRes,nIndex,0))) {
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione [ordine %s]. Ordine senza colli di produzione\n",DBgetvalue(DBRes,nIndex,0));
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
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione righe [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* righe storicizzate : cancello le righe di produzione */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from rig_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResDelete);
		}
		DBclear(DBResRighe);

		/*
		* Colli
		*/
		DBResColli=DBExecQuery(Cfg.nDebugLevel>1,"insert into col_prod_stor select * from col_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResColli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione colli [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* colli storicizzati : cancello i colli di produzione */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from col_prod where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResDelete);
		}
		DBclear(DBResColli);

		/*
		* Ordini / Articoli Ricevuti / Note
		*/
		DBResOrdini=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_ord_stor select * from ric_ord where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResOrdini) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione ordini ricevuti [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* ordini ricevuti storicizzati : cancello gli ordini ricevuti */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_ord where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResDelete);
		}
		DBclear(DBResOrdini);

		char *fields_list ="ordprog, racdpro, raprrow, raubicc, ratpcon, raqtord, raqtor , raqtpes, raswffo, rapcric, rapcrip, rapzpro, racdiva, rapciva, rapzpre, racdflg, ranmced";

		/*
		* 07-01-2022 rm
		*/
		DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_art_stor (%s) select %s from ric_art where ordprog='%s';",fields_list, fields_list, DBgetvalue(DBRes,nIndex,0));

		/*
		* ordprog     | text                        |
		* racdpro     | text                        |
		* raprrow     | text                        |
		* raubicc     | text                        |
		* ratpcon     | text                        |
		* raqtord     | integer                     | default 0
		* raqtor      | integer                     | default 0
		* raqtpes     | integer                     | default 0
		* raswffo     | text                        |
		* rapcric     | integer                     | default 0
		* rapcrip     | integer                     | default 0
		* rapzpro     | integer                     | default 0
		* racdiva     | text                        |
		* rapciva     | integer                     | default 0
		* rapzpre     | integer                     | default 0
		* racdflg     | text                        |
		* ranmced     | integer                     | default 0
		* */

		// DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_art_stor select * from ric_art where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));

		if(DBresultStatus(DBResArticoli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione articoli ricevuti [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* articoli ricevuti storicizzati : cancello gli articoli ricevuti */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_art where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResDelete);
		}
		DBclear(DBResArticoli);

		DBResArticoli=DBExecQuery(Cfg.nDebugLevel>1,"insert into ric_note_stor select * from ric_note where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
		if(DBresultStatus(DBResArticoli) != DBRES_COMMAND_OK){
            trace_debug_gtk("RED",TRUE,TRUE,find_child(GTK_WIDGET(priv->main_window), "txt_msgs"),"Fallita storicizzazione note ricevute [ordine %s]",DBgetvalue(DBRes,nIndex,0));
		} else {
			/* note ricevute storicizzate : cancello le note ricevute */
			DBResDelete=DBExecQuery(Cfg.nDebugLevel>1,"delete from ric_note where ordprog='%s';",DBgetvalue(DBRes,nIndex,0));
			DBclear(DBResDelete);
		}
		DBclear(DBResArticoli);

	    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(priv->pr_progress), (gdouble)nIndex / (gdouble)nNumeroRecords);
	}

	DBclear(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"COMMIT WORK;");
	DBclear(DBRes);
	
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord;");
	nNumeroRecords=DBntuples(DBRes);
	DBclear(DBRes);
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
			DBclear(DBResDelete);
		}
		DBclear(DBResArticoli);
	}
	
    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_refresh_table(priv->main_window, (gchar *)"sw_list_ordini", (gchar *)"lst", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Ordini", "lista_ordini", Cfg.nTipoSelezione , NULL);

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
	gboolean ok=TRUE;
    StoricizzazionedatiWindowPrivate *priv;

    priv = storicizzazione_dati_get_instance_private (STORICIZZAZIONE_DATI_WINDOW (win));

    gtk_widget_set_sensitive(priv->pb_ok,TRUE);


	DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini);
	nNumeroRecords=DBntuples(DBRes);
	DBclear(DBRes);

	if(nNumeroRecords){
		DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and roflimc='%c';",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini, CARICHI_ORDINE_DA_IMPORTARE);
		nNumeroRecordsKO=DBntuples(DBRes);
		DBclear(DBRes);
		if(nNumeroRecordsKO){
			ok=FALSE;
			/*
			* Visualizza messaggio di errore
			*/
            dlg_msg( GTK_WINDOW(win), "Storicizzazione Ordini", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,
                "Tra gli ordini da storicizzare ce ne sono alcuni ancora non importati nel programma carichi\nSe il programma carichi non e' gia' attivo, farlo partire\nQuindi aspettare 5 minuti e riprovare a storicizzare.");

		} else {
			DBRes=DBExecQuery(TRUE,"select ordprog from ric_ord where rostato='%c' and ordtipo='%s' and rofliex not in ('%c','%c');",ORDINE_SPEDITO_HOST,Cfg.szTipoOrdini,EMAIL_SPEDITA_XAB,EMAIL_NON_SPEDITA_XAB);
			nNumeroRecordsKO=DBntuples(DBRes);
			DBclear(DBRes);
			if(nNumeroRecordsKO){
				ok=FALSE;
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
    return ok;
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

static void storicizzazione_dati_init (StoricizzazionedatiWindow *win)
{
    StoricizzazionedatiWindowPrivate *priv;

    priv = storicizzazione_dati_get_instance_private (win);

    priv->pszCfgFileName = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    gtk_widget_init_template (GTK_WIDGET (win));

    init_storicizzazione(win);

    
    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void storicizzazione_dati_dispose (GObject *object)
{
    StoricizzazionedatiWindowPrivate *priv;

    priv = storicizzazione_dati_get_instance_private (STORICIZZAZIONE_DATI_WINDOW (object));

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (storicizzazione_dati_parent_class)->dispose (object);
}

static void storicizzazione_dati_class_init (StoricizzazionedatiWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = storicizzazione_dati_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), StoricizzazionedatiWindow, pb_ok);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), StoricizzazionedatiWindow, pb_close);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), StoricizzazionedatiWindow, lb_msg);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), StoricizzazionedatiWindow, lb_label);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), StoricizzazionedatiWindow, pr_progress);
}

StoricizzazionedatiWindow * storicizzazione_dati_new (MainWindow *win)
{
    StoricizzazionedatiWindow *w = g_object_new (STORICIZZAZIONE_DATI_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    StoricizzazionedatiWindowPrivate *priv = storicizzazione_dati_get_instance_private (STORICIZZAZIONE_DATI_WINDOW (w));

    priv->main_window = win;

    return w;
}
