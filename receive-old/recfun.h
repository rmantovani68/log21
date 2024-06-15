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

/* receive.c */

void SafeIntFunc();
void SafeExit(void);
BOOL CheckDatiRicezione( void );
void ReadConfiguration(void);
void ReadProcInfo(char *szFileConfig);
int main(int argc,char **argv);
BOOL RicavaDistribuzione(PDATI_DISTRIBUZIONE,char *szOrdProg,int nPeso,int nColli);

/* recutil */

BOOL FileExists(char *szFileName);
char *GetDate(long time,char *szDateString);
char *GetDateYYYYMMDD(long time,char *szDateString);
char *GetTime(long time,char *szTimeString);
char *GetTimeHHMMSS(long time,char *szTimeString);
char *LeftStr(char *szString,int nLen);
char *RightStr(char *szString,int nLen);
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText);
char *StrTrimAll(char *str);
char *StrTrimLeft(char *str);
char *StrTrimRight(char *str);
char *SubStr(char *szString,int nPos,int nLen);
