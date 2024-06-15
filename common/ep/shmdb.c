/*
* shmdb.c : shm functions on db
*/
#include <glib.h>
// #include <gio/gio.h>
#include <stdlib.h>
// #include <stdio.h>
// #include <stdarg.h>
// #include <time.h>
// #include <signal.h>
#include <string.h>
#include <unistd.h>
// #include <fcntl.h>
// #include <termio.h>
// #include <sys/stat.h>

#include <pmx_msq.h>
// #include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
    #include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
// #include <proc_list.h>

#include <shmdb.h>

extern char __configuration_file__[];

/**
 * @brief 
 * 
 * @param pCfg 
 */
void shmdb_read_configuration(PCFG_STRUCT pCfg)
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
    GetFileString(szParagraph,"DBHost",     "localhost",   pCfg->szDBHost,       80, pCfg->szCniCfg, NULL);
    GetFileString(szParagraph,"DBPort",     "5432",        pCfg->szDBPort,       80, pCfg->szCniCfg, NULL);
    GetFileString(szParagraph,"DBName",     "database",    pCfg->szDBName,       80, pCfg->szCniCfg, NULL);

    pCfg->nDebugVersion   = GetFileInt(szParagraph, "DebugVersion",     0, pCfg->szCniCfg,NULL);
    pCfg->nShmKey         = GetFileInt(szParagraph, "ShmKey",         256, pCfg->szCniCfg,NULL);
    pCfg->nSettoriShmKey  = GetFileInt(szParagraph, "SettoriShmKey",  257, pCfg->szCniCfg,NULL);
    pCfg->nNumeroSettori  = GetFileInt("settori",   "NumeroSettori",   12, pCfg->szCniCfg,NULL);
}

ep_bool_t shmdb_setup_linea(PLINEA_STRUCT *pDatiLinea)
{
    ep_bool_t bOK=TRUE;
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

ep_bool_t shmdb_remove_linea(PLINEA_STRUCT *pDatiLinea)
{
    ep_bool_t bOK=TRUE;

    /*
    * cancello la shared memory
    */
    if(!RemoveShm(Cfg.nShmKey,sizeof(LINEA_STRUCT))){
        bOK=FALSE;
    }
    return bOK;
}

ep_bool_t shmdb_connect_linea(PLINEA_STRUCT *pDatiLinea)
{
    ep_bool_t bOK=TRUE;
    /*
    * Creo la shared memory per la gestione della linea
    */
    if((*pDatiLinea=(PLINEA_STRUCT)GetShm(Cfg.nShmKey,sizeof(LINEA_STRUCT)))==NULL){
        bOK=FALSE;
    }

    return bOK;
}



ep_bool_t shmdb_disconnect_linea(PLINEA_STRUCT *pDatiLinea)
{
    ep_bool_t bOK=TRUE;

    if(pDatiLinea){
        DetachShm((char *)*pDatiLinea);

        *pDatiLinea=(PLINEA_STRUCT)NULL;
    }
    return bOK;
}

ep_bool_t shmdb_setup_settori(PSETTORE *pSettori)
{
    ep_bool_t bOK=TRUE;
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

ep_bool_t shmdb_remove_settori(PSETTORE *pSettori)
{
    ep_bool_t bOK=TRUE;

    /*
    * cancello la shared memory
    */
    if(!RemoveShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE))){
        bOK=FALSE;
    }
    return bOK;
}

ep_bool_t shmdb_connect_settori(PSETTORE *pSettori)
{
    ep_bool_t bOK=TRUE;

    /*
    * Leggo la shared memory (SETTORI)
    */
    if((*pSettori=(PSETTORE)GetShm(Cfg.nSettoriShmKey,Cfg.nNumeroSettori*sizeof(SETTORE)))==NULL){
        
        bOK=FALSE;
    }
    return bOK;
}


ep_bool_t shmdb_disconnect_settori(PSETTORE *pSettori)
{
    ep_bool_t bOK=TRUE;

    if(pSettori){
        DetachShm((char *)*pSettori);

        *pSettori=(PSETTORE)NULL;
    }
    return bOK;
}

void shmdb_dump_dati_linea(PLINEA_STRUCT pDatiLinea)
{
    trace_debug(FALSE, TRUE, "------------------------------------");
    trace_debug(FALSE, TRUE, "Stato Linea : %s", pDatiLinea->nStatoLinea == LINEA_IN_START ? "START" : "STOP");
    trace_debug(FALSE, TRUE, "Stato PLC   : %s", pDatiLinea->bPLCStatus ? "START" : "STOP");
    trace_debug(FALSE, TRUE, "RX DATA     : %s", pDatiLinea->bRxData ? "SI" : "NO");
    trace_debug(FALSE, TRUE, "TX DATA     : %s", pDatiLinea->bTxData ? "SI" : "NO");
    trace_debug(FALSE, TRUE, "------------------------------------");
}


ep_bool_t _shmdb_get_string(char *table_name, char *key_field_name, char *key_field_value, char *field_name, char *field_value)
{
    ep_bool_t rc = TRUE;
    char sql_cmd[256];
    DBresult *DBRes;

    if ( key_field_name ) {
        sprintf(sql_cmd, "select %s from %s where %s = %s;", field_name, table_name, key_field_name, key_field_value);
    } else {
        sprintf(sql_cmd, "select %s from %s;", field_name, table_name);
    }
    DBRes = DBExecQuery(Cfg.nDebugVersion, sql_cmd);
    if (DBresultStatus(DBRes) != DBRES_TUPLES_OK || DBntuples(DBRes)!=1){
        strcpy(field_value, "ERR");
        rc = FALSE;
    } else {
        strcpy(field_value, DBgetvalue(DBRes,0,0));
    }
    DBclear(DBRes);

    return rc;
}


ep_bool_t _shmdb_set_string(char *table_name, char *key_field_name, char *key_field_value, char *field_name, char *field_value)
{
    ep_bool_t rc = TRUE;
    char sql_cmd[256];
    DBresult *DBRes;

    if ( key_field_name ) {
        sprintf(sql_cmd, "update %s set %s = %s where %s = %s;", table_name, field_name, field_value, key_field_name, key_field_value);
    } else {
        sprintf(sql_cmd, "update %s set %s = %s;", table_name, field_name, field_value);
    }

    DBRes = DBExecQuery(Cfg.nDebugVersion, "update %s set %s = %s;", table_name, field_name, field_value);
    if (DBresultStatus(DBRes) != DBRES_COMMAND_OK || 1 != atoi(DBcmdTuples(DBRes))) {
        rc = FALSE;
    }
    DBclear(DBRes);

    return rc;
}

void shmdb_set_dati_linea(PLINEA_STRUCT pDatiLinea)
{
    char szBuffer[128];
    
    sprintf(szBuffer,"%d",pDatiLinea->nStatoLinea); _shmdb_set_string("shm_linea", "nStatoLinea", szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bAnalisi);    _shmdb_set_string("shm_linea", "bAnalisi",    szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bStampaRAC);  _shmdb_set_string("shm_linea", "bStampaRAC",  szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bStampa);     _shmdb_set_string("shm_linea", "bStampa",     szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bRxData);     _shmdb_set_string("shm_linea", "bRxData",     szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bTxData);     _shmdb_set_string("shm_linea", "bTxData",     szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bRxActive);   _shmdb_set_string("shm_linea", "bRxActive",   szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bTxActive);   _shmdb_set_string("shm_linea", "bTxActive",   szBuffer, NULL, NULL);
    sprintf(szBuffer,"%d",pDatiLinea->bPLCStatus);  _shmdb_set_string("shm_linea", "bPLCStatus",  szBuffer, NULL, NULL);
}

void shmdb_get_dati_linea(PLINEA_STRUCT pDatiLinea)
{
    char szBuffer[128];

    if(_shmdb_get_string("shm_linea", "nStatoLinea", szBuffer, NULL, NULL)) { pDatiLinea->nStatoLinea = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bAnalisi"   , szBuffer, NULL, NULL)) { pDatiLinea->bAnalisi    = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bStampaRAC" , szBuffer, NULL, NULL)) { pDatiLinea->bStampaRAC  = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bStampa"    , szBuffer, NULL, NULL)) { pDatiLinea->bStampa     = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bRxData"    , szBuffer, NULL, NULL)) { pDatiLinea->bRxData     = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bTxData"    , szBuffer, NULL, NULL)) { pDatiLinea->bTxData     = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bRxActive"  , szBuffer, NULL, NULL)) { pDatiLinea->bRxActive   = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bTxActive"  , szBuffer, NULL, NULL)) { pDatiLinea->bTxActive   = atoi(szBuffer); }
    if(_shmdb_get_string("shm_linea", "bPLCStatus" , szBuffer, NULL, NULL)) { pDatiLinea->bPLCStatus  = atoi(szBuffer); }
}



void shmdb_dump_settore(PSETTORE pSettore)
{
    trace_debug(FALSE, TRUE, "------------------------------------");
    trace_debug(FALSE, TRUE, "Settore          : %d", pSettore->nSettore);
    trace_debug(FALSE, TRUE, "------------------------------------");
    trace_debug(FALSE, TRUE, "Cedola           : %d", pSettore->nCedola);
    trace_debug(FALSE, TRUE, "Riga in prelievo : %d", pSettore->nIndiceRigaOrdine);
    trace_debug(FALSE, TRUE, "Stato settore    : %d", pSettore->nStatoSettore);
    trace_debug(FALSE, TRUE, "Codice Spedizione: %s", pSettore->szCodSped);
    trace_debug(FALSE, TRUE, "Numero Collo     : %d", pSettore->nNumCollo);
    trace_debug(FALSE, TRUE, "Codice Ubicazione: %s", pSettore->szCodUbi);
    trace_debug(FALSE, TRUE, "Codice Prodotto  : %s : %s", pSettore->szCodProd, pSettore->szTitolo);
    trace_debug(FALSE, TRUE, "Numero Copie     : %d", pSettore->nNumCopie);

}

void shmdb_set_settore(int nIndex, PSETTORE pSettore)
{
    char field_value[128];
    char key_field_name[128];
    char key_field_value[128];

    strcpy(key_field_name,"settore");
    sprintf(key_field_value,"%d", nIndex);

    sprintf(field_value,"%d",pSettore->nCedola            ); _shmdb_set_string("shm_settori", "nCedola",            field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nIndiceRigaOrdine  ); _shmdb_set_string("shm_settori", "nIndiceRigaOrdine",  field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nStatoSettore      ); _shmdb_set_string("shm_settori", "nStatoSettore",      field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nNumCollo          ); _shmdb_set_string("shm_settori", "nNumCollo",          field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nNumCopie          ); _shmdb_set_string("shm_settori", "nNumCopie",          field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nModulo            ); _shmdb_set_string("shm_settori", "nModulo",            field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nRiga              ); _shmdb_set_string("shm_settori", "nRiga",              field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nColonna           ); _shmdb_set_string("shm_settori", "nColonna",           field_value, key_field_name, key_field_value);
    sprintf(field_value,"%d",pSettore->nOpBlinking        ); _shmdb_set_string("shm_settori", "nOpBlinking",        field_value, key_field_name, key_field_value);
    strcpy(field_value,      pSettore->szCodSped          ); _shmdb_set_string("shm_settori", "szCodSped",          field_value, key_field_name, key_field_value);
    strcpy(field_value,      pSettore->szCodUbi           ); _shmdb_set_string("shm_settori", "szCodUbi",           field_value, key_field_name, key_field_value);
    strcpy(field_value,      pSettore->szCodProd          ); _shmdb_set_string("shm_settori", "szCodProd",          field_value, key_field_name, key_field_value);
    strcpy(field_value,      pSettore->szCodOperatore     ); _shmdb_set_string("shm_settori", "szCodOperatore",     field_value, key_field_name, key_field_value);
    strcpy(field_value,      pSettore->szCognomeOperatore ); _shmdb_set_string("shm_settori", "szCognomeOperatore", field_value, key_field_name, key_field_value);
}

void shmdb_set_settori(PSETTORE pSettori)
{
    int nIndex;

    for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){
        PSETTORE pSettore=&pSettori[nIndex];

        shmdb_set_settore(nIndex+1, pSettore);
    }
}

void shmdb_get_settore(int nIndex, PSETTORE pSettore)
{
    char field_value[128];
    char key_field_name[128];
    char key_field_value[128];

    strcpy(key_field_name,"settore");
    sprintf(key_field_value,"%d", nIndex);

    _shmdb_get_string("shm_settori", "nCedola",            field_value, key_field_name, key_field_value); pSettore->nCedola            = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nIndiceRigaOrdine",  field_value, key_field_name, key_field_value); pSettore->nIndiceRigaOrdine  = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nStatoSettore",      field_value, key_field_name, key_field_value); pSettore->nStatoSettore      = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nNumCollo",          field_value, key_field_name, key_field_value); pSettore->nNumCollo          = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nNumCopie",          field_value, key_field_name, key_field_value); pSettore->nNumCopie          = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nModulo",            field_value, key_field_name, key_field_value); pSettore->nModulo            = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nRiga",              field_value, key_field_name, key_field_value); pSettore->nRiga              = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nColonna",           field_value, key_field_name, key_field_value); pSettore->nColonna           = atoi(field_value); 
    _shmdb_get_string("shm_settori", "nOpBlinking",        field_value, key_field_name, key_field_value); pSettore->nOpBlinking        = atoi(field_value); 
    _shmdb_get_string("shm_settori", "szCodSped",          field_value, key_field_name, key_field_value); strcpy(pSettore->szCodSped          , field_value); 
    _shmdb_get_string("shm_settori", "szCodUbi",           field_value, key_field_name, key_field_value); strcpy(pSettore->szCodUbi           , field_value); 
    _shmdb_get_string("shm_settori", "szCodProd",          field_value, key_field_name, key_field_value); strcpy(pSettore->szCodProd          , field_value); 
    _shmdb_get_string("shm_settori", "szCodOperatore",     field_value, key_field_name, key_field_value); strcpy(pSettore->szCodOperatore     , field_value); 
    _shmdb_get_string("shm_settori", "szCognomeOperatore", field_value, key_field_name, key_field_value); strcpy(pSettore->szCognomeOperatore , field_value); 
}

void shmdb_get_settori(PSETTORE pSettori)
{
    int nIndex;

    for (nIndex = 0; nIndex < Cfg.nNumeroSettori; nIndex++){

        PSETTORE pSettore=&pSettori[nIndex];

        shmdb_get_settore(nIndex+1, pSettore);

    }
}
