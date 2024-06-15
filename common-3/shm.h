/*
* shm.h : shm include 
*/

typedef struct tagCfgStruct{
	char szCniCfg[128];
	char szPathData[128];
	char szPathExe[128];
	char szPathTrace[128];

	char szMemCachedServerName[128];
	int nMemCachedPortNumber;

	int nShmKey;
	int nSettoriShmKey;
	int nNumeroCedole;
	int nNumeroSettori;
} CFGSTRUCT, *PCFGSTRUCT;

extern CFGSTRUCT Cfg;

void trace_debug(BOOL bDate,BOOL bNewLine,char *fmt,...);
void ReadConfiguration(PCFGSTRUCT pCfg);
BOOL get_string_from_memcached(char *szName, char *szValue, memcached_st *memc);

BOOL setup_linea_shm        (PLINEA_STRUCT *pDatiLinea);
BOOL remove_linea_shm       (PLINEA_STRUCT *pDatiLinea);
BOOL connect_linea_shm      (PLINEA_STRUCT *pDatiLinea);
BOOL disconnect_linea_shm   (PLINEA_STRUCT *pDatiLinea);

void dump_dati_linea        (PLINEA_STRUCT pDatiLinea);
void memcache_set_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc);
void memcache_get_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc);

BOOL setup_settori_shm      (PSETTORE *pSettori);
BOOL remove_settori_shm     (PSETTORE *pSettori);
BOOL connect_settori_shm    (PSETTORE *pSettori);
BOOL disconnect_settori_shm (PSETTORE *pSettori);

void dump_settore           (PSETTORE pSettore);
void memcache_set_settori   (PSETTORE pSettori, memcached_st *memc);
void memcache_set_settore   (int nIndex, PSETTORE pSettore, memcached_st *memc);
void memcache_get_settori   (PSETTORE pSettori, memcached_st *memc);
void memcache_get_settore   (int nIndex, PSETTORE pSettore, memcached_st *memc);
