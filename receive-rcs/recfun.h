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
BOOL GetTipoUbicazione(char *szOrdProg,char *szTipoOrdine,char *szTipoUbicazioni);
BOOL CheckDatiRicezione( void );
BOOL ReadConfiguration(char *szCfgFileName,BOOL bReadProcInfo);
void DeleteBackupFiles(void);
int main(int argc,char **argv);
void do_elaborazione_kit(char *szCDKIT,int nNMCED);
