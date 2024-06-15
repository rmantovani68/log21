/*
* modulo : dbfun-deltacarichi.c
* --------------------
* Progetto Easy Picking - DB Management - PostgreSQL - GTK
*
* Data creazione 18/06/2002
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2002
*
* Modifiche
* -------------   -------------------------------------------------------
* rm 18-06-2002 : Creazione
* -------------   -------------------------------------------------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <glib.h>

#if defined(TRACE) || defined(TRACE_SQL)
	#include <trace.h>
#endif

#include <ep-common.h>
#include <picking.h>
#include <ep-db.h>
#include <dbfun-deltacarichi.h>

/*****************************************************************************
* INIZIO LIBRERIA CHIAMATE AL DATABASE
******************************************************************************/

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
	/* Ricavo la quantita giacente */
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
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_udc_stor select * from carichi_udc where udnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_distinte_stor select * from carichi_distinte where cdnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_prodotti_udc where pucdudc in (select udcdudc from carichi_udc where udnmdis=%d) ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_udc where udnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		} else {
			bOK=TRUE;
		}
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_distinte where cdnmdis=%d ;",nDistinta);
		if(DBresultStatus(DBRes) != DBRES_TUPLES_OK){
			bOK=FALSE;
		} else {
			bOK=TRUE;
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
	} else {
	}

	// Eseguo la cancellazione della distinta e setto il num distinta degli udc a 0
	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_udc set udnmdis=0 where udnmdis=%d ;",nDistinta);
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_distinte where cdnmdis=%d ;",nDistinta);
		DBclear(DBRes);
	} else {
	}
	
	return bOK;
}


ep_bool_t StoricizzaProdottoInGiacenza(char *szCDPRO)
{
	DBresult *DBRes;
	ep_bool_t bOK=TRUE;


	if (bOK) {
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_giacenze_stor select * from carichi_giacenze where cgcdpro='%s';",szCDPRO);
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"insert into carichi_movimentazioni_stor select * from carichi_movimentazioni where mocdpro='%s';",szCDPRO);
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_giacenze where  cgcdpro='%s';",szCDPRO);
		DBclear(DBRes);
		DBRes=DBExecQuery(DEBUG_FLAG>1,"delete from carichi_movimentazioni where  mocdpro='%s';",szCDPRO);
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
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstato='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstato='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoSpedizioneDistinta(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstspe='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoSpedizioneDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstspe='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoFlussoDistinta(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte set cdstfls='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}


ep_bool_t SetStatoUdc(char *szUdc,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_udc set udstato='%c' where udcdudc='%s';",cStato,szUdc);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoFlussoDistintaStorico(int nDistinta,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_distinte_stor set cdstfls='%c' where cdnmdis=%d;",cStato,nDistinta);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
		//trace_debug_gtk("RED",TRUE,TRUE,txt_msgs,"Errore in update su ord_prod [%d]",nProgressivoSottordine);
	}
	DBclear(DBRes);

	return(bRetVal);
}

ep_bool_t SetStatoProdottoUdc(char *szPRPRU,char cStato)
{
	ep_bool_t bRetVal=TRUE;
	DBresult *DBRes = NULL;
	
	DBRes=DBExecQuery(DEBUG_FLAG>1,"update carichi_prodotti_udc set pustato='%c' where puprpru='%s';",cStato,szPRPRU);
	if (DBresultStatus(DBRes) != DBRES_COMMAND_OK ) {
		bRetVal=FALSE;
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
