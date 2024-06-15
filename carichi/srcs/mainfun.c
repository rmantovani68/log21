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
#include <picking.h>
#include <ep-db.h>
#include <ep-popover.h>
#include <trace.h>
#include <pmx_msq.h>
#include <msg-box.h>

#include <gtk-support.h>
#include <dbfun-gtk.h>

#include <proc_list.h>

#include "mainstruct.h"
#include "mainext.h"
#include "mainapp.h"
#include "mainwin.h"
#include "mainfun.h"

/* ----------------------------- */
/* LIBRERIA CHIAMATE AL DATABASE */
/* ----------------------------- */

ep_bool_t InizioTransazione(void)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	/*
	* Entro in transazione
	*/
	DBRes=DBExecQuery(DEBUG_FLAG,"begin work;");
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		bOK=FALSE;
	} else {
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t FineTransazione(ep_bool_t bCommit)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	/*
	* Esco dalla transazione
	*/
	if(bCommit){
		DBRes=DBExecQuery(DEBUG_FLAG>1,"commit work;"); 
	} else {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"abort work;");
	}
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		bOK=FALSE;
	} else {
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t MovimentaProdotto(char *szCausale, char *szCDPRO, int nQTPRO, int nDistinta, char *szUDC, char *szTerminale, char *szUser)
{
	DBresult *DBRes;
	int nGiacenzaPresente=0;
	ep_bool_t bOK=TRUE;

	if (!CodiceProdottoEsistente(szCDPRO)) {
		// return FALSE;
	}

	/*
	* Possono esserci qt negative nella causale diff inv
	*/

	InizioTransazione();
	/* Ricavo la quantità giacente */
	nGiacenzaPresente=GetGiacenzaProdotto(szCDPRO);
	/*
	* In base alla causale faccio cose...
	*/
	if(!strcmp(szCausale,"INGRESSO")){
		/*
		* Per un ingresso merce devo aggiornare o creare una giacenza
		*/
		/* inserimento del movimento */
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni ( MOCDPRO, MONMCPE, MODSCAU, MONMDIS, MOCDUDC, MOTMMOV, MOCDTRM,MOCDUSR) values ( '%s',%d,'%s',0,'','now','%s','%s');" ,szCDPRO,nQTPRO,szCausale,szTerminale,szUser); 
		DBclear(DBRes);
		/*
		* Modifico o creo la giacenza
		*/
		if (DeltaGiacenza(szCDPRO,nQTPRO) && DeltaIngressoGiacenza(szCDPRO,nQTPRO)){
			if (DeltaDifferenzaGiacenza(szCDPRO,nQTPRO)){
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
	} else if(!strcmp(szCausale,"SPEDIZIONE")){
		// Verifico di non produrre una giacemza negativa
		/* inserimento del movimento */
		// LG: per il momento tolgo dalla spedizione distinta e udc. Verificare
		//     se nel flusso vogliamo mettere questi dati.
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni ( MOCDPRO, MONMCPE, MODSCAU, MONMDIS, MOCDUDC, MOTMMOV, MOCDTRM,MOCDUSR) values ( '%s',%d,'%s',0,'','now','%s','%s');" ,szCDPRO,nQTPRO,szCausale,szTerminale,szUser); 
		DBclear(DBRes);
		/*
		* Modifico la giacenza (col -, perche' e' una spedizione!!!)
		*/
		if (DeltaGiacenza(szCDPRO,-nQTPRO) && DeltaSpeditoGiacenza(szCDPRO,nQTPRO)){
			if (DeltaDifferenzaGiacenza(szCDPRO,-nQTPRO)){
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
	} else if(!strcmp(szCausale,"RESTITUZIONE")){
		/* inserimento del movimento */
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni ( MOCDPRO, MONMCPE, MODSCAU, MONMDIS, MOCDUDC, MOTMMOV, MOCDTRM, MOCDUSR) values ( '%s',%d,'%s',%d,'%s','now','%s','%s');" ,szCDPRO,nQTPRO,szCausale,nDistinta,szUDC,szTerminale,szUser); 
		DBclear(DBRes);
		/*
		* Modifico la giacenza (col -, perche' e' una Restituzione!!!)
		*/
		if (DeltaGiacenza(szCDPRO,-nQTPRO) && DeltaRestituitoGiacenza(szCDPRO,nQTPRO)){
			if (DeltaConteggioGiacenza(szCDPRO,-nQTPRO)){
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
	} else if(!strcmp(szCausale,"DIFF_INV")){
		/* inserimento del movimento */
		// LG: per il momento tolgo dalla spedizione distinta e udc. Verificare
		//     se nel flusso vogliamo mettere questi dati.
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni ( MOCDPRO, MONMCPE, MODSCAU, MONMDIS, MOCDUDC, MOTMMOV, MOCDTRM,MOCDUSR) values ( '%s',%d,'%s',0,'','now','%s','%s');" ,szCDPRO,nQTPRO,szCausale,szTerminale,szUser); 
		DBclear(DBRes);
		/*
		*/
		if (DeltaGiacenza(szCDPRO,-nQTPRO) && DeltaDifferenzaInventarialeGiacenza(szCDPRO,nQTPRO)){
			if (DeltaDifferenzaGiacenza(szCDPRO,-nQTPRO)){
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
	} else if(!strcmp(szCausale,"CONTEGGIO")){
		/* inserimento del movimento */
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni ( MOCDPRO, MONMCPE, MODSCAU, MONMDIS, MOCDUDC, MOTMMOV, MOCDTRM,MOCDUSR) values ( '%s',%d,'%s',0,'%s','now','%s','%s');" ,szCDPRO,nQTPRO,szCausale,szUDC,szTerminale,szUser); 
		DBclear(DBRes);
		if (DeltaConteggioGiacenza(szCDPRO,nQTPRO)){
			if (DeltaDifferenzaGiacenza(szCDPRO,-nQTPRO)){
				if(CaricaProdottoSuUdc(szUDC,szCDPRO,nQTPRO)){
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
	} else {
		bOK=FALSE;
	}
	FineTransazione(bOK);

	return bOK;
}

ep_bool_t AnnullaMovimentazioniProdottoSuUdc(char *szCDPRO,char *szCDUDC)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	int nPRMOV;
	int nTuples;
	int nIndex;

	DBRes=DBExecQuery(DEBUG_FLAG>1,"select moprmov from carichi_movimentazioni where MOCDPRO='%s' and MOCDUDC='%s' and MODSCAU in ('CONTEGGIO','RESTITUZIONE') ;",szCDPRO,szCDUDC);
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
		bOK=FALSE;
	} else {
		nTuples=DBntuples(DBRes);
		for (nIndex=0;nIndex<nTuples;nIndex++){
			nPRMOV=atoi(DBgetvalue(DBRes,nIndex,0));
			if(AnnullaMovimentazione(nPRMOV)){
			} else {
				bOK=FALSE;
			}
		}
	}
	DBclear(DBRes);
	if (GetNumeroProdottiSuUdc(szCDUDC)==0){
		ResettaUdc(szCDUDC);
	}
	
	return bOK;
}

ep_bool_t AnnullaMovimentazioniUdc(char *szCDUDC)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	int nTuples;
	int nIndex;
	char szCDPRO[64];

	DBRes=DBExecQuery(DEBUG_FLAG>1,"select distinct(pucdpro) from carichi_prodotti_udc where pucdudc='%s' ;",szCDUDC);
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
		bOK=FALSE;
	} else {
		nTuples=DBntuples(DBRes);
		for (nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szCDPRO,DBgetvalue(DBRes,nIndex,0));
			if(AnnullaMovimentazioniProdottoSuUdc(szCDPRO,szCDUDC)){
			} else {
				bOK=FALSE;
			}
		}
	}
	DBclear(DBRes);
	if (bOK) {
		if(ResettaUdc(szCDUDC)){
		} else {
			bOK=FALSE;
		}
	}

	
	return bOK;
}

ep_bool_t ResettaUdc(char *szCDUDC)
{
	ep_bool_t bOK=TRUE;

	if(SetDistintaPerUdc(szCDUDC,0)){
		if (SetStatoUdc(szCDUDC,UDC_CREATO)){
		} else {
			bOK=FALSE;
		}
	} else {
		bOK=FALSE;
	}

	
	return bOK;
}



ep_bool_t AnnullaMovimentazione(int nProgressivoMovimentazione)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	char szCDPRO[64];

	char szCDCAU[128];
	char szCDUDC[128];
	int nQTPRO;


	DBRes=DBExecQuery(DEBUG_FLAG>1,"select MOCDPRO, MONMCPE, MODSCAU, MOCDUDC from carichi_movimentazioni where moprmov=%d;",nProgressivoMovimentazione);
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
		bOK=FALSE;
	} else {
		strcpy(szCDPRO,DBgetvalue(DBRes,0,0));
		nQTPRO=atoi(DBgetvalue(DBRes,0,1));
		strcpy(szCDCAU,DBgetvalue(DBRes,0,2));
		strcpy(szCDUDC,DBgetvalue(DBRes,0,3));
	}
	DBclear(DBRes);

	if (bOK){
		InizioTransazione();
		/*
		* In base alla causale faccio cose...
		*/
		if(!strcmp(szCDCAU,"INGRESSO")){
			if (DeltaGiacenza(szCDPRO,-nQTPRO) && DeltaIngressoGiacenza(szCDPRO,-nQTPRO)){
				if (DeltaDifferenzaGiacenza(szCDPRO,-nQTPRO)){
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else if(!strcmp(szCDCAU,"SPEDIZIONE")){
			if (DeltaGiacenza(szCDPRO,nQTPRO) && DeltaSpeditoGiacenza(szCDPRO,-nQTPRO)){
				if (DeltaDifferenzaGiacenza(szCDPRO,nQTPRO)){
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else if(!strcmp(szCDCAU,"RESTITUZIONE")){
			if (DeltaGiacenza(szCDPRO,nQTPRO) && DeltaRestituitoGiacenza(szCDPRO,-nQTPRO)){
				if (DeltaConteggioGiacenza(szCDPRO,nQTPRO)){
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else if(!strcmp(szCDCAU,"DIFF_INV")){
			if (DeltaGiacenza(szCDPRO,nQTPRO) && DeltaDifferenzaInventarialeGiacenza(szCDPRO,-nQTPRO)){
				if (DeltaDifferenzaGiacenza(szCDPRO,nQTPRO)){
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else if(!strcmp(szCDCAU,"CONTEGGIO")){
			if (DeltaConteggioGiacenza(szCDPRO,-nQTPRO)){
				if (DeltaDifferenzaGiacenza(szCDPRO,nQTPRO)){
					if(ScaricaProdottoDaUdc(szCDUDC,szCDPRO,nQTPRO)){
					} else {
						bOK=FALSE;
					}
				} else {
					bOK=FALSE;
				}
			} else {
				bOK=FALSE;
			}
		} else {
			bOK=FALSE;
		}
		if (bOK){
			DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_movimentazioni where moprmov=%d;",nProgressivoMovimentazione);
			if (!DBRes || DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
				bOK=FALSE;
			}
			if(atoi(DBcmdTuples(DBRes))==0){
				/* TODO */
			}
			DBclear(DBRes);
		}
		FineTransazione(bOK);
	}

	return bOK;
}


ep_bool_t DeltaGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnmcpe=cgnmcpe+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		// La giacenza non esiste, la creo
		if(atoi(DBcmdTuples(DBRes))==0){
			/* non presente - inserisco */
			DBResInsert=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_giacenze (cgcdpro,cgnmcpe,cgcdflg) values ('%s',%d,' ');",szCDPRO,nQTPRO);
			DBclear(DBResInsert);
#ifdef TRACE
			trace_debug(FALSE, TRUE, "Inserita giacenza [PRODOTTO: %s - Quantita': %d]",szCDPRO,nQTPRO);
#endif
		} else {
			/* presente - tutto ok */
#ifdef TRACE
			trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
#endif
		}
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaConteggioGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgconta=cgconta+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaDifferenzaGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnmdif=cgnmcpe-cgconta where cgcdpro='%s';",szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaIngressoGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnming=cgnming+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaSpeditoGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnmspe=cgnmspe+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaRestituitoGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnmres=cgnmres+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}

ep_bool_t DeltaDifferenzaInventarialeGiacenza(char *szCDPRO, int nQTPRO)
{
	DBresult *DBRes;
	//DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	// Update della giacenza
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_giacenze set cgnmdin=cgnmdin+(%d) where cgcdpro='%s';",nQTPRO,szCDPRO);
	if(DBresultStatus(DBRes) != DBRES_COMMAND_OK){
		/* errore su update */
		bOK=FALSE;
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Errore su update giacenza [%s]",szCDPRO);
#endif
	} else {
		trace_debug(FALSE, TRUE, "Aggiornata giacenza [%s]",szCDPRO);
	}
	DBclear(DBRes);
	return bOK;
}




int CreaDistinta(void)
{
	DBresult *DBRes;
	ep_bool_t bOK=FALSE;
	int nDist=0;
	/* verifico presenza UDC */
	if((nDist=GeneraNumeroDistinta())<=0){
		bOK=FALSE;
	} else {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_distinte (cdnmdis,cdtmins,cdstato,cdstspe) values (%d,'now',' ',' ');",nDist);
		if(DBresultStatus(DBRes) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBRes))!=1){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
	}
	return nDist;


}

ep_bool_t DistintaEsistente(int nDistinta)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;

	DBRes=DBExecQuery(DEBUG_FLAG>1,"select * from carichi_distinte where cdnmdis=%d;",nDistinta );
	if(DBntuples(DBRes)){
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);
	return bOK;
}


ep_bool_t UdcEsistente(char *szUDC)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;

	/* verifico presenza UDC */
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select  * from carichi_udc where udcdudc='%s';",szUDC);
	if(DBntuples(DBRes)!=1){
		bOK=FALSE;
	} else {
	}
	DBclear(DBRes);

	return bOK;
}


ep_bool_t CreaUdc(char *szUDC)
{
	DBresult *DBResInsert;
	ep_bool_t bOK=FALSE;

	DBResInsert=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_udc ( udcdudc,udnmdis,udtmins,udcdflg,udstato) values ( '%s',0,'now',' ',' ' );",szUDC);
	if(DBresultStatus(DBResInsert) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBResInsert))!=1){
		bOK=FALSE;
	} else {
		bOK=TRUE;
	}

	return bOK;
}

ep_bool_t SetDistintaPerUdc(char *szUDC, int nDistinta)
{
	DBresult *DBResInsert;
	ep_bool_t bOK=FALSE;

	if (!(DistintaEsistente(nDistinta) && UdcEsistente(szUDC))){
		bOK=FALSE;
	} else {
		DBResInsert=DBExecQuery(DEBUG_FLAG>1,"update carichi_udc set udnmdis=%d where udcdudc='%s';",nDistinta,szUDC);
		if(DBresultStatus(DBResInsert) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBResInsert))!=1){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBResInsert);
	}

	return bOK;
}

ep_bool_t CaricaProdottoSuUdc(char *szUDC,char *szCDPRO,int nQTPRO)
{
	DBresult *DBRes;
	DBresult *DBResUpdate;
	DBresult *DBResInsert;
	ep_bool_t bOK=TRUE;
	int nPRPRU;

	if (!UdcEsistente(szUDC)){
		CreaUdc(szUDC);
	}
	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"select puprpru from carichi_prodotti_udc where pucdudc='%s' and pucdpro='%s';",szUDC,szCDPRO);
		if(DBntuples(DBRes)==1){
			nPRPRU=atoi(DBgetvalue(DBRes,0,0));
			DBResUpdate=DBExecQuery(DEBUG_FLAG>1,"update carichi_prodotti_udc set punmcpe=punmcpe+%d where puprpru=%d;",nQTPRO,nPRPRU);
			if(DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBResUpdate))!=1){
				bOK=FALSE;
			}
			DBclear(DBResUpdate);
		} else {
			/* inserisco */
			DBResInsert=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_prodotti_udc ( pucdudc,pucdpro,punmcpe, pucdflg, pustato) values ( '%s','%s',%d,' ',' ');",szUDC,szCDPRO,nQTPRO);
			if(DBresultStatus(DBResInsert) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBResInsert))!=1){
				bOK=FALSE;
			}
			DBclear(DBResInsert);
		}
		DBclear(DBRes);
	}

	return bOK;
}

ep_bool_t ScaricaProdottoDaUdc(char *szUDC,char *szCDPRO,int nQTPRO)
{
	DBresult *DBRes;
	DBresult *DBResUpdate;
	ep_bool_t bOK=TRUE;
	int nPRPRU;
	int nNMCPE;

	if (!UdcEsistente(szUDC)){
		bOK=FALSE;
	}
	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"select puprpru,punmcpe from carichi_prodotti_udc where pucdudc='%s' and pucdpro='%s';",szUDC,szCDPRO);
		if(DBntuples(DBRes)==1){
			nPRPRU=atoi(DBgetvalue(DBRes,0,0));
			nNMCPE=atoi(DBgetvalue(DBRes,0,1));
			if (nQTPRO==nNMCPE){
				DBResUpdate=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_prodotti_udc where puprpru=%d;",nPRPRU);
				if(DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK ){
					bOK=FALSE;
				}
				if(atoi(DBcmdTuples(DBResUpdate))==0){
					/* TODO */
				}
				DBclear(DBResUpdate);
			} else {
				DBResUpdate=DBExecQuery(DEBUG_FLAG>1,"update carichi_prodotti_udc set punmcpe=punmcpe-%d where puprpru=%d;",nQTPRO,nPRPRU);
				if(DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBResUpdate))!=1){
					bOK=FALSE;
				}
				DBclear(DBResUpdate);
			}
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
	}

	return bOK;
}

int MovimentaProdottiDaOrdiniSpediti(void)
{
	DBresult *DBRes;
	DBresult *DBResTmp;
	ep_bool_t bOK=TRUE;
	int nQTSPE=0;
	int nIndexSpedizioni=0;
	int nNumeroSpedizioni=0;
	int nNMCED=0;
	char szCDPRO[32];

	DBResTmp=DBExecQuery(DEBUG_FLAG>2,"drop table carichi_ric_ord_tmp");
	DBclear(DBResTmp);

	DBResTmp=DBExecQuery(DEBUG_FLAG>2,"create table carichi_ric_ord_tmp as select * from ric_ord where roflimc='%c' and rostato in ('%c','%c','%c')",
		CARICHI_ORDINE_DA_IMPORTARE,
		ORDINE_STAMPATA_DIST,
		ORDINE_FILE_INVIATO,
		ORDINE_SPEDITO_HOST ); 
	DBclear(DBResTmp);

	DBRes=DBExecQuery(DEBUG_FLAG>2,"select ra.racdpro,ro.ronmced,sum(ra.raqtord) from carichi_ric_ord_tmp ro,ric_art ra where ro.ordprog=ra.ordprog group by ra.racdpro,ro.ronmced;"); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
	} else {
		nNumeroSpedizioni=DBntuples(DBRes);
		if(nNumeroSpedizioni){
#ifdef TRACE
			trace_debug(FALSE, TRUE, "Importazione di %d spedizioni da ric_ord",nNumeroSpedizioni);
#endif
		}
		for (nIndexSpedizioni=0;nIndexSpedizioni<nNumeroSpedizioni;nIndexSpedizioni++){
			strcpy(szCDPRO,DBgetvalue(DBRes,nIndexSpedizioni,0));
			nNMCED=atoi(DBgetvalue(DBRes,nIndexSpedizioni,1));
			nQTSPE=atoi(DBgetvalue(DBRes,nIndexSpedizioni,2));
			if(MovimentaProdotto("SPEDIZIONE",szCDPRO,nQTSPE,0,"","importato","importato")){
			} else {
			}
		}
	}
	DBclear(DBRes);
	if (nNumeroSpedizioni>0) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"update ric_ord set roflimc='%c' where ordprog in (select ordprog from carichi_ric_ord_tmp);" ,CARICHI_ORDINE_IMPORTATO); 
		if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ){
			bOK=FALSE;
		} else {
		}
		DBclear(DBRes);
	}

	return nNumeroSpedizioni;
}



ep_bool_t CodiceProdottoEsistente(char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;

	DBRes=DBExecQuery(DEBUG_FLAG>1,"select * from catalogo where prcdpro='%s' ;",szCDPRO );
	if(DBntuples(DBRes)){
	} else {
		bOK=FALSE;
	}
	DBclear(DBRes);
	return bOK;
}


int GeneraNumeroDistinta(void)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	ep_bool_t bCodiceEsistente=FALSE;
	int nDistinta=0;
	do {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"select nextval('carichi_numero_distinta_seq');");
		if(DBntuples(DBRes)){
			nDistinta=atoi(DBgetvalue(DBRes,0,0));
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
		if(DistintaEsistente(nDistinta)){
			bCodiceEsistente=TRUE;
		} else {
			bCodiceEsistente=FALSE;
		}
	}	while (bCodiceEsistente);

	return nDistinta;
}

ep_bool_t CheckLogin(char *szUserId,char *szPassword)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;

	DBRes=DBExecQuery(DEBUG_FLAG,"select * from carichi_operatori where opidope='%s' and oppwope='%s';", szUserId, szPassword );
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
		bOK=FALSE;
	} else {
	}
	/*
	* Se non ho righe...
	*/
	if(DBntuples(DBRes)==0){
		bOK=FALSE;
	} else {
	}
	DBclear(DBRes);
	return bOK;
}

int GetGiacenzaProdotto(char *szCDPRO)
{
	DBresult *DBRes;
	int nQT=0;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select cgnmcpe from carichi_giacenze where cgcdpro='%s' ;",szCDPRO); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
		nQT=0;
	} else {
		nQT=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nQT;
}

ep_bool_t ProdottoInGiacenza(char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bTrovato=FALSE;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select * from carichi_giacenze where cgcdpro='%s' ;",szCDPRO); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
		bTrovato=FALSE;
	} else {
		bTrovato=TRUE;
	}
	DBclear(DBRes);
	return bTrovato;
}


int GetContatoProdotto(char *szCDPRO)
{
	DBresult *DBRes;
	int nQT=0;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select cgconta from carichi_giacenze where cgcdpro='%s' ;",szCDPRO); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
		nQT=0;
	} else {
		nQT=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nQT;
}


int GetDifferenzaProdotto(char *szCDPRO)
{
	DBresult *DBRes;
	int nQT=0;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select cgnmdif from carichi_giacenze where cgcdpro='%s' ;",szCDPRO); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
		nQT=0;
	} else {
		nQT=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nQT;
}


ep_bool_t CreaNuovoUdc(char *szCodiceUdc)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;

	if(GetNuovoCodiceUdc(szCodiceUdc)){
	} else {
		bOK=FALSE;
	}

	/*
	* Inserisco un nuovo udc in tabella udc
	*/
	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_udc (udcdudc,udnmdis,udtmins,udcdflg,udstato) values ('%s',0,'now',' ',' ');",
			szCodiceUdc);
		if (DBresultStatus(DBRes) != DBRES_COMMAND_OK || atoi(DBcmdTuples(DBRes))!=1){
			bOK=FALSE;
		} else {
		}
		DBclear(DBRes);
	}

	return bOK;
}

ep_bool_t GetNuovoCodiceUdc(char *szNuovoCodiceUdc)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	ep_bool_t bUdcEsistente=FALSE;


	do {
		/*
		* Ricavo il codice mondadori per l'UdC
		* Il codice mondadori e' di 7 cifre, lo paddo con degli 0 a sinistra
		*/
		DBRes=DBExecQuery(DEBUG_FLAG>1,"select lpad((select nextval('carichi_codice_udc_seq'))::text,7,'0');");
		if(DBntuples(DBRes)){
			szNuovoCodiceUdc[0]='\0';
			strcpy(szNuovoCodiceUdc,DBgetvalue(DBRes,0,0));
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
		if(bOK){
		} else {
		}
		if(!UdcEsistente(szNuovoCodiceUdc)){
			bUdcEsistente=FALSE;
		} else {
			bUdcEsistente=TRUE;
		}
	}	while (bUdcEsistente);

	return bOK;
}


ep_bool_t RestituzioneDistinta(int nDistinta, char *szTerminale, char *szUser)
{
	DBresult *DBRes;
	DBresult *DBResUdc;
	char szCDUDC[128];
	char szCDPRO[128];
	char szPRPRU[128];
	int nNMCPE;
	ep_bool_t bOK=TRUE;
	int nIndex;
	int nIndexUdc;
	int nTuples;
	int nTuplesUdc;

	DBResUdc=DBExecQuery(DEBUG_FLAG>1,"select udcdudc from carichi_udc where udnmdis=%d  and udstato='%c';",nDistinta,UDC_CREATO);
	nTuplesUdc=DBntuples(DBResUdc);
	for (nIndexUdc=0;nIndexUdc<nTuplesUdc;nIndexUdc++){
		strcpy(szCDUDC,DBgetvalue(DBResUdc,nIndexUdc,0));

		DBRes=DBExecQuery(DEBUG_FLAG>1,"select pucdpro,punmcpe,puprpru from carichi_prodotti_udc where pucdudc='%s' and pustato='%c';",szCDUDC,PRODOTTO_CREATO);
		nTuples=DBntuples(DBRes);

		for (nIndex=0;nIndex<nTuples;nIndex++){
			strcpy(szCDPRO,DBgetvalue(DBRes,nIndex,0));
			nNMCPE=atoi(DBgetvalue(DBRes,nIndex,1));
			strcpy(szPRPRU,DBgetvalue(DBRes,nIndex,2));
			if(!MovimentaProdotto("RESTITUZIONE",szCDPRO,nNMCPE,nDistinta,szCDUDC,szTerminale, szUser)){
				bOK=FALSE;
			} else {
				if (SetStatoProdottoUdc(szPRPRU,PRODOTTO_RESTITUITO)){
				} else {
					bOK=FALSE;
				}
			}
		}
		if (SetStatoUdc(szCDUDC,UDC_RESTITUITO)){
		} else {
			bOK=FALSE;
		}
		DBclear(DBRes);
		
	}

	DBclear(DBResUdc);

	if (SetStatoDistinta(nDistinta,DISTINTA_RESTITUITA)){
	} else {
		bOK=FALSE;
	}
	
	return bOK;
}

ep_bool_t StoricizzaDistinta(int nDistinta)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	char cStatoDistinta;
	char cStatoSpedizioneDistinta;

	cStatoDistinta=GetStatoDistinta(nDistinta);
	cStatoSpedizioneDistinta=GetStatoSpedizioneDistinta(nDistinta);
	if ((cStatoDistinta==DISTINTA_RESTITUITA) && ((cStatoSpedizioneDistinta==DISTINTA_SPEDITA || cStatoSpedizioneDistinta==DISTINTA_SPEDITA_STAMPATA)) ){
	} else {
		bOK=FALSE;
	}

	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG,"insert into carichi_prodotti_udc_stor select * from carichi_prodotti_udc where pucdudc in (select udcdudc from carichi_udc where udnmdis=%d) ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_udc_stor select * from carichi_udc where udnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_distinte_stor select * from carichi_distinte where cdnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_prodotti_udc where pucdudc in (select udcdudc from carichi_udc where udnmdis=%d) ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_udc where udnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_distinte where cdnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		}
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
	}

	
	return bOK;
}

/*
* Elimina una distinta.
* Controlla prima gli stati di spedizione e restituzione.
* Setta gli udc della distinta eliminata a distinta 0.
*/
ep_bool_t EliminaDistinta(int nDistinta)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	char cStatoDistinta;
	char cStatoSpedizioneDistinta;

	// ricavo gli stati della distinta
	cStatoDistinta=GetStatoDistinta(nDistinta);
	cStatoSpedizioneDistinta=GetStatoSpedizioneDistinta(nDistinta);
	if ((cStatoDistinta==DISTINTA_RESTITUITA) || (cStatoSpedizioneDistinta==DISTINTA_SPEDITA) ){
		bOK=FALSE;
	}

	// Eseguo la cancellazione della distinta e setto il num distinta degli udc a 0
	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_udc set udnmdis=0 where udnmdis=%d ;",nDistinta);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_distinte where cdnmdis=%d ;",nDistinta);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
	}
	
	return bOK;
}


ep_bool_t StoricizzaProdottoInGiacenza(char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;


	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_giacenze_stor select * from carichi_giacenze where cgcdpro='%s';",szCDPRO);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni_stor select * from carichi_movimentazioni where mocdpro='%s';",szCDPRO);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_giacenze where  cgcdpro='%s';",szCDPRO);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_movimentazioni where  mocdpro='%s';",szCDPRO);
		if(atoi(DBcmdTuples(DBRes))==0){
			/* TODO */
		}
		DBclear(DBRes);
	} else {
	}

	return bOK;
}

char GetStatoDistinta(int nDistinta)
{
	DBresult *DBRes;
	char szBuffer[32];
	char cStato=DISTINTA_ERRATA;
	/*
	* Controllo l'esistenza del sottordine 
	*/
	DBRes=DBExecQuery(DEBUG_FLAG,"select cdstato from carichi_distinte where cdnmdis=%d;",nDistinta);
	if(DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
	} else {
		strcpy(szBuffer,DBgetvalue(DBRes,0,0));
		if (strlen(szBuffer)>0){
			cStato=szBuffer[0];
		}
	}
	DBclear(DBRes);
	return cStato;
}

char GetStatoSpedizioneDistinta(int nDistinta)
{
	DBresult *DBRes;
	char szBuffer[32];
	char cStato=DISTINTA_ERRATA;
	/*
	* Controllo l'esistenza del sottordine 
	*/
	DBRes=DBExecQuery(DEBUG_FLAG,"select cdstspe from carichi_distinte where cdnmdis=%d;",nDistinta);
	if(DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
	} else {
		strcpy(szBuffer,DBgetvalue(DBRes,0,0));
		if (strlen(szBuffer)>0){
			cStato=szBuffer[0];
		}
	}
	DBclear(DBRes);
	return cStato;
}

int GetDistintaUdc(char *szUdc)
{
	DBresult *DBRes;
	int nDistinta=0;
	/*
	* Controllo l'esistenza del sottordine 
	*/
	DBRes=DBExecQuery(DEBUG_FLAG,"select udnmdis from carichi_udc where udcdudc='%s';",szUdc);
	if(DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0){
	} else {
		nDistinta=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nDistinta;
}

ep_bool_t GetProdottoInUdc(char *szUDC,char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select pucdpro from carichi_prodotti_udc where pucdudc='%s' ;",szUDC); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0 ){
		bOK=FALSE;
	} else {
		bOK=TRUE;
		strcpy(szCDPRO,DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return bOK;
}

int GetQuantitaProdottoInUdc(char *szUDC,char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;
	int nNMCPE=0;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select punmcpe from carichi_prodotti_udc where pucdudc='%s' and pucdpro='%s' ;",szUDC,szCDPRO); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)==0 ){
		bOK=FALSE;
	} else {
		bOK=TRUE;
		nNMCPE=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nNMCPE;
}

ep_bool_t SetStatoDistinta(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstato='%c' where cdnmdis=%d and cdstato!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstato='%c' where cdnmdis=%d and cdstato!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoSpedizioneDistinta(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstspe='%c' where cdnmdis=%d and cdstspe!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoSpedizioneDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstspe='%c' where cdnmdis=%d and cdstspe!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoFlussoDistinta(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstfls='%c' where cdnmdis=%d and cdstfls!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoUdc(char *szUdc,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_udc set udstato='%c' where udcdudc='%s' and udstato!='%c';",cStato,szUdc,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoFlussoDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstfls='%c' where cdnmdis=%d and cdstfls!='%c';",cStato,nDistinta,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoProdottoUdc(char *szPRPRU,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_prodotti_udc set pustato='%c' where puprpru='%s' and pustato!='%c';",cStato,szPRPRU,cStato);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
	}
	if(atoi(DBcmdTuples(DBRes))==0){
		/* TODO */
	}
	DBclear(DBRes);

	return(bRetVal);
}

int GetNumeroProdottiSuUdc(char *szCDUDC)
{
	DBresult *DBRes;
	int nQT=0;
	/*
	* ottengo i dati del prelievo
	*/
	DBRes=DBExecQuery(DEBUG_FLAG>1,"select count(*) from carichi_prodotti_udc where pucdudc='%s' ;",szCDUDC); 
	if (DBresultStatus(DBRes) != DBRES_TUPLES_OK ){
		nQT=-1;
	} else {
		nQT=atoi(DBgetvalue(DBRes,0,0));
	}
	DBclear(DBRes);
	return nQT;
}



int CheckImportCarichi(void)
{
	DBresult *DBRes;
	DBresult *DBResUpdate;
	ep_bool_t bOK=TRUE;
	int nIndex=0;
	int nCarichi=0;
	char szCDPRO[32];
	char szSEGNO[32];
	int nNMCPE;
	int nPRCAR;

	/* seleziono i carichi importati */
	DBRes=DBExecQuery(DEBUG_FLAG>2,"select cacdpro,canmcpe,casegno,caprcar from carichi_import_carichi where cacdflg='%c'",CARICHI_IMPORT_CARICO_OK); 
	if (DBresultStatus(DBRes) == DBRES_TUPLES_OK ){
		nCarichi=DBntuples(DBRes);
		for (nIndex=0;nIndex<nCarichi;nIndex++){

			strcpy(szCDPRO, DBgetvalue(DBRes,nIndex,0));
			nNMCPE =   atoi(DBgetvalue(DBRes,nIndex,1));
			strcpy(szSEGNO, DBgetvalue(DBRes,nIndex,2));
			nPRCAR =   atoi(DBgetvalue(DBRes,nIndex,3));

			/* gestione movimenti in negativo */
			if(szSEGNO[0]=='-'){
				nNMCPE=-1*nNMCPE;
			}
			if(MovimentaProdotto("INGRESSO",szCDPRO,nNMCPE,0,"","importato","importato")){
				DBResUpdate=DBExecQuery(DEBUG_FLAG>1,"update carichi_import_carichi set cacdflg='%c' where caprcar=%d;" ,CARICHI_IMPORT_CARICO_MOVIMENTATO,nPRCAR); 
				if (DBresultStatus(DBResUpdate) != DBRES_COMMAND_OK ){
					bOK=FALSE;
				}
				DBclear(DBResUpdate);
			}
		}
	}
	DBclear(DBRes);
	if(nCarichi){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Importazione di %d carichi da carichi_import_carichi",nCarichi);
#endif
	}

	return nCarichi;
}

gchar *get_field_from_list_selected_row(gpointer win, gchar *lst_name, int column)
{
    GList *lista;
    GtkTreeIter iter;
    GtkWidget *lst = find_child(GTK_WIDGET(win), lst_name);
    gchar *value=NULL;

    lista=gtk_tree_selection_get_selected_rows(TREE_SELECTION_LST(lst), NULL);
    if(lista && g_list_length(lista)){
		char *ptr;

        do {
            if(gtk_tree_model_get_iter(TREE_MODEL_LST(lst), &iter, (GtkTreePath *)(lista->data))){

                gtk_tree_model_get_text(TREE_MODEL_LST(lst), &iter, column, &ptr); 
                value = g_strdup(ptr); 
            }
        } while((lista=g_list_next(lista)));
    }
    return value;
}


void do_action_using_ep_popover (gpointer win, EpPopoverField *fields, gchar **values, GtkButton *button, EpPopoverType type, action_callback action_cb)
{
    gchar *cdpro=NULL;
    gchar *cdudc=NULL;
    int qty=0;
    gboolean ok = FALSE;

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    EpPopover *ep = EP_POPOVER(ep_popover_new (GTK_WIDGET(button), type, GTK_BUTTONS_OK_CANCEL));

    gtk_popover_popup(GTK_POPOVER(ep));

	int i=0;
	while(fields[i] && values[i]){
		gtk_entry_set_text(GTK_ENTRY(ep_popover_get_field(ep, fields[i])), values[i]);
		i++;
	}

    int rc = ep_popover_run(ep);
    switch(rc){
        case GTK_RESPONSE_OK:
        {
            /* TODO - passare solo i campi associati al tipo popover */
            cdudc = (gchar *)gtk_entry_get_text(GTK_ENTRY(ep_popover_get_field(ep, EP_FIELD_UDC)));
            cdpro = (gchar *)gtk_entry_get_text(GTK_ENTRY(ep_popover_get_field(ep, EP_FIELD_PRODUCT)));
            qty = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(ep_popover_get_field(ep, EP_FIELD_QUANTITY)));
            int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,"Conferma operazione ?");
            switch(rc){
                case GTK_RESPONSE_OK:
                {
                    gchar *msg=NULL;

                    if(!(ok=action_cb(win, cdudc, cdpro, qty, &msg))){
                        /* TODO : messaggio in txt_msgs */
                        dlg_msg( GTK_WINDOW(win), "Errore", GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,"Errore in operazione:\n%s", msg?msg:"NON SPECIFICATO");
                    } else {
                        /* TODO : messaggio in txt_msgs */
                    }
					if(msg) g_free(msg);
                }
                break;
            }
        }
        break;
    }
    gtk_widget_destroy(GTK_WIDGET(ep));
}


/* --------------------------- */
/* Callbacks                   */
/* --------------------------- */


/*
* callback function called by ep_popover
*/
gboolean do_conteggio_movimentazioni(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
    gboolean ok = TRUE;
#ifdef TODO

    if(strlen(cdudc)!=7){                           ok=FALSE; msg = g_strdup_printf("Codice UdC [%s] di lunghezza errata", cdudc); }
    if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){ ok=FALSE; msg = g_strdup_printf("Codice prodotto [%s] di lunghezza errata", cdpro); }
    if(!CodiceProdottoEsistente(cdpro)) {           ok=FALSE; msg = g_strdup_printf("Codice prodotto [%s] non esistente", cdpro); }
    if(nQTPRO<=0){                                  ok=FALSE; msg = g_strdup_printf("Quantita' prodotto [%s] minore o uguale a 0 (%d)", cdpro, qty); }
    if(nQTPRO>500000){                              ok=FALSE; msg = g_strdup_printf("Quantita' prodotto [%s] maggiore di 500000 (%d)", cdpro, qty); }

    nQuantitaContata = GetContatoProdotto(cdpro);
    nQuantitaGiacente = GetGiacenzaProdotto(cdpro);
    if(nQuantitaGiacente-nQuantitaContata-nQTPRO<0){ ok=FALSE; msg = g_strdup_printf("Scarico maggiore di giacenza (%d-%d-%d < 0)", nQuantitaGiacente,nQuantitaContata,nQTPRO); }

    if(ok){
        InizioTransazione();
        if(MovimentaProdotto("CONTEGGIO",cdpro,qty,0,cdudc,Macchina.szCodice,Macchina.szCodice)){
            msg = g_strdup_printf("Conteggio udc [%s] prodotto [%s] quantità (%d) eseguito correttamente", cdudc, cdpro, qty);
        } else {
            ok=FALSE;
            msg = g_strdup_printf("Conteggio udc [%s] prodotto [%s] quantità (%d) non eseguito", cdudc, cdpro, qty);
        }
        FineTransazione(ok);
    }
#else
    *msg = g_strdup_printf("Conteggio udc [%s] prodotto [%s] quantità (%d) da implementare", cdudc, cdpro, qty);
    ok=FALSE;
#endif
    return ok;
}

/*
* callback function called by ep_popover
*/
gboolean do_differenza_inventariale(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
    gboolean ok = TRUE;

#ifdef TODO
    if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){ ok=FALSE; msg = g_strdup_printf("Codice prodotto [%s] di lunghezza errata", cdpro); }
    if(!CodiceProdottoEsistente(cdpro)) {           ok=FALSE; msg = g_strdup_printf("Codice prodotto [%s] non esistente", cdpro); }
    if(nQTPRO<=0){                                  ok=FALSE; msg = g_strdup_printf("Quantita' prodotto [%s] minore o uguale a 0 (%d)", cdpro, qty); }
    if(nQTPRO>500000){                              ok=FALSE; msg = g_strdup_printf("Quantita' prodotto [%s] maggiore di 500000 (%d)", cdpro, qty); }

    if(ok){
        InizioTransazione();
        if(MovimentaProdotto("DIFF_INV",szCDPRO,nQTPRO,0,"",Macchina.szCodice,Macchina.szCodice)){
            msg = g_strdup_printf("Differenza inventariale prodotto [%s] quantità (%d) eseguita correttamente", cdpro, qty);
        } else {
            ok=FALSE;
            msg = g_strdup_printf("Conteggio udc [%s] prodotto [%d] quantità (%d) non eseguito", cdudc, cdpro, qty);
        } else {
        }
        FineTransazione(ok);
    }
#else
    *msg = g_strdup_printf("Conteggio udc [%s] prodotto [%s] quantità (%d) da implementare", cdudc, cdpro, qty);
    ok=FALSE;
#endif
    return ok;
}






/* lista 'liste' */
/*
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
*/


void refresh_stack_main(gpointer win)
{
    g_printf("refresh_stack_main\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_main));
    if(w == priv->box_giacenze){
        refresh_box_giacenze(win);
    }
    if(w == priv->box_movimenti){
        refresh_box_movimenti(win);
    }
    if(w == priv->box_catalogo){
        refresh_box_catalogo(win);
    }
    if(w == priv->box_reports){
        refresh_box_reports(win);
    }
}

void refresh_box_giacenze(gpointer win)
{
    g_printf("refresh_box_giacenze\n");

    refresh_stack_giacenze(win);
}

void clear_box_movimenti(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_button_set_label( GTK_BUTTON(priv->movimenti_ricerca_pb_from_date),"...");
	gtk_button_set_label( GTK_BUTTON(priv->movimenti_ricerca_pb_to_date),"...");
	gtk_entry_set_text(GTK_ENTRY(priv->movimenti_ricerca_entry_udc), "");
	gtk_entry_set_text(GTK_ENTRY(priv->movimenti_ricerca_entry_prodotto), "");
	gtk_entry_set_text(GTK_ENTRY(priv->movimenti_ricerca_entry_titolo), "");
	gtk_combo_box_set_active  (GTK_COMBO_BOX(priv->movimenti_ricerca_cb_causale), 0);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->movimenti_ricerca_sb_distinta), 0.0);
}

void refresh_box_movimenti(gpointer win)
{
    g_printf("refresh_box_movimenti\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
	char szBuffer[32];

    priv->MovimentiDatiRicerca.szDataDal[0]='\0';
    priv->MovimentiDatiRicerca.szDataAl [0]='\0';
    priv->MovimentiDatiRicerca.szCodiceUdc[0]='\0';
    priv->MovimentiDatiRicerca.szCodiceProdotto[0]='\0';
    priv->MovimentiDatiRicerca.szTitoloProdotto[0]='\0';
    priv->MovimentiDatiRicerca.szCausale[0]='\0';

    const gchar *ptr = NULL;
	ptr = gtk_button_get_label( GTK_BUTTON(priv->movimenti_ricerca_pb_from_date));                     if(ptr) strcpy(priv->MovimentiDatiRicerca.szDataDal,       ptr);       
	ptr = gtk_button_get_label( GTK_BUTTON(priv->movimenti_ricerca_pb_to_date));                       if(ptr) strcpy(priv->MovimentiDatiRicerca.szDataAl ,       ptr);   
	ptr = gtk_entry_get_text(GTK_ENTRY(priv->movimenti_ricerca_entry_udc));                            if(ptr) strcpy(priv->MovimentiDatiRicerca.szCodiceUdc,     ptr); 
	ptr = gtk_entry_get_text(GTK_ENTRY(priv->movimenti_ricerca_entry_prodotto));                       if(ptr) strcpy(priv->MovimentiDatiRicerca.szCodiceProdotto,ptr); 
	ptr = gtk_entry_get_text(GTK_ENTRY(priv->movimenti_ricerca_entry_titolo));                         if(ptr) strcpy(priv->MovimentiDatiRicerca.szTitoloProdotto,ptr); 
	ptr = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->movimenti_ricerca_cb_causale));  if(ptr) strcpy(priv->MovimentiDatiRicerca.szCausale,       ptr); 
	priv->MovimentiDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->movimenti_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%DATADAL%", priv->MovimentiDatiRicerca.szDataDal,        TRUE);
	PS=add_item_to_parse(&PS,"%DATAAL%",  priv->MovimentiDatiRicerca.szDataAl,         FALSE);
	PS=add_item_to_parse(&PS,"%CDUDC%",   priv->MovimentiDatiRicerca.szCodiceUdc,      FALSE);
	PS=add_item_to_parse(&PS,"%CDPRO%",   priv->MovimentiDatiRicerca.szCodiceProdotto, FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",   priv->MovimentiDatiRicerca.szTitoloProdotto, FALSE);
	PS=add_item_to_parse(&PS,"%CDCAU%",   priv->MovimentiDatiRicerca.szCausale,        FALSE);

	if(priv->MovimentiDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and monmdis=%d ",priv->MovimentiDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}

    char szSelectCmd[4096];
    GetFileString("Movimentazioni","lista_movimentazioni", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(priv->sw_list_main_movimentazioni,"lst_movimentazioni", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);
}

void refresh_box_catalogo(gpointer win)
{
    g_printf("refresh_box_catalogo\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    char szSelectCmd[4096];
    GetFileString("Catalogo","lista_catalogo", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(priv->sw_list_main_catalogo,"lst_catalogo", szSelectCmd, GTK_SELECTION_MULTIPLE, NULL, NULL, NULL);
}

void refresh_box_reports(gpointer win)
{
    g_printf("refresh_box_reports\n");

    refresh_stack_reports(win);
}


void refresh_stack_giacenze(gpointer win)
{
    g_printf("\trefresh_stack_giacenze\n");

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

void clear_box_giacenze_totali(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo), "");
}


void on_giacenze_totali_selection_changed (GtkTreeSelection *selection, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	if(gtk_tree_selection_count_selected_rows ( selection)) {
		gtk_widget_set_sensitive(priv->pb_giacenze_totali_edit_prodotto,TRUE);
	} else {
		gtk_widget_set_sensitive(priv->pb_giacenze_totali_edit_prodotto,FALSE);
	}

}

void refresh_box_giacenze_totali(gpointer win)
{
    g_printf("\trefresh_box_giacenze_totali\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
	int nColonne;
	gchar *pszBuffer=NULL;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;

    gtk_widget_set_visible (priv->giacenze_ricerca_sb_distinta,      FALSE);  gtk_widget_set_visible (priv->lb_giacenze_distinta,   FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_udc,        FALSE);  gtk_widget_set_visible (priv->lb_giacenze_udc,        FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_prodotto,   TRUE);   gtk_widget_set_visible (priv->lb_giacenze_prodotto,   TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_titolo,     TRUE);   gtk_widget_set_visible (priv->lb_giacenze_dstit,     TRUE);

	strcpy(priv->GiacenzeDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc)));
	strcpy(priv->GiacenzeDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo)));
	priv->GiacenzeDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeDatiRicerca.szCodiceUdc, TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeDatiRicerca.szCodiceProdotto,  FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeDatiRicerca.szTitoloProdotto,  FALSE);

	if(priv->GiacenzeDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}

    /*
	gtk_widget_set_sensitive(priv->vb_giacenze_totali,      TRUE);
	gtk_widget_set_sensitive(priv->pb_bancali_set_distinta, TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_crea,        TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_restituzione,TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_evadi,       TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_storicizza,  TRUE);
    */

    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_totali", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nGiacenzeTotali=RefreshTable(priv->sw_list_giacenze_totali,"lst_giacenze_totali", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, G_CALLBACK(on_giacenze_totali_selection_changed), win);

	GtkWidget *lst = find_child(priv->sw_list_giacenze_totali, "lst_giacenze_totali");
	nColonne = gtk_tree_model_get_n_columns(TREE_MODEL_LST(lst));

	if(gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter)){

		do {

			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,4,&pszBuffer); nCopie = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,5,&pszBuffer); nConta = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,6,&pszBuffer); nDiff  = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,8,&pszBuffer); nSped  = atoi(pszBuffer); g_free(pszBuffer);

			if ((nDiff==0) & (nCopie==0)){
				strcpy(szColore,"MediumSeaGreen");
			} else if ((nDiff==0) & (nCopie!=0)){
				strcpy(szColore,"LightGreen");
			} else if (nSped==0){
				strcpy(szColore,"DarkKhaki");
			} else if (nConta==0){
				strcpy(szColore,"LightGreen");
			} else if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			} else if (nDiff>0){
				strcpy(szColore,"Orange");
			} else {
				strcpy(szColore,"Gainsboro");
			}
			/*
			* Se nDiff<0 metto comunque il rosso
			*/
			if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			}

			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-1, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-2, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-3, TRUE,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-4, FALSE,-1);
			
			
		} while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst) ,&iter));

		
	}
	gtk_widget_set_sensitive(priv->pb_giacenze_totali_edit_prodotto,FALSE);
}

void clear_box_giacenze_prodotti_udc(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta), -1.0);
}

void on_giacenze_prodotti_udc_selection_changed (GtkTreeSelection *selection, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	if(gtk_tree_selection_count_selected_rows ( selection)) {
		gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_elimina,       TRUE);
		gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_edit_prodotto, TRUE);
	} else {
		gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_elimina,       FALSE);
		gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_edit_prodotto, FALSE);
	}

}

void refresh_box_giacenze_prodotti_udc(gpointer win)
{
    g_printf("\trefresh_box_giacenze_prodotti_udc\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_widget_set_visible (priv->giacenze_ricerca_sb_distinta,      TRUE); gtk_widget_set_visible (priv->lb_giacenze_distinta,   TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_udc,        TRUE); gtk_widget_set_visible (priv->lb_giacenze_udc,        TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_prodotto,   TRUE); gtk_widget_set_visible (priv->lb_giacenze_prodotto,   TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_titolo,     TRUE); gtk_widget_set_visible (priv->lb_giacenze_dstit,     TRUE);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;

	strcpy(priv->GiacenzeDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc)));
	strcpy(priv->GiacenzeDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo)));
	priv->GiacenzeDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeDatiRicerca.szCodiceUdc, TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeDatiRicerca.szCodiceProdotto,  FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeDatiRicerca.szTitoloProdotto,  FALSE);

	if(priv->GiacenzeDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}
    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_prodotti_udc", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nProdotti=RefreshTable(priv->sw_list_giacenze_prodotti_udc,"lst_giacenze_prodotti_udc", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, G_CALLBACK(on_giacenze_prodotti_udc_selection_changed), win);
    gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_elimina,       FALSE);
    gtk_widget_set_sensitive( priv->pb_prodotti_bancalati_edit_prodotto, FALSE);
}

void clear_box_giacenze_udc(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo), "");
}

void on_giacenze_udc_selection_changed (GtkTreeSelection *selection, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	if(gtk_tree_selection_count_selected_rows ( selection)) {
		gtk_widget_set_sensitive( priv->pb_bancali_edit_bancale,        TRUE);
		gtk_widget_set_sensitive( priv->pb_bancali_set_distinta,        TRUE);
		gtk_widget_set_sensitive( priv->pb_bancali_stampa_packing_list, TRUE);
		gtk_widget_set_sensitive( priv->pb_bancali_elimina,             TRUE);
	} else {
		gtk_widget_set_sensitive( priv->pb_bancali_edit_bancale,        FALSE);
		gtk_widget_set_sensitive( priv->pb_bancali_set_distinta,        FALSE);
		gtk_widget_set_sensitive( priv->pb_bancali_stampa_packing_list, FALSE);
		gtk_widget_set_sensitive( priv->pb_bancali_elimina,             FALSE);
	}

}

void refresh_box_giacenze_udc(gpointer win)
{
    g_printf("\trefresh_box_giacenze_udc\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_widget_set_visible (priv->giacenze_ricerca_sb_distinta,      TRUE ); gtk_widget_set_visible (priv->lb_giacenze_distinta,   TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_udc,        FALSE); gtk_widget_set_visible (priv->lb_giacenze_udc,        FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_prodotto,   FALSE); gtk_widget_set_visible (priv->lb_giacenze_prodotto,   FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_titolo,     FALSE); gtk_widget_set_visible (priv->lb_giacenze_dstit,     FALSE);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUDC=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;

	strcpy(priv->GiacenzeDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc)));
	strcpy(priv->GiacenzeDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo)));
	priv->GiacenzeDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeDatiRicerca.szCodiceUdc, TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeDatiRicerca.szCodiceProdotto,  FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeDatiRicerca.szTitoloProdotto,  FALSE);

	if(priv->GiacenzeDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}
    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_udc", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nUDC=RefreshTable(priv->sw_list_giacenze_udc,"lst_giacenze_udc", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, G_CALLBACK(on_giacenze_udc_selection_changed), win);
	gtk_widget_set_sensitive( priv->pb_bancali_edit_bancale,        FALSE);
	gtk_widget_set_sensitive( priv->pb_bancali_set_distinta,        FALSE);
	gtk_widget_set_sensitive( priv->pb_bancali_stampa_packing_list, FALSE);
	gtk_widget_set_sensitive( priv->pb_bancali_elimina,             FALSE);
}

void clear_box_giacenze_distinte(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta), -1.0);
}

void on_giacenze_distinte_selection_changed (GtkTreeSelection *selection, gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	if(gtk_tree_selection_count_selected_rows ( selection)) {
		gtk_widget_set_sensitive( priv->pb_distinte_crea_file_dati, TRUE);
		gtk_widget_set_sensitive( priv->pb_distinte_restituzione,   TRUE);
		gtk_widget_set_sensitive( priv->pb_distinte_storicizza,     TRUE);
		gtk_widget_set_sensitive( priv->pb_distinte_elimina,        TRUE);
		gtk_widget_set_sensitive( priv->pb_distinte_edit_distinta,  TRUE);
	} else {
		gtk_widget_set_sensitive( priv->pb_distinte_crea_file_dati, FALSE);
		gtk_widget_set_sensitive( priv->pb_distinte_restituzione,   FALSE);
		gtk_widget_set_sensitive( priv->pb_distinte_storicizza,     FALSE);
		gtk_widget_set_sensitive( priv->pb_distinte_elimina,        FALSE);
		gtk_widget_set_sensitive( priv->pb_distinte_edit_distinta,  FALSE);
	}
}

void refresh_box_giacenze_distinte(gpointer win)
{
    g_printf("\trefresh_box_giacenze_distinte\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

    gtk_widget_set_visible (priv->giacenze_ricerca_sb_distinta,      TRUE ); gtk_widget_set_visible (priv->lb_giacenze_distinta,   TRUE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_udc,        FALSE); gtk_widget_set_visible (priv->lb_giacenze_udc,        FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_prodotto,   FALSE); gtk_widget_set_visible (priv->lb_giacenze_prodotto,   FALSE);
    gtk_widget_set_visible (priv->giacenze_ricerca_entry_titolo,     FALSE); gtk_widget_set_visible (priv->lb_giacenze_dstit,     FALSE);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUDC=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;

	strcpy(priv->GiacenzeDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc)));
	strcpy(priv->GiacenzeDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo)));
	priv->GiacenzeDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeDatiRicerca.szCodiceUdc, TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeDatiRicerca.szCodiceProdotto,  FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeDatiRicerca.szTitoloProdotto,  FALSE);

	if(priv->GiacenzeDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}
    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_distinte", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nDistinte=RefreshTable(priv->sw_list_giacenze_distinte,"lst_giacenze_distinte", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, G_CALLBACK(on_giacenze_distinte_selection_changed), win);
    gtk_widget_set_sensitive( priv->pb_distinte_crea_file_dati, FALSE);
    gtk_widget_set_sensitive( priv->pb_distinte_restituzione,   FALSE);
    gtk_widget_set_sensitive( priv->pb_distinte_storicizza,     FALSE);
    gtk_widget_set_sensitive( priv->pb_distinte_elimina,        FALSE);
    gtk_widget_set_sensitive( priv->pb_distinte_edit_distinta,  FALSE);
}


void refresh_stack_reports(gpointer win)
{
    g_printf("\trefresh_stack_reports\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_reports));
    if(w == priv->box_reports_storico_giacenze) {
        refresh_box_reports_storico_giacenze(win);
    }
    if(w == priv->box_reports_storico_movimenti){
        refresh_box_reports_storico_movimenti(win);
    }
    if(w == priv->box_reports_carichi_importati){
        refresh_box_reports_carichi_importati(win);
    }
}


void refresh_box_reports_storico_giacenze(gpointer win)
{
    g_printf("\t\trefresh_box_reports_storico_giacenze\n");

    refresh_stack_report_giacenze(win);
}

void refresh_box_reports_storico_movimenti(gpointer win)
{
    g_printf("\t\trefresh_box_reports_storico_movimenti\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	char szBuffer[32];
    const gchar *ptr=NULL;

	ptr = gtk_button_get_label( GTK_BUTTON(priv->reports_movimenti_pb_from_date));   if(ptr) strcpy(priv->MovimentiDatiRicerca.szDataDal,    ptr);       
	ptr = gtk_button_get_label( GTK_BUTTON(priv->reports_movimenti_pb_to_date));     if(ptr) strcpy(priv->MovimentiDatiRicerca.szDataAl,     ptr);       
	strcpy(priv->MovimentiDatiRicerca.szCodiceUdc,     gtk_entry_get_text(GTK_ENTRY(priv->reports_movimenti_entry_udc)));
	strcpy(priv->MovimentiDatiRicerca.szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(priv->reports_movimenti_entry_prodotto)));
	strcpy(priv->MovimentiDatiRicerca.szTitoloProdotto,gtk_entry_get_text(GTK_ENTRY(priv->reports_movimenti_entry_titolo)));
	strcpy(priv->MovimentiDatiRicerca.szCausale,       gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priv->reports_movimenti_cb_causale)));
	priv->MovimentiDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->reports_movimenti_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%DATADAL%", priv->MovimentiDatiRicerca.szDataDal,          TRUE);
	PS=add_item_to_parse(&PS,"%DATAAL%",  priv->MovimentiDatiRicerca.szDataAl,           FALSE);
	PS=add_item_to_parse(&PS,"%CDUDC%",   priv->MovimentiDatiRicerca.szCodiceUdc,        FALSE);
	PS=add_item_to_parse(&PS,"%CDPRO%",   priv->MovimentiDatiRicerca.szCodiceProdotto,   FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",   priv->MovimentiDatiRicerca.szTitoloProdotto,   FALSE);
	PS=add_item_to_parse(&PS,"%CDCAU%",   priv->MovimentiDatiRicerca.szCausale,          FALSE);

	if(priv->MovimentiDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and monmdis=%d ",priv->MovimentiDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}

    char szSelectCmd[4096];
    GetFileString("Movimentazioni","lista_movimentazioni", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    RefreshTable(priv->sw_list_stor_movimenti,"lst_movimenti", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);
}

void clear_box_reports_storico_movimenti(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    /* TODO */
}

void refresh_box_reports_carichi_importati(gpointer win)
{
    g_printf("\t\trefresh_box_reports_carichi_importati\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nCarichi=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;
	char szNMDOC[128];
	char szCDPRO[128];
	char szDSTIT[128];


	strcpy(priv->GiacenzeStorDatiRicerca.szNumeroDoc,       gtk_entry_get_text(GTK_ENTRY(priv->carichi_ricerca_entry_doc)));
	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->carichi_ricerca_entry_cdpro)));
	strcpy(priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->carichi_ricerca_entry_dstit)));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%NMDOC%",priv->GiacenzeStorDatiRicerca.szNumeroDoc,       TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  FALSE);

	// gtk_widget_set_sensitive(get_widget(main_window,"vb_giacenze_stor_totali"),TRUE);

    char szSelectCmd[4096];
    GetFileString("Carichi Importati","lista_carichi_importati", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nCarichi=RefreshTable(priv->sw_list_carichi_importati,"lst_carichi_importati", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);

	GtkWidget *lst = find_child(priv->sw_list_carichi_importati, "lst_carichi_importati");
	nColonne = gtk_tree_model_get_n_columns(TREE_MODEL_LST(lst));

	if(gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter)){
		int nOreDiff;

		do {

			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,12,&pszBuffer); nOreDiff=atoi(pszBuffer); g_free(pszBuffer);

			if ((nOreDiff>8)){
				strcpy(szColore,"OrangeRed");
			} else {
				strcpy(szColore,"White");
			}

			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-1, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-2, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-3, TRUE,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-4, FALSE,-1);
			
			
		} while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst) ,&iter));

		
	}
	
}

void clear_box_reports_carichi_importati(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
    /* TODO */
}


void refresh_stack_report_giacenze(gpointer win)
{
    g_printf("\t\trefresh_stack_report_giacenze\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);
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

void refresh_box_report_storico_giacenze_totali(gpointer win)
{
    g_printf("\t\trefresh_box_report_giacenze_totali\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	int nColonne;
	gchar *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;


	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_udc)));
	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_titolo)));
	priv->GiacenzeStorDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_stor_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeStorDatiRicerca.szCodiceUdc,     TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeStorDatiRicerca.szCodiceProdotto,FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeStorDatiRicerca.szTitoloProdotto,FALSE);

	if(priv->GiacenzeStorDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeStorDatiRicerca.nDistinta);
	}
	PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);

    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_totali_storico", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nDistinte=RefreshTable(priv->sw_list_giacenze_stor_totali,"lst_giacenze_totali_storico", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);

	GtkWidget *lst = find_child(priv->sw_list_giacenze_stor_totali, "lst_giacenze_totali_storico");
	nColonne = gtk_tree_model_get_n_columns(TREE_MODEL_LST(lst));

	if(gtk_tree_model_get_iter_first(TREE_MODEL_LST(lst), &iter)){

		do {

			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,4,&pszBuffer); nCopie = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,5,&pszBuffer); nConta = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,6,&pszBuffer); nDiff  = atoi(pszBuffer); g_free(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(lst),&iter,8,&pszBuffer); nSped  = atoi(pszBuffer); g_free(pszBuffer);

			if ((nDiff==0) & (nCopie==0)){
				strcpy(szColore,"MediumSeaGreen");
			} else if ((nDiff==0) & (nCopie!=0)){
				strcpy(szColore,"LightGreen");
			} else if (nSped==0){
				strcpy(szColore,"DarkKhaki");
			} else if (nConta==0){
				strcpy(szColore,"LightGreen");
			} else if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			} else if (nDiff>0){
				strcpy(szColore,"Orange");
			} else {
				strcpy(szColore,"Gainsboro");
			}
			/*
			* Se nDiff<0 metto comunque il rosso
			*/
			if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			}

			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-1, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-2, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-3, TRUE,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(lst), &iter, nColonne-4, FALSE,-1);
			
			
		} while(gtk_tree_model_iter_next(TREE_MODEL_LST(lst) ,&iter));
	}
}

void clear_box_report_storico_giacenze_totali(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_udc), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_prodotto), "");
	gtk_entry_set_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_titolo), "");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->giacenze_stor_ricerca_sb_distinta), -1.0);
}


void refresh_box_report_storico_giacenze_prodotti_udc(gpointer win)
{
    g_printf("\t\trefresh_box_report_giacenze_prodotti_udc\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;


	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_udc)));
	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_titolo)));
	priv->GiacenzeStorDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_stor_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeStorDatiRicerca.szCodiceUdc,     TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeStorDatiRicerca.szCodiceProdotto,FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeStorDatiRicerca.szTitoloProdotto,FALSE);

	if(priv->GiacenzeStorDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeStorDatiRicerca.nDistinta);
	}
	PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);

	// gtk_widget_set_sensitive(get_widget(main_window,"vb_giacenze_stor_totali"),TRUE);

    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_prodotti_udc_storico", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nProdotti=RefreshTable(priv->sw_list_giacenze_stor_prodotti_udc,"lst_giacenze_prodotti_udc_storico", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);
}

void clear_box_report_storico_giacenze_prodotti_udc(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
}

void refresh_box_report_storico_giacenze_udc(gpointer win)
{
    g_printf("\t\trefresh_box_report_giacenze_udc\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;


	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_udc)));
	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_titolo)));
	priv->GiacenzeStorDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_stor_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeStorDatiRicerca.szCodiceUdc,     TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeStorDatiRicerca.szCodiceProdotto,FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeStorDatiRicerca.szTitoloProdotto,FALSE);

	if(priv->GiacenzeStorDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeStorDatiRicerca.nDistinta);
	}
	PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);

	// gtk_widget_set_sensitive(get_widget(main_window,"vb_giacenze_stor_totali"),TRUE);

    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_udc_storico", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nUdc=RefreshTable(priv->sw_list_giacenze_stor_udc,"lst_giacenze_udc_storico", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);
}

void clear_box_report_storico_giacenze_udc(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
}

void refresh_box_report_storico_giacenze_distinte(gpointer win)
{
    g_printf("\t\trefresh_box_report_giacenze_distinte\n");

    MainWindowPrivate *priv = get_main_window_private_instance (win);

	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;


	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceUdc,       gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_udc)));
	strcpy(priv->GiacenzeStorDatiRicerca.szCodiceProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_prodotto)));
	strcpy(priv->GiacenzeStorDatiRicerca.szTitoloProdotto,  gtk_entry_get_text(GTK_ENTRY(priv->giacenze_stor_ricerca_entry_titolo)));
	priv->GiacenzeStorDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_stor_ricerca_sb_distinta));

    GList *PS=NULL;
	PS=add_item_to_parse(&PS,"%CDUDC%",priv->GiacenzeStorDatiRicerca.szCodiceUdc,     TRUE);
	PS=add_item_to_parse(&PS,"%CDPRO%",priv->GiacenzeStorDatiRicerca.szCodiceProdotto,FALSE);
	PS=add_item_to_parse(&PS,"%DSTIT%",priv->GiacenzeStorDatiRicerca.szTitoloProdotto,FALSE);

	if(priv->GiacenzeStorDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",priv->GiacenzeStorDatiRicerca.nDistinta);
	}
	PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);

	// gtk_widget_set_sensitive(get_widget(main_window,"vb_giacenze_stor_totali"),TRUE);

    char szSelectCmd[4096];
    GetFileString("Giacenze","lista_giacenze_distinte_storico", "", szSelectCmd, sizeof(szSelectCmd),priv->pszCfgFileName,NULL); 
    nDistinte=RefreshTable(priv->sw_list_giacenze_stor_distinte,"lst_giacenze_distinte_storico", szSelectCmd, GTK_SELECTION_MULTIPLE, PS, NULL, NULL);
}

void clear_box_report_storico_giacenze_distinte(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
}



void refresh_main_window(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);

    GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_main));
    if(w == priv->box_giacenze){
        g_printf("box_giacenze\n");
        GtkWidget *w = gtk_stack_get_visible_child(GTK_STACK(priv->stack_giacenze));
        if(w == priv->box_giacenze_totali){
            g_printf("\tbox_giacenze_totali\n");
        }
        if(w == priv->box_giacenze_prodotti_udc){
            g_printf("\tbox_giacenze_prodotti_udc\n");
        }
        if(w == priv->box_giacenze_udc){
            g_printf("\tbox_giacenze_udc\n");
        }
        if(w == priv->box_giacenze_distinte){
            g_printf("\tbox_giacenze_distinte\n");
        }
    }
    if(w == priv->box_movimenti){
        g_printf("box_movimenti\n");
    }
    if(w == priv->box_catalogo){
        g_printf("box_catalogo\n");
    }
    if(w == priv->box_reports){
        g_printf("box_reports\n");
    }

#ifdef TODO
	{
		case MAIN_WINDOW_GIACENZE_TASK:
			gtk_widget_show(priv->pb_edit);
			switch (gtk_notebook_get_current_page(GTK_NOTEBOOK(priv->nb_giacenze)))
			{
				case GIACENZE_TOTALI_TASK:
				break;
				case GIACENZE_PRODOTTI_UDC_TASK:
				break;
				case GIACENZE_UDC_TASK:
				break;
				case GIACENZE_DISTINTE_TASK:
				break;
			}
		break;
		case MAIN_WINDOW_MOVIMENTAZIONI_TASK:
			gtk_widget_hide(priv->pb_edit);
			RefreshMainWindowMovimentazioni();
		break;
		case MAIN_WINDOW_CATALOGO_TASK:
			gtk_widget_hide(priv->pb_edit);
			RefreshMainWindowCatalogo();
		break;
		case MAIN_WINDOW_REPORT_TASK:
			gtk_widget_hide(priv->pb_edit);
		break;
	}
#endif

}


void RefreshMainWindowGiacenze(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
#ifdef TODO
	int nColonne;
	char *pszBuffer;
	char szColore[32];
	GtkTreeIter iter;
	char szBuffer[32];
	int nGiacenzeTotali=0;
	int nDistinte=0;
	int nProdotti=0;
	int nUdc=0;
	int nDiff=0;
	int nSped=0;
	int nConta=0;
	int nCopie=0;

	strcpy(GiacenzeDatiRicerca.szCodiceUdc,gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_udc)));
	sprintf(szBuffer,"%s",GiacenzeDatiRicerca.szCodiceUdc);
	PS=add_item_to_parse(&PS,"%CDUDC%",szBuffer,FALSE);

	strcpy(GiacenzeDatiRicerca.szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_prodotto)));
	sprintf(szBuffer,"%s",GiacenzeDatiRicerca.szCodiceProdotto);
	PS=add_item_to_parse(&PS,"%CDPRO%",szBuffer,FALSE);

	strcpy(GiacenzeDatiRicerca.szTitoloProdotto,gtk_entry_get_text(GTK_ENTRY(priv->giacenze_ricerca_entry_titolo)));
	sprintf(szBuffer,"%s",GiacenzeDatiRicerca.szTitoloProdotto);
	PS=add_item_to_parse(&PS,"%DSTIT%",szBuffer,FALSE);

	GiacenzeDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->giacenze_ricerca_sb_distinta));
	if(GiacenzeDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and cdnmdis=%d ",GiacenzeDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}
	gtk_widget_set_sensitive(priv->vb_giacenze_totali,       TRUE);
	gtk_widget_set_sensitive(priv->pb_bancali_set_distinta,  TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_crea,         TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_restituzione, TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_evadi,        TRUE);
	gtk_widget_set_sensitive(priv->pb_distinte_storicizza,   TRUE);

	nGiacenzeTotali=RefreshTable(priv->eb_giacenze_totali,"sw_list_giacenze_totali","lst_giacenze_totali",NULL,Cfg.szCfgFileName,"Giacenze","lista_giacenze_totali",GTK_SELECTION_EXTENDED,PS);
	nProdotti=RefreshTable(priv->eb_giacenze_prodotti_udc,"sw_list_giacenze_prodotti_udc","lst_giacenze_prodotti_udc",NULL,Cfg.szCfgFileName,"Giacenze","lista_giacenze_prodotti_udc",GTK_SELECTION_EXTENDED,PS);
	nUdc=RefreshTable(priv->eb_giacenze_udc,"sw_list_giacenze_udc","lst_giacenze_udc",NULL,Cfg.szCfgFileName,"Giacenze","lista_giacenze_udc",GTK_SELECTION_EXTENDED,PS);
	nDistinte=RefreshTable(priv->eb_giacenze_distinte,"sw_list_giacenze_distinte","lst_giacenze_distinte",NULL,Cfg.szCfgFileName,"Giacenze","lista_giacenze_distinte",GTK_SELECTION_EXTENDED,PS);

	nColonne=gtk_tree_model_get_n_columns(TREE_MODEL_LST(priv->eb_giacenze_totali));
	if(gtk_tree_model_get_iter_first(TREE_MODEL_LST(priv->eb_giacenze_totali) ,&iter)){
		do {
			gtk_tree_model_get_text(TREE_MODEL_LST(priv->eb_giacenze_totali),&iter,4,&pszBuffer);
			nCopie=atoi(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(priv->eb_giacenze_totali),&iter,5,&pszBuffer);
			nConta=atoi(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(priv->eb_giacenze_totali),&iter,6,&pszBuffer);
			nDiff=atoi(pszBuffer);
			gtk_tree_model_get_text(TREE_MODEL_LST(priv->eb_giacenze_totali),&iter,8,&pszBuffer);
			nSped=atoi(pszBuffer);
			if ((nDiff==0) & (nCopie==0)){
				strcpy(szColore,"MediumSeaGreen");
			} else if ((nDiff==0) & (nCopie!=0)){
				strcpy(szColore,"LightGreen");
			} else if (nSped==0){
				strcpy(szColore,"DarkKhaki");
			} else if (nConta==0){
				strcpy(szColore,"LightGreen");
			} else if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			} else if (nDiff>0){
				strcpy(szColore,"Orange");
			} else {
				strcpy(szColore,"Gainsboro");
			}
			/*
			* Se nDiff<0 metto comunque il rosso
			*/
			if (nDiff<0){
				strcpy(szColore,"OrangeRed");
			}

			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(priv->eb_giacenze_totali), &iter, nColonne-1, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(priv->eb_giacenze_totali), &iter, nColonne-2, szColore,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(priv->eb_giacenze_totali), &iter, nColonne-3, TRUE,-1);
			gtk_list_store_set((GtkListStore *)TREE_MODEL_LST(priv->eb_giacenze_totali), &iter, nColonne-4, FALSE,-1);
			
			
		} while(gtk_tree_model_iter_next(TREE_MODEL_LST(priv->eb_giacenze_totali) ,&iter));
		g_free(pszBuffer);
	}
#endif	
}


void RefreshMainWindowMovimentazioni(gpointer win)
{
    MainWindowPrivate *priv = get_main_window_private_instance (win);
#ifdef TODO
	char szBuffer[32];
	time_t t_dal,t_al;
	struct tm * tmtime;

	t_dal=gnome_date_edit_get_time((GnomeDateEdit *)priv->movimentazioni_ricerca_de_dal);
	tmtime = localtime(&t_dal);
	strftime(MovimentiDatiRicerca.szDataDal,128,"%Y-%m-%d",tmtime);

	t_al=gnome_date_edit_get_time((GnomeDateEdit *)priv->movimentazioni_ricerca_de_al);
	tmtime = localtime(&t_al);
	strftime(MovimentiDatiRicerca.szDataAl,128,"%Y-%m-%d",tmtime);

	PS=add_item_to_parse(&PS,"%DATADAL%",MovimentiDatiRicerca.szDataDal,FALSE);
	PS=add_item_to_parse(&PS,"%DATAAL%",MovimentiDatiRicerca.szDataAl,FALSE);

	strcpy(MovimentiDatiRicerca.szCodiceUdc,gtk_entry_get_text(GTK_ENTRY(priv->movimentazioni_ricerca_entry_udc)));
	sprintf(szBuffer,"%s",MovimentiDatiRicerca.szCodiceUdc);
	PS=add_item_to_parse(&PS,"%CDUDC%",szBuffer,FALSE);

	strcpy(MovimentiDatiRicerca.szCodiceProdotto,gtk_entry_get_text(GTK_ENTRY(priv->movimentazioni_ricerca_entry_prodotto)));
	sprintf(szBuffer,"%s",MovimentiDatiRicerca.szCodiceProdotto);
	PS=add_item_to_parse(&PS,"%CDPRO%",szBuffer,FALSE);

	strcpy(MovimentiDatiRicerca.szTitoloProdotto,gtk_entry_get_text(GTK_ENTRY(priv->movimentazioni_ricerca_entry_titolo)));
	sprintf(szBuffer,"%s",MovimentiDatiRicerca.szTitoloProdotto);
	PS=add_item_to_parse(&PS,"%DSTIT%",szBuffer,FALSE);

	MovimentiDatiRicerca.nDistinta = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(priv->movimentazioni_ricerca_sb_distinta));
	if(MovimentiDatiRicerca.nDistinta<0){
		sprintf(szBuffer," ");
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	} else {
		sprintf(szBuffer," and monmdis=%d ",MovimentiDatiRicerca.nDistinta);
		PS=add_item_to_parse(&PS,"%NMDIS%",szBuffer,FALSE);
	}

	strcpy(MovimentiDatiRicerca.szCausale,gtk_combo_box_get_active_text(GTK_COMBO_BOX(priv->movimentazioni_ricerca_cb_causale)));
	sprintf(szBuffer,"%s",MovimentiDatiRicerca.szCausale);
	PS=add_item_to_parse(&PS,"%CDCAU%",szBuffer,FALSE);


	RefreshTable(priv->eb_main_movimentazioni,"sw_list_main_movimentazioni","lst_movimentazioni",NULL,Cfg.szCfgFileName,"Movimentazioni","lista_movimentazioni",GTK_SELECTION_EXTENDED,PS);
#endif
}


ep_bool_t StampaEtichettaUdc(char *szCodiceUdc)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s,%s", szCodiceUdc,Cfg.szLabelPrinterConsole);


	//SendMessage(ProcList,PROC_PRINTLABEL,Macchina.nNumero,STAMPA_ETICHETTA_UDC,szBuffer);
	return TRUE;
}


/*
* callback function called by ep_popover
*/
gboolean do_crea_udc(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	gboolean rc=TRUE;
	int nIndex;
	int nQuantitaInserita;
	int nUdcCreati=0;
	char szCodiceUdc[128];

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");

	nQuantitaInserita = qty;
	nQuantitaInserita=min(nQuantitaInserita,100);
	for(nIndex=0;nIndex<nQuantitaInserita;nIndex++){
		nUdcCreati=nUdcCreati+1;
		GetNuovoCodiceUdc(szCodiceUdc);
		if (!StampaEtichettaUdc(szCodiceUdc)){
			rc=FALSE;
		} else {
			trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"StampaEtichettaUdc [%s]",szCodiceUdc);
		}
	}


	trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"Creati %d nuovi UdC",nUdcCreati);

	return TRUE;
}

/*
* callback function called by ep_popover
*/
gboolean do_stampa_etichetta(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	GtkWidget *dlg;
	gboolean rc=TRUE;
	char szCodiceUdc[32];

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");

	if (StampaEtichettaUdc(cdudc)){
		trace_debug_gtk(NULL, TRUE, TRUE, txt_msgs,"StampaEtichettaUdc [%s]",cdudc);
	} else {
		rc=FALSE;
	}

	return rc;
}

/*
* callback function called by ep_popover
*/
gboolean do_giacenze_totali_ingresso(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	char szMsg[256];
	ep_bool_t bOK=TRUE;

	if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){
		bOK=FALSE;
		strcpy(szMsg,"Codice prodotto di lunghezza errata");
	} else {
		if(qty<=0){
			bOK=FALSE;
			strcpy(szMsg,"Quantita' prodotto minore o uguale a 0");
		} else {
			if(qty>500000){
				bOK=FALSE;
				strcpy(szMsg,"Quantita' prodotto maggiore di 500000");
			}
		}
	}

	if(bOK){
		int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi il carico dall'applicazione ?");
		switch(rc){
			case GTK_RESPONSE_YES:
				if (!CodiceProdottoEsistente(cdpro)) {
					bOK=FALSE;
					strcpy(szMsg,"Prodotto non esistente !");
				} else {
					if(!MovimentaProdotto("INGRESSO",cdpro,qty,0,"",Macchina.szCodice,Macchina.szCodice)){
						bOK=FALSE;
						strcpy(szMsg,"Errore in movimentazione prodotto");
					} else {
						sprintf(szMsg,"Prodotto [%s][%d] caricato", cdpro, qty);
					}
				}
			break;
		}
	}

	*msg = g_strdup(szMsg);
	return bOK;
}

/*
* callback function called by ep_popover
*/
gboolean do_giacenze_totali_spedizione(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	char szMsg[256];
	ep_bool_t bOK=TRUE;

	if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){
		bOK=FALSE;
		strcpy(szMsg,"Codice prodotto di lunghezza errata");
	} else {
		if(qty<=0){
			bOK=FALSE;
			strcpy(szMsg,"Quantita' prodotto minore o uguale a 0");
		} else {
			if(qty>500000){
				bOK=FALSE;
				strcpy(szMsg,"Quantita' prodotto maggiore di 500000");
			} else {
				int nQuantitaGiacente=GetGiacenzaProdotto(cdpro);
				if(nQuantitaGiacente-qty<0){
					bOK=FALSE;
					strcpy(szMsg,"Scarico maggiore di giacenza");
				}
			}
		}
	}


	if(bOK){
		int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi lo scarico ?");
		switch(rc){
			case GTK_RESPONSE_YES:
				if(!MovimentaProdotto("SPEDIZIONE",cdpro,qty,0,"",Macchina.szCodice,Macchina.szCodice)){
					bOK=FALSE;
					strcpy(szMsg,"Errore in movimentazione prodotto");
				} else {
					sprintf(szMsg,"Prodotto [%s][%d] scaricato", cdpro, qty);
				}
			break;
		}
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

/*
* callback function called by ep_popover
*/
gboolean do_giacenze_totali_conteggia(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	char szMsg[256];
	ep_bool_t bOK=TRUE;

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");

	if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){
		bOK=FALSE;
		strcpy(szMsg,"Codice prodotto di lunghezza errata");
	} else {
		if(qty<=0){
			bOK=FALSE;
			strcpy(szMsg,"Quantita' prodotto minore o uguale a 0");
		} else {
			if(qty>500000){
				bOK=FALSE;
				strcpy(szMsg,"Quantita' prodotto maggiore di 500000");
			} else {
				int nQuantitaContata=GetContatoProdotto(cdpro);
				int nQuantitaGiacente=GetGiacenzaProdotto(cdpro);
				if(nQuantitaGiacente-nQuantitaContata-qty<0){
					bOK=FALSE;
					strcpy(szMsg,"Scarico maggiore di giacenza");
				}
			}
		}
	}


	if(bOK){
		int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi il conteggio ?");
		switch(rc){
			case GTK_RESPONSE_YES:
				InizioTransazione();
				if(!MovimentaProdotto("CONTEGGIO",cdpro,qty,0,cdudc,Macchina.szCodice,Macchina.szCodice)){
					bOK=FALSE;
					sprintf(szMsg,"errore in conteggio prodotto [%s][%d]", cdpro, qty);
				} else {
					sprintf(szMsg,"Prodotto [%s][%d] conteggiato correttamente", cdpro, qty);
				}
				FineTransazione(bOK);
			break;
		}
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

/*
* callback function called by ep_popover
*/
gboolean do_giacenze_totali_differenza_inventariale(gpointer win, gchar *cdudc, gchar *cdpro, int qty, gchar **msg)
{
	char szMsg[256];
	ep_bool_t bOK=TRUE;

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");

	if(strlen(cdpro)!=CARICHI_PRODUCT_CODE_LENGTH){
		bOK=FALSE;
		strcpy(szMsg,"Codice prodotto di lunghezza errata");
	} else {
		if(qty<=0){
			bOK=FALSE;
			strcpy(szMsg,"Quantita' prodotto minore o uguale a 0");
		} else {
			if(qty>500000){
				bOK=FALSE;
				strcpy(szMsg,"Quantita' prodotto maggiore di 500000");
			} else {
				int nQuantitaGiacente=GetGiacenzaProdotto(cdpro);
				if(nQuantitaGiacente-qty<0){
					bOK=FALSE;
					strcpy(szMsg,"Scarico maggiore di giacenza");
				}
			}
		}
	}


	if(bOK){
		int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi lo scarico ?");
		switch(rc){
			case GTK_RESPONSE_YES:
				if(!MovimentaProdotto("DIFF_INV",cdpro,qty,0,"",Macchina.szCodice,Macchina.szCodice)){
					bOK=FALSE;
					strcpy(szMsg,"Errore in movimentazione prodotto");
					sprintf(szMsg,"Errore in movimentazione prodotto [%s][%d]", cdpro, qty);
				} else {
					sprintf(szMsg,"Prodotto [%s][%d] movimentato correttamente", cdpro, qty);
				}
			break;
		}
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

/*
* callback function called by action
*/
gboolean do_giacenze_totali_storicizza(gpointer win)
{
	DBresult *DBRes=NULL;
	int nTuples=0;
	int nIndex=0;
	DBresult *DBResDistinte=NULL;
	int nTuplesDistinte=0;
	int nIndexDistinte=0;
	char cStatoDistinta;
	char cStatoSpedizioneDistinta;
	ep_bool_t bOK=TRUE;
	ep_bool_t bTrOK=TRUE;
	char szCDPRO[128];
	char szCDPROIncompatibile[128];
	int nDistinta=0;
	DBresult *DBResProdotti=NULL;
	int nTuplesProdotti=0;
	int nIndexProdotti=0;

    MainWindowPrivate *priv = get_main_window_private_instance (win);
    GtkWidget *txt_msgs = find_child(GTK_WIDGET(win),"txt_msgs");

	/*
	* Cerco i prodotti con differenza tra giacenti e contati=0
	*/
	DBRes=DBExecQuery(Cfg.nDebugLevel>1,"select cgcdpro from carichi_giacenze where cgnmdif=0;");
	nTuples=DBntuples(DBRes);

	for (nIndex=0;nIndex<nTuples;nIndex++){
		bOK=TRUE;
		strcpy(szCDPRO,DBgetvalue(DBRes,nIndex,0));
		
		/*
		* Cerco le distinte con dentro il prodotto
		*/
		DBResDistinte=DBExecQuery(Cfg.nDebugLevel>1,"select distinct(udnmdis) from carichi_udc,carichi_prodotti_udc where pucdudc=udcdudc and pucdpro='%s' and udnmdis!=0;",szCDPRO);
		nTuplesDistinte=DBntuples(DBResDistinte);
		for (nIndexDistinte=0;nIndexDistinte<nTuplesDistinte;nIndexDistinte++){
			nDistinta=atoi(DBgetvalue(DBResDistinte,nIndexDistinte,0));
			/*
			* Storicizzo prima le distinte le distinte
			* IMPORTANTISSIMO
			* Per ogni distinta in cui il prodotto e' presente
			* devo verificare che tutti i prodotti e gli udc in essa siano compatibili
			* con la storicizzazione.
			* Ad esempio che non esistano in distinta altri prodotti che abbiano diff!=0
			* Il rischio e' quello di storicizzare roba che poi non mi permette le
			* corrette visualizzazioni con le query in ael.cfg
			*/

			DBResProdotti=DBExecQuery(Cfg.nDebugLevel>1,"select distinct(pucdpro) from carichi_distinte, carichi_udc,carichi_prodotti_udc, carichi_giacenze where cgcdpro=pucdpro and cdnmdis=udnmdis and pucdudc=udcdudc and cdnmdis=%d and cgnmdif!=0;", nDistinta);
			nTuplesProdotti=DBntuples(DBResProdotti);
			for (nIndexProdotti=0;nIndexProdotti<nTuplesProdotti;nIndexProdotti++){
				strcpy(szCDPROIncompatibile,DBgetvalue(DBResProdotti,nIndexProdotti,0));
				trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Attenzione: Prodotto %s in distinta %d con differenza diversa da 0, correggere",szCDPROIncompatibile,nDistinta);
			}
			DBclear(DBResProdotti);

			/*
			* Poi devo verificare anche la distinta stessa
			*/
			cStatoDistinta=GetStatoDistinta(nDistinta);
			cStatoSpedizioneDistinta=GetStatoSpedizioneDistinta(nDistinta);
			if ((cStatoDistinta==DISTINTA_RESTITUITA) && ((cStatoSpedizioneDistinta==DISTINTA_SPEDITA || cStatoSpedizioneDistinta==DISTINTA_SPEDITA_STAMPATA)) ){
				InizioTransazione();
				if(StoricizzaDistinta(nDistinta)){
					trace_debug_gtk("BLUE",TRUE,TRUE,txt_msgs,"Distinta %d Storicizzata",nDistinta);
					bTrOK=TRUE;
				} else {
					trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Attenzione: errore in storicizzazione distinta %d",nDistinta);
					bTrOK=FALSE;
					bOK=FALSE;
				}
				FineTransazione(bTrOK);
			} else {
				trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Attenzione: Prodotto %s non storicizzato perche' la distinta %d non e' stata restituita o non ne e' stato fatto il file dati",szCDPRO, nDistinta);
				bOK=FALSE;
			}

		}
		DBclear(DBResDistinte);

		/*
		* Storicizzo la giacenza
		*/
		InizioTransazione();
		if (bOK){
			if (StoricizzaProdottoInGiacenza(szCDPRO)) {
				trace_debug_gtk("BLUE",TRUE,TRUE,txt_msgs,"Prodotto %s Storicizzato", szCDPRO);
			} else {
				trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Attenzione: errore in storicizzazione prodotto %s ", szCDPRO);
				bOK=FALSE;
			}
		}
		FineTransazione(bOK);
	}
	DBclear(DBRes);

	return bOK;
}

gboolean do_annulla_movimento(gpointer win, gchar *prgmov, gchar **msg)
{
	ep_bool_t bOK=TRUE;
	char szMsg[256];

	int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi l'annullamento del movimento ?");
	switch(rc){
		case GTK_RESPONSE_YES:
		{
			int nProgressivo = atoi(prgmov);

			if(!AnnullaMovimentazione(nProgressivo)){
				strcpy(szMsg,"Errore in annullamento movimento");
				bOK=FALSE;
			} else {
				sprintf(szMsg,"Movimento [%d] annullato correttamente", nProgressivo);
			}
		}
		break;
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

gboolean do_nuova_distinta(gpointer win, gchar **msg)
{

	ep_bool_t bOK=TRUE;
	char szMsg[256];

	int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi la creazione di una nuova distinta?");
	switch(rc){
		case GTK_RESPONSE_YES:
		{
			int nDistinta;

			if((nDistinta=CreaDistinta())){
				sprintf(szMsg, "Creata distinta %d",nDistinta);
			} else {
				sprintf(szMsg, "Errore in creazione distinta");
			}
		}
		break;
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

gboolean do_elimina_distinta(gpointer win, gchar *nmdis, gchar **msg)
{
	ep_bool_t bOK=TRUE;
	char szMsg[256];

	int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi la eliminazione della distinta selezionata ?");
	switch(rc){
		case GTK_RESPONSE_YES:
		{
			int nDistinta=atoi(nmdis);
			ep_bool_t bCommit=TRUE;

			InizioTransazione();
			if(!EliminaDistinta(nDistinta)){
				bCommit=FALSE;
				bOK=FALSE;
				sprintf(szMsg, "Errore in eliminazione  distinta [%s]", nmdis);
			} else {
				sprintf(szMsg, "Eliminata  distinta [%s]", nmdis);
			}
			FineTransazione(bCommit);
		}
		break;
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

gboolean do_restituzione_distinta(gpointer win, gchar *nmdis, gchar **msg)
{
	ep_bool_t bOK=TRUE;
	char szMsg[256];

	int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi la restituzione della distinta selezionata ?");
	switch(rc){
		case GTK_RESPONSE_YES:
		{
			int nDistinta=atoi(nmdis);
			ep_bool_t bCommit=TRUE;

			if(!RestituzioneDistinta(nDistinta,Macchina.szCodice,Macchina.szCodice)){
				bOK=FALSE;
				sprintf(szMsg, "Errore in restituzionre  distinta [%s]", nmdis);
			} else {
				sprintf(szMsg, "Restituita  distinta [%s]", nmdis);
			}
		}
		break;
	}

	*msg = g_strdup(szMsg);

	return bOK;
}

gboolean do_storicizza_distinta(gpointer win, gchar *nmdis, gchar **msg)
{
	ep_bool_t bOK=TRUE;
	char szMsg[256];

	int rc=dlg_msg( GTK_WINDOW(win), "Conferma", GTK_MESSAGE_QUESTION,GTK_BUTTONS_YES_NO,"Confermi la storicizzazione della distinta selezionata ?");
	switch(rc){
		case GTK_RESPONSE_YES:
		{
			int nDistinta=atoi(nmdis);

			InizioTransazione();
			if(!StoricizzaDistinta(nDistinta)){
				bOK=FALSE;
				sprintf(szMsg, "Errore in storicizzazione  distinta [%s]", nmdis);
			} else {
				sprintf(szMsg, "Storicizzata distinta [%s]", nmdis);
			}
			FineTransazione(bOK);
		}
		break;
	}

	*msg = g_strdup(szMsg);

	return bOK;
}
