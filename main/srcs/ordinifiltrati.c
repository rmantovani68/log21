#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>

#include <gtk-support.h>
#include <msg-box.h>
#include <dbfun-gtk.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "ordinifiltrati.h"
#include "mainfun.h"

#define WINDOW_NAME "ordini-filtrati-window"

struct _OrdiniFiltratiWindow
{
    GtkDialog parent;
};

typedef struct _OrdiniFiltratiWindowPrivate OrdiniFiltratiWindowPrivate;

struct _OrdiniFiltratiWindowPrivate
{
    MainWindow *main_window;

    gchar *pszCfgFileName;

    GtkWidget *cb_filtro_cedola;
    GtkWidget *cb_filtro_spedizione;
    GtkWidget *cb_filtro_linea;
    GtkWidget *cb_filtro_vettore;
    GtkWidget *cb_cambia_vettore;
    GtkWidget *sw_list_ordini_filtrati;
};

G_DEFINE_TYPE_WITH_PRIVATE(OrdiniFiltratiWindow, ordini_filtrati, GTK_TYPE_DIALOG)

void refresh_lista_ordini_filtrati(gpointer win)
{
	char szDataKey [256];
	char szBuffer [256];
	char szFiltro [256];
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));
	

	sprintf(szDataKey," true ");
	strcpy(szBuffer,gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->cb_filtro_cedola)));

	if (strlen(szBuffer)){
		sprintf(szFiltro," and rotpspe='%s' ", szBuffer);
		strcat(szDataKey, szFiltro);
	}
	strcpy(szBuffer,gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->cb_filtro_spedizione)));
	if (strlen(szBuffer)){
		sprintf(szFiltro," and rotpspe='%s' ", szBuffer);
		strcat(szDataKey, szFiltro);
	}
	strcpy(szBuffer,gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->cb_filtro_linea)));
	if (strlen(szBuffer)){
		sprintf(szFiltro," and trim(rocdlin)='%s' ", szBuffer);
		strcat(szDataKey, szFiltro);
	}

	strcpy(szBuffer,"");
	strncat(szBuffer,gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->cb_filtro_vettore)),6);
	if (strlen(szBuffer)){
		sprintf(szFiltro," and rocdve2='%s' ", szBuffer);
		strcat(szDataKey, szFiltro);
	}

    trace_debug(TRUE, 1,  "refresh_lista_ordini_filtrati : query  |%s|",szDataKey);

    char szSelectCmd[4096];
    GList *PS=NULL;
	add_item_to_parse(&PS,"%ORDTIPO%",Cfg.szTipoOrdini,TRUE);
	add_item_to_parse(&PS,"%KEY%",szDataKey,FALSE);
    GetFileString("ordini filtrati","lista_ordini_filtrati", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 
	RefreshTable(find_child(GTK_WIDGET (win),"sw_list_ordini_filtrati"),"lst",szSelectCmd, Cfg.nTipoSelezione,PS, NULL, NULL);
}

ep_bool_t CambiaVettoreOrdine(char *szOrdine,char *szNuovoVettore)
{
	ep_bool_t bRetValue=FALSE;
	DBresult *DBRes;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rocdve2='%s' where ordprog='%s';", szNuovoVettore,szOrdine);
	bRetValue = (DBresultStatus(DBRes)==DBRES_COMMAND_OK);
	DBclear(DBRes);

	return bRetValue;
}

ep_bool_t RipristinaVettoreOrdine(char *szOrdine)
{
	ep_bool_t bRetValue=FALSE;
	DBresult *DBRes;

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rocdve2=rocdveo where ordprog='%s';",szOrdine);
	bRetValue = (DBresultStatus(DBRes)==DBRES_COMMAND_OK);
	DBclear(DBRes);

	return bRetValue;
}


/*
* Cambia il vettore agli ordini selezionati
*/
void do_cambia_vettore (gpointer win, gchar *vettore)
{
	char szOrdine[64];
	int nOrdini=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;

	DBRes=DBExecQuery(FALSE,"select s.ordprog from sel_ord_tmp_%s_%s s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nOrdini=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szOrdine,DBgetvalue(DBRes,nIndex,0));
		if(!CambiaVettoreOrdine(szOrdine,vettore)){
            trace_debug_gtk("RED", TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Errore in cambio vettore a ordine [%s]\n",szOrdine);
		} else {
			nOrdini++;
		}
	}
	DBclear(DBRes);

    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Cambiato il vettore %s a %d ordini\n",vettore,nOrdini);
	refresh_lista_ordini_filtrati(win);
}

/*
* Ripristina il vettore originale agli ordini selezionati
*/
void do_ripristina_vettore (gpointer win)
{
	char szOrdine[64];
	int nOrdini=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;

	/*
	* Riprendo il dlg_parent dal dlg
	*/
	DBRes=DBExecQuery(FALSE,"select s.ordprog from sel_ord_tmp_%s_%s s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nOrdini=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szOrdine,DBgetvalue(DBRes,nIndex,0));
		if(!RipristinaVettoreOrdine(szOrdine)){
            trace_debug_gtk("RED", TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Errore in ripristino vettore a ordine [%s]\n",szOrdine);
		} else {
			nOrdini++;
		}
	}
	DBclear(DBRes);

    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Ripristinato il vettore a %d ordini\n",nOrdini);

	refresh_lista_ordini_filtrati(win);
}

/*
* Riassegna_note_corriere
*/
void do_riassegna_note_corriere(gpointer win)
{
	char szOrdine[64];
	int nOrdini=0;
	int nIndex;
	int nItems;
	DBresult *DBRes;

	DBRes=DBExecQuery(FALSE,"select s.ordprog from sel_ord_tmp_%s_%s s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
	nItems=DBntuples(DBRes);
	nOrdini=0;
	for (nIndex=0; nIndex<nItems; nIndex++){
		strcpy(szOrdine,DBgetvalue(DBRes,nIndex,0));
		if(!SetNoteOrdine(win, szOrdine)){
            trace_debug_gtk("RED", TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Errore in riassegnazione nota corriere [%s]\n",szOrdine);
		} else {
			nOrdini++;
		}
	}
	DBclear(DBRes);

    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Riassegnata nota corriere a %d ordini\n",nOrdini);

	refresh_lista_ordini_filtrati(win);
}

static void insert_ordine_callback(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer win)
{
    gchar *ptr;
    DBresult *DBRes = NULL;
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

    gtk_tree_model_get_text(model, iter, 0, &ptr);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"insert into sel_ord_tmp_%s_%s select ordprog from ric_ord where ordprog='%s' and ordtipo='%s';",
        Cfg.szTmpSuffix,
        Cfg.szTipoOrdini,
        ptr,
        Cfg.szTipoOrdini);
    if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        trace_debug_gtk("RED", TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Errore nella selezione ordini filtrati (%s)\n",ptr);
    }
    DBclear(DBRes);

    g_free(ptr);
}



void do_cambia_vettore_activated(gpointer win)
{
	GList *lista;
	ep_bool_t bOK=TRUE;
	int nTuples;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

	lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
		/*
		* Almeno un ordine selezionato
		*/
		char *pszString;
		char szORDPROG[128];
		char szNuovoVettore[128];
		int nRowIndex=-1;
		DBresult *DBRes = NULL;

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where false;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);

		szNuovoVettore[0]='\0';
		// prendo solo i primi 6 caratteri
		strncat(szNuovoVettore,gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->cb_cambia_vettore)),6);

        gtk_tree_selection_selected_foreach (TREE_SELECTION_LST(lst), insert_ordine_callback, (gpointer)win );

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select * from sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
		nTuples=DBntuples(DBRes);
		DBclear(DBRes);

		if (nTuples){
            char szSelectCmd[4096];
            GetFileString("ordini filtrati","lista_ordini", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 
            int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                            "Cambio Vettore", 
                            GTK_MESSAGE_QUESTION, 
                            GTK_BUTTONS_OK_CANCEL, 
                            szSelectCmd, 
                            GTK_SELECTION_SINGLE, 
                            NULL, 
                            "Premere OK per cambiare il vettore in %s agli ordini selezionati",szNuovoVettore);
            switch(rc){
                case GTK_RESPONSE_OK:
                    do_cambia_vettore(win, szNuovoVettore);
                break;
            }
		} else {
			/*
			* nessun ordine della distribuzione selezionata e' nello stato corretto
			*/
            dlg_msg( GTK_WINDOW(win), "Cambio Vettore", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
		}
    } else {
        dlg_msg( GTK_WINDOW(win), "Cambio Vettore", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
    }

	g_list_free(lista);
}

void do_ripristina_vettore_activated(gpointer win)
{
	ep_bool_t bOK=TRUE;
	int nTuples;
	GList *lista;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

	lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
		/*
		* Almeno un ordine selezionato
		*/
		char *pszString;
		char szORDPROG[128];
		int nRowIndex=-1;
		DBresult *DBRes = NULL;

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where false;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);


        gtk_tree_selection_selected_foreach (TREE_SELECTION_LST(lst), insert_ordine_callback, (gpointer)win );

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select * from sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
		nTuples=DBntuples(DBRes);
		DBclear(DBRes);

		if (nTuples){
            char szSelectCmd[4096];
            GetFileString("ordini filtrati","lista_ordini", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 
            int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                            "Ripristina Vettore", 
                            GTK_MESSAGE_QUESTION, 
                            GTK_BUTTONS_OK_CANCEL, 
                            szSelectCmd, 
                            GTK_SELECTION_SINGLE, 
                            NULL, 
                            "Premere OK per ripristinare il vettore originale agli ordini selezionati");
            switch(rc){
                case GTK_RESPONSE_OK:
                    do_ripristina_vettore(win);
                break;
            }
		} else {
			/*
			* nessun ordine della distribuzione selezionata e' nello stato corretto
			*/
            dlg_msg( GTK_WINDOW(win), "Ripristina Vettore Originale", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
		}
    } else {
        dlg_msg( GTK_WINDOW(win), "Ripristina Vettore Originale", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
    }
	g_list_free(lista);
}

void do_riassegna_note_activated(gpointer  win)
{
	ep_bool_t bOK=TRUE;
	int nTuples;
	GList *lista;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst");
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

	lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
		/*
		* Almeno un ordine selezionato
		*/
		char *pszString;
		char szORDPROG[128];
		int nRowIndex=-1;
		DBresult *DBRes = NULL;

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"drop table sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"create table sel_ord_tmp_%s_%s as select ordprog from ric_ord where false;", Cfg.szTmpSuffix, Cfg.szTipoOrdini); DBclear(DBRes);


        gtk_tree_selection_selected_foreach (TREE_SELECTION_LST(lst), insert_ordine_callback, (gpointer)win );

		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select * from sel_ord_tmp_%s_%s;", Cfg.szTmpSuffix, Cfg.szTipoOrdini);
		nTuples=DBntuples(DBRes);
		DBclear(DBRes);

		if (nTuples){
            char szSelectCmd[4096];
            GetFileString("ordini filtrati","lista_ordini", "", szSelectCmd, sizeof(szSelectCmd), priv->pszCfgFileName,NULL); 
            int rc = dlg_msg_with_table(GTK_WINDOW(win), 
                            "Riassegna Note Corriere", 
                            GTK_MESSAGE_QUESTION, 
                            GTK_BUTTONS_OK_CANCEL, 
                            szSelectCmd, 
                            GTK_SELECTION_SINGLE, 
                            NULL, 
                            "Premere OK per riassegnare Note Corriere agli ordini selezionati");
            switch(rc){
                case GTK_RESPONSE_OK:
                    do_riassegna_note_corriere(win);
                break;
            }
		} else {
			/*
			* nessun ordine della distribuzione selezionata e' nello stato corretto
			*/
            dlg_msg( GTK_WINDOW(win), "Riassegna Note Corriere", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
		}
    } else {
        dlg_msg( GTK_WINDOW(win), "Riassegna Note Corriere", GTK_MESSAGE_INFO,GTK_BUTTONS_CLOSE,"Nessun ordine selezionato!");
    }
	g_list_free(lista);
}



/*
* do_lista_ordini_filtrati(dlg)
*/
void do_lista_ordini_filtrati(gpointer win)
{
	DBresult *DBRes;
	int nIndex;
    OrdiniFiltratiWindowPrivate *priv;

    priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_cedola), ""); 
	DBRes=DBExecQuery(1,"select distinct(ronmced) from ric_ord order by ronmced;");
	for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
		gchar *szBuffer;
		szBuffer=g_strdup_printf("%s",DBgetvalue(DBRes,nIndex,0));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_cedola), szBuffer); 
		g_free(szBuffer);
	}
	DBclear(DBRes);
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->cb_filtro_cedola), 0);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_spedizione), ""); 
	DBRes=DBExecQuery(1,"select distinct(rotpspe) from ric_ord order by rotpspe;");
	for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
		gchar *szBuffer;
		szBuffer=g_strdup_printf("%s",DBgetvalue(DBRes,nIndex,0));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_spedizione), szBuffer); 
		g_free(szBuffer);
	}
	DBclear(DBRes);
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->cb_filtro_spedizione), 0);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_linea), ""); 
	DBRes=DBExecQuery(1,"select distinct(rocdlin) from ric_ord order by rocdlin;");
	for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
		gchar *szBuffer;
		szBuffer=g_strdup(DBgetvalue(DBRes,nIndex,0));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_linea), szBuffer); 
		g_free(szBuffer);
	}
	DBclear(DBRes);
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->cb_filtro_linea), 0);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_vettore), ""); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_cambia_vettore), ""); 
	DBRes=DBExecQuery(1,"select elccdvet||' - '||elcdsvet from ttve order by elccdvet;");
	for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
		gchar *szBuffer;
		szBuffer=g_strdup(DBgetvalue(DBRes,nIndex,0));
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_filtro_vettore), szBuffer); 
		gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->cb_cambia_vettore), szBuffer); 
		g_free(szBuffer);
	}
	DBclear(DBRes);
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->cb_filtro_vettore), 0);
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->cb_cambia_vettore), 0);

	refresh_lista_ordini_filtrati(win);

}



/*************************************************************************************************************
* FINE DLG_ORDINI_FILTRATI
*************************************************************************************************************/




static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void filtra_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	refresh_lista_ordini_filtrati(win);
}

static void clear_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	do_lista_ordini_filtrati(win);
}

static void cambia_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_cambia_vettore_activated(win);
}

static void ripristina_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_ripristina_vettore_activated(win);
}

static void riassegna_note_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_riassegna_note_activated(win);
}

static GActionEntry entries[] = {
  {"close",          close_activated,          NULL, NULL, NULL},
  {"filtra",         filtra_activated,         NULL, NULL, NULL},
  {"clear",          clear_activated,          NULL, NULL, NULL},
  {"cambia",         cambia_activated,         NULL, NULL, NULL},
  {"ripristina",     ripristina_activated,     NULL, NULL, NULL},
  {"riassegna_note", riassegna_note_activated, NULL, NULL, NULL}
};

static void ordini_filtrati_init (OrdiniFiltratiWindow *win)
{
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (win));

    gtk_widget_init_template (GTK_WIDGET (win));

    priv->pszCfgFileName = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

	do_lista_ordini_filtrati(win);

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);
}

static void ordini_filtrati_dispose (GObject *object)
{
    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (object));

    if(priv->pszCfgFileName){
        g_free(priv->pszCfgFileName);
        priv->pszCfgFileName = NULL;
    }

    G_OBJECT_CLASS (ordini_filtrati_parent_class)->dispose (object);
}

static void ordini_filtrati_class_init (OrdiniFiltratiWindowClass *class)
{
    G_OBJECT_CLASS (class)->dispose = ordini_filtrati_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, cb_filtro_cedola       );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, cb_filtro_spedizione   );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, cb_filtro_linea        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, cb_filtro_vettore      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, cb_cambia_vettore      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), OrdiniFiltratiWindow, sw_list_ordini_filtrati);
}

OrdiniFiltratiWindow * ordini_filtrati_new (MainWindow *win)
{
    OrdiniFiltratiWindow *w = g_object_new (ORDINI_FILTRATI_WINDOW_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    OrdiniFiltratiWindowPrivate *priv = ordini_filtrati_get_instance_private (ORDINI_FILTRATI_WINDOW (w));

    priv->main_window = win;

    return w;
}
