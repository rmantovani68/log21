/*
* Modulo : ep-common.c
* -----------------------
* Funzioni di utilità per Easy Picking
*
* Data creazione : Fri May 31 17:33:07 CEST 2002
*
* Autore : Roberto Mantovani
*
* Copyright SMD srl 2002
*/
#include <glib.h>

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#if defined(TRACE) || defined(TRACE_SQL) || defined(TRACE_PLC)
	#include <trace.h>
#endif

#include <pmx_msq.h>

#include <picking.h>
#include <ep-common.h>

#include <proc_list.h>


PROCESSO ProcList[NUM_PROC+1];	/* processi componenti il progetto */


char *Escape(char *t) 
{
	static char s[2048];

	int i, j;
	i = j = 0;

	while ( t[i] ) {

		/*  Translate the special character, if we have one  */

		switch( t[i] ) {
			case '\n':
				s[j++] = '\\';
				s[j] = 'n';
			break;

			case '\t':
				s[j++] = '\\';
				s[j] = 't';
			break;

			case '\a':
				s[j++] = '\\';
				s[j] = 'a';
			break;

			case '\b':
				s[j++] = '\\';
				s[j] = 'b';
			break;

			case '\f':
				s[j++] = '\\';
				s[j] = 'f';
			break;

			case '\r':
				s[j++] = '\\';
				s[j] = 'r';
			break;

			case '\v':
				s[j++] = '\\';
				s[j] = 'v';
			break;

			case '\\':
				s[j++] = '\\';
				s[j] = '\\';
			break;

			case '\"':
				s[j++] = '\\';
				s[j] = '\"';
			break;

			default:
				/*  This is not a special character, so just copy it  */
				s[j] = t[i];
			break;
		}
		++i;
		++j;
	}
	s[j] = t[i];    /*  Don't forget the null character  */

	return s;
}


char *Unescape(char *t) 
{
	static char s[2048];
	int i, j;
	i = j = 0;

	while ( t[i] ) {
		switch ( t[i] ) {
			case '\\':

				/*  We've found an escape sequence, so translate it  */
				switch( t[++i] ) {
					case 'n':
					s[j] = '\n';
					break;

					case 't':
					s[j] = '\t';
					break;

					case 'a':
					s[j] = '\a';
					break;

					case 'b':
					s[j] = '\b';
					break;

					case 'f':
					s[j] = '\f';
					break;

					case 'r':
					s[j] = '\r';
					break;

					case 'v':
					s[j] = '\v';
					break;

					case '\\':
					s[j] = '\\';
					break;

					case '\"':
					s[j] = '\"';
					break;

					default:

					/*  We don't translate this escape
					*                      sequence, so just copy it verbatim  */

					s[j++] = '\\';
					s[j] = t[i];
				}
			break;

			default:
				/*  Not an escape sequence, so just copy the character  */
				s[j] = t[i];
			break;
		}
		++i;
		++j;
	}
	s[j] = t[i];    /*  Don't forget the null character  */

	return s;
}





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
* GetTimeFromHHMMSS()
* get time of day from a string formatted HHMMSS
*/
char *GetTimeFromHHMMSS(char *szFromTime,char *szTimeString)
{
	strcpy(szTimeString,szFromTime);			/* ora */
	szTimeString[2]='\0';
	strcat(szTimeString,":");
	strcat(szTimeString,szFromTime+2);	/* minuti */
	szTimeString[5]='\0';
	strcat(szTimeString,":");
	strcat(szTimeString,szFromTime+4);	/* secondi */
	szTimeString[8]='\0';

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
* GetTimeHHMM()
* get time of day as a string Formatted HHMM
*/
char *GetTimeHHMM(long time,char *szTimeString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szTimeString, "%.2d%.2d", 
		ltime->tm_hour, ltime->tm_min);

	return szTimeString;
}


/*
* GetTimeSpanHHMMSS(char *szToTime, char *szFromTime, char *szTimeString);
*/
char *GetTimeSpanHHMMSS(const char *szFromTime, const char* szToTime, char *szTimeString)
{
	int nTimeStart,nTimeEnd,nTimeSpan;
	int nOre,nMin,nSec;
	char szToOre[20],szToMin[20],szToSec[20];
	char szFromOre[20],szFromMin[20],szFromSec[20];

	szTimeString[0]='\0';

	if (strlen(szToTime)!=6 && strlen(szFromTime)!=6){
		return szTimeString;
	} else {
		sprintf(szFromSec,szFromTime+4);
		szFromSec[2]='\0';
		sprintf(szFromMin,szFromTime+2);
		szFromMin[2]='\0';
		sprintf(szFromOre,szFromTime);
		szFromOre[2]='\0';

		sprintf(szToSec,szToTime+4);
		szToSec[2]='\0';
		sprintf(szToMin,szToTime+2);
		szToMin[2]='\0';
		sprintf(szToOre,szToTime);
		szToOre[2]='\0';

		nTimeStart=(atoi(szFromSec) + (atoi(szFromMin)*60) + (atoi(szFromOre)*3600) );
		nTimeEnd=(atoi(szToSec) + (atoi(szToMin)*60) + (atoi(szToOre)*3600) );
		nTimeSpan=nTimeEnd-nTimeStart;

		nOre=nTimeSpan/3600;	
		nMin=(nTimeSpan%3600)/60;
		nSec=(nTimeSpan%3600)%60;

		sprintf(szTimeString,"%02d%02d%02d",nOre,nMin,nSec);

		return szTimeString;
	}
}

/*
* GetYear()
* get date a string
*/
char *GetYear(long time,char *szDateString)
{
	struct tm *ltime;
	long curr_time;

	curr_time = time;
	ltime = localtime(&curr_time);
	sprintf(szDateString, "%4d", 1900+ltime->tm_year);

	return szDateString;
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
* GetDateYYMMDD()
* get date as a string format YYMMDD
*/
char *GetDateYYMMDD(long time,char *szDateString)
{
	long curr_time=time;
	struct tm *ltime=localtime(&curr_time);

	sprintf(szDateString,"%2.2d%2.2d%2.2d", ltime->tm_year%100,ltime->tm_mon+1,ltime->tm_mday);

	return szDateString;
}

/*
* GetDatef(char *szDateFormat)
* Funzione generica di formattazione della data attuale
* Formato di stampa : strftime()
* -------------------------------------------------------
* %% - same as % 
* %a - day of week, using the locale's abbreviated weekday names 
* %A - day of week, using the locale's full weekday names 
* %b - month, using the locale's abbreviated month names 
* %B - month, using the locale's full month names 
* %c - date and time as %x %X 
* %C - century (00-99)
* %d - day of the month (01-31) 
* %D - date as %m/%d/%y 
* %e - day of the month (1-31; single digits are preceded by a blank) 
* %h - synonym for %b
* %H - hour using 24-hour clock (00-23) 
* %I - hour using 12-hour clock (01-12) 
* %j - day of the year (001-366) 
* %k - hour using 24-hour clock (0-23; single digits are preceded by a blank) 
* %l - hour using 12-hour clock (1-12; single digits are preceded by a blank) 
* %m - month number (01-12) 
* %M - minute (00-59) 
* %n - same as \n
* %p - locale's equivalent of the AM/PM designation associated with a 12-hour clock
* %r - time as %I:%M:%S %p 
* %R - time as %H:%M 
* %S - second (00-59) 
* %t - same as \t
* %T - time as %H:%M:%S 
* %U - week number of the year (00-53), where the first Sunday is the first day of week 1
* %w - day of week (0-6), where Sunday is day 0
* %W - week number of the year (00-53), where the first Monday is the first day of week 1
* %x - date, using the locale's date format 
* %X - time, using the locale's time format 
* %y - year within century (00-99) 
* %Y - year, including century (for example, 1988) 
* %Z - time zone abbreviation
*
*/
char *GetDatef(char *szDateFormat)
{
	static char szDateBuffer[128];
	long curr_time=time((long *)0);
	struct tm *ltime=localtime(&curr_time);

	strftime(szDateBuffer,sizeof(szDateBuffer),szDateFormat,ltime);

	return szDateBuffer;
}

/*
* GetDateDDMM()
* get date as a string format DDMM
*/
char *GetDateDDMM(long time,char *szDateString)
{
	long curr_time=time;
	struct tm *ltime=localtime(&curr_time);

	sprintf(szDateString,"%2.2d%2.2d", ltime->tm_mday, ltime->tm_mon+1);

	return szDateString;
}

/*
* GetDateDDMMYYYY()
* get date as a string format DDMMYYYY
*/
char *GetDateDDMMYYYY(long time,char *szDateString)
{
	long curr_time=time;
	struct tm *ltime=localtime(&curr_time);

	sprintf(szDateString,"%2.2d%2.2d%4.4d", ltime->tm_mday, ltime->tm_mon+1, 1900+ltime->tm_year);

	return szDateString;
}

/*
* GetDateFromYYYYMMDD()
* get date from a string format YYYYMMDD
*/
char *GetDateFromYYYYMMDD(char *szFromDate,char *szDateString)
{
	if(strlen(szFromDate)){
		szDateString[0]='\0';
	}
	strcpy(szDateString,szFromDate+6);	/* giorno */
	strcat(szDateString,"-");
	strcat(szDateString,szFromDate+4);	/* mese */
	szDateString[5]='\0';
	strcat(szDateString,"-");
	strcat(szDateString,szFromDate+2);	/* anno */
	szDateString[8]='\0';

	return szDateString;
}
/*
* GetDateYYMMFromDDMMYY()
* get date YYMM from a string formatted DDMMYY
*/
char *GetDateYYMMFromDDMMYY(char *szFromDate,char *szDateString)
{
	char szYear[80];
	char szMonth[80];

	/* anno */
	strcpy(szYear,szFromDate+4);
	szYear[2]='\0';	
	/* mese */
	strcpy(szMonth,szFromDate+2);
	szMonth[2]='\0';	

	sprintf(szDateString,"%02d%02d", atoi(szYear),atoi(szMonth));

	return szDateString;
}

/*
* GetDateYYYYMMDDFromDDMMYY()
* get date YYYYMMDD from a string formatted DDMMYY
*/
char *GetDateYYYYMMDDFromDDMMYY(char *szFromDate,char *szDateString)
{
	char szBuffer[80];

	if(strlen(szFromDate)<6){
		return "";
	}
	/* anno */
	strcpy(szBuffer,szFromDate+4);
	szBuffer[2]='\0';	
	sprintf(szDateString,"%04d",
		atoi(szBuffer)>90?1900+atoi(szBuffer):2000+atoi(szBuffer));
	strcat(szDateString,szFromDate+2);	/* mese */
	szDateString[6]='\0';
	strcat(szDateString,szFromDate);	/* giorno */
	szDateString[8]='\0';

	return szDateString;
}

/*
* GetDateFromDDMMYY()
* get date from a string formatted DDMMYY
*/
char *GetDateFromDDMMYY(char *szFromDate,char *szDateString)
{
	strcpy(szDateString,szFromDate);	/* giorno */
	szDateString[2]='\0';
	strcat(szDateString,"-");
	strcat(szDateString,szFromDate+2);	/* mese */
	szDateString[5]='\0';
	strcat(szDateString,"-");
	strcat(szDateString,szFromDate+4);	/* anno */

	return szDateString;
}

/*
* GetDateYYYYMMDDFromDD_MM_YYYY()
* get date YYYYMMDD from a string formatted DD-MM-YYYY
*/
char *GetDateYYYYMMDDFromDD_MM_YYYY(char *szFromDate,char *szDateString)
{

	if(strlen(szFromDate)!=10){
		szDateString[0]='\0';
	}
	strcpy(szDateString,szFromDate+6); /* anno */
	szDateString[4]='\0';	
	strcat(szDateString,szFromDate+3);	/* mese */
	szDateString[6]='\0';
	strcat(szDateString,szFromDate);	/* giorno */
	szDateString[8]='\0';

	return szDateString;
}

/*
* GetDateYYYYMMDDFromYYYY_MM_DD()
* get date YYYYMMDD from a string formatted YYYY-MM-DD
*/
char *GetDateYYYYMMDDFromYYYY_MM_DD(char *szFromDate,char *szDateString)
{

	if(strlen(szFromDate)!=10){
		szDateString[0]='\0';
	}
	strcpy(szDateString,szFromDate); /* anno */
	szDateString[4]='\0';	
	strcat(szDateString,szFromDate+5);	/* mese */
	szDateString[6]='\0';
	strcat(szDateString,szFromDate+8);	/* giorno */
	szDateString[8]='\0';

	return szDateString;
}

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
	nPos=min(nPos,(int)strlen(szString)-nLen);

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
* StrNormalize()
*/
char *StrNormalize(char *str)
{
	char *src;

	src=str;
	while(*src){
		if((unsigned char)*src>(unsigned char)127) *src=' ';
		src++;
	}
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
* RightPad()
* mette gli spazi in coda alla stringa passata in ingresso
* ritorna la stringa passata in ingresso
*/
char *RightPad(char *szString,int nLen)
{
	char szSpaceString[256];
	int nLenUsed=min(255,nLen);
	int nStringLen;

	nStringLen=strlen(szString);

	if (nStringLen>=nLenUsed){
		return(szString);
	}
	memset(szSpaceString,' ',nLenUsed);
	szSpaceString[nLenUsed]='\0';


	memcpy(szSpaceString+(nLenUsed-nStringLen),szString,nStringLen);
	strcpy(szString,szSpaceString);

	return(szString);
}

/*
* LeftPad()
* mette gli spazi in TESTA alla stringa passata in ingresso
* ritorna la stringa passata in ingresso
*/
char *LeftPad(char *szString,int nLen)
{
	char szSpaceString[256];
	int nLenUsed=min(255,nLen);
	int nStringLen;

	nStringLen=strlen(szString);

	if (nStringLen>=nLenUsed){
		return(szString);
	}
	memset(szSpaceString,' ',nLenUsed);
	szSpaceString[nLenUsed]='\0';


	memcpy(szSpaceString+(nLenUsed-nStringLen),szString,nStringLen);
	strcpy(szString,szSpaceString);

	return(szString);
}

/*
* PadZero()
* mette gli zeri in testa alla stringa passata in ingresso
* ritorna la stringa passata in ingresso
*/
char *PadZero(char *szString,int nLen)
{
	char szZeroString[256];
	int nLenUsed=min(255,nLen);
	int nStringLen;

	StrTrimAll(szString);

	nStringLen=strlen(szString);

	if (nStringLen>=nLenUsed){
		return(szString);
	}
	memset(szZeroString,'0',nLenUsed);
	szZeroString[nLenUsed]='\0';


	memcpy(szZeroString+(nLenUsed-nStringLen),szString,nStringLen);
	strcpy(szString,szZeroString);

	return(szString);
}

/*
* ElimNewLine()
*/
char *ElimNewLine(char *str)
{
	char *dst,*src;

	dst=src=str;

	while(*src!='\n' && *src!='\r' && *src!='\0'){
		*dst++=*src++;
	}
	*dst='\0';

	return(str);
}

/*
* CmpFileDate()
* Confronta la data del file in oggetto con quella attuale.
* Ritorna :
*  0 : Data odierna
*  1 : Data maggiore della data odierna
* -1 : Data minore della data odierna
* -2 : file non esistente (verificare errno)
*/
int CmpFileDate(char *pszFileName)
{
	struct stat st;
	struct tm *ftime;
	char szFileDate[128];
	char szActualDate[128];

	GetDateYYYYMMDD(time((long *)0), szActualDate);

	/* 
	* ricavo la data del file 
	*/
	if(!stat(pszFileName,&st)){
		ftime = localtime(&(st.st_mtime));
		sprintf(szFileDate,"%4.4d%2.2d%2.2d", 1900+ftime->tm_year,ftime->tm_mon+1,ftime->tm_mday);
		return(strcmp(szFileDate,szActualDate));
	}
	return -2;
}

/*
* GetFileDateTime()
*/
char *GetFileDateTime(char *pszFileName)
{
	struct stat st;
	struct tm *ftime;
	static char szFileDate[128];

	/* 
	* ricavo la data del file 
	*/
	if(!stat(pszFileName,&st)){
		ftime = localtime(&(st.st_mtime));
		sprintf(szFileDate,"%2.2d-%2.2d-%4.4d %02d:%02d:%02d",
			ftime->tm_mday, ftime->tm_mon+1,1900+ftime->tm_year,
			ftime->tm_hour, ftime->tm_min, ftime->tm_sec);
	}
	return szFileDate;
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
* StrIsAlpha()
*/
BOOL StrIsAlpha(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isalpha(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}

/*
* StrIsDigit()
*/
BOOL StrIsDigit(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isdigit(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}

/*
* StrIsAlnum()
*/
BOOL StrIsAlnum(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isalnum(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}

/*
* StrIsSpace()
*/
BOOL StrIsSpace(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isspace(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}

/*
* StrIsLower()
*/
BOOL StrIsLower(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!islower(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}


/*
* StrIsUpper()
*/
BOOL StrIsUpper(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isupper(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}


/*
* StrIsXdigit()
*/
BOOL StrIsXdigit(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isxdigit(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}


/*
* StrIsAscii()
*/
BOOL StrIsAscii(char *str)
{
	int nIndex=0;

	while(str[nIndex]){
		if(!isascii(str[nIndex]))
			return FALSE;
		nIndex++;
	}
	return TRUE;
}

/*
* Sostituzione di caretteri
*/
char *Substitute(char *szString,char c1,char c2)
{
	char *pPtr;

	if(c1==c2){
		return szString;
	}

	/*
	* sostituzione di (c1 )
	*/ 
	while ((pPtr=strchr(szString,c1))){
		*pPtr = c2;
	}
	return szString;
}

/*
* RunSimpleProcess()
* lancio il processo nProcID
*/
BOOL RunSimpleProcess(int nProcID,char *szPath)
{
	char szProcName[128];
	char szPID[128];

	if(ProcList[nProcID].bExecute){

		/* 
		* Creazione coda messaggi locale per il processo
		*/
		if(ProcList[nProcID].nQKey){
			/* 
			* Creazione coda messaggi locale 
			*/
			if((ProcList[nProcID].nQNumber = CreateMsgQ(ProcList[nProcID].nQKey, 1))<0){
#ifdef TRACE
				trace_out_vstr(1,"Creazione coda msg %s (%d) fallita",
					ProcList[nProcID].szProcName,
					ProcList[nProcID].nQKey);
#endif
			}
		}

		sprintf(szProcName,"%s/%s",szPath,ProcList[nProcID].szProcName);
		sprintf(szPID,"-p %d",nProcID);
		ProcList[nProcID].nProcId = ExecModule(szProcName, szProcName,szPID,NULL);

		if (ProcList[nProcID].nProcId < 0) {
#ifdef TRACE
			trace_out_vstr(1,"Fallita esecuzione modulo %s",
				ProcList[nProcID].szProcName);
#endif
			return FALSE;
		}
	}
	return TRUE;
}

void DetachSharedMemory(char *shm_ptr)
{
    DetachShm(shm_ptr);
}

int CheckSharedMemory(int shm_key, int shm_size)
{
    return CheckShm(shm_key, shm_size);
}

char *CreateSharedMemory(int shm_key, int shm_size)
{
    return SetupShm(shm_key, shm_size);
}

char *OpenSharedMemory(int shm_key, int shm_size)
{
    return GetShm(shm_key, shm_size);
}

int DeleteSharedMemory(int shm_key, int shm_size)
{
    return RemoveShm(shm_key, shm_size);
}

int CreateProcessMsgQ(int nProcID, int clear)
{
    return (ProcList[nProcID].nQNumber = CreateMsgQ(ProcList[nProcID].nQKey, 1));
}

int OpenProcessMsgQ(int nProcID)
{
    return (ProcList[nProcID].nQNumber=OpenMsgQ(ProcList[nProcID].nQKey));
}

void DeleteProcessMsgQ(int nProcID)
{
    if(ProcList[nProcID].nQKey){
        DeleteMsgQ(ProcList[nProcID].nQNumber);
    }
}

void TerminateProcess(int nProcID)
{
	if(ProcList[nProcID].bExecute){
		kill((pid_t)ProcList[nProcID].nProcId, SIGTERM);
        DeleteProcessMsgQ(nProcID);
	}
}

char *ProcessGetName(int nProcID)
{
    return ProcList[nProcID].szProcName;
}

int ProcessReceiveMsg(int nProcID, int *mtype, char *msg)
{
	return ReceiveMsg(ProcList[nProcID].nQNumber, mtype, msg);
}


int ProcessReceiveMsgNoWait(int nProcID, int *mtype, char *msg)
{
	return ReceiveMsgNoWait(ProcList[nProcID].nQNumber, mtype, msg);
}

/*
* ReadProcInfo()
* Lettura dei parametri relativi ai processi 
*/
void ReadProcInfo(char *szFileConfig)
{
	char szProcIndex[128];
	char szProcBuffer[128];
	int nIndex;
	BOOL bOK=TRUE;

	nIndex=0;
	while(bOK){
		sprintf(szProcIndex,"Proc_%02d",nIndex);
		if(GetFileString("procinfo",szProcIndex, "", szProcBuffer, 80,szFileConfig,NULL)){
			sscanf(szProcBuffer, "%[^,],%d,%d,%d",
					ProcList[nIndex].szProcName,
					&ProcList[nIndex].nQKey,
					&ProcList[nIndex].nPriority,
					&ProcList[nIndex].bExecute);
			ProcList[nIndex].nGId=nIndex+1;
			/* 
			* rm 16-01-2002 : Elimino gli spazi in testa e in coda per 
			*                 problemi con la RunSimpleProcess() 
			*/
			StrTrimAll(ProcList[nIndex].szProcName);

			/*
			* resetto il numero della coda messaggi del processo
			*/
			ProcList[nIndex].nQNumber=0;
		} else {
			ProcList[nIndex].szProcName[0]='\0';
			ProcList[nIndex].nQKey=0;
			ProcList[nIndex].nPriority=0;
			ProcList[nIndex].bExecute=0;
			ProcList[nIndex].nGId=0;
			bOK=FALSE;
		}
		nIndex++;
	}
}

/*
* SendMessageNoWait()
* Spedizione messaggio con timeout e tentativi (NON BLOCCANTE)
*/
BOOL SendMessageNoWait(int nDest,int nSrce,int nMsgCode,char *szText,int nTimeOut,int nRetries)
{
	int nRC;

	if((nRC=SendSpecificMsgNoWait(ProcList[nDest].nQNumber, ProcList[nDest].nGId, ProcList[nSrce].nGId, nMsgCode, szText,nTimeOut,nRetries))!=0){
		trace_out_vstr_date(1,"Errore [%d] in spedizione Messaggio a [%s]", nRC,ProcList[nDest].szProcName);
		return FALSE;
	}
	return TRUE;
}


/*
* SendMessage()
* Spedizione messaggio con timeout e tentativi (NON BLOCCANTE)
*/
BOOL SendMessage(int nDest,int nSrce,int nMsgCode,char *szText)
{
	int nRC;

	if((nRC=SendSpecificMsg(ProcList[nDest].nQNumber, ProcList[nDest].nGId, ProcList[nSrce].nGId, nMsgCode, szText))!=0){
		trace_out_vstr_date(1,"Errore [%d] in spedizione Messaggio a [%s]", nRC,ProcList[nDest].szProcName);
		return FALSE;
	}
	return TRUE;
}



/*
 *	****************************************************************************
 *
 *	Funzione GetFileBuffer
 *
 *	Legge il file in ingresso e ne deposita il contenuto in un buffer allocato
 *	dinamicamente.
 *
 *	char	*pszFile	-	nome del file,
 *	FILE	*pFile		-	puntatore al file.
 *
 *	Se si passa il puntatore al file viene usato questo e quindi non e'
 *	necessario passare anche il nome del file.
 *
 *	Ritorna:
 *		il puntatore al buffer allocato (bisogna fare una free quando non si
 *		usa piu').
 *
 *	****************************************************************************
 */
char *GetFileBuffer(char *pszFile, FILE *pFile)
{
	int			nRead;
	char		*pszFileBuffer=NULL;
	long		lFileSize;
	FILE		*fp;
	BOOL		bMyOpen=FALSE;
	struct stat	FileStat;


	if( pFile!=NULL )
	{
		/*
		 *	Puntatore al file gia' aperto
		 */
		fp=pFile;
		bMyOpen=FALSE;

		fseek(fp, 0, 0);
	}
	else
	{
		/*
		 *	Apro il file per conto mio
		 */
		if( (fp=fopen(pszFile, "r"))==NULL )
		{
			fprintf(stderr,"GetFileBuffer() : File %s not found\r\n", pszFile);
			return(NULL);
		}
		bMyOpen=TRUE;
	}

	/*
	 *	Ricavo la dimensione del file.
	 */
	fstat(fileno(fp), &FileStat);
	lFileSize=FileStat.st_size;

	if( lFileSize>0 )
	{
		/*
		 *	Alloco il buffer necessario (+1 per il \0)
		 */
		if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
		{

			if( bMyOpen )
				fclose(fp);
			return(NULL);
		}

		/*
		 *	Leggo il file ed aggiungo un \0 in fondo.
		 */
		nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
		pszFileBuffer[lFileSize]='\0';

		if( nRead!=(int)lFileSize )
		{
			free(pszFileBuffer);

			if( bMyOpen )
				fclose(fp);
			return(NULL);
		}
	}

	if( bMyOpen )
		fclose(fp);

	return(pszFileBuffer);
}	/*	Fine GetFileBuffer */


/*
 *	****************************************************************************
 *
 *	Funzione GetBufferParagraph
 *
 *	Cerca il paragrafo pszParagraph all'interno del buffer del file e ne ritorna
 *	il puntatore.
 *
 *	char	*pszFile		-	nome del file per printf di errore,
 *	char	*pszParagraph	-	[PARAGRAFO],
 *	char	*pszFileBuffer	-	buffer del file.
 *
 *	Ritorna:
 *		puntatore al paragrafo nel buffer del file
 *
 *	****************************************************************************
 */
char *GetBufferParagraph( char	*pszFile, char	*pszParagraph, char	*pszFileBuffer)
{
	char	*pszPara;
	char	szPara[128];


	if( pszFileBuffer==NULL )
	{
		/*
		 *	ERRORE puntatore a file nullo
		 */
		return(NULL);
	}

	sprintf(szPara, "[%s]", pszParagraph);

	if( (pszPara=strstr(pszFileBuffer, szPara))!=NULL )
	{
		/*
		 *	Paragrafo trovato.
		 *
		 *	19.06.95	MF	aggiunta la somma di strlen
		 */
		return(pszPara+strlen(szPara));
	}

	/*
	 *	Debug solo se necessario
	 */
	if( pszFile!=NULL )
		fprintf(stderr,"GetBufferParagraph() : file %s - paragraph %s not found\r\n", pszFile, pszParagraph);

	/*
	 *	Paragrafo non trovato
	 */
	return(NULL);
}	/*	Fine GetBufferParagraph */


/*
 *	****************************************************************************
 *
 *	Funzione GetBufferString
 *
 *	Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *	pszParagraph letto da pszFileBuffer.
 *	La ricerca viene iniziata dal puntatore pszParagraph.
 *
 *	char	*pszParagraph	-	puntatore al [PARAGRAFO] nel buffer
 *	char	*pszItem		-	DATO=
 *	char	*pszDefault		-	stringa di default,
 *	char	*pszBufDep		-	deposito della stringa (dato=STRINGA)
 *	int		nLen			-	lunghezza buffer di deposito (escluso \0)
 *	char	*pszFileBuffer	-	buffer del file
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL GetBufferString( char	*pszParagraph, char	*pszItem, char	*pszDefault, char	*pszBufDep, int		nLen, char	*pszFileBuffer)
{
	int		nLunghString;
	char	*pszTmp=NULL;
	char	*pszNewLine=NULL;
	char	szElem[128];


	/*
	 *	Per prima cosa metto il default
	 */
	strcpy(pszBufDep, pszDefault);

	if( pszFileBuffer==NULL || pszParagraph==NULL )
		return(FALSE);

	sprintf(szElem, "%s=", pszItem);

	pszTmp=pszParagraph;

	/*
	 *	Loop sulla ricerca del LineFeed
	 */
	while( *pszTmp!='\0' && (pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL )
	{
		++pszNewLine;

		/*
		 *	Salto l'eventuale CarriageReturn
		 */
		if( *pszNewLine==CHAR_CR )
			++pszNewLine;

		switch( *pszNewLine )
		{
			case '[':
				/*
				 *	Il paragrafo che volevo era gia' stato trovato e quindi,
				 *	avendone trovato un altro, ho finito la ricerca.
				 */
				return(FALSE);
			break;

			default:
				/*
				 *	Cerco il <dato>=
				 */
				if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
				{
					/*
					 *	Trovato!
					 */
					pszTmp=pszNewLine+strlen(szElem);

					/*
					 *	Cerco l'eventuale 0A e calcolo quanti caratteri copiare
					 */
					if( (pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL )
						nLunghString=min(pszNewLine-pszTmp, nLen);
					else
						nLunghString=min((int)strlen(pszTmp), nLen);

					memcpy(pszBufDep, pszTmp, nLunghString);
					pszBufDep[nLunghString]='\0';

					return(TRUE);
				}
			break;
		}

		pszTmp=pszNewLine;
	}

	return(FALSE);
}	/*	Fine GetBufferString */


/*
 *	****************************************************************************
 *
 *	Funzione GetBufferInt
 *
 *	Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *	pszParagraph letto da pszFileBuffer.
 *	La ricerca viene iniziata dal puntatore pszParagraph.
 *
 *	char	*pszParagraph	-	puntatore al [PARAGRAFO] nel buffer
 *	char	*pszItem		-	DATO=
 *	int		nDefault		-	valore di default,
 *	char	*pszFileBuffer	-	buffer del file
 *
 *	Ritorna:
 *		il numero desiderato.
 *
 *	****************************************************************************
 */
int GetBufferInt( char	*pszParagraph, char	*pszItem, int		nDefault, char	*pszFileBuffer)
{
	char	szBuffer[80];
	BOOL	bRC;

	bRC=GetBufferString(
		pszParagraph,
		pszItem,
		"",
		szBuffer,
		sizeof(szBuffer)-1,
		pszFileBuffer
	);

	/*
	 *	Con errore ritorno il default
	 */
	if( !bRC )
		return(nDefault);

	return(atoi(szBuffer));
}	/*	Fine GetBufferInt */


/*
 *	****************************************************************************
 *
 *	Funzione GetFileString
 *
 *	Riempie pszBufDep con il valore a destra dell'= di pszItem nel paragrafo
 *	pszParagraph letto da pFile.
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	char	*pszDefault		-	valore di default con cui riempire pszBufDep
 *	char	*pszBufDep		-	deposito della stringa (dato=STRINGA)
 *	int		nLen			-	lunghezza buffer di deposito (escluso \0)
 *	char	*pszFile		-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto (lettura o
 *								modifica).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il
 *	puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve esser
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL GetFileString( char	*pszParagraph, char	*pszItem, char	*pszDefault, char	*pszBufDep, int		nLen, char	*pszFile, FILE	*pFile)
{
	int		nLunghString;
	char	*pszTmp;
	char	szPara[128];
	char	szElem[128];
	char	szBuffer[4096];
	FILE	*fp;
	BOOL	bPara=FALSE;
	BOOL	bMyOpen=FALSE;
    int     bFound = FALSE;


	sprintf(szPara, "[%s]", pszParagraph);
	sprintf(szElem, "%s=", pszItem);


	if( pFile!=NULL ) {
		/*
		 *	Puntatore a file gia' aperto
		 */
		fp=pFile;
		bMyOpen=FALSE;

		fseek(fp, 0, 0);
	} else {
		/*
		 *	Apro il file per conto mio
		 */
		if( (fp=fopen(pszFile, "r"))==NULL )
		{
			fprintf(stderr,"GetFileString() : file %s not found\r\n", pszFile);
			bFound = FALSE;
		}
		bMyOpen=TRUE;
	}

	/*
	 *	Loop di lettura riga x riga del file
	 */
    bFound = -1;
	while( bFound == -1 && fgets(szBuffer, sizeof(szBuffer), fp)!=NULL ) {
		if( (pszTmp=strchr(szBuffer, CHAR_CR))!=NULL || (pszTmp=strchr(szBuffer, CHAR_LF))!=NULL) {
			*pszTmp='\0';
		}

		if( *szBuffer=='[' ) {
			/*
			 *	E' un paragrafo
			 */


			if( strcmp(szPara, szBuffer)==0 ) {
				/*
				 *	Trovato il paragrafo ricercato.
				 */
				bPara=TRUE;
			} else {
				if( bPara ) {
					/*
					 *	Il paragrafo che volevo era gia' stato trovato e quindi,
					 *	avendone trovato un altro, ho finito la ricerca.
					 */
					bFound = FALSE;
				}
			}
		} else {
			/*
			 *	Non e' un paragrafo e quindi sara' un elemento
			 */
			if( bPara && (pszTmp=strchr(szBuffer, '='))!=NULL ) {
				/*
				 *	Il paragrafo corrisponde, l'uguale e' stato trovato,
				 *	controllo che sia effettivamente quello che cerco.
				 */
				if( memcmp(szElem, szBuffer, strlen(szElem))==0 ) {
					/*
					 *	Trovato!!
					 */
					++pszTmp;

					bFound = TRUE;

				}
			}
		}
	}

    /*
    * rm 08-07-2021 : correzione in caso di mancanza paragrafo
    */
    bFound = bFound==-1?FALSE:bFound;

    if(bFound){
        nLunghString=min((int)strlen(pszTmp), nLen);
        memcpy(pszBufDep, pszTmp, nLunghString);
        pszBufDep[nLunghString]='\0';
    } else {
        strcpy(pszBufDep, pszDefault);
    }

	if( bMyOpen )
		fclose(fp);

	return(bFound);
}	/*	Fine GetFileString */


/*
 *	****************************************************************************
 *
 *	Funzione GetFileInt
 *
 *	Ritorna l'intero a destra dell'= dell'elemento letto da file nel
 *	paragrafo specificato.
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	int		nDefault		-	valore di default da ritornare
 *	char	*szFile			-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto (lettura o
 *								modifica).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		il valore letto
 *
 *	****************************************************************************
 */
int GetFileInt( char	*pszParagraph, char	*pszItem, int		nDefault, char	*pszFile, FILE	*pFile)
{
	char	szBuffer[80];
	BOOL	bRC;

	bRC=GetFileString( pszParagraph, pszItem, "", szBuffer, sizeof(szBuffer)-1, pszFile, pFile);

	/*
	 *	Con errore ritorno il default
	 */
	if( !bRC )
		return(nDefault);

	return(atoi(szBuffer));
}	/*	Fine GetFileInt */

/*
 *	****************************************************************************
 *
 *	Funzione PutFileString
 *
 *	Scrive nel paragrafo specificato elemento=valore
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	char	*pszValue		-	dato=VALORE
 *	char	*szFile			-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto in modifica (r+).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL PutFileString( char	*pszParagraph, char	*pszItem, char	*pszValue, char	*pszFile, FILE	*pFile)
{
	int			nPos;
	int			nRead;
	int			nOldLen=0;
	int			nLenElemVal;
	char		*pszFileBuffer=NULL;
	char		*pszTmp=NULL;
	char		*pszNewLine=NULL;
	char		szPara[128];
	char		szElem[128];
	char		szElemVal[128];
	long		lFileSize;
	FILE		*fp=NULL;
	BOOL		bMyOpen=FALSE;
	BOOL		bFound=FALSE;
	struct stat	FileStat;


	sprintf(szPara, "[%s]", pszParagraph);
	sprintf(szElem, "%s=", pszItem);
	sprintf(szElemVal, "%s%s", szElem, pszValue);

	nLenElemVal=strlen(szElemVal);

	if( pFile!=NULL )
	{
		/*
		 *	Utilizzo il puntatore al file gia' aperto
		 */
		fp=pFile;
		bMyOpen=FALSE;

		fseek(fp, 0, 0);
	}
	else
	{
		/*
		 *	Apro il file per conto mio
		 */
		if( (fp=fopen(pszFile, "r+"))==NULL )
		{
			/*
			 *	File non trovato, lo apro in scrittura
			 */
			if( (fp=fopen(pszFile, "w"))==NULL )
			{
				return(FALSE);
			}
		}
		bMyOpen=TRUE;
	}

	/*
	 *	Ricavo la dimensione del file
	 */
	fstat(fileno(fp), &FileStat);
	lFileSize=FileStat.st_size;

	if( lFileSize>0 )
	{
		/*
		 *	Alloco il buffer (+1 per il \0)
		 */
		if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
		{
			if( bMyOpen )
				fclose(fp);
			return(FALSE);
		}

		/*
		 *	Leggo il file e metto il \0 finale
		 */
		nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
		pszFileBuffer[lFileSize]='\0';

		if( nRead!=(int)lFileSize )
		{
			free(pszFileBuffer);

			if( bMyOpen )
				fclose(fp);
			return(FALSE);
		}
	}

	/*
	 *	La dimensione effettiva del file non e' quella letta ma la lunghezza
	 *	del buffer perche' in caso di cancellazione vengono messi degli zeri
	 *	in fondo che fanno dei casini.
	 */
	lFileSize=strlen(pszFileBuffer);

	if( lFileSize>0 && (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
	{
		/*
		 *	Paragrafo trovato.
		 */

		/*
		 *	Loop sulla ricerca del LineFeed
		 */
		while(
			!bFound &&
			pszTmp-pszFileBuffer<=lFileSize &&
			(pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL
		)
		{
			++pszNewLine;

			/*
			 *	Salto l'eventuale CarriageReturn
			 */
			if( *pszNewLine==CHAR_CR )
				++pszNewLine;

			switch( *pszNewLine )
			{
				case '[':
					/*
					 *	Nuovo paragrafo -> ho finito la ricerca
					 */
					bFound=TRUE;
				break;

				default:
					if( isprint(*pszNewLine) )
					{
						/*
						 *	Cerco il <dato>=
						 */
						if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
						{
							bFound=TRUE;

							/*
							 *	Controllo <dato>=<value>
							 */
							if(
								memcmp(pszNewLine, szElemVal, nLenElemVal)==0 &&
								(*(pszNewLine+nLenElemVal)==CHAR_CR ||
								*(pszNewLine+nLenElemVal)==CHAR_LF)
							)
							{
								/*
								 *	Trovato dato e valore uguali nello stesso
								 *	paragrafo (DATO=VAL).
								 */
								if( bMyOpen )
									fclose(fp);
								free(pszFileBuffer);

								return(TRUE);
							}
						}
					}
				break;
			}

			pszTmp=pszNewLine;
		}

		if( pszNewLine==NULL )
			pszNewLine=pszFileBuffer+lFileSize;

		/*
		 *	Calcolo l'offset per FSEEK
		 */
		nPos=min((int)(pszNewLine-pszFileBuffer), (int)lFileSize);

		/*
		 *	Mi posiziono e scrivo il dato sul file
		 */
		fseek(fp, nPos, 0);
		fputs(szElemVal, fp);
		fputs("\n", fp);

		if( nPos<(int)lFileSize )
		{
			/*
			 *	Ci sono altri caratteri da accodare
			 */
			if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
			{
				char *pszOldVal;

				/*
				 *	Sono fermo sul vecchio dato
				 */
				pszOldVal=strchr(pszNewLine, CHAR_LF);

				if( pszOldVal!=NULL ){
					/*
					* Ricavo la lunghezza del dato precedente
					*/
					nOldLen=pszOldVal-pszNewLine;

					++pszOldVal;
				}
				pszNewLine=pszOldVal;
			}

			if( pszNewLine!=NULL )
			{
				fwrite(
					pszNewLine,
					sizeof(char),
					lFileSize-(pszNewLine-pszFileBuffer),
					fp
				);

				/*
				* rm + mf 04-12-2001 : cambio la dimensione del file per evitare 'sporcature in fondo'
				*/
				if( nOldLen>nLenElemVal ){
					ftruncate(fileno(fp),lFileSize-(nOldLen-nLenElemVal));
				}
			}
		}
	}
	else
	{
		/*
		 *	Paragrafo non trovato, accodo il tutto.
		 */
		fseek(fp, lFileSize, 0);

		fputs("\n", fp);
		fputs(szPara, fp);
		fputs("\n", fp);
		fputs(szElemVal, fp);
		fputs("\n", fp);
	}

	if( bMyOpen )
		fclose(fp);
	else
		fflush(fp);

	free(pszFileBuffer);

	return(TRUE);
}	/*	Fine PutFileString */


/*
 *	****************************************************************************
 *
 *	Funzione PutFileInt
 *
 *	Scrive nel paragrafo specificato elemento=numero intero.
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	int		nValue			-	dato=VALORE
 *	char	*szFile			-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto in modifica (r+).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL PutFileInt( char	*pszParagraph, char	*pszItem, int		nValue, char	*pszFile, FILE	*pFile)
{
	char	szBuffer[80];
	BOOL	bRC;

	sprintf(szBuffer, "%d", nValue);

	bRC=PutFileString(
		pszParagraph,
		pszItem,
		szBuffer,
		pszFile,
		pFile
	);

	return(bRC);
}	/*	Fine PutFileInt */


/*
 *	****************************************************************************
 *
 *	Funzione DelFileString
 *
 *	Cancella dal paragrafo specificato la stringa elemento=valore.
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	char	*szFile			-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto in modifica (r+).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL DelFileString( char	*pszParagraph, char	*pszItem, char	*pszFile, FILE	*pFile)
{
	int			nPos;
	int			nRead;
	int			nIndex;
	char		*pszFileBuffer=NULL;
	char		*pszTmp=NULL;
	char		*pszNewLine=NULL;
	char		szPara[128];
	char		szElem[128];
	long		lFileSize;
	FILE		*fp=NULL;
	BOOL		bMyOpen=FALSE;
	BOOL		bFound=FALSE;
	struct stat	FileStat;


	sprintf(szPara, "[%s]", pszParagraph);
	sprintf(szElem, "%s=", pszItem);

	if( pFile!=NULL )
	{
		/*
		 *	Utilizzo il puntatore al file gia' aperto
		 */
		fp=pFile;
		bMyOpen=FALSE;

		fseek(fp, 0, 0);
	}
	else
	{
		/*
		 *	Apro il file per conto mio
		 */
		if( (fp=fopen(pszFile, "r+"))==NULL )
		{
			fprintf(stderr,"DelFileString() : file %s not found\r\n", pszFile);
			return(FALSE);
		}
		bMyOpen=TRUE;
	}

	/*
	 *	Ricavo la dimensione del file
	 */
	fstat(fileno(fp), &FileStat);
	lFileSize=FileStat.st_size;

	if( lFileSize<=0 )
	{
		/*
		 *	Il file e' gia' vuoto, cosa cancello?
		 */
		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
	{
		/*
		 *	Non ottengo la memoria che mi serve.
		 */
		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	/*
	 *	Leggo tutto il file e metto il barra-zero di sicurezza
	 */
	nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
	pszFileBuffer[lFileSize]='\0';

	if( nRead!=(int)lFileSize )
	{
		free(pszFileBuffer);

		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	if( (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
	{
		/*
		 *	Paragrafo trovato.
		 */

		/*
		 *	Cerco il LineFeed
		 */
		while(
			!bFound &&
			pszTmp-pszFileBuffer<=lFileSize &&
			(pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL
		)
		{
			++pszNewLine;

			/*
			 *	Salto l'eventuale CarriageReturn
			 */
			if( *pszNewLine==CHAR_CR )
				++pszNewLine;

			switch( *pszNewLine )
			{
				case '[':
					/*
					 *	E' finito il paragrafo perche' ne e' iniziato uno nuovo
					 *	e percio' ho finito la ricerca
					 */
					if( bMyOpen )
						fclose(fp);
					free(pszFileBuffer);

					return(FALSE);
				break;

				default:
					/*
					 *	Cerco il <DATO>=
					 */
					if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
						bFound=TRUE;
				break;
			}

			pszTmp=pszNewLine;
		}

		if( pszNewLine==NULL )
			pszNewLine=pszFileBuffer+lFileSize;

		nPos=min((int)(pszNewLine-pszFileBuffer), (int)lFileSize);

		if( nPos<(int)lFileSize )
		{
			/*
			 *	Ci sono altri caratteri da accodare
			 */
			fseek(fp, nPos, 0);

			/*
			 *	Sono fermo sul <dato>= e percio' vado al prossimo LF
			 */
			pszNewLine=strchr(pszNewLine, CHAR_LF);

			if( pszNewLine!=NULL )
			{
				++pszNewLine;

				/*
				 *	Salto l'eventuale CarriageReturn
				 */
				if( *pszNewLine==CHAR_CR )
					++pszNewLine;

				fwrite(
					pszNewLine,
					sizeof(char),
					lFileSize-(pszNewLine-pszFileBuffer)-1,
					fp
				);

				for(
					nIndex=0;
					nIndex<(pszNewLine-pszFileBuffer)-nPos;
					nIndex++
				)
				{
					/*
					 *	Scrivo zeri perche' non interferiscono con il file se
					 *	viene editato con un editor (e a me non da fastidio).
					 */
					fputc(0x00, fp);
				}
			}
		}
	}

	if( bMyOpen )
		fclose(fp);
	else
		fflush(fp);

	free(pszFileBuffer);

	return(TRUE);
}	/*	Fine DelFileString */


/*
 *	****************************************************************************
 *
 *	Funzione DelFileParagraph
 *
 *	Cancella il paragrafo specificato.
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*szFile			-	nomefile
 *	FILE	*pFile			-	puntatore a file gia' aperto in modifica (r+).
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL DelFileParagraph(char *pszParagraph, char *pszFile, FILE *pFile)
{
	int			nPos;
	int			nRead;
	int			nIndex;
	char		*pszStartPara=NULL;
	char		*pszFileBuffer=NULL;
	char		*pszTmp=NULL;
	char		*pszNewLine=NULL;
	char		szPara[128];
	long		lFileSize;
	FILE		*fp=NULL;
	BOOL		bMyOpen=FALSE;
	BOOL		bFound=FALSE;
	struct stat	FileStat;


	sprintf(szPara, "[%s]", pszParagraph);

	if( pFile!=NULL )
	{
		/*
		 *	Utilizzo il puntatore al file gia' aperto
		 */
		fp=pFile;
		bMyOpen=FALSE;

		fseek(fp, 0, 0);
	}
	else
	{
		/*
		 *	Apro il file per conto mio
		 */
		if( (fp=fopen(pszFile, "r+"))==NULL )
		{
			fprintf(stderr,"DelFileParagraph() : file %s not found\r\n", pszFile);
			return(FALSE);
		}
		bMyOpen=TRUE;
	}

	/*
	 *	Ricavo la dimensione del file
	 */
	fstat(fileno(fp), &FileStat);
	lFileSize=FileStat.st_size;

	if( lFileSize<=0 )
	{
		/*
		 *	Il file e' gia' vuoto, cosa cancello?
		 */
		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	if( (pszFileBuffer=malloc(lFileSize+1))==NULL )
	{
		/*
		 *	Non ottengo la memoria che mi serve.
		 */
		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	/*
	 *	Leggo tutto il file e metto il barra-zero di sicurezza
	 */
	nRead=fread(pszFileBuffer, sizeof(char), (int)lFileSize, fp);
	pszFileBuffer[lFileSize]='\0';

	if( nRead!=(int)lFileSize )
	{
		free(pszFileBuffer);

		if( bMyOpen )
			fclose(fp);
		return(FALSE);
	}

	if( (pszStartPara=strstr(pszFileBuffer, szPara))!=NULL )
	{
		/*
		 *	Paragrafo trovato.
		 */
		pszTmp=pszStartPara;

		/*
		 *	Cerco il LineFeed
		 */
		while(
			!bFound &&
			pszTmp-pszFileBuffer<=lFileSize &&
			(pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL
		)
		{
			++pszNewLine;

			/*
			 *	Salto l'eventuale CarriageReturn
			 */
			if( *pszNewLine==CHAR_CR )
				++pszNewLine;

			switch( *pszNewLine )
			{
				case '[':
					/*
					 *	E' finito il paragrafo perche' ne e' iniziato uno nuovo
					 *	e percio' ho finito la ricerca
					 */
					bFound=TRUE;
				break;
			}

			pszTmp=pszNewLine;
		}

		if( pszNewLine==NULL )
			pszNewLine=pszFileBuffer+lFileSize;

		nPos=min((int)(pszStartPara-pszFileBuffer), (int)lFileSize);

		if( nPos<(int)lFileSize )
		{
			/*
			 *	Ci sono altri caratteri da accodare
			 */
			fseek(fp, nPos, 0);

			fwrite(
				pszNewLine,
				sizeof(char),
				lFileSize-(pszNewLine-pszFileBuffer)-1,
				fp
			);

			for(
				nIndex=0;
				nIndex<(pszNewLine-pszFileBuffer)-nPos;
				nIndex++
			)
			{
				/*
				 *	Scrivo zeri perche' non interferiscono con il file se
				 *	viene editato con un editor (e a me non da fastidio).
				 */
				fputc(0x00, fp);
			}
		}
	}

	if( bMyOpen )
		fclose(fp);
	else
		fflush(fp);

	free(pszFileBuffer);

	if( pszStartPara==NULL )
		return(FALSE);

	return(TRUE);
}	/*	Fine DelFileParagraph */


#ifdef PROVE_PER_SCRITTURA_SU_BUFFER
/*
 *	****************************************************************************
 *
 *	Funzione PutBufferString
 *
 *	Scrive nel paragrafo specificato elemento=valore
 *
 *	char	*pszParagraph	-	[PARAGRAFO]
 *	char	*pszItem		-	DATO=
 *	char	*pszValue		-	dato=VALORE
 *	char	*pszFileBuffer	-	buffer del file
 *
 *	E' possibile passare in ingresso il nome del file da aprire o il puntatore
 *	al file aperto.
 *	Se il puntatore al file e' diverso da NULL il nome del file puo' essere
 *	NULL, ma se il puntatore al file e' NULL il nome del file non deve essere
 *	NULL.
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
char *PutBufferString(
	char	*pszParagraph,
	char	*pszItem,
	char	*pszValue,
	char	*pszFileBuffer
)
{
	char	*pszTmp=NULL;
	char	*pszNewLine=NULL;
	char	szPara[128];
	char	szElem[128];
	char	szElemVal[128];
	char	*pszNewFileBuffer=NULL;


	if( pszFileBuffer==NULL || pszParagraph==NULL || strlen(pszParagraph)<=0 )
	{
		return(NULL);
	}

	sprintf(szPara, "[%s]", pszParagraph);
	sprintf(szElem, "%s=", pszItem);
	sprintf(szElemVal, "%s%s", szElem, pszValue);

	if( (pszTmp=strstr(pszFileBuffer, szPara))!=NULL )
	{
		/*
		 *	Paragrafo trovato.
		 */

		/*
		 *	Loop sulla ricerca del LineFeed
		 */
		while( *pszTmp!='\0' && (pszNewLine=strchr(pszTmp, CHAR_LF))!=NULL )
		{
			++pszNewLine;
			/*
			 *	Salto l'eventuale CarriageReturn
			 */
			if( *pszNewLine==CHAR_CR )
				++pszNewLine;

			switch( *pszNewLine )
			{
				/*
				 *	Nuovo paragrafo -> ho finito la ricerca
				 */
				case '[':
				{
					char	*pszDove;

					pszDove=pszNewLine;

					/*
					 *	La realloc e' da modificare
					 */
					pszNewFileBuffer=realloc(pszFileBuffer, strlen(szElemVal));
					if( pszNewFileBuffer==NULL )
						return(NULL);

					pszFileBuffer=pszNewFileBuffer;

					memmove(
						pszDove+strlen(szElemVal),
						pszDove,
						strlen(szElemVal)
					);
					memcpy(pszDove, szElemVal, strlen(szElemVal));
				}
				break;

				default:
					/*
					 *	Linea vuota? (se non e' un carattere ascii)
					 */
					if( isprint(*pszNewLine) )
					{
						/*
						 *	Cerco il <dato>=
						 */
						if( memcmp(pszNewLine, szElem, strlen(szElem))==0 )
						{
							/*
							 *	Controllo <dato>=<value>
							 */
							if( memcmp(pszNewLine, szElemVal, strlen(szElemVal))!=0 )
							{
								int		nStart=strlen(szElem);
								int		nEnd=strlen(pszValue);
								int		nIndex;


								for(
									nIndex=0;
									*(pszNewLine+nStart+nIndex)!=BLANK &&
										isprint(*(pszNewLine+nStart+nIndex)) &&
											nIndex<nEnd;
									nIndex++
								)
								{
									*(pszNewLine+nStart+nIndex)=*(pszValue+nIndex);
								}

								if( nIndex<nEnd )
								{
									/*
									 *	La realloc e' da modificare
									 */
									pszNewFileBuffer=realloc(
										pszFileBuffer,
										nEnd-nIndex
									);
									if( pszNewFileBuffer==NULL )
										return(NULL);

									pszFileBuffer=pszNewFileBuffer;

									memmove(
										pszNewLine+nStart+nEnd,
										pszNewLine+nStart+nIndex,
										nEnd-nIndex
									);

									memcpy(
										pszNewLine+nStart+nIndex,
										pszValue+nIndex,
										nEnd-nIndex
									);
								}
							}

							return(pszFileBuffer);
						}
					}
				break;
			}

			pszTmp=pszNewLine;
		}
	}

	pszTmp=pszFileBuffer;
	while( *pszTmp!='\0' )
		++pszTmp;

	/*
	 *	La realloc e' da modificare
	 */
	pszNewFileBuffer=realloc(pszFileBuffer, strlen(szPara)+strlen(szElemVal)+3);
	if( pszNewFileBuffer==NULL )
		return(NULL);

	pszFileBuffer=pszNewFileBuffer;

	/*
	 *	Paragrafo non trovato, accodo il tutto.
	 */
	strcpy(pszTmp, "\n");
	strcpy(pszTmp+strlen(pszTmp), szPara);
	strcpy(pszTmp+strlen(pszTmp), "\n");
	strcpy(pszTmp+strlen(pszTmp), szElemVal);
	strcpy(pszTmp+strlen(pszTmp), "\n");

	return(pszFileBuffer);
}	/*	Fine PutBufferString */
#endif


/*
 *	****************************************************************************
 *
 *	Funzione PutFileBuffer
 *
 *	Scrive il file con il contenuto passato in ingresso.
 *
 *	char	*pszFile		-	nome del file
 *	char	*pszFileBuffer	-	buffer del file
 *	long	lSize			-	dimensione del buffer
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL PutFileBuffer(char *pszFile, char *pszFileBuffer, long lSize)
{
	UINT	uWrite;
	FILE	*fp;

	/*
	 *	Apro il file per conto mio
	 */
	if( (fp=fopen(pszFile, "w"))==NULL )
	{
		return(FALSE);
	}

	uWrite=(UINT)fwrite(pszFileBuffer, sizeof(char), (UINT)lSize, fp);

	fclose(fp);

	if( uWrite!=(UINT)lSize )
		return(FALSE);

	return(TRUE);
}	/*	Fine PutFileBuffer */


/*
 *	****************************************************************************
 *
 *	Funzione BackupFile
 *
 *	Copia il file con l'estensione in piu' ".bak".
 *
 *	char	*pszFile		-	nome del file
 *
 *	Ritorna:
 *		TRUE	-	OK
 *		FALSE	-	errore.
 *
 *	****************************************************************************
 */
BOOL BackupFile(char *pszFile)
{
	char		*pszFileBuffer;
	char		*pszFileBak;
	BOOL		bRC;
	struct stat	FileStat;


	pszFileBak=malloc(strlen(pszFile)+strlen(BACK_EXTENSION));
	if( pszFileBak==NULL )
	{
		return(FALSE);
	}

	/*
	 *	Compongo il nome file .bak
	 */
	sprintf(pszFileBak, "%s%s", pszFile, BACK_EXTENSION);

	/*
	 *	Ottengo il buffer del file
	 */
	pszFileBuffer=GetFileBuffer(pszFile, NULL);
	if( pszFileBuffer==NULL )
	{
		free(pszFileBak);
		return(FALSE);
	}

	/*
	 *	Ricavo la dimensione del file.
	 */
	stat(pszFile, &FileStat);

	/*
	 *	Scrivo il nuovo file
	 */
	bRC=PutFileBuffer(pszFileBak, pszFileBuffer, (long)FileStat.st_size);

	free(pszFileBak);
	free(pszFileBuffer);

	return(bRC);
}	/*	Fine BackupFile */

void remove_parse_item(gpointer data, gpointer user_data)
{
	PPARSE_STRUCT pPS=(PPARSE_STRUCT)data;

	g_free(pPS->szName);
	g_free(pPS->szValue);
	g_free(pPS);
}

GList *add_item_to_parse(GList **PSList,char *szName,char *szValue,BOOL bCleanList)
{
	PPARSE_STRUCT pPS;
	BOOL bFound=FALSE;
	GList    *list=NULL;

	if(bCleanList){
		g_list_foreach(*PSList,remove_parse_item,NULL);
	}

	list=g_list_first(*PSList);
	while(list){
		pPS = list->data;

		if(pPS==(PPARSE_STRUCT)NULL){
			/*
			* Problemi di allocazione ?
			*/
#ifdef TRACE
			trace_out_vstr_date(1,"add_item_to_parse() : NULL Item");
#endif
			continue;
		}

		if(!strcmp(pPS->szName,szName)){
			bFound=TRUE;
			break;
		}
		list=g_list_next(list);
	}
	if(!bFound){
		pPS=g_new0(PARSE_STRUCT,1);
		pPS->szName=g_strdup(szName);
		pPS->szValue=g_strdup(szValue);
		*PSList=g_list_append(*PSList,pPS);
	} else {
		g_free(pPS->szName);
		g_free(pPS->szValue);
		pPS->szName=g_strdup(szName);
		pPS->szValue=g_strdup(szValue);
	}
	return *PSList;
}

/*
* Sostituisce i valori parametrici di una stringa
*/
char *ParseStatement(char *pszStatement,GList *PSList)
{
	char szTmp[4096];
	char *pPtr;
	PPARSE_STRUCT pPS;
	GList    *list=NULL;

	list=g_list_first(PSList);
	while(list){
		pPS = list->data;

		if(pPS==(PPARSE_STRUCT)NULL){
			/*
			* Problemi di allocazione ?
			*/
#ifdef TRACE
			trace_out_vstr_date(1,"ParseStatement() : NULL Item");
#endif
			continue;
		}
		while((pPtr=strstr(pszStatement,pPS->szName))){
			*pPtr='\0';
			strcpy(szTmp,pszStatement);
			strcat(szTmp,pPS->szValue);
			strcat(szTmp,pPtr+strlen(pPS->szName));
			strcpy(pszStatement,szTmp);
		}

		list=g_list_next(list);
	}

	return pszStatement;
}

/*
* ReadCfg()
* lettura della configurazione dal file cni.cfg
*/
BOOL ReadCfg(gchar *szCfgFileName, PCFGITEM pCfgItems, BOOL bReadProcInfo)
{
	char szParagraph[128];

	if(!FileExists(szCfgFileName)){
#ifdef TRACE
	trace_out_vstr_date(1,"Read Configuration : file [%s] not present", szCfgFileName);
#endif
		return FALSE;
	}

	/* CFGItems viene dichiarato e inizializzato in mainvar.c */
	ReadCfgItems(pCfgItems,szCfgFileName,0);
	
	if(bReadProcInfo){
		ReadProcInfo(szCfgFileName);
	}

	return TRUE;
}


BOOL ReadCfgItems(PCFGITEM pItems,char *pszCfgFileName,int nPID)
{
	int nIndex=0;
	GList *PS=NULL;
	char szBuffer[128];
	char szParagraph[128];

	sprintf(szBuffer,"%d",nPID);

	PS=add_item_to_parse(&PS,"#",szBuffer,FALSE);

	while(pItems[nIndex].pszItemName){

		strcpy(szParagraph,pItems[nIndex].pszParagraph);
		ParseStatement(szParagraph,PS);

		switch(pItems[nIndex].ItemType){
			case CFG_TYPE_BOOL:
				*(pItems[nIndex].ItemValue.pbBool) = GetFileInt( szParagraph, pItems[nIndex].pszItemName, pItems[nIndex].ItemDefaultValue.nInt, pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_INT:
				*(pItems[nIndex].ItemValue.pnInt) = GetFileInt( szParagraph, pItems[nIndex].pszItemName, pItems[nIndex].ItemDefaultValue.nInt, pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_STRING:
				GetFileString( szParagraph, pItems[nIndex].pszItemName, pItems[nIndex].ItemDefaultValue.szString, pItems[nIndex].ItemValue.pszString, 80,pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_NONE:
				/* eliminate compiler warning */
			break;
		}
		nIndex++;
	}
	return TRUE;
}

BOOL WriteCfgItems(PCFGITEM pItems,char *pszCfgFileName)
{
	int nIndex=0;

	while(pItems[nIndex].pszItemName){
		switch(pItems[nIndex].ItemType){
			case CFG_TYPE_BOOL:
				PutFileInt( pItems[nIndex].pszParagraph,pItems[nIndex].pszItemName,*(pItems[nIndex].ItemValue.pnInt),pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_INT:
				PutFileInt( pItems[nIndex].pszParagraph,pItems[nIndex].pszItemName,*(pItems[nIndex].ItemValue.pnInt),pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_STRING:
				PutFileString( pItems[nIndex].pszParagraph, pItems[nIndex].pszItemName, pItems[nIndex].ItemValue.pszString,pszCfgFileName,NULL); 
			break;
			case CFG_TYPE_NONE:
				/* eliminate compiler warning */
			break;
		}
		nIndex++;
	}
	return TRUE;
}

BOOL TraceCfgItems(PCFGITEM pItems)
{
	int nIndex=0;

	trace_out_vstr(1,"Lista Parametri");
	trace_out_vstr(1,"----------   --------------------   --------------------");
	while(pItems[nIndex].pszItemName){
		switch(pItems[nIndex].ItemType){
			case CFG_TYPE_BOOL:
				trace_out_vstr(1,"%-10.10s - %-20.20s : %d",pItems[nIndex].pszParagraph,pItems[nIndex].pszItemName,*(pItems[nIndex].ItemValue.pnInt)); 
			break;
			case CFG_TYPE_INT:
				trace_out_vstr(1,"%-10.10s - %-20.20s : %d",pItems[nIndex].pszParagraph,pItems[nIndex].pszItemName,*(pItems[nIndex].ItemValue.pnInt)); 
			break;
			case CFG_TYPE_STRING:
				trace_out_vstr(1,"%-10.10s - %-20.20s : %s",pItems[nIndex].pszParagraph,pItems[nIndex].pszItemName,pItems[nIndex].ItemValue.pszString); 
			break;
			case CFG_TYPE_NONE:
				/* eliminate compiler warning */
			break;
		}
		nIndex++;
	}
	trace_out_vstr(1,"----------   --------------------   --------------------");
	return TRUE;
}

/*
* -------------------------------------------------
* Gestione colloquio con PLC
* -------------------------------------------------
*/

extern int errno;



/*
* lista dei comandi utilizzati
*/
char *szCmds[]={
	"errs",     /* richiesta o sospensione trasmissione errori */
	"info",     /* informazioni sul funzionamento del PLC */
	"go",       /* GO PLC */
	"halt",     /* HALT PLC */
	"list",     /* lista dispositivi connessi */
	"memchk",   /* convalida e registra variabili di debug */
	"memcopy",  /* dati relativi alle variabili registrate con memchk */
	"set",      /* scrittura dei valori delle variabili */
	"stat",     /* gestione della statistica  */
	"status",   /* stato del PLC (HALT/GO) */
	"trace",    /* Gestione del trace A */
	"trace",    /* Gestione del trace C */
	"trace",    /* Gestione del trace D */
	"trace",    /* Gestione del trace E */
	"trace",    /* Gestione del trace M */
	"trace",    /* Gestione del trace T */
	"trace",    /* Gestione del trace V */
	"trigger",  /* Gestione del trigger A */
	"trigger",  /* Gestione del trigger C */
	"trigger",  /* Gestione del trace D */
	"var",      /* risoluzione di un simbolo */
	"ver",      /* versione di QPLC */
	"wadd",     /* Aggiunta di un breakpoint */
	"wcreate",  /* Creazione di una lista di breakpoint */
	"wdel",     /* Cancella una lista di watch */
	"wenable",  /* Abilita una lista di watch */
	"wreset",   /* Cancella tutte le liste di watch */
	"wshow",    /* Visualizza i dati di debug relativi all'ultimo ciclo */
	"Nothing",  /* niente ... */
	(char *)NULL
};

/*
* lista delle risposte ammesse
*/
char *szAnswrs[]={
	"OK",     /* Conferma ultimo comando eseguito      : OK */
	"A",      /* Messaggio asincrono                   : A <codice> [dati] */
	"E",      /* Errore                                : E <codice> */
	"D",      /* Messaggio sincrono                    : D <dati> */
	"L",      /* Lunghezza prossimo messaggio sincrono : L <lunghezza> */
	(char *)NULL
};

char *szErrs[]={
	NULL,
	"Command unknown",
	"Command too long",
	"Invalid arguments number",
	"invalid parameter",
	"PLC bad state",
};

PLC_TYPES PLCTypes[]={
	{"BOOL",1},
	{"BYTE",1},
	{"INT",2},
	{"SINT",1},
	{"DINT",4},
	{"USINT",1},
	{"UINT",2},
	{"UDINT",4},
	{"REAL",4},
	{"TIME",4},
	{"STRING",0},
	{"WORD",2},
	{"DWORD",4},
	{"UNKNOWN",0},
};

/*
* Connessione al PLC CNI (QPLC) via socket 
*/
int fdPLC;
PLCVAR PLCVars[MAX_PLC_VARS];
char szRcvBuffer[MAX_RCV_SIZE];
int nLastCommand;

char *GetPLCVersion(char *szData)
{
	int nIndex;
	static char szPLCVersion[128];

	/* 
	* mi aspetto la risposta nella forma
	* D <versione>
	*/

	strcpy(szPLCVersion,szData+2);

	/* sostituisco i tab con lo spazio */
	nIndex=0;
	while(szPLCVersion[nIndex]){
		if(szPLCVersion[nIndex]=='\t'){
			szPLCVersion[nIndex]=' ';
		}
		nIndex++;
	}
	return szPLCVersion;
}

BOOL GetPLCStatus(char *szData)
{
	/* 
	* mi aspetto la risposta nella forma
	* D <status>
	*/

	switch(*(szData+2)){
		case '0':	/* PLC in HALT */
			return FALSE;
		break;
		case '1':	/* PLC in GO */
			return TRUE;
		break;
	}
	return FALSE;
}



unsigned int CheckPLCResponse(unsigned int bReceive)
{
	unsigned int bExit=FALSE;

	while(!bExit){
		if(ReceiveData(szRcvBuffer)){
			bExit=TRUE;
		}
	}
	return bExit;
}

		
int OpenSocket(int *fdPort,char *name,int nPortNumber)
{
	struct sockaddr_in sin;
	struct hostent *h;
	long lAddr;


	*fdPort = socket(AF_INET, SOCK_STREAM, 0);
	if (*fdPort < 0) {
		perror("socket");
		return 0;
	}
	/*
	* rendo non bloccante il file descriptor
	fcntl(*fdPort,F_SETFL,O_NONBLOCK);
	*/

	memset((char *)&sin,0,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(nPortNumber);
	lAddr = inet_addr(name);
	if (lAddr == -1) {
		h = gethostbyname(name);
		if (! h) {
			fprintf(stderr,"Unknown host %s\n",name);
			return(0);
		}
		sin.sin_addr.s_addr = *(long *)(h -> h_addr);
	}
	else {
		sin.sin_addr.s_addr = lAddr;
	}
	if (connect(*fdPort,(struct sockaddr *)&sin,sizeof(sin)) < 0) {
		perror("connect");
		return 0;
	}

	return *fdPort;
}

/*
* ReceiveData()
* ricezione dati in coda di attesa dal PLC Server
*/
int ReceiveData(char *szRcvBuf)
{
	int rv;

	rv = recv(fdPLC,szRcvBuf,MAX_RCV_SIZE,0);
	if(rv<0){
#ifdef TRACE
		trace_out_vstr_date(1,"RECV ERROR : [%d] [%s]",rv, strerror(errno));
#endif
	} else if(rv==0){
		/*
		* connection lost (PLC IN CRASH ?!)
		*/
	} else {
		szRcvBuf[rv]='\0';
	}

#ifdef TRACE_PLC
	if(rv){
		trace_out_vstr_date(1,"PLC REC : [%s] [%d]",szRcvBuf, rv);
	}
#endif

	return(rv);
}

unsigned int SendCmd(int nCmd,char *szParams,unsigned int bRecData)
{
	char szSendData[1024];
	int nRC;
	char *pCmdPtr[256];
	char *pAnswer;
	char *pRcvPtr;
	char *szCmdSep="\n";
	int nCmdPtrIndex=0;
	unsigned int bExit=FALSE;
	unsigned int bError=FALSE;

	nLastCommand=nCmd;

	strcpy(szSendData,szCmds[nCmd]);
	
	if(szParams!=(char *)NULL){
		strcat(szSendData," ");
		strcat(szSendData,szParams);
	}
	
#ifdef TRACE_PLC
	trace_out_vstr_date(1,"PLC CMD: [%s]\n",szSendData);
#endif

	strcat(szSendData,"\n");

	/* spedizione messaggio al PLC */
	nRC=send(fdPLC,szSendData,strlen(szSendData),0);

	/* arrivato qualcosa .. controllo  */
	while(!bError && !bExit){
		/* ricezione dati ... */
		if((nRC=ReceiveData(szRcvBuffer))<=0){
			/*
			* Errore di connessione (PLC CRASH ?!)
			*/
			bExit=TRUE;
			continue;
		}
			
		/* costruisco un array di puntatory alle risposte arrivate */
		pRcvPtr=szRcvBuffer;
		while((pAnswer=strtok(pRcvPtr,szCmdSep))!=NULL){
			pCmdPtr[nCmdPtrIndex]=pAnswer;
			
			nCmdPtrIndex++;
			pRcvPtr=NULL;
		}
		pCmdPtr[nCmdPtrIndex]=NULL;

		nCmdPtrIndex=0;
		while(pCmdPtr[nCmdPtrIndex]){
			pRcvPtr=pCmdPtr[nCmdPtrIndex];

			if(pRcvPtr[0]=='E'){
				/* 
				* messaggio di errore  ...
				*/
#ifdef TRACE
				szSendData[strlen(szSendData)-1]='\0';
				trace_out_vstr_date(1,"PLC ERROR : CMD:[%s] [%s]",szSendData,pRcvPtr);
#endif
				/* esco dal ciclo while di attesa risposta ... */
				bError=TRUE;
			} else if(pRcvPtr[0]=='A'){
				/* 
				* messaggio asincrono ...
				* lo gestisco dopo ....
				*/
				
#ifdef TRACE
				trace_out_vstr_date(1,"PLC ASYNC : [%s]",pRcvPtr);
#endif
			} else {
				/* esco dal ciclo while di attesa risposta ... */
				bExit=TRUE;
			}
			nCmdPtrIndex++;
		}
	}
	if(nRC==0){
#ifdef TRACE
		trace_out_vstr_date(1,"PLC CONNECTION ERROR");
#endif
	}
	return(bError?0:nRC);
}

int GetVarType(unsigned long ulFlags)
{
	ulFlags&=SHV_MODE_TYPE;

	switch(ulFlags){
		case SHV_MODE_TYPE_BOOL:   return TYPE_BOOL; break;
		case SHV_MODE_TYPE_CHAR:   return TYPE_BYTE; break;
		case SHV_MODE_TYPE_UCHAR:  return TYPE_BYTE; break;
		case SHV_MODE_TYPE_LONG:   return TYPE_DINT; break;
		case SHV_MODE_TYPE_ULONG:  return TYPE_UDINT; break;
		case SHV_MODE_TYPE_FLOAT:  return TYPE_REAL; break;
		case SHV_MODE_TYPE_DOUBLE: return TYPE_REAL; break; // LREAL ?
		default:	return TYPE_UNKNOWN; break;
	}
}

/*
* GetVariableInfo
* Lettura dei dati reltivi ad una variabile PLC
*/
unsigned int GetVariableInfo(char *szData,PLCVAR *pPLCVar)
{
	int nPLCType;
	char *szSep=" ";
	char *pPtr;

	
	/* 
	* Risposta nella forma 
	*
	*	D I <address> <file> : simbolo ordinario
	*		indirizzo : indirizzo della variabile
	*       file      : file contenente la definizione del nome
	*
	*	D S <address> <dim1> <dim2> <flags> <size> <key> : segnale di scambio
	*		indirizzo : indirizzo della variabile
	*		dim1,dim2 : dimensioni del segnale di scambio (1 se scalare)
	*		flags     : caratteristiche del segnale di scambio (bitmask)
	*       size      : dimensioni (in byte) di un elemento del segnale di scambio
	*		key       : chiave dell'elemento selezionato
	*/ 

	
	if((pPtr=strtok(szData,szSep))!=(char *)NULL){ /* Risposta (D o E) */
		if(*pPtr=='E'){
			return FALSE;
		}
	}

	if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* Tipo della variabile */
		switch(*pPtr){
			case 'I':	/* simbolo ordinario */
				pPLCVar->usSymbolType=VAR_NORMAL;
			break;
			case 'S':	/* segnale di scambio */
				pPLCVar->usSymbolType=VAR_SIGNAL;
			break;
		}
	}
	if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* Indirizzo (in esadecimale) */
		sscanf(pPtr,"%lx",&(pPLCVar->ulAddress));
	}

	switch(pPLCVar->usSymbolType){
		case VAR_NORMAL:
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ /* file */
				strcpy(pPLCVar->szFile,pPtr);
			}
			/*
			* VARIABILE NORMALE
			* assegno il tipo del PLC e il modo di visualizzazione appropriato
			*/
			pPLCVar->usType=TYPE_BYTE;
		break;
		case VAR_SIGNAL:
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // dim1
				sscanf(pPtr,"%lx",&(pPLCVar->ulDim1));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // dim2
				sscanf(pPtr,"%lx",&(pPLCVar->ulDim2));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // flags
				sscanf(pPtr,"%lx",&(pPLCVar->ulFlags));
				/* 
				* leggo il tipo variabile dal PLC 
				*/
				nPLCType=GetVarType(pPLCVar->ulFlags);
			}	

			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // size
				sscanf(pPtr,"%lx",&(pPLCVar->ulSize));
			}
			if((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // key
				sscanf(pPtr,"%lx",&(pPLCVar->ulKey));
			}
			/*
			* VARIABILE SEGNALE DI SCAMBIO
			* assegno il tipo del PLC e il modo di visualizzazione appropriato
			*/
			pPLCVar->usType=nPLCType;
		break;
	}
	return TRUE;
}

void GetMemChkInfo(PLCVAR *PLCVars,char *szData)
{
	int nItemIndex=0;
	char *szSep=" ";
	char *pPtr;
	
	/* 
	* Risposta nella forma 
	*
	*	D <flag> [<flag> ...]
	*		flag : validitá (1) / non validità (0) dell'area richiesta dalla memchk
	*/ 
	if((pPtr=strtok(szData,szSep))!=(char *)NULL){ // Risposta (D)
	}
	while((pPtr=strtok(NULL,szSep))!=(char *)NULL){ // Flag 0,1,...,n esimo
		PLCVars[nItemIndex].bEnabled=(atoi(pPtr)==0?FALSE:TRUE);
		nItemIndex++;
	}
}

void SendVarListMemChk(PLCVAR *PLCVars)
{
	int nIndex=0;
	char szParams[512];
	char szVarData[128];
	int nDataLength;
	/* 
	* Inizializzo i parametri del comando memchk 
	*/
	szParams[0]='\0';

	while(strlen(PLCVars[nIndex].szName)){
		if(PLCVars[nIndex].bEnabled){
			nDataLength=PLCTypes[PLCVars[nIndex].usType].nLen;

			sprintf(szVarData,"%lx %x ",PLCVars[nIndex].ulAddress,nDataLength);
			strcat(szParams,szVarData);
		}

		nIndex++;		
	}
	if(strlen(szParams)){
		if(SendCmd(CMD_MEMCHK,szParams,TRUE)){
			GetMemChkInfo(PLCVars,szRcvBuffer);
		}
	}
}

/*
* Leggo i dati di tutti isegnali di scambio interessati
*/
void GetVarsInfo(PLCVAR *PLCVars)
{
	char szParams[128];
	int nIndex=0;
	PLCVAR *pPLCVar;

	while(strlen(PLCVars[nIndex].szName)){
		pPLCVar=&PLCVars[nIndex];
		sprintf(szParams,"%s %d %d",pPLCVar->szName,pPLCVar->nIndex1,pPLCVar->nIndex2);

		/* 
		* richiesta informazioni al PLC
		*/
		if(SendCmd(CMD_VAR,(char *)szParams,TRUE)){
			GetVariableInfo(szRcvBuffer,pPLCVar);
			pPLCVar->bEnabled=1;
		} else {
			pPLCVar->bEnabled=0;
		}
		nIndex++;
	}
}

/*
* GetDebugValue()
*/
int GetDebugValue(PLCVAR *pPV,char *pszBuffer,int nOffset)
{
	int nIndex;
	unsigned int tmp;
	char szByte[3];

	szByte[2]='\0';
	
	for(nIndex=0;nIndex<PLCTypes[pPV->usType].nLen;nIndex++){
		szByte[0]=pszBuffer[nOffset];
		szByte[1]=pszBuffer[nOffset+1];
		sscanf(szByte,"%x",&tmp);
		pPV->Value.Bytes[nIndex]=(unsigned char)tmp;
		nOffset+=2;
	}
	return PLCTypes[pPV->usType].nLen;
}

/*
* GetMemCopyInfo(char *szData)
* lettura dei valori delle variabili registrate
*/
void GetMemCopyInfo(char *szData)
{
	int nItemIndex=0;
	int nByteOffset=0;
	int nBytes;
	
	/* 
	* Risposta nella forma 
	*
	*	D <flag> [<flag> ...]
	*		flag : validitá (1) / non validità (0) dell'area richiesta dalla memchk
	*/ 
	while(strlen(PLCVars[nItemIndex].szName)){
		if(PLCVars[nItemIndex].bEnabled){
			nBytes=GetDebugValue(&PLCVars[nItemIndex],szData+2,nByteOffset);
			
			nByteOffset+=nBytes*2;
		}
		nItemIndex++;
	}
}


/*
* scrittura variabili PLC utilizzate ...
*/
int WriteVarList(char *szCfgFile)
{
	int   nIndex;
	char  szBuffer[128];
	char  szField[80];
	char  szParagraph[80];


	strcpy(szParagraph,"Variables");

	/*
	* Cancellazione del paragrafo attuale
	*/
	DelFileParagraph(szParagraph, szCfgFile, NULL);

	nIndex=0;
	while(strlen(PLCVars[nIndex].szName)){
		sprintf(szField,"VAR_%02d",nIndex);
		sprintf(szBuffer,"%s,%s",PLCVars[nIndex].szName,PLCVars[nIndex].szDescription);

		PutFileString(szParagraph,szField,szBuffer,szCfgFile,NULL);
		nIndex++;
	}

	return nIndex;
}

/*
* lettura variabili PLC utilizzate ...
*/
int ReadVarList(char *szCfgFile)
{
	int   nIndex;
	char  szBuffer[128];
	char  szField[80];
	char  szParagraph[80];
	char  *pPtr;
	char  *szSeparator=",\t\n";

	/*
	* lettura del file di configurazione
	*/
	strcpy(szParagraph,"Variables");

	nIndex=0;
	sprintf(szField,"VAR_%02d",nIndex);
	while(GetFileString(szParagraph,szField,"",szBuffer,sizeof(szBuffer),szCfgFile,NULL)){
		if(strlen(szBuffer)){
			/* Nome Variabile */
			if((pPtr=strtok(szBuffer,szSeparator))!=NULL){
				strcpy(PLCVars[nIndex].szName,pPtr);
			} else {return(-1);}
			/* Titolo Variabile */
			if((pPtr=strtok(NULL,szSeparator))!=NULL){
				strcpy(PLCVars[nIndex].szDescription,pPtr);
			} else {return(-1);}
		}
		nIndex++;
		sprintf(szField,"VAR_%02d",nIndex);
	}
	PLCVars[nIndex].szName[0]='\0';

	return nIndex;
}

BOOL UpFront(int nVarIndex)
{
	return (VarValueChanged(nVarIndex) && !VarOldValue(nVarIndex));
}

BOOL DownFront(int nVarIndex)
{
	return (VarValueChanged(nVarIndex) && !VarValue(nVarIndex));
}

BOOL SetVarValue(int  nVarIndex, int nValue)
{
	char szBuffer[128];

	sprintf(szBuffer,"%s 0 0 %d", PLCVars[nVarIndex].szName,nValue);
	return SendCmd(CMD_SET,szBuffer,TRUE);
}

unsigned long VarValue(int  nVarIndex)
{
	return PLCVars[nVarIndex].Value.DWORDValue;
}

unsigned long VarOldValue(int  nVarIndex)
{
	return PLCVars[nVarIndex].OldValue.DWORDValue;
}

BOOL VarValueChanged(int  nVarIndex)
{
	if(memcmp(&PLCVars[nVarIndex].OldValue,&PLCVars[nVarIndex].Value,sizeof(VAR_VALUE))){
		return TRUE;
	}
	return FALSE;
}

int GetVarIndex(char *szVarName)
{
	int nIndex;

	nIndex=0;
	while(strlen(PLCVars[nIndex].szName)){
		if(!strcmp(szVarName,PLCVars[nIndex].szName)){
			/*
			* Trovata la Variabile
			*/
			return nIndex;
		}
		nIndex++;
	}
	return -1;
}

BOOL PLCConnect(char *szHostName)
{
	if((OpenSocket(&fdPLC,szHostName,PLC_SERVICE_PORT))==0){
		return FALSE;
	}

	CheckPLCResponse(TRUE);

	/* versione del PLC */
	if(SendCmd(CMD_VER,(char *)NULL,TRUE)){	
		GetPLCVersion(szRcvBuffer);
	}

	/* status del PLC */
	if(SendCmd(CMD_STATUS,(char *)NULL,TRUE)){
		GetPLCStatus(szRcvBuffer);
	}

	/*
	* Lettura dati variabili
	*/
	GetVarsInfo(PLCVars);

	/*
	* Registrazione delle variabili
	*/
	SendVarListMemChk(PLCVars);

	return TRUE;
}


/*
* text_read_file(char *szFileName)
* legge il file szfilename e lo inserisce nella gtk_text specificata in input
*/
int text_read_file(char *szFileName,char *szBuffer)
{
	int fd, num_bytes;
	char *buf;
	struct stat file_status;

	if ((fd = open(szFileName, O_RDONLY)) <= 0)
		return -1;
	else {
		if (fstat(fd, &file_status) == -1)
			return -2;
		if ((buf = malloc(file_status.st_size)) == (char *)NULL)
			return -3;
		if ((num_bytes = read(fd, buf, file_status.st_size)) <= 0)
			return -4;
	}
	buf[file_status.st_size]='\0';
	close(fd);
	strcpy(szBuffer,buf);
	free(buf);

	return file_status.st_size;
}

int GetMaxLineWidth(char *szFile)
{
	FILE *fp;
	char szBuffer[1024];
	int nMaxWidth=0;

	if ((fp=fopen(szFile,"r"))!=(FILE *)NULL) {
		while( fgets(szBuffer, sizeof(szBuffer), fp)!=NULL ){
			nMaxWidth=max(nMaxWidth,strlen(szBuffer));
		}
		fclose(fp);
	}

	return nMaxWidth;
}

/* 
* Stampa un file
* defaults: stampa ascii
*/
BOOL PrintFile(char *szFile,char *szPrinter,char *szTitle,int nPrintType,BOOL bDelete)
{
	char szCommand[2048];
	char szBuffer[128];
	BOOL bHtml2PS=FALSE;
	BOOL bA2PS=FALSE;
	BOOL bAscii=FALSE;
	char szHtml2PSCfgFileName[128];
	BOOL bOK=TRUE;

	switch(nPrintType){
		case PRINT_A2PS:
			bA2PS=TRUE;
		break;
		case PRINT_HTML2PS:
			bHtml2PS=TRUE;
		break;
		default:
		case PRINT_ASCII:
			bAscii=TRUE;
		break;
	}

	if(bHtml2PS){
		if(szHtml2PSCfgFileName){
			/* rm : da sistemare ... */
			strcpy(szHtml2PSCfgFileName,"/etc/html2ps.cfg");
			sprintf(szCommand,"html2ps -f %s -o %s.ps %s",szHtml2PSCfgFileName,szFile,szFile);
		} else {
			sprintf(szCommand,"html2ps -o %s.ps %s",szFile,szFile);
		}
		strcat(szCommand," | lpr ");
		/* stampante specificata */
		if(szPrinter && strlen(szPrinter)){
			sprintf(szBuffer," -P%s",szPrinter);
			strcat(szCommand,szBuffer);
		}
	} else if(bA2PS){
		strcpy(szCommand,"a2ps -R -q --medium=A4  --columns=1");
		/* titolo */
		if(szTitle && strlen(szTitle)){
			sprintf(szBuffer," --center-title=\"%s\"",szTitle);
			strcat(szCommand,szBuffer);
		}
		sprintf(szBuffer," -l %d", GetMaxLineWidth(szFile));
		strcat(szCommand,szBuffer);

		strcat(szCommand," ");
		strcat(szCommand,szFile);

		/* stampante specificata */
		if(szPrinter && strlen(szPrinter)){
			sprintf(szBuffer," -P%s",szPrinter);
			strcat(szCommand,szBuffer);
		}
	} else if(bAscii) {
		sprintf(szCommand,"cat %s",szFile);
		strcat(szCommand," | lpr ");
		/* stampante specificata */
		if(szPrinter && strlen(szPrinter)){
			sprintf(szBuffer," -P%s",szPrinter);
			strcat(szCommand,szBuffer);
		}
	} else {
		return FALSE;
	}
	

#ifdef TRACE
	trace_out_vstr_date(1,"PrintFile : %s",szCommand);
#endif
	if(system(szCommand)==-1){
		bOK=FALSE;
	}
	if(bDelete){
		unlink(szFile);
	}

	return bOK;
}

