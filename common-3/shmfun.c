/*
* shmfun.c : stampa shm
*/
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>


#include <libmemcached/memcached.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <picking.h>
#include <proc_list.h>

#include "shm.h"


extern char __configuration_file__[];

/*
* void ReadConfiguration(void)
* lettura della configurazione dal file 
*/
void ReadConfiguration(PCFGSTRUCT pCfg)
{
	char szParagraph[128];
	char szCurrentDirectory[128];


	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	
	strcpy(pCfg->szCniCfg,szCurrentDirectory);
	strcat(pCfg->szCniCfg,"/");
	strcat(pCfg->szCniCfg,__configuration_file__);

	strcpy(szParagraph,"General Settings");
	GetFileString(szParagraph,"PathData",   "../data",     pCfg->szPathData,     80, pCfg->szCniCfg, NULL);
	GetFileString(szParagraph,"PathExe",    "./",          pCfg->szPathExe,      80, pCfg->szCniCfg, NULL);
	GetFileString(szParagraph,"PathTrace",  "../trace",    pCfg->szPathTrace,    80, pCfg->szCniCfg, NULL);

	pCfg->nShmKey         = GetFileInt(szParagraph,"ShmKey",         256, pCfg->szCniCfg,NULL);
	pCfg->nSettoriShmKey  = GetFileInt(szParagraph,"SettoriShmKey",  257, pCfg->szCniCfg,NULL);

	pCfg->nNumeroSettori  = GetFileInt("settori","NumeroSettori",12,  pCfg->szCniCfg,NULL);
	

	GetFileString("netshm","MemCachedServerName",  "localhost",    pCfg->szMemCachedServerName,    80, pCfg->szCniCfg, NULL);
	pCfg->nMemCachedPortNumber  = GetFileInt("netshm","MemCachedPortNumber",11211,  pCfg->szCniCfg,NULL);

}

BOOL setup_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
	BOOL bOK=TRUE;
	/*
	* creo la shared memory
	*/
	if((*pDatiLinea=(PLINEA_STRUCT)SetupShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
		*pDatiLinea=(PLINEA_STRUCT)NULL;
		bOK=FALSE;
	} else {
		memset(*pDatiLinea,0,sizeof(LINEA_STRUCT));
	}
	return bOK;
}

BOOL remove_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
	BOOL bOK=TRUE;

	/*
	* cancello la shared memory
	*/
	if(!RemoveShm(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
		bOK=FALSE;
	}
	return bOK;
}

BOOL connect_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
	BOOL bOK=TRUE;
	/*
	* Creo la shared memory per la gestione della linea
	*/
	if((*pDatiLinea=(PLINEA_STRUCT)GetShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
		bOK=FALSE;
	}

	return bOK;
}



BOOL disconnect_linea_shm(PLINEA_STRUCT *pDatiLinea)
{
	BOOL bOK=TRUE;

	if(pDatiLinea){
		DetachShm((char *)*pDatiLinea);

		*pDatiLinea=(PLINEA_STRUCT)NULL;
	}
	return bOK;
}

BOOL setup_settori_shm(PSETTORE *pSettori)
{
	BOOL bOK=TRUE;
	/*
	* alloco la shared memory dei settori
	*/
	if((*pSettori=(PSETTORE)SetupShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
		bOK=FALSE;
	} else {
		memset(*pSettori,0,Cfg.nNumeroSettori*sizeof(SETTORE));
	}
	return bOK;
}

BOOL remove_settori_shm(PSETTORE *pSettori)
{
	BOOL bOK=TRUE;

	/*
	* cancello la shared memory
	*/
	if(!RemoveShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
		bOK=FALSE;
	}
	return bOK;
}

BOOL connect_settori_shm(PSETTORE *pSettori)
{
	BOOL bOK=TRUE;

	/*
	* Leggo la shared memory (SETTORI)
	*/
	if((*pSettori=(PSETTORE)GetShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
		
		bOK=FALSE;
	}
	return bOK;
}


BOOL disconnect_settori_shm(PSETTORE *pSettori)
{
	BOOL bOK=TRUE;

	if(pSettori){
		DetachShm((char *)*pSettori);

		*pSettori=(PSETTORE)NULL;
	}
	return bOK;
}


void trace_debug(BOOL bDate,BOOL bNewLine,char *fmt,...)
{
  va_list args;
  char szMessage[1024];

  va_start(args,fmt);
  (void)vsprintf(szMessage,fmt,args);
  va_end(args);
#ifdef TRACE
	if(bNewLine){
		trace_out_vstr_date(1,szMessage);
	} else {
		trace_out_vstr(0,szMessage);
	}
#endif
}

BOOL get_string_from_memcached(char *szName, char *szValue, memcached_st *memc)
{
	memcached_return rc;
	size_t nLen;
	uint32_t flags;
	char *pszValue;

	pszValue = memcached_get(memc, szName, strlen(szName), &nLen, &flags, &rc); 
	if (rc == MEMCACHED_SUCCESS) { 
		if(pszValue){
			strcpy(szValue,pszValue);
			free(pszValue); 
		} else {
			strcpy(szValue,"");
		}
	}
	return (rc == MEMCACHED_SUCCESS);
}



void dump_dati_linea(PLINEA_STRUCT pDatiLinea)
{
	trace_debug(FALSE,TRUE,"------------------------------------");
	trace_debug(FALSE,TRUE,"Stato Linea : %s", pDatiLinea->nStatoLinea == LINEA_IN_START ? "START" : "STOP");
	trace_debug(FALSE,TRUE,"Stato PLC   : %s", pDatiLinea->bPLCStatus ? "START" : "STOP");
	trace_debug(FALSE,TRUE,"RX DATA     : %s", pDatiLinea->bRxData ? "SI" : "NO");
	trace_debug(FALSE,TRUE,"TX DATA     : %s", pDatiLinea->bTxData ? "SI" : "NO");
	trace_debug(FALSE,TRUE,"------------------------------------");
}

void memcache_set_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc)
{
	char szBuffer[128];
	memcached_return rc;

	sprintf(szBuffer,"%d",pDatiLinea->nStatoLinea); if((rc = memcached_set(memc, "nStatoLinea", strlen("nStatoLinea"), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bAnalisi);    if((rc = memcached_set(memc, "bAnalisi",    strlen("bAnalisi"),    szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bStampaRAC);  if((rc = memcached_set(memc, "bStampaRAC",  strlen("bStampaRAC"),  szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bStampa);     if((rc = memcached_set(memc, "bStampa",     strlen("bStampa"),     szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bRxData);     if((rc = memcached_set(memc, "bRxData",     strlen("bRxData"),     szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bTxData);     if((rc = memcached_set(memc, "bTxData",     strlen("bTxData"),     szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bRxActive);   if((rc = memcached_set(memc, "bRxActive",   strlen("bRxActive"),   szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bTxActive);   if((rc = memcached_set(memc, "bTxActive",   strlen("bTxActive"),   szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pDatiLinea->bPLCStatus);  if((rc = memcached_set(memc, "bPLCStatus",  strlen("bPLCStatus"),  szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
}



void memcache_get_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc)
{
	char szBuffer[128];

	if(get_string_from_memcached("nStatoLinea",szBuffer,memc)) { pDatiLinea->nStatoLinea = atoi(szBuffer); }
	if(get_string_from_memcached("bAnalisi"   ,szBuffer,memc)) { pDatiLinea->bAnalisi    = atoi(szBuffer); }
	if(get_string_from_memcached("bStampaRAC" ,szBuffer,memc)) { pDatiLinea->bStampaRAC  = atoi(szBuffer); }
	if(get_string_from_memcached("bStampa"    ,szBuffer,memc)) { pDatiLinea->bStampa     = atoi(szBuffer); }
	if(get_string_from_memcached("bRxData"    ,szBuffer,memc)) { pDatiLinea->bRxData     = atoi(szBuffer); }
	if(get_string_from_memcached("bTxData"    ,szBuffer,memc)) { pDatiLinea->bTxData     = atoi(szBuffer); }
	if(get_string_from_memcached("bRxActive"  ,szBuffer,memc)) { pDatiLinea->bRxActive   = atoi(szBuffer); }
	if(get_string_from_memcached("bTxActive"  ,szBuffer,memc)) { pDatiLinea->bTxActive   = atoi(szBuffer); }
	if(get_string_from_memcached("bPLCStatus" ,szBuffer,memc)) { pDatiLinea->bPLCStatus  = atoi(szBuffer); }
}



void dump_settore(PSETTORE pSettore)
{
	trace_debug(FALSE,TRUE,"------------------------------------");
	trace_debug(FALSE,TRUE,"Settore          : %d", pSettore->nSettore);
	trace_debug(FALSE,TRUE,"------------------------------------");
	trace_debug(FALSE,TRUE,"Cedola           : %d", pSettore->nCedola);
	trace_debug(FALSE,TRUE,"Riga in prelievo : %d", pSettore->nIndiceRigaOrdine);
	trace_debug(FALSE,TRUE,"Stato settore    : %d", pSettore->nStatoSettore);
	trace_debug(FALSE,TRUE,"Codice Spedizione: %s", pSettore->szCodSped);
	trace_debug(FALSE,TRUE,"Numero Collo     : %d", pSettore->nNumCollo);
	trace_debug(FALSE,TRUE,"Codice Ubicazione: %s", pSettore->szCodUbi);
	trace_debug(FALSE,TRUE,"Codice Prodotto  : %s : %s", pSettore->szCodProd, pSettore->szTitolo);
	trace_debug(FALSE,TRUE,"Numero Copie     : %d", pSettore->nNumCopie);

}

void memcache_set_settore(int nIndex, PSETTORE pSettore, memcached_st *memc)
{
	char szBuffer[128];
	char szVarName[128];
	memcached_return rc;

	sprintf(szBuffer,"%d",pSettore->nCedola            ); sprintf(szVarName,"Settore_%02d_nCedola"             , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nIndiceRigaOrdine  ); sprintf(szVarName,"Settore_%02d_nIndiceRigaOrdine"   , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nStatoSettore      ); sprintf(szVarName,"Settore_%02d_nStatoSettore"       , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nNumCollo          ); sprintf(szVarName,"Settore_%02d_nNumCollo"           , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nNumCopie          ); sprintf(szVarName,"Settore_%02d_nNumCopie"           , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nModulo            ); sprintf(szVarName,"Settore_%02d_nModulo"             , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nRiga              ); sprintf(szVarName,"Settore_%02d_nRiga"               , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nColonna           ); sprintf(szVarName,"Settore_%02d_nColonna"            , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	sprintf(szBuffer,"%d",pSettore->nOpBlinking        ); sprintf(szVarName,"Settore_%02d_nOpBlinking"         , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	strcpy(szBuffer,      pSettore->szCodSped          ); sprintf(szVarName,"Settore_%02d_szCodSped"           , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	strcpy(szBuffer,      pSettore->szCodUbi           ); sprintf(szVarName,"Settore_%02d_szCodUbi"            , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	strcpy(szBuffer,      pSettore->szCodProd          ); sprintf(szVarName,"Settore_%02d_szCodProd"           , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	strcpy(szBuffer,      pSettore->szCodOperatore     ); sprintf(szVarName,"Settore_%02d_szCodOperatore"      , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
	strcpy(szBuffer,      pSettore->szCognomeOperatore ); sprintf(szVarName,"Settore_%02d_szCognomeOperatore"  , nIndex); if((rc = memcached_set(memc, szVarName, strlen(szVarName), szBuffer, strlen(szBuffer), (time_t)0, (uint32_t)0))!=MEMCACHED_SUCCESS){ fprintf(stderr, "Couldn't store key: %s\n", memcached_strerror(memc, rc)); }
}

void memcache_set_settori(PSETTORE pSettori, memcached_st *memc)
{
	int nIndex;

	for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){

		PSETTORE pSettore=&pSettori[nIndex];

		memcache_set_settore(nIndex, pSettore, memc);

	}
}

void memcache_get_settore(int nIndex, PSETTORE pSettore, memcached_st *memc)
{
	char szVarName[128];
	char szBuffer[128];

	sprintf(szVarName,"Settore_%02d_nCedola"             , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nCedola            = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nIndiceRigaOrdine"   , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nIndiceRigaOrdine  = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nStatoSettore"       , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nStatoSettore      = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nNumCollo"           , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nNumCollo          = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nNumCopie"           , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nNumCopie          = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nModulo"             , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nModulo            = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nRiga"               , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nRiga              = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nColonna"            , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nColonna           = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_nOpBlinking"         , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { pSettore->nOpBlinking        = atoi(szBuffer); }
	sprintf(szVarName,"Settore_%02d_szCodSped"           , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { strcpy(pSettore->szCodSped          , szBuffer); }
	sprintf(szVarName,"Settore_%02d_szCodUbi"            , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { strcpy(pSettore->szCodUbi           , szBuffer); }
	sprintf(szVarName,"Settore_%02d_szCodProd"           , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { strcpy(pSettore->szCodProd          , szBuffer); }
	sprintf(szVarName,"Settore_%02d_szCodOperatore"      , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { strcpy(pSettore->szCodOperatore     , szBuffer); }
	sprintf(szVarName,"Settore_%02d_szCognomeOperatore"  , nIndex); if(get_string_from_memcached(szVarName,szBuffer,memc)) { strcpy(pSettore->szCognomeOperatore , szBuffer); }
}

void memcache_get_settori(PSETTORE pSettori, memcached_st *memc)
{
	int nIndex;

	for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){

		PSETTORE pSettore=&pSettori[nIndex];

		memcache_get_settore(nIndex, pSettore, memc);

	}
}
