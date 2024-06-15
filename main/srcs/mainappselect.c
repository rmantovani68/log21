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
#include "mainappselect.h"
#include "mainfun.h"

struct _MainAppSelect
{
    GtkDialog parent;
};

typedef struct _MainAppSelectPrivate MainAppSelectPrivate;

struct _MainAppSelectPrivate
{
    gpointer main_app_window;

    gchar *pszCfgFileName;

    GtkWidget *sw            ;
    GtkWidget *pb_close      ;
    GtkWidget *pb_expand     ;
    GtkWidget *pb_and        ;
    GtkWidget *pb_or         ;
    GtkWidget *pb_start_sel  ;
    GtkWidget *pb_reset_sel  ;
    GtkWidget *pb_select_all ;
    GtkWidget *pb_lancio     ;
    GtkWidget *pb_articoli   ;
    GtkWidget *cb_field      ;
    GtkWidget *cb_expression ;
    GtkWidget *lb_selection  ;
    GtkWidget *txf_select    ;

    GtkTextBuffer *select_buffer;

    char szSelectOrdiniCmd[256];
    char szSelectOrdiniCondition[256];
    char szFiltroSelezione[256];
    char szListaCampi[256];

};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppSelect, main_app_select, GTK_TYPE_DIALOG)

static void init_select(gpointer win)
{
	DBresult *DBRes = NULL;
	int nFields;
	int nFieldIndex;
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	/*
	* Lista Campi da utilizzare nelle selezioni
	* al primo posto ci vuole sempre ordprog
	*/
	GetFileString("Selezione Ordini","lista_campi", "ordprog,rocdrid,rodscli,rolocli,roprcli,rocpcli,roswcol,ronmced", priv->szListaCampi, 256,priv->pszCfgFileName,NULL);

	/*
	* Reset Select Ordini da lanciare
	*/

	sprintf(priv->szSelectOrdiniCmd,"select %s from ric_ord where ",priv->szListaCampi);
	strcpy(priv->szSelectOrdiniCondition,"");
	sprintf(priv->szFiltroSelezione," AND (rostato is NULL OR rostato='%c' OR rostato='%c') AND ordtipo='%s'",ORDINE_RICEVUTO,ORDINE_SPEDITO,Cfg.szTipoOrdini);

    priv->select_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(priv->txf_select));

    gtk_text_buffer_set_text (priv->select_buffer, priv->szSelectOrdiniCmd, -1);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_expression), "");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_expression), "Uguale a");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_expression), "Maggiore di");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_expression), "Minore di");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_expression), "Diverso da");

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_field), "");
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select %s from ric_ord where false;",priv->szListaCampi);
	nFields=DBnfields(DBRes);
	for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
        gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_field), DBfname(DBRes,nFieldIndex));
	}

	gtk_widget_set_sensitive(priv->pb_lancio,FALSE);
	gtk_widget_set_sensitive(priv->pb_articoli,FALSE);
	gtk_widget_set_sensitive(priv->pb_and,FALSE);
	gtk_widget_set_sensitive(priv->pb_or,FALSE);
}


static void do_lista_articoli (gpointer win, gchar *cfg_file_name, gpointer title)
{
	DBresult *DBRes;
    char szSelectCmd[4096];

	/*
	* Tabella sel_art_tmp_[TIPO_ORDINI] : 
	* sacdpro
	* sanmrgh
	* sanmcpe
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_art_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBFreeQueryResult(DBRes);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_art_tmp_%s_%s as select r.racdpro as sacdpro,count(r.racdpro) as sanmrgh,sum(r.raqtord) as sanmcpe from ric_art r,sel_ord_tmp_%s_%s s where r.ordprog=s.ordprog group by r.racdpro;",
		Cfg.szTmpSuffix, Cfg.szTipoOrdini, Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	DBFreeQueryResult(DBRes);

    GetFileString("distribuzione","lista_articoli", "", szSelectCmd, sizeof(szSelectCmd), cfg_file_name,NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                    "Lista Articoli", 
                    GTK_MESSAGE_INFO, 
                    GTK_BUTTONS_CLOSE, 
                    szSelectCmd, 
                    GTK_SELECTION_SINGLE, 
                    NULL, 
                    title?title:"");
}


static void do_articoli(gpointer win)
{
	DBresult *DBRes;
	char szSelectCmd[1024];
	ep_bool_t bOK=TRUE;
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s;",Cfg.szTipoOrdini); DBFreeQueryResult(DBRes);

	sprintf(szSelectCmd,"create table sel_ord_tmp_%s as select ordprog from ric_ord where ",Cfg.szTipoOrdini);
	strcat(szSelectCmd,priv->szSelectOrdiniCondition);
	strcat(szSelectCmd,priv->szFiltroSelezione);
	strcat(szSelectCmd,";");

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSelectCmd);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		gtk_text_printf("RED",txt_msgs,"Errore nella selezione ordini\n");
		bOK=FALSE;
	}
	DBFreeQueryResult(DBRes);

	if(bOK){
		do_lista_articoli(win, priv->pszCfgFileName, "Lista Articoli relativi alla selezione");
	}
}


static void expand_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static void and_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	strcat(priv->szSelectOrdiniCondition," AND ");
	gtk_widget_set_sensitive(priv->pb_and,FALSE);
	gtk_widget_set_sensitive(priv->pb_or,FALSE);
}

static void or_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	strcat(priv->szSelectOrdiniCondition," OR ");
	gtk_widget_set_sensitive(priv->pb_and,FALSE);
	gtk_widget_set_sensitive(priv->pb_or,FALSE);
}

static void start_selection_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	int nTuples;
	DBresult *DBRes;
    GtkWidget *lst;
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	strcat(priv->szSelectOrdiniCmd,priv->szSelectOrdiniCondition);
	strcat(priv->szSelectOrdiniCmd,priv->szFiltroSelezione);
	strcat(priv->szSelectOrdiniCmd,";");

    gtk_text_buffer_set_text (priv->select_buffer, priv->szSelectOrdiniCmd, -1);

	if((lst=find_child(GTK_WIDGET(win),"lst"))){
		gtk_widget_destroy(lst);
	}

	nTuples=CreateListFromSelect(priv->sw,"lst",GTK_SELECTION_SINGLE,priv->szSelectOrdiniCmd);

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,priv->szSelectOrdiniCmd);
	if(DBresultStatus(DBRes) == DBRES_TUPLES_OK ){
		gtk_label_printf(priv->lb_selection,"Selezionati %d ordini",nTuples);
		if (nTuples){
            gtk_widget_set_sensitive(priv->pb_lancio,TRUE);
            gtk_widget_set_sensitive(priv->pb_articoli,TRUE);
		}
	} else {
		gtk_label_printf(priv->lb_selection,"SELECT ERRATA");
		gtk_widget_set_sensitive(priv->pb_lancio,FALSE);
		gtk_widget_set_sensitive(priv->pb_articoli,FALSE);
        if((lst=find_child(GTK_WIDGET(win),"lst"))){
			gtk_widget_destroy(lst);
		}
	}
	DBFreeQueryResult(DBRes);
	/*
	* Reset Select Ordini e Lancio Ordini
	*/
	sprintf(priv->szSelectOrdiniCmd,"select %s from ric_ord where ",priv->szListaCampi);
    gtk_text_buffer_set_text (priv->select_buffer, priv->szSelectOrdiniCmd, -1);
}

static void reset_selection_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	sprintf(priv->szSelectOrdiniCmd,"select %s from ric_ord where ",priv->szListaCampi);
	strcpy(priv->szSelectOrdiniCondition,"");
    gtk_text_buffer_set_text (priv->select_buffer, priv->szSelectOrdiniCmd, -1);

	gtk_label_printf(priv->lb_selection," ");

	gtk_widget_set_sensitive(priv->pb_lancio,FALSE);
	gtk_widget_set_sensitive(priv->pb_articoli,FALSE);
}

static void select_all_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));

	strcat(priv->szSelectOrdiniCondition,"TRUE");
}

static void lancio_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

static void articoli_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_articoli(win);
}


static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Close Action : activated\n");
}

static GActionEntry entries[] = {
  {"close",             close_activated,           NULL, NULL, NULL},
  {"expand",            expand_activated,          NULL, NULL, NULL},    
  {"and",               and_activated,             NULL, NULL, NULL},
  {"or",                or_activated,              NULL, NULL, NULL},
  {"start_selection",   start_selection_activated, NULL, NULL, NULL},
  {"reset_selection",   reset_selection_activated, NULL, NULL, NULL},
  {"select_all",        select_all_activated,      NULL, NULL, NULL},
  {"lancio",            lancio_activated,          NULL, NULL, NULL},
  {"articoli",          articoli_activated,        NULL, NULL, NULL}
};



static void main_app_select_init (MainAppSelect *win)
{
    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (win));
    gtk_widget_init_template (GTK_WIDGET (win));

    priv->pszCfgFileName = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    init_select(win);
    
    init_actions(win, entries, G_N_ELEMENTS(entries), "select");
}

static void main_app_select_dispose (GObject *object)
{
    MainAppSelectPrivate *priv;

    priv = main_app_select_get_instance_private (MAIN_APP_SELECT (object));

    if(priv->pszCfgFileName) {
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (main_app_select_parent_class)->dispose (object);
}

static void main_app_select_class_init (MainAppSelectClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_select_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/select.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, sw            );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_close      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_expand     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_and        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_or         );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_start_sel  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_reset_sel  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_select_all );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_lancio     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, pb_articoli   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, cb_field      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, cb_expression );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, lb_selection  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSelect, txf_select    );

}

MainAppSelect * main_app_select_new (gpointer win)
{
    MainAppSelect *w = g_object_new (MAIN_APP_SELECT_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppSelectPrivate *priv = main_app_select_get_instance_private (MAIN_APP_SELECT (w));

    priv->main_app_window = win;

    return w;
}
