/*
* Modulo : recfun.h
* -------------------
* Include file per la dichiarazione di tutti i prototipi di funzione
*
*
* Data creazione 17-05-2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/

/* main.c */

int main(int argc,char **argv);

/* mainfun.c */

void SafeIntFunc();
void SafeExit(void);
ep_bool_t CheckFileCorriereToSend( void );
ep_bool_t CreaFileCorriere(char *szExportFile,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVE2, char *szNMDIS,ep_bool_t bStorico,ep_bool_t bTotale);
ep_bool_t CreaFileCorriereAgg(char *szExportFile,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVE2, char *szNMDIS,ep_bool_t bStorico,ep_bool_t bTotale);
ep_bool_t CreaFileCorriereSDA(char *szExportFile,char *szNMCED,char *szTPSPE,char *szCDLIN,char *szCDVE2,char *szNMDIS,ep_bool_t bStorico,ep_bool_t bTotale);
ep_bool_t CheckMailCorriereToSend( void );
ep_bool_t CheckFlussoUdcRestituiti( void );
int FileCorriereInCodaInvii(char *szFileCorriere,char *szFileCorriereAgg,char *szCedola,char *szSpedizione,char *szLinea,char *szVettore,char *szDistinta);
void ReadConfiguration(void);
void ReadProcInfo(char *szFileConfig);
void SpedizioneDati(char *szAperturaFile,char *szExportFileName,char *szExportRigheFileName,ep_bool_t bCambiaStato, ep_bool_t bStorico);
void do_aggiorna_db (void);
ep_bool_t do_vacuum_table(char *szTable);
ep_bool_t CheckMailToSend( void );
ep_bool_t CheckMailError( void );
ep_bool_t CheckCambioStatoLinea(char cStato, ep_bool_t bCambiaStato);
int GetNewTasksFromDB(PATASK pTask);


/* mainutil */

ep_bool_t local_SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
int ContaRighe(char *szFileName);
