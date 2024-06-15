/*
* shmdb.h : shm db include 
*/

typedef struct tag_cfg_struct {
	int nDebugVersion;

    char szCniCfg[128];
    char szPathData[128];
    char szPathExe[128];
    char szPathTrace[128];

    char szDBServerName[128];

    int nShmKey;
    int nSettoriShmKey;
    int nNumeroSettori;

    char szDBHost[128];
    char szDBPort[128];
    char szDBName[128];

} CFG_STRUCT, *PCFG_STRUCT;

extern CFG_STRUCT Cfg;

void shmdb_read_configuration(PCFG_STRUCT pCfg);

ep_bool_t shmdb_setup_linea        (PLINEA_STRUCT *pDatiLinea);
ep_bool_t shmdb_remove_linea       (PLINEA_STRUCT *pDatiLinea);
ep_bool_t shmdb_connect_linea      (PLINEA_STRUCT *pDatiLinea);
ep_bool_t shmdb_disconnect_linea   (PLINEA_STRUCT *pDatiLinea);

void shmdb_dump_dati_linea        (PLINEA_STRUCT pDatiLinea);
void shmdb_set_dati_linea(PLINEA_STRUCT pDatiLinea);
void shmdb_get_dati_linea(PLINEA_STRUCT pDatiLinea);

ep_bool_t shmdb_setup_settori      (PSETTORE *pSettori);
ep_bool_t shmdb_remove_settori     (PSETTORE *pSettori);
ep_bool_t shmdb_connect_settori    (PSETTORE *pSettori);
ep_bool_t shmdb_disconnect_settori (PSETTORE *pSettori);

void shmdb_dump_settore           (PSETTORE pSettore);
void shmdb_set_settori   (PSETTORE pSettori);
void shmdb_set_settore   (int nIndex, PSETTORE pSettore);
void shmdb_get_settori   (PSETTORE pSettori);
void shmdb_get_settore   (int nIndex, PSETTORE pSettore);
