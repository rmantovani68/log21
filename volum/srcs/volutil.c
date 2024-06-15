/*
* volutil.c : Analisi Volumetrica : funzioni di utilita'
* 
* Progetto Easy Picking 4.0 : wella
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <glib.h>
#include <gio/gio.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <ep-common.h>
#include <ep-db.h>
#include <picking.h>
#include <proc_list.h>

#include	"volum.h"
#include	"volstruct.h"
#include	"volext.h"
#include	"volfun.h"

#ifdef VEDI_EP_COMMON
char *DeleteNewLine(char *str)
{
	char *dst,*src;

	src=str+strlen(str)-1;
	while(*src=='\n' || *src=='\r') {
		*src='\0';
		src--;
	}
	*(src+1)='\0';
	dst=src=str;
	while((*dst++=*src++));
	return(str);
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
	while((*dst++=*src++));
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
	while((*dst++=*src++));
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
* GetDate()
* get date a string dd-mm-yy
*/
char *GetDate(long time,char *szDateString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szDateString, "%2.2d/%2.2d/%2.2d", 
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

#endif 

/*
* Funzione SafeIntFunc:
* uscita di sicurezza
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stopped");
#endif
	if(Cfg.bAlone){
        DeleteProcessMsgQ(PROC_VOLUM);
	} else {
		/*
		* avverto MAIN dello stop
		*/
		SendMessage(PROC_MAIN, PROC_VOLUM, PROGRAM_STOPPED, NULL);
		DBDisconnect();
	}
		


	exit(1);
} /* Fine SafeIntFunc */
