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


/*
* Gestione Eventi 
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

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	if(bFirstTime){
		RefreshStaticElements(win);
		RefreshAsDisconnected(win);
	}

    /*
    * eseguito una volta al secondo ...
    */
    if(lActualTime!=time((time_t *)0)){

		/*
		* Verifico la shared memory per la gestione della linea
		*/
		if((CheckShm(Cfg.nShmKey,sizeof(LINEA_STRUCT))==0)){

			if(pDatiLinea||pSettori){
				RefreshAsDisconnected(win);
				DetachShm((char *)pDatiLinea);
				DetachShm((char *)pSettori);
				pDatiLinea=NULL;
				pSettori=NULL;
			}
		}
		/*
		* Gestione stati linea
		*/
		if(pDatiLinea==NULL){
			if(!connect_linea_shm(&pDatiLinea)){
				trace_debug(TRUE, TRUE, "Connessione a dati linea fallita.");
			}
			nOldStatoLinea=0xff;
		}
		if(pSettori==NULL){
			if(!connect_settori_shm(&pSettori)){
				trace_debug(TRUE, TRUE, "Connessione a settori fallita.");
				pSettori=NULL;
				RefreshAsDisconnected(win);
			} else {
				do_aggiorna_frame(win);
			}
		}
		if(pDatiLinea){
			if(pDatiLinea->nStatoLinea!=nOldStatoLinea){

				nOldStatoLinea = pDatiLinea->nStatoLinea;
				RefreshLineaElements(win);

			}
		}

		if(pSettori != NULL && (nActualFrame==FRAME_DATI_PRODOTTO) && ((lActualTime%5)==0)){
			/*
			* Ricarico il titolo ...
			*/
			char szTitolo[256];

			szTitolo[0]='\0';

			GetTitolo(pSettori[priv->settore].szCodProd,szTitolo);

			strcpy(pSettori[priv->settore].szTitolo,szTitolo);

			RefreshSettoreElements(win);
			RefreshDatiProdottoElements(win);
		}

		if((nActualFrame==FRAME_LISTA_COLLI) && ((lActualTime%5)==0)){
			RefreshListaColliElements(win);
		}
		if((nActualFrame==FRAME_STATISTICHE) && ((lActualTime%30)==0)){
			RefreshStatisticheElements(win);
		}


        time(&lActualTime);
    }

    /* */
    if(pSettori){
        
        /*
        * Gestione settore
        */
        if(pSettori[priv->settore].nStatoSettore!=OldSettore.nStatoSettore || 
            strcmp(pSettori[priv->settore].szCodSped,OldSettore.szCodSped) || 
            pSettori[priv->settore].nNumCollo!=OldSettore.nNumCollo  ||
            strcmp(pSettori[priv->settore].szCodProd,OldSettore.szCodProd) || 
            strcmp(pSettori[priv->settore].szCodUbi,OldSettore.szCodUbi) || 
            pSettori[priv->settore].nNumCopie!=OldSettore.nNumCopie  ||
            pSettori[priv->settore].nModulo!=OldSettore.nModulo  ||
            pSettori[priv->settore].nRiga!=OldSettore.nRiga  ||
            pSettori[priv->settore].nColonna!=OldSettore.nColonna  )
        {
            char szTitolo[256];

            szTitolo[0]='\0';

            GetTitolo(pSettori[priv->settore].szCodProd,szTitolo);

            strcpy(pSettori[priv->settore].szTitolo,szTitolo);

            RefreshBlank(win);

            gtk_update();
#ifdef __linux__            
            usleep(200000);
#endif

            RefreshSettoreElements(win);
            if(nActualFrame==FRAME_DATI_PRODOTTO){
                RefreshDatiProdottoElements(win);
            } else if(nActualFrame==FRAME_LISTA_COLLI){
                switch(OldSettore.nStatoSettore){
                    case PASSA_SETT_SUCC:
                    case CHIUSURA_COLLO:
                    case FINE_ORDINE:
                        RefreshListaColliElements(win);
                    break;
                }
            }
            gtk_update();

            OldSettore.nStatoSettore     = pSettori[priv->settore].nStatoSettore;
            OldSettore.nNumCollo         = pSettori[priv->settore].nNumCollo;
            OldSettore.nNumCopie         = pSettori[priv->settore].nNumCopie;
            strcpy(OldSettore.szCodSped,   pSettori[priv->settore].szCodSped);
            strcpy(OldSettore.szCodProd,   pSettori[priv->settore].szCodProd);
            strcpy(OldSettore.szCodUbi,    pSettori[priv->settore].szCodUbi);
            OldSettore.nModulo           = pSettori[priv->settore].nModulo;
            OldSettore.nRiga             = pSettori[priv->settore].nRiga;
            OldSettore.nColonna          = pSettori[priv->settore].nColonna;
        }
    }

    /*
    * Messaggi da altri processi
    */
    while((nCarIn = ProcessReceiveMsgNoWait(priv->pid, &nAllMsg, szInMsg))>=0){
        UnpackMessage(szInMsg, &InMsgStruct, szText, nCarIn);

#ifdef TRACE
        if(Cfg.nDebugLevel>3){
            trace_debug(TRUE, TRUE,  "Msg [%d] %s%s%s from [%s]",
                InMsgStruct.code,
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
                GdkColor *pcColor;

                switch (InMsgStruct.srce){
                    case PROC_VOLUM: pcColor=&gORANGE;break;
                    case PROC_SETTORI: pcColor=&gGREEN;break;
                    case PROC_PRINTLABEL: pcColor=&gBLUE;break;
                    case PROC_RECEIVE: pcColor=&gLBLUE;break;
                    case PROC_AUTO_TASKS: pcColor=&gGREY;break;
                    default: pcColor=&gBLACK;break;
                }
                /* visualizza messaggio ... */
                trace_debug(TRUE, 1,  "%s msg : %s", ProcessGetName(InMsgStruct.srce),szText);
            }
            break;
            case PROGRAM_STARTED:
                trace_debug(TRUE, 1,  "%s Started", ProcessGetName(InMsgStruct.srce));
            break;
            case PROGRAM_STOPPED:
                trace_debug(TRUE, 1,  "%s Stopped", ProcessGetName(InMsgStruct.srce));
            break;
        }

        switch (InMsgStruct.srce){
            case PROC_VOLUM:
                switch (InMsgStruct.code){
                }
            break;
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
