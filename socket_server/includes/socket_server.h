/**
* @file socket_server.h
* 
* Progetto Easy Picking 4.0
*
* @author Roberto Mantovani
*
* @copyright CNI srl 1995-2000
*/

typedef struct tagCfgStruct{
    char szCfgFileName[128];
    char szPathTrace[128];
    int nMainID;
    int nDelay;
} CFGSTRUCT, *PCFGSTRUCT;

/*
* Inizio dichiarazione dei prototipi
*/
void SafeIntFunc();

ep_bool_t ReadConfiguration(char *szCfgFileName,ep_bool_t bReadProcInfo);

ep_bool_t GestSocket(int *pnFd, void *ClientData, void *CallData);
int GetCmdIndex(char *szCmd);
