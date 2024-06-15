/*
* mainfun.c
* funzioni 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/
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

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include <msg-box.h>
#include "mainfun.h"


int update_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, gchar *pszKey, gboolean MoveToRow)
{
    char szBuffer[128];
    char szUpdateCmd[4096];

    GList *PS=NULL;

    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_key");
    int nKeyIndex=GetFileInt(paragraph,szBuffer, 0,cfg_file_name, NULL); 

    strcpy(szBuffer,table_item_name);
    strcat(szBuffer,"_update");
    GetFileString(paragraph,szBuffer, "", szUpdateCmd, sizeof(szUpdateCmd),cfg_file_name, NULL); 

    add_item_to_parse(&PS, "%ORDTIPO%", Cfg.szTipoOrdini, TRUE);
    add_item_to_parse(&PS, "%KEY%",     pszKey,           FALSE);
    UpdateTable(find_child(GTK_WIDGET (win), parent_name), list_name, nKeyIndex, szUpdateCmd,  pszKey, PS, MoveToRow);

	return TRUE;
}


int refresh_table(gpointer win, gchar *parent_name, gchar *list_name, gchar *cfg_file_name, gchar *paragraph, gchar *table_item_name, GtkSelectionMode mode, gchar *key)
{
    char szSelectCmd[4096];

    GetFileString(paragraph,table_item_name, "", szSelectCmd, sizeof(szSelectCmd),cfg_file_name, NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    if(key){
        add_item_to_parse(&PS, "%KEY%",key, FALSE);
    }

    int rc =  RefreshTable(find_child(GTK_WIDGET (win),parent_name), list_name, szSelectCmd, mode, PS, NULL, NULL);
    return rc;
}

void AddBancale(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

	priv->Linea.nmbnc++;
}

void SetFase(gpointer win, int nFase)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

	if(nFase==FASE_OLD){
		priv->nFase=priv->nFaseOld;
	} else {

		priv->nFaseOld=priv->nFase;
		priv->nFase=nFase;

		switch(nFase){
			case ATTESA_CONFERMA:
			break;
			case FASE_OLD:
			break;
			case ATTESA_INIZIO_LINEA:
			{
                /*
				GtkWidget *lst;
				if((lst=priv->lst)) gtk_widget_destroy(lst);
                */

				reset_dati_linea(win, &(priv->Linea));
				reset_dati_bancale(win, &(priv->Bancale));
				reset_dati_collo(win, &(priv->Collo));

				display_dati_linea(win, &(priv->Linea));
				display_dati_bancale(win, &(priv->Bancale));
				display_dati_collo(win, &(priv->Collo));

				gtk_entry_set_text(GTK_ENTRY(priv->entry_barcode),"");

				gtk_widget_set_sensitive(priv->pb_inizio_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_fine_linea,          FALSE);
				gtk_widget_set_sensitive(priv->pb_select_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_apertura_bancale,    FALSE);
				gtk_widget_set_sensitive(priv->pb_chiusura_bancale,    FALSE);
				gtk_widget_set_sensitive(priv->pb_stampa_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_stampa_bancale,      TRUE);
				gtk_widget_set_sensitive(priv->pb_config,              TRUE);
				gtk_widget_set_sensitive(priv->pb_exit,                TRUE);
				gtk_widget_set_sensitive(priv->entry_barcode,          FALSE);

				refresh_lista_linee_spedizione(win);

			}
			break;
			case ATTESA_INIZIO_BANCALE:
			{
                /*
				GtkWidget *lst;
				if((lst=priv->lst)) gtk_widget_destroy(lst);
                */

				reset_dati_bancale(win, &(priv->Bancale));
				reset_dati_collo(win, &(priv->Collo));

				display_dati_bancale(win, &(priv->Bancale));
				display_dati_collo(win, &(priv->Collo));

				gtk_entry_set_text(GTK_ENTRY(priv->entry_barcode),"");

				gtk_widget_set_sensitive(priv->pb_inizio_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_fine_linea,          TRUE);
				gtk_widget_set_sensitive(priv->pb_select_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_apertura_bancale,    TRUE);
				gtk_widget_set_sensitive(priv->pb_chiusura_bancale,    FALSE);
				gtk_widget_set_sensitive(priv->pb_stampa_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_stampa_bancale,      TRUE);
				gtk_widget_set_sensitive(priv->pb_config,              TRUE);
				gtk_widget_set_sensitive(priv->pb_exit,                TRUE);
				gtk_widget_set_sensitive(priv->entry_barcode,          FALSE);

				refresh_lista_bancali_spedizione(win);
				refresh_lista_linee_spedizione(win);
			}
			break;
			case ATTESA_LETTURA_BARCODE_COLLO:
				gtk_widget_set_sensitive(priv->pb_inizio_linea,        FALSE);
				gtk_widget_set_sensitive(priv->pb_fine_linea,          FALSE);
				gtk_widget_set_sensitive(priv->pb_select_linea,        FALSE);
				gtk_widget_set_sensitive(priv->pb_apertura_bancale,    FALSE);
				gtk_widget_set_sensitive(priv->pb_chiusura_bancale,    TRUE);
				gtk_widget_set_sensitive(priv->pb_stampa_linea,        TRUE);
				gtk_widget_set_sensitive(priv->pb_stampa_bancale,      TRUE);
				gtk_widget_set_sensitive(priv->pb_config,              FALSE);
				gtk_widget_set_sensitive(priv->pb_exit,                FALSE);
				gtk_widget_set_sensitive(priv->entry_barcode,          TRUE);

				gtk_window_set_focus (GTK_WINDOW(win), priv->entry_barcode);
				refresh_lista_colli_bancale(win);
			break;
		}
	}
}


/*
* ep_bool_t EvadiCollo(char *szOrdProg,int nCollo)
* ritorna: TRUE se update effettuato.
*/
ep_bool_t EvadiCollo(gpointer win, char *szOrdProg,int nCollo)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
	DBresult *DBResCollo;
	DBresult *DBRes;
	int nPesoReale=0;
	int nPesoTeorico=0;
	ep_bool_t bRetValue=TRUE;

#ifdef TRACE
	trace_debug(TRUE, TRUE, "EvadiCollo [%s - %d]",szOrdProg,nCollo);
#endif

	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"begin work;");
	DBclear(DBRes);

	DBResCollo=DBExecQuery(Cfg.nDebugLevel>1,"select cppsrea,cppspre from col_prod where ordprog='%s' AND cpnmcol = %d;",szOrdProg,nCollo);
	if(DBntuples(DBResCollo)){
		nPesoReale=atoi(DBgetvalue(DBResCollo,0,0));
		nPesoTeorico=atoi(DBgetvalue(DBResCollo,0,1));
	} else {
		bRetValue=FALSE;
	}
	DBclear(DBResCollo);

	DBResCollo=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c',cppsrea=%d,cptmeva='now' where ordprog='%s' AND cpnmcol = %d;",COLLO_EVASO,nPesoReale?nPesoReale:nPesoTeorico,szOrdProg,nCollo);

	if (!DBResCollo || DBresultStatus(DBResCollo) != DBRES_COMMAND_OK || 1!=atoi(DBcmdTuples(DBResCollo))) {
		bRetValue=FALSE;
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Fallito update su col_prod [%s - %d]",szOrdProg,nCollo);
#endif
	}
	DBclear(DBResCollo);

	if(bRetValue){
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"commit work;");
		DBclear(DBRes);
	} else {
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"abort work;");
		DBclear(DBRes);
	}

	/* verifico se tutti i colli dell'ordine sono evasi e - nel caso - evado l'ordine */
	CheckOrdineEvaso(win, szOrdProg);


	return(bRetValue);
}


/*
* ep_bool_t CheckOrdineEvaso(szOrdProg)
* controlla che l'ordine szOrdProg sia evaso (-> tutti i suoi sottordini);
* settando il colore delle rispettive righe nella lista colli.
* ritorna: TRUE se l'ordine e' evaso completamente.
*/
ep_bool_t CheckOrdineEvaso(gpointer win, char *szOrdProg)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
	DBresult *DBRes;
	DBresult *DBResColli;
	ep_bool_t bRetValue=TRUE;
	ep_bool_t bEvaso=FALSE;
	int nColli;
	int nColloIndex;
	int nCollo;
	char szStatoOrdine[40];
	int nPesoRealeOrdine;

	/* controllo se tutti i colli di linea dell'ordine sono evasi */
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rostato from ric_ord where ordprog='%s';",szOrdProg);
	if (DBntuples(DBRes)){
		strcpy(szStatoOrdine,DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	if(szStatoOrdine[0]==ORDINE_EVASO){
		/* ordine gia' evaso - non faccio niente */
		return TRUE;
	}
	
	/* controllo se tutti i colli di linea dell'ordine sono evasi */
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cppsrea from col_prod where ordprog='%s' and cpstato!='%c' and cpswlin='%d';",szOrdProg,COLLO_EVASO,COLLO_AUTOMATICO);

	if (DBresultStatus(DBRes)==DBRES_TUPLES_OK){

		if (DBntuples(DBRes)==0){
			/* tutti i colli sono stati evasi -> evadi l'ordine */
			bEvaso=TRUE;
#ifdef TRACE
			trace_debug(TRUE, TRUE, "CheckOrdineEvaso() -> Evado Ordine %s",szOrdProg);
#endif
		}
	} else {
		bRetValue=FALSE;
	}	
	DBclear(DBRes);

	if(bRetValue){
		if(bEvaso){
			DBresult *DBResWork;

			DBResWork=DBExecQuery(Cfg.nDebugLevel>1,"begin work;");
			DBclear(DBResWork);
			/*
			* Faccio un giro sui colli non automatici
			* (pallet/fuori formato/preconf ...)
			* e evado e scarico le qt in ubicazione
			*/
			DBResColli=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,cpnmcol,cppspre from col_prod where ordprog='%s' and cpstato!='%c' and cpswlin!='%d';",szOrdProg,COLLO_EVASO,COLLO_AUTOMATICO);
			nColli=DBntuples(DBResColli);
			if(nColli){
#ifdef TRACE
				trace_debug(TRUE, TRUE, "CheckOrdineEvaso() -> Evado %d colli non automatici dell'ordine %s",nColli,szOrdProg);
#endif
				for(nColloIndex=0;nColloIndex<nColli;nColloIndex++){
					nCollo=atoi(DBgetvalue(DBResColli,nColloIndex,1));
					/* evado i colli non automatici */
					DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c',cppsrea=%d,cptmeva='now' where ordprog='%s' and cpnmcol=%d;",
						COLLO_EVASO,atoi(DBgetvalue(DBResColli,nColloIndex,2)),szOrdProg,nCollo);
					DBclear(DBRes);
				}
			}
			DBclear(DBResColli);

			/*
			* cambio lo stato dell'ordine, data e ora evasione
			*/
			nPesoRealeOrdine=CalcPesoRealeOrdine(win, szOrdProg);

			DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c', ropsrea=%d,rotmeva='now' where ordprog='%s';",ORDINE_EVASO,nPesoRealeOrdine,szOrdProg);
			DBclear(DBRes);

		}
	}
	if(bRetValue){
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"commit work;");
		DBclear(DBRes);
	} else {
		DBRes=DBExecQuery(Cfg.nDebugLevel>1,"abort work;");
		DBclear(DBRes);
	}

	return bRetValue && bEvaso;
}


/*
* int CalcPesoRealeOrdine(char *szOrdProg)
* calcola il peso reale dell'ordine szOrdProg sommando i pesi dei suoi
* sottordini; inserisce il valore ottenuto nel DataBase assieme alla
* data e ora di evasione.
* ritorna: il peso reale dell'ordine
*/
int CalcPesoRealeOrdine(gpointer win, char *szOrdProg)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
	DBresult *DBRes;
	int nIndex;
	int nPesoRealeOrdine=0;
	int nColli;

	DBRes=DBExecQuery(TRUE,"select cppsrea,cppspre from col_prod where ordprog='%s';",szOrdProg);
	if (DBresultStatus(DBRes) == DBRES_TUPLES_OK){

		nColli=DBntuples(DBRes);

		for(nIndex=0;nIndex<nColli;nIndex++){
			if(atoi(DBgetvalue(DBRes,nIndex,0))){
				nPesoRealeOrdine+=(atoi(DBgetvalue(DBRes,nIndex,0)));
			} else {
				nPesoRealeOrdine+=(atoi(DBgetvalue(DBRes,nIndex,1)));
			}
		}

	} else {
#ifdef TRACE
		trace_debug(TRUE, TRUE, "CalcPesoRealeOrdine(%s) - Fallita select su col_prod",szOrdProg);
#endif
	}	
	DBclear(DBRes);

	return nPesoRealeOrdine;
}

void Beep(void)
{
    gdk_display_beep(gdk_display_get_default());
}

void FlashError(void)
{
#ifdef TODO
	SetPLCVar("LMPBNC",TRUE);
	usleep(200000);
	SetPLCVar("LMPBNC",FALSE);
	usleep(200000);
	SetPLCVar("LMPBNC",TRUE);
	usleep(200000);
	SetPLCVar("LMPBNC",FALSE);
	gdk_beep();
#endif
}

void stampa_linea(gpointer win, int nKey)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
	FILE *fp;
	char szFileName[128];
	char szDateBuffer[128];
	char szTimeBuffer[128];
	char szTitle[128];
	char szKey[128];
	LINEASPEDIZIONESTRUCT tmpLinea;

	if(!get_dati_linea(win, nKey,&tmpLinea)){
		Beep();
		trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Linea [%d] non presente\n",nKey);
		return;
	}

	sprintf(szFileName,"%s/Linea_%06d",Cfg.szPathStampe,nKey);
	if ((fp=fopen(szFileName,"w"))==(FILE *)NULL) {
		Beep();
		trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in stampa linea [%d]\n",nKey);
		return;
	}

	fprintf(fp,"A&L Easy Picking 4.0 - %s %s   Stampa Linea [%d] [%4d:%s-%s]\n\n", GetDate(szDateBuffer), GetTime(szTimeBuffer),tmpLinea.key,tmpLinea.nmced,tmpLinea.tpspe,tmpLinea.cdlin);

	fprintf(fp,"Progressivo Linea : %4d\n", tmpLinea.key);
	fprintf(fp,"Numero Bancali    : %4d\n", tmpLinea.nmbnc);
	fprintf(fp,"Numero Colli      : %4d\n", tmpLinea.nmcll);
	fprintf(fp,"Peso Netto        : %4d\n", tmpLinea.psrea);
	fprintf(fp,"Peso Calcolato    : %4d\n", tmpLinea.pspre);
	fprintf(fp,"Numero Cedola     : %4d\n", tmpLinea.nmced);
	fprintf(fp,"Tipo Spedizione   : %s\n",  tmpLinea.tpspe);
	fprintf(fp,"Codice Linea      : %s\n",  tmpLinea.cdlin);

	fprintf(fp,"\nLista bancali\n\n");

	sprintf(szKey,"b.key_linea=%d and c.key_bancale=b.key_bancale",tmpLinea.key);

    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%", Cfg.szTipoOrdini, TRUE);
    add_item_to_parse(&PS, "%KEY%",     szKey,            FALSE);
	PrintTable(priv->pszCfgFileName, "check_bancali","lista_bancali_spedizione",NULL,NULL, PS, fp, Cfg.nTipoStampe);

	fprintf(fp,"");
	fclose(fp);

	/* stampa la lista prodotti lotto di reso */
	sprintf(szTitle,"A&L - Stampa Linea %s %s",tmpLinea.tpspe,tmpLinea.cdlin);
	PrintFile(szFileName,Cfg.szPrinterConsole,szTitle,Cfg.nTipoStampe, TRUE);

	/* unlink(szFileName); */
}

void stampa_bancale(gpointer win, int nKey,ep_bool_t bPackingList,ep_bool_t bHeader)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
	FILE *fp;
	char szDateBuffer[128];
	char szTimeBuffer[128];
	char szFileName[128];
	char szTitle[128];
	char szKey[128];
	BANCALESTRUCT tmpBancale;

	if(!get_dati_bancale(win, nKey,&tmpBancale)){
		Beep();
		trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Bancale [%d] non presente\n",nKey);
		return;
	}

	if(bHeader){
		/* stampa frontale bancale */
		sprintf(szFileName,"%s/Bancale_%06d",Cfg.szPathStampe,nKey);
		if ((fp=fopen(szFileName,"w"))==(FILE *)NULL) {
			trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in stampa frontale bancale [%d]\n",tmpBancale.key);
			return;
		}
		fprintf(fp,"\n\n\n\n\n");
		fprintf(fp,"%s\n",tmpBancale.dslin);
		fprintf(fp,"%s %s\n",tmpBancale.tpspe,tmpBancale.cdlin);
		fprintf(fp,"Colli  :%4d\n",tmpBancale.nmcll);
		fprintf(fp,"Bancale:%4d\n",tmpBancale.nmbnc);
		fprintf(fp,"");
		fclose(fp);
		// sprintf(szTitle," --borders=no --underlay=\"Bancale %d\" ",nKey);
		// PrintFile(szFileName,Cfg.szPrinterConsole,NULL,TRUE,12,szTitle);
		sprintf(szTitle,"A&L - Stampa Bancale %d",tmpBancale.key);
        PrintFile(szFileName,Cfg.szPrinterConsole,szTitle,Cfg.nTipoStampe, TRUE);
	}

	if(bPackingList){
		sprintf(szFileName,"%s/Bancale_%06d",Cfg.szPathStampe,tmpBancale.key);
		if ((fp=fopen(szFileName,"w"))==(FILE *)NULL) {
			trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in stampa bancale [%d]\n",tmpBancale.key);
			return;
		}

		fprintf(fp,"A&L Easy Picking 4.0 - %s %s   Stampa Bancale [%d]\n\n", GetDate(szDateBuffer), GetTime(szTimeBuffer),tmpBancale.key);

		fprintf(fp,"Progressivo Linea : %4d\n", tmpBancale.key);
		fprintf(fp,"Numero Bancale    : %4d\n", tmpBancale.nmbnc);
		fprintf(fp,"Numero Colli      : %4d\n", tmpBancale.nmcll);
		fprintf(fp,"Peso Netto        : %4d\n", tmpBancale.psrea);
		fprintf(fp,"Peso Calcolato    : %4d\n", tmpBancale.pspre);
		fprintf(fp,"Numero Cedola     : %4d\n", tmpBancale.nmced);
		fprintf(fp,"Tipo Spedizione   : %s\n",  tmpBancale.tpspe);
		fprintf(fp,"Codice Linea      : %s\n",  tmpBancale.cdlin);

		fprintf(fp,"\nLista colli\n\n");

		sprintf(szKey,"key_bancale=%d",tmpBancale.key);

        GList *PS=NULL;
        add_item_to_parse(&PS, "%ORDTIPO%", Cfg.szTipoOrdini, TRUE);
        add_item_to_parse(&PS, "%KEY%",     szKey,            FALSE);
        PrintTable(priv->pszCfgFileName, "check_bancali","lista_colli_bancale",NULL,NULL, PS, fp, Cfg.nTipoStampe);

		fprintf(fp,"");
		fclose(fp);

		sprintf(szTitle,"A&L - Stampa Bancale %d",tmpBancale.key);
		// PrintFile(szFileName,Cfg.szPrinterConsole,szTitle,TRUE,0,NULL);
        PrintFile(szFileName,Cfg.szPrinterConsole,szTitle,Cfg.nTipoStampe, TRUE);
	}
}

/* 
* 'inizio' di una linea di spedizione
*/
void inizio_linea(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select nextval ('sequence_linee_spedizione');");
    if(DBntuples(DBRes)){
        priv->Linea.key=atoi(DBgetvalue(DBRes,0,0));
    } else {
        Beep();
        bOK=FALSE;
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in inserimento linea di spedizione");
    }
    DBclear(DBRes);

    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "insert into storico_linee_spedizione ( key_linea,tminit, stato) values (%d,'now','%c');", priv->Linea.key, LINEA_INIZIATA);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
            trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Linea di spedizione [%d] iniziata correttamente",priv->Linea.key);
        } else {
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in inizio linea di spedizione [%d]",priv->Linea.key);
        }
        DBclear(DBRes);

        get_dati_linea(win, priv->Linea.key,&(priv->Linea));
        display_dati_linea(win, &(priv->Linea));
    }

    SetFase(win, ATTESA_INIZIO_BANCALE);
}

void attiva_linea(gpointer win, GtkWidget *parent)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    char *pszString;
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(parent,"lst_linee");

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);

    if(lista && g_list_length(lista)){
        if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){
            /* ho ottenuto l'iter */
            gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter, 0, &pszString); 

            priv->Linea.key=atoi(pszString);

            g_free(pszString);

            if(get_dati_linea(win, priv->Linea.key,&(priv->Linea))){
                display_dati_linea(win,&(priv->Linea));
                refresh_lista_linee_spedizione(win);
                refresh_lista_bancali_spedizione(win);
                refresh_lista_colli_bancale(win);

                SetFase(win, ATTESA_INIZIO_BANCALE);
            }
        }
    }
}

void reset_dati_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    pLinea->key=0;
    pLinea->nmbnc=0;
    pLinea->nmcll=0;
    pLinea->psrea=0;
    pLinea->pspre=0;
    pLinea->nmced=0;
    pLinea->tpspe[0]='\0';;
    pLinea->cdlin[0]='\0';;
}

void display_dati_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    gtk_label_printf(priv->lb_key_linea,"%d",pLinea->key);
    gtk_label_printf(priv->lb_lnnmced,"%d",pLinea->nmced);
    gtk_label_printf(priv->lb_lnnmbnc,"%d",pLinea->nmbnc);
    gtk_label_printf(priv->lb_lnnmcll,"%d",pLinea->nmcll);
    gtk_label_printf(priv->lb_lnpsrea,"%7.2f",(float)(pLinea->psrea/(float)1000));
    gtk_label_printf(priv->lb_lnpspre,"%7.2f",(float)(pLinea->pspre/(float)1000));
    gtk_label_printf(priv->lb_lntpspe,pLinea->tpspe);
    gtk_label_printf(priv->lb_lncdlin,pLinea->cdlin);
}

void reset_dati_bancale(gpointer win, PBANCALESTRUCT pBancale)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    pBancale->key=0;
    pBancale->nmbnc=0;
    pBancale->nmcll=0;
    pBancale->psrea=0;
    pBancale->pspre=0;
    pBancale->nmced=0;
    pBancale->tpspe[0]='\0';;
    pBancale->cdlin[0]='\0';;
}

void display_dati_bancale(gpointer win,PBANCALESTRUCT pBancale)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    gtk_label_printf(priv->lb_key_bancale,"%d",pBancale->key);
    gtk_label_printf(priv->lb_bnnmcll,"%d",pBancale->nmcll);
    gtk_label_printf(priv->lb_bnpsrea,"%7.2f",(float)(pBancale->psrea/(float)1000));
    gtk_label_printf(priv->lb_bnpspre,"%7.2f",(float)(pBancale->pspre/(float)1000));
    gtk_label_printf(priv->lb_bntpspe,pBancale->tpspe);
    gtk_label_printf(priv->lb_bncdlin,pBancale->cdlin);
}

void reset_dati_collo(gpointer win, PCOLLOSTRUCT pCollo)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    pCollo->key=0;
    pCollo->ordprog[0]='\0';
    pCollo->cptpfor[0]='\0';
    pCollo->cpnmcol=0;
    pCollo->cpnmrgh=0;
    pCollo->cpnmcpe=0;
    pCollo->cppsrea=0;
    pCollo->cppspre=0;
    pCollo->cpnmced=0;
    pCollo->cptpspe[0]='\0';
    pCollo->cpcdlin[0]='\0';
}

void display_dati_collo(gpointer win,PCOLLOSTRUCT pCollo)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    gtk_label_printf(priv->lb_key_collo,"%d",pCollo->key);
    gtk_label_printf(priv->lb_ordprog,pCollo->ordprog);
    gtk_label_printf(priv->lb_cptpfor,pCollo->cptpfor);
    gtk_label_printf(priv->lb_cpnmcol,"%d",pCollo->cpnmcol);
    gtk_label_printf(priv->lb_cpnmrgh,"%d",pCollo->cpnmrgh);
    gtk_label_printf(priv->lb_cpnmcpe,"%d",pCollo->cpnmcpe);
    gtk_label_printf(priv->lb_cppsrea,"%7.2f",(float)(pCollo->cppsrea/(float)1000));
    gtk_label_printf(priv->lb_cppspre,"%7.2f",(float)(pCollo->cppspre/(float)1000));
    gtk_label_printf(priv->lb_cptpspe,"%s",pCollo->cptpspe);
    gtk_label_printf(priv->lb_cpcdlin,"%s",pCollo->cpcdlin);
}

void refresh_lista_linee_spedizione(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);

    refresh_table(win, "sw_linee","lst_linee", priv->pszCfgFileName, "check_bancali","lista_linee_attive",GTK_SELECTION_SINGLE,NULL);
}

void refresh_lista_bancali_spedizione(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    char szBuffer[128];

    sprintf(szBuffer,"b.key_linea=%d and c.key_bancale=b.key_bancale",priv->Linea.key);
    refresh_table(win, "sw_bancali","lst_bancali", priv->pszCfgFileName, "check_bancali","lista_bancali_spedizione",GTK_SELECTION_SINGLE,szBuffer);
}

void do_update_lista_bancali_spedizione(gpointer win,PBANCALESTRUCT pBancale)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    char szBuffer[128];

    sprintf(szBuffer,"b.key_linea=%d and b.key_bancale=%d and c.key_bancale=b.key_bancale",priv->Linea.key,pBancale->key);
    update_table(win, "sw_bancali","lst_bancali", priv->pszCfgFileName, "check_bancali","lista_bancali_spedizione",szBuffer,TRUE);
}

void refresh_lista_colli_bancale(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    char szBuffer[128];

    sprintf(szBuffer,"key_bancale=%d",priv->Bancale.key);
    refresh_table(win, "sw_colli","lst_colli", priv->pszCfgFileName, "check_bancali","lista_colli_bancale",GTK_SELECTION_SINGLE,szBuffer);
}

void update_lista_colli_bancale(gpointer win,PCOLLOSTRUCT pCollo)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    char szBuffer[128];

    sprintf(szBuffer,"key_bancale=%d and key_collo=%d",priv->Bancale.key,pCollo->key);
    update_table(win, "sw_colli","lst_colli", priv->pszCfgFileName, "check_bancali","lista_colli_bancale",szBuffer,TRUE);
}

ep_bool_t get_dati_linea(gpointer win, int nKey,PLINEASPEDIZIONESTRUCT pLinea)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    reset_dati_linea(win, pLinea);

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select key_linea, lnnmced, lntpspe, lncdlin from storico_linee_spedizione where key_linea=%d;",nKey);
    
    if(DBntuples(DBRes)){
        pLinea->key = atoi(DBgetvalue(DBRes,0,0));
        pLinea->nmced = atoi(DBgetvalue(DBRes,0,1));
        strcpy(pLinea->tpspe,DBgetvalue(DBRes,0,2));
        strcpy(pLinea->cdlin,DBgetvalue(DBRes,0,3));
    } else {
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);

    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select count(key_collo),sum(cppsrea),sum(cppspre) from storico_colli_bancale where key_linea=%d group by key_linea;",nKey);
        if(DBntuples(DBRes)){
            pLinea->nmcll = atoi(DBgetvalue(DBRes,0,0));
            pLinea->pspre = atoi(DBgetvalue(DBRes,0,1));
            pLinea->psrea = atoi(DBgetvalue(DBRes,0,2));
        }
        DBclear(DBRes);
    }
    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select count(key_bancale) from storico_bancali where key_linea=%d group by key_linea;",nKey);
        if(DBntuples(DBRes)){
            pLinea->nmbnc = atoi(DBgetvalue(DBRes,0,0));
        }
    }
    return bOK;
}

ep_bool_t get_dati_bancale(gpointer win, int nKey,PBANCALESTRUCT pBancale)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    reset_dati_bancale(win, pBancale);


    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select key_bancale, bnnmbnc, bnnmced, bntpspe, bncdlin from storico_bancali where key_bancale=%d;",nKey);
    if(DBntuples(DBRes)){
        pBancale->key = atoi(DBgetvalue(DBRes,0,0));
        pBancale->nmbnc = atoi(DBgetvalue(DBRes,0,1));
        pBancale->nmced = atoi(DBgetvalue(DBRes,0,2));
        strcpy(pBancale->tpspe,DBgetvalue(DBRes,0,3));
        strcpy(pBancale->cdlin,DBgetvalue(DBRes,0,4));
    } else {
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);

    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select count(key_collo),sum(cppsrea),sum(cppspre) from storico_colli_bancale where key_bancale=%d group by key_bancale;",nKey);
        if(DBntuples(DBRes)){
            pBancale->nmcll = atoi(DBgetvalue(DBRes,0,0));
            pBancale->pspre = atoi(DBgetvalue(DBRes,0,1));
            pBancale->psrea = atoi(DBgetvalue(DBRes,0,2));
        }
        DBclear(DBRes);
    }

    get_dati_distribuzione(pBancale->tpspe,pBancale->cdlin,pBancale->dslin);

    return bOK;
}

char *get_dati_distribuzione(char *tpspe,char *cdlin,char *dslin)
{
    DBresult *DBRes;

    /* ricerca descrizione linea di spedizione  */
    /* usa la tabella sddslin creata alo scopo */

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select dsdslin from sddslin where dstpspe='%s' and dscdlin='%s';",tpspe,cdlin);
    if(DBntuples(DBRes)){
        strcpy(dslin,DBgetvalue(DBRes,0,0));
    } else {
        strcpy(dslin,"LINEA INESISTENTE");
    }
    DBclear(DBRes);

    return dslin;
}

ep_bool_t get_dati_collo(gpointer win, char *szBarcodeCollo,PCOLLOSTRUCT pCollo)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    reset_dati_collo(win, pCollo);

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select c.ordprog,c.cptpfor,c.cpnmcol,c.cpnmrgh,c.cpnmcpe,c.cppspre,c.cppsrea,c.cpstato,o.ronmced,o.rotpspe,o.rocdlin from col_prod c,ric_ord o where cpbrcde='%s' and c.ordprog=o.ordprog;",szBarcodeCollo);
    if(DBntuples(DBRes)){
        strcpy(pCollo->ordprog,DBgetvalue(DBRes,0,0));
        strcpy(pCollo->cptpfor,DBgetvalue(DBRes,0,1));
        pCollo->cpnmcol = atoi(DBgetvalue(DBRes,0,2));
        pCollo->cpnmrgh = atoi(DBgetvalue(DBRes,0,3));
        pCollo->cpnmcpe = atoi(DBgetvalue(DBRes,0,4));
        pCollo->cppspre = atoi(DBgetvalue(DBRes,0,5));
        pCollo->cppsrea = atoi(DBgetvalue(DBRes,0,6));
        strcpy(pCollo->cpstato,DBgetvalue(DBRes,0,7));
        pCollo->cpnmced = atoi(DBgetvalue(DBRes,0,8));
        strcpy(pCollo->cptpspe,DBgetvalue(DBRes,0,9));
        strcpy(pCollo->cpcdlin,DBgetvalue(DBRes,0,10));
    } else {
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);

    return bOK;
}

/*
void do_destroy_widget       (gpointer user_data, GtkButton *button)
{
    gtk_widget_destroy(user_data);
    SetFase(FASE_OLD);
    SetPLCVar("LMPBNC",FALSE);
}
*/

void do_barcode_collo(gpointer win, char *szBarcode)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;


    if( get_dati_collo(win, szBarcode,&(priv->Collo))){
        /* 
        * collo presente in tabella colli di produzione 
        * verifico che sia evaso correttamente
        */
        if(priv->Collo.cpstato[0]!=COLLO_EVASO){
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] non evaso",priv->Collo.ordprog,priv->Collo.cpnmcol);
            FlashError();
            /*
            * rm 16-03-2004 : evasione automatica collo 
            */
            EvadiCollo(win, priv->Collo.ordprog,priv->Collo.cpnmcol);

        }
    } else {
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s] non presente",szBarcode);
        FlashError();
        bOK=FALSE;
    }
    /*
    * verifico che non sia gia' stato letto
    */
    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "select key_bancale,key_linea,ordprog,cpnmcol,data(tmread),ora(tmread) from storico_colli_bancale where ordprog='%s' and cpnmcol=%d;", priv->Collo.ordprog,priv->Collo.cpnmcol);
        if(DBntuples(DBRes)){
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%s] gia' letto",
                DBgetvalue(DBRes,0,2),
                DBgetvalue(DBRes,0,3));
            bOK=FALSE;
            Beep();

#ifdef TODO
            do_dlg_message_printf(main_window,"Collo [%s][%s] gia' letto \nLinea [%s] Bancale [%s] \n[%s:%s]",
                DBgetvalue(DBRes,0,2),
                DBgetvalue(DBRes,0,3),
                DBgetvalue(DBRes,0,0),
                DBgetvalue(DBRes,0,1),
                DBgetvalue(DBRes,0,4),
                DBgetvalue(DBRes,0,5));
#endif
        }
        DBclear(DBRes);
    }
    
    /*
    * Assegno un progressivo di lettura al collo
    */
    if(bOK){

        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select nextval ('sequence_colli_bancale');");
        if(DBntuples(DBRes)){
            priv->Collo.key=atoi(DBgetvalue(DBRes,0,0));
        } else {
            Beep();
            bOK=FALSE;
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in registrazione collo su bancale");
        }
        DBclear(DBRes);
    }
    /*
    * Verifico la linea di spedizione del collo
    */
    if(bOK){
        if(priv->Linea.tpspe[0]=='\0' || !strcmp(priv->Linea.tpspe,"TPSPE")){
            /* 
            * codice linea di spedizione non ancora assegnato
            */
            priv->Linea.nmced=priv->Collo.cpnmced;
            strcpy(priv->Linea.tpspe,priv->Collo.cptpspe);
            strcpy(priv->Linea.cdlin,priv->Collo.cpcdlin);

            update_linea(win, &(priv->Linea));

        } else {
            if(Cfg.bControllaCedola){
                /* 
                * cedola + codice linea di spedizione non congruente
                */
                if(priv->Linea.nmced!=priv->Collo.cpnmced || strcmp(priv->Linea.tpspe,priv->Collo.cptpspe) || strcmp(priv->Linea.cdlin,priv->Collo.cpcdlin)){
                    trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] linea [%s-%s] o cedola [%d] non congruenti",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin,priv->Collo.cpnmced);
                    Beep();
                    bOK=FALSE;
                    // TODO do_dlg_message_printf(main_window, "Collo [%s][%d]\nlinea [%s-%s] o cedola [%d] non congruenti",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin,priv->Collo.cpnmced);
                }
            } else {
                /* 
                * no cedola + codice linea di spedizione non congruente
                */
                if(strcmp(priv->Linea.tpspe,priv->Collo.cptpspe) || strcmp(priv->Linea.cdlin,priv->Collo.cpcdlin)){
                    trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] linea [%s-%s] non congruente",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin);
                    Beep();
                    bOK=FALSE;
                    // TODO do_dlg_message_printf(main_window, "Collo [%s][%d]\nlinea [%s-%s] non congruente",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin);
                }
            }
        }
    }
    if(bOK){
        if(priv->Bancale.tpspe[0]=='\0'){
            /* 
            * codice linea di spedizione non ancora assegnato
            */
            priv->Bancale.nmced=priv->Collo.cpnmced;
            strcpy(priv->Bancale.tpspe,priv->Collo.cptpspe);
            strcpy(priv->Bancale.cdlin,priv->Collo.cpcdlin);

            update_bancale(win, &(priv->Bancale));

        } else {
            if(Cfg.bControllaCedola){
                /* 
                * cedola + codice linea di spedizione non congruente
                */
                if(priv->Bancale.nmced!=priv->Collo.cpnmced || strcmp(priv->Bancale.tpspe,priv->Collo.cptpspe) || strcmp(priv->Bancale.cdlin,priv->Collo.cpcdlin)){
                    trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] linea [%s-%s] o cedola [%d] non congruenti",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin,priv->Collo.cpnmced);
                    Beep();
                    bOK=FALSE;
                }
            } else {
                /* 
                * non cedola + codice linea di spedizione non congruente
                */
                if(strcmp(priv->Bancale.tpspe,priv->Collo.cptpspe) || strcmp(priv->Bancale.cdlin,priv->Collo.cpcdlin)){
                    trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] linea [%s-%s] non congruente",priv->Collo.ordprog,priv->Collo.cpnmcol,priv->Collo.cptpspe,priv->Collo.cpcdlin);
                    Beep();
                    bOK=FALSE;
                }
            }
        }
    }

    /*
    * registro la lettura
    */
    if(bOK){

        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "insert into storico_colli_bancale ( key_linea,key_bancale, key_collo, ordprog, cpnmced, cptpspe, cpcdlin, cpnmcol, cppsrea, cppspre, tmread) values (%d,%d,%d,'%s',%d,'%s','%s',%d,%d,%d,'now');",
            priv->Linea.key,
            priv->Bancale.key,
            priv->Collo.key,
            priv->Collo.ordprog,
            priv->Collo.cpnmced,
            priv->Collo.cptpspe,
            priv->Collo.cpcdlin,
            priv->Collo.cpnmcol,
            priv->Collo.cppsrea,
            priv->Collo.cppspre);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
            trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Collo [%s][%d] letto correttamente",priv->Collo.ordprog,priv->Collo.cpnmcol);
        } else {
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in registrazione Collo [%s][%d]",priv->Collo.ordprog,priv->Collo.cpnmcol);
            Beep();
            bOK=FALSE;
        }
        DBclear(DBRes);
    }
    if(bOK){
        priv->Bancale.nmcll++;
        priv->Bancale.psrea+=priv->Collo.cppsrea;
        priv->Bancale.pspre+=priv->Collo.cppspre;

        priv->Linea.nmcll++;
        priv->Linea.psrea+=priv->Collo.cppsrea;
        priv->Linea.pspre+=priv->Collo.cppspre;

        update_lista_colli_bancale(win,&(priv->Collo));

        display_dati_collo(win,&(priv->Collo));
        display_dati_bancale(win,&(priv->Bancale));
        display_dati_linea(win,&(priv->Linea));
    }

}

/* 
* 'apertura' di un bancale di spedizione
*/
void do_apertura_bancale(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select nextval ('sequence_bancali');");
    if(DBntuples(DBRes)){
        priv->Bancale.key=atoi(DBgetvalue(DBRes,0,0));
    } else {
        Beep();
        bOK=FALSE;
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in inserimento bancale");
    }
    DBclear(DBRes);

    if(bOK){
        DBRes = DBExecQuery(Cfg.nDebugLevel>1, "insert into storico_bancali ( key_linea, key_bancale,bnnmbnc,tminit, stato) values (%d, %d,%d,'now','%c');", priv->Linea.key, priv->Bancale.key, priv->Linea.nmbnc+1,BANCALE_APERTO);
        if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
            trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Bancale [%d] aperto correttamente",priv->Bancale.key);
        } else {
            trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in apertura bancale [%d]",priv->Bancale.key);
        }
        DBclear(DBRes);
    }
    if(bOK){
        display_dati_bancale(win,&(priv->Bancale));
        refresh_lista_bancali_spedizione(win);
    }

    SetFase(win, ATTESA_LETTURA_BARCODE_COLLO);
}

/* 
* 'chiusura' di un bancale di spedizione
*/
ep_bool_t do_chiusura_bancale(gpointer win)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "update storico_bancali set tmclose='now', stato='%c' where key_bancale=%d;",BANCALE_CHIUSO,priv->Bancale.key);
    if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
        trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Bancale [%d] chiuso correttamente",priv->Bancale.key);
    } else {
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in chiusura bancale [%d]",priv->Bancale.key);
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);


    if(bOK){
		/* 
		* incremento il numero di bancali
		*/
		AddBancale(win);

        SetFase(win, ATTESA_INIZIO_BANCALE);
    }
	return bOK;
}


ep_bool_t update_bancale(gpointer win, PBANCALESTRUCT pBancale)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "update storico_bancali set bnnmced=%d,bntpspe='%s',bncdlin='%s' where key_bancale=%d;",pBancale->nmced,pBancale->tpspe,pBancale->cdlin,pBancale->key);
    if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
        trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Bancale [%d] aggiornato correttamente",pBancale->key);
    } else {
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in aggiornamento bancale [%d]",pBancale->key);
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);


    return bOK;
}

ep_bool_t update_linea(gpointer win, PLINEASPEDIZIONESTRUCT pLinea)
{
    MainWindowPrivate *priv = internal_main_window_get_instance_private (win);
    DBresult *DBRes;
    ep_bool_t bOK=TRUE;

    DBRes = DBExecQuery(Cfg.nDebugLevel>1, "update storico_linee_spedizione set lnnmced=%d,lntpspe='%s',lncdlin='%s' where key_linea=%d;",pLinea->nmced,pLinea->tpspe,pLinea->cdlin,pLinea->key);
    if (DBresultStatus(DBRes) == DBRES_COMMAND_OK  && atoi(DBcmdTuples(DBRes))==1){
        trace_debug_gtk("GREEN",FALSE,TRUE,priv->txt_msgs,"Linea di spedizione [%d] aggiornata correttamente",pLinea->key);
    } else {
        trace_debug_gtk("RED",FALSE,TRUE,priv->txt_msgs,"Errore in aggiornamento linea di spedizione [%d]",pLinea->key);
        Beep();
        bOK=FALSE;
    }
    DBclear(DBRes);

    return bOK;
}

