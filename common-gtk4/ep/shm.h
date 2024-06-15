/*
* shm.h : shm include 
*/


typedef struct tag_cfg_struct {
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
} CFG_STRUCT, *PCFG_STRUCT;

extern CFG_STRUCT Cfg;

void shm_read_configuration(PCFG_STRUCT pCfg);

ep_bool_t get_string_from_memcached(char *szName, char *szValue, memcached_st *memc);

ep_bool_t setup_linea_shm        (PLINEA_STRUCT *pDatiLinea);
ep_bool_t remove_linea_shm       (PLINEA_STRUCT *pDatiLinea);
ep_bool_t connect_linea_shm      (PLINEA_STRUCT *pDatiLinea);
ep_bool_t disconnect_linea_shm   (PLINEA_STRUCT *pDatiLinea);

void dump_dati_linea        (PLINEA_STRUCT pDatiLinea);
void memcache_set_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc);
void memcache_get_dati_linea(PLINEA_STRUCT pDatiLinea, memcached_st *memc);

ep_bool_t setup_settori_shm      (PSETTORE *pSettori);
ep_bool_t remove_settori_shm     (PSETTORE *pSettori);
ep_bool_t connect_settori_shm    (PSETTORE *pSettori);
ep_bool_t disconnect_settori_shm (PSETTORE *pSettori);

void dump_settore           (PSETTORE pSettore);
void memcache_set_settori   (PSETTORE pSettori, memcached_st *memc);
void memcache_set_settore   (int nIndex, PSETTORE pSettore, memcached_st *memc);
void memcache_get_settori   (PSETTORE pSettori, memcached_st *memc);
void memcache_get_settore   (int nIndex, PSETTORE pSettore, memcached_st *memc);
