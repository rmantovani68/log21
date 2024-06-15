/*
* mainutil.c
* utility functions
* Easy Picking 3.0
* Copyright CNI srl 1999-2000
* Autore : Roberto Mantovani
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifdef TRACE
	#include <trace.h>
#endif

#include <gnome.h>
#include <gtk/gtk.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <libpq-fe.h>

#include "interface.h"
#include "support.h"

#include <proc_list.h>
#include <ca_file.h>
#include <pmx_msq.h>
#include <dbfun.h>

#include "picking.h"
#include "mainstruct.h"
#include "mainext.h"
#include "mainfun.h"
#include "callbacks.h"

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
	if(strlen(szFromTime)){
		strcpy(szTimeString,szFromTime);			/* ora */
		szTimeString[2]='\0';
		strcat(szTimeString,":");
		strcat(szTimeString,szFromTime+2);	/* minuti */
		szTimeString[5]='\0';
		strcat(szTimeString,":");
		strcat(szTimeString,szFromTime+4);	/* secondi */
		szTimeString[8]='\0';
	} else {
		szTimeString[0]='\0';
	}

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
		strcpy(szDateString,szFromDate+6);	/* giorno */
		strcat(szDateString,"-");
		strcat(szDateString,szFromDate+4);	/* mese */
		szDateString[5]='\0';
		strcat(szDateString,"-");
		strcat(szDateString,szFromDate+2);	/* anno */
		szDateString[8]='\0';
	} else {
		szDateString[0]='\0';
	}

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
		szDateString[0]='\0';
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
* funzione di stampa formattata in un gtk label widget
*/
void gtk_label_printf(GtkWidget *w,char *fmt,...)
{
	va_list args;
	char msgbuf[1024];

	va_start(args,fmt);
	(void)vsprintf(msgbuf,fmt,args);
	va_end(args);

	gtk_label_set_text (GTK_LABEL(w), g_utf8_normalize(msgbuf,-1,G_NORMALIZE_DEFAULT));
}

/*
* funzione di stampa formattata in un gtk text widget
*/
void gtk_text_printf(char *szColor,GtkWidget *w,char *fmt,...)
{
	va_list args;
	char msgbuf[1024];
	GtkTextBuffer *buffer;
	GtkTextTag *tag;
	char szTagName[128];
	GtkTextIter iter; 
	BOOL bOK=FALSE;


	va_start(args,fmt);
	(void)vsprintf(msgbuf,fmt,args);
	va_end(args);
	/*
	gtk_text_insert (GTK_TEXT(w), NULL, Color, NULL, msgbuf, -1);
	gtk_editable_insert_text (GTK_EDITABLE(w), msgbuf, strlen(msgbuf),&nPos);
	if(NULL!=Color){
		gtk_widget_modify_text (w, GTK_STATE_NORMAL, Color);
	}
	*/

	/* ricavo il buffer */
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (w));

	if(szColor && strlen(szColor)){
		sprintf(szTagName,"%s_fg",szColor);
		tag = gtk_text_buffer_create_tag (buffer, NULL, "foreground", szColor, NULL);  
		if(tag){
			gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_mark (buffer, "insert"));
			gtk_text_buffer_insert_with_tags (buffer, &iter, msgbuf, -1, tag,NULL);
			bOK=TRUE;
		}
	}
	/*
	gtk_text_buffer_set_text (buffer, msgbuf, -1);
	*/
	if(!bOK){
		gtk_text_buffer_insert_at_cursor (buffer, g_utf8_normalize(msgbuf,-1,G_NORMALIZE_DEFAULT), -1);
	}

	gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_mark (buffer, "insert"));
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (w), &iter, 0.0, FALSE, 0.0, 0.0);
}

/*
* gtk_text_read_file(w,szFileName)
* legge il file szfilename e lo inserisce nella gtk_text specificata in input
*/
void gtk_text_read_file(GtkWidget *w,char *szFileName)
{
	int fd, num_bytes;
	char *buf;
	struct stat file_status;

	if ((fd = open(szFileName, O_RDONLY)) <= 0)
		return;
	else {
		if (fstat(fd, &file_status) == -1)
			return;
		if ((buf = malloc(file_status.st_size)) == (char *)NULL)
			return;
		if ((num_bytes = read(fd, buf, file_status.st_size)) <= 0)
			return;
	}
	buf[file_status.st_size]='\0';
	close(fd);
	/* gtk_text_insert (GTK_TEXT(w), NULL, NULL, NULL, buf, -1); */
	free(buf);
}



/*
* void gtk_update(void)
*/
void gtk_update(void)
{
  while (gtk_events_pending())
    gtk_main_iteration();
}

void gtk_widget_set_background(GtkWidget *w,GdkColor *Color)
{
	GtkStyle *style;

	if(w==(GtkWidget *)NULL){
		return;
	}

	/* set foreground and text to color */
	style = gtk_style_copy(gtk_widget_get_style(w));
	gtk_style_ref(style);
	style->bg[GTK_STATE_NORMAL] = *Color;
	gtk_widget_set_style(w, style);
	gtk_style_unref(style);
}


void gtk_widget_set_foreground(GtkWidget *w,GdkColor *Color)
{
	GtkStyle *style;

	if(w==(GtkWidget *)NULL){
		return;
	}

	/* set foreground and text to color */
	style = gtk_style_copy(gtk_widget_get_style(w));
	gtk_style_ref(style);
	style->text[GTK_STATE_NORMAL] = *Color;
	style->fg[GTK_STATE_NORMAL] = *Color;
	gtk_widget_set_style(w, style);
	gtk_style_unref(style);
}

unsigned long CryptString(char *pszString)
{
	int   nLen=strlen(pszString);
	int   nIndex;
	unsigned long ulNum=0;


	for( nIndex=0; *pszString!='\0'; nIndex++ ){
		ulNum += (*pszString*(*pszString+nLen)*(nIndex+1));
		++pszString;
	}

	return(ulNum);
} /*  Fine CryptString */

int CreateTable(GtkWidget *parent,char *szScrolledWindowName,char *szCListName,char *szLabelTitle,char *szLabelDescr,char *szLabelRows,char *szParagraph,char *szTableName,int nSelectionMode,char *szKey)
{
	char szSelectCmd[2048];
	char szCmd[2048];
	char szBuffer[256];
	char szTitle[256];
	char *pPtr;
	GtkWidget *lst;
	GtkWidget *sw=get_widget(parent,szScrolledWindowName);
	int nTuples=0;

	if(szParagraph && strlen(szParagraph) && strlen(szTableName)){

		xncGetFileString(szParagraph,szTableName, "", szSelectCmd, sizeof(szSelectCmd),Cfg.szCfgFile,NULL); 

		strcpy(szCmd,szSelectCmd);
		/*
		* Sostituisco i valori parametrici
		* %KEY%       : Chiave
		* %TIPOUBI%   : Tipo Ubicazioni
		* %ORDTIPO%   : Tipo Ordini
		*/
		if(szKey && (pPtr=strstr(szSelectCmd,"%KEY%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,szKey);
			strcat(szCmd,pPtr+strlen("%KEY%"));
		}
		strcpy(szSelectCmd,szCmd);

		if(szKey && (pPtr=strstr(szSelectCmd,"%TIPOUBI%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoUbicazioni);
			strcat(szCmd,pPtr+strlen("%TIPOUBI%"));
		}
		strcpy(szSelectCmd,szCmd);

		if((pPtr=strstr(szSelectCmd,"%ORDTIPO%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoOrdini);
			strcat(szCmd,pPtr+strlen("%ORDTIPO%"));
		}
		strcpy(szSelectCmd,szCmd);


		/*
		* Creazione lista
		*/
		if((lst=get_widget(sw,szCListName))){
			gtk_signal_disconnect_by_func (GTK_OBJECT (lst), GTK_SIGNAL_FUNC (on_clist_select_row),   (gpointer)NULL);
			gtk_signal_disconnect_by_func (GTK_OBJECT (lst), GTK_SIGNAL_FUNC (on_clist_unselect_row), (gpointer)NULL);
			gtk_widget_destroy(lst);
		}

		nTuples=CreateListFromSelect(parent,sw,&lst,szCListName,nSelectionMode,szSelectCmd);

		gtk_signal_connect (GTK_OBJECT (lst), "select-row", GTK_SIGNAL_FUNC (on_clist_select_row),     (gpointer)NULL);
		gtk_signal_connect (GTK_OBJECT (lst), "unselect-row", GTK_SIGNAL_FUNC (on_clist_unselect_row), (gpointer)NULL);


		/* descrizione */
		if(szLabelDescr && strlen(szLabelDescr) && get_widget(parent,szLabelDescr)){
			strcpy(szBuffer,szTableName);
			strcat(szBuffer,"_descr");
			xncGetFileString(szParagraph,szBuffer, "", szTitle, sizeof(szTitle),Cfg.szCfgFile,NULL); 
			gtk_label_printf(get_widget(parent,szLabelDescr),szTitle);
		}
		/* nome relazione */
		if(szLabelTitle && strlen(szLabelTitle) && get_widget(parent,szLabelTitle)){
			gtk_label_printf(get_widget(parent,szLabelTitle),szTableName);
		}
		/* numero di records */
		if(szLabelRows && strlen(szLabelRows) && get_widget(parent,szLabelRows)){
			gtk_label_printf(get_widget(parent,szLabelRows),"%d",nTuples);
		}

	}
	return nTuples;
}


int RefreshTable(GtkWidget *parent,char *szScrolledWindowName,char *szCListName,char *szLabelTitle,char *szLabelDescr,char *szLabelRows,char *szParagraph,char *szTableName,int nSelectionMode,char *szKey)
{
	char szSelectCmd[2048];
	char szCmd[2048];
	char szBuffer[256];
	char szTitle[256];
	char *pPtr;
	GtkWidget *lst;
	GtkWidget *sw=get_widget(parent,szScrolledWindowName);
	int nTuples=0;

	if(szParagraph && strlen(szParagraph) && strlen(szTableName)){

		xncGetFileString(szParagraph,szTableName, "", szSelectCmd, sizeof(szSelectCmd),Cfg.szCfgFile,NULL); 

		strcpy(szCmd,szSelectCmd);
		/*
		* Sostituisco i valori parametrici
		* %KEY%       : Chiave
		* %TIPOUBI%   : Tipo Ubicazioni
		* %ORDTIPO%   : Tipo Ordini
		*/
		if(szKey && strlen(szKey) && (pPtr=strstr(szSelectCmd,"%KEY%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,szKey);
			strcat(szCmd,pPtr+strlen("%KEY%"));
		}
		strcpy(szSelectCmd,szCmd);

		if(szKey && strlen(szKey) && (pPtr=strstr(szSelectCmd,"%TIPOUBI%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoUbicazioni);
			strcat(szCmd,pPtr+strlen("%TIPOUBI%"));
		}
		strcpy(szSelectCmd,szCmd);

		if((pPtr=strstr(szSelectCmd,"%ORDTIPO%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoOrdini);
			strcat(szCmd,pPtr+strlen("%ORDTIPO%"));
		}
		strcpy(szSelectCmd,szCmd);

		/*
		* refresh lista a video
		*/
#ifdef ELIMINATO
		if((lst=get_widget(sw,szCListName))){
			gtk_signal_disconnect_by_func (GTK_OBJECT (lst), GTK_SIGNAL_FUNC (on_clist_select_row),   (gpointer)NULL);
			gtk_signal_disconnect_by_func (GTK_OBJECT (lst), GTK_SIGNAL_FUNC (on_clist_unselect_row), (gpointer)NULL);
			gtk_widget_destroy(lst);
		}

		nTuples=CreateListFromSelect(parent,sw,&lst,szCListName,nSelectionMode,szSelectCmd);
#endif
		if((lst=get_widget(sw,szCListName))){
			gtk_clist_clear(GTK_CLIST(lst));
			nTuples=RefreshListFromSelect(parent,sw,lst,szCListName,nSelectionMode,szSelectCmd);
		} else {
			nTuples=CreateListFromSelect(parent,sw,&lst,szCListName,nSelectionMode,szSelectCmd);
		}

		/* descrizione */
		if(szLabelDescr && strlen(szLabelDescr) && get_widget(parent,szLabelDescr)){
			strcpy(szBuffer,szTableName);
			strcat(szBuffer,"_descr");
			xncGetFileString(szParagraph,szBuffer, "", szTitle, sizeof(szTitle),Cfg.szCfgFile,NULL); 
			gtk_label_printf(get_widget(parent,szLabelDescr),szTitle);
		}
		/* nome relazione */
		if(szLabelTitle && strlen(szLabelTitle) && get_widget(parent,szLabelTitle)){
			gtk_label_printf(get_widget(parent,szLabelTitle),szTableName);
		}
		/* numero di records */
		if(szLabelRows && strlen(szLabelRows) && get_widget(parent,szLabelRows)){
			gtk_label_printf(get_widget(parent,szLabelRows),"%d",nTuples);
		}

		gtk_signal_connect (GTK_OBJECT (lst), "select-row", GTK_SIGNAL_FUNC (on_clist_select_row),     (gpointer)NULL);
		gtk_signal_connect (GTK_OBJECT (lst), "unselect-row", GTK_SIGNAL_FUNC (on_clist_unselect_row), (gpointer)NULL);
	}
	return nTuples;
}

/*
* RefreshListFromSelect()
* Aggiorna una lista GTK da una select SQL
* ritorna il numero di tuple o -1 se errore
*/
int RefreshListFromSelect(GtkWidget *super_parent, GtkWidget *parent, GtkWidget *clist, char *szCListName, int nSelectMode, char *szSqlCmd)
{
	int nTuples;
	int nFields;
	int nIndex;
	int nColumnIndex;
	int nColumnWidth;
	char szColumnName[128];
	int nFieldIndex;
	int nType;
	char *RecordData[1][MAX_FIELDS_NUMBER];
	PGresult *PGRes = NULL;
	
	gtk_clist_freeze(GTK_CLIST(clist));
	/*
	* Eseguo la select specificata
	*/
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSqlCmd);

	nTuples=PQntuples(PGRes);
	nFields=PQnfields(PGRes);

	if(nTuples){
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* Alloco lo spazio per una tupla
			*/
			RecordData[0][nFieldIndex]=malloc(100);
		}
		/*
		* Aggiungo le tuple alla lista (una alla volta)
		*/
		for(nIndex=0;nIndex<nTuples;nIndex++){

			for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				int nLength;
				/*
				* Setto il testo del campo
				*/
				nType=PQftype(PGRes,nFieldIndex);
				switch(nType){
					case 1042:
						/* si tratta di una stringa */
						nLength=PQgetlength(PGRes,0,nFieldIndex);
						sprintf(RecordData[0][nFieldIndex],"%s",PQgetvalue(PGRes,nIndex,nFieldIndex));
					break;
					case 701:
						/* si tratta di un float */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%.*f",3,atof(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						if(!nIndex){
							gtk_clist_set_column_justification(GTK_CLIST(clist),nFieldIndex,GTK_JUSTIFY_RIGHT);
						}
					break;
					case 20:
					case 21:
					case 23:
					case 1700:
						/* si tratta di un int */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%d",atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						if(!nIndex){
							gtk_clist_set_column_justification(GTK_CLIST(clist),nFieldIndex,GTK_JUSTIFY_RIGHT);
						}
					break;
					default:
						/* altro... lo tratto come una stringa */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%s",PQgetvalue(PGRes,nIndex,nFieldIndex));
					break;
				}
			}

			gtk_clist_append( GTK_CLIST(clist), RecordData[0]);
		}

		/* 
		* libero la memoria allocata 
		*/
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			free(RecordData[0][nFieldIndex]);
		}
	}
	PQclear(PGRes);
	
	/*
	* + st + 04/10/2000
	* Aggiusto la larghezza delle colonne in modo da poter visualizzare completamente i dati contenuti;
	* Per le colonne vuote imposto una larghezza minima che permette di visualizzare completamente il titolo;
	*/
	for(nColumnIndex=0;nColumnIndex<nFields;nColumnIndex++){
		sprintf(szColumnName, gtk_clist_get_column_title(GTK_CLIST(clist), nColumnIndex));
		nColumnWidth=Cfg.nCharWidth*strlen(StrTrimAll(szColumnName));
		gtk_clist_set_column_min_width(GTK_CLIST(clist), nColumnIndex,	nColumnWidth);
	}
	gtk_clist_columns_autosize(GTK_CLIST(clist));
	
	gtk_clist_thaw(GTK_CLIST(clist));

	return nTuples;
}


/*
* CreateListFromSelect()
* Crea una lista GTK da una select SQL
* ritorna il numero di tuple o -1 se errore
*/
int CreateListFromSelect(GtkWidget *super_parent, GtkWidget *parent, GtkWidget **clist, char *szCListName, int nSelectMode, char *szSqlCmd)
{
	int nTuples;
	int nFields;
	int nIndex;
	int nColumnIndex;
	int nColumnWidth;
	char szColumnName[128];
	int nFieldIndex;
	int nType;
	char *RecordData[1][MAX_FIELDS_NUMBER];
	char szCmd[4096];
	PGresult *PGRes = NULL;
	GtkWidget *ab_main;

	ab_main=get_widget(super_parent,"ab_main");
	
	strcpy(szCmd,szSqlCmd);

	/*
	* Eseguo la select specificata
	*/
	PGRes=PGExecSQL(Cfg.nDebugVersion>1,szSqlCmd);

	nTuples=PQntuples(PGRes);
	nFields=PQnfields(PGRes);

	if(nTuples>=0){
		/*
		* Creazione clist widget 
		*/
		*clist = gtk_clist_new (nFields);
		gtk_widget_ref (*clist);
		gtk_object_set_data_full (GTK_OBJECT (super_parent), szCListName, *clist, (GtkDestroyNotify) gtk_widget_unref);
		gtk_container_add (GTK_CONTAINER (parent), *clist);

		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* impostazione titolo colonna : utilizzo la .._set_column_title per avere la possibilita'
			* di ottenere la larghezza ottimale in fase di visualizzazione
			*/ 
			gtk_clist_set_column_title(GTK_CLIST(*clist), nFieldIndex, PQfname(PGRes,nFieldIndex));
		}
		gtk_clist_column_titles_show (GTK_CLIST (*clist));
		gtk_clist_set_selection_mode (GTK_CLIST (*clist),nSelectMode);
	}
	if(nTuples){
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* Alloco lo spazio per una tupla
			*/
			RecordData[0][nFieldIndex]=malloc(100);
		}
		/*
		* Aggiungo le tuple alla lista (una alla volta)
		*/
		gtk_clist_freeze(GTK_CLIST(*clist));
		for(nIndex=0;nIndex<nTuples;nIndex++){

			for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				int nLength;
				/*
				* Setto il testo del campo
				*/
				nType=PQftype(PGRes,nFieldIndex);
				switch(nType){
					case 1042:
						/* si tratta di una stringa */
						nLength=PQgetlength(PGRes,0,nFieldIndex);
						//sprintf(RecordData[0][nFieldIndex],"%*s",nLength,PQgetvalue(PGRes,nIndex,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%s",PQgetvalue(PGRes,nIndex,nFieldIndex));
					break;
					case 701:
						/* si tratta di un float */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%.*f",3,atof(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						//sprintf(RecordData[0][nFieldIndex],"%f",atof(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						if(!nIndex){
							gtk_clist_set_column_justification(GTK_CLIST(*clist),nFieldIndex,GTK_JUSTIFY_RIGHT);
						}
					break;
					case 20:
					case 21:
					case 23:
					case 1700:
						/* si tratta di un int */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						// sprintf(RecordData[0][nFieldIndex],"%*d",nLength,atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						// sprintf(RecordData[0][nFieldIndex],"%d",atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						sprintf(RecordData[0][nFieldIndex],"%*d",nLength,atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
						if(!nIndex){
							gtk_clist_set_column_justification(GTK_CLIST(*clist),nFieldIndex,GTK_JUSTIFY_RIGHT);
						}
					break;
					default:
						/* altro... lo tratto come una stringa */
						nLength=strlen(PQfname(PGRes,nFieldIndex));
						//sprintf(RecordData[0][nFieldIndex],"%*s",nLength,PQgetvalue(PGRes,nIndex,nFieldIndex));
						sprintf(RecordData[0][nFieldIndex],"%s",PQgetvalue(PGRes,nIndex,nFieldIndex));
					break;
				}
			}

			gtk_clist_append( (GtkCList *) *clist, RecordData[0]);
			if(ab_main && (nIndex%(max(1,nTuples/100)))==0) {
//			if(ab_main && (nIndex%10)==0) {
				gnome_appbar_set_progress_percentage ((GnomeAppBar *)ab_main, (float)nIndex/(float)nTuples);
				gtk_update();
			}
		}

		/* 
		* libero la memoria allocata 
		*/
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			free(RecordData[0][nFieldIndex]);
		}
	}
	gtk_clist_thaw(GTK_CLIST(*clist));
	PQclear(PGRes);


	/*
	* + st + 04/10/2000
	* Aggiusto la larghezza delle colonne in modo da poter visualizzare completamente i dati contenuti;
	* Per le colonne vuote imposto una larghezza minima che permette di visualizzare completamente il titolo;
	*/
	for(nColumnIndex=0;nColumnIndex<nFields;nColumnIndex++){
		sprintf(szColumnName, gtk_clist_get_column_title(GTK_CLIST(*clist), nColumnIndex));
		nColumnWidth=Cfg.nCharWidth*strlen(StrTrimAll(szColumnName));
		gtk_clist_set_column_min_width(GTK_CLIST(*clist), nColumnIndex,	nColumnWidth);
		/* 
		gtk_clist_set_column_width(GTK_CLIST(*clist), nColumnIndex, gtk_clist_optimal_column_width(GTK_CLIST(*clist),nColumnIndex));
		*/
	}
	gtk_clist_columns_autosize(GTK_CLIST(*clist));
	/*
	* callback per ordinamento clist
	*/
	gtk_signal_connect (GTK_OBJECT (*clist), "click-column", GTK_SIGNAL_FUNC (on_lst_sort_column_clicked), NULL);
	
	if(nTuples>=0){
		gtk_widget_show (*clist);
	}

	if(ab_main) {
		gnome_appbar_set_progress_percentage ((GnomeAppBar *)ab_main, (float)0);
		//gtk_update();
	}
	return nTuples;
}

void trace_debug(GdkColor *Color,...)
{
	va_list args;
	char szMessage[1024];
	char *fmt;

	va_start(args,Color);
	fmt=va_arg(args,char *);
	(void)vsprintf(szMessage,fmt,args);
	va_end(args);
#ifdef TRACE
	trace_out_vstr_date(1,szMessage);
#endif
	strcat(szMessage,"\n");
	gtk_text_printf(NULL,txt_msgs,szMessage);
}

/*
* Stampa la tabella specificata 
* se fp e' NULL apre il file e lo stampa
* altrimenti utilizza il file pointer passato in ingresso
*/
BOOL PrintTable(char *szParagraph,char *szTableName,char *szPrinterName,char *szKey,FILE *fp)
{
	char szSelectCmd[2048];
	char szCmd[2048];
	char szBuffer[256];
	char szTitle[256];
	char szPageTitle[256];
	FILE *fpFile;
	char szFileName[128];
	char *pPtr;
	int nRC;
	BOOL bOK=TRUE;

	if(strlen(szParagraph) && strlen(szTableName)){
		strcpy(szBuffer,szTableName);
		strcat(szBuffer,"_descr");
		xncGetFileString(szParagraph,szBuffer, "", szTitle, sizeof(szTitle),Cfg.szCfgFile,NULL); 

		if(fp==(FILE *)NULL){
			sprintf(szFileName,"%s/%s",Cfg.szPathStampe,szTableName);
			if ((fpFile=fopen(szFileName,"w"))==(FILE *)NULL) {
				bOK=FALSE;
			} else {
				fprintf(fpFile,"%s %s\n%s\n\n", GetDate(time((long *)0),szDateBuffer), GetTime(time((long *)0),szTimeBuffer),szTitle);
			}
		} else {
			fpFile=fp;
		}

		if(bOK==FALSE){
			return bOK;
		}

		xncGetFileString(szParagraph,szTableName, "", szSelectCmd, sizeof(szSelectCmd),Cfg.szCfgFile,NULL); 

		strcpy(szCmd,szSelectCmd);
		/*
		* Sostituisco i valori parametrici
		* %KEY%   : Chiave
		* %TIPOUBI%   : Tipo Ubicazioni
		* %ORDTIPO%   : Tipo Ordini
		*/

		if((pPtr=strstr(szSelectCmd,"%ORDTIPO%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoOrdini);
			strcat(szCmd,pPtr+strlen("%ORDTIPO%"));
		}
		strcpy(szSelectCmd,szCmd);

		if((pPtr=strstr(szSelectCmd,"%TIPOUBI%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			strcat(szCmd,Cfg.szTipoUbicazioni);
			strcat(szCmd,pPtr+strlen("%TIPOUBI%"));
		}
		strcpy(szSelectCmd,szCmd);

		if((pPtr=strstr(szSelectCmd,"%KEY%"))){
			*pPtr='\0';
			strcpy(szCmd,szSelectCmd);
			if(szKey!=NULL){
				strcat(szCmd,szKey);
			} else {
				strcat(szCmd,"NULL");
			}
			strcat(szCmd,pPtr+strlen("%KEY%"));
		}
		strcpy(szSelectCmd,szCmd);

		nRC=PrintListFromSelect(fpFile,szSelectCmd);

		/*
		* Salto pagina
		*/
		if(fp==(FILE *)NULL){
			fprintf(fpFile,"");
			fclose(fpFile);

			if(nRC){
				strcpy(szPageTitle,"CNI Easy Picking 3.0");
				PrintFile(szFileName,szPrinterName,szPageTitle,Cfg.nA2ps,0);
			} else {
				bOK=FALSE;
			}
		}
	} else {
		bOK=FALSE;
	}
	return bOK;
}


void PrintFile(char *szFile,char *szPrinter,char *szTitle,BOOL bA2PS,int nWidth)
{
	char szCommand[2048];
	char szBuffer[128];


	if(bA2PS){
		strcpy(szCommand,"a2ps -R -q --medium=A4  --columns=1");
		/* titolo */
		if(szTitle && strlen(szTitle)){
			sprintf(szBuffer," --center-title=\"%s\"",szTitle);
			strcat(szCommand,szBuffer);
		}
		if(nWidth){
			sprintf(szBuffer," -l %d",nWidth);
			strcat(szCommand,szBuffer);
		} else {
			sprintf(szBuffer," -l %d", GetMaxLineWidth(szFile));
			strcat(szCommand,szBuffer);
		}
	} else {
		strcpy(szCommand,"lpr -r ");
	}
	
	/* stampante specificata */
	if(szPrinter && strlen(szPrinter)){
		sprintf(szBuffer," -P%s",szPrinter);
		strcat(szCommand,szBuffer);
	}

	sprintf(szBuffer," %s",szFile);
	strcat(szCommand,szBuffer);

#ifdef TRACE
	trace_out_vstr_date(1,"PrintFile : %s A2PS:%d",szCommand,bA2PS);
#endif

	system(szCommand);
}


/*
* PrintListFromSelect()
* ritorna il numero di tuple o -1 se errore
*/
int PrintListFromSelect(FILE *fp,char *szSqlCmd)
{
	int nTuples;
	int nFields;
	int nIndex;
	char szBuffer[128];
	int nFieldIndex;
	PGresult *PGRes = NULL;
	int nFieldLength[256];

	/*
	* Eseguo la select specificata
	*/
	PGRes=PGExecSQL(Cfg.nDebugVersion,szSqlCmd);

	nTuples=PQntuples(PGRes);
	nFields=PQnfields(PGRes);

	if(nTuples){
		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* ricavo le lunghezze dei titoli
			*/ 
			if(PQftype(PGRes,nFieldIndex)==1042){
				/* si tratta di una stringa */
				nFieldLength[nFieldIndex]=PQgetlength(PGRes,0,nFieldIndex);
			} else if(PQftype(PGRes,nFieldIndex)==701){
				/* si tratta di un float */
				nFieldLength[nFieldIndex]=strlen(PQfname(PGRes,nFieldIndex));
			} else if(PQftype(PGRes,nFieldIndex)==23){
				/* si tratta di un int */
				nFieldLength[nFieldIndex]=strlen(PQfname(PGRes,nFieldIndex));
			} else {
				/* altro... lo tratto come una stringa */
				nFieldLength[nFieldIndex]=PQgetlength(PGRes,0,nFieldIndex);
			}
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer);
			fprintf(fp,"+");
		}
		fprintf(fp,"\n");

		fprintf(fp,"|");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			/*
			* impostazione titolo colonna : utilizzo la .._set_column_title per avere la possibilita'
			* di ottenere la larghezza ottimale in fase di visualizzazione
			*/ 
			fprintf(fp,"%-*.*s|",nFieldLength[nFieldIndex],nFieldLength[nFieldIndex],PQfname(PGRes,nFieldIndex));
		}
		fprintf(fp,"\n");

		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer); fprintf(fp,"+");
		}
		fprintf(fp,"\n");

		/*
		* Aggiungo le tuple alla lista (una alla volta)
		*/
		for(nIndex=0;nIndex<nTuples;nIndex++){

			fprintf(fp,"|");
			for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
				if(PQftype(PGRes,nFieldIndex)==1042){
					/* si tratta di una stringa */
					fprintf(fp,"%*s|",nFieldLength[nFieldIndex],PQgetvalue(PGRes,nIndex,nFieldIndex));
				} else if(PQftype(PGRes,nFieldIndex)==701){
					/* si tratta di un float */
					fprintf(fp,"%*.*f|",nFieldLength[nFieldIndex],3,atof(PQgetvalue(PGRes,nIndex,nFieldIndex)));
				} else if(PQftype(PGRes,nFieldIndex)==23){
					/* si tratta di un int */
					fprintf(fp,"%*d|",nFieldLength[nFieldIndex],atoi(PQgetvalue(PGRes,nIndex,nFieldIndex)));
				} else {
					/* altro... lo tratto come una stringa */
					fprintf(fp,"%*s|",nFieldLength[nFieldIndex],PQgetvalue(PGRes,nIndex,nFieldIndex));
				}
			}
			fprintf(fp,"\n");
		}
		fprintf(fp,"+");
		for(nFieldIndex=0;nFieldIndex<nFields;nFieldIndex++){
			memset(szBuffer,'-',nFieldLength[nFieldIndex]); szBuffer[nFieldLength[nFieldIndex]]='\0';
			fprintf(fp,szBuffer); fprintf(fp,"+");
		}
		fprintf(fp,"\n");

	}
	
	PQclear(PGRes);

	return nTuples;
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

