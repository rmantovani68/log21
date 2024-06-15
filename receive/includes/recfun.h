/*
* Modulo : recfun.h
* -------------------
* Include file per la dichiarazione di tutti i prototipi di funzione
*
* Data creazione 17-05-2001
*
* Autore : Roberto Mantovani
*
* Copyright SMD srl 2002
*/

/* receive.c */

void SafeIntFunc();
void SafeExit(void);
ep_bool_t GetTipoUbicazione(char *szOrdProg,char *szTipoOrdine,char *szTipoUbicazioni);
ep_bool_t CheckDatiRicezione( void );
ep_bool_t ReadConfiguration(char *szCfgFileName,ep_bool_t bReadProcInfo);
void DeleteBackupFiles(void);
int main(int argc,char **argv);
void do_elaborazione_kit(char *szCDKIT,int nNMCED);
