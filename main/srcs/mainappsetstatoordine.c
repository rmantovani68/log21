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
#include "mainappsetstatoordine.h"
#include "mainfun.h"

struct _MainAppSetStatoOrdine
{
    GtkDialog parent;
};

typedef struct _MainAppSetStatoOrdinePrivate MainAppSetStatoOrdinePrivate;

struct _MainAppSetStatoOrdinePrivate
{
    MainAppWindow *main_app_window;
    GtkWidget *rb_ricevuto;
    GtkWidget *rb_spedito;
    GtkWidget *rb_elaborato;
    GtkWidget *rb_stampato_rac;
    GtkWidget *rb_in_prelievo;
    GtkWidget *rb_prelevato;
    GtkWidget *rb_evaso;
    GtkWidget *rb_stampato_xab;
    GtkWidget *rb_stampato_dist;
    GtkWidget *rb_spedito_host;
    GtkWidget *rb_file_inviato;
};

G_DEFINE_TYPE_WITH_PRIVATE(MainAppSetStatoOrdine, main_app_set_stato_ordine, GTK_TYPE_DIALOG)

ep_bool_t set_stato_ordine(gpointer win, gchar *ordine, char stato_ordine, char stato_colli, char stato_righe)
{
    ep_bool_t rc = TRUE;
	DBresult *DBRes;
    MainAppSetStatoOrdinePrivate *priv = main_app_set_stato_ordine_get_instance_private (win);
	char szSQLCmd[512];

    GtkWidget *lst = find_child(GTK_WIDGET(priv->main_app_window),"lst_ordini");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");
    /*
    * Se si tratta di una evasione setto la data, l'ora e il peso reale
    */
    if(stato_ordine==ORDINE_EVASO){
        /* cambio lo stato dei colli non evasi */
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c',cppsrea=cppspre,cptmeva='now' where ordprog='%s' and cpstato!='%c';", stato_colli, ordine, stato_colli);
        DBFreeQueryResult(DBRes);
    } else {
        if(stato_colli) {
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c' where ordprog='%s';",stato_colli,ordine);
            DBFreeQueryResult(DBRes);
        }
    }

    if(stato_righe){
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpstato='%c' where ordprog='%s';",stato_righe,ordine);
        DBFreeQueryResult(DBRes);
    }

    /*
    * Se si tratta di una evasione setto la data, l'ora e il peso reale
    */
    if(stato_ordine==ORDINE_EVASO){
        sprintf(szSQLCmd,"update ric_ord set rostato='%c',ropsrea=ropspre,rotmeva='now' where ordprog='%s' and rostato!='%c';", stato_ordine, ordine, stato_ordine);
    } else {
        sprintf(szSQLCmd,"update ric_ord set rostato='%c' where ordprog='%s';",stato_ordine,ordine);
    }
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);
    
    if(atoi(DBcmdTuples(DBRes))==0){
        trace_debug_gtk("RED",TRUE,1,txt_msgs,"Fallito settaggio stato [%c] Ordine [%s]",stato_ordine,ordine);
        rc = FALSE;
    }

    DBFreeQueryResult(DBRes);

    return rc;
}

static void do_set_stato_ordini(gpointer win)
{
    GList *lista;
    GtkTreeIter iter;
    char *pszOrdine;
    char szOrdine[256];
	char szSQLCmd[512];
	int nOrdini=0;
	DBresult *DBRes;
    MainAppSetStatoOrdinePrivate *priv = main_app_set_stato_ordine_get_instance_private (win);

    GtkWidget *lst = find_child(GTK_WIDGET(priv->main_app_window),"lst_ordini");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(priv->main_app_window), "txt_msgs");

    char SO=0;
    char SC=0;
    char SR=0;
    gboolean ok = TRUE;

    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_ricevuto     ))) {SO = ORDINE_RICEVUTO          ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_prelevato    ))) {SO = ORDINE_PRELEVATO         ; SC=COLLO_CHIUSO;      SR=RIGA_PRELEVATA;   } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_spedito      ))) {SO = ORDINE_SPEDITO           ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_evaso        ))) {SO = ORDINE_EVASO             ; SC=COLLO_EVASO;       SR=RIGA_EVASA;       } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_elaborato    ))) {SO = ORDINE_ELABORATO         ; SC=COLLO_ELABORATO;   SR=RIGA_ELABORATA;   } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_stampato_rac ))) {SO = ORDINE_STAMPATA_RAC      ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_stampato_xab ))) {SO = ORDINE_STAMPATA_XAB      ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_stampato_dist))) {SO = ORDINE_STAMPATA_DISTINTA ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_in_prelievo  ))) {SO = ORDINE_IN_PRELIEVO       ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_spedito_host ))) {SO = ORDINE_SPEDITO_HOST      ;                                            } else   
    if(gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON(priv->rb_file_inviato ))) {SO = ORDINE_FILE_INVIATO      ;                                            } else
        ok = FALSE;

	if(ok==FALSE){
        trace_debug_gtk("RED",TRUE,1, txt_msgs, "Errore in set stato ordini");
		return;
	}

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
        do {
            if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
                /* ho ottenuto l'iter */
                gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,Cfg.nOrdiniKeyField, &pszOrdine);
                strcpy(szOrdine,pszOrdine);
                g_free(pszOrdine);

                if( set_stato_ordine(win, szOrdine, SO, SC, SR)) {
                    /*
                    * Update a video
                    */
                    UpdateOrdine(priv->main_app_window, szOrdine);
                    trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"ordine -> [%s][%c]", szOrdine,SO);
                    nOrdini++;
                }
            }
        } while((lista=g_list_next(lista)));
        trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"settati %d ordini", nOrdini);
    }
}



static void close_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    gtk_widget_destroy(GTK_WIDGET(win));
}

static void ok_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    do_set_stato_ordini(win);
}

static GActionEntry entries[] = {
  {"ok", ok_activated, NULL, NULL, NULL},
  {"close", close_activated, NULL, NULL, NULL}
};

static void main_app_set_stato_ordine_init (MainAppSetStatoOrdine *win)
{
    MainAppSetStatoOrdinePrivate *priv;

    priv = main_app_set_stato_ordine_get_instance_private (win);
    gtk_widget_init_template (GTK_WIDGET (win));
    
    init_actions(win, entries, G_N_ELEMENTS(entries), "set_stato_ordine");
}

static void main_app_set_stato_ordine_dispose (GObject *object)
{
    MainAppSetStatoOrdinePrivate *priv;

    priv = main_app_set_stato_ordine_get_instance_private (MAIN_APP_SET_STATO_ORDINE (object));

    G_OBJECT_CLASS (main_app_set_stato_ordine_parent_class)->dispose (object);
}

static void main_app_set_stato_ordine_class_init (MainAppSetStatoOrdineClass *class)
{
    G_OBJECT_CLASS (class)->dispose = main_app_set_stato_ordine_dispose;

    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/easy-picking/mainapp/set_stato_ordine.ui");

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_ricevuto     );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_spedito      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_elaborato    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_stampato_rac );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_in_prelievo  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_prelevato    );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_evaso        );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_stampato_xab );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_stampato_dist);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_spedito_host );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainAppSetStatoOrdine, rb_file_inviato );

}

MainAppSetStatoOrdine * main_app_set_stato_ordine_new (MainAppWindow *win)
{
    MainAppSetStatoOrdine *w = g_object_new (MAIN_APP_SET_STATO_ORDINE_TYPE, "transient-for", win, "use-header-bar", TRUE, NULL);

    MainAppSetStatoOrdinePrivate *priv = main_app_set_stato_ordine_get_instance_private (MAIN_APP_SET_STATO_ORDINE (w));

    priv->main_app_window = win;

    return w;
}
