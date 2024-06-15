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
#include <msg-box.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

void on_lista_ordini_row_activated (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
     g_log (NULL,G_LOG_LEVEL_DEBUG, "lista ordini row-activated");
}

gboolean on_lista_ordini_button_pressed (GtkWidget *treeview, GdkEventButton *event, gpointer userdata)
{
    if (event->type == GDK_BUTTON_PRESS  &&  event->button == 3) {
        g_print ("lista ordini Single right click on the tree view.\n");

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



void refresh_lista_ordini(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_refresh_table(win, (gchar *)"sw_list_ordini", (gchar *)"lst_ordini", (gchar *)priv->pszCfgFileName, (gchar *)"Tabella Ordini", (gchar *)"lista_ordini", Cfg.nTipoSelezione, NULL);
    g_signal_connect(find_child(GTK_WIDGET(win), "lst_ordini"), "row-activated"     , G_CALLBACK (on_lista_ordini_row_activated),  win);
    g_signal_connect(find_child(GTK_WIDGET(win), "lst_ordini"), "button-press-event", G_CALLBACK (on_lista_ordini_button_pressed), win);
}

gboolean LanciaOrdine(gpointer win, gchar *ordine,gboolean forzatura)
{
    gboolean bRetValue=FALSE;
    DBresult *DBRes;
    DBresult *DBResProgLancio;
    int nProgressivoLancio=0;
    char szStato[40];
    gboolean bOK=TRUE;

    trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "LanciaOrdine - %s - FORZATURA : %d",ordine, forzatura);

    DBRes=DBExecQuery(Cfg.nDebugLevel,"select rostato,roprgln from ric_ord where ordprog='%s';",ordine);
    if(DBntuples(DBRes)){
        strcpy(szStato,DBgetvalue(DBRes,0,0));
        nProgressivoLancio=atoi(DBgetvalue(DBRes,0,1));
        if (szStato[0]==ORDINE_RICEVUTO || 
            szStato[0]==ORDINE_SPEDITO || 
            szStato[0]==ORDINE_ELABORATO || 
            szStato[0]==ORDINE_IN_ANALISI || 
            szStato[0]==ORDINE_STAMPATA_RAC){
            if(nProgressivoLancio==0){
                /*
                * Il progressivo non e' mai stato registrato e quindi si tratta 
                * della prima volta (di lancio dell'ordine)
                */
                DBResProgLancio=DBExecQuery(Cfg.nDebugLevel>1,"select nextval ('proglancio');");
                if(DBntuples(DBResProgLancio)){
                    nProgressivoLancio=atoi(DBgetvalue(DBResProgLancio,0,0));
                } else {
                    bOK=FALSE;
                }
                DBclear(DBResProgLancio);
            }
        } else {
            bOK=FALSE;
            trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Attenzione : ordine [%s] stato non coerente !\n",ordine);
        }
    }
    DBclear(DBRes);
    
    if(bOK){
        if(forzatura){
            /* setto il flag ordine 'F' : forzato */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c',rocdflg='%c',roprgln=%d where ordprog='%s';",ORDINE_SPEDITO,ORDINE_FORZATO,nProgressivoLancio,ordine);
        } else {
            /* resetto il flag ordine */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c',rocdflg=' ',roprgln=%d where ordprog='%s';",ORDINE_SPEDITO,nProgressivoLancio,ordine);
        }
        if((bRetValue = (DBresultStatus(DBRes)==DBRES_COMMAND_OK))){
            UpdateOrdine(win, ordine);
        }
        DBclear(DBRes);
    }
    return bRetValue;
}

void CheckAnno(int nDebugLevel)
{
    int nAnno;
    DBresult *DBRes;
    DBresult *DBResAnno;
    char szDateBuffer[128];

    /*
    *    - st - 5/12/2000
    * controllo il cambiamento dell'anno per aggiornare il valore del segnacollo
    */
    DBResAnno=DBExecQuery(nDebugLevel,"select last_value from sequence_anno;");
    if(DBntuples(DBResAnno)){
        nAnno=atoi(DBgetvalue(DBResAnno,0,0));

        if (atoi(LeftStr(GetDateYYYYMMDD(szDateBuffer),4))!=nAnno){
            /* anno nuovo ... sequenze nuove ! */
            nAnno=atoi(LeftStr(GetDateYYYYMMDD(szDateBuffer),4));
            /* 
            * Anno 
            */
            DBRes=DBExecQuery(nDebugLevel,"select setval ('sequence_anno',%d);",nAnno); DBclear(DBRes);
            /*
            * XAB - PARTE DA 1
            */
            DBRes=DBExecQuery(nDebugLevel,"select setval ('sequence_xab',%d);",1); DBclear(DBRes);
            /*
            * DISTINTA - PARTE DA 1
            */
            DBRes=DBExecQuery(nDebugLevel,"select setval ('sequence_distinta',%d);",1); DBclear(DBRes);
            /*
            * Progressivo Lancio - Parte da 1000
            */
            DBRes=DBExecQuery(nDebugLevel,"select setval ('proglancio',%d);",1000); DBclear(DBRes);
        }
    }
    DBclear(DBResAnno);
}


/*
* ep_bool_t CambiaStatoOrdine(char *szOrdProg, char cStato)
* cambia lo stato del sottordine in PRODUZIONE in cStato
* ritorna: TRUE se update effettuato.
*/
ep_bool_t CambiaStatoOrdine(MainWindow *win, char *szOrdProg, char cStato)
{
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    
    /*
    * LG 18-10-2007
    * Controllo che l'ordine sia di produzione prima di cambiargli lo stato.
    */
    if (!OrdineProduzione(szOrdProg)) {
        trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Fallito Cambio Stato [ordine %s]. Ordine senza colli di produzione\n",szOrdProg);
        return FALSE;
    }
    
    /* ric_ord */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c' where ordprog='%s';",cStato,szOrdProg);
    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
        trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Errore in update su ric_ord [%s][%c]\n",szOrdProg,cStato);
    }
    DBclear(DBRes);

    return(bRetVal);
}

ep_bool_t SetNoteOrdine(MainWindow *win, char *szOrdProg)
{
    DBresult *DBRes;
    DBresult *DBResNote;
    char szNMCED[128];
    char szTPSPE[128];
    char szCDLIN[128];
    char szCDCLI[128];
    ep_bool_t bFound=FALSE;
    ep_bool_t bOK=TRUE;
    char *pPtr;
    int nNoteDOW;
    int nNoteWeek;
    int nCurrentDOW=-1;
    char szDSTSP[1024];
    char szBuffer[1024];
    char szDayName[128];
    char szData[128];
    char szDataDDMM[128];
    char szTMRCZ[128];
    int nTuples;
    char szDatiSDA[128];
    char szDSNOT[128];

    strcpy(szDSNOT,Cfg.szNotaCorriere);

    /* 
    * inizializzo i dati per SDA in caso di mancanza di data consegna 'variabile' specificata in configurazione 
    */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char('now'::timestamp,'DDMM')");
    strcpy(szDataDDMM,DBgetvalue(DBRes,0,0));
    DBclear(DBRes);
    /* rm + lg : non uso un default ... */
    strcpy(szDatiSDA,"");

    /*
    * seleziono gli ordini 
    */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,ronmced,rotpspe,rocdlin,rocdrid from ric_ord  where ordprog='%s';",szOrdProg);

    if((nTuples=DBntuples(DBRes))){

        strcpy(szNMCED,DBgetvalue(DBRes,0,1));
        StrTrimAll(szNMCED);
        strcpy(szTPSPE,DBgetvalue(DBRes,0,2));
        StrTrimAll(szTPSPE);
        strcpy(szCDLIN,DBgetvalue(DBRes,0,3));
        StrTrimAll(szCDLIN);
        strcpy(szCDCLI,DBgetvalue(DBRes,0,4));
        StrTrimAll(szCDCLI);
    } else {
        bOK=FALSE;
    }
    DBclear(DBRes);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cdtmrcz from cedole where cdnmced=%s;",szNMCED);

    if((nTuples=DBntuples(DBRes))){
        strcpy(szTMRCZ,DBgetvalue(DBRes,0,0));
    } else {
        bOK=FALSE;
    }
    DBclear(DBRes);

    /*
    * Primo giro e' sulle tabelle specifiche per cedola
    */
    //1: cerco con parametri elctpspe,elccdlin,elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='%s' and elccdlin='%s' and elccdcli='%s';",
            szNMCED,szTPSPE,szCDLIN,szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //2: cerco con parametri elctpspe='',elccdlin,elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='' and elccdlin='%s' and elccdcli='%s';",
            szNMCED,szCDLIN, szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //3: cerco con parametri elctpspe='',elccdlin='',elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='' and elccdlin='' and elccdcli='%s';",
            szNMCED,szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //4: cerco con parametri elctpspe='',elccdlin,elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='' and elccdlin='%s' and elccdcli='';",
            szNMCED,szCDLIN);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //5: cerco con parametri elctpspe,elccdlin='',elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='%s' and elccdlin='' and elccdcli='%s';",
            szNMCED,szTPSPE,szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //6: cerco con parametri elctpspe,elccdlin,elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='%s' and elccdlin='%s' and elccdcli='';",
            szNMCED,szTPSPE,szCDLIN);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //7: cerco con parametri elctpspe,elccdlin='',elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='%s' and elccdlin='' and elccdcli='';",
            szNMCED,szTPSPE);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //8: cerco con parametri elctpspe='',elccdlin='',elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='%s' and elctpspe='' and elccdlin='' and elccdcli='';",
            szNMCED);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    /*
    * Secondo giro e' sulle tabelle  generali
    * rm+lg 25-07-2013 : accorpato ttds_gen in ttds, vale il campo nmced = ''
    */
    //1: cerco con parametri elctpspe,elccdlin,elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='%s' and elccdlin='%s' and elccdcli='%s';",
            szTPSPE,szCDLIN,szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //2: cerco con parametri elctpspe='',elccdlin,elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and  elctpspe='' and elccdlin='%s' and elccdcli='%s';",
            szCDLIN, szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //3: cerco con parametri elctpspe='',elccdlin='',elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='' and elccdlin='' and elccdcli='%s';",
            szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //4: cerco con parametri elctpspe='',elccdlin,elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='' and elccdlin='%s' and elccdcli='';",
            szCDLIN);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //5: cerco con parametri elctpspe,elccdlin='',elccdcli
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='%s' and elccdlin='' and elccdcli='%s';",
            szTPSPE,szCDCLI);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //6: cerco con parametri elctpspe,elccdlin,elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='%s' and elccdlin='%s' and elccdcli='';",
            szTPSPE,szCDLIN);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //7: cerco con parametri elctpspe,elccdlin='',elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='%s' and elccdlin='' and elccdcli='';",
            szTPSPE);
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    //8: cerco con parametri elctpspe='',elccdlin='',elccdcli=''
    if(!bFound){
        DBResNote=DBExecQuery(Cfg.nDebugLevel>1,"select elcdstsp,elcdsnot from ttds where elcnmced='' and elctpspe='' and elccdlin='' and elccdcli='';");
        if(DBntuples(DBResNote)){
            strcpy(szDSTSP,DBgetvalue(DBResNote,0,0));
            strcpy(szDSNOT,DBgetvalue(DBResNote,0,1));
            bFound=TRUE;
        } else {
            bFound=FALSE;
        }
        DBclear(DBResNote);
    }
    if (!bFound) {
        trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Ordine [%s]: campo note corriere [%s-%s-%s] non trovato neanche nella tabella generale",szOrdProg,szNMCED,szTPSPE,szCDCLI);
    } else {
        //trasformo i campi parametrici
        /*
        * DSNOT
        */
        nNoteDOW=-1;
        nNoteWeek=0;
        strcpy(szBuffer,szDSNOT);
        if ((pPtr = strstr(szBuffer, "$DOM1"))) {
            nNoteDOW=0;
            nNoteWeek=0;
            strcpy(szDayName,"DOMENICA");
        } else if ((pPtr = strstr(szBuffer, "$LUN1"))) {
            nNoteDOW=1;
            nNoteWeek=0;
            strcpy(szDayName,"LUNEDI");
        } else if ((pPtr = strstr(szBuffer, "$MAR1"))) {
            nNoteDOW=2;
            nNoteWeek=0;
            strcpy(szDayName,"MARTEDI");
        } else if ((pPtr = strstr(szBuffer, "$MER1"))) {
            nNoteDOW=3;
            nNoteWeek=0;
            strcpy(szDayName,"MERCOLEDI");
        } else if ((pPtr = strstr(szBuffer, "$GIO1"))) {
            nNoteDOW=4;
            nNoteWeek=0;
            strcpy(szDayName,"GIOVEDI");
        } else if ((pPtr = strstr(szBuffer, "$VEN1"))) {
            nNoteDOW=5;
            nNoteWeek=0;
            strcpy(szDayName,"VENERDI");
        } else if ((pPtr = strstr(szBuffer, "$SAB1"))) {
            nNoteDOW=6;
            nNoteWeek=0;
            strcpy(szDayName,"SABATO");
        } else if ((pPtr = strstr(szBuffer, "$DOM2"))) {
            nNoteDOW=0;
            nNoteWeek=1;
            strcpy(szDayName,"DOMENICA");
        } else if ((pPtr = strstr(szBuffer, "$LUN2"))) {
            nNoteDOW=1;
            nNoteWeek=1;
            strcpy(szDayName,"LUNEDI");
        } else if ((pPtr = strstr(szBuffer, "$MAR2"))) {
            nNoteDOW=2;
            nNoteWeek=1;
            strcpy(szDayName,"MARTEDI");
        } else if ((pPtr = strstr(szBuffer, "$MER2"))) {
            nNoteDOW=3;
            nNoteWeek=1;
            strcpy(szDayName,"MERCOLEDI");
        } else if ((pPtr = strstr(szBuffer, "$GIO2"))) {
            nNoteDOW=4;
            nNoteWeek=1;
            strcpy(szDayName,"GIOVEDI");
        } else if ((pPtr = strstr(szBuffer, "$VEN2"))) {
            nNoteDOW=5;
            nNoteWeek=1;
            strcpy(szDayName,"VENERDI");
        } else if ((pPtr = strstr(szBuffer, "$SAB2"))) {
            nNoteDOW=6;
            nNoteWeek=1;
            strcpy(szDayName,"SABATO");
        } else {
        }

        if (nNoteDOW>=0) {
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select EXTRACT(DOW FROM date '%s')::int;",szTMRCZ);
            if((nTuples=DBntuples(DBRes))){
                nCurrentDOW=atoi(DBgetvalue(DBRes,0,0));
            } else {
                bOK=FALSE;
            }
            DBclear(DBRes);

            /* se la settimana e' la attuale */
            if (bOK && nNoteDOW>nCurrentDOW) {
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' - (SELECT EXTRACT(DOW FROM date '%s'))::int + %d),'DD/MM/YYYY');",nNoteWeek,szTMRCZ,szTMRCZ,nNoteDOW);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szData,DBgetvalue(DBRes,0,0));
                } else {
                }
                DBclear(DBRes);
                /* ricavo il DDMM per la gestione corriere SDA */
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' - (SELECT EXTRACT(DOW FROM date '%s'))::int + %d),'DDMM');",nNoteWeek,szTMRCZ,szTMRCZ,nNoteDOW);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szDataDDMM,DBgetvalue(DBRes,0,0));
                } else {
                }
                DBclear(DBRes);
            } else {
                /* se la settimana e' la successiva alla attuale */
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' + 7 + (%d - (SELECT EXTRACT(DOW FROM date '%s'))::int)),'DD/MM/YYYY');",nNoteWeek,szTMRCZ,nNoteDOW,szTMRCZ);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szData,DBgetvalue(DBRes,0,0));
                } else {
                    bOK=FALSE;
                }
                DBclear(DBRes);
                /* ricavo il DDMM per la gestione corriere SDA */
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' + 7 + (%d - (SELECT EXTRACT(DOW FROM date '%s'))::int)),'DDMM');",nNoteWeek,szTMRCZ,nNoteDOW,szTMRCZ);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szDataDDMM,DBgetvalue(DBRes,0,0));
                } else {
                    bOK=FALSE;
                }
                DBclear(DBRes);
            }

            /*
            * Con gli strstr di prima ho che pPtr punta a $ nella stringa.
            * Mi basta sostituire $ con \0 affinche' szBuffer sia troncato esattamente
            * all'altezza di $ e poi, dopo aver fatto il cat di giorno e data, fare il cat
            * di pPtr spostato di 4 caratteri ($ piu tre del giorno)
            */

            *pPtr='\0';
            strcpy(szDSNOT,szBuffer);
            strcat(szDSNOT,szDayName);
            strcat(szDSNOT," ");
            strcat(szDSNOT,szData);
            strcat(szDSNOT,pPtr+strlen("$****"));
            StrTrimAll(szDSNOT);

            /* Errato: costruisco stringa per SDA : [G][DDMM]M : [G] iniziale giorno settimana [DD] giorno mese [MM] mese anno M : fisso (mattina) */
            //sprintf(szDatiSDA,"%c%sM",szInizialeGiorno[nNoteDOW],szDataDDMM);
            /* Buono: costruisco stringa per SDA : G[DDMM]P : G fisso (giorno) giorno settimana [DD] giorno mese [MM] mese anno P : fisso (pomeriggio) */
            sprintf(szDatiSDA,"G%sP",szDataDDMM);

        } else {
            /* non ho riferimenti di data consegna */
            strcpy(szDatiSDA,"");
        }
        /*
        * DSTSP
        */
        nNoteDOW=-1;
        nNoteWeek=0;
        strcpy(szBuffer,szDSTSP);
        if ((pPtr = strstr(szBuffer, "$DOM1"))) {
            nNoteDOW=0;
            nNoteWeek=0;
            strcpy(szDayName,"DOMENICA");
        } else if ((pPtr = strstr(szBuffer, "$LUN1"))) {
            nNoteDOW=1;
            nNoteWeek=0;
            strcpy(szDayName,"LUNEDI");
        } else if ((pPtr = strstr(szBuffer, "$MAR1"))) {
            nNoteDOW=2;
            nNoteWeek=0;
            strcpy(szDayName,"MARTEDI");
        } else if ((pPtr = strstr(szBuffer, "$MER1"))) {
            nNoteDOW=3;
            nNoteWeek=0;
            strcpy(szDayName,"MERCOLEDI");
        } else if ((pPtr = strstr(szBuffer, "$GIO1"))) {
            nNoteDOW=4;
            nNoteWeek=0;
            strcpy(szDayName,"GIOVEDI");
        } else if ((pPtr = strstr(szBuffer, "$VEN1"))) {
            nNoteDOW=5;
            nNoteWeek=0;
            strcpy(szDayName,"VENERDI");
        } else if ((pPtr = strstr(szBuffer, "$SAB1"))) {
            nNoteDOW=6;
            nNoteWeek=0;
            strcpy(szDayName,"SABATO");
        } else if ((pPtr = strstr(szBuffer, "$DOM2"))) {
            nNoteDOW=0;
            nNoteWeek=1;
            strcpy(szDayName,"DOMENICA");
        } else if ((pPtr = strstr(szBuffer, "$LUN2"))) {
            nNoteDOW=1;
            nNoteWeek=1;
            strcpy(szDayName,"LUNEDI");
        } else if ((pPtr = strstr(szBuffer, "$MAR2"))) {
            nNoteDOW=2;
            nNoteWeek=1;
            strcpy(szDayName,"MARTEDI");
        } else if ((pPtr = strstr(szBuffer, "$MER2"))) {
            nNoteDOW=3;
            nNoteWeek=1;
            strcpy(szDayName,"MERCOLEDI");
        } else if ((pPtr = strstr(szBuffer, "$GIO2"))) {
            nNoteDOW=4;
            nNoteWeek=1;
            strcpy(szDayName,"GIOVEDI");
        } else if ((pPtr = strstr(szBuffer, "$VEN2"))) {
            nNoteDOW=5;
            nNoteWeek=1;
            strcpy(szDayName,"VENERDI");
        } else if ((pPtr = strstr(szBuffer, "$SAB2"))) {
            nNoteDOW=6;
            nNoteWeek=1;
            strcpy(szDayName,"SABATO");
        } else {
        }

        if (nNoteDOW>=0) {
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select EXTRACT(DOW FROM date '%s')::int;",szTMRCZ);
            if((nTuples=DBntuples(DBRes))){
                nCurrentDOW=atoi(DBgetvalue(DBRes,0,0));
            } else {
                bOK=FALSE;
            }
            DBclear(DBRes);

            if (nNoteDOW>nCurrentDOW) {
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' - (SELECT EXTRACT(DOW FROM date '%s'))::int + %d),'DD/MM/YYYY');",nNoteWeek,szTMRCZ,szTMRCZ,nNoteDOW);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szData,DBgetvalue(DBRes,0,0));
                } else {
                    bOK=FALSE;
                }
                DBclear(DBRes);
            } else {
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select to_char(((%d * 7) + date '%s' + 7 + (%d - (SELECT EXTRACT(DOW FROM date '%s'))::int)),'DD/MM/YYYY');",nNoteWeek,szTMRCZ,nNoteDOW,szTMRCZ);
                if((nTuples=DBntuples(DBRes))){
                    strcpy(szData,DBgetvalue(DBRes,0,0));
                } else {
                    bOK=FALSE;
                }
                DBclear(DBRes);
            }

            /*
            * Con gli strstr di prima ho che pPtr punta a $ nella stringa.
            * Mi basta sostituire $ con \0 affinche' szBuffer sia troncato esattamente
            * all'altezza di $ e poi, dopo aver fatto il cat di giorno e data, fare il cat
            * di pPtr spostato di 4 caratteri ($ piu tre del giorno)
            */

            *pPtr='\0';
            strcpy(szDSTSP,szBuffer);
            strcat(szDSTSP,szDayName);
            strcat(szDSTSP," ");
            strcat(szDSTSP,szData);
            strcat(szDSTSP,pPtr+strlen("$****"));
            StrTrimAll(szDSTSP);

        } else {
        }
    }
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rodsnot='%s',rodstsp='%s',roflnot='%c',rodtsda='%s' where ordprog='%s';",szDSNOT,szDSTSP,NOTA_CORRIERE_SETTATA,szDatiSDA,szOrdProg);
    if (DBresultStatus(DBRes)==DBRES_COMMAND_OK) {
    } else {
    }
    DBclear(DBRes);


    return bFound && bOK;
}

/*
* Verifica se un ordine e' di produzione cioe' contiene almeno un collo.
*/
ep_bool_t OrdineProduzione(char *szOrdine)
{
    ep_bool_t bRetValue=FALSE;
    DBresult *DBRes;

    DBRes=DBExecQuery(0, "select * from col_prod where ordprog='%s' limit 1;",szOrdine);
    if(DBntuples(DBRes)){
        bRetValue = TRUE;
    }
    DBclear(DBRes);

    return bRetValue;
}

/*
* ep_bool_t EvadiOrdine(char *szOrdProg)
* setta lo stato dei colli (e delle corripondenti righe) in EVASO
* gli assegna il peso reale e la data-ora di evasione
* cala la giacenza della ubicazione corrispondente
* ritorna: TRUE se update effettuato.
*/
ep_bool_t EvadiOrdine(MainWindow *win, char *szOrdProg)
{
    DBresult *DBRes;
    DBresult *DBResImp;
    DBresult *DBResColli;
    ep_bool_t bRetValue=TRUE;
    int nColli;
    int nIndex;
    int nColloIndex;
    int nCollo;
    int nTuples;
    char szCodProd[128];
    char szUbicazione[128];
    int nQTSpe;
    int nQTOrd;
    int nProgRiga;
    char szStato[40];
    int nPesoRealeOrdine=0;
    MainWindowPrivate *priv = get_main_window_private_instance (win);

#ifdef TRACE
    trace_debug(TRUE, TRUE, "EvadiOrdine [%s]",szOrdProg);
#endif


    /*
    * LG 18-10-2007
    * Controllo che l'ordine sia di produzione prima di cambiargli lo stato.
    */
    if (!OrdineProduzione(szOrdProg)) {
        trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Fallita Evasione Ordine [ordine %s]. Ordine senza colli di produzione\n",szOrdProg);
        return FALSE;
    }

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"begin work;");
    DBclear(DBRes);

    /*
    * Faccio un giro sui colli 
    * e evado e scarico le qt in ubicazione
    */
    DBResColli=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog,cpnmcol,cppspre from col_prod where ordprog='%s' and cpstato!='%c' and cpswlin!='%d';",szOrdProg,COLLO_EVASO,COLLO_AUTOMATICO);
    nColli=DBntuples(DBResColli);
    for(nColloIndex=0;nColloIndex<nColli;nColloIndex++){
        nCollo=atoi(DBgetvalue(DBResColli,nColloIndex,1));
        /* evado i colli */
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c',cppsrea=%d,cptmeva='now' where ordprog='%s' and cpnmcol=%d;",
            COLLO_EVASO,atoi(DBgetvalue(DBResColli,nColloIndex,2)),szOrdProg,nCollo);
        DBclear(DBRes);

        nPesoRealeOrdine+=atoi(DBgetvalue(DBResColli,nColloIndex,2));

        if(Cfg.bRifornimenti){
            /* 
            * aggiorno le qta di impegnato e giacenza  delle righe dei colli 
            */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rpcdpro,rpcdubi,rpprrig,rDBtspe,rDBtord,ubqtcas,ubqtimp,rpstato from rig_prod,ubicazioni where rpcdubi=ubicazione and ordprog='%s' and rpnmcol=%d;",szOrdProg,nCollo);
            nTuples=DBntuples(DBRes);
            for(nIndex=0;nIndex<nTuples;nIndex++){
                /*
                * gestisco le giacenze
                */
                strcpy(szCodProd,DBgetvalue(DBRes,nIndex,0));
                strcpy(szUbicazione,DBgetvalue(DBRes,nIndex,1));

                nProgRiga=atoi(DBgetvalue(DBRes,nIndex,2));
                nQTSpe=atoi(DBgetvalue(DBRes,nIndex,3));
                nQTOrd=atoi(DBgetvalue(DBRes,nIndex,4));
                strcpy(szStato,DBgetvalue(DBRes,nIndex,7));

                /* 
                * rm 29-01-02 : modificata la gestione della evasione righe per evitare scritture non aggiornate
                */
                DBResImp=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set ubqtcas=ubqtcas-%d,ubqtimp=ubqtimp-%d where ubicazione='%s';",
                    nQTSpe,nQTOrd,szUbicazione);
                if(atoi(DBcmdTuples(DBResImp))==0){
                    bRetValue=FALSE;
                }
                DBclear(DBResImp);

                /*
                * cambio lo stato della riga
                */
                DBResImp=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpstato='%c' where ordprog='%s' and rpnmcol=%d and rpprrig=%d;",RIGA_EVASA,szOrdProg,nCollo,nProgRiga);
                if(atoi(DBcmdTuples(DBResImp))==0){
                    bRetValue=FALSE;
                }
                DBclear(DBResImp);
            }
            DBclear(DBRes);
        }
    }
    DBclear(DBResColli);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set ropsrea=ropspre, rotmeva='now' where ordprog='%s';",szOrdProg);
    DBclear(DBRes);

    CambiaStatoOrdine(win, szOrdProg,ORDINE_EVASO);

    UpdateOrdine(win, szOrdProg);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"commit work;");
    DBclear(DBRes);

    return(bRetValue);
}



void SetMaster(MainWindow *win, gboolean bStato)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    Cfg.bMaster=bStato;

    if(Cfg.bMaster){
        gtk_widget_show(priv-> pbm_pulizia_dati_ricezione);
        gtk_widget_show(priv-> pbm_pulizia_dati_produzione);
        gtk_widget_show(priv-> pb_ricezione_dati);
        gtk_widget_show(priv-> pbm_spedizione_dati);
        gtk_widget_show(priv-> pbm_storicizzazione_dati);
        gtk_widget_show(priv-> pbm_configurazione);
        gtk_widget_show(priv-> pbm_setta_stato_ordine);
        gtk_widget_show(priv-> pixmap_rx); 
        gtk_widget_show(priv-> lb_rx);
        gtk_widget_show(priv-> pixmap_tx); 
        gtk_widget_show(priv-> lb_tx);
        gtk_widget_show(priv-> i2);
        gtk_widget_show(priv-> i5);

        gtk_image_set_from_icon_name (GTK_IMAGE(priv->pixmap_master),"gtk-yes", GTK_ICON_SIZE_BUTTON);

        trace_debug(TRUE, TRUE, "Cambiato utente [MASTER]");
        gtk_text_printf("GREEN",find_child(GTK_WIDGET(win), "txt_msgs"),"Cambiato utente [MASTER]\n");

    } else {
        gtk_widget_hide(priv->pbm_pulizia_dati_ricezione);
        gtk_widget_hide(priv->pbm_pulizia_dati_produzione);
        gtk_widget_hide(priv->pb_ricezione_dati);
        gtk_widget_hide(priv->pbm_spedizione_dati);
        gtk_widget_hide(priv->pbm_storicizzazione_dati);
        gtk_widget_hide(priv->pbm_configurazione);
        gtk_widget_hide(priv->pbm_setta_stato_ordine);
        gtk_widget_hide(priv->pixmap_rx); 
        gtk_widget_hide(priv->lb_rx);
        gtk_widget_hide(priv->pixmap_tx); 
        gtk_widget_hide(priv->lb_tx);
        gtk_widget_hide(priv->i2);
        gtk_widget_hide(priv->i5);

        gtk_image_set_from_icon_name (GTK_IMAGE(priv->pixmap_master),"gtk-no", GTK_ICON_SIZE_BUTTON);

        trace_debug(TRUE, TRUE, "Cambiato utente [OPERATORE]");
        gtk_text_printf("GREEN",find_child(GTK_WIDGET(win), "txt_msgs"),"Cambiato utente [OPERATORE]\n");
    }
}

void SetIOS(MainWindow *win, gboolean bStato)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    Cfg.bIOSConnected=bStato;

    if(!Cfg.bIOSConnected){
        gtk_widget_hide(priv->pb_start);
        gtk_widget_hide(priv->pb_stop);
        gtk_widget_hide(priv->impianto_menu);
        gtk_widget_hide(priv->pbm_carica_stato);
        gtk_widget_hide(priv->pbm_salva_stato);
        gtk_widget_hide(priv->pbm_carica_ultimo_stato_salvato);
        gtk_widget_hide(priv->pixmap_stato_linea);
        gtk_widget_hide(priv->lb_stato_linea);
        gtk_widget_hide(priv->separator_1);
        gtk_widget_hide(priv->i0);
        gtk_widget_hide(priv->i1);
    } else {
        gtk_widget_show(priv->pb_start);
        gtk_widget_show(priv->pb_stop);
        gtk_widget_show(priv->impianto_menu);
        gtk_widget_show(priv->pbm_carica_stato);
        gtk_widget_show(priv->pbm_salva_stato);
        gtk_widget_show(priv->pbm_carica_ultimo_stato_salvato);
        gtk_widget_show(priv->pixmap_stato_linea);
        gtk_widget_show(priv->lb_stato_linea);
        gtk_widget_show(priv->separator_1);
        gtk_widget_show(priv->i0);
        gtk_widget_show(priv->i1);
    }
}

void StartLinea(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    SendMessage(PROC_SETTORI, PROC_MAIN, START_LINEA, NULL);

    /*
    * Metto lo stato in wait in attesa che settori metta in start
    */

    gtk_label_printf(find_child(GTK_WIDGET (win), "lb_stato_linea"), "WAIT");
    gtk_widget_set_foreground( find_child(GTK_WIDGET (win),"lb_stato_linea"),&cORANGE);
    gtk_image_set_from_icon_name (GTK_IMAGE(find_child(GTK_WIDGET (win), "pixmap_stato_linea")),"gtk-no", GTK_ICON_SIZE_BUTTON);

    DBresult *DBRes;
    DBRes=DBExecQuery(Cfg.nDebugLevel,"insert into eventi values ('START','now');");
    DBclear(DBRes);
}

void StopLinea(MainWindow *win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    SendMessage( PROC_SETTORI, PROC_MAIN, STOP_LINEA, NULL);

    DBresult *DBRes;
    DBRes=DBExecQuery(Cfg.nDebugLevel,"insert into eventi values ('STOP','now');");
    DBclear(DBRes);
}

ep_bool_t ResetOrdine(gpointer win, gchar *ordine)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    DBresult *DBRes;
    char stato=ORDINE_RICEVUTO;
    ep_bool_t rc = TRUE;

    /*
    * resetto lo stato degli ordini e delle relative righe e colli
    */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"delete from rig_prod where ordprog='%s';",ordine); DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"delete from col_prod where ordprog='%s';",ordine); DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord  set rostato='%c',roprgln=0, ronmcll=0, ronmcla=0, ronmrgp=0 where ordprog='%s';",stato,ordine);
    
    if(atoi(DBcmdTuples(DBRes))==0){
        trace_debug_gtk(NULL, TRUE, TRUE, find_child(GTK_WIDGET(win), "txt_msgs"), "Fallito reset Ordine %s\n",ordine);
        rc = FALSE;
    }
    DBclear(DBRes);

    return rc;
}

void UpdateOrdine(MainWindow *win, char *szOrdineKey)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    ep_set_tipo_ordini(Cfg.szTipoOrdini);
    ep_set_tipo_ubicazioni(Cfg.szTipoUbicazioni);
    ep_update_table(win, "sw_list_ordini", "lst_ordini", (gchar *)priv->pszCfgFileName, "Tabella Ordini", "lista_ordini",szOrdineKey, FALSE);
}

