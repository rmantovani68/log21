/*
* mainproc.c
* work procedures 
* Easy Picking 4.0
* Copyright A&L srl 2003-2021
* Autore : Roberto Mantovani
*/

#include <gtk/gtk.h>
#include <glib/gprintf.h>

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
#include "mainfun.h"

gint SaveStateTimer( gpointer win )
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szFileName[128];

    if(priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_START){
        sprintf(szFileName,"%s/settori.stato.save", Cfg.szPathData);
        SendMessage(PROC_SETTORI, PROC_MAIN, SAVE_STATE, szFileName);
    }

    return TRUE;
}


/*
* Gestione Eventi (Messaggi e PLC)
*/
gint ProcessMsgs( gpointer win )
{
    int nAllMsg = 0;
    int nCarIn;
    char szInMsg[INFO_LEN];
    char szText[INFO_LEN];
    ep_msg_header_t InMsgStruct;
    static time_t lActualTime=0;
    static ep_bool_t bFirstTime=TRUE;
    static LINEA_STRUCT OldLineData;
    static ep_bool_t bDatiSpediti=FALSE;
    static ep_bool_t bReindexEseguito=FALSE;
    char szTxMonitor[128];
    char szRxMonitor[128];
    char szCommand[128];

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    if(bFirstTime){
        if(Cfg.nReindex){
            do_aggiorna_db(win);
        }
        gtk_text_printf(NULL,priv->txt_msgs,"A&L - Easy Picking 4.0 - Application Ready\n");
        switch(Cfg.szTipoOrdini[0]){
            default:
            case ORDINE_CATALOGO:
                gtk_text_printf("GREEN",priv->txt_msgs,"Tipo Ordini : [%c:%s]\n",Cfg.szTipoOrdini[0],"CATALOGO");
            break;
            case ORDINE_LANCIO:
                gtk_text_printf("GREEN",priv->txt_msgs,"Tipo Ordini : [%c:%s]\n",Cfg.szTipoOrdini[0],"LANCIO");
            break;
        }

        /*
        * installo il timeout di salvataggio settori
        * Utilizzando un tempo definito in configurazione
        */
        Cfg.nSaveStateTimer=g_timeout_add(Cfg.nSaveStateDelay, SaveStateTimer, win);

        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_lista_ordini),TRUE); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_messaggi),FALSE); 
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->rb_impianto),FALSE); 
    }

    /*
    * Controllo lo stato della Trasmissione/Ricezione
    */
    if(lActualTime!=time((time_t *)0)){


        /* ogni secondo controllo lo stato della ricezione / trasmissione */

        /* Spedizioni */

        sprintf(szRxMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szImportSpedizioni));
        if(FileExists(szRxMonitor)){ 
            if (priv->pDatiLinea) priv->pDatiLinea->bRxData=TRUE; 
        } else {
            /* Catalogo */
            sprintf(szRxMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szImportCatalogo));
            if(FileExists(szRxMonitor)){ 
                if (priv->pDatiLinea) priv->pDatiLinea->bRxData=TRUE; 
            } else {
                /* Tabelle */
                sprintf(szRxMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szImportTabelle));
                if(FileExists(szRxMonitor)){ 
                    if (priv->pDatiLinea) priv->pDatiLinea->bRxData=TRUE; 
                } else {
                    if (priv->pDatiLinea) priv->pDatiLinea->bRxData=FALSE;
                }
            }
        }

        /* Spedizioni */
        sprintf(szTxMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportSpedizioni));
        if(FileExists(szTxMonitor)){ 
            if (priv->pDatiLinea) priv->pDatiLinea->bTxData=TRUE; 
        } else {
            /* Corriere */
            sprintf(szTxMonitor,"%s/%s",Cfg.szPathExport,StrTrimAll(Cfg.szExportCorriere));
            if(FileExists(szTxMonitor)){ 
                if (priv->pDatiLinea) priv->pDatiLinea->bTxData=TRUE; 
            } else {
                if (priv->pDatiLinea) priv->pDatiLinea->bTxData=FALSE; 
            }
        }

        time(&lActualTime);

    }

    /*
    * Gestione stati linea
    */
    if(bFirstTime || (priv->pDatiLinea && (priv->pDatiLinea->nStatoLinea != OldLineData.nStatoLinea))){

        if (priv->pDatiLinea) OldLineData.nStatoLinea = priv->pDatiLinea->nStatoLinea;

        gtk_label_printf(find_child(GTK_WIDGET(win),"lb_stato_linea"), priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_START ? "START" : priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_STOP ? "STOP": "WAIT");
        gtk_widget_set_foreground(find_child(GTK_WIDGET(win),"lb_stato_linea"), priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_START ? &cGREEN : priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_STOP ? &cRED : &cORANGE);
        gtk_image_set_from_icon_name (GTK_IMAGE(find_child(GTK_WIDGET(win), "pixmap_stato_linea")), priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_START ? "gtk-yes" : priv->pDatiLinea && priv->pDatiLinea->nStatoLinea==LINEA_IN_STOP ? "gtk-no" : "gtk-pause", GTK_ICON_SIZE_BUTTON);
    }

    /*
    * Gestione stato tx/rx
    */
    if(bFirstTime || (priv->pDatiLinea && priv->pDatiLinea->bTxData!=OldLineData.bTxData)){
        OldLineData.bTxData = priv->pDatiLinea && priv->pDatiLinea->bTxData;
        gtk_image_set_from_icon_name (GTK_IMAGE(find_child(GTK_WIDGET(win), "pixmap_tx")), priv->pDatiLinea && priv->pDatiLinea->bTxData ? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
    }
    if(bFirstTime || (priv->pDatiLinea && priv->pDatiLinea->bRxData!=OldLineData.bRxData)){
        OldLineData.bRxData = priv->pDatiLinea && priv->pDatiLinea->bRxData;
        gtk_image_set_from_icon_name (GTK_IMAGE(find_child(GTK_WIDGET(win), "pixmap_rx")), priv->pDatiLinea && priv->pDatiLinea->bRxData ? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
    }

    
    /*
    * Gestione analisi volumetrica
    */
    if(bFirstTime || (priv->pDatiLinea && priv->pDatiLinea->bAnalisi!=OldLineData.bAnalisi)){
        OldLineData.bAnalisi = priv->pDatiLinea && priv->pDatiLinea->bAnalisi;
        gtk_label_printf(find_child(GTK_WIDGET(win),"lb_analisi_volumetrica"), priv->pDatiLinea && priv->pDatiLinea->bAnalisi ? "ANALISI" : "");
        gtk_widget_set_foreground(find_child(GTK_WIDGET(win),"lb_analisi_volumetrica"), priv->pDatiLinea && priv->pDatiLinea->bAnalisi ? &cGREEN : &cRED);
        gtk_image_set_from_icon_name (GTK_IMAGE(GTK_WIDGET(find_child(GTK_WIDGET(win), "pixmap_analisi_volumetrica"))), priv->pDatiLinea && priv->pDatiLinea->bAnalisi ? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
    }
    /*
    * Gestione Stampa RAC
    */
    if(bFirstTime || (priv->pDatiLinea && priv->pDatiLinea->bStampaRAC!=OldLineData.bStampaRAC)){
        OldLineData.bStampaRAC = priv->pDatiLinea && priv->pDatiLinea->bStampaRAC;
        gtk_label_printf(find_child(GTK_WIDGET(win),"lb_stampa_rac"), priv->pDatiLinea && priv->pDatiLinea->bStampaRAC ? "STAMPA RAC" : "");
        gtk_widget_set_foreground(find_child(GTK_WIDGET(win),"lb_stampa_rac"), priv->pDatiLinea && priv->pDatiLinea->bStampaRAC ? &cGREEN : &cRED);
        gtk_image_set_from_icon_name (GTK_IMAGE(GTK_WIDGET(find_child(GTK_WIDGET(win), "pixmap_stampa_rac"))), priv->pDatiLinea && priv->pDatiLinea->bStampaRAC ? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
    }
    /*
    * Gestione Stampa
    */
    if(bFirstTime || (priv->pDatiLinea && priv->pDatiLinea->bStampa!=OldLineData.bStampa)){
        OldLineData.bStampa= priv->pDatiLinea && priv->pDatiLinea->bStampa;
        gtk_label_printf(find_child(GTK_WIDGET(win),"lb_stampa"), priv->pDatiLinea && priv->pDatiLinea->bStampa? "STAMPA" : "");
        gtk_widget_set_foreground(find_child(GTK_WIDGET(win),"lb_stampa"), priv->pDatiLinea && priv->pDatiLinea->bStampa? &cGREEN : &cRED);
        gtk_image_set_from_icon_name (GTK_IMAGE(GTK_WIDGET(find_child(GTK_WIDGET(win), "pixmap_stampa"))), priv->pDatiLinea && priv->pDatiLinea->bStampa? "gtk-yes" : "gtk-no", GTK_ICON_SIZE_BUTTON);
    }


    /*
    * Messaggi da altri processi
    */
    while((nCarIn = ProcessReceiveMsgNoWait(PROC_MAIN, &nAllMsg, szInMsg))>=0){

        UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

#ifdef TRACE
        if(Cfg.nDebugLevel){
            trace_debug(TRUE, TRUE, "Msg [code=%d, srce=%d, dest=%d] %s%s%s from [%s]",
                InMsgStruct.code,
                InMsgStruct.srce,
                InMsgStruct.dest,
                strlen(szText)?"[":"",strlen(szText)? szText : "",strlen(szText)?"]":"",
                ProcessGetName(InMsgStruct.srce));
        }
#endif

        /*
        * notifica di Start/Stop Processi
        */
        switch (InMsgStruct.code){
            case DISPLAY_MSG:
            {
                gchar *color;
                switch (InMsgStruct.srce){
                    case PROC_VOLUM:        color="ORANGE";break;
                    case PROC_SETTORI:      color="GREEN";break;
                    case PROC_PRINTLABEL:   color="BLUE";break;
                    case PROC_RECEIVE:      color="CYAN";break;
                    case PROC_AUTO_TASKS:   color="GREY";break;
                    default:                color="BLACK";break;
                }
                trace_debug_gtk(color, TRUE, 1, priv->txt_msgs, "%s Display MSG %s", ProcessGetName(InMsgStruct.srce),szText);
            }
            break;
            case PROGRAM_STARTED:
            {
                gchar *color;
                switch (InMsgStruct.srce){
                    case PROC_VOLUM:        color="ORANGE";break;
                    case PROC_SETTORI:      color="GREEN";break;
                    case PROC_PRINTLABEL:   color="BLUE";break;
                    case PROC_RECEIVE:      color="CYAN";break;
                    case PROC_AUTO_TASKS:   color="GREY";break;
                    case PROC_FINE_LINEA:   color="YELLOW";break;
                    default:                color="BLACK";break;
                }
                trace_debug_gtk(color, TRUE, 1, priv->txt_msgs, "%s Started", ProcessGetName(InMsgStruct.srce));
                switch (InMsgStruct.srce){
                    case PROC_VOLUM:        color="ORANGE";break;
                    case PROC_SETTORI:      color="GREEN";break;
                    case PROC_PRINTLABEL:   color="BLUE";break;
                    case PROC_RECEIVE:      color="CYAN";break;
                    case PROC_AUTO_TASKS:   color="GREY";break;
                    case PROC_FINE_LINEA:
                        SendMessage(PROC_FINE_LINEA, PROC_MAIN, DISPLAY_MSG, "TEST MESSAGE -1\n");
                        SendMessage(PROC_FINE_LINEA, PROC_MAIN, DISPLAY_MSG, "TEST MESSAGE -2\n");
                    break;
                    default:                color="BLACK";break;
                }
            }
            break;
            case PROGRAM_STOPPED:
            {
                gchar *color;
                switch (InMsgStruct.srce){
                    case PROC_VOLUM:        color="ORANGE";break;
                    case PROC_SETTORI:      color="GREEN";break;
                    case PROC_PRINTLABEL:   color="BLUE";break;
                    case PROC_RECEIVE:      color="CYAN";break;
                    case PROC_AUTO_TASKS:   color="GREY";break;
                    default:                color="BLACK";break;
                }
                trace_debug_gtk(color, TRUE, 1, priv->txt_msgs, "%s Stopped", ProcessGetName(InMsgStruct.srce));
            }
            break;
            case REFRESH_ORDINE:
                trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "REFRESH_ORDINE [%s]",szText);
                UpdateOrdine(win, szText);
            break;
            case REFRESH_LISTA_ORDINI:
                trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "REFRESH_LISTA_ORDINI");

                refresh_lista_ordini(win);
            break;
            case RIASSEGNA_NOTE_ORDINE_CEDOLA:
            {
                char szNMCED[128];
                char szOrdProg[128];
                char *pPtr;
                char *szSeparator=",\t\n";
                DBresult *DBRes;
                int nIndex;

                /* CEDOLA */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }
#ifdef TRACE
                trace_debug(TRUE, TRUE, "RIASSEGNA_NOTE_ORDINE_CEDOLA [%s]",szNMCED);
#endif

                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where ronmced='%s';",szNMCED);
                if (DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)){
                    for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                        strcpy(szOrdProg,DBgetvalue(DBRes,nIndex,0));
                        /*
                        * lg 27-02-2012: setta le note ordine nella tabella ric_ord
                        */
                        if(SetNoteOrdine(win, szOrdProg)){
                        } else {
                        }
                    }
                }
                DBclear(DBRes);
            }
            break;
        }

        switch (InMsgStruct.srce){
            case PROC_VOLUM:
                switch (InMsgStruct.code){
                    case VOLUM_START_ANALISI:
                    break;
                    case VOLUM_END_ANALISI:
                    {
                        trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs,"Fine Analisi Volumetrica");
                        /*
                        * Stampa tutti i moduli RAC
                        */
                        if(Cfg.nStampaRAC==DOPO_IL_LANCIO){
                            /*
                            * se richiesto 
                            * stampo le packing list
                            */
                            if(Cfg.bStampaPackingList){
                                /*
                                * Stampare tutti gli ordini in stato A
                                */
                                DBresult *DBRes;
                                int nIndex;
                                int nCollo;
                                char szOrdProg[128];
                                char szBuffer[256];

                                trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs,"Inizio Stampa packing list colli");

                                /* stampo la packing list di tutti i colli degli ordini analizzati */
                                DBRes=DBExecQuery(Cfg.nDebugLevel,"select c.ordprog,c.cpnmcol from col_prod c,ric_ord o where c.ordprog=o.ordprog and o.rostato='%c' order by o.roprgln,c.cpnmcol;",ORDINE_ELABORATO);
                                if (DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)){
                                    for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                                        strcpy(szOrdProg,DBgetvalue(DBRes,nIndex,0));
                                        nCollo=atoi(DBgetvalue(DBRes,nIndex,1));
                                        sprintf(szBuffer,"%s,%d,%s",szOrdProg,nCollo,Cfg.szPrinterConsole);
                                        SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST_COLLO,szBuffer);
                                    }
                                }
                                DBclear(DBRes);
                            }
                            /*
                            * stampa etichette dopo analisi volumetrica
                            */
                            trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs,"Inizio Stampa Moduli RAC");
                            SendMessage(PROC_PRINTLABEL, PROC_MAIN, PRINT_ALL_RAC,Cfg.szLabelPrinterConsole);

                        }
                    }
                    break;
                    case VOLUM_START_ANALISI_TEST:
                        trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs,"Inizio calcolo numero colli");
                    break;
                    case VOLUM_END_ANALISI_TEST:
                    {
                        trace_debug_gtk("ORANGE", TRUE, TRUE, priv->txt_msgs,"Fine calcolo numero colli");

                        refresh_lista_ordini(win);
                    }
                    break;
                    case VOLUM_END_ANALISI_ORDINE:
                        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Fine analisi ordine [%s]",szText);
                        UpdateOrdine(win, szText);
                    break;
                    case VOLUM_ERRORE_ANALISI_ORDINE:
                        trace_debug_gtk(NULL, TRUE, TRUE, priv->txt_msgs, "Errore in analisi ordine : %s\n",szText);
                        UpdateOrdine(win, szText);
                    break;
                }
            break;
            case PROC_SETTORI:
                switch (InMsgStruct.code){
                    case SETTORI_ORDINE_EVASO:
                    {
                        char szOrdProg[128]="";
                        char szBuffer[128];
                        ep_bool_t bColliPresenti=FALSE;
                        DBresult *DBRes;
                        int nIndex;
                        int nCollo;
                        int nCopie;

                        /* OrdProg       */ strcpy(szOrdProg,szText);

                        if(Cfg.nDebugLevel>1){
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Ordine [%s] Evaso da settori", szOrdProg);
                        }
                        /*
                        * EvasioneOrdine
                        */
                        EvadiOrdine(win, szOrdProg);

                        UpdateOrdine(win, szOrdProg);

                        bColliPresenti=FALSE;
                        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cpnmcol,cpnmcpe from col_prod where ordprog='%s';",szOrdProg);
                        if (DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)){
                            for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                                nCopie=atoi(DBgetvalue(DBRes,nIndex,1));
                                if(nCopie){
                                    /* ci sono colli con copie (NON VUOTI) */
                                    bColliPresenti=TRUE;
                                }
                            }
                        }
                        DBclear(DBRes);

                        if(bColliPresenti && Cfg.nStampaRAC==INIZIO_PRELIEVO){
                            /*
                            * stampa etichetta su inizio prelievo
                            */
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Stampa Modulo RAC Ordine [%s]",szOrdProg);

                            /*
                            * Spedizione messaggi di stampa etichette e packing list a printlabel
                            */
                            sprintf(szBuffer,"%s,%s",szOrdProg,Cfg.szLabelPrinterConsole);
                            SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC,szBuffer);
                            //sprintf(szBuffer,"%s,%s",szOrdProg,priv->szXABPrinterConsole);
                            //SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_OLD_RAC,szBuffer);

                            if(Cfg.bStampaPackingList){
                                /* stampo la packing list di tutti i colli appartenenti all'ordine */
                                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cpnmcol from col_prod where ordprog='%s';",szOrdProg);
                                if (DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)){
                                    for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                                        nCollo=atoi(DBgetvalue(DBRes,nIndex,0));
                                        sprintf(szBuffer,"%s,%d,%s",szOrdProg,nCollo,Cfg.szPrinterConsole);
                                        SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST_COLLO,szBuffer);
                                    }
                                }
                                DBclear(DBRes);
                            }
                        }
                    }
                    break;
                    case SETTORI_ORDINE_IN_PRELIEVO:
                    {
                        int nIsola=0,nSettore=0;
                        char szOrdProg[128]="";
                        char *pPtr;
                        char szBuffer[128];
                        char *szSeparator=",\t\n";

                        /* Isola        */ if((pPtr=strtok(szText,szSeparator))!=NULL){ nIsola=atoi(pPtr); }
                        /* Settore      */ if((pPtr=strtok(  NULL,szSeparator))!=NULL){ nSettore=atoi(pPtr); }
                        /* OrdProg       */ if((pPtr=strtok(  NULL,szSeparator))!=NULL){ strcpy(szOrdProg,pPtr); }

                        if(Cfg.nDebugLevel>1){
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Ordine in Prelievo [%d,%d,%s]", nIsola, nSettore, szOrdProg);
                        }

                        UpdateOrdine(win, szOrdProg);

                        if(Cfg.nStampaRAC==INIZIO_PRELIEVO){
                            /*
                            * stampa etichetta su inizio prelievo
                            */
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Stampa Modulo RAC Ordine [%s]",szOrdProg);

                            /*
                            * Spedizione messaggi di stampa etichette e packing list a printlabel
                            */
                            sprintf(szBuffer,"%s,%s",szOrdProg,Cfg.szLabelPrinterConsole);
                            SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_RAC,szBuffer);

                            if(Cfg.bStampaPackingList){
                                DBresult *DBRes;
                                int nIndex;
                                int nCollo;

                                /* stampo la packing list di tutti i colli appartenenti all'ordine */
                                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cpnmcol from col_prod where ordprog='%s';",szOrdProg);
                                if (DBresultStatus(DBRes)==DBRES_TUPLES_OK && DBntuples(DBRes)){
                                    for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                                        nCollo=atoi(DBgetvalue(DBRes,nIndex,0));
                                        sprintf(szBuffer,"%s,%d,%s",szOrdProg,nCollo,Cfg.szPrinterConsole);
                                        SendMessage(PROC_PRINTLABEL,PROC_MAIN,PRINT_PACKING_LIST_COLLO,szBuffer);
                                    }
                                }
                                DBclear(DBRes);
                            }
                        }
                    }
                    break;

                    case SETTORI_ORDINE_COLLO_PRELEVATO:
                    {
                        int nIsola=0,nSettore=0,nCollo=0;
                        char szOrdProg[128]="";
                        char  *pPtr;
                        char  *szSeparator=",\t\n";

                        /* Isola        */ if((pPtr=strtok(szText,szSeparator))!=NULL){ nIsola=atoi(pPtr); }
                        /* Settore      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nSettore=atoi(pPtr); }
                        /* OrdProg      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szOrdProg,pPtr); }
                        /* Collo        */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nCollo=atoi(pPtr); }

                        if(Cfg.nDebugLevel>1){
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"SETTORI_ORDINE_COLLO_PRELEVATO [%d,%d,%s,%d]",
                                nIsola, nSettore, szOrdProg,nCollo);
                        }
                    }
                    break;
                    case SETTORI_ORDINE_PRELEVATO:
                    {
                        int nIsola=0,nSettore=0;
                        char szOrdProg[128]="";
                        char  *pPtr;
                        char  *szSeparator=",\t\n";

                        /* Isola        */ if((pPtr=strtok(szText,szSeparator))!=NULL){ nIsola=atoi(pPtr); }
                        /* Settore      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nSettore=atoi(pPtr); }
                        /* OrdProg      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szOrdProg,pPtr); }

                        if(Cfg.nDebugLevel>1){
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"SETTORI_ORDINE_PRELEVATO [%d,%d,%s]", nIsola, nSettore, szOrdProg);
                        }

                        UpdateOrdine(win, szOrdProg);
                    }
                    break;
                    case SETTORI_ORDINE_SETT_SUCC:
                    {
                        int nIsola=0,nSettore=0;
                        char szOrdProg[128]="";
                        char  *pPtr;
                        char  *szSeparator=",\t\n";

                        /* Isola        */ if((pPtr=strtok(szText,szSeparator))!=NULL){ nIsola=atoi(pPtr); }
                        /* Settore      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ nSettore=atoi(pPtr); }
                        /* OrdProg      */ if((pPtr=strtok(NULL  ,szSeparator))!=NULL){ strcpy(szOrdProg,pPtr); }

                        if(Cfg.nDebugLevel>1){
                            trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"SETTORI_ORDINE_SETT_SUCC [%d,%d,%s]", nIsola, nSettore, szOrdProg);
                        }
                    }
                    break;
                }
            break;
            case PROC_PRINTLABEL:
                switch (InMsgStruct.code){
                    case PRINTLABEL_RAC_STAMPATO:
                    {
                        char szOrdProg[128];
                        char szStato[60];
                        char *pPtr;
                        char *szSeparator=",\t\n";
                        DBresult *DBRes;

                        /* Ordine */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szOrdProg,pPtr); }

                        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rostato from ric_ord where ordprog='%s';",szOrdProg);
                        if(DBntuples(DBRes)){
                            strcpy(szStato,DBgetvalue(DBRes,0,0));
                            /*
                            * Cambia lo stato in RAC solo se l'ordine e' elaborato
                            * cosi' si risolve il problema della stampa rac su prelievo e
                            * anche quello della evasione ordine
                            */
                            if(szStato[0]==ORDINE_ELABORATO){
                                CambiaStatoOrdine(win, szOrdProg, ORDINE_STAMPATA_RAC);
                                UpdateOrdine(win, szOrdProg);
                            }
                        }
                        DBclear(DBRes);
                    }
                    break;

                    case PRINTLABEL_XAB_STAMPATA:
                    {
                        UpdateOrdine(win, szText);
                    }
                    break;
                    case PRINTLABEL_XAB_LINEA_STAMPATA:
                    case PRINTLABEL_DISTINTA_STAMPATA:
                    {
                        char *pPtr;
                        char *szSeparator=",\t\n";
                        char szNMCED[128];
                        char szTPSPE[128];
                        char szCDLIN[128];
                        char szCDVET[128];
                        int nIndex;
                        DBresult *DBRes;

                        /* NMCED */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }
                        /* TPSPE */ if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szTPSPE,pPtr); }
                        /* CDLIN */ if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDLIN,pPtr); }
                        /* CDVET */ if((pPtr=strtok(NULL,szSeparator))!=NULL){ strcpy(szCDVET,pPtr); }

                        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s';",szNMCED,szTPSPE,szCDLIN,szCDVET);
                        for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                            UpdateOrdine(win, DBgetvalue(DBRes,nIndex,0));
                        }
                        DBclear(DBRes);
                    }
                    break;
                    case PRINTLABEL_ALL_RAC_STAMPATO:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Fine stampa etichette");
                    break;

                    case PRINTLABEL_RAC_ERROR:
                    case PRINTLABEL_RAC_COLLO_ERROR:
                    case PRINTLABEL_ALL_RAC_ERROR:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"ERRORE in Stampa Etichette [%s]",strlen(szText)?szText:"-");
                    break;
                    case PRINTLABEL_XAB_ERROR:
                    case PRINTLABEL_XAB_LINEA_ERROR:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"ERRORE in Stampa XAB [%s]",strlen(szText)?szText:"-");
                    break;
                    case PRINTLABEL_DISTINTA_ERROR:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"ERRORE in Stampa DISTINTA [%s]",strlen(szText)?szText:"-");
                    break;
                    case PRINTLABEL_DATI_ORDINE_STAMPATA:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Fine stampa dati ordine");
                    break;
                    case PRINTLABEL_DATI_ORDINE_ERROR:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"ERRORE in stampa dati ordine [%s]",strlen(szText)?szText:"-");
                    break;
                }
            break;
            /********************************************************************************************
            * INIZIO MESSAGGI DA AUTO_TASKS
            *********************************************************************************************/
            case PROC_AUTO_TASKS:
                switch (InMsgStruct.code){
                    case AUTO_TASKS_FILE_CORRIERE_CREATO:
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Creato file corriere %s",strlen(szText)?szText:"?");
                    break;
                    case AUTO_TASKS_FILE_CORRIERE_IN_CODA:
                    {
                        char *pPtr;
                        char *szSeparator=",\t\n";
                        char szNMCED[128];
                        char szTPSPE[128];
                        char szCDLIN[128];
                        char szCDVET[128];
                        char szNMDIS[128];
                        char szPRGEM[128];
                        /* NMCED */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }
                        /* TPSPE */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szTPSPE,pPtr); }
                        /* CDLIN */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szCDLIN,pPtr); }
                        /* CDVET */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szCDVET,pPtr); }
                        /* NMDIS */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szNMDIS,pPtr); }
                        /* PRGEM */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szPRGEM,pPtr); }
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"File corriere di linea [%s,%s,%s,%s]\n   collegati alla linea %s allegati alla mail con progressivo %s",szNMCED,szTPSPE,szCDLIN,szCDVET,szNMDIS,szPRGEM);
                    }
                    break;
                    case AUTO_TASKS_FILE_CORRIERE_SPEDITO:
                    {
                        DBresult *DBRes;
                        char *pPtr;
                        char *szSeparator=",\t\n";
                        char szPRGEM[128];
                        char szNMDIS[128];
                        int nIndex;
                        /* NMCED */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szPRGEM,pPtr); }
                        /* NMDIS */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szNMDIS,pPtr); }
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"Spedita mail %s con allegati relativi alla distinta %s",szPRGEM,szNMDIS);

                        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where roprgem=%s;",szPRGEM);
                        for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                            UpdateOrdine(win, DBgetvalue(DBRes,nIndex,0));
                        }
                        DBclear(DBRes);
                    }
                    break;
                    case AUTO_TASKS_FILE_CORRIERE_DA_NON_SPEDIRE:
                    {
                        DBresult *DBRes;
                        char *pPtr;
                        char *szSeparator=",\t\n";
                        char szNMCED[128];
                        char szTPSPE[128];
                        char szCDLIN[128];
                        char szCDVET[128];
                        int nIndex;
                        /* NMCED */ if((pPtr=strtok(szText,szSeparator))!=NULL){ strcpy(szNMCED,pPtr); }
                        /* TPSPE */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szTPSPE,pPtr); }
                        /* CDLIN */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szCDLIN,pPtr); }
                        /* CDVET */ if((pPtr=strtok(NULL,szSeparator))!=NULL)  { strcpy(szCDVET,pPtr); }
                        trace_debug_gtk("GREEN", TRUE, TRUE, priv->txt_msgs,"File corriere di linea [%s,%s,%s,%s]\n settato per non essere spedito",szNMCED,szTPSPE,szCDLIN,szCDVET);
                        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ordprog from ric_ord where ronmced='%s' and rotpspe='%s' and rocdlin='%s' and rocdve2='%s';",szNMCED,szTPSPE,szCDLIN,szCDVET);
                        for(nIndex=0;nIndex<DBntuples(DBRes);nIndex++){
                            UpdateOrdine(win, DBgetvalue(DBRes,nIndex,0));
                        }
                        DBclear(DBRes);
                    }
                    break;
                }
            break;
            /********************************************************************************************
            * FINE MESSAGGI DA AUTO_TASKS
            *********************************************************************************************/
        }
    }

    if(bFirstTime){
        bFirstTime=FALSE;
    }
    /* 
    * As this is a timeout function, return TRUE so that it
    * continues to get called 
    */
    return(TRUE);
}


gint StatisticaImpianto( gpointer win )
{

    char szSelectCmd[4096];

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GetFileString("Statistica Ordini","statistica_ordini", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    GList *PS=NULL;
    add_item_to_parse(&PS, "%ORDTIPO%",Cfg.szTipoOrdini, TRUE);
    int rc =  RefreshTable(find_child(GTK_WIDGET(win),"sw_impianto"), "lst_impianto", szSelectCmd, GTK_SELECTION_NONE, PS, NULL, NULL);
    return rc;
}
