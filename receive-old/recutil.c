/*
* Modulo : recutil.c
* --------------------
* Funzioni di varia utilita'
*
*
* Data creazione 17/05/2001
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <malloc.h>
#include <signal.h>
#include <libpq-fe.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef TRACE
	#include <trace.h>
#endif
#include <timeout.h>
#include <msq_lib.h>
#include <proc_list.h>


#include <dbfun.h>
#include "receive.h"
#include "recstruct.h"
#include "recext.h"
#include "recfun.h"


/*
* GetTime()
* get time of day as a string
*/
char *GetTime(long time,char *szTimeString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szTimeString, "%.2d:%.2d:%.2d", 
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

	return szTimeString;
}

/*
* GetTimeHHMMSS()
* get time of day as a string Formatted HHMMSS
*/
char *GetTimeHHMMSS(long time,char *szTimeString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szTimeString, "%.2d%.2d%.2d", 
		ltime->tm_hour, ltime->tm_min, ltime->tm_sec);

	return szTimeString;
}

/*
* GetDate()
* get date a string
*/
char *GetDate(long time,char *szDateString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szDateString, "%.2d-%.2d-%.2d", 
		ltime->tm_mday, ltime->tm_mon+1, ltime->tm_year%100);

	return szDateString;
}

/*
* GetDateYYYYMMDD()
* get date as a string format YYYYMMDD
*/
char *GetDateYYYYMMDD(long time,char *szDateString)
{
	long curr_time=time;
	struct tm *ltime=localtime(&curr_time);

	sprintf(szDateString,"%4.4d%2.2d%2.2d", 1900+ltime->tm_year,ltime->tm_mon+1,ltime->tm_mday);

	return szDateString;
}

/*
* LeftStr()
* ritorna i primi nLen caratteri della stringa
*/
char *LeftStr(char *szString,int nLen)
{
	static char szBuffer[1024];

	strncpy(szBuffer,szString,nLen);
	szBuffer[nLen]='\0';

	return(szBuffer);
}

/*
* RightStr()
* ritorna gli ultimi nLen caratteri della stringa
*/
char *RightStr(char *szString,int nLen)
{
	static char szBuffer[1024];

	strcpy(szBuffer,szString+((strlen(szString)-nLen)));

	return(szBuffer);
}

/*
* ritorna una porzione della stringa passata in ingresso
* a partire da nPos per nLen caratteri
*/
char *SubStr(char *szString,int nPos,int nLen)
{
	static char szBuffer[1024];

	/*
	* normalizzo nLen per evitare crash di applicazione
	*/
	nLen=min(1023,nLen);
	nPos=min(nPos,strlen(szString)-nLen);

	memset(szBuffer,(int)' ',nLen);
	strncpy(szBuffer,szString+nPos,nLen);
	szBuffer[nLen]='\0';

	return(szBuffer);
}


/*
* StrTrimLeft()
*/
char *StrTrimLeft(char *str)
{
	char *dst,*src;

	dst=src=str;
	while(*src==' ') src++;
	while((*(dst++) = *(src++)));
	return(str);
}


/*
* StrTrimRight()
*/
char *StrTrimRight(char *str)
{
	char *dst,*src;

	src=str+strlen(str)-1;
	while(*src==' ') src--;
	*(src+1)='\0';
	dst=src=str;
	while((*(dst++) = *(src++)));
	return(str);
}


/*
* StrTrimAll()
*/
char *StrTrimAll(char *str)
{
	StrTrimLeft(str);
	StrTrimRight(str);
	return(str);
} 

/*
* BOOL FileExists(char *szFileName)
*/
BOOL FileExists(char *szFileName)
{
	FILE *fp;

	if((fp=fopen(szFileName,"r"))!=(FILE *)NULL){
		fclose(fp);
		return(TRUE);
	}
	return(FALSE);
}

/*
* SendMessage()
* Spedizione messaggio con timeout e tentativi (NON BLOCCANTE)
*/
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText)
{
	int nRC;

	if((nRC=SendSpecificMsgNoWait(ProcList[nDest].nQNumber, ProcList[nDest].nGId, ProcList[nSrce].nGId, nMsgCode, szText,200,5))!=0){
		trace_out_vstr_date(1,"Errore [%d] in spedizione Messaggio a [%s]", nRC,ProcList[nDest].szProcName);
		return FALSE;
	}
	return TRUE;
}






