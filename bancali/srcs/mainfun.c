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
#include <math.h>

#include <ep-common.h>
#include <ep-db.h>
#include <trace.h>
#include <pmx_msq.h>
#include <picking.h>

#include <gtk-support.h>
#include <msg-box.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"
#include "bilancia.h"


/*
* ReadBitmaskInfo(char *szFileConfig)
* Lettura dei parametri relativi allo stato del EV2002
*/
void ReadBitmaskInfo(char *szFileConfig)
{
    char szProcIndex[256];
    char szProcBuffer[256];
    int nBitStato=4;
    int nBit=4;
    int nIndexStato;
    int nIndexBit;

    for (nIndexStato=0; nIndexStato<nBitStato; nIndexStato++) {
        for (nIndexBit=0; nIndexBit<nBit; nIndexBit++) {
            sprintf(szProcIndex,"BitMask_%01d_%01d",nIndexStato, nIndexBit);
            GetFileString(ProcessGetName(PROC_BANCALI), szProcIndex, "", szProcBuffer, 250, szFileConfig, NULL);
            sscanf(szProcBuffer, "%[^,],%[^,],%d",
                    stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].szDes,
                    stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].szWidget,
                    &stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nValue);
            stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].nOldValue=0;
        }
    }
}


/*
* void ShowLabelBilancia()
* Assegna il titolo alle label corrispondenti ai segnali 
* di stato(bitmask) in dlg_bilancia 
*/
void ShowLabelBilancia(gpointer win)
{
    int nBitStato=4;
    int nBit=4;
    int nIndexStato;
    int nIndexBit;


    for (nIndexStato=0; nIndexStato<nBitStato; nIndexStato++) {
        for (nIndexBit=0; nIndexBit<nBit; nIndexBit++) {
            gtk_label_set_text(GTK_LABEL(find_child(GTK_WIDGET(win),stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].szWidget)),
            stringa_ev2002.stato_ev2002[nIndexStato][nIndexBit].szDes);
        }
    }
}


/*
* void Gestev_2002msgs(void)
*    gestisce il messaggio inviato da terminale ev_2002,
* riempie la struttura stringa_ev2002 ed aggiorna
* le label e pixmap in main_window
*/
void Gestev_2002msg(char *szText)
{
    char szChar[2];
    int nIndex;
    int nChar;
    int nVal;
    char szPeso[128];
    char szTara[128];
    char szDateBuffer[128];
    char szTimeBuffer[128];
    char *pPtr;

    /*
    * Gestione bitmask
    */
#ifdef TRACE_BILANCIA
    trace_debug(TRUE, TRUE, "Arrivato : [%s]",szText);
    trace_debug(FALSE, TRUE, "---BITMASK----");
#endif
    szChar[1]='\0';
    for(nIndex=0;nIndex<4;nIndex++){
#ifdef TRACE_BILANCIA
        trace_debug(FALSE, TRUE, "Carattere %d : ",nIndex+24);
#endif
        szChar[0]=szText[24+nIndex];
        sscanf(szChar,"%x",&nVal);
#ifdef TRACE_BILANCIA
        trace_debug(FALSE, FALSE, " = %d : ",nVal);
#endif
        for(nChar=0;nChar<4;nChar++){
            stringa_ev2002.stato_ev2002[nIndex][nChar].nValue=nVal&(1<<nChar)?1:0;
        }
#ifdef TRACE_BILANCIA
        for (nChar=3;nChar>=0;nChar--){
            trace_debug(FALSE, FALSE, "%d ",stringa_ev2002.stato_ev2002[nIndex][nChar].nValue);
        }
#endif
    }

    /*
    *    memorizzazione dati contenuti nel 
    * messaggio da ev_2002
    */
    sprintf(stringa_ev2002.time,"%s %s",
    GetDate( szDateBuffer),
    GetTime( szTimeBuffer));
    strcpy(stringa_ev2002.szMessaggio,szText);
    delete_new_line(stringa_ev2002.szMessaggio);
    strcpy(szPeso,SubStr(szText,1,9));
    if((pPtr=strchr(szPeso,','))){
        *pPtr='.';
    }
    stringa_ev2002.fPesonetto=atof(szPeso);

    strcpy(szTara,SubStr(szText,11,9));
    if((pPtr=strchr(szTara,','))){
        *pPtr='.';
    }
    stringa_ev2002.fTara=atof(szTara);
    strcpy(stringa_ev2002.szUnita_misura, SubStr(szText,21,2));
    
    /*
    * aggiorno il led di peso stabile in main_window
    */
    /*
     * TODO
    gtk_image_set_from_stock (GTK_IMAGE(get_widget(main_window,"pixmap_peso_stab")),stringa_ev2002.stato_ev2002[1][1].nValue?"gtk-yes":"gtk-no", GTK_ICON_SIZE_BUTTON);
    */


}

/*
* ep_bool_t ShowPesoBilancia(void)
* se collo selezionato (bColloPresente=TRUE) visualizza in main_window
* le informazioni relative
*/
ep_bool_t ShowPesoBilancia(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    static float fOldPeso=-1;
    static ep_bool_t bRetVal=TRUE;
    float fDiffPeso;    
    char szText[120];

    if (bColloPresente){
        fDiffPeso=fabs((float)((float)(Collo.nPeso+Collo.nTara)/(float)1000)-stringa_ev2002.fPesonetto);
        /*
        * aggiorno la label del peso in main_window
        */
        if (fOldPeso!=stringa_ev2002.fPesonetto){
            fOldPeso=stringa_ev2002.fPesonetto;
            gtk_label_printf(priv->lb_PESOREA,"%7.3f Kg",stringa_ev2002.fPesonetto);
        }
        /*
        * segnale di Peso valido e pesata non minima: visualizzo in lb_risultato il 
        * rispetto dell'eventuale tolleranza
        */
        if ((stringa_ev2002.stato_ev2002[1][1].nValue) && (!stringa_ev2002.stato_ev2002[0][0].nValue)){
            /*
            * Assegno il peso in grammi
            */
            Collo.nPesoReale=(int)(stringa_ev2002.fPesonetto*1000);
            sprintf(szText,"Pesato Collo %s - %d\n",Collo.szOrdProg,Collo.nCollo);
#ifdef TRACE_PESO
            trace_debug(TRUE, TRUE, "Pesato Collo %s - %d\n",Collo.szOrdProg,Collo.nCollo);
#endif
            gtk_label_printf(priv->lb_PESODIFF,"%7.3f Kg",fDiffPeso);
            if (Cfg.nTolleranza==TOLLERANZA_ASSOLUTA){
                /*
                * Tolleranza assoluta
                */
                gtk_label_printf(priv->lb_risultato, (fDiffPeso>Cfg.nTolleranzaAssoluta)?  "COLLO FUORI TOLLERANZA": "COLLO CONFORME");
#ifdef TRACE_PESO
                trace_debug(TRUE, TRUE, "%s",(fDiffPeso>Cfg.nTolleranzaAssoluta)?"COLLO FUORI TOLLERANZA (ASS)":"COLLO CONFORME");
#endif
                } else {
                /*
                * Tolleranza percentuale
                */
                gtk_label_printf(priv->lb_risultato, ((fDiffPeso*100)/stringa_ev2002.fPesonetto)>Cfg.nTolleranzaPercentuale?  "COLLO FUORI TOLLERANZA": "COLLO CONFORME");
#ifdef TRACE_PESO
                trace_debug(TRUE, TRUE, "%s",(fDiffPeso>Cfg.nTolleranzaAssoluta)?"COLLO FUORI TOLLERANZA (PERC)":"COLLO CONFORME");
#endif
            }
        }
    } else {
        /*
        * nessun collo e' stato selezionato
        */
    }
    return(bRetVal);
}

/*
* void ClearLabel(char *pszText)
* pulisce le label e la c_list della main_window
*/
void ClearLabel(gpointer win, char *pszText)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_entry_set_text(GTK_ENTRY(priv->entry_BOLLA) ,"");
    gtk_entry_set_text(GTK_ENTRY(priv->entry_COLLO) ,"");
    gtk_label_set_text(GTK_LABEL(priv->lb_CDRID)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_PRDOC)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_NMCOL)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_SWCOL)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_NMCPE)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_NMRGH)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_PSPRE)    ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_VOLUME)   ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_VOLUME_P) ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_VOL_UTI)  ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_VOL_P_UTI),NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_PESOCALC) ,NULL);

    gtk_label_set_text(GTK_LABEL(priv->lb_PESODIFF) ,NULL);
    gtk_label_set_text(GTK_LABEL(priv->lb_risultato),pszText);
    
    /*
    * disattivo i push_button di clear ed evadi in main_window
    */
    gtk_widget_set_sensitive(priv->pb_evadi,FALSE);
    gtk_widget_set_sensitive(priv->pb_clear,FALSE);
    gtk_widget_set_sensitive(priv->pb_stampa,FALSE);
}

char *GetStatoCollo(char cStato,char *pszBuffer)
{
    switch(cStato){
        case COLLO_ELABORATO:
            strcpy(pszBuffer,"ELABORATO");
        break;
        case COLLO_PRELIEVO:
            strcpy(pszBuffer,"IN PRELIEVO");
        break;
        case COLLO_CHIUSO:
            strcpy(pszBuffer,"CHIUSO");
        break;
        case COLLO_EVASO:
            strcpy(pszBuffer,"EVASO");
        break;
        default:
            strcpy(pszBuffer,"NON DEFINITO");
        break;
    }
    return pszBuffer;
}

ep_bool_t StampaPackingListCollo(gpointer win, char *szOrdProg, int nCollo, char *szPrinterName)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    FILE *fp;
    char szTitle[128];
    char szFileName[128];
    char szKey[128];
    ep_bool_t bOK=TRUE;
    DBresult *DBRes;

    /*
    * Apertura del file per la stampa
    */
    sprintf(szFileName,"%s/PL_%s",Cfg.szPathStampe,szOrdProg);
    if ((fp=fopen(szFileName,"w"))!=(FILE *)NULL) {
        DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rodscli,roincli,rocpcli,rolocli,roprcli,ronmcll,ronmrgh,ronmcpe,ropspre from ric_ord where ordprog='%s';",szOrdProg);

        if(DBresultStatus(DBRes)==DBRES_TUPLES_OK && (DBntuples(DBRes)==1)){
            fprintf(fp,"Cliente : %s\n",DBgetvalue(DBRes,0,0));
            fprintf(fp,"          %s\n",DBgetvalue(DBRes,0,1));
            fprintf(fp,"          %s - %s %s\n",DBgetvalue(DBRes,0,2),DBgetvalue(DBRes,0,3),DBgetvalue(DBRes,0,4));
            fprintf(fp,"%s  Colli : %d  Righe : %3d  Copie : %4d\n",szOrdProg, atoi(DBgetvalue(DBRes,0,5)), atoi(DBgetvalue(DBRes,0,6)), atoi(DBgetvalue(DBRes,0,7)));
        } else {
#ifdef TRACE
            bOK=FALSE;
#endif
        }
        DBclear(DBRes);

        fprintf(fp,"Lista righe collo %d\n",nCollo);

        sprintf(szKey,"r.ordprog='%s' AND r.rpnmcol=%d",szOrdProg,nCollo);

        GList *PS=NULL;
        add_item_to_parse(&PS, "%ORDTIPO%", Cfg.szTipoOrdini, TRUE);
        add_item_to_parse(&PS, "%KEY%",     szKey,            FALSE);
        PrintTable(priv->pszCfgFileName, "bancali","righe_collo",NULL,NULL, PS,fp, Cfg.nTipoStampe);
        // PrintTable(priv->pszCfgFileName, "check_bancali","lista_bancali_spedizione",NULL,NULL, PS, fp, Cfg.nTipoStampe);

        /*
        * Salto pagina
        */
        fprintf(fp,"");

        fclose(fp);

        sprintf(szTitle,"A&L - Stampa Collo %s - %d",szOrdProg,nCollo);
        PrintFile(szFileName,szPrinterName,szTitle,Cfg.nA2ps,0);
    }
    return bOK;
}

