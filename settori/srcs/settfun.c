/*
* settfun.c : gestione settori di magazzino : funzioni di gestione
* 
* Progetto Easy Picking 4.0 : wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
//#include <termio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <glib.h>

#include <gio/gio.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#include <trace.h>
#include <proc_list.h>
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>

#include "linklist.h"
#include "settstruct.h"
#include "settext.h"
#include "settfun.h"


unsigned int nGetOrderTimePassed[MAX_SETTORI];
struct timeval tvOldTime[MAX_SETTORI];

/*
* void ReadConfiguration(void)
* lettura della configurazione dal file cni.cfg
*/
void ReadConfiguration(ep_bool_t bReadProcInfo)
{
    char szParagraph[128];
    char szCurrentDirectory[128];
    char szProcIndex[128];
    char szProcBuffer[128];
    char szCfgFileName[128];
    int nIndex;


    /*
    * leggo il file di configurazione dalla directory corrente
    */
    getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
    
    strcpy(szCfgFileName,szCurrentDirectory);
    strcat(szCfgFileName,"/");
    strcat(szCfgFileName, __configuration_file__);

    strcpy(szParagraph,"General Settings");
    GetFileString(szParagraph,"TipoOrdini", "X",           Cfg.szTipoOrdini,   80, szCfgFileName, NULL);
    GetFileString(szParagraph,"PathData",   "../data",     Cfg.szPathData,     80, szCfgFileName, NULL);
    GetFileString(szParagraph,"PathExport", "../export",   Cfg.szPathExport,   80, szCfgFileName, NULL);
    GetFileString(szParagraph,"PathExe",    "./",          Cfg.szPathExe,      80, szCfgFileName, NULL);
    GetFileString(szParagraph,"PathTrace",  "../trace",    Cfg.szPathTrace,    80, szCfgFileName, NULL);

    Cfg.nShmKey            = GetFileInt(szParagraph,"ShmKey",         256, szCfgFileName,NULL);
    Cfg.nSettoriShmKey     = GetFileInt(szParagraph,"SettoriShmKey",  257, szCfgFileName,NULL);
    Cfg.nStampaRAC         = GetFileInt(szParagraph,"StampaRAC",        0, szCfgFileName,NULL);
    Cfg.nImballiUbicati    = GetFileInt(szParagraph,"ImballiUbicati",   0, szCfgFileName,NULL);
    Cfg.nChiusuraCollo     = GetFileInt(szParagraph,"ChiusuraCollo",    1, szCfgFileName,NULL);
    Cfg.nFineOrdine        = GetFileInt(szParagraph,"FineOrdine",       1, szCfgFileName,NULL);
    Cfg.nDebugLevel        = GetFileInt(szParagraph,"DebugLevel",     0, szCfgFileName,NULL);
    Cfg.bRifornimenti      = GetFileInt(szParagraph,"Rifornimenti",     0, szCfgFileName, NULL);

    if(bReadProcInfo){
        ReadProcInfo(szCfgFileName);
    }

    /*
    * parametri specifici dell'applicazione
    */
    /* Delay in millisecondi */
    Cfg.nDelay          = GetFileInt(ProcessGetName(PROC_SETTORI),"Delay",          100, szCfgFileName,NULL);
    Cfg.nGetOrderDelay  = GetFileInt(ProcessGetName(PROC_SETTORI),"GetOrderDelay", 1000, szCfgFileName,NULL);

    Cfg.nNumeroIsole    = GetFileInt(ProcessGetName(PROC_SETTORI),"NumeroIsole",      1, szCfgFileName,NULL);
    Cfg.nNumeroSettori  = GetFileInt(ProcessGetName(PROC_SETTORI),"NumeroSettori",   12, szCfgFileName,NULL);

    /* Numero di errori IOS */
    Cfg.nMaxDamages     = GetFileInt(ProcessGetName(PROC_SETTORI),"MaxDamages",       5, szCfgFileName,NULL);

    /* Numero dei moduli PKL */
    Cfg.nPKLNumber      = GetFileInt(ProcessGetName(PROC_IOS),"PKLNumber",            1, szCfgFileName,NULL);

    /*
    * dati relativi ai moduli PKL
    */
    for(nIndex=0; nIndex < Cfg.nPKLNumber; nIndex++){
        sprintf(szProcIndex,"PKL_%02d",nIndex);
        GetFileString(ProcessGetName(PROC_IOS),szProcIndex, "", szProcBuffer, 80,szCfgFileName,NULL);

        sscanf(szProcBuffer, "%d,%d,%d",
            &(Cfg.PKL[nIndex].nIOS),
            &(Cfg.PKL[nIndex].nCPU),
            &(Cfg.PKL[nIndex].nModulo));
    }
    /*
    * lettura dei dati di configurazione del DataBase (PostgreSQL)
    */
    strcpy(szParagraph,"DataBase Settings");

	GetFileString(szParagraph,"DBHost",     "localhost", Cfg.szDBHost,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBPort",     "5432",      Cfg.szDBPort,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBName",     "momo",      Cfg.szDBName,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBUser",     "user",      Cfg.szDBUser,     80,szCfgFileName,NULL); 
	GetFileString(szParagraph,"DBPassword", "pwd",       Cfg.szDBPassword, 80,szCfgFileName,NULL); 
}


/*
* cerca in archivio operativo ordini il primo ordine da prelevare 
* Se StampaRAC = 0 : in stato 'R' : Stampata RAC
* Se StampaRAC = 1 : in stato 'A' : Elaborato
*/
ep_bool_t GetOrdine(PORDINE pOrdine,int nIsola,int nCedola)
{
    DBresult *DBRes = NULL;
    int nTuples;
    char cStatoOrdine=0xFF;
    char szBuffer[80];
    ep_bool_t bFound=FALSE;
    ep_bool_t bProducibile=FALSE;
    int nIndex;
    int nColliAutomatici;

    switch(Cfg.nStampaRAC){
        case DOPO_IL_LANCIO:
            cStatoOrdine = ORDINE_STAMPATA_RAC;
        break;
        case INIZIO_PRELIEVO:
            cStatoOrdine = ORDINE_ELABORATO;
        break;
    }

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rostato,ordprog,ronmcll,ronmrgp,ronmcpp,rocdflg,ronmcla from ric_ord where rostato='%c' AND ronmced='%d' AND rocdflg!='%c' and ordtipo='%s' order by roprgln,ordprog;",cStatoOrdine,nCedola,ORDINE_BLOCCATO,Cfg.szTipoOrdini);

    nTuples=DBntuples(DBRes);
#ifdef TRACE_ELIMINATO
    trace_debug(TRUE, TRUE, "GetOrdine(%d) : Trovati %d ordini",nIsola,nTuples);
#endif

    /*
    * A questo punto ho ottenuto 0 o piu' ordini di produzione in stato 'A' : Analizzati, 
    * relativi all'isola di magazzino in oggetto e con almeno un collo di produzione
    */
    nIndex=0;
    while(nIndex<nTuples && !bFound){
        nColliAutomatici=atoi(DBgetvalue(DBRes,nIndex,6));
        if(nColliAutomatici){
            strcpy(pOrdine->szOrdProg,DBgetvalue(DBRes,nIndex,1));
            pOrdine->nColli=atoi(DBgetvalue(DBRes,nIndex,2));
            pOrdine->nRighe=atoi(DBgetvalue(DBRes,nIndex,3));
            pOrdine->nCopie=atoi(DBgetvalue(DBRes,nIndex,4));
            strcpy(szBuffer,DBgetvalue(DBRes,nIndex,5)); pOrdine->cFlag=szBuffer[0];
            bFound=TRUE;
            continue;
        } else {
            char szMsg[128];
            /*
            * rm 14-11-2001 : prima verifico la producibilita' !
            */
            strcpy(pOrdine->szOrdProg,DBgetvalue(DBRes,nIndex,1));
            pOrdine->nColli=atoi(DBgetvalue(DBRes,nIndex,2));
            pOrdine->nRighe=atoi(DBgetvalue(DBRes,nIndex,3));
            pOrdine->nCopie=atoi(DBgetvalue(DBRes,nIndex,4));
            strcpy(szBuffer,DBgetvalue(DBRes,nIndex,5)); pOrdine->cFlag=szBuffer[0];

            if(!OrdineProducibile(pOrdine)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "GetOrdine : Ordine MANUALE [%s] NON PRODUCIBILE", pOrdine->szOrdProg);
#endif
                sprintf(szMsg,"ATTENZIONE : Ordine MANUALE [%s] NON PRODUCIBILE\n", pOrdine->szOrdProg);
                SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
                SendMessage(PROC_MAIN, PROC_SETTORI, REFRESH_ORDINE, pOrdine->szOrdProg);
                bProducibile=FALSE;
            } else {
                /*
                * Se l'ordine e' producibile allora impegno le copie 
                * (verranno evasi da main (EvadiOrdine())
                */
                ProduzioneOrdine(pOrdine);
                bProducibile=TRUE;
            }

            if(bProducibile){
                /*
                * Messaggio di Ordine in prelievo (fasullo) a MAIN
                * per fare scattare la gestione stampe su ordine in prelievo
                */
                sprintf(szMsg,"ATTENZIONE : Ordine [%s] senza colli di linea : EVASIONE AUTOMATICA\n",DBgetvalue(DBRes,nIndex,1)); 
                SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Ordine [%s] EVASO : nessun collo di linea automatica", DBgetvalue(DBRes,nIndex,1));
#endif
                CambiaStatoOrdine(DBgetvalue(DBRes,nIndex,1), ORDINE_IN_PRELIEVO);
                SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, DBgetvalue(DBRes,nIndex,1));
            }
        }
        nIndex++;
    }

    DBclear(DBRes);

    return(bFound);
}

/*
* 10-12-2001 rm
* Verifico forzatura ordine
*/
ep_bool_t OrdineForzato(PORDINE pOrdine)
{

    if(pOrdine->cFlag==ORDINE_FORZATO){
        /*
        * L'ordine e' forzato
        */
        return TRUE;
    }
    return FALSE;
}

/*
* 30-08-2001 rm
* Problema : se le copie richieste sono 180 e la qt.max e' 30 come faccio ??????
* l'ordine rimane bloccato a vita !!!
*/
ep_bool_t OrdineProducibile(PORDINE pOrdine)
{
    DBresult *DBRes = NULL;
    DBresult *DBResUbi = NULL;
    DBresult *DBResRighe = NULL;
    int nTuples;
    char szCodProd[128];
    char szUbicazione[128];
    int nQTRic;    /* Quantita' Richiesta per il prodotto */
    int nQTCas;    /* Quantita' presente in ubicazione */
    int nQTImp;    /* Quantita' impegnata in ubicazione */
    ep_bool_t bProducibile=TRUE;
    ep_bool_t bRetValue=TRUE;
    int nIndex;

    if(!Cfg.bRifornimenti){
        return TRUE;
    }

    /*
    * Si ragiona per righe di produzione
    */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rpcdpro,sum(rpqtspe) from rig_prod where ordprog='%s' group by rpcdpro;",pOrdine->szOrdProg);

    if (DBRes && DBresultStatus(DBRes) == DBRES_TUPLES_OK ) {
        if((nTuples=DBntuples(DBRes))){
            for(nIndex=0;nIndex<nTuples;nIndex++){
                /*
                * Verifico producibilita' riga
                */
                strcpy(szCodProd,DBgetvalue(DBRes,nIndex,0));
                nQTRic=atoi(DBgetvalue(DBRes,nIndex,1));

                /*
                * verifico giacenza e impegnato
                */
                DBResUbi=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione,ubqtcas,ubqtimp from ubicazioni where codprod='%s' and ubitipo='%s' order by priorita;",szCodProd,Cfg.szTipoOrdini);
                if(DBntuples(DBResUbi)){
                    strcpy(szUbicazione,DBgetvalue(DBResUbi,0,0));
                    nQTCas=atoi(DBgetvalue(DBResUbi,0,1));
                    nQTImp=atoi(DBgetvalue(DBResUbi,0,2));

                    /*
                    * Controllo giacenze
                    */

                    if(nQTRic > nQTCas-nQTImp){
                        /*
                        * Se almeno una riga non e' producibile allora 
                        * significa che l'ordine non e' producibile 
                        */
                        bProducibile=FALSE;

                        /*
                        * Marco la riga ricevuta come bloccata / bloccante
                        */
                        DBResRighe=DBExecQuery(Cfg.nDebugLevel>1,"update ric_art set racdflg='%c' where ordprog='%s' and racdpro='%s';",RIGA_BLOCCATA,pOrdine->szOrdProg,szCodProd);
                        DBclear(DBResRighe);
                    }
                }
                DBclear(DBResUbi);
            }
        }
    } else {
        bRetValue=FALSE;
    }

    DBclear(DBRes);

    if(bRetValue){
        if(!bProducibile){
            /*
            * cambio flg
            */
            CambiaFlagOrdine(pOrdine->szOrdProg, ORDINE_BLOCCATO);
        }
    }

    return bRetValue ? bProducibile : bRetValue;
}

ep_bool_t ProduzioneOrdine(PORDINE pOrdine)
{
    DBresult *DBRes = NULL;
    DBresult *DBResImp = NULL;
    int nTuples;
    char szBuffer[128];
    char cFlag;
    char szCodProd[128];
    char szUbicazione[128];
    int nQTRic;    /* Quantita' Richiesta per il prodotto */
    int nQTCas;    /* Quantita' presente in ubicazione */
    int nQTImp;    /* Quantita' impegnata in ubicazione */
    int nCollo;
    int nProgRiga;
    ep_bool_t bRetValue=TRUE;
    int nIndex;

    if(!Cfg.bRifornimenti){
        return TRUE;
    }

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"begin work;");
    DBclear(DBRes);

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rpcdpro,rpcdubi,rpnmcol,rpprrig,rpqtord,ubqtcas,ubqtimp,rpcdflg from rig_prod,ubicazioni where rpcdubi=ubicazione and ordprog='%s';",pOrdine->szOrdProg);
    if((nTuples=DBntuples(DBRes))){
        for(nIndex=0;nIndex<nTuples;nIndex++){

            strcpy(szCodProd,DBgetvalue(DBRes,nIndex,0));
            strcpy(szUbicazione,DBgetvalue(DBRes,nIndex,1));

            nCollo=atoi(DBgetvalue(DBRes,nIndex,2));
            nProgRiga=atoi(DBgetvalue(DBRes,nIndex,3));
            nQTRic=atoi(DBgetvalue(DBRes,nIndex,4));
            nQTCas=atoi(DBgetvalue(DBRes,nIndex,5));
            nQTImp=atoi(DBgetvalue(DBRes,nIndex,6));
            strcpy(szBuffer,DBgetvalue(DBRes,nIndex,7)); cFlag=szBuffer[0];

            /*
            * aggiorno l'impegnato della ubicazione
            * rm 29-01-02 : modificata la gestione della evasione righe per evitare scritture non aggiornate
            */
            DBResImp=DBExecQuery(Cfg.nDebugLevel>1,"update ubicazioni set ubqtimp=ubqtimp+%d where ubicazione='%s';",nQTRic,szUbicazione);
            if(atoi(DBcmdTuples(DBResImp))==0){
                bRetValue=FALSE;
            }
            DBclear(DBResImp);
            DBResImp=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpcdflg='%c' where ordprog='%s' and rpnmcol=%d and rpprrig=%d;",RIGA_IMPEGNATA,pOrdine->szOrdProg,nCollo,nProgRiga);
            if(atoi(DBcmdTuples(DBResImp))==0){
                bRetValue=FALSE;
            }
            DBclear(DBResImp);

        }
    } else {
        bRetValue=FALSE;
    }

    DBclear(DBRes);
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"commit work;");
    DBclear(DBRes);

    return bRetValue;
}


/*
* Operazioni di inizio prelievo nel settore
* ricerca di un ordine in attesa di prelievo e sua immissione in linea
*/
ep_bool_t InizioPrelievoSettore(PORDINE pOrdine,PSETTORE pSettore)
{
    char szMsgTxt[128];
    DBresult *DBRes;
    PDISPLAYSTRUCT pDisplay;
    int nTuples;
    int nIndex;
    int nRigaIndex;
    char szMsg[128];

    /*
    * comincio ad elaborare l'ordine :
    * esistono righe di prelievo in ubicazioni automatiche e in colli automatici ?
    *   SI :
    *      Esistono prodotti da prelevare in questo settore ?
    *      SI : 
    *        Carico tutte le righe dell'ordine relative al settore in esame 
    *      NO : 
    *        Passo ordine a settore successivo
    *   NO :
    *      Chiudo il collo e l'ordine
    */
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select r.ordprog,r.rpcdpro,r.rpcdubi,r.rpqtspe,r.rpprrig,r.rpnmcol,u.ios,u.cpu,u.modulo,u.riga,u.colonna,u.display,u.settore,c.cpswlin from rig_prod as r, ubicazioni as u, col_prod as c where r.ordprog='%s' and r.ordprog=c.ordprog and r.rpnmcol=c.cpnmcol and u.ubicazione=r.rpcdubi and u.isola=%d and r.rpcdflg not in ('%c','%c') and u.ubitipo='%s' and c.cpswlin='%d' order by u.priorita,r.rpnmcol,r.rpprrig;",
        pOrdine->szOrdProg,
        pSettore->nIsola,
        RIGA_TAGLIATA,
        RIGA_NON_UBICATA,
        Cfg.szTipoOrdini,
        COLLO_AUTOMATICO);

    if (!DBRes || DBresultStatus(DBRes) != DBRES_TUPLES_OK ) {
        /* errore */
#ifdef TRACE
        trace_debug(TRUE, TRUE, "ATTENZIONE : ERRORE IN RICERCA UBICAZIONI ORDINE [%s]",pOrdine->szOrdProg);
#endif
        sprintf(szMsg,"ATTENZIONE : ERRORE IN RICERCA UBICAZIONI ORDINE [%s]\n", pOrdine->szOrdProg);
        SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
    } else {

        nTuples=DBntuples(DBRes);

        if(nTuples){
            /* 
            * ci sono righe di ubicazione automatica 
            */

            /*
            * Resetto il numero copie
            */
            nIndex=0;
            while(nIndex<nTuples){
                pSettore->RigaOrdine[nIndex].nNumCopie = 0;
                nIndex++;
            }
            /*
            * A questo punto ho ottenuto 0 o piu' righe di produzione 
            * relativi all'isola di magazzino ed al settore in oggetto 
            */
            nIndex=0;
            nRigaIndex=0;
            while(nIndex<nTuples){
                /*
                * Se si tratta di una riga relativa al settore in esame ...
                */
                if(pSettore->nSettore==atoi(DBgetvalue(DBRes,nIndex,12))){
                    strcpy(pSettore->RigaOrdine[nRigaIndex].szCodSped, DBgetvalue(DBRes,nIndex,0));
                    strcpy(pSettore->RigaOrdine[nRigaIndex].szCodProd, DBgetvalue(DBRes,nIndex,1));
                    strcpy(pSettore->RigaOrdine[nRigaIndex].szCodUbi,  DBgetvalue(DBRes,nIndex,2));
                    if(Cfg.nChiusuraCollo){
                        pSettore->RigaOrdine[nRigaIndex].nNumCopie         = atoi(DBgetvalue(DBRes,nIndex,3));
                        pSettore->RigaOrdine[nRigaIndex].nProgressivo      = atoi(DBgetvalue(DBRes,nIndex,4));
                        /*
                        * prelievi differenziati per collo
                        */
                        pSettore->RigaOrdine[nRigaIndex].nNumCollo         = atoi(DBgetvalue(DBRes,nIndex,5));
                    } else {
                        pSettore->RigaOrdine[nRigaIndex].nNumCopie        += atoi(DBgetvalue(DBRes,nIndex,3));
                        pSettore->RigaOrdine[nRigaIndex].nProgressivo      = atoi(DBgetvalue(DBRes,nIndex,4));
                        /*
                        * prelievi non differenziati per collo
                        */
                        pSettore->RigaOrdine[nRigaIndex].nNumCollo         = 1;
                    }
                    pSettore->RigaOrdine[nRigaIndex].nIOS              = atoi(DBgetvalue(DBRes,nIndex,6));
                    pSettore->RigaOrdine[nRigaIndex].nCPU              = atoi(DBgetvalue(DBRes,nIndex,7));
                    pSettore->RigaOrdine[nRigaIndex].nModulo           = atoi(DBgetvalue(DBRes,nIndex,8));
                    pSettore->RigaOrdine[nRigaIndex].nRiga             = atoi(DBgetvalue(DBRes,nIndex,9));
                    pSettore->RigaOrdine[nRigaIndex].nColonna          = atoi(DBgetvalue(DBRes,nIndex,10));
                    pSettore->RigaOrdine[nRigaIndex].nDisplay          = atoi(DBgetvalue(DBRes,nIndex,11));
                    pSettore->RigaOrdine[nRigaIndex].nIsola            = pSettore->nIsola;
                    pSettore->RigaOrdine[nRigaIndex].nStato=0;

                    /*
                    * Assegnazione valore prossimo collo a scopo chiusura collo prima del passaggio a 
                    * settore successivo
                    */
                    if(Cfg.nChiusuraCollo){
                        /*
                        * Gestione Collo
                        */
                        if(nIndex+1<nTuples){
                            pSettore->RigaOrdine[nRigaIndex].nNextCollo = atoi(DBgetvalue(DBRes,nIndex+1,5));
                        } else {
                            pSettore->RigaOrdine[nRigaIndex].nNextCollo = -1;
                        }
                    } else {
                        /*
                        * Gestione Ordine
                        */
                        if(nIndex+1<nTuples){
                            pSettore->RigaOrdine[nRigaIndex].nNextCollo = 1;
                        } else {
                            pSettore->RigaOrdine[nRigaIndex].nNextCollo = -1;
                        }
                    }

                    if(Cfg.nChiusuraCollo){
                        nRigaIndex++;
                    } else {
                        if(nIndex+1<nTuples){
                            if(strcmp(pSettore->RigaOrdine[nRigaIndex].szCodProd, DBgetvalue(DBRes,nIndex+1,1))){
                                nRigaIndex++;
                            }
                        } else {
                            nRigaIndex++;
                        }
                    }
                }
                nIndex++;
            }

            DBclear(DBRes);

            pSettore->nIndiceRigaOrdine=0;         /* puntatore alla riga d'ordine da prelevare */
            pSettore->nNumeroRigheOrdine=nRigaIndex;   /* numero delle righe d'ordine da prelevare */
            /*
            * se ci sono righe d'ordine da prelevare in questo settore
            * comincio dalla prima
            * altrimenti passo al prossimo settore
            */
            if(nRigaIndex){
                PRIGA_PRELIEVO pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);

                /* 
                * setto lo stato della riga (ATTESA_PRELIEVO)
                */
                pRigaOrdine->nStato=ATTESA_PRELIEVO;

                /*
                * setto i dati relativi alla riga d'ordine corrente del settore
                */
                strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
                strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
                strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
                pSettore->nNumCollo=pRigaOrdine->nNumCollo;
                pSettore->nNumCopie=pRigaOrdine->nNumCopie;
                pSettore->nIOS=pRigaOrdine->nIOS;
                pSettore->nCPU=pRigaOrdine->nCPU;
                pSettore->nModulo=pRigaOrdine->nModulo;
                pSettore->nRiga=pRigaOrdine->nRiga;
                pSettore->nColonna=pRigaOrdine->nColonna;

                pSettore->nDisplay=GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);

                pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

                /*
                * scrivo i dati per il prelievo sul display
                */
                /*
                * OrdProg : OOOOOO
                * O : Ordine
                * si visualizza : OOOO (Ultime 4 cifre)
                */
                strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
                pDisplay->nStatoRiga1=NORMAL;
                pDisplay->nStatoRiga2=NORMAL;

                UpdateDisplay(pDisplay,TUTTO);

                /*
                * acccendo la luce di prelievo prodotto
                */
                pSettore->nStatoLampada=1;
                SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);
                /*
                * setto lo stato settore in attesa di conferma copie prelevate
                */
                pSettore->nStatoSettore=PRELIEVO_COPIE;
            } else {
                /*
                * non ci sono righe per questo settore
                * scrivo i dati per il passaggio sul display
                */
                pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

                strcpy(pSettore->szCodSped,pOrdine->szOrdProg);
                strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                strcpy(pDisplay->szRiga_2_Display,"PPPP");
                pDisplay->nStatoRiga1=NORMAL;
                pDisplay->nStatoRiga2=BLINK;
                UpdateDisplay(pDisplay,TUTTO);

                /*
                * faccio confermare l'ultima lampada premuta 
                */
                pSettore->nStatoLampada=1;

                SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                pSettore->nStatoSettore=PASSA_SETT_SUCC;
            }
        } else {
            /* non ci sono righe di ubicazione automatica : chiudo l'ordine */
            /*
            * scrivo i dati per il fine ordine sul display
            */
            strcpy(pSettore->szCodSped,pOrdine->szOrdProg);

            pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
            strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
            strcpy(pDisplay->szRiga_2_Display,"FFFF");
            pDisplay->nStatoRiga1=NORMAL;
            pDisplay->nStatoRiga2=BLINK;
            UpdateDisplay(pDisplay,TUTTO);

            /*
            * faccio confermare l'ultima lampada premuta 
            */
            pSettore->nStatoLampada=1;

            SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

            pSettore->nStatoSettore=FINE_ORDINE;
            /*
            * Messaggio di Ordine Terminato a MAIN
            */
            CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_PRELEVATO);
            /*
            * Messaggio di Ordine terminato a MAIN
            */
            sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
            SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, szMsgTxt);
        }
    }
    return TRUE;
}


/*
* GestioneSettore()
* Gestione settore di magazzino
* logica di funzionamento del sequenziatore 
* si tratta di un automa a stati finiti
*/
ep_bool_t GestioneSettore(EVENTO *pEvento)
{
    static ep_bool_t bProducibile=TRUE;
    ORDINE Ordine;
    PDISPLAYSTRUCT pDisplay;
    PORDINE pOrdine;
    DBresult *DBRes = NULL;
    int nTuples;
    char szMsgTxt[128];
    char szMsg[128];
    int nIndex;
    int nSettoreIndex;
    PSETTORE pSettore;
    struct timeval tvActualTime;


    if((nSettoreIndex=GetSettore(pEvento->nIsola,pEvento->nSettore))==-1){
#ifdef TRACE
        trace_debug(TRUE, TRUE, "GestioneSettore(I:%d,S:%d) : settore non presente !",pEvento->nIsola,pEvento->nSettore);
#endif
        return FALSE;
    }
    pSettore=&pSettori[nSettoreIndex];

#ifdef TRACE_ELIMINATO
    trace_debug(TRUE, TRUE, "GestioneSettore(I:%d,S:%d) : settore %d",pEvento->nIsola,pEvento->nSettore,nSettoreIndex);
#endif
    /*
    * caso di settore in stato di attesa 
    * e nessun evento verificatosi
    */
    if(pSettore->nStatoSettore==ATTESA && pEvento->nEvento==-1){
        /*
        * - se si tratta del primo settore, 
        * - se la lista ordini e' vuota,
        * - se l'archivio rig_prod contiene prelievi di righe sul primo settore :
        * si mettono queste righe d'ordine in lista  
        */
        if(pSettore->nTipoSettore==SETTORE_START){
            if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))==NULL){
                /*
                * Gestione Timeout per get ordine (evita di ricercare un ordine sul settore di start con 
                * 'troppa' insistenza).
                */
                gettimeofday(&tvActualTime,NULL);
                /* calcolo il tempo trascorso dall'ultimo tentativo */
                nGetOrderTimePassed[pSettore->nSettore]=(((tvActualTime.tv_sec-tvOldTime[pSettore->nSettore].tv_sec)*1000) + ((tvActualTime.tv_usec-tvOldTime[pSettore->nSettore].tv_usec)/1000));   /* milli-secondi */
#ifdef TRACE_ELIMINATO
                    trace_debug(TRUE, TRUE, "Settore : %d Isola : %d : nGetOrderTimePassed = %6d",pSettore->nSettore,pSettore->nIsola,nGetOrderTimePassed[pSettore->nSettore]);
#endif
                /*
                * Verifico se e' passato il timeout per la ricerca dell'ordine da mandare sul settore di start
                * se l'ordine precedente non era producibile
                * passo SUBITO al prossimo ...
                */
                if(bProducibile==FALSE || nGetOrderTimePassed[pSettore->nSettore] > Cfg.nGetOrderDelay){
#ifdef TRACE_ELIMINATO
                    trace_debug(TRUE, TRUE, "Settore : %d Isola : %d : Azzero nGetOrderTimePassed (%d)",pSettore->nSettore,pSettore->nIsola,nGetOrderTimePassed[pSettore->nSettore]);
#endif
                    nGetOrderTimePassed[pSettore->nSettore]=0;
                    gettimeofday(&(tvOldTime[pSettore->nSettore]),NULL);
                }
                if(nGetOrderTimePassed[pSettore->nSettore]==0){
                    /* 
                    * se non ci sono ordini forzati in attesa di analisi
                    */
                    if(!Cfg.bAttesaAnalisiVolumetrica){
                        /* 
                        * Cerco un ordine in attesa di prelievo
                        */
                        if(GetOrdine(&Ordine,pSettore->nIsola,pSettore->nCedola)){
                            /* 
                            * se ordine forzato rilancio la analisi volumetrica
                            */
                            if(OrdineForzato(&Ordine)){
                                SendMessage(PROC_VOLUM, PROC_SETTORI, ANALISI_ORDINE, Ordine.szOrdProg);
                                /* 
                                * Attesa di analisi volumetrica ordine 
                                */
                                Cfg.bAttesaAnalisiVolumetrica=TRUE;
                            }

                            if(!Cfg.bAttesaAnalisiVolumetrica){
                                /*
                                * rm 29-08-2001 
                                * Verifico che l'ordine sia producibile
                                * o che sia 'forzato'  :  da gestire
                                * o che sia 'bloccato' :  da gestire
                                */
                                if(!OrdineProducibile(&Ordine)){
#ifdef TRACE
                                    trace_debug(TRUE, TRUE, "Ordine [%s] NON PRODUCIBILE", Ordine.szOrdProg);
#endif
                                    sprintf(szMsg,"ATTENZIONE : Ordine [%s] NON PRODUCIBILE\n", Ordine.szOrdProg);
                                    SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
                                    SendMessage(PROC_MAIN, PROC_SETTORI, REFRESH_ORDINE, Ordine.szOrdProg);
                                    bProducibile=FALSE;
                                } else {
                                    /*
                                    * Se l'ordine e' producibile allora impegno le copie 
                                    */
                                    ProduzioneOrdine(&Ordine);
                                    bProducibile=TRUE;
                                }

                                if(bProducibile){
#ifdef TRACE
                                    trace_debug(TRUE, TRUE, "Inizio Prelievo Ordine [%s] su Isola %d", Ordine.szOrdProg,pSettore->nIsola);
#endif
                                    if(Ordine.nRighe==0 || Ordine.nCopie==0){
#ifdef TRACE
                                        trace_debug(TRUE, TRUE, "Ordine [%s] VUOTO", Ordine.szOrdProg);
#endif
                                        sprintf(szMsg,"ATTENZIONE : Ordine [%s] VUOTO : EVASIONE AUTOMATICA\n", Ordine.szOrdProg);
                                        SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);

                                        /*
                                        * Messaggio di Ordine in evaso a MAIN
                                        */
                                        SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_EVASO, Ordine.szOrdProg);
                                    } else {
                                        /*
                                        * alloco un nuovo ordine da prelevare
                                        */
                                        pOrdine=(PORDINE)malloc(sizeof(ORDINE));
                                        memset(pOrdine,0,sizeof(ORDINE));

                                        strcpy(pOrdine->szOrdProg,Ordine.szOrdProg);   /* Chiave Spedizione */
                                        pOrdine->nIsola=Ordine.nIsola;
                                        pOrdine->nColli=Ordine.nColli;
                                        pOrdine->nRighe=Ordine.nRighe;
                                        pOrdine->nCopie=Ordine.nCopie;

                                        CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_IN_PRELIEVO);

                                        /*
                                        * Messaggio di Ordine in prelievo a MAIN
                                        */
                                        sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
                                        SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_IN_PRELIEVO, szMsgTxt);


                                        /*
                                        * aggiungo l'ordine alla coda di questo settore
                                        */
                                        link_add(&(ListaOrdini[nSettoreIndex]),pOrdine);
                                    }/* if(Ordine.nRighe==0 || Ordine.nCopie==0)                      - Verifica contenuto ordine */
                                }  /* if(bProducibile)                                              - Verifica producibilita' ordine */
                            }    /* if(!Cfg.bAttesaAnalisiVolumetrice)                            - Flag di attesa analisi volumetrica per ordini forzati*/
                        }      /* if(GetOrdine(&Ordine,pSettore->nIsola,pSettore->nCedola))     - Ricerca ordine in attesa di prelievo */
                    }        /* if(!Cfg.bAttesaAnalisiVolumetrice)                            - Flag di attesa analisi volumetrica per ordini forzati*/
                }          /* if(nGetOrderTimePassed[pSettore->nSettore]==0)                - Tempo di ritardo sulla ricerca ordine */
            }            /* if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))==NULL) - Verifica di coda vuota */
        }              /* if(pSettore->nTipoSettore==SETTORE_START)                     - Verifica di settore di start */

        /*
        * caso valido per tutti i settori, compreso il primo:
        * se la coda ordini e' vuota non faccio niente
        * altrimenti passo alla elaborazione del primo ordine in coda
        */
        if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

            if(pSettore->nTipoSettore==SETTORE_START && Cfg.nImballiUbicati==1){
                PIMBALLO_PRELIEVO pImballo;
                /*
                * Se si tratta di un settore di start e gestisco
                * gli imballi a ubicazione
                * passo al prelievo imballi e stampa etichetta per i colli AUTOMATICI
                * altrimenti passo alla gestione righe di prelievo
                */
                DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select c.ordprog,c.cptpfor,count(c.cptpfor),u.ios,u.cpu,u.modulo,u.riga,u.colonna,u.display,u.settore from col_prod as c, ubicazioni as u where ordprog='%s' and c.cpswlin!='1' and u.codprod=c.cptpfor and u.isola=%d group by c.cptpfor,c.ordprog,c.ordprog,u.ios,u.cpu,u.modulo,u.riga,u.colonna,u.display,u.settore,u.priorita order by u.priorita;",pOrdine->szOrdProg,pSettore->nIsola);

                nTuples=DBntuples(DBRes);
                for(nIndex=0;nIndex<nTuples;nIndex++){
                    pImballo=&(pSettore->Imballo[nIndex]);
                    strcpy(pImballo->szOrdProg,   DBgetvalue(DBRes,nIndex,0));
                    strcpy(pImballo->szFormato,   DBgetvalue(DBRes,nIndex,1));
                    pImballo->nNumCollo    = atoi(DBgetvalue(DBRes,nIndex,2));
                    pImballo->nIOS         = atoi(DBgetvalue(DBRes,nIndex,3));
                    pImballo->nCPU         = atoi(DBgetvalue(DBRes,nIndex,4));
                    pImballo->nModulo      = atoi(DBgetvalue(DBRes,nIndex,5));
                    pImballo->nRiga        = atoi(DBgetvalue(DBRes,nIndex,6));
                    pImballo->nColonna     = atoi(DBgetvalue(DBRes,nIndex,7));
                    pImballo->nDisplay     = atoi(DBgetvalue(DBRes,nIndex,8));
                }
                DBclear(DBRes);

                pSettore->nIndiceImballo=0;
                pSettore->nNumeroImballi=nIndex;
                pImballo=&(pSettore->Imballo[pSettore->nIndiceImballo]);

                strcpy(pSettore->szCodSped,pImballo->szOrdProg);
                pSettore->nNumCollo = pImballo->nNumCollo;
                pSettore->nIOS      = pImballo->nIOS;
                pSettore->nCPU      = pImballo->nCPU;
                pSettore->nModulo   = pImballo->nModulo;
                pSettore->nRiga     = pImballo->nRiga;
                pSettore->nColonna  = pImballo->nColonna;
                pSettore->nDisplay  = GetDisplay(pImballo->nDisplay,pSettore->nIsola);

                /*
                * scrivo i dati per il prelievo imballi sul display
                */
                pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                sprintf(pDisplay->szRiga_2_Display,"I%3d",pSettore->nNumCollo);
                pDisplay->nStatoRiga1=NORMAL;
                pDisplay->nStatoRiga2=NORMAL;

                UpdateDisplay(pDisplay,TUTTO);

                /*
                * acccendo la luce di prelievo prodotto
                */
                pSettore->nStatoLampada=1;
                SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);
                /*
                * setto lo stato settore in attesa di conferma copie prelevate
                */
                pSettore->nStatoSettore=PRELIEVO_IMBALLI;
            } else {
                InizioPrelievoSettore(pOrdine,pSettore);
            }
        }
    }

    switch(pEvento->nEvento){
        case IOS_PKL_BUTTON_PRESSED:
            switch(pSettore->nStatoSettore){
                case PRELIEVO_IMBALLI:
                {
                    PIMBALLO_PRELIEVO pImballo;
                    /*
                    * sono in attesa prelievo imballo
                    * e' stato premuto il tasto di prelievo effettuato (riga/colonna giuste)
                    * - se si tratta dell'ultimo imballo passo al prelievo righe
                    */
                    /*
                    * controllo che la riga/colonna siano quelle giuste
                    */
                    switch(pEvento->nEvento){
                        case IOS_PKL_BUTTON_PRESSED:
                        {
                            if(pEvento->nModulo==pSettore->nModulo && pEvento->nRiga==pSettore->nRiga && pEvento->nColonna==pSettore->nColonna){

                                /* reset lampada */
                                pSettore->nStatoLampada=0;

                                if(pSettore->nIndiceImballo+1<pSettore->nNumeroImballi){
                                    /*
                                    * STAMPA ETICHETTA IMBALLO ATTUALE e 
                                    * ----------------------------------
                                    * Prelievo imballo successivo
                                    */
                                    pSettore->nIndiceImballo++;
                                    pImballo=&(pSettore->Imballo[pSettore->nIndiceImballo]);

                                    strcpy(pSettore->szCodSped,pImballo->szOrdProg);
                                    pSettore->nNumCollo = pImballo->nNumCollo;
                                    pSettore->nIOS      = pImballo->nIOS;
                                    pSettore->nCPU      = pImballo->nCPU;
                                    pSettore->nModulo   = pImballo->nModulo;
                                    pSettore->nRiga     = pImballo->nRiga;
                                    pSettore->nColonna  = pImballo->nColonna;
                                    pSettore->nDisplay  = GetDisplay(pImballo->nDisplay,pSettore->nIsola);

                                    /*
                                    * scrivo i dati per il prelievo imballo sul display
                                    */
                                    pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                    strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                    sprintf(pDisplay->szRiga_2_Display,"I%3d",pSettore->nNumCollo);
                                    pDisplay->nStatoRiga1=NORMAL;
                                    pDisplay->nStatoRiga2=NORMAL;

                                    UpdateDisplay(pDisplay,TUTTO);

                                    /*
                                    * acccendo la luce di prelievo prodotto
                                    */
                                    pSettore->nStatoLampada=1;
                                    SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);
                                    /*
                                    * setto lo stato settore in attesa di conferma copie prelevate
                                    */
                                    pSettore->nStatoSettore=PRELIEVO_IMBALLI;
                                } else {
                                    /*
                                    * scrivo i dati per il prelievo imballo sul display
                                    */
                                    pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                    strcpy(pDisplay->szRiga_1_Display,"    ");
                                    strcpy(pDisplay->szRiga_2_Display,"    ");
                                    pDisplay->nStatoRiga1=NORMAL;
                                    pDisplay->nStatoRiga2=NORMAL;

                                    UpdateDisplay(pDisplay,TUTTO);
                                    /*
                                    * Fine Prelievo imballi
                                    */
                                    if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
                                        InizioPrelievoSettore(pOrdine,pSettore);
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                break;
                case PRELIEVO_COPIE:
                {
                    /*
                    * sono in attesa prelievo
                    * e' stato premuto il tasto di prelievo effettuato (riga/colonna giuste)
                    * - se si tratta dell'ultima riga del collo chiudo il collo
                    * - se ci sono ancora righe in questo settore vado avanti nel prelievo
                    * - se ci sono altre righe nei settori successivi passo il collo al settore successivo 
                    */
                    /*
                    * controllo che la riga/colonna siano quelle giuste
                    */
                    switch(pEvento->nEvento){
                        case IOS_PKL_BUTTON_PRESSED:
                        {
                            PRIGA_PRELIEVO pOldRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
                            PRIGA_PRELIEVO pRigaOrdine=pOldRigaOrdine;

                            if(pEvento->nModulo==pSettore->nModulo && pEvento->nRiga==pSettore->nRiga && pEvento->nColonna==pSettore->nColonna){

                                /* reset lampada */
                                pSettore->nStatoLampada=0;
                                /*
                                * prelievo confermato
                                * setto lo stato della riga (PRELEVATA)
                                * passo alla prossima riga da prelevare (se c'e')
                                * se il collo e' diverso allora chiudo il collo
                                */
                                pRigaOrdine->nStato=PRELEVATA;

                                /*
                                * Aggiorna i dati della riga ordine in oggetto con il codice operatore, data e ora prelievo e stato
                                */
                                UpdateRigaOrdine(pRigaOrdine,pSettore);

                                if(pSettore->nIndiceRigaOrdine==pSettore->nNumeroRigheOrdine-1){
                                    if(!Cfg.nChiusuraCollo && pRigaOrdine->nNextCollo==-1){
                                        /*
                                        * scrivo i dati per il fine ordine sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        strcpy(pDisplay->szRiga_2_Display,"FFFF");
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);

                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=FINE_ORDINE;
                                    } else if(!Cfg.nChiusuraCollo || pOldRigaOrdine->nNumCollo==pOldRigaOrdine->nNextCollo){
                                        /*
                                        * finite le righe d'ordine
                                        * se il prossimo collo e' uguale a quello attuale
                                        * allora faccio il passaggio a settore successivo
                                        * altrimenti faccio la chiusura collo
                                        */

                                        /*
                                        * scrivo i dati per il passaggio sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        strcpy(pDisplay->szRiga_2_Display,"PPPP");
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);

                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=PASSA_SETT_SUCC;
                                    } else {
                                        /*
                                        * chiudo il collo se in gestione collo e non ordine (rif.Wella)
                                        */
                                        /*
                                        * scrivo i dati per il passaggio sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        sprintf(pDisplay->szRiga_2_Display,"C%3d", pOldRigaOrdine->nNumCollo);
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);

                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=CHIUSURA_COLLO;
                                    }
                                } else {
                                    /*
                                    * ci sono ancora righe d'ordine da fare
                                    * controllo prima che non ci sia da chiudere il collo
                                    */
                                    if(!Cfg.nChiusuraCollo || pOldRigaOrdine->nNumCollo==pOldRigaOrdine->nNextCollo){
                                        pSettore->nIndiceRigaOrdine++;
                                        pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
                                        /*
                                        * il collo e' lo stesso : faccio il prelievo
                                        */
                                        /* 
                                        * setto lo stato della riga (ATTESA_PRELIEVO)
                                        */
                                        pRigaOrdine->nStato=ATTESA_PRELIEVO;

                                        /*
                                        * setto i dati della riga corrente del settore
                                        */
                                        strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
                                        strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
                                        strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
                                        pSettore->nNumCollo=pRigaOrdine->nNumCollo;
                                        pSettore->nNumCopie=pRigaOrdine->nNumCopie;
                                        pSettore->nModulo=pRigaOrdine->nModulo;
                                        pSettore->nRiga=pRigaOrdine->nRiga;
                                        pSettore->nColonna=pRigaOrdine->nColonna;
                                        if(pSettore->nDisplay!=pRigaOrdine->nDisplay){
                                            /*
                                            * Pulisco il display
                                            */
                                            pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                            strcpy(pDisplay->szRiga_1_Display,"    ");
                                            strcpy(pDisplay->szRiga_2_Display,"    ");
                                            pDisplay->nStatoRiga1=NORMAL;
                                            pDisplay->nStatoRiga2=NORMAL;
                                            UpdateDisplay(pDisplay,TUTTO);
                                        }
                                        pSettore->nDisplay = GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        /*
                                        * scrivo i dati per il prelievo sul display
                                        */
                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=NORMAL;
                                        UpdateDisplay(pDisplay,TUTTO);
                                        /*
                                        * acccendo la luce di prelievo prodotto
                                        */
                                        pSettore->nStatoLampada=1;
                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);
                                        /*
                                        * setto lo stato settore in attesa di conferma copie prelevate
                                        */
                                        pSettore->nStatoSettore=PRELIEVO_COPIE;
                                    } else {
                                        /*
                                        * il collo e' diverso : faccio la chiusura collo
                                        */
                                        /*
                                        * scrivo i dati per la chiusura sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        sprintf(pDisplay->szRiga_2_Display,"C%3d", pOldRigaOrdine->nNumCollo);
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);

                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=CHIUSURA_COLLO;
                                    }
                                }
                            }
                        }
                        break;
                    }
                }
                break;
                case PASSA_SETT_SUCC:
                {
                    /*
                    * dopo conferma operatore
                    * infilo l'ordine in coda al primo settore abilitato successivo al presente
                    * e mi rimetto in attesa prossimo ordine in coda
                    */
                    /*
                    * sono in attesa di conferma passaggio a settore successivo
                    * e' stato premuto il tasto di conferma passaggio (riga/colonna giuste)
                    * metto l'ordine in coda al settore successivo e
                    * resetto il display e passo allo stato di attesa
                    */
                    /*
                    * controllo che la riga/colonna siano quelle giuste
                    */
                    switch(pEvento->nEvento){
                        case IOS_PKL_BUTTON_PRESSED:
                            if(pEvento->nModulo==pSettore->nModulo && pEvento->nRiga==pSettore->nRiga && pEvento->nColonna==pSettore->nColonna){

                                /* reset lampada */
                                pSettore->nStatoLampada=0;

                                pDisplay=&(Cfg.Displays[pSettore->nDisplay]);

                                strcpy(pDisplay->szRiga_1_Display,"    ");
                                strcpy(pDisplay->szRiga_2_Display,"    ");
                                pDisplay->nStatoRiga1=NORMAL;
                                pDisplay->nStatoRiga2=NORMAL;
                                UpdateDisplay(pDisplay,TUTTO);

                                /*
                                * passaggio ordine confermato
                                */
                                pSettore->szCodSped[0]='\0';
                                pSettore->szCodProd[0]='\0';
                                pSettore->szCodUbi[0]='\0';
                                pSettore->nNumCollo=0;
                                pSettore->nNumCopie=0;
                                pSettore->nIOS=pSettore->nFirstIOS;
                                pSettore->nCPU=pSettore->nFirstCPU;
                                pSettore->nModulo=pSettore->nFirstMod;
                                pSettore->nRiga=pSettore->nFirstRow;
                                pSettore->nColonna=pSettore->nFirstColumn;
                                pSettore->nDisplay=pSettore->nFirstDisplay;
                                pSettore->nIndiceRigaOrdine=0;
                                pSettore->nNumeroRigheOrdine=0;

                                if(pSettore->nTipoSettore==SETTORE_END){
                                    /*
                                    * rimuovo il primo ordine dalla coda
                                    */
                                    if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

                                        link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
                                        free(pOrdine);
                                    }
                                } else {
                                    /*
                                    * metto il primo ordine in coda al settore successivo
                                    * e lo rimuovo dalla coda del settore attuale
                                    */
                                    if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
                                        sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);

                                        /*
                                        * Messaggio di Ordine Settore Successivo a MAIN
                                        */
                                        SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_SETT_SUCC, szMsgTxt);
                                        /*
                                        * rimuovo l'ordine dal settore attuale
                                        */
                                        link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
                                        /*
                                        * aggiungo l'ordine al primo settore abilitato successivo al presente
                                        */
                                        /*
                                        * DA SISTEMARE ... GESTIRE I SETTORI PER ISOLA !!!!
                                        */
                                        while(pSettori[nSettoreIndex].nIsola==pSettore->nIsola && pSettori[nSettoreIndex].nStatoSettore==DISABILITATO){
                                            nSettoreIndex++;
                                        }
#ifdef TRACE
                                        trace_debug(TRUE, TRUE, "Passo Ordine [%s] a Settore [%2d]", pOrdine->szOrdProg,nSettoreIndex+1);
#endif

                                        link_add(&(ListaOrdini[nSettoreIndex+1]),pOrdine);
                                    }
                                }

                                pSettore->nStatoSettore=ATTESA;
                            }
                        break;
                    }
                }
                break;
                case CHIUSURA_COLLO:
                {
                    /*
                    * sono in attesa di chiusura collo
                    * e' stato premuto il tasto di chiusura effettuata (riga/colonna giuste)
                    * - se ci sono altre righe in questo settore: 
                    *   - prelievo copie
                    * - se si tratta dell'ultima riga del collo:
                    *   - se si tratta dell'ultimo collo faccio fine ordine
                    *   - se c'e' un altro collo passo al settore successivo
                    */
                    /*
                    * controllo che la riga/colonna siano quelle giuste
                    */
                    switch(pEvento->nEvento){
                        case IOS_PKL_BUTTON_PRESSED:
                        {
                            PRIGA_PRELIEVO pOldRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
                            PRIGA_PRELIEVO pRigaOrdine;

                            if(pEvento->nIOS==pSettore->nIOS && 
                                pEvento->nCPU==pSettore->nCPU && 
                                pEvento->nModulo==pSettore->nModulo && 
                                pEvento->nRiga==pSettore->nRiga && 
                                pEvento->nColonna==pSettore->nColonna){

                                /* reset lampada */
                                pSettore->nStatoLampada=0;
                                /*
                                * chiusura collo confermata
                                */
                                if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
                                    if(StatoCollo(pOrdine->szOrdProg,pSettore->nNumCollo)!=COLLO_EVASO){
                                        CambiaStatoCollo(pOrdine->szOrdProg,pSettore->nNumCollo,COLLO_CHIUSO);
                                    }

                                    /*
                                    * Messaggio di Collo Chiuso a MAIN
                                    */
                                    sprintf(szMsgTxt,"%d,%d,%s,%d", pSettore->nIsola, pSettore->nSettore, pOrdine->szOrdProg,pSettore->nNumCollo);
                                    SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_COLLO_PRELEVATO, szMsgTxt);

                                }
                                /*
                                * passo alla prossima riga da prelevare (se c'e')
                                */
                                if(pSettore->nIndiceRigaOrdine==pSettore->nNumeroRigheOrdine-1){
                                    /*
                                    * non ci sono piu' righe da prelevare in questo settore;
                                    * visto che sono in conferma chiusura collo questo significa che :
                                    *  - se il prossimo collo ESISTE verra' iniziato da uno dei prossimi settori
                                    *    e quindi faccio il passaggio a settore successivo
                                    *  - se il prossimo collo NON ESISTE passo a Fine Ordine
                                    *
                                    */
                                    if(pOldRigaOrdine->nNextCollo==-1){
                                        /* Fine Ordine */
                                        if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
                                            /*
                                            * Messaggio di Ordine Terminato a MAIN
                                            */
                                            if(StatoOrdine(pOrdine->szOrdProg)!=ORDINE_EVASO){
                                                CambiaStatoOrdine(pOrdine->szOrdProg, ORDINE_PRELEVATO);
                                            }
                                            /*
                                            * Messaggio di Ordine terminato a MAIN
                                            */
                                            sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);
                                            SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_PRELEVATO, szMsgTxt);
                                        }
                                        /*
                                        * scrivo i dati per il fine ordine sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        strcpy(pDisplay->szRiga_2_Display,"FFFF");
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);

                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=FINE_ORDINE;

                                    } else {
                                        /* Passa a Settore Successivo */
                                        /*
                                        * scrivo i dati per il passaggio sul display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                        strcpy(pDisplay->szRiga_2_Display,"PPPP");
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=BLINK;
                                        UpdateDisplay(pDisplay,TUTTO);


                                        /*
                                        * faccio confermare l'ultima lampada premuta 
                                        */
                                        pSettore->nStatoLampada=1;

                                        SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);

                                        pSettore->nStatoSettore=PASSA_SETT_SUCC;
                                    }
                                } else {
                                    /*
                                    * ci sono altre righe da prelevare in questo settore
                                    */
                                    pSettore->nIndiceRigaOrdine++;
                                    pRigaOrdine=&(pSettore->RigaOrdine[pSettore->nIndiceRigaOrdine]);
                                    /*
                                    * il collo e' lo stesso : faccio il prelievo
                                    */
                                    /* 
                                    * setto lo stato della riga (ATTESA_PRELIEVO)
                                    */
                                    pRigaOrdine->nStato=ATTESA_PRELIEVO;

                                    /*
                                    * setto i dati di lavoro del settore
                                    */
                                    strcpy(pSettore->szCodSped,pRigaOrdine->szCodSped);
                                    strcpy(pSettore->szCodProd,pRigaOrdine->szCodProd);
                                    strcpy(pSettore->szCodUbi,pRigaOrdine->szCodUbi);
                                    pSettore->nNumCollo=pRigaOrdine->nNumCollo;
                                    pSettore->nNumCopie=pRigaOrdine->nNumCopie;
                                    pSettore->nIOS=pRigaOrdine->nIOS;
                                    pSettore->nCPU=pRigaOrdine->nCPU;
                                    pSettore->nModulo=pRigaOrdine->nModulo;
                                    pSettore->nRiga=pRigaOrdine->nRiga;
                                    pSettore->nColonna=pRigaOrdine->nColonna;
                                    if(pSettore->nDisplay!=pRigaOrdine->nDisplay){
                                        /*
                                        * Pulisco il display
                                        */
                                        pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                        strcpy(pDisplay->szRiga_1_Display,"    ");
                                        strcpy(pDisplay->szRiga_2_Display,"    ");
                                        pDisplay->nStatoRiga1=NORMAL;
                                        pDisplay->nStatoRiga2=NORMAL;
                                        UpdateDisplay(pDisplay,TUTTO);
                                    }
                                    pSettore->nDisplay = GetDisplay(pRigaOrdine->nDisplay,pSettore->nIsola);
                                    /*
                                    * scrivo i dati per il prelievo sul display
                                    */
                                    pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                    strcpy(pDisplay->szRiga_1_Display,GetDisplayData(pSettore->szCodSped));
                                    sprintf(pDisplay->szRiga_2_Display,"P%3d",pSettore->nNumCopie);
                                    pDisplay->nStatoRiga1=NORMAL;
                                    pDisplay->nStatoRiga2=NORMAL;
                                    UpdateDisplay(pDisplay,TUTTO);
                                    /*
                                    * acccendo la luce di prelievo prodotto
                                    */
                                    pSettore->nStatoLampada=1;
                                    SetLampada(pSettore->nIOS,pSettore->nCPU,pSettore->nModulo,pSettore->nRiga,pSettore->nColonna);
                                    /*
                                    * setto lo stato settore in attesa di conferma copie prelevate
                                    */
                                    pSettore->nStatoSettore=PRELIEVO_COPIE;
                                }
                            }
                        }
                        break;
                    }
                }
                break;
                case FINE_ORDINE:
                {
                    /*
                    * sono in attesa di conferma fine ordine
                    * e' stato premuto il tasto di conferma fine ordine (riga/colonna giuste)
                    * resetto il display e passo allo stato di attesa
                    */
                    /*
                    * controllo che la riga/colonna siano quelle giuste
                    */
                    switch(pEvento->nEvento){
                        case IOS_PKL_BUTTON_PRESSED:
                            if(pEvento->nIOS==pSettore->nIOS && 
                                pEvento->nCPU==pSettore->nCPU && 
                                pEvento->nModulo==pSettore->nModulo && 
                                pEvento->nRiga==pSettore->nRiga && 
                                pEvento->nColonna==pSettore->nColonna){


                                pDisplay=&(Cfg.Displays[pSettore->nDisplay]);
                                strcpy(pDisplay->szRiga_1_Display,"    ");
                                strcpy(pDisplay->szRiga_2_Display,"    ");
                                pDisplay->nStatoRiga1=NORMAL;
                                pDisplay->nStatoRiga2=NORMAL;
                                UpdateDisplay(pDisplay,TUTTO);



                                /* reset lampada */
                                pSettore->nStatoLampada=0;

                                /* metto il settore in attesa di ordini da gestire */
                                pSettore->nStatoSettore=ATTESA;

                                /*
                                * fine ordine confermata
                                */
                                pSettore->szCodSped[0]='\0';
                                pSettore->szCodProd[0]='\0';
                                pSettore->szCodUbi[0]='\0';
                                pSettore->nNumCollo=0;
                                pSettore->nNumCopie=0;
                                pSettore->nIOS=pSettore->nFirstIOS;
                                pSettore->nCPU=pSettore->nFirstCPU;
                                pSettore->nModulo=pSettore->nFirstMod;
                                pSettore->nRiga=pSettore->nFirstRow;
                                pSettore->nColonna=pSettore->nFirstColumn;
                                pSettore->nDisplay=pSettore->nFirstDisplay;
                                pSettore->nIndiceRigaOrdine=0;
                                pSettore->nNumeroRigheOrdine=0;

                                /*
                                * Gestione di fine settore particolare per Wella (NO RULLIERA DI SCARICO)
                                */
                                if(Cfg.nFineOrdine || pSettore->nTipoSettore==SETTORE_END){

                                    /*
                                    * rimuovo il primo ordine dalla coda
                                    */
                                    if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){

                                        link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
                                        free(pOrdine);
                                    }
                                } else {
                                    /*
                                    * Faccio proseguire l'ordine fino all'ultimo settore dell'isola
                                    * (SETTORE DI END)
                                    */
                                    /*
                                    * metto il primo ordine in coda al settore successivo
                                    * e lo rimuovo dalla coda del settore attuale
                                    */
                                    if((pOrdine=link_first(&(ListaOrdini[nSettoreIndex])))!=NULL){
                                        sprintf(szMsgTxt,"%d,%d,%s",pSettore->nIsola,pSettore->nSettore,pOrdine->szOrdProg);

                                        /*
                                        * Messaggio di Ordine Settore Successivo a MAIN
                                        */
                                        SendMessage(PROC_MAIN, PROC_SETTORI,  SETTORI_ORDINE_SETT_SUCC, szMsgTxt);
                                        /*
                                        * rimuovo l'ordine dal settore attuale
                                        */
                                        link_remove(&(ListaOrdini[nSettoreIndex]),pOrdine);
                                        /*
                                        * aggiungo l'ordine al primo settore abilitato successivo al presente
                                        */
                                        /*
                                        * DA SISTEMARE ... GESTIRE I SETTORI PER ISOLA !!!!
                                        */
                                        while(pSettori[nSettoreIndex].nIsola==pSettore->nIsola && pSettori[nSettoreIndex].nStatoSettore==DISABILITATO){
                                            nSettoreIndex++;
                                        }
#ifdef TRACE
                                        trace_debug(TRUE, TRUE, "Passo Ordine [%s] a Settore [%2d]", pOrdine->szOrdProg,nSettoreIndex);
#endif

                                        link_add(&(ListaOrdini[nSettoreIndex+1]),pOrdine);
                                    }
                                }
                            }
                        break;
                    }
                }
                break;
            }
        break;
    }

    return TRUE;
}

/*
* lettura delle ubicazioni in archivio
*/
int ReadUbicazioni(PUBICAZIONI pUbi)
{
    int nIndex;
    DBresult *DBRes = NULL;
    int nTuples;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select ubicazione, codprod, isola, settore, display, ios, cpu, modulo, riga, colonna, priorita, cnistato from ubicazioni where ubitipo='%s';",Cfg.szTipoOrdini);

    nTuples=DBntuples(DBRes);

    for(nIndex=0;nIndex<nTuples;nIndex++){

        strcpy(pUbi->Ubicazione[nIndex].szCodUbi,         DBgetvalue(DBRes,nIndex,0));
        strcpy(pUbi->Ubicazione[nIndex].szCodProd,        DBgetvalue(DBRes,nIndex,1));
        pUbi->Ubicazione[nIndex].nIsola            = atoi(DBgetvalue(DBRes,nIndex,2));
        pUbi->Ubicazione[nIndex].nSettore          = atoi(DBgetvalue(DBRes,nIndex,3));
        pUbi->Ubicazione[nIndex].nDisplay          = atoi(DBgetvalue(DBRes,nIndex,4));
        pUbi->Ubicazione[nIndex].nIOS              = atoi(DBgetvalue(DBRes,nIndex,5));
        pUbi->Ubicazione[nIndex].nCPU              = atoi(DBgetvalue(DBRes,nIndex,6));
        pUbi->Ubicazione[nIndex].nModulo           = atoi(DBgetvalue(DBRes,nIndex,7));
        pUbi->Ubicazione[nIndex].nRowIndex         = atoi(DBgetvalue(DBRes,nIndex,8));
        pUbi->Ubicazione[nIndex].nColIndex         = atoi(DBgetvalue(DBRes,nIndex,9));
        pUbi->Ubicazione[nIndex].nPriorita         = atoi(DBgetvalue(DBRes,nIndex,10));

        pUbi->Ubicazione[nIndex].nIndex=nIndex;
        
    }
    DBclear(DBRes);
    pUbi->nUbicazioni=nIndex;

    return nIndex;
}

void SortUbicazioni(PUBICAZIONI pUbi)
{
    qsort(pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

int CmpUbicazione(PUBICAZIONE pUbi1,PUBICAZIONE pUbi2)
{
    if(pUbi1->nIOS < pUbi2->nIOS){
        return -1;
    }
    if(pUbi1->nIOS > pUbi2->nIOS){
        return 1;
    }
    if(pUbi1->nCPU < pUbi2->nCPU){
        return -1;
    }
    if(pUbi1->nCPU > pUbi2->nCPU){
        return 1;
    }
    if(pUbi1->nModulo < pUbi2->nModulo){
        return -1;
    }
    if(pUbi1->nModulo > pUbi2->nModulo){
        return 1;
    }
    if(pUbi1->nRowIndex < pUbi2->nRowIndex){
        return -1;
    }
    if(pUbi1->nRowIndex > pUbi2->nRowIndex){
        return 1;
    }
    if(pUbi1->nColIndex < pUbi2->nColIndex){
        return -1;
    }
    if(pUbi1->nColIndex > pUbi2->nColIndex){
        return 1;
    }
    return 0;
}


PUBICAZIONE SearchUbicazione(PUBICAZIONI pUbi,int nIOS,int nCPU,int nModulo,int nRowIndex,int nColIndex)
{
    UBICAZIONE Ubi;

    Ubi.nIOS=nIOS;
    Ubi.nCPU=nCPU;
    Ubi.nModulo=nModulo;
    Ubi.nRowIndex=nRowIndex;
    Ubi.nColIndex=nColIndex;

    return bsearch(&Ubi,pUbi->Ubicazione,pUbi->nUbicazioni,sizeof(UBICAZIONE),(int(*)())CmpUbicazione);
}

/*
* Visualizza dati su display A&L PKDIS
*/
void UpdateDisplay(PDISPLAYSTRUCT pDisplay,int nRiga)
{
    int nIOS;
    int nCPU;
    int nModulo; 

    if(pDisplay==(PDISPLAYSTRUCT)NULL){
        return;
    }

    nIOS=pDisplay->nIOS;
    nCPU=pDisplay->nCPU;
    nModulo=pDisplay->nModulo; 

    switch(nRiga){
        case TUTTO:
            DisplayWriteStringBCD( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display,0,0);
            DisplayWriteString( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
            /*
            DisplayWriteString( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display);
            DisplayWriteString( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
            */
        break;

        case RIGA_1:
            DisplayWriteStringBCD( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display,0,0);
            /*
            DisplayWriteString( nIOS, nCPU, nModulo, 0, pDisplay->nStatoRiga1, pDisplay->szRiga_1_Display);
            */
        break;

        case RIGA_2:
            DisplayWriteString( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
            /*
            DisplayWriteString( nIOS, nCPU, nModulo, 1, pDisplay->nStatoRiga2, pDisplay->szRiga_2_Display);
            */
        break;
    }
}

void DisplayWriteString(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString)
{
    char szBuffer[40];

    sprintf(szBuffer,"%d,%d,%d,%d,%d,%x,%x,%x,%x", nIOS, nCPU, nModulo, nRowIndex, nMode, szString[0], szString[1], szString[2], szString[3]);

    SendMessage(PROC_IOS, PROC_SETTORI,  IOS_DPY_STRING, szBuffer);
}

int GetBCDNibble(unsigned char cChar)
{
    switch(cChar){
        case '0': case '1': case '2': case '3': case '4': 
        case '5': case '6': case '7': case '8': case '9':
            return cChar-'0';
        break;
        case ' ': return 10; break;
        case '-': return 11; break;
        case '.': return 12; break;
        case 'X': return 13; break;    /* non gestito */
        case 'Y': return 14; break;    /* non gestito */
        case 'Z': return 15; break;    /* non gestito */
    }
    return 15;
}

/*
* DisplayWriteStringBCD(nIOS, nCPU, nModulo, nRowIndex, nMode, szString,nDot,nBlink)
* Comando di settaggio display PKDIS usando comandi BCD (11,12,13,14-18,19)
*/
void DisplayWriteStringBCD(int nIOS, int nCPU, int nModulo, int nRowIndex, int nMode, char *szString,unsigned char nDot,unsigned char nBlink)
{
    char szBuffer[40];
    unsigned char DPYByte[3];

    DPYByte[0]  = GetBCDNibble(szString[0])<<4;
    DPYByte[0] |= GetBCDNibble(szString[1]);
    DPYByte[1]  = GetBCDNibble(szString[2])<<4;
    DPYByte[1] |= GetBCDNibble(szString[3]);

    /* flags */
    /* 4 bit bassi : puntino */
    /* 4 bit alti  : lampeggio */
    DPYByte[2]  = nDot | (nBlink<<4);

    sprintf(szBuffer,"%d,%d,%d,%d,%x,%x,%x", nIOS, nCPU, nModulo, nRowIndex, DPYByte[0],DPYByte[1],DPYByte[2]);

    SendMessage(PROC_IOS, PROC_SETTORI,  IOS_DPY_BCD, szBuffer);
}

void ScongelaPKL(void)
{
    int nIndex;
    char szBuffer[80];

    for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
        sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
        SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SCONGELA, szBuffer);
    }
}

void CongelaPKL(void)
{
    int nIndex;
    char szBuffer[80];

    for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
        sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
        SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_CONGELA, szBuffer);
    }
}

void SetLampada(int nIOS,int nCPU,int nModule,int nRowIndex, int nColumn)
{
    char szBuffer[40];

    sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS, nCPU, nModule, nRowIndex, nColumn);

    SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_SET_RC, szBuffer);
}

void ResetLampada(int nIOS,int nCPU,int nModule,int nRowIndex, int nColumn)
{
    char szBuffer[40];

    sprintf(szBuffer,"%d,%d,%d,%d,%d", nIOS, nCPU, nModule, nRowIndex, nColumn); 

    SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_RC, szBuffer);
}

void ResetLuci(void)
{
    int nIndex;
    char szBuffer[80];

    for(nIndex=0;nIndex<Cfg.nPKLNumber;nIndex++){
        sprintf(szBuffer,"%d,%d,%d", Cfg.PKL[nIndex].nIOS, Cfg.PKL[nIndex].nCPU, Cfg.PKL[nIndex].nModulo);
        SendMessage(PROC_IOS, PROC_SETTORI,  IOS_PKL_RESET_ALL, szBuffer);
    }
}

void ResetSettori(ep_bool_t bLinee)
{
    int nIndex;

    for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
        ResetSettore(pSettori[nIndex].nIsola,pSettori[nIndex].nSettore,bLinee);
        /*
        * rm 11-07-2001 : ritardo per evitare problemi con la IOS
        */
        usleep(1000*Cfg.nDelay);
    }
}

void ResetSettore(int nIsola,int nSettore,ep_bool_t bLinee)
{
    PORDINE pOrdine;
    int nFirstDisplay=0;
    int nFirstIOS=0;
    int nFirstCPU=0;
    int nFirstMod=0;
    int nFirstCol=0;
    int nFirstRow=0;
    DBresult *DBRes = NULL;
    int nTuples;
    int nIndex;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "ResetSettore(Isola:%d,Settore:%d)",nIsola,nSettore);
#endif

    /*
    * Cerco l'indice del settore relativo ai parametri isola e settore passati alla funzione
    */
    for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
        if(pSettori[nIndex].nIsola==nIsola && pSettori[nIndex].nSettore==nSettore){
            pSettori[nIndex].nIndiceRigaOrdine=0;        /* puntatore alla riga d'ordine da prelevare */
            pSettori[nIndex].nNumeroRigheOrdine=0;       /* numero delle righe d'ordine da prelevare */
            /* 
            * Lista FIFO Ordini 
            */
            while((pOrdine=link_pop(&(ListaOrdini[nIndex])))){
                free(pOrdine);
            }
            if(pSettori[nIndex].nStatoSettore!=DISABILITATO){
                pSettori[nIndex].nStatoSettore=ATTESA;       /* Stato attuale del settore */
            }
            /*
            * Determino i dati della prima ubicazione del settore in oggetto
            */
            DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select display, ios, cpu, modulo, riga, colonna from ubicazioni where ubitipo='%s' AND isola='%d' AND settore='%4d' order by priorita;",Cfg.szTipoOrdini, nIsola,nSettore);

            nTuples=DBntuples(DBRes);

            if(nTuples>=1){
                nFirstDisplay = GetDisplay(atoi(DBgetvalue(DBRes,0,0)),nIsola);
                nFirstIOS=atoi(DBgetvalue(DBRes,0,1));
                nFirstCPU=atoi(DBgetvalue(DBRes,0,2));
                nFirstMod=atoi(DBgetvalue(DBRes,0,3));
                nFirstRow=atoi(DBgetvalue(DBRes,0,4));
                nFirstCol=atoi(DBgetvalue(DBRes,0,5));
            } else {
#ifdef TRACE
                trace_debug(TRUE, TRUE, "ResetSettore(Isola:%d,Settore:%d) : Non esistono ubicazioni per il settore in oggetto",nIsola,nSettore);
#endif
            }
            pSettori[nIndex].nFirstIOS=nFirstIOS;
            pSettori[nIndex].nFirstCPU=nFirstCPU;
            pSettori[nIndex].nFirstMod=nFirstMod;
            pSettori[nIndex].nFirstRow=nFirstRow;
            pSettori[nIndex].nFirstColumn=nFirstCol;
            pSettori[nIndex].nFirstDisplay=nFirstDisplay;

            pSettori[nIndex].nIOS=nFirstIOS;             /* Nro IOS Tasto Premuto / da premere */
            pSettori[nIndex].nCPU=nFirstCPU;             /* Nro CPU Tasto Premuto / da premere */
            pSettori[nIndex].nModulo=nFirstMod;          /* Modulo Tasto Premuto / da premere */
            pSettori[nIndex].nRiga=nFirstRow;            /* Riga Tasto Premuto / da premere */
            pSettori[nIndex].nColonna=nFirstCol;         /* Colonna Tasto Premuto / da premere */
            pSettori[nIndex].nDisplay=nFirstDisplay;     /* Display */
            pSettori[nIndex].nNumCopie=0;                /* Numero Copie */
            pSettori[nIndex].nNumCollo=0;                /* Numero Collo */
            pSettori[nIndex].nStatoLampada=0;            /* Stato Lampada */
            pSettori[nIndex].szCodSped[0]='\0';          /* Codice Ordine Attivo */
            pSettori[nIndex].szCodProd[0]='\0';          /* Codice Prodotto da prelevare/prelevato*/
            pSettori[nIndex].szCodUbi[0]='\0';           /* Codice Ubicazione del prodotto da prelevare/prelevato*/

            DBclear(DBRes);
        }

    }

    /*
    * Cerco l'indice del o dei displays relativo/i ai parametri isola e settore passati alla funzione
    */
    for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
        if(Cfg.Displays[nIndex].nIsola==nIsola && Cfg.Displays[nIndex].nSettore==nSettore){
            if(bLinee){
                strcpy(Cfg.Displays[nIndex].szRiga_1_Display,"----");     /* Riga 1 del display */
                strcpy(Cfg.Displays[nIndex].szRiga_2_Display,"----");     /* Riga 2 del display */
            } else {
                strcpy(Cfg.Displays[nIndex].szRiga_1_Display,"    ");     /* Riga 1 del display */
                strcpy(Cfg.Displays[nIndex].szRiga_2_Display,"    ");     /* Riga 2 del display */
            } 

            Cfg.Displays[nIndex].nStatoRiga1=NORMAL;
            Cfg.Displays[nIndex].nStatoRiga2=NORMAL;

            UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
        }
    }
}

/*
* cambia il flag dell'ordine in PRODUZIONE e RICEZIONE
*/
ep_bool_t CambiaFlagOrdine(char *szOrdProg, char cFlag)
{
    ep_bool_t bRetValue=TRUE;
    DBresult *DBRes = NULL;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rocdflg='%c' where ordprog='%s';",cFlag,szOrdProg);
    if(atoi(DBcmdTuples(DBRes))==0){
        bRetValue=FALSE;
    }
    DBclear(DBRes);

    return bRetValue;
}



/*
* cambia lo stato dell'ordine in PRODUZIONE e RICEZIONE
*/
ep_bool_t CambiaStatoOrdine(char *szOrdProg, char cStato)
{
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "CambiaStatoOrdine [%s][%c]",szOrdProg,cStato);
#endif

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update ric_ord set rostato='%c' where ordprog='%s';",cStato,szOrdProg);
    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Fallito update su ric_ord [%s]",szOrdProg);
#endif
    }
    DBclear(DBRes);

    return(bRetVal);
}

ep_bool_t CambiaStatoColliOrdine(char *szOrdProg, char cStato)
{
    int nColli=0;
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "CambiaStatoColliOrdine [%s][%c] : ",szOrdProg,cStato);
#endif
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c' where ordprog='%s';",cStato,szOrdProg);
    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Fallito update su col_prod [%s]",szOrdProg);
#endif
    }
    nColli=atoi(DBcmdTuples(DBRes));

    DBclear(DBRes);

#ifdef TRACE
    trace_debug(FALSE, TRUE, "Cambiati %d colli",nColli);
#endif

    return(nColli!=0);
}

/*
* TODO
* rm 09-10-2000
* -------------
* CambiaStatoRighe Ordine NON viene utilizzata...
* serve una funzione di cambio stato RIGA ordine
* per la gestione statistiche di prelievo operatore
*/
ep_bool_t CambiaStatoRigheOrdine(char *szOrdProg,int nCollo,char cStato)
{
    int nRighe=0;
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "CambiaStatoRigheOrdine [%s][%c] : ",szOrdProg,cStato);
#endif
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update rig_prod set rpstato='%c' where ordprog='%s';",cStato,szOrdProg);
    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Fallito update su rig_prod [%s]",szOrdProg);
#endif
    }
    nRighe=atoi(DBcmdTuples(DBRes));

    DBclear(DBRes);

#ifdef TRACE
    trace_debug(FALSE, TRUE, "Cambiate %d righe",nRighe);
#endif

    return(nRighe!=0);
}


/*
* UpdateRigaOrdine()
* ATTENZIONE : ragiona per codice ordine e codice ubicazione
* NON UTILIZZARE IL NUMERO COLLO A CAUSA DELLA GESTIONE PRELIEVO PER ORDINE : VEDI InizioPrelievoSettore()
*/
ep_bool_t UpdateRigaOrdine(PRIGA_PRELIEVO pRigaOrdine,PSETTORE pSettore)
{
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;
    char cStato;
    char szSQLCmd[128];

    switch(pRigaOrdine->nStato){
        default:
        case ATTESA_PRELIEVO:
            cStato=RIGA_ELABORATA;
            /*
            * modifico solo lo stato della riga
            */
            sprintf(szSQLCmd,"update rig_prod set rpstato='%c' where ordprog='%s' AND rpcdubi='%s';",
                cStato,
                pRigaOrdine->szCodSped,
                pRigaOrdine->szCodUbi);
        break;
        case PRELEVATA:
            cStato=RIGA_PRELEVATA;
            /*
            * In caso di prelievo setto codice operatore e data/ora prelievo
            */
            sprintf(szSQLCmd,"update rig_prod set rpstato='%c',rpcdopr='%s',rptmpre='now' where ordprog='%s' AND rpnmcol=%d and rpcdubi='%s';",
                cStato,
                pSettore->szCodOperatore,
                pRigaOrdine->szCodSped,
                pRigaOrdine->nNumCollo,
                pRigaOrdine->szCodUbi);

        break;
    }
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,szSQLCmd);

    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Fallito update su rig_prod [%s,%d]",pRigaOrdine->szCodSped,pRigaOrdine->nProgressivo);
#endif
    }
    DBclear(DBRes);


    return(bRetVal);
}

char StatoOrdine(char *szOrdProg)
{
    DBresult *DBRes = NULL;
    char szBuffer[128];
    char cStato;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select rostato from ric_ord where ordprog='%s';",szOrdProg);
    if(DBresultStatus(DBRes) == DBRES_TUPLES_OK && DBntuples(DBRes)){
        strcpy(szBuffer,DBgetvalue(DBRes,0,0)); cStato=szBuffer[0];
    } else {
        cStato=' ';
    }
    DBclear(DBRes);

    return(cStato);
}

char StatoCollo(char *szOrdProg,int nCollo)
{
    DBresult *DBRes = NULL;
    char szBuffer[128];
    char cStato;

    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cpstato from col_prod where ordprog='%s' AND cpnmcol = %d;",szOrdProg,nCollo);
    if(DBresultStatus(DBRes) == DBRES_TUPLES_OK && DBntuples(DBRes)){
        strcpy(szBuffer,DBgetvalue(DBRes,0,0)); cStato=szBuffer[0];
    } else {
        cStato=' ';
    }
    DBclear(DBRes);

    return(cStato);
}

ep_bool_t CambiaStatoCollo(char *szOrdProg,int nCollo,char cStato)
{
    ep_bool_t bRetVal=TRUE;
    DBresult *DBRes = NULL;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "CambiaStatoCollo [%s][%d][%c] : ",szOrdProg,nCollo,cStato);
#endif
    DBRes=DBExecQuery(Cfg.nDebugLevel>1,"update col_prod set cpstato='%c' where ordprog='%s' AND cpnmcol = %d;",cStato,szOrdProg,nCollo);
    if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
        bRetVal=FALSE;
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Fallito update su col_prod [%s]",szOrdProg);
#endif
    }
    DBclear(DBRes);

    return(bRetVal);
}

int GetSettore(int nIsola,int nSettore)
{
    int nIndex;

    for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
        if(pSettori[nIndex].nIsola==nIsola && pSettori[nIndex].nSettore==nSettore){
            return nIndex;
        }
    }
    return -1;
}

/*
* SaveState(char *pszFileName)
* Salvataggio stato settori
*/
ep_bool_t SaveState(char *pszFileName)
{
    FILE *fp;
    int nIndex;
    PORDINE pOrdine;
    ep_bool_t bRetValue=TRUE;

#ifdef TRACE
    trace_debug(TRUE, TRUE, "Salvataggio stato settori : [%s]", pszFileName);
#endif

    if((fp=fopen(pszFileName,"w"))!=(FILE *)NULL){
        /*
        * salvo la strutture dei settori
        */
        for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
            /* 
            * struttura di definizione del settore 
            */
            if(!fwrite((char *)&(pSettori[nIndex]),sizeof(SETTORE),1,fp)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Errore in salvataggio stato settori : fwrite()" );
#endif
                bRetValue=FALSE;
            }
            /* 
            * numero ordini relativi al settore 
            */
            if(!fwrite((char *)&(ListaOrdini[nIndex].n_link),sizeof(int),1,fp)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Errore in salvataggio stato settori : fwrite()");
#endif
                bRetValue=FALSE;
            }
            for(pOrdine=NULL; (pOrdine=link_next(&(ListaOrdini[nIndex]),pOrdine));){
                if(!fwrite((char *)pOrdine,sizeof(ORDINE),1,fp)){
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "Errore in salvataggio stato settori : fwrite()");
#endif
                    bRetValue=FALSE;
                }
            }
        }
        /*
        * salvo la strutture dei displays
        */
        for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
            /* 
            * struttura di definizione del display 
            */
            if(!fwrite((char *)&(Cfg.Displays[nIndex]),sizeof(DISPLAYSTRUCT),1,fp)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Errore in salvataggio stato displays : fwrite()" );
#endif
                bRetValue=FALSE;
            }
        }
        /*
        * Chiusura del file
        */
        fclose(fp);
    } else {
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Errore in salvataggio stato settori : fopen(%s)", pszFileName);
#endif
        bRetValue=FALSE;
    }
    return bRetValue;
}


/*
* RestoreState(char *pszFileName)
* Ripristino stato settori
*/
ep_bool_t RestoreState(char *pszFileName)
{
    ep_bool_t bRetValue=TRUE;
    FILE *fp;
    int nIndex;
    int nOrdine;
    int nItems;
    PORDINE pOrdine;


#ifdef TRACE
        trace_debug(TRUE, TRUE, "Ripristino stato settori : [%s]", pszFileName);
#endif

    if((fp=fopen(pszFileName,"r"))!=(FILE *)NULL){
        /*
        * leggo le strutture dei settori
        */
        for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
            /* 
            * struttura di definizione del settore 
            */
            if(!fread((char *)&pSettori[nIndex],sizeof(SETTORE),1,fp)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Errore in caricamento stato settori : fread()");
#endif
                bRetValue=FALSE;
            } else {
                /* 
                * Svuoto la lista FIFO Ordini attuale
                */
                while((pOrdine=link_pop(&(ListaOrdini[nIndex])))){
                    free(pOrdine);
                }
                /* 
                * numero ordini collegati al settore 
                */
                if(!fread((char *)&nItems,sizeof(int),1,fp)){
#ifdef TRACE
                    trace_debug(TRUE, TRUE, "Errore in caricamento numero ordini : fread()");
#endif
                    bRetValue=FALSE;
                } else {
                    for(nOrdine=0;nOrdine<nItems;nOrdine++){
                        pOrdine=(PORDINE)malloc(sizeof(ORDINE));
                        memset(pOrdine,0,sizeof(ORDINE));
                        if(!fread((char *)pOrdine,sizeof(ORDINE),1,fp)){
#ifdef TRACE
                            trace_debug(TRUE, TRUE, "Errore in caricamento stato ordini : fread()");
#endif
                            bRetValue=FALSE;
                        } else {
                            link_add(&(ListaOrdini[nIndex]),pOrdine);
                        }
                    }
                }
            }
        }

        /*
        * leggo le strutture dei settori
        */
        for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
            /* 
            * struttura di definizione del display 
            */
            if(!fread((char *)&Cfg.Displays[nIndex],sizeof(DISPLAYSTRUCT),1,fp)){
#ifdef TRACE
                trace_debug(TRUE, TRUE, "Errore in caricamento stato display : fread()");
#endif
                bRetValue=FALSE;
            }
        }
        if(bRetValue){
            /*
            * ripristino stato displays e luci
            */
            for(nIndex=0;nIndex<Cfg.nNumeroSettori;nIndex++){
                if(pSettori[nIndex].nStatoLampada==1){
                    SetLampada( pSettori[nIndex].nIOS,pSettori[nIndex].nCPU, pSettori[nIndex].nModulo,pSettori[nIndex].nRiga,pSettori[nIndex].nColonna);
                }
            }
            for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
                UpdateDisplay(&(Cfg.Displays[nIndex]),TUTTO);
            }
        }
        /*
        * Chiusura file
        */
        fclose(fp);
    } else {
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Errore in rispristino stato settori : fopen(%s)", pszFileName);
#endif
        bRetValue=FALSE;
    }
    return bRetValue;
}

/*
* OrdProg : OOOOOO
* si visualizza : OOOO (ultime 4 cifre ordine)
*/
char *GetDisplayData(char *pszOrdProg)
{
    static char szData[128];

    strcpy(szData,RightStr(pszOrdProg,4));

    return szData;
}

void SaveStateIntFunc()
{
    char szBuffer[128];

    sprintf(szBuffer,"%s/settori.stato",Cfg.szPathData);
    SaveState(szBuffer);
}


void SetStatoLinea(int nStatoLinea)
{
    if(pDatiLinea) pDatiLinea->nStatoLinea=nStatoLinea;
}

int GetStatoLinea(void)
{
    if(pDatiLinea) return (pDatiLinea->nStatoLinea);
    return LINEA_IN_STOP;
}



/*
* int ReadDisplaysInfo(DISPLAYSTRUCT pDisplays)
*
* - Legge dalla relazione displays le informazioni relative ai displays della linea e
*   le memorizza nella struttura DISPLAYSTRUCT;
* - ritorna il numero dei displays memorizzati nella relazione 
*/
int ReadDisplaysInfo(PDISPLAYSTRUCT pDisplays)
{
    ep_bool_t rc = TRUE;
    DBresult *PGres;
    int nDisplays=0;
    int nIndex=0;
    
    PGres=DBExecQuery(Cfg.nDebugLevel>1,"select display,isola,settore,ios,cpu,modulo from displays order by display;");

    if(DBresultStatus(PGres) == DBRES_TUPLES_OK && (nDisplays=DBntuples(PGres))){

        /* memorizzo le informazioni nella struttura preposta */
        for(nIndex=0; nIndex<nDisplays; nIndex++){

                pDisplays[nIndex].nDisplay = atoi(DBgetvalue(PGres,nIndex,0));
                pDisplays[nIndex].nIsola   = atoi(DBgetvalue(PGres,nIndex,1));
                pDisplays[nIndex].nSettore = atoi(DBgetvalue(PGres,nIndex,2));
                pDisplays[nIndex].nIOS     = atoi(DBgetvalue(PGres,nIndex,3));
                pDisplays[nIndex].nCPU     = atoi(DBgetvalue(PGres,nIndex,4));
                pDisplays[nIndex].nModulo  = atoi(DBgetvalue(PGres,nIndex,5));
        }

        DBclear(PGres);

    } else {
        char szMsg[256];

        /* c'e' qualcosa che non va', lo segnalo */
        sprintf(szMsg,"ATTENZIONE : fallita ReadDisplaysInfo()");
        SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
        trace_debug(TRUE, TRUE, szMsg);
#endif

        DBclear(PGres);
        rc = FALSE;

    }
    return rc?nDisplays:0;
    
}

/*
* ep_bool_t ReadSettoriInfo(void)
*
* - Legge dalla relazione settori le informazioni relative ai settori della linea e
*   le memorizza nella struttura SETTORISTRUCT;
* - controlla che il numero dei settori memorizzati nella relazione del DB
*   coincida con il parametro <nNumeroSettori> passatogli
*/
ep_bool_t ReadSettoriInfo(int nNumeroSettori)
{
    DBresult *PGres;
    int nSettori=0;
    int nIndex=0;
    ep_bool_t rc = TRUE;

    PGres=DBExecQuery(Cfg.nDebugLevel>1,"select settore,isola,flag,tipo,operatore,cedola from settori order by settore;");

    if(DBresultStatus(PGres) == DBRES_TUPLES_OK && ((nSettori=DBntuples(PGres))==nNumeroSettori) ){

#ifdef TRACE
        trace_debug(FALSE, TRUE, "SETT I S T OPERATORE CEDOLA");
        trace_debug(FALSE, TRUE, "---- - - - --------- ------");
#endif
        /* memorizzo le informazioni nella struttura preposta */
        for(nIndex=0; nIndex<nSettori; nIndex++){

            pSettori[nIndex].nSettore       = atoi(DBgetvalue(PGres,nIndex,0));
            pSettori[nIndex].nIsola         = atoi(DBgetvalue(PGres,nIndex,1));
            // pSettori[nIndex].nStatoSettore  = atoi(DBgetvalue(PGres,nIndex,2));
            pSettori[nIndex].nTipoSettore   = atoi(DBgetvalue(PGres,nIndex,3));
            strcpy(pSettori[nIndex].szCodOperatore,DBgetvalue(PGres,nIndex,4));
            pSettori[nIndex].nCedola        = atoi(DBgetvalue(PGres,nIndex,5));
#ifdef TRACE
            trace_out_vstr(1,"%4d %1d %1d %1d [%s] %d",
                pSettori[nIndex].nSettore,
                pSettori[nIndex].nIsola,
                pSettori[nIndex].nStatoSettore,
                pSettori[nIndex].nTipoSettore,
                pSettori[nIndex].szCodOperatore,
                pSettori[nIndex].nCedola);
#endif
        }
#ifdef TRACE
        trace_debug(FALSE, TRUE, "---- - - - --------- ------");
#endif
    
        DBclear(PGres);

    } else {
        char szMsg[256];

        /* c'e' qualcosa che non va', lo segnalo */
        sprintf(szMsg,"ATTENZIONE : fallita ReadSettoriInfo()");
        SendMessage(PROC_MAIN, PROC_SETTORI, DISPLAY_MSG, szMsg);
#ifdef TRACE
        trace_debug(TRUE, TRUE, szMsg);
#endif

        DBclear(PGres);
        rc = FALSE;
    }
    return rc;
}


int GetDisplay(int nDisplay,int nIsola)
{
    int nIndex;
    ep_bool_t bFound=FALSE;
    
    for(nIndex=0;nIndex<Cfg.nNumeroDisplays;nIndex++){
        if((Cfg.Displays[nIndex].nDisplay==nDisplay) && (Cfg.Displays[nIndex].nIsola==nIsola)){
            bFound=TRUE;
            break;
        }
    }
    if(!bFound){
        return 0;
    }
    
    return nIndex;
}

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
    trace_debug(TRUE, TRUE, "Stopped");
#endif

    DetachSharedMemory((char *)pDatiLinea);
    DetachSharedMemory((char *)pSettori);

    if(!DeleteSharedMemory(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
#ifdef TRACE
        trace_debug(TRUE, TRUE, "Error Removing Settori Shared Memory (%d)", Cfg.nSettoriShmKey);
#endif
    }

    if(Cfg.bAlone){

        DeleteProcessMsgQ(PROC_SETTORI);

        if(!DeleteSharedMemory(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
            trace_debug(TRUE, TRUE, "Error Removing Shared Memory (%d)", Cfg.nShmKey);
        }
        pDatiLinea=(PLINEA_STRUCT)NULL;

        TerminateProcess(PROC_IOS);
    }
    DBDisconnect();

    /*
    * avverto MAIN dello stop
    */
    SendMessage(PROC_MAIN, PROC_SETTORI,  PROGRAM_STOPPED, NULL);

    exit(1);
} /* Fine SafeIntFunc */
