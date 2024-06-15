#include <gtk/gtk.h>
#include <glib/gprintf.h>
#include <sys/stat.h>
#include <time.h>

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>
#include <msg-box.h>
#include <ep-about.h>
#include <ep-popover.h>

#include "edit_bancale.h"
#include "edit_distinta.h"
#include "edit_prodotto.h"
#include "import_spedizioni.h"
#include "import_catalogo.h"

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

static MainApp *_app;

#define WINDOW_NAME "main-window"

G_DEFINE_TYPE_WITH_PRIVATE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

void on_row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    g_printf("row-activated\n");
}

gboolean on_button_pressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
        g_print ("Single right click on the tree view.\n");

        /* optional: select row if no row is selected or only
        *  one other row is selected (will only do something
        *  if you set a tree selection mode as described later
        *  in the tutorial) */
        if (1) {
            GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

            /* Note: gtk_tree_selection_count_selected_rows() does not
            *   exist in gtk+-2.0, only in gtk+ >= v2.2 ! */
            if (gtk_tree_selection_count_selected_rows(selection)  <= 1) {
                GtkTreePath *path;

                /* Get tree path for row that was clicked */
                if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint) event->x, (gint) event->y, &path, NULL, NULL, NULL)) {
                    gtk_tree_selection_unselect_all(selection);
                    gtk_tree_selection_select_path(selection, path);
                    gtk_tree_path_free(path);
                }
            }
        } /* end of optional bit */

        // TODO view_popup_menu(treeview, event, userdata);

        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;

}

static gboolean init_db_connection(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    gboolean rc;

    /*
    * connect to db
    */
    if(!(rc = DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword))){        
        trace_debug_gtk("RED",TRUE,TRUE,NULL,"Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        g_printf("Connection to database [%s:%s] failed.\n", Cfg.szDBHost, Cfg.szDBName);
        // dlg_msg(GTK_WINDOW(win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Connection to database [%s:%s] failed.", Cfg.szPGHost, Cfg.szPGDataBase);
    }

    return rc;
}

static void close_db_connection(void)
{
    DBDisconnect();
}

static void init_trace(MainWindow *win)
{
#ifdef TRACE
    MainWindowPrivate *priv = get_main_window_private_instance (win);


    char szBufTrace[80];

    strcpy(szBufTrace, Cfg.szPathTrace);
    strcat(szBufTrace,"/");
    strcat(szBufTrace, priv->pszAppName);
    strcat(szBufTrace, TRACE_FILE_SUFFIX);

    open_trace(priv->pszAppName, szBufTrace, PICKING_TRACE_FILE_MAXSIZE);

    trace_debug(FALSE, TRUE, "Module Name     : %s",priv->pszAppName);
    trace_debug(FALSE, TRUE, "Version         : %s",__version__);
    trace_debug(FALSE, TRUE, "Customer Name   : %s",__customer__);
    trace_debug(FALSE, TRUE, "Authors         : %s",__authors__);
    trace_debug(FALSE, TRUE, "Copyright       : %s",__copyright__);
    trace_debug(FALSE, TRUE, "RCSID String    : %s",rcsid);
    trace_debug(TRUE, TRUE, "Started");
#endif
}

static void Exit(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    close_db_connection();

    /* rimuovo il timer per gestione eventi*/
    if(Cfg.nMainTimer)
        g_source_remove(Cfg.nMainTimer);

    /* cancello la coda messaggi principale */
    DeleteProcessMsgQ(priv->nPID);

    trace_debug(TRUE, TRUE, "Stopped");

    close_trace ();
}


static void search_changed_callback (GtkSearchEntry *entry, gpointer win)
{
#ifdef TODO
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win),"lst_ordini");
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");
	char szParagraph[128];
	char szBuffer[128];
	char szData[128];
	int nRowIndex=0;
	int nRiga=-1;
	int nKeyIndex=-1;
	char *pszString;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    const gchar *pSearch = gtk_entry_get_text (GTK_ENTRY(entry));
    int n = strlen(pSearch);

	strcpy(szParagraph,"Tabella Ordini");
	strcpy(szBuffer,"lista_ordini");
	strcat(szBuffer,"_key");
	nKeyIndex=GetFileInt(szParagraph,szBuffer, 0, priv->pszCfgFileName,NULL); 


    gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter);

    do {

        gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,nKeyIndex, &pszString);
        
        if(!g_ascii_strcasecmp (pszString, pSearch)){
            /* found */
            gtk_tree_view_set_cursor(TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, FALSE);
            gtk_tree_view_scroll_to_cell (TREE_VIEW_LST(lst), gtk_tree_model_get_path (TREE_MODEL_LST(lst), &iter), NULL, TRUE, 0.5, 0.0);
            break;
        }

        g_free(pszString);

    } while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst), &iter));

#endif
}

static void init_app(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    /* read configuration from cfg file */
    ReadCfg(priv->pszCfgFileName, CFGItems, TRUE);

    init_trace(win);

    trace_debug(TRUE, TRUE, "Local Module ID : %d",priv->nPID);

    /*
    * Verifico la presenza di una istanza attiva del processo MAIN
    * controllando la presenza della coda MSG
    */
    if(TRUE || OpenProcessMsgQ(priv->nPID)<0){
        /*
        * coda messaggi non presente - tutto ok
        * creo la coda locale per l'applicazione
        */
        if(CreateProcessMsgQ(priv->nPID, 1)<0){
            trace_debug(TRUE, TRUE, "Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Creazione coda msg di %s (%d) fallita", priv->pszAppName, priv->nPID);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        }
        gtk_search_bar_connect_entry (GTK_SEARCH_BAR (priv->search_bar), GTK_ENTRY (priv->search_entry));
        g_signal_connect (priv->search_entry, "search-changed" , G_CALLBACK (search_changed_callback), win);

        gchar *pszTitle=g_strdup_printf("%s %s-%s - Gestione Carichi %s", __application_name__, __version__, __customer__, rcsid);
        gtk_window_set_title (GTK_WINDOW (win), pszTitle);
        g_free(pszTitle);

        /* Database */
        if(init_db_connection(win)){
            /*
            refresh_table(win, (gchar *)"sw_list_ordini", (gchar *)"lst_ordini", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Ordini", (gchar *)"lista_ordini", Cfg.nTipoSelezione);
            g_signal_connect(find_child(GTK_WIDGET(win), "lst_ordini"), "row-activated"     , G_CALLBACK (on_row_activated),  win);
            g_signal_connect(find_child(GTK_WIDGET(win), "lst_ordini"), "button-press-event", G_CALLBACK (on_button_pressed), win);
            */

            /*
            * Timer per gestione eventi
            */
            Cfg.nMainTimer = g_timeout_add(Cfg.nMainDelay,ProcessMsgs, win);

        } else {
            dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"error in database connection");
            // gtk_widget_destroy (GTK_WIDGET (win));
            // g_application_quit (G_APPLICATION (_app));
        }
    } else {
        dlg_msg(GTK_WINDOW (win), "Application error", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Altra istanza dell'applicazione presente");
        gtk_widget_destroy (GTK_WIDGET (win));
        g_application_quit (G_APPLICATION (_app));
    }
}

/* --------------------------------------- */
/* Utility Functions                       */
/* --------------------------------------- */

/*
* used in the 'calendar' button
*/
static void set_generic_date_button (gpointer win, GtkButton *button)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    EpPopover *ep = EP_POPOVER(ep_popover_new (GTK_WIDGET(button), EP_TYPE_CALENDAR, GTK_BUTTONS_NONE));

    gtk_popover_popup(GTK_POPOVER(ep));
    int rc = ep_popover_run(ep);
    switch(rc){
        case GTK_RESPONSE_OK:
        {
            guint day, month, year;
            gtk_calendar_get_date (GTK_CALENDAR(ep_popover_get_field(ep, EP_FIELD_CALENDAR)), &year, &month, &day);
            const gchar *date = g_strdup_printf("%04d-%02d-%02d", year, month+1, day);
            gtk_button_set_label(button, date);
            g_free((gpointer)date);
        }
        break;
    }
    gtk_widget_destroy(GTK_WIDGET(ep));
}



/* --------------------------------------- */
/* Actions                                 */
/* --------------------------------------- */


static void giacenze_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->box_giacenze);
}


static void giacenze_edit_prodotto_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_totali",0);

    w = (GtkWidget *)edit_prodotto_window_new (GTK_WINDOW(win), cdpro, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(cdpro)
        g_free(cdpro);
}


static void giacenze_ingresso_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_totali",0);
    gchar *dstit = get_field_from_list_selected_row(win, "lst_giacenze_totali",1);


	EpPopoverField fields[3]; gchar *values[3];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_TITLE;   values[i++]=dstit; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_giacenze_totali_ingresso), EP_TYPE_PRODUCT_TITLE_QUANTITY, do_giacenze_totali_ingresso);

    if(cdpro)
        g_free(cdpro);

    if(dstit)
        g_free(dstit);

}

static void giacenze_spedizione_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_totali",0);

	EpPopoverField fields[2]; gchar *values[2];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_giacenze_totali_spedizione), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_spedizione);

    if(cdpro)
        g_free(cdpro);
}

static void giacenze_conteggia_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_totali",0);

	EpPopoverField fields[2]; gchar *values[2];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_giacenze_totali_conteggia), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_conteggia);

    if(cdpro)
        g_free(cdpro);
}

static void giacenze_differenza_inventariale_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_totali",0);

	EpPopoverField fields[2]; gchar *values[2];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_giacenze_totali_conteggia), EP_TYPE_PRODUCT_QUANTITY, do_giacenze_totali_differenza_inventariale);

    if(cdpro)
        g_free(cdpro);
}

static void giacenze_storicizza_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi storicizzazione giacenze ?");
    switch(rc){
        case GTK_RESPONSE_YES:
			do_giacenze_totali_storicizza(win);
        break;
    }

}

static void giacenze_stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
	g_printf(__FUNCTION__);
	g_printf("\n");
}


static void about_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    w = about_window_new (GTK_WINDOW(win), GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL);

    gtk_window_present (GTK_WINDOW (w));
}


static void quit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    int rc=dlg_msg( GTK_WINDOW(win), "Exit", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Uscire dall'applicazione ?");
    switch(rc){
        case GTK_RESPONSE_YES:
            Exit(win);
            gtk_widget_destroy (GTK_WIDGET (win));
            g_application_quit (G_APPLICATION (_app));
        break;
    }

}

static void action_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("Action : activated\n");
}

void search_activated(GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gboolean mode = gtk_search_bar_get_search_mode (GTK_SEARCH_BAR(priv->search_bar));

    gtk_search_bar_set_search_mode (GTK_SEARCH_BAR(priv->search_bar), !mode);
}

static void movimenti_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->box_movimenti);
}

static void catalogo_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->box_catalogo);
}

static void reports_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_stack_set_visible_child ( GTK_STACK(priv->stack_main), priv->box_reports);
}


static void giacenze_ricerca_pb_refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_giacenze));
    if(w == priv->box_giacenze_totali){
        refresh_box_giacenze_totali(win);
    }
    if(w == priv->box_giacenze_prodotti_udc){
        refresh_box_giacenze_prodotti_udc(win);
    }
    if(w == priv->box_giacenze_udc){
        refresh_box_giacenze_udc(win);
    }
    if(w == priv->box_giacenze_distinte){
        refresh_box_giacenze_distinte(win);
    }
}

static void giacenze_ricerca_pb_clear_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_giacenze));
    if(w == priv->box_giacenze_totali){
        clear_box_giacenze_totali(win);
        refresh_box_giacenze_totali(win);
    }
    if(w == priv->box_giacenze_prodotti_udc){
        clear_box_giacenze_prodotti_udc(win);
        refresh_box_giacenze_prodotti_udc(win);
    }
    if(w == priv->box_giacenze_udc){
        clear_box_giacenze_udc(win);
        refresh_box_giacenze_udc(win);
    }
    if(w == priv->box_giacenze_distinte){
        clear_box_giacenze_distinte(win);
        refresh_box_giacenze_distinte(win);
    }
}

static void pb_giacenze_stor_ricerca_refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_reports));
    if(w == priv->box_reports_storico_giacenze){
        GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_report_giacenze));
        if(w == priv->box_report_giacenze_totali){
            refresh_box_report_storico_giacenze_totali(win);
        }
        if(w == priv->box_report_giacenze_prodotti_udc){
            refresh_box_report_storico_giacenze_prodotti_udc(win);
        }
        if(w == priv->box_report_giacenze_udc){
            refresh_box_report_storico_giacenze_udc(win);
        }
        if(w == priv->box_report_giacenze_distinte){
            refresh_box_report_storico_giacenze_distinte(win);
        }
    }
    if(w == priv->box_reports_storico_movimenti){
        refresh_box_reports_storico_movimenti(win);
    }
    if(w == priv->box_reports_carichi_importati){
        refresh_box_reports_carichi_importati(win);
    }

}


static void pb_giacenze_stor_ricerca_clear_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_reports));
    if(w == priv->box_reports_storico_giacenze){
        GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_report_giacenze));
        if(w == priv->box_report_giacenze_totali){
            clear_box_report_storico_giacenze_totali(win);
            refresh_box_report_storico_giacenze_totali(win);
        }
        if(w == priv->box_report_giacenze_prodotti_udc){
            clear_box_report_storico_giacenze_prodotti_udc(win);
            refresh_box_report_storico_giacenze_prodotti_udc(win);
        }
        if(w == priv->box_report_giacenze_udc){
            clear_box_report_storico_giacenze_udc(win);
            refresh_box_report_storico_giacenze_udc(win);
        }
        if(w == priv->box_report_giacenze_distinte){
            clear_box_report_storico_giacenze_distinte(win);
            refresh_box_report_storico_giacenze_distinte(win);
        }
    }
    if(w == priv->box_reports_storico_movimenti){
        clear_box_reports_storico_movimenti(win);
        refresh_box_reports_storico_movimenti(win);
    }
    if(w == priv->box_reports_carichi_importati){
        clear_box_reports_carichi_importati(win);
        refresh_box_reports_carichi_importati(win);
    }

}



static void movimenti_ricerca_pb_from_date_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    set_generic_date_button (win, GTK_BUTTON(priv->movimenti_ricerca_pb_from_date));
}

static void movimenti_ricerca_pb_to_date_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    set_generic_date_button (win, GTK_BUTTON(priv->movimenti_ricerca_pb_to_date));
}


static void movimenti_ricerca_pb_refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    refresh_box_movimenti(win);
}

static void movimenti_ricerca_pb_clear_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    clear_box_movimenti(win);
    refresh_box_movimenti(win);
}


static void reports_movimenti_pb_from_date_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    set_generic_date_button (win, GTK_BUTTON(priv->reports_movimenti_pb_from_date));
}

static void reports_movimenti_pb_to_date_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    set_generic_date_button (win, GTK_BUTTON(priv->reports_movimenti_pb_to_date));
}

static void reports_movimenti_pb_refresh_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    refresh_box_reports_storico_movimenti(win);
}

static void reports_movimenti_pb_clear_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    clear_box_reports_storico_movimenti(win);
    refresh_box_reports_storico_movimenti(win);
}


static void movimenti_conteggia_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_conteggia_activated\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_movimenti",0);

	EpPopoverField fields[2]; gchar *values[2];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_movimentazioni_conteggia), EP_TYPE_UDC_PRODUCT_QUANTITY, do_conteggio_movimentazioni);

    if(cdpro)
        g_free(cdpro);

}

static void movimenti_diff_inv_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_diff_inv_activated\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_movimenti",0);

	EpPopoverField fields[2]; gchar *values[2];

	int i=0;
	fields[i] = EP_FIELD_PRODUCT; values[i++]=cdpro; 
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_movimentazioni_differenza_inventariale), EP_TYPE_PRODUCT_QUANTITY, do_differenza_inventariale);
    

    if(cdpro)
        g_free(cdpro);

}

static void movimenti_ingresso_merce_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_ingresso_merce\n");
}

static void movimenti_spedizione_merce_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_spedizione_merce\n");
}

static void movimenti_restituzione_merce_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_restituzione_merce\n");
}

static void  movimenti_annulla_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_annulla_activated\n");

    int rc=dlg_msg(GTK_WINDOW(win), "Annulla Movimentazioni", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi l'annullamento delle movimentazioni selezionate?");
    switch(rc){
        case GTK_RESPONSE_YES:
        break;
    }
}

static void  movimenti_stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf("movimenti_stampa_activated\n");

    int rc=dlg_msg(GTK_WINDOW(win), "Stampa Movimenti", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Confermi la stampa dei movimenti?");
    switch(rc){
        case GTK_RESPONSE_OK:
        break;
    }
}



static void prodotti_bancale_stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
    g_printf("\n");
}

static void prodotti_bancale_elimina_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
    g_printf("\n");
}

static void prodotti_bancale_edit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdpro = get_field_from_list_selected_row(win, "lst_giacenze_prodotti_udc",1);

    w = (GtkWidget *)edit_prodotto_window_new (GTK_WINDOW(win), cdpro, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(cdpro)
        g_free(cdpro);
}

                                       
static void bancali_edit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *cdudc = get_field_from_list_selected_row(win, "lst_giacenze_udc",0);

    w = (GtkWidget *)edit_bancale_window_new (GTK_WINDOW(win), cdudc, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(cdudc)
        g_free(cdudc);
}

static void bancali_set_distinta_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
}

static void bancali_stampa_packing_list_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
}

static void bancali_elimina_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
}

                                       
static void distinte_nuova_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
	gchar *msg=NULL;

	if(do_nuova_distinta(win, &msg)){
        trace_debug_gtk("BLUE",TRUE,TRUE,priv->txt_msgs,msg?msg:"Creata nuova distinta");
        refresh_box_giacenze_distinte(win);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,msg?msg:"Errore in creazione nuova distinta");
	}

	if(msg) g_free(msg);
}

static void distinte_stampa_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
}

static void distinte_file_dati_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
}

static void distinte_restituzione_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
	gchar *msg=NULL;

    gchar *nmdis = get_field_from_list_selected_row(win, "lst_giacenze_distinte",0);

	if(do_restituzione_distinta(win, nmdis, &msg)){
        trace_debug_gtk("BLUE",TRUE,TRUE,priv->txt_msgs,msg?msg:"Restituita distinta");
        refresh_box_giacenze_distinte(win);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,msg?msg:"Errore in restituzione distinta");
	}

	if(msg) g_free(msg);
}

static void distinte_storicizza_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
	gchar *msg=NULL;

    gchar *nmdis = get_field_from_list_selected_row(win, "lst_giacenze_distinte",0);

	if(do_storicizza_distinta(win, nmdis, &msg)){
        trace_debug_gtk("BLUE",TRUE,TRUE,priv->txt_msgs,msg?msg:"Storicizzata distinta");
        refresh_box_giacenze_distinte(win);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,msg?msg:"Errore in storicizzazione distinta");
	}

	if(msg) g_free(msg);
}

static void distinte_elimina_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
	gchar *msg=NULL;

    gchar *nmdis = get_field_from_list_selected_row(win, "lst_giacenze_distinte",0);

	if(do_elimina_distinta(win, nmdis, &msg)){
        trace_debug_gtk("BLUE",TRUE,TRUE,priv->txt_msgs,msg?msg:"Cancellata distinta");
        refresh_box_giacenze_distinte(win);
	} else {
        trace_debug_gtk("RED",TRUE,TRUE,priv->txt_msgs,msg?msg:"Errore in cancellazione distinta");
	}

	if(msg) g_free(msg);
}

static void distinte_edit_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    GtkWidget *w;

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gchar *nmdis = get_field_from_list_selected_row(win, "lst_giacenze_distinte",0);

    w = (GtkWidget *)edit_distinta_window_new (GTK_WINDOW(win), nmdis, FALSE);

    gtk_window_present (GTK_WINDOW (w));

    if(nmdis)
        g_free(nmdis);
}


static void crea_udc_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    g_printf(__FUNCTION__);
	g_printf("\n");
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	EpPopoverField fields[1]; gchar *values[1];

	int i=0;
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_crea_nuovi_udc), EP_TYPE_QUANTITY, do_crea_udc);
}

static void stampa_etichette_activated (GSimpleAction *action, GVariant *parameter, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	EpPopoverField fields[1]; gchar *values[1];

	int i=0;
	fields[i] = EP_FIELD_NONE;    values[i++]=NULL;

    do_action_using_ep_popover (win, fields, values, GTK_BUTTON(priv->pb_stampa_etichette), EP_TYPE_UDC, do_stampa_etichetta);

}











static void on_stack_main_visible_child_changed (GtkWidget *widget, GParamSpec *property, gpointer win)
{
    g_printf("on_stack_main_visible_child_changed\n");
    refresh_stack_main(win);
}

static void on_stack_giacenze_visible_child_changed (GtkWidget *widget, GParamSpec *property, gpointer win)
{
    g_printf("on_stack_giacenze_visible_child_changed\n");
    refresh_stack_giacenze(win);
}

static void on_stack_reports_visible_child_changed (GtkWidget *widget, GParamSpec *property, gpointer win)
{
    g_printf("on_stack_reports_visible_child_changed\n");
    refresh_stack_reports(win);
}

static void on_stack_report_giacenze_visible_child_changed (GtkWidget *widget, GParamSpec *property, gpointer win)
{
    g_printf("on_stack_reports_giacenze_visible_child_changed\n");
    refresh_stack_report_giacenze(win);
}



static GActionEntry entries[] =
{
    { "about",                                   about_activated,                              NULL, NULL, NULL },
    { "help",                                    action_activated,                             NULL, NULL, NULL },
    { "search",                                  search_activated,                             NULL, NULL, NULL },
    { "crea-udc",                                crea_udc_activated,                           NULL, NULL, NULL },
    { "stampa-etichette",                        stampa_etichette_activated,                   NULL, NULL, NULL },

    { "movimenti_ricerca_pb_from_date",          movimenti_ricerca_pb_from_date_activated,     NULL, NULL, NULL },
    { "movimenti_ricerca_pb_to_date",            movimenti_ricerca_pb_to_date_activated,       NULL, NULL, NULL },
    { "movimenti_ricerca_pb_refresh",            movimenti_ricerca_pb_refresh_activated,       NULL, NULL, NULL },
    { "movimenti_ricerca_pb_clear",              movimenti_ricerca_pb_clear_activated,         NULL, NULL, NULL },

    { "reports_movimenti_pb_from_date",          reports_movimenti_pb_from_date_activated,     NULL, NULL, NULL },
    { "reports_movimenti_pb_to_date",            reports_movimenti_pb_to_date_activated,       NULL, NULL, NULL },
    { "reports_movimenti_pb_refresh",            reports_movimenti_pb_refresh_activated,       NULL, NULL, NULL },
    { "reports_movimenti_pb_clear",              reports_movimenti_pb_clear_activated,         NULL, NULL, NULL },

    { "movimenti-conteggia",                     movimenti_conteggia_activated,                NULL, NULL, NULL },
    { "movimenti-differenza-inventariale",       movimenti_diff_inv_activated,                 NULL, NULL, NULL },
    { "movimenti-ingresso-merce",                movimenti_ingresso_merce_activated,           NULL, NULL, NULL },
    { "movimenti-spedizione-merce",              movimenti_spedizione_merce_activated,         NULL, NULL, NULL },
    { "movimenti-restituzione-merce",            movimenti_restituzione_merce_activated,       NULL, NULL, NULL },
    { "movimenti-annulla",                       movimenti_annulla_activated,                  NULL, NULL, NULL },
    { "movimenti-stampa",                        movimenti_stampa_activated,                   NULL, NULL, NULL },

    { "giacenze-edit-prodotto",                  giacenze_edit_prodotto_activated,             NULL, NULL, NULL },

    { "giacenze_ricerca_pb_refresh",             giacenze_ricerca_pb_refresh_activated,        NULL, NULL, NULL },
    { "giacenze_ricerca_pb_clear",               giacenze_ricerca_pb_clear_activated,          NULL, NULL, NULL },

    { "pb_giacenze_stor_ricerca_refresh",        pb_giacenze_stor_ricerca_refresh_activated,   NULL, NULL, NULL },
    { "pb_giacenze_stor_ricerca_clear",          pb_giacenze_stor_ricerca_clear_activated,     NULL, NULL, NULL },

    { "giacenze",                                giacenze_activated,                           NULL, NULL, NULL },
    { "movimenti",                               movimenti_activated,                          NULL, NULL, NULL },
    { "catalogo",                                catalogo_activated,                           NULL, NULL, NULL },
    { "reports",                                 reports_activated,                            NULL, NULL, NULL },

    { "quit",                                    quit_activated,                               NULL, NULL, NULL },

    { "prodotti-bancale-stampa",                 prodotti_bancale_stampa_activated,            NULL, NULL, NULL },
    { "prodotti-bancale-elimina",                prodotti_bancale_elimina_activated,           NULL, NULL, NULL },
    { "prodotti-bancale-edit",                   prodotti_bancale_edit_activated,              NULL, NULL, NULL },

    { "bancali-edit",                            bancali_edit_activated,                       NULL, NULL, NULL },
    { "bancali-set-distinta",                    bancali_set_distinta_activated,               NULL, NULL, NULL },
    { "bancali-stampa-packing-list",             bancali_stampa_packing_list_activated,        NULL, NULL, NULL },
    { "bancali-elimina",                         bancali_elimina_activated,                    NULL, NULL, NULL },

    { "distinte-nuova",                          distinte_nuova_activated,                     NULL, NULL, NULL },
    { "distinte-stampa",                         distinte_stampa_activated,                    NULL, NULL, NULL },
    { "distinte-file-dati",                      distinte_file_dati_activated,                 NULL, NULL, NULL },
    { "distinte-restituzione",                   distinte_restituzione_activated,              NULL, NULL, NULL },
    { "distinte-storicizza",                     distinte_storicizza_activated,                NULL, NULL, NULL },
    { "distinte-elimina",                        distinte_elimina_activated,                   NULL, NULL, NULL },
    { "distinte-edit",                           distinte_edit_activated,                      NULL, NULL, NULL },

    { "giacenze-ingresso",                       giacenze_ingresso_activated,                  NULL, NULL, NULL },
    { "giacenze-spedizione",                     giacenze_spedizione_activated,                NULL, NULL, NULL },
    { "giacenze-conteggia",                      giacenze_conteggia_activated,                 NULL, NULL, NULL },
    { "giacenze-differenza-inventariale",        giacenze_differenza_inventariale_activated,   NULL, NULL, NULL },
    { "giacenze-storicizza",                     giacenze_storicizza_activated,                NULL, NULL, NULL },
    { "giacenze-edit-prodotto",                  giacenze_edit_prodotto_activated,             NULL, NULL, NULL },
    { "giacenze-stampa",                         giacenze_stampa_activated,                    NULL, NULL, NULL }


};

MainWindowPrivate *get_main_window_private_instance (gpointer win)
{
    return  main_window_get_instance_private (MAIN_WINDOW(win));
}

static void main_window_init (MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	/*
	* Codice Macchina
	*/

	/* dati macchina */
	strcpy(Macchina.szNome,APP_NAME);
	Macchina.nNumero=APP_PID;
	sprintf(Macchina.szCodice,"%s_%02d", Macchina.szNome, Macchina.nNumero);


    priv->pszAppName       = g_strdup (APP_NAME);
    priv->pszAppTitle      = g_strdup (APP_TITLE);
    priv->pszAppClass      = g_strdup (APP_CLASS);
    priv->nPID             = APP_PID;
    priv->pszCfgFileName   = g_strdup_printf("%s/%s", g_get_current_dir (), __configuration_file__);

    priv->screen = gdk_screen_get_default ();
    priv->provider = gtk_css_provider_new ();

    gchar *css_filename = g_strdup_printf("/org/%s/%s/%s.css", priv->pszAppClass, priv->pszAppName, priv->pszAppName);
    gtk_css_provider_load_from_resource (priv->provider, css_filename);
    g_free(css_filename);

    gtk_style_context_add_provider_for_screen (priv->screen, GTK_STYLE_PROVIDER(priv->provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    gtk_widget_init_template (GTK_WIDGET (win));

    gchar *settings_filename = g_strdup_printf("org.%s.%s", APP_CLASS, APP_NAME);
    priv->settings = g_settings_new (settings_filename);
    g_free(settings_filename);

    priv->wsi = window_size_info_new (&(priv->wsi), WINDOW_NAME);

    window_load_state (win, priv->wsi, priv->settings);

    g_signal_connect(G_OBJECT(win), "window-state-event",       G_CALLBACK(on_window_state_event),   priv->wsi);
    g_signal_connect(G_OBJECT(win), "size-allocate",            G_CALLBACK(on_window_size_allocate), priv->wsi);
    g_signal_connect(G_OBJECT(priv->stack_main),             "notify::visible-child",    G_CALLBACK(on_stack_main_visible_child_changed),     win);
    g_signal_connect(G_OBJECT(priv->stack_giacenze),         "notify::visible-child",    G_CALLBACK(on_stack_giacenze_visible_child_changed), win);
    g_signal_connect(G_OBJECT(priv->stack_reports),          "notify::visible-child",    G_CALLBACK(on_stack_reports_visible_child_changed),  win);
    g_signal_connect(G_OBJECT(priv->stack_report_giacenze),  "notify::visible-child",    G_CALLBACK(on_stack_report_giacenze_visible_child_changed),  win);


    init_app(win);

    /* esempio di gimpruler */
    /*
    GtkWidget *gr = gimp_ruler_new(GTK_ORIENTATION_HORIZONTAL);
    gimp_ruler_set_range((GimpRuler *)gr, 0.0 , 100.0, 10.0);
    gtk_box_pack_start (GTK_BOX(find_child(GTK_WIDGET(win),"vbruler")), GTK_WIDGET(gr), TRUE, TRUE, 5);
    gtk_widget_show(GTK_WIDGET(gr));
    */

    init_actions(win, entries, G_N_ELEMENTS(entries), WINDOW_NAME);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), ""); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), "INGRESSO"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), "SPEDIZIONE"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), "CONTEGGIO"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), "RESTITUZIONE"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale), "DIFF_INV"); 
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->movimenti_ricerca_cb_causale), 0);

	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), ""); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), "INGRESSO"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), "SPEDIZIONE"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), "CONTEGGIO"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), "RESTITUZIONE"); 
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale), "DIFF_INV"); 
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->reports_movimenti_cb_causale), 0);
}

static void main_window_dispose (GObject *object)
{
    MainWindowPrivate *priv = get_main_window_private_instance (object);
    MainWindow *win = MAIN_WINDOW (object);

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

    if(priv->pszAppTitle){       g_free(priv->pszAppTitle);      priv->pszAppTitle = NULL;    }
    if(priv->pszAppName){        g_free(priv->pszAppName);       priv->pszAppName = NULL;    }
    if(priv->pszAppClass){       g_free(priv->pszAppClass);      priv->pszAppClass = NULL;    }
    if(priv->pszCfgFileName){    g_free(priv->pszCfgFileName);   priv->pszCfgFileName = NULL;    }

    G_OBJECT_CLASS (main_window_parent_class)->dispose (object);
}

static void main_window_class_init (MainWindowClass *class)
{
    g_printf("main_window_class_init\n");

    G_OBJECT_CLASS (class)->dispose = main_window_dispose;

    gchar *ui_filename = g_strdup_printf("/org/%s/%s/%s.ui", APP_CLASS, APP_NAME, WINDOW_NAME);
    gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), ui_filename);
    g_free(ui_filename);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_crea_nuovi_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_stampa_etichette);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  txt_msgs);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_main);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_giacenze);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_movimenti);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_catalogo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_reports);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_giacenze);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_giacenze_totali);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_giacenze_prodotti_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_giacenze_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_giacenze_distinte);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_reports);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_reports_storico_giacenze);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_reports_storico_movimenti);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_reports_carichi_importati);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  stack_report_giacenze);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_report_giacenze_totali);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_report_giacenze_prodotti_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_report_giacenze_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  box_report_giacenze_distinte);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_giacenze);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_movimenti);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_catalogo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  rb_reports);
    
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_bar  );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  search_entry);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_entry_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_entry_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_entry_titolo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_entry_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_entry_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_entry_titolo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_stor_ricerca_entry_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_stor_ricerca_entry_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_stor_ricerca_entry_titolo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_entry_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_entry_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_entry_titolo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  carichi_ricerca_entry_doc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  carichi_ricerca_entry_cdpro);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  carichi_ricerca_entry_dstit);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_cb_causale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_cb_causale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_pb_from_date);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_pb_to_date);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_pb_from_date);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_pb_to_date);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_pb_refresh);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_pb_clear);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_ingresso);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_spedizione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_conteggia);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_differenza_inventariale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_storicizza);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_edit_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_totali_stampa);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_prodotti_bancalati_stampa);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_prodotti_bancalati_elimina);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_prodotti_bancalati_edit_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_edit_bancale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_set_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_stampa_packing_list);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_elimina);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_crea);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_stampa);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_crea_file_dati);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_restituzione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_storicizza);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_elimina);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_edit_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_pb_refresh);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_pb_clear);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_ingresso);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_spedizione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_conteggia);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_restituzione);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_differenza_inventariale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_annulla_movimento);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_movimentazioni_stampa);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_stor_ricerca_refresh);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_giacenze_stor_ricerca_clear);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_prodotti_bancalati_stor_edit_prodotto);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_prodotti_bancalati_stor_satmpa);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_stor_edit_bancale);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_bancali_stor_stampa_packing_list);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_stor_crea_file_dati);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_stor_edit_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_distinte_stor_stampa_distinte);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_pb_refresh);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_pb_clear);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_carichi_ricerca_trova);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  pb_carichi_ricerca_pulisci);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sb_column);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_ricerca_sb_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  movimenti_ricerca_sb_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  giacenze_stor_ricerca_sb_distinta);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  reports_movimenti_sb_distinta);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_totali);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_prodotti_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_distinte);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_main_movimentazioni);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_main_catalogo);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_stor_totali);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_stor_prodotti_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_stor_udc);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_giacenze_stor_distinte);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_stor_movimenti);
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  sw_list_carichi_importati);

    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_giacenze_distinta );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_giacenze_udc      );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_giacenze_prodotto );
    gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), MainWindow,  lb_giacenze_dstit   );





/*
 * lista dei widgets presenti nella main-window
 *
 * le entry vanno gestite in lettura pwer la selezione delle liste associate
 *
<object class="GtkScrolledWindow" id="sw_list_giacenze_totali">
<object class="GtkScrolledWindow" id="sw_list_giacenze_prodotti_udc">
<object class="GtkScrolledWindow" id="sw_list_giacenze_udc">
<object class="GtkScrolledWindow" id="sw_list_giacenze_distinte">
<object class="GtkScrolledWindow" id="sw_list_main_movimentazioni">
<object class="GtkScrolledWindow" id="sw_list_main_catalogo">
<object class="GtkScrolledWindow" id="sw_list_giacenze_stor_totali">
<object class="GtkScrolledWindow" id="sw_list_giacenze_stor_prodotti_udc">
<object class="GtkScrolledWindow" id="sw_list_giacenze_stor_udc">
<object class="GtkScrolledWindow" id="sw_list_giacenze_stor_distinte">
<object class="GtkScrolledWindow" id="sw_list_stor_movimenti">
<object class="GtkScrolledWindow" id="sw_list_carichi_importati">
 *
 *
 *
<object class="GtkEntry" id="giacenze_ricerca_entry_udc">
<object class="GtkEntry" id="giacenze_ricerca_entry_prodotto">
<object class="GtkEntry" id="giacenze_ricerca_entry_titolo">
<object class="GtkEntry" id="movimenti_ricerca_entry_udc">
<object class="GtkEntry" id="movimenti_ricerca_entry_prodotto">
<object class="GtkEntry" id="movimenti_ricerca_entry_titolo">
<object class="GtkEntry" id="reports_entry_udc">
<object class="GtkEntry" id="reports_entry_prodotto">
<object class="GtkEntry" id="reports_entry_titolo">
<object class="GtkEntry" id="reports_movimenti_entry_cdudc">
<object class="GtkEntry" id="reports_movimenti_entry_cdpro">
<object class="GtkEntry" id="reports_movimenti_entry_titolo">
<object class="GtkEntry" id="carichi_ricerca_entry_doc">
<object class="GtkEntry" id="carichi_ricerca_entry_cdpro">
<object class="GtkEntry" id="carichi_ricerca_entry_dstit">

 *
 * i combo vanno impostati e letti per la selezione delle liste
 *
<object class="GtkComboBoxText" id="movimenti_ricerca_cb_causale">
<object class="GtkComboBoxText" id="reports_movimenti_cb_causale">


 *
 * alcuni pulsanti vanno gestiti come ad es: 
 *
<object class="GtkButton" id="movimenti_ricerca_pb_from_date">
<object class="GtkButton" id="movimenti_ricerca_pb_to_date">
<object class="GtkButton" id="reports_movimenti_pb_from_date">
<object class="GtkButton" id="reports_movimenti_pb_to_date">

 *
 * altri servono come 'relatives' per i popup...
 *
<object class="GtkButton" id="giacenze_ricerca_pb_trova">
<object class="GtkButton" id="giacenze_ricerca_pb_clean">
<object class="GtkButton" id="pb_giacenze_totali_ingresso">
<object class="GtkButton" id="pb_giacenze_totali_spedizione">
<object class="GtkButton" id="pb_giacenze_totali_conteggia">
<object class="GtkButton" id="pb_giacenze_totali_differenza_inventariale">
<object class="GtkButton" id="pb_giacenze_totali_storicizza">
<object class="GtkButton" id="pb_giacenze_totali_edit_prodotto">
<object class="GtkButton" id="pb_giacenze_totali_stampa">
<object class="GtkButton" id="pb_prodotti_bancalati_stampa">
<object class="GtkButton" id="pb_prodotti_bancalati_elimina">
<object class="GtkButton" id="pb_prodotti_bancalati_edit_prodotto">
<object class="GtkButton" id="pb_bancali_edit_bancale">
<object class="GtkButton" id="pb_bancali_set_distinta">
<object class="GtkButton" id="pb_bancali_stampa_packing_list">
<object class="GtkButton" id="pb_bancali_elimina">
<object class="GtkButton" id="pb_distinte_crea">
<object class="GtkButton" id="pb_distinte_stampa">
<object class="GtkButton" id="pb_distinte_crea_file_dati">
<object class="GtkButton" id="pb_distinte_restituzione">
<object class="GtkButton" id="pb_distinte_storicizza">
<object class="GtkButton" id="pb_distinte_elimina">
<object class="GtkButton" id="pb_distinte_edit_distinta">
<object class="GtkButton" id="movimenti_ricerca_pb_refresh">
<object class="GtkButton" id="movimenti_ricerca_pb_clear">
<object class="GtkButton" id="pb_movimentazioni_ingresso">
<object class="GtkButton" id="pb_movimentazioni_spedizione">
<object class="GtkButton" id="pb_movimentazioni_conteggia">
<object class="GtkButton" id="pb_movimentazioni_restituzione">
<object class="GtkButton" id="pb_movimentazioni_differenza_inventariale">
<object class="GtkButton" id="pb_movimentazioni_annulla_movimento">
<object class="GtkButton" id="pb_movimentazioni_stampa">
<object class="GtkButton" id="reports_pb_refresh">
<object class="GtkButton" id="reports_pb_clean">
<object class="GtkButton" id="pb_prodotti_bancalati_stor_edit_prodotto">
<object class="GtkButton" id="pb_prodotti_bancalati_stor_satmpa">
<object class="GtkButton" id="pb_bancali_stor_edit_bancale">
<object class="GtkButton" id="pb_bancali_stor_stampa_packing_list">
<object class="GtkButton" id="pb_distinte_stor_crea_file_dati">
<object class="GtkButton" id="pb_distinte_stor_edit_distinta">
<object class="GtkButton" id="pb_distinte_stor_stampa_distinte">
<object class="GtkButton" id="reports_movimenti_pb_refresh">
<object class="GtkButton" id="reports_movimenti_pb_clear">
<object class="GtkButton" id="pb_carichi_ricerca_trova">
<object class="GtkButton" id="pb_carichi_ricerca_pulisci">

*
* spin_button  da leggere per la selezione liste
*

<object class="GtkSpinButton" id="sb_column">
<object class="GtkSpinButton" id="giacenze_ricerca_sb_distinta">
<object class="GtkSpinButton" id="movimenti_ricerca_sb_distinta">
<object class="GtkSpinButton" id="reports_sb_distinta">
<object class="GtkSpinButton" id="reports_movimenti_sb_distinta">
*/

}

MainWindow *main_window_new (MainApp *app)
{
    _app = app;
    g_printf("main_window_new\n");
    MainWindow *win = g_object_new (MAIN_WINDOW_TYPE, "application", app, NULL);
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    priv->app = app;

    return win;
}
