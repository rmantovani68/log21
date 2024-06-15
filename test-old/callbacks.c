/*
* callbacks.c
* Callbacks functions
* Easy Picking 3.0
* Copyright CNI-HSD srl 1999-2002
* Autori : Roberto Mantovani
*          Stefano Tarroni
*/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef TRACE
#	include <trace.h>
#endif

#include <gtk/gtk.h>
#include <libpq-fe.h>
#include <time.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "picking.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include <proc_list.h>
#include <pmx_msq.h>
#include <ca_file.h>
#include <dbfun.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"

BOOL bFineTest=FALSE;


/********************************************************************
********************* Pulsanti di main_window ***********************
********************************************************************/

void on_main_pb_config_clicked (gpointer user_data, GtkButton *button)
{
	EditConfigurazione();
}

void on_main_pb_print_clicked               (gpointer user_data, GtkButton *button)
{
}

void on_main_pb_settori_clicked             (gpointer user_data, GtkButton *button)
{
	do_settori(main_window);
}

void on_main_pb_test_clicked                (gpointer user_data, GtkButton *button)
{
	do_test();

}



void on_main_pb_exit_clicked (gpointer user_data, GtkButton *button)
{
	do_exit();
}

/********************************************************************
**************** Fine Pulsanti di main_window ***********************
********************************************************************/




/********************************************************************
******************************  MENU' *******************************
********************************************************************/

void on_pbm_print_activate                  (GtkMenuItem     *menuitem, gpointer         user_data)
{
}

void on_pbm_aggiorna_activate               (GtkMenuItem     *menuitem, gpointer         user_data)
{
}

void on_pbm_exit_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	do_exit();
}


void on_pbm_about_activate(GtkMenuItem * menuitem, gpointer user_data)
{
	do_about();
}


void on_pbm_help_activate(GtkMenuItem * menuitem, gpointer user_data)
{
}

void on_pbm_configurazione_activate(GtkMenuItem * menuitem, gpointer user_data) 
{
	EditConfigurazione();
}

/********************************************************************
************************** FINE MENU' *******************************
********************************************************************/

void on_mb_main_activate_current (GtkMenuShell *menushell, gboolean force_hide, gpointer user_data)
{
}


void on_dlg_exit_pb_ok_clicked(gpointer user_data, GtkButton *button)
{
	ExitApp();
}

void on_dlg_test_rb_singolo_settore_toggled (GtkToggleButton *togglebutton, gpointer         user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
  GtkWidget *sb_settore= get_widget(dlg,"sb_settore");

	gtk_widget_set_sensitive(sb_settore,gtk_toggle_button_get_active(togglebutton));
}

void on_dlg_test_rb_pulsante_simulato_toggled (GtkToggleButton *togglebutton, gpointer         user_data)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
  GtkWidget *sb_delay= get_widget(dlg,"sb_delay");

	gtk_widget_set_sensitive(sb_delay,gtk_toggle_button_get_active(togglebutton));
}


void on_dlg_test_pb_start_clicked              (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
	GtkWidget *rb_tutti_i_settori= get_widget(dlg,"rb_tutti_i_settori");
  GtkWidget *sb_settore= get_widget(dlg,"sb_settore");
  GtkWidget *sb_delay= get_widget(dlg,"sb_delay");
  GtkWidget *rb_attesa_pulsante= get_widget(dlg,"rb_attesa_pulsante");
  GtkWidget *rb_priorita= get_widget(dlg,"rb_priorita");
  GtkWidget *tb_ciclo_infinito= get_widget(dlg,"tb_ciclo_infinito");
  GtkWidget *pb_start= get_widget(dlg,"pb_start");
  GtkWidget *pb_stop= get_widget(dlg,"pb_stop");
  GtkWidget *pb_cancel= get_widget(dlg,"pb_cancel");
  GtkWidget *pb_reset= get_widget(dlg,"pb_reset");
  GtkWidget *clist= get_widget(dlg,"lst");

	char *RecordData[1][10];
	int nSettore;
	int nDelay;
	char szFila[128];
	BOOL bLoop;
	BOOL bSimulazione;
	BOOL bPriorita;
	BOOL bTastoOK=FALSE;
	char szBuffer[80];
	char szFilaUbicazione[80];
	PGresult *PGRes = NULL;
	int nSettoreIndex;
	int nDisplayIndex;
	int nIOS;
	int nCPU;
	int nModulo;
	int nRow;
	int nCol;
	PDISPLAYSTRUCT pDisplay;
	int nIndex;
	int nTuples;
	int nAllMsg = 0;
	int nCarIn;
	char szInMsg[INFO_LEN];
	char szText[INFO_LEN];
	MSGHDRINT InMsgStruct;
	GdkPixmap *pixmap;
	GdkBitmap *mask;

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_tutti_i_settori))){
		nSettore=0;
	} else {
		nSettore= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sb_settore));
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_attesa_pulsante))){
		bSimulazione=FALSE;
	} else {
		bSimulazione=TRUE;
		nDelay= gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(sb_delay));
	}

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_priorita))){
		bPriorita=TRUE;
	} else {
		bPriorita=FALSE;
	}

	bLoop=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tb_ciclo_infinito));

	/* la fila non viene gestita */
	szFila[0]='\0';

	RecordData[0][0]=malloc(100);
	RecordData[0][1]=malloc(100);
	RecordData[0][2]=malloc(100);
	/* 
	* Inizio test
	*/
	gtk_widget_set_sensitive(pb_cancel,FALSE);
	gtk_widget_set_sensitive(pb_start,FALSE);
	gtk_widget_set_sensitive(pb_stop,TRUE);
	gtk_widget_set_sensitive(pb_reset,FALSE);
	bFineTest=FALSE;

	gtk_label_printf(get_widget(dlg,"lb_msg"),"Iniziato test impianto");

	do {
		ResetLuci();
		ResetSettori();
		ScongelaPKL();

		/*
		* Selezione di tutte le ubicazioni dell'impianto
		*/
		PGRes=PGExecSQL(TRUE,"select ubicazione, codprod, isola, settore, display, ios, cpu, modulo, riga, colonna, priorita, cnistato, fila(ubicazione) from ubicazioni where ubitipo = '%s' and ubcdflg='%c' order by isola,%s;",
			Cfg.szTipoUbicazioni,
			UBICAZIONE_AUTOMATICA,
			bPriorita?"priorita": "fila(ubicazione),montante(ubicazione)::int2,colonna(ubicazione)::int2,piano(ubicazione)::int2");

		nTuples=PQntuples(PGRes);

		for(nIndex=0;nIndex<nTuples;nIndex++){
			nSettoreIndex=atoi(PQgetvalue(PGRes,nIndex,3));
			nDisplayIndex=atoi(PQgetvalue(PGRes,nIndex,4));
			nIOS=atoi(PQgetvalue(PGRes,nIndex,5));
			nCPU=atoi(PQgetvalue(PGRes,nIndex,6));
			nModulo=atoi(PQgetvalue(PGRes,nIndex,7));
			nRow=atoi(PQgetvalue(PGRes,nIndex,8));
			nCol=atoi(PQgetvalue(PGRes,nIndex,9));
			strcpy(szFilaUbicazione,PQgetvalue(PGRes,nIndex,12));
			pDisplay=&(Cfg.Displays[nDisplayIndex-1]);

			/*
			* rm 07-08-2001
			* Gestione test su settore specifico
			*/
			if(!nSettore || nSettore==nSettoreIndex){
				if(!strlen(szFila) || szFila[0]==szFilaUbicazione[0]){
					sprintf(pDisplay->szRiga_1_Display,"%04d",nRow);     /* Riga 1 del display */
					sprintf(pDisplay->szRiga_2_Display,"%04d",nCol);     /* Riga 2 del display */
					pDisplay->nStatoRiga1=NORMAL;
					pDisplay->nStatoRiga2=NORMAL;
					UpdateDisplay(pDisplay,TUTTO);

					// sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS,nCPU,nModulo,nRow,nCol);
					// SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SET_RC, szBuffer);

					SetLampada(nIOS, nCPU, nModulo,nRow,nCol);

					strcpy(RecordData[0][0]," ");
					strcpy(RecordData[0][1],PQgetvalue(PGRes,nIndex,0));   /* ubicazione */
					strcpy(RecordData[0][2],"ATTESA PRESSIONE TASTO");

					gtk_clist_append( GTK_CLIST(clist), RecordData[0]);
					if(create_gdk_pixmap(clist,&pixmap, &mask,"led_orange_on.xpm")){
						gtk_clist_set_pixmap( GTK_CLIST(clist), nIndex, 0, pixmap,mask);
						free_gdk_pixmap(pixmap,mask);
					}

					clist_move_to_row(GTK_CLIST(clist),nIndex);

					bTastoOK=FALSE;

					if(bSimulazione){
						int nDelayIndex;

						/* attesa di 10 X delay/10 */
						for(nDelayIndex=0;nDelayIndex<10;nDelayIndex++){
							usleep(100*nDelay);
							gtk_update();
						}
						sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS,nCPU,nModulo,nRow,nCol);
						SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_RC, szBuffer);
						bTastoOK=TRUE;

						gtk_clist_set_text( GTK_CLIST(clist), nIndex,2,"PRESSIONE TASTO SIMULATA");
						if(create_gdk_pixmap(clist,&pixmap, &mask,"stock_jump_to.xpm")){
							gtk_clist_set_pixmap( GTK_CLIST(clist), nIndex, 0, pixmap,mask);
							free_gdk_pixmap(pixmap,mask);
						}
					}

					while(!bTastoOK && !bFineTest){
						nAllMsg = 0;
						if((nCarIn = ReceiveMsgNoWait(ProcList[PROC_SETTORI].nQNumber, &nAllMsg, szInMsg))>=0){
							UnpackStructMsg(szInMsg, &InMsgStruct, szText, nCarIn);

							switch (InMsgStruct.srce-1){
								case PROC_IOS:
									switch(InMsgStruct.code){
										case IOS_PKL_BUTTON_PRESSED:
										{
											PUBICAZIONE pUbicazione;
											int nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex;

											sscanf(szText,"%d,%d,%d,%d,%d",&nMsgIOS,&nMsgCPU,&nMsgModulo,&nMsgRowIndex,&nMsgColIndex);

											/* trace_debug(&cGREEN,"TEST : Ricevuto IOS_PKL_BUTTON_PRESSED [IOS:%d CPU:%d m:%02d,r:%02d,c:%02d]", nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex); */

											gtk_clist_set_text( GTK_CLIST(clist), nIndex,2,"PRESSIONE TASTO OK");
											if(create_gdk_pixmap(clist,&pixmap, &mask,"led_green_on.xpm")){
												gtk_clist_set_pixmap( GTK_CLIST(clist), nIndex, 0, pixmap,mask);
												free_gdk_pixmap(pixmap,mask);
											}

											if((pUbicazione=SearchUbicazione(&Ubicazioni,nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex))!=NULL){
												if(nMsgIOS==nIOS && nMsgCPU==nCPU && nMsgModulo==nModulo && nMsgRowIndex==nRow && nMsgColIndex==nCol){
													bTastoOK=TRUE;
												}
											}
										}
										break;
										case IOS_PKL_LIGHT_DAMAGED:
										{
											PUBICAZIONE pUbicazione;
											int nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex;

											sscanf(szText,"%d,%d,%d,%d,%d",&nMsgIOS,&nMsgCPU,&nMsgModulo,&nMsgRowIndex,&nMsgColIndex);

											/* trace_debug(&cRED,"TEST : Ricevuto IOS_PKL_LIGHT_DAMAGED [IOS:%d CPU:%d m:%02d,r:%02d,c:%02d]", nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex); */

											gtk_clist_set_text( GTK_CLIST(clist), nIndex,2,"LAMPADA ROTTA");
											if(create_gdk_pixmap(clist,&pixmap, &mask,"led_red_on.xpm")){
												gtk_clist_set_pixmap( GTK_CLIST(clist), nIndex, 0, pixmap,mask);
												free_gdk_pixmap(pixmap,mask);
											}

											if((pUbicazione=SearchUbicazione(&Ubicazioni,nMsgIOS,nMsgCPU,nMsgModulo,nMsgRowIndex,nMsgColIndex))!=NULL){
												if(nMsgIOS==nIOS && nMsgCPU==nCPU && nMsgModulo==nModulo && nMsgRowIndex==nRow && nMsgColIndex==nCol){
													bTastoOK=TRUE;
												}
											}
										}
										break;
									}
								break;
							}
						}
						usleep(1000*Cfg.nDelay);
						gtk_update();
					}
					usleep(1000*Cfg.nDelay);
					gtk_update();
				}
			}
			gtk_update();
			if(bFineTest)
				break;
		}
		/*
		* Fine del test
		*/
		PQclear(PGRes);
		gtk_update();
		gtk_clist_clear(GTK_CLIST(clist));
	} while(bLoop && !bFineTest);

	if(bFineTest){
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Test impianto interrotto dall'utente");
	} else {
		gtk_label_printf(get_widget(dlg,"lb_msg"),"Test impianto terminato");
	}

	gtk_widget_set_sensitive(pb_cancel,TRUE);
	gtk_widget_set_sensitive(pb_stop,FALSE);
	gtk_widget_set_sensitive(pb_start,TRUE);
	gtk_widget_set_sensitive(pb_reset,TRUE);

	free(RecordData[0][0]);
	free(RecordData[0][1]);
	free(RecordData[0][2]);
}

void on_dlg_test_pb_stop_clicked          (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg = create_dlg_message();
	GtkWidget *lb_msg=get_widget(dlg,"lb_msg");
	GtkWidget *pb_ok=get_widget(dlg,"pb_ok");


	gtk_label_printf(lb_msg,"Premere OK confermare la fine del test in corso\n");
	g_signal_connect_swapped (GTK_OBJECT (pb_ok), "clicked", G_CALLBACK (do_fine_test), dlg);
	gtk_window_set_title (GTK_WINDOW (dlg), "Fine Test");
	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
	gtk_widget_show(dlg);

}

void on_dlg_test_pb_reset_clicked           (gpointer user_data, GtkButton *button)
{
	GtkWidget *dlg=GTK_WIDGET(user_data);
  GtkWidget *clist= get_widget(dlg,"lst");

	gtk_label_printf(get_widget(dlg,"lb_msg"),"Eseguito reset");
	gtk_clist_clear(GTK_CLIST(clist));

}

void do_fine_test(gpointer user_data, GtkButton *button)
{
	bFineTest=TRUE;
}

gboolean on_main_window_destroy_event (GtkWidget *widget, GdkEvent *event, gpointer user_data) 
{
	do_exit();
  return TRUE;
}


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


void on_clist_select_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),TRUE);
	gtk_widget_set_sensitive(get_widget(main_window,"pbm_edit"),TRUE);
}

void on_clist_unselect_row(GtkCList *clist, gint row, gint column, GdkEvent *event, gpointer user_data)
{
	gtk_widget_set_sensitive(get_widget(main_window,"pb_edit"),FALSE);
	gtk_widget_set_sensitive(get_widget(main_window,"pbm_edit"),FALSE);
}

void do_exit(void)
{
	GtkWidget *dlg_exit = create_dlg_exit();
	gtk_window_set_focus (GTK_WINDOW (dlg_exit), get_widget(dlg_exit,"pb_ok"));
	gtk_widget_show(dlg_exit);
	gtk_window_set_transient_for(GTK_WINDOW(dlg_exit),GTK_WINDOW(main_window));
}

void do_test(void)
{
	GtkWidget *dlg= create_dlg_test();
	GtkWidget *sw=get_widget(dlg,"sw_list");
	GtkWidget *clist;


	/*
	* Creazione clist widget 
	*/
	clist = gtk_clist_new (3);
	gtk_widget_ref (clist);
	gtk_object_set_data_full (GTK_OBJECT (dlg), "lst", clist, (GtkDestroyNotify) gtk_widget_unref);
	gtk_container_add (GTK_CONTAINER (sw), clist);

	/*
	* impostazione titolo colonna
	*/ 
	gtk_clist_set_column_title(GTK_CLIST(clist), 0, " ");
	gtk_clist_set_column_title(GTK_CLIST(clist), 1, "UBICAZIONE");
	gtk_clist_set_column_title(GTK_CLIST(clist), 2, "STATO");

	gtk_clist_set_column_min_width(GTK_CLIST(clist), 0,	100);
	gtk_clist_set_column_min_width(GTK_CLIST(clist), 1,	100);
	gtk_clist_set_column_min_width(GTK_CLIST(clist), 2,	100);

	gtk_clist_column_titles_show (GTK_CLIST (clist));
	gtk_clist_set_selection_mode (GTK_CLIST (clist),GTK_SELECTION_SINGLE);
	gtk_widget_show(clist);

	gtk_window_set_focus (GTK_WINDOW (dlg), get_widget(dlg,"pb_ok"));
	gtk_widget_show(dlg);
	gtk_window_set_transient_for(GTK_WINDOW(dlg),GTK_WINDOW(main_window));
}


/********************************************************************
************************** DLG_CONFIG *******************************
********************************************************************/

void on_dlg_config_pb_ok_clicked(gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));
	gtk_widget_destroy(GTK_WIDGET(user_data));
}


void on_dlg_config_pb_apply_clicked(gpointer user_data, GtkButton *button)
{
	ApplyConfigurazione(GTK_WIDGET(user_data));
}


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

void do_settori(GtkWidget *parent)
{
	int nRows,nColumns;
	GtkWidget *viewport_main;

  if((viewport_main=get_widget(parent,"viewport_main"))){
		gtk_widget_destroy(viewport_main);
	}
	viewport_main = gtk_viewport_new (NULL, NULL);
	gtk_widget_set_name (viewport_main, "viewport_main");
	gtk_widget_ref (viewport_main);
	gtk_object_set_data_full (GTK_OBJECT (main_window), "viewport_main", viewport_main,
														(GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (viewport_main);
	gtk_container_add (GTK_CONTAINER (get_widget(main_window,"sw_main")), viewport_main);

	for(nColumns=4;nColumns>=3;nColumns--){
		nRows=(Cfg.nNumeroSettori/nColumns)+((Cfg.nNumeroSettori%nColumns)?1:0);
		if(Cfg.nNumeroSettori%nColumns==0 || nColumns==3){
			break;
		}
	}

	create_table_settori(parent,get_widget(parent,"viewport_main"),nRows,nColumns);
	refresh_table_settori(parent,TRUE);
}

void create_table_settori(GtkWidget *dlg,GtkWidget *parent,int nRows,int nCols)
{
  GtkWidget *table_settori;
  GtkWidget *frame_settore;
  GtkWidget *vbox_settore;
  GtkWidget *hbox155;
  GtkWidget *vbox339;
  GtkWidget *lb_operatore;
  GtkWidget *lb_ordine;
  GtkWidget *lb_prodotto;
  GtkWidget *frame_display;
  GtkWidget *vbox_display;
  GtkWidget *lb_riga_1;
  GtkWidget *lb_riga_2;
  GtkWidget *hsep_settore;
  GtkWidget *table_settore;
  GtkWidget *lb_null;
  GtkWidget *lb_copie_prelevate;
  GtkWidget *lb_righe_prelevate;
  GtkWidget *lb_righe_da_prelevare;
  GtkWidget *lb_copie_da_prelevare;
  GtkWidget *lb_copie_evase;
  GtkWidget *lb_righe_evase;
  GtkWidget *frame235;
  GtkWidget *lb_titolo_righe;
  GtkWidget *frame236;
  GtkWidget *lb_titolo_copie;
  GtkWidget *frame237;
  GtkWidget *lb_titolo_prelevato;
  GtkWidget *frame238;
  GtkWidget *lb_titolo_da_prelevare;
  GtkWidget *frame239;
  GtkWidget *lb_titolo_evaso;
	char szBuffer[128];
	int nCol,nRow,nSettore;


  table_settori = gtk_table_new (nRows,nCols, FALSE);
  gtk_widget_set_name (table_settori, "table_settori");
  gtk_widget_ref (table_settori);
  gtk_object_set_data_full (GTK_OBJECT (dlg), "table_settori", table_settori, (GtkDestroyNotify) gtk_widget_unref);
  gtk_widget_show (table_settori);
  gtk_container_add (GTK_CONTAINER (parent), table_settori);

	nSettore=0;
	for(nRow=0;nRow<nRows;nRow++){
		for(nCol=0;nCol<nCols;nCol++){
			if(nSettore+1>Cfg.nNumeroSettori){
				break;
			}
			sprintf(szBuffer,"Settore %d",nSettore+1);
			frame_settore = gtk_frame_new (szBuffer);
			sprintf(szBuffer,"frame_settore_%d",nSettore+1);
			gtk_widget_set_name (frame_settore, szBuffer);
			gtk_widget_ref (frame_settore);
			gtk_object_set_data_full (GTK_OBJECT (dlg), szBuffer, frame_settore, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame_settore);
			gtk_table_attach (GTK_TABLE (table_settori), frame_settore, nCol, nCol+1, nRow, nRow+1,
												(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
												(GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
			gtk_container_set_border_width (GTK_CONTAINER (frame_settore), 5);

			vbox_settore = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_name (vbox_settore, "vbox_settore");
			gtk_widget_ref (vbox_settore);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "vbox_settore", vbox_settore, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (vbox_settore);
			gtk_container_add (GTK_CONTAINER (frame_settore), vbox_settore);

			hbox155 = gtk_hbox_new (FALSE, 0);
			gtk_widget_set_name (hbox155, "hbox155");
			gtk_widget_ref (hbox155);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "hbox155", hbox155, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (hbox155);
			gtk_box_pack_start (GTK_BOX (vbox_settore), hbox155, TRUE, TRUE, 0);

			vbox339 = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_name (vbox339, "vbox339");
			gtk_widget_ref (vbox339);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "vbox339", vbox339, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_set_usize (vbox339, 150, -2);
			gtk_widget_show (vbox339);
			gtk_box_pack_start (GTK_BOX (hbox155), vbox339, TRUE, TRUE, 0);

			lb_operatore = gtk_label_new ("operatore");
			gtk_widget_set_name (lb_operatore, "lb_operatore");
			gtk_widget_ref (lb_operatore);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_operatore", lb_operatore, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_operatore);
			gtk_box_pack_start (GTK_BOX (vbox339), lb_operatore, TRUE, TRUE, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_operatore), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_operatore), 5, 0);

			lb_ordine = gtk_label_new ("ordine");
			gtk_widget_set_name (lb_ordine, "lb_ordine");
			gtk_widget_ref (lb_ordine);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_ordine", lb_ordine, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_ordine);
			gtk_box_pack_start (GTK_BOX (vbox339), lb_ordine, TRUE, TRUE, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_ordine), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_ordine), 5, 0);

			lb_prodotto = gtk_label_new ("prodotto");
			gtk_widget_set_name (lb_prodotto, "lb_prodotto");
			gtk_widget_ref (lb_prodotto);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_prodotto", lb_prodotto, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_prodotto);
			gtk_box_pack_start (GTK_BOX (vbox339), lb_prodotto, TRUE, TRUE, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_prodotto), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_prodotto), 5, 0);

			frame_display = gtk_frame_new (NULL);
			gtk_widget_set_name (frame_display, "frame_display");
			gtk_widget_ref (frame_display);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame_display", frame_display, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame_display);
			gtk_box_pack_start (GTK_BOX (hbox155), frame_display, FALSE, TRUE, 0);

			vbox_display = gtk_vbox_new (FALSE, 0);
			gtk_widget_set_name (vbox_display, "vbox_display");
			gtk_widget_ref (vbox_display);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "vbox_display", vbox_display, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (vbox_display);
			gtk_container_add (GTK_CONTAINER (frame_display), vbox_display);

			lb_riga_1 = gtk_label_new ("");
			gtk_widget_set_name (lb_riga_1, "lb_riga_1");
			gtk_widget_ref (lb_riga_1);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_riga_1", lb_riga_1, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_set_usize (lb_riga_1, 100, -2);
			gtk_widget_show (lb_riga_1);
			gtk_box_pack_start (GTK_BOX (vbox_display), lb_riga_1, TRUE, TRUE, 0);

			lb_riga_2 = gtk_label_new ("");
			gtk_widget_set_name (lb_riga_2, "lb_riga_2");
			gtk_widget_ref (lb_riga_2);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_riga_2", lb_riga_2, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_set_usize (lb_riga_2, 100, -2);
			gtk_widget_show (lb_riga_2);
			gtk_box_pack_start (GTK_BOX (vbox_display), lb_riga_2, TRUE, TRUE, 0);

			hsep_settore = gtk_hseparator_new ();
			gtk_widget_set_name (hsep_settore, "hsep_settore");
			gtk_widget_ref (hsep_settore);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "hsep_settore", hsep_settore, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (hsep_settore);
			gtk_box_pack_start (GTK_BOX (vbox_settore), hsep_settore, FALSE, FALSE, 0);

			table_settore = gtk_table_new (4, 3, FALSE);
			gtk_widget_set_name (table_settore, "table_settore");
			gtk_widget_ref (table_settore);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "table_settore", table_settore, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (table_settore);
			gtk_box_pack_start (GTK_BOX (vbox_settore), table_settore, TRUE, TRUE, 0);
			gtk_container_set_border_width (GTK_CONTAINER (table_settore), 5);
			gtk_table_set_row_spacings (GTK_TABLE (table_settore), 5);
			gtk_table_set_col_spacings (GTK_TABLE (table_settore), 5);

			lb_null = gtk_label_new ("");
			gtk_widget_set_name (lb_null, "lb_null");
			gtk_widget_ref (lb_null);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_null", lb_null, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_null);
			gtk_table_attach (GTK_TABLE (table_settore), lb_null, 0, 1, 0, 1,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_null), 0, 0.5);

			lb_copie_prelevate = gtk_label_new ("");
			gtk_widget_set_name (lb_copie_prelevate, "lb_copie_prelevate");
			gtk_widget_ref (lb_copie_prelevate);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_copie_prelevate", lb_copie_prelevate, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_copie_prelevate);
			gtk_table_attach (GTK_TABLE (table_settore), lb_copie_prelevate, 2, 3, 1, 2,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_copie_prelevate), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_copie_prelevate), 5, 0);

			lb_righe_prelevate = gtk_label_new ("");
			gtk_widget_set_name (lb_righe_prelevate, "lb_righe_prelevate");
			gtk_widget_ref (lb_righe_prelevate);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_righe_prelevate", lb_righe_prelevate, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_righe_prelevate);
			gtk_table_attach (GTK_TABLE (table_settore), lb_righe_prelevate, 1, 2, 1, 2,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_righe_prelevate), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_righe_prelevate), 5, 0);

			lb_righe_da_prelevare = gtk_label_new ("");
			gtk_widget_set_name (lb_righe_da_prelevare, "lb_righe_da_prelevare");
			gtk_widget_ref (lb_righe_da_prelevare);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_righe_da_prelevare", lb_righe_da_prelevare, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_righe_da_prelevare);
			gtk_table_attach (GTK_TABLE (table_settore), lb_righe_da_prelevare, 1, 2, 2, 3,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_righe_da_prelevare), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_righe_da_prelevare), 5, 0);

			lb_copie_da_prelevare = gtk_label_new ("");
			gtk_widget_set_name (lb_copie_da_prelevare, "lb_copie_da_prelevare");
			gtk_widget_ref (lb_copie_da_prelevare);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_copie_da_prelevare", lb_copie_da_prelevare, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_copie_da_prelevare);
			gtk_table_attach (GTK_TABLE (table_settore), lb_copie_da_prelevare, 2, 3, 2, 3,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_copie_da_prelevare), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_copie_da_prelevare), 5, 0);

			lb_copie_evase = gtk_label_new ("");
			gtk_widget_set_name (lb_copie_evase, "lb_copie_evase");
			gtk_widget_ref (lb_copie_evase);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_copie_evase", lb_copie_evase, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_copie_evase);
			gtk_table_attach (GTK_TABLE (table_settore), lb_copie_evase, 2, 3, 3, 4,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_copie_evase), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_copie_evase), 5, 0);

			lb_righe_evase = gtk_label_new ("");
			gtk_widget_set_name (lb_righe_evase, "lb_righe_evase");
			gtk_widget_ref (lb_righe_evase);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_righe_evase", lb_righe_evase, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_righe_evase);
			gtk_table_attach (GTK_TABLE (table_settore), lb_righe_evase, 1, 2, 3, 4,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (0), 0, 0);
			gtk_misc_set_alignment (GTK_MISC (lb_righe_evase), 1, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_righe_evase), 5, 0);

			frame235 = gtk_frame_new (NULL);
			gtk_widget_set_name (frame235, "frame235");
			gtk_widget_ref (frame235);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame235", frame235, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame235);
			gtk_table_attach (GTK_TABLE (table_settore), frame235, 1, 2, 0, 1,
												(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
												(GtkAttachOptions) (GTK_FILL), 0, 0);

			lb_titolo_righe = gtk_label_new ("Righe");
			gtk_widget_set_name (lb_titolo_righe, "lb_titolo_righe");
			gtk_widget_ref (lb_titolo_righe);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_titolo_righe", lb_titolo_righe, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_titolo_righe);
			gtk_container_add (GTK_CONTAINER (frame235), lb_titolo_righe);
			gtk_misc_set_alignment (GTK_MISC (lb_titolo_righe), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_titolo_righe), 5, 0);

			frame236 = gtk_frame_new (NULL);
			gtk_widget_set_name (frame236, "frame236");
			gtk_widget_ref (frame236);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame236", frame236, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame236);
			gtk_table_attach (GTK_TABLE (table_settore), frame236, 2, 3, 0, 1,
												(GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
												(GtkAttachOptions) (GTK_FILL), 0, 0);

			lb_titolo_copie = gtk_label_new ("Copie");
			gtk_widget_set_name (lb_titolo_copie, "lb_titolo_copie");
			gtk_widget_ref (lb_titolo_copie);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_titolo_copie", lb_titolo_copie, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_titolo_copie);
			gtk_container_add (GTK_CONTAINER (frame236), lb_titolo_copie);
			gtk_misc_set_alignment (GTK_MISC (lb_titolo_copie), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_titolo_copie), 5, 0);

			frame237 = gtk_frame_new (NULL);
			gtk_widget_set_name (frame237, "frame237");
			gtk_widget_ref (frame237);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame237", frame237, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame237);
			gtk_table_attach (GTK_TABLE (table_settore), frame237, 0, 1, 1, 2,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (GTK_FILL), 0, 0);

			lb_titolo_prelevato = gtk_label_new ("Prelevato");
			gtk_widget_set_name (lb_titolo_prelevato, "lb_titolo_prelevato");
			gtk_widget_ref (lb_titolo_prelevato);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_titolo_prelevato", lb_titolo_prelevato, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_titolo_prelevato);
			gtk_container_add (GTK_CONTAINER (frame237), lb_titolo_prelevato);
			gtk_misc_set_alignment (GTK_MISC (lb_titolo_prelevato), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_titolo_prelevato), 5, 0);

			frame238 = gtk_frame_new (NULL);
			gtk_widget_set_name (frame238, "frame238");
			gtk_widget_ref (frame238);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame238", frame238, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame238);
			gtk_table_attach (GTK_TABLE (table_settore), frame238, 0, 1, 2, 3,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (GTK_FILL), 0, 0);

			lb_titolo_da_prelevare = gtk_label_new ("Da Prelevare");
			gtk_widget_set_name (lb_titolo_da_prelevare, "lb_titolo_da_prelevare");
			gtk_widget_ref (lb_titolo_da_prelevare);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_titolo_da_prelevare", lb_titolo_da_prelevare, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_titolo_da_prelevare);
			gtk_container_add (GTK_CONTAINER (frame238), lb_titolo_da_prelevare);
			gtk_misc_set_alignment (GTK_MISC (lb_titolo_da_prelevare), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_titolo_da_prelevare), 5, 0);

			frame239 = gtk_frame_new (NULL);
			gtk_widget_set_name (frame239, "frame239");
			gtk_widget_ref (frame239);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "frame239", frame239, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (frame239);
			gtk_table_attach (GTK_TABLE (table_settore), frame239, 0, 1, 3, 4,
												(GtkAttachOptions) (GTK_FILL),
												(GtkAttachOptions) (GTK_FILL), 0, 0);

			lb_titolo_evaso = gtk_label_new ("Evaso");
			gtk_widget_set_name (lb_titolo_evaso, "lb_titolo_evaso");
			gtk_widget_ref (lb_titolo_evaso);
			gtk_object_set_data_full (GTK_OBJECT (frame_settore), "lb_titolo_evaso", lb_titolo_evaso, (GtkDestroyNotify) gtk_widget_unref);
			gtk_widget_show (lb_titolo_evaso);
			gtk_container_add (GTK_CONTAINER (frame239), lb_titolo_evaso);
			gtk_misc_set_alignment (GTK_MISC (lb_titolo_evaso), 0, 0.5);
			gtk_misc_set_padding (GTK_MISC (lb_titolo_evaso), 5, 0);
			nSettore++;
		}
	}

}



void refresh_table_settori(GtkWidget *dlg,BOOL bRefreshDB)
{
	GtkWidget *frame_settore;
	int nSettore;
	char szBuffer[80];


	for(nSettore=0;nSettore<Cfg.nNumeroSettori;nSettore++){
		PSETTORE pSettore=&(pSettori[nSettore]);

		sprintf(szBuffer,"frame_settore_%d",nSettore+1);
		frame_settore=get_widget(dlg,szBuffer);


		gtk_widget_set_foreground(get_widget(frame_settore,"lb_ordine"),&cGREEN);
		gtk_widget_set_foreground(get_widget(frame_settore,"lb_prodotto"),&cGREEN);
		gtk_widget_set_foreground(get_widget(frame_settore,"lb_operatore"),&cGREEN);
		gtk_widget_set_foreground(get_widget(frame_settore,"lb_riga_1"),&cGREEN);
		gtk_widget_set_foreground(get_widget(frame_settore,"lb_riga_2"),&cGREEN);

		gtk_label_printf(get_widget(frame_settore,"lb_riga_1"),pSettore->szRiga_1_Display);
		gtk_label_printf(get_widget(frame_settore,"lb_riga_2"),pSettore->szRiga_2_Display);
	}
}

void do_lista_ubicazioni(GtkWidget *parent)
{
	char szSelectCmd[2048];
	GtkWidget *clist;

	xncGetFileString("test","ubicazioni", "", szSelectCmd, sizeof(szSelectCmd),Cfg.szCniCfg,NULL); 

	CreateListFromSelect(main_window, parent, &clist, "lst", GTK_SELECTION_SINGLE, szSelectCmd, 0, 0);
}





