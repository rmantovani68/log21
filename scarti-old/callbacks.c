/*
* Callbacks.c
* Modulo: Gestione Scarti
* Easy Picking 3.0 
* Copyright A&L srl 2003-2021
* Autori : Roberto Mantovani 
*          Stefano Tarroni
* Data Creazione : 28-06-2002 
*/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <gtk/gtk.h>

#ifdef TRACE
#include <trace.h>
#endif

#include <gtk/gtk.h>
#include <libpq-fe.h>

#include <signal.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include <proc_list.h>
#include <pmx_msq.h>
#include <dbfun.h>
#include <picking.h>

#include "main.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

/*
* Alcune globali di cui non posso fare a meno ...
*/
static GtkWidget *dlg_scarti=0;
static GtkWidget *dlg_storico_scarti=0;
static char szSelectedCDPRO[128];
static char szSelectedDSAUT[128];
static char szSelectedDSTIT[128];
static char szSelectedDSEDT[128];
static char szSelectedCDFLG[128];
static int nSelectedNMCPE;
static int nSelectedPREZZ;
static char szFromDate[128];
static char szToDate[128];

static int nNumCopie=1;

/*
* void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data)
*
* Effettua un ordinamento sulla clist : utilizza la colonna (nColumnNumber) selezionata come discriminante;
* se la stessa colonna viene selezionanata di seguito l'ordinamento varia da ascendente a discentente.
*/
void on_lst_sort_column_clicked (GtkCList *clist, gint nColumnNumber, gpointer user_data)
{
	static int nOldColumnNumber=-1;
	static int nOrd;

	if (nColumnNumber!=nOldColumnNumber){
		nOrd=0;
		nOldColumnNumber=nColumnNumber;
	} else {
		nOrd++;
	}
	gtk_clist_set_sort_type(GTK_CLIST(clist),(nOrd%2==0)?GTK_SORT_ASCENDING:GTK_SORT_DESCENDING);
	gtk_clist_set_sort_column(GTK_CLIST(clist), nColumnNumber);
	gtk_clist_sort(GTK_CLIST(clist));
}

void on_lst_scarti_select_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	PGresult *PGRes;
	char szCDPRO[128];
	char szDSAUT[128];
	char szDSTIT[128];
	char szDSEDT[128];
	char szCDFLG[128];
	char *pszString;
	int nCopie;
	int nPrezzo;
	BOOL bOK=TRUE;

	gtk_clist_get_text(clist,row,0,&pszString);
	strcpy(szCDPRO,pszString);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select trim(sccdpro),trim(scdsaut),trim(scdstit),trim(scdsedt),scnmcpe,scprezz,trim(sccdflg) from scarti where trim(sccdpro)='%s';",szCDPRO);
	if(PQntuples(PGRes)==0){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non presente !",szCDPRO);
		bOK=FALSE;
	} else if(PQntuples(PGRes)>1){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non univoco !",szCDPRO);
		strcpy(szCDPRO,PQgetvalue(PGRes,0,0));
		strcpy(szDSAUT,PQgetvalue(PGRes,0,1));
		strcpy(szDSTIT,PQgetvalue(PGRes,0,2));
		strcpy(szDSEDT,PQgetvalue(PGRes,0,3));
		nCopie = atoi(PQgetvalue(PGRes,0,4));
		nPrezzo= atoi(PQgetvalue(PGRes,0,5));
		strcpy(szCDFLG,PQgetvalue(PGRes,0,6));
	} else if(PQntuples(PGRes)==1){
		strcpy(szCDPRO,PQgetvalue(PGRes,0,0));
		strcpy(szDSAUT,PQgetvalue(PGRes,0,1));
		strcpy(szDSTIT,PQgetvalue(PGRes,0,2));
		strcpy(szDSEDT,PQgetvalue(PGRes,0,3));
		nCopie = atoi(PQgetvalue(PGRes,0,4));
		nPrezzo= atoi(PQgetvalue(PGRes,0,5));
		strcpy(szCDFLG,PQgetvalue(PGRes,0,6));
	}
	DBclear(PGRes);

	if(bOK){
		gtk_widget_set_sensitive(get_widget(dlg,"pb_modify"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"pb_delete"),TRUE);

		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cdpro")),StrTrimAll(szCDPRO));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_dsaut")),StrTrimAll(szDSAUT));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_dstit")),StrTrimAll(szDSTIT));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_dsedt")),StrTrimAll(szDSEDT));
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg")),StrTrimAll(szCDFLG));
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_nmcpe")),nCopie);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prezz")),nPrezzo);
	}
}

void on_lst_scarti_unselect_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	gtk_widget_set_sensitive(get_widget(GTK_WIDGET(user_data),"pb_modify"),FALSE);
	gtk_widget_set_sensitive(get_widget(GTK_WIDGET(user_data),"pb_delete"),FALSE);
}


/*********************************************************
************     DLG_EXIT          ***********************
*********************************************************/
void on_dlg_exit_pb_ok_clicked              (gpointer user_data, GtkButton *button)
{
	ExitApp();
}



/*********************************************************
************     DLG_CONFIG        ***********************
*********************************************************/

void on_dlg_config_pb_apply_clicked         (gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));
}

void on_dlg_config_pb_ok_clicked         (gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));

	gtk_widget_destroy(GTK_WIDGET(user_data));
}


/*********************************************************
************     DLG_APP_ERROR     ***********************
*********************************************************/
void on_dlg_app_error_pb_ok_clicked         (gpointer user_data, GtkButton *button)
{
	/* chiudo la GUI */
	gtk_main_quit();
}


/*********************************************************
************     MAIN_WINDOW       ***********************
*********************************************************/
gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) 
{
	return TRUE;
}

/*********************************************************
************    TOOLBAR BUTTONS    ***********************
*********************************************************/



void on_main_window_pb_settings_clicked     (gpointer user_data, GtkButton *button)
{
	do_config();
}

void on_main_window_pb_help_clicked         (gpointer user_data, GtkButton *button)
{
	do_about();
}

void on_main_window_pb_exit_clicked         (gpointer user_data, GtkButton *button)
{
  do_exit();
}

void on_main_window_pb_scarti_clicked         (gpointer user_data, GtkButton *button)
{
  do_scarti();
}

void on_main_window_pb_barcode_clicked      (gpointer user_data, GtkButton *button)
{
	do_barcode();
}

void on_main_window_pb_num_copie_clicked         (gpointer user_data, GtkButton *button)
{
  do_num_copie();
}


/*********************************************************
**************   MENUBAR OPTIONS    **********************
*********************************************************/


void on_pbm_exit_activate               (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_exit();
}


void on_pbm_configurazione_activate     (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_config();
}


void on_pbm_about_activate               (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_about();
}

void on_pbm_trace_activate                  (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_trace();
}

void on_pbm_reindex_activate                (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_message(main_window,"Riorganizza Indici","Premere Ok per riorganizzare gli indici",GTK_SIGNAL_FUNC(do_reindex));
}


void on_pbm_tabella_scarti_activate         (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_scarti();
}


/*********************************************************
************    ALTRE CALLBACKS     **********************
*********************************************************/

extern char rcsid[];
extern char __version__[];
extern char __customer__[];
extern char __copyright__[];
extern char __authors__[];

void do_about(void)
{
	GtkWidget *dlg = create_dlg_about();

	gtk_label_printf(get_widget(dlg,"lb_version"), __version__);
	gtk_label_printf(get_widget(dlg,"lb_customer"), __customer__);
	gtk_label_printf(get_widget(dlg,"lb_rcsid"), rcsid);
	gtk_label_printf(get_widget(dlg,"lb_authors"), __authors__);
	gtk_label_printf(get_widget(dlg,"lb_copyright"), __copyright__);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_widget_show(dlg);
}

void do_exit(void)
{
	GtkWidget *dlg= create_dlg_exit();
	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}

void do_config(void)
{
	GtkWidget *dlg= create_dlg_config();

	EditConfigurazione(dlg);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}

void do_trace(void)
{
	gtk_window_set_focus (GTK_WINDOW (dlg_trace), get_widget(dlg_trace,"pb_close"));
	gtk_window_set_transient_for(GTK_WINDOW(dlg_trace),GTK_WINDOW(main_window));
	gtk_widget_show(dlg_trace);
}

void do_scarti(void)
{
	GtkWidget *dlg = create_dlg_scarti();

	do_refresh_table_scarti(dlg);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_close"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));

	/* globale */
	dlg_scarti=dlg;
}

void do_storico_scarti(void)
{
	GtkWidget *dlg = create_dlg_storico_scarti();
	GtkWidget *lb_date_from = get_widget(dlg,"lb_date_from");
	GtkWidget *lb_date_to = get_widget(dlg,"lb_date_to");


	gtk_label_printf(lb_date_from,"  -  -  ");
	gtk_label_printf(lb_date_to,"  -  -  ");
	strcpy(szFromDate,"");
	strcpy(szToDate,"");

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_close"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));

	dlg_storico_scarti=dlg;
}

/*
* rm : 02-07-2002
* Attenzione : lavora sulla variabile globale szReadBarcode
*/
int do_check_barcode(void)
{
	int nRC=-1;
	PGresult *PGRes;
	char szCDPRO[128];
	char szDSTIT[128];
	char szDSAUT[128];
	char szDSEDT[128];
	char szCDFLG[128];
	char szPREZZ[128];
	int nNMCPE=0;
	GdkColor *pcColor;
	char szFGColor[128];
	char szBuffer[128];

	strcpy(szCDPRO,SubStr(szReadBarcode,5,7));
	strcpy(szDSTIT,"NON PRESENTE");
	strcpy(szDSAUT,"NON PRESENTE");
	strcpy(szDSEDT,"NON PRESENTE");
	strcpy(szCDFLG,"NON PRESENTE");
	strcpy(szPREZZ,"NON PRESENTE");


	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select trim(sccdpro),trim(scdstit),trim(scdsaut),trim(scdsedt),trim(sccdflg),scnmcpe,(scprezz/1000)||','||(scprezz%%1000)/10  from scarti where trim(sccdpro)='%s';",szCDPRO);
	if(PQntuples(PGRes)>1){
		sprintf(szBuffer,"Attenzione : codice [%s] non univoco !",szCDPRO);
		gnome_appbar_set_status((GnomeAppBar *)get_widget(main_window,"ab_main"), szBuffer);

		strcpy(szCDPRO,PQgetvalue(PGRes,0,0));
		strcpy(szDSTIT,PQgetvalue(PGRes,0,1));
		strcpy(szDSAUT,PQgetvalue(PGRes,0,2));
		strcpy(szDSEDT,PQgetvalue(PGRes,0,3));
		strcpy(szCDFLG,PQgetvalue(PGRes,0,4));
		nNMCPE=atoi(PQgetvalue(PGRes,0,5));
		strcpy(szPREZZ,PQgetvalue(PGRes,0,6));
		nRC=0;

		/* trovato : ma non univoco */
	} else if(PQntuples(PGRes)==1){
		strcpy(szCDPRO,PQgetvalue(PGRes,0,0));
		strcpy(szDSTIT,PQgetvalue(PGRes,0,1));
		strcpy(szDSAUT,PQgetvalue(PGRes,0,2));
		strcpy(szDSEDT,PQgetvalue(PGRes,0,3));
		strcpy(szCDFLG,PQgetvalue(PGRes,0,4));
		nNMCPE=atoi(PQgetvalue(PGRes,0,5));
		strcpy(szPREZZ,PQgetvalue(PGRes,0,6));
		nRC=0;

		/* trovato : univoco */
	}
	DBclear(PGRes);

	if(dlg_scarti){
		nRC=-2;
	}

	/* segnalazione a video */
	switch(nRC){
		default: /* TROVATO */
			PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sfcolor from scarti_flags where sfcdflg='%s';",StrTrimAll(szCDFLG));
			if(PQntuples(PGRes)){
				pcColor=GetColorValue(Colors,GetColorIndex(Colors,PQgetvalue(PGRes,0,0)));
				strcpy(szFGColor,"BLACK");
			} else {
				pcColor=&cGREY;
				strcpy(szFGColor,"RED");
			}
			DBclear(PGRes);

			/*
			ChangeBackgroundColor(get_widget(main_window,"eb_barcode"),pcColor);
			ChangeTextColor(get_widget(main_window,"lb_barcode"),&cBLACK);
			gtk_label_set_text(GTK_LABEL(get_widget(main_window,"lb_barcode")),szReadBarcode);
			*/
			ChangeBackgroundColor(get_widget(main_window,"eb_barcode"),pcColor);




			/* uso SPAN ... EVVAI !!! */
			gtk_label_printf(get_widget(main_window,"lb_barcode"),"<markup><b><span foreground='%s' size='80000'>%s</span><span size='40000' foreground='black'>\nCopie:</span><span size='40000' foreground='red'>%4d</span><span size='60000' foreground='orange'>\nEuro %s</span></b></markup>",szFGColor,SubStr(szReadBarcode,5,7),nNumCopie,szPREZZ);

			sprintf(szBuffer,"[%s - %s - %s %d %s]", SubStr(szReadBarcode,5,7), szDSTIT, szCDFLG,nNumCopie,nNumCopie>1?"copie":"copia");
			gnome_appbar_set_status((GnomeAppBar *)get_widget(main_window,"ab_main"), szBuffer);

			gtk_label_printf(get_widget(main_window,"lb_status_barcode"),"[%s]", szReadBarcode);
			gtk_label_printf(get_widget(main_window,"lb_status_descrizione"),"<markup><b><big>[%s]</big></b></markup>", szDSTIT);
			gtk_label_printf(get_widget(main_window,"lb_status_flag"),"<markup><b><big>[%s]</big></b></markup>", szCDFLG);
			gtk_label_printf(get_widget(main_window,"lb_num_copie"),"<markup><b><big>[%d]</big></b></markup>", nNumCopie);
		break;

		case -1:
			/* non trovato */
			/*
			ChangeBackgroundColor(get_widget(main_window,"eb_barcode"),&cLYELLOW);
			ChangeTextColor(get_widget(main_window,"lb_barcode"),&cRED);
			gtk_label_set_text(GTK_LABEL(get_widget(main_window,"lb_barcode")),szReadBarcode);
			*/
			ChangeBackgroundColor(get_widget(main_window,"eb_barcode"),&cYELLOW);
			strcpy(szFGColor,"RED");

			gtk_label_printf(get_widget(main_window,"lb_barcode"),"<markup><b><span foreground='%s' size='80000'>%s</span><span size='40000' foreground='black'>\nCopie:</span><span size='40000' foreground='red'>%4d</span><span size='60000' foreground='orange'>\n%s</span></b></markup>",szFGColor,SubStr(szReadBarcode,5,7),nNumCopie,szPREZZ);

			sprintf(szBuffer,"[%s - %s - %s %d %s]", SubStr(szReadBarcode,5,7), szDSTIT, szCDFLG,nNumCopie,nNumCopie>1?"copie":"copia");
			gnome_appbar_set_status((GnomeAppBar *)get_widget(main_window,"ab_main"), szBuffer);

			gtk_label_printf(get_widget(main_window,"lb_status_barcode"),"[%s]", szReadBarcode);
			gtk_label_printf(get_widget(main_window,"lb_status_descrizione"),"<markup><b><big>[%s]</big></b></markup>", szDSTIT);
			gtk_label_printf(get_widget(main_window,"lb_status_flag"),"<markup><b><big>[%s]</big></b></markup>", szCDFLG);
			gtk_label_printf(get_widget(main_window,"lb_num_copie"),"<markup><b><big>[%d]</big></b></markup>", nNumCopie);
		break;
		case -2:
		{
			/* caso di lettura durante l'edit della tabella scarti */
			GtkCList *lst=GTK_CLIST(get_widget(dlg_scarti,"lst"));
			int nRowIndex;

			/* inserimento della lettura nel field del dlg scarti */
			gtk_entry_set_text(GTK_ENTRY(get_widget(dlg_scarti,"entry_cdpro")),szCDPRO);
			/* 
			* ricerco ed eventualmente seleziono la riga contenente il codice inserito
			*/
			if((nRowIndex=gtk_clist_search_key(szCDPRO,lst,0,0,FALSE))>=0){
				gtk_clist_move_to_row(lst,nRowIndex);
			}
		}
		break;
	}
	if(nRC>=-1){
		/* inserimento della lettura in storico */
		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"insert into storico_scarti (sscdpro,sscdflg,ssprgid,sstmread,ssnmcpe) values('%s','%s',%d,'now',%d);",szCDPRO,szCDFLG,Cfg.nProgramID,nNumCopie);
		if (PQresultStatus(PGRes) != PGRES_COMMAND_OK ) {
			trace_debug(&cRED,txt_trace,PQresultErrorMessage(PGRes));
		}
		DBclear(PGRes);
	}
	nNumCopie=1;
	// gtk_label_printf(get_widget(main_window,"lb_num_copie"),"<markup><b><big>[%d]</big></b></markup>", nNumCopie);
	return nRC;
}

GtkWidget *dlg_message;

void do_message(GtkWidget *parent,char *szTitle,char *szMessage,GtkSignalFunc signal)
{
	GtkWidget *lb_msg;
	GtkWidget *pb_ok;
	/*
	* nessuna distribuzione della lista e' stata selezionata
	*/
	dlg_message = create_dlg_message();

	lb_msg=get_widget(dlg_message,"lb_msg");
	pb_ok=get_widget(dlg_message,"pb_ok");

	gtk_label_printf(lb_msg,szMessage);
	gtk_window_set_title (GTK_WINDOW (dlg_message), szTitle);
	gtk_window_set_focus (GTK_WINDOW (dlg_message), pb_ok);
	gtk_signal_connect (GTK_OBJECT (pb_ok), "clicked", signal, parent);
	gtk_widget_show(dlg_message);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_message),GTK_WINDOW(parent));
}

void do_reindex(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;

	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");
	GtkWidget *lb_msg=get_widget(dlg,"lb_msg");

	gtk_label_printf(lb_msg,"Riorganizzazione indici in corso\nAttendere");
	gtk_update();
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"vacuum analyze scarti;");
	DBclear(PGRes);
	gtk_label_printf(lb_msg,"Riorganizzazione indici terminata\nPremere Ok per uscire");
	gtk_update();
	gtk_signal_disconnect_by_func (GTK_OBJECT (pb_ok), GTK_SIGNAL_FUNC(do_reindex), dlg);
	gtk_signal_connect (GTK_OBJECT (pb_ok), "clicked", GTK_SIGNAL_FUNC(on_button_pressed_destroy), dlg);
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


void on_pbm_storico_scarti_activate         (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_storico_scarti();
}

void on_pbm_barcode_activate                (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_barcode();
}

void on_pbm_tabella_flags_activate          (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_flags();
}

void on_pbm_cancella_tabella_scarti_activate (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_message(main_window,"Cancella Tabella Scarti","ATTENZIONE!\nQuesta operazione cancella la tabella scarti attualmente in uso - procedere ?",GTK_SIGNAL_FUNC(do_delete_tabella_scarti));
}


void on_pbm_cancella_storico_scarti_activate (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_message(main_window,"Cancella Storico Scarti","ATTENZIONE!\nQuesta operazione cancella la tabella dello storico scarti attualmente in uso - procedere ?",GTK_SIGNAL_FUNC(do_delete_tabella_storico_scarti));
}

void on_pbm_import_dati_activate            (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_import();
}


void on_pbm_export_dati_activate            (GtkMenuItem     *menuitem, gpointer         user_data)
{
	do_export();
}



void on_dlg_scarti_pb_delete_clicked           (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	PGresult *PGRes;
	BOOL bOK=TRUE;


	strcpy(szSelectedCDPRO,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdpro"))));StrTrimAll(szSelectedCDPRO);
	strcpy(szSelectedDSAUT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsaut"))));StrTrimAll(szSelectedDSAUT);
	strcpy(szSelectedDSTIT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dstit"))));StrTrimAll(szSelectedDSTIT);
	strcpy(szSelectedDSEDT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsedt"))));StrTrimAll(szSelectedDSEDT);
	strcpy(szSelectedCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szSelectedCDFLG);
	nSelectedNMCPE = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_nmcpe")));
	nSelectedPREZZ = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prezz")));

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sccdpro from scarti where trim(sccdpro)='%s';",szSelectedCDPRO);
	if(PQntuples(PGRes)==0){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non presente !",szSelectedCDPRO);
		bOK=FALSE;
	} else if(PQntuples(PGRes)>1){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non univoco !",szSelectedCDPRO);
		bOK=FALSE;
	}
	DBclear(PGRes);
	if(bOK){
		do_message(dlg,"Cancella Record","Premere Ok per eliminare il record",GTK_SIGNAL_FUNC(do_delete_barcode));
	}
}


void on_dlg_scarti_pb_modify_clicked        (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	PGresult *PGRes;
	BOOL bOK=TRUE;


	strcpy(szSelectedCDPRO,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdpro"))));StrTrimAll(szSelectedCDPRO);
	strcpy(szSelectedDSAUT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsaut"))));StrTrimAll(szSelectedDSAUT);
	strcpy(szSelectedDSTIT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dstit"))));StrTrimAll(szSelectedDSTIT);
	strcpy(szSelectedDSEDT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsedt"))));StrTrimAll(szSelectedDSEDT);
	strcpy(szSelectedCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szSelectedCDFLG);
	nSelectedNMCPE = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_nmcpe")));
	nSelectedPREZZ = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prezz")));

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sccdpro from scarti where trim(sccdpro)='%s';",szSelectedCDPRO);
	if(PQntuples(PGRes)==0){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non presente !",szSelectedCDPRO);
		bOK=FALSE;
	} else if(PQntuples(PGRes)>1){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] non univoco !",szSelectedCDPRO);
		bOK=FALSE;
	}
	DBclear(PGRes);
	if(bOK){
		do_message(dlg,"Modifica Record","Premere Ok per modificare il record",GTK_SIGNAL_FUNC(do_modify_barcode));
	}
}


void on_dlg_scarti_pb_add_clicked        (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	PGresult *PGRes;
	BOOL bOK=TRUE;


	strcpy(szSelectedCDPRO,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdpro"))));StrTrimAll(szSelectedCDPRO);
	strcpy(szSelectedDSAUT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsaut"))));StrTrimAll(szSelectedDSAUT);
	strcpy(szSelectedDSTIT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dstit"))));StrTrimAll(szSelectedDSTIT);
	strcpy(szSelectedDSEDT,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_dsedt"))));StrTrimAll(szSelectedDSEDT);
	strcpy(szSelectedCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szSelectedCDFLG);
	nSelectedNMCPE = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_nmcpe")));
	nSelectedPREZZ = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_prezz")));

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sccdpro from scarti where trim(sccdpro)='%s';",szSelectedCDPRO);
	if(PQntuples(PGRes)!=0){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Attenzione : codice [%s] gi� presente !",szSelectedCDPRO);
		bOK=FALSE;
	}
	DBclear(PGRes);
	if(bOK){
		do_message(dlg,"Inserimento Record","Premere Ok per inserire il record",GTK_SIGNAL_FUNC(do_add_barcode));
	}
}

void do_delete_barcode(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;
	BOOL bOK=TRUE;

	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");
	GtkWidget *lb_msg=get_widget(dlg_scarti,"lb_msg");

	gtk_label_printf(lb_msg,"Eliminazione record [%s]",szSelectedCDPRO);
	gtk_update();
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"delete from scarti where trim(sccdpro)='%s';",szSelectedCDPRO);
	if(PQresultStatus(PGRes)!=PGRES_COMMAND_OK || (atoi(PQcmdTuples(PGRes))!=1)){
		bOK=FALSE;
		trace_debug(&cRED,txt_trace,PQresultErrorMessage(PGRes));
	}
	DBclear(PGRes);
	if(bOK){
		gtk_label_printf(lb_msg,"Record [%s] eliminato",szSelectedCDPRO);
	} else {
		gtk_label_printf(lb_msg,"Errore in eliminazione Record [%s]",szSelectedCDPRO);
	}
	gtk_update();

	do_refresh_table_scarti(dlg_scarti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pb_ok), GTK_SIGNAL_FUNC(do_delete_barcode), dlg);
	gtk_signal_connect (GTK_OBJECT (pb_ok), "clicked", GTK_SIGNAL_FUNC(on_button_pressed_destroy), dlg);
}

void do_add_barcode(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;
	BOOL bOK=TRUE;

	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");
	GtkWidget *lb_msg=get_widget(dlg_scarti,"lb_msg");

	gtk_label_printf(lb_msg,"Inserimento record [%s]",szSelectedCDPRO);
	gtk_update();
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"insert into scarti (sccdpro,scdstit,scdsaut,scdsedt,sccdflg,scnmcpe,scprezz) values('%s','%s','%s','%s',%s,%d,%d);",
		szSelectedCDPRO,
		szSelectedDSTIT,
		szSelectedDSAUT,
		szSelectedDSEDT,
		szSelectedCDFLG,
		nSelectedNMCPE,
		nSelectedPREZZ);
	if (PQresultStatus(PGRes) != PGRES_COMMAND_OK ) {
		bOK=FALSE;
		trace_debug(&cRED,txt_trace,PQresultErrorMessage(PGRes));
	}
	DBclear(PGRes);
	if(bOK){
		gtk_label_printf(lb_msg,"Record [%s] inserito",szSelectedCDPRO);
	} else {
		gtk_label_printf(lb_msg,"ERRORE in inserimento record [%s]",szSelectedCDPRO);
	}
	gtk_update();

	do_refresh_table_scarti(dlg_scarti);

	gtk_signal_disconnect_by_func (GTK_OBJECT (pb_ok), GTK_SIGNAL_FUNC(do_add_barcode), dlg);
	gtk_signal_connect (GTK_OBJECT (pb_ok), "clicked", GTK_SIGNAL_FUNC(on_button_pressed_destroy), dlg);
}

void do_modify_barcode(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;
	BOOL bOK=TRUE;

	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");
	GtkWidget *lb_msg=get_widget(dlg_scarti,"lb_msg");

	gtk_label_printf(lb_msg,"Modifica record [%s]",szSelectedCDPRO);
	gtk_update();
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update scarti set scdstit='%s',scdsaut='%s',scdsedt='%s',sccdflg='%s',scnmcpe=%d,scprezz=%d where trim(sccdpro)='%s';",
		szSelectedDSTIT,
		szSelectedDSAUT,
		szSelectedDSEDT,
		szSelectedCDFLG,
		nSelectedNMCPE,
		nSelectedPREZZ,
		szSelectedCDPRO);
	if(PQresultStatus(PGRes)!=PGRES_COMMAND_OK || (atoi(PQcmdTuples(PGRes))!=1)){
		bOK=FALSE;
		trace_debug(&cRED,txt_trace,PQresultErrorMessage(PGRes));
	}
	DBclear(PGRes);

	if(bOK){
		gtk_label_printf(lb_msg,"Record [%s] modificato",szSelectedCDPRO);
	} else {
		gtk_label_printf(lb_msg,"<b><span foreground='red'>ERRORE in modifica record [%s]</span></b>",szSelectedCDPRO);
	}
	gtk_update();

	do_refresh_table_scarti(dlg_scarti);

	
	gtk_signal_disconnect_by_func (GTK_OBJECT (pb_ok), GTK_SIGNAL_FUNC(do_add_barcode), dlg);
	gtk_signal_connect (GTK_OBJECT (pb_ok), "clicked", GTK_SIGNAL_FUNC(on_button_pressed_destroy), dlg);
}

void on_dlg_scarti_pb_close_clicked(gpointer user_data, GtkButton *button)
{
	gtk_widget_destroy(GTK_WIDGET(user_data));
	dlg_scarti=0;
}


gboolean dlg_scarti_do_close_event(GtkWidget       *widget, GdkEvent        *event, gpointer         user_data)
{
  return TRUE;
}


void on_dlg_scarti_pb_print_clicked(gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Stampa Tabella Scarti","Premere Ok per stampare la tabella scarti",GTK_SIGNAL_FUNC(do_print_scarti));
}

void on_dlg_scarti_entry_barcode_activate   (GtkEditable     *editable, gpointer         user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	strcpy(szReadBarcode,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdpro"))));StrTrimAll(szReadBarcode);
	if(strlen(szReadBarcode)!=13){
		/* aggiungo un tot di zeri all'inizio ... */
		PadZero(szReadBarcode,12);
	}
	if(!do_check_barcode()){
		/* trovato */
		gtk_widget_destroy (dlg);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"<span foreground='red'><b>Prodotto [%s] NON TROVATO</b></span>",SubStr(szReadBarcode,5,7));
	}
}

void do_print_scarti(GtkButton *button, gpointer user_data)
{
	PrintTable("scarti","scarti",Cfg.szPrinter,NULL);
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}

void on_dlg_storico_scarti_pb_print_clicked (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Stampa Statistiche Scarti","Premere Ok per stampare le statistiche scarti",GTK_SIGNAL_FUNC(do_print_storico_scarti));
}

void do_print_storico_scarti(GtkButton *button, gpointer user_data)
{
	char szKey[128];

	if(strlen(szFromDate) && strlen(szToDate)){
		sprintf(szKey,"data_YYYYMMDD(sstmread) >= '%s' and data_YYYYMMDD(sstmread) <='%s'",szFromDate,szToDate);
	} else {
		strcpy(szKey,"true");
	}
	PrintTable("scarti","storico_scarti",Cfg.szPrinter,szKey);
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}


void on_dlg_storico_scarti_pb_select_date_range_clicked (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=create_dlg_select_date_range();
	int nAnno,nMese,nGiorno;

	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));

	gtk_calendar_get_date(GTK_CALENDAR(get_widget(dlg,"cal_from_date")),&nAnno,&nMese,&nGiorno);
	gtk_label_printf(get_widget(dlg,"lb_from_date"),"%2d-%2d-%2d",nGiorno,nMese+1,nAnno);

	gtk_calendar_get_date(GTK_CALENDAR(get_widget(dlg,"cal_to_date")),&nAnno,&nMese,&nGiorno);
	gtk_label_printf(get_widget(dlg,"lb_to_date"),"%2d-%2d-%2d",nGiorno,nMese+1,nAnno);
}

void on_dlg_storico_scarti_pb_refresh_clicked (gpointer user_data, GtkButton *button)
{
	char szKey[128];

	if(strlen(szFromDate) && strlen(szToDate)){
		sprintf(szKey,"data_YYYYMMDD(sstmread) >= '%s' and data_YYYYMMDD(sstmread) <='%s'",szFromDate,szToDate);
	} else {
		strcpy(szKey,"true");
	}
	RefreshTable(dlg_storico_scarti,"sw_list","lst","lb_titolo","scarti","storico_scarti",GTK_SELECTION_SINGLE,szKey, NULL, NULL);
}


void on_dlg_select_date_range_cal_from_date_day_selected (GtkCalendar     *calendar, gpointer         user_data)
{
	GtkWidget *dlg = GTK_WIDGET(user_data);
	int nAnno,nMese,nGiorno;

	gtk_calendar_get_date(calendar,&nAnno,&nMese,&nGiorno);
	gtk_label_printf(get_widget(dlg,"lb_from_date"),"%2d-%2d-%2d",nGiorno,nMese+1,nAnno);

}


void on_dlg_select_date_range_cal_to_date_day_selected (GtkCalendar     *calendar, gpointer         user_data)
{
	GtkWidget *dlg = GTK_WIDGET(user_data);
	int nAnno,nMese,nGiorno;

	gtk_calendar_get_date(calendar,&nAnno,&nMese,&nGiorno);
	gtk_label_printf(get_widget(dlg,"lb_to_date"),"%2d-%2d-%2d",nGiorno,nMese+1,nAnno);

}


void on_dlg_select_date_range_pb_ok_clicked (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg = GTK_WIDGET(user_data);
	GtkWidget *lb_date_from = get_widget(dlg_storico_scarti,"lb_date_from");
	GtkWidget *lb_date_to = get_widget(dlg_storico_scarti,"lb_date_to");
	int nAnno,nMese,nGiorno;

	gtk_calendar_get_date(GTK_CALENDAR(get_widget(dlg,"cal_from_date")),&nAnno,&nMese,&nGiorno);
	gtk_label_printf(lb_date_from,"%2d-%2d-%4d",nGiorno,nMese+1,nAnno);
	sprintf(szFromDate,"%04d%02d%02d",nAnno,nMese+1,nGiorno);

	gtk_calendar_get_date(GTK_CALENDAR(get_widget(dlg,"cal_to_date")),&nAnno,&nMese,&nGiorno);
	gtk_label_printf(lb_date_to,"%2d-%2d-%4d",nGiorno,nMese+1,nAnno);
	sprintf(szToDate,"%04d%02d%02d",nAnno,nMese+1,nGiorno);
	gtk_widget_destroy(GTK_WIDGET(user_data));
}




void on_dlg_config_cb_postscript_toggled(gpointer user_data, GtkToggleButton *togglebutton)
{
	GtkWidget *dlg = GTK_WIDGET(user_data);

	if(gtk_toggle_button_get_active(togglebutton)){
		gtk_widget_set_sensitive(get_widget(dlg,"rb_a2ps"),TRUE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_html2ps"),TRUE);
	} else {
		gtk_widget_set_sensitive(get_widget(dlg,"rb_a2ps"),FALSE);
		gtk_widget_set_sensitive(get_widget(dlg,"rb_html2ps"),FALSE);
	}

}

void do_refresh_table_scarti(GtkWidget *dlg)
{
	RefreshTable(dlg,"sw_list","lst","lb_titolo","scarti","scarti",GTK_SELECTION_SINGLE,NULL, NULL, NULL);

	gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "select_row", GTK_SIGNAL_FUNC (on_lst_scarti_select_row), dlg);
	gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "unselect_row", GTK_SIGNAL_FUNC (on_lst_scarti_unselect_row), dlg);

	gtk_widget_set_sensitive(get_widget(dlg,"pb_modify"),FALSE);
	gtk_widget_set_sensitive(get_widget(dlg,"pb_delete"),FALSE);
}



void do_barcode(void)
{
	GtkWidget *dlg= create_dlg_barcode();
	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"entry_barcode"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}

void do_num_copie(void)
{
	GtkWidget *dlg= create_dlg_num_copie();
	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"sb_num_copie"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}

void on_dlg_num_copie_pb_ok_clicked           (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	nNumCopie = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(get_widget(dlg,"sb_num_copie")));

	gtk_label_printf(get_widget(main_window,"lb_num_copie"),"<markup><b><big>[%d]</big></b></markup>", nNumCopie);

	gtk_widget_destroy(dlg);
}


void on_dlg_barcode_pb_ok_clicked           (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	strcpy(szReadBarcode,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_barcode"))));StrTrimAll(szReadBarcode);
	if(strlen(szReadBarcode)!=13){
		/* aggiungo un tot di zeri all'inizio ... */
		PadZero(szReadBarcode,12);
	}
	if(!do_check_barcode()){
		/* trovato */
		gtk_widget_destroy (dlg);
	} else {
		gtk_widget_destroy (dlg);
		//gtk_label_printf(get_widget(dlg,"lb_msg"),"<span foreground='red'><b>Prodotto [%s] NON TROVATO</b></span>",SubStr(szReadBarcode,5,7));
	}
}

void do_flags(void)
{
	GtkWidget *dlg= create_dlg_flags();
	GtkWidget *combo_colore;
	GList *items = NULL;
	int nIndex=0;
	PGresult *PGRes;
	PGresult *PGResFlag;
	PGresult *PGResInsert;

	gtk_widget_show(dlg);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sccdflg from scarti group by sccdflg;");
	for(nIndex=0;nIndex<PQntuples(PGRes);nIndex++){
		PGResFlag=PGExecSQL(Cfg.nDebugVersion>1,"select sfcdflg from scarti_flags where sfcdflg='%s';",PQgetvalue(PGRes,nIndex,0));
		if(!PQntuples(PGResFlag)){
			PGResInsert=PGExecSQL(Cfg.nDebugVersion>1,"insert into  scarti_flags (sfcdflg,sfcolor) values ('%s','%s');",PQgetvalue(PGRes,nIndex,0),"PINK");
			DBclear(PGResInsert);
		}
		DBclear(PGResFlag);
	}
	DBclear(PGRes);

	RefreshTable(dlg,"sw_list","lst",NULL,"scarti","lista_flags",GTK_SELECTION_SINGLE,NULL, NULL, NULL);
	gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "select_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_select_row), dlg);
	gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "unselect_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_unselect_row), dlg);

	combo_colore=get_widget(dlg,"combo_colore");

	while(strlen(Colors[nIndex].szColorName)){
		items = g_list_append (items, Colors[nIndex].szColorName);
		nIndex++;
	}

	gtk_combo_set_popdown_strings (GTK_COMBO (combo_colore), items);

	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}


void on_dlg_flags_pb_add_clicked            (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Aggiungi flag di scarto","Premere Ok per aggiungere il flag di scarto",GTK_SIGNAL_FUNC(do_add_scarti_flags));
}


void on_dlg_flags_pb_modify_clicked         (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Modifica flag di scarto","Premere Ok per modificare il flag di scarto",GTK_SIGNAL_FUNC(do_mod_scarti_flags));
}


void on_dlg_flags_pb_delete_clicked         (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Cancella flag di scarto","Premere Ok per cancellare il flag di scarto",GTK_SIGNAL_FUNC(do_del_scarti_flags));
}


void on_dlg_flags_pb_print_clicked          (gpointer user_data, GtkButton *button)
{
	do_message(main_window,"Stampa Tabella Flags Scarti","Premere Ok per stampare la tabella dei flags",GTK_SIGNAL_FUNC(do_print_scarti_flags));
}

void do_print_scarti_flags(GtkButton *button, gpointer user_data)
{
	PrintTable("scarti","scarti_flags",Cfg.szPrinter,NULL);
	gtk_widget_destroy(GTK_WIDGET(user_data));
}

void do_add_scarti_flags(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	char szCDFLG[128];
	char szColorName[128];
	PGresult *PGRes;

	strcpy(szCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szCDFLG);
	strcpy(szColorName,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry)));
	StrTrimAll(szColorName);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"insert into scarti_flags (sfcdflg,sfcolor) values ('%s','%s');",szCDFLG,szColorName);
	if(PQcmdTuples(PGRes)){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Inserimento flag avvenuto correttamente");

		RefreshTable(dlg,"sw_list","lst",NULL,"scarti","lista_flags",GTK_SELECTION_SINGLE,NULL, NULL, NULL);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "select_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_select_row), dlg);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "unselect_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_unselect_row), dlg);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Errore in inserimento flag !");
	}
	DBclear(PGRes);


	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}

void do_mod_scarti_flags(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	char szCDFLG[128];
	char szColorName[128];
	PGresult *PGRes;

	strcpy(szCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szCDFLG);
	strcpy(szColorName,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry)));
	StrTrimAll(szColorName);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update scarti_flags set sfcolor='%s' where sfcdflg='%s';",szColorName,szCDFLG);
	if(PQcmdTuples(PGRes)){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Aggiornamento flag avvenuto correttamente");

		RefreshTable(dlg,"sw_list","lst",NULL,"scarti","lista_flags",GTK_SELECTION_SINGLE,NULL, NULL, NULL);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "select_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_select_row), dlg);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "unselect_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_unselect_row), dlg);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Errore in aggiornamento flag !");
	}
	DBclear(PGRes);

	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}

void do_del_scarti_flags(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	char szCDFLG[128];
	char szColorName[128];
	PGresult *PGRes;

	strcpy(szCDFLG,gtk_entry_get_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg"))));StrTrimAll(szCDFLG);
	strcpy(szColorName,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry)));
	StrTrimAll(szColorName);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"delete from scarti_flags where sfcdflg='%s';",szCDFLG);
	if(PQcmdTuples(PGRes)){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Cancellazione flag avvenuto correttamente");

		RefreshTable(dlg,"sw_list","lst",NULL,"scarti","lista_flags",GTK_SELECTION_SINGLE,NULL, NULL, NULL);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "select_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_select_row), dlg);
		gtk_signal_connect (GTK_OBJECT(get_widget(dlg,"lst")), "unselect_row", GTK_SIGNAL_FUNC (on_lst_scarti_flags_unselect_row), dlg);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Errore in cancellazione flag !");
	}
	DBclear(PGRes);

	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}


void on_dlg_flags_pb_close_clicked          (gpointer user_data, GtkButton *button)
{
	gtk_widget_destroy(GTK_WIDGET(user_data));
}


gboolean dlg_flags_do_close_event               (GtkWidget       *widget, GdkEvent        *event, gpointer         user_data)
{
  return TRUE;
}


void on_combo_colore_changed                (GtkEditable     *editable, gpointer         user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	char szColorName[128];
	int nColorIndex;

	strcpy(szColorName,gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry)));StrTrimAll(szColorName);

	nColorIndex=GetColorIndex(Colors,szColorName);
	if(nColorIndex!=-1){
		// gtk_widget_set_background(get_widget(dlg,"eb_colore"),GetColorValue(Colors,nColorIndex));
		ChangeBackgroundColor(get_widget(dlg,"eb_colore"),GetColorValue(Colors,nColorIndex));
	}
}

void on_lst_scarti_flags_select_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	char *pszString;
	char szCDFLG[128];
	PGresult *PGRes;
	int nColorIndex;

	/* in pos 0 c'e' la chiave  ... */
	gtk_clist_get_text(clist,row,0,&pszString);
	strcpy(szCDFLG,pszString);
	StrTrimAll(szCDFLG);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"select sfcdflg,sfcolor from scarti_flags where sfcdflg='%s';",szCDFLG);
	if(PQntuples(PGRes)){
		gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg")),PQgetvalue(PGRes,0,0));
		gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry), PQgetvalue(PGRes,0,1));

		nColorIndex=GetColorIndex(Colors,PQgetvalue(PGRes,0,1));
		if(nColorIndex!=-1){
			// gtk_widget_set_background(get_widget(dlg,"eb_colore"),GetColorValue(Colors,nColorIndex));
			ChangeBackgroundColor(get_widget(dlg,"eb_colore"),GetColorValue(Colors,nColorIndex));
		}

	}
	DBclear(PGRes);
}

void on_lst_scarti_flags_unselect_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	gtk_entry_set_text(GTK_ENTRY(get_widget(dlg,"entry_cdflg")),"");
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(get_widget(dlg,"combo_colore"))->entry), "");
	// gtk_widget_set_background(get_widget(dlg,"eb_colore"),&cGREY);
	ChangeBackgroundColor(get_widget(dlg,"eb_colore"),&cGREY);
}


void do_delete_tabella_storico_scarti(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;

	GetDateYYYYMMDD(time((long *)0),szDateBuffer), 
	GetTimeHHMMSS(time((long *)0),szTimeBuffer);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"drop table storico_scarti_%s_%s;",szDateBuffer,szTimeBuffer);
	DBclear(PGRes);
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"create table storico_scarti_%s_%s as select * from storico_scarti;",szDateBuffer,szTimeBuffer);
	DBclear(PGRes);
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"truncate table storico_scarti;");
	DBclear(PGRes);
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}

void do_delete_tabella_scarti(GtkButton *button, gpointer user_data)
{
	PGresult *PGRes;

	GetDateYYYYMMDD(time((long *)0),szDateBuffer), 
	GetTimeHHMMSS(time((long *)0),szTimeBuffer);

	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"drop table scarti_%s_%s;",szDateBuffer,szTimeBuffer);
	DBclear(PGRes);
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"create table scarti_%s_%s as select * from scarti;",szDateBuffer,szTimeBuffer);
	DBclear(PGRes);
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,"truncate table scarti;");
	DBclear(PGRes);
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}

gboolean dlg_work_do_close_event                (GtkWidget       *widget, GdkEvent        *event, gpointer         user_data)
{
  return TRUE;
}




void on_dlg_work_pb_close_clicked           (gpointer user_data, GtkButton *button)
{
	gtk_widget_destroy(GTK_WIDGET(user_data));
}



void do_import(void)
{
	GtkWidget *dlg=create_dlg_work();
	GList *items = NULL;
	int nIndex=0;

	gnome_file_entry_set_title((GnomeFileEntry *)get_widget(dlg,"file_entry"), "Import Dati");
	gnome_file_entry_set_default_path((GnomeFileEntry *)get_widget(dlg,"file_entry"), Cfg.szPathExport);

	gtk_window_set_title (GTK_WINDOW (dlg), "Import Dati");
	gtk_widget_show(dlg);

	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_ok")), "clicked", GTK_SIGNAL_FUNC(do_confirm_import_dati), dlg);
}

void do_export(void)
{
	GtkWidget *dlg=create_dlg_work();
	GtkWidget *entry_file;
	GList *items = NULL;
	int nIndex=0;

	gnome_file_entry_set_title((GnomeFileEntry *)get_widget(dlg,"file_entry"), "Export Dati");
	gnome_file_entry_set_default_path((GnomeFileEntry *)get_widget(dlg,"file_entry"), Cfg.szPathExport);

	gtk_window_set_title (GTK_WINDOW (dlg), "Export Dati");
	gtk_widget_show(dlg);

	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_signal_connect (GTK_OBJECT (get_widget(dlg,"pb_ok")), "clicked", GTK_SIGNAL_FUNC(do_confirm_export_dati), dlg);
}

void on_combo_file_entry_changed            (GtkEditable     *editable, gpointer         user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *entry_file;

	entry_file=gnome_file_entry_gtk_entry((GnomeFileEntry *)get_widget(dlg,"file_entry"));
	
	gtk_label_printf(get_widget(dlg,"lb_msg"),"Selezionato file : %s",gtk_entry_get_text(GTK_ENTRY(entry_file)));
}

void on_rb_posizioni_fisse_toggled          (gpointer user_data, GtkToggleButton *togglebutton)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	gtk_widget_set_sensitive(get_widget(dlg,"combo_delimitatore"),!gtk_toggle_button_get_active(togglebutton));
}


void on_rb_campi_delimitati_toggled         (gpointer user_data, GtkToggleButton *togglebutton)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	gtk_widget_set_sensitive(get_widget(dlg,"combo_delimitatore"),gtk_toggle_button_get_active(togglebutton));
}

void do_confirm_import_dati(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Import Dati","Confermi l'import dei dati ?",GTK_SIGNAL_FUNC(do_import_dati));
}

void do_confirm_export_dati(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);

	do_message(dlg,"Import Dati","Confermi l'export dei dati ?",GTK_SIGNAL_FUNC(do_export_dati));
}

void do_export_dati(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *entry_file;
	GtkWidget *entry_delimitatore;
	BOOL bCampiDelimitati;
	BOOL bErrore=FALSE;
	char cDelimitatore;
	char szBuffer[256];
	char szFileName[256];
	char szCommand[256];

	entry_file=gnome_file_entry_gtk_entry((GnomeFileEntry *)get_widget(dlg,"file_entry"));
	strcpy(szFileName,gtk_entry_get_text(GTK_ENTRY(entry_file)));

	trace_debug(&cGREEN,txt_trace,"Export dati - Eseguo comando '%s'",szCommand);

	sprintf(szCommand,"%s/export_dati.sh > %s", Cfg.szPathExe,szFileName);

	bErrore=!system(szCommand);

	if(bErrore){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"ERRORE NELL'EXPORT DEI DATI");
		trace_debug(&cRED,txt_trace,"Errore in esecuzione comando '%s'",szCommand);
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"EXPORT DEI DATI TERMINATO");
		trace_debug(&cRED,txt_trace,"Comando '%s' eseguito correttamente",szCommand);
	}
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}


void do_import_dati(GtkButton *button, gpointer user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *entry_file;
	GtkWidget *entry_delimitatore;
	BOOL bCampiDelimitati;
	BOOL bErrore;
	BOOL bOK=TRUE;
	char cDelimitatore;
	char szBuffer[256];
	char szFileName[256];
	char szCommand[256];

	entry_file=gnome_file_entry_gtk_entry((GnomeFileEntry *)get_widget(dlg,"file_entry"));
	entry_delimitatore=GTK_WIDGET(GTK_COMBO(get_widget(dlg,"combo_delimitatore"))->entry);

	strcpy(szBuffer,gtk_entry_get_text(GTK_ENTRY(entry_delimitatore)));
	strcpy(szFileName,gtk_entry_get_text(GTK_ENTRY(entry_file)));

	cDelimitatore=szBuffer[0];

	bCampiDelimitati=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(get_widget(dlg,"rb_campi_delimitati")));

	if(bCampiDelimitati){
		sprintf(szCommand,"txt2sql -v -c %s/scarti.cfg -i %s -h %s -d %s -t scarti -s '%c'",
			Cfg.szPathData,szFileName,Cfg.szPGHost,Cfg.szPGDataBase,cDelimitatore);
	} else {
		sprintf(szCommand,"txt2sql -v -c %s/scarti.cfg -i %s -h %s -d %s -t scarti",
			Cfg.szPathData,szFileName,Cfg.szPGHost,Cfg.szPGDataBase);
	}

	trace_debug(&cGREEN,txt_trace,"Import dati - Eseguo comando '%s'",szCommand);

	bErrore=!system(szCommand);

	if(bErrore){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"ERRORE NELL'IMPORT DEI DATI");
		trace_debug(&cRED,txt_trace,"Errore in esecuzione comando '%s'",szCommand);
		bOK=FALSE;
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"IMPORT DEI DATI TERMINATO");
		trace_debug(&cRED,txt_trace,"Comando '%s' eseguito correttamente",szCommand);
	}
	if(bOK){
		PGresult *PGRes;

		PGRes=PGExecSQL(Cfg.nDebugVersion>1,"update scarti set sccdpro=lpad(sccdpro,7,'0') where length(sccdpro)<7;");
		DBclear(PGRes);
	}
	gtk_widget_destroy(GTK_WIDGET(dlg_message));
}


