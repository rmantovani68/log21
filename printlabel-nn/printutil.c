/*
* printfun.c : Stampa Picking List : funzioni di utilita'
* 
* Progetto Mondadori Picking
*
* Autore : Daniele Ravaioli
*
* Copyright A&L srl 2003-2021
*
* 02-01-2002 rm+st : ATTEZIONE : i prezzi in euro sono interi (in centesimi)
*                    usare la funzione NotazioneConVirgola() per la normalizzazione
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/stat.h>
#include <glib.h>

#include <libpq-fe.h>
#include <pmx_msq.h>
#include <msq_lib.h>
#include <shared.h>
#ifdef TRACE
	#include <trace.h>
#endif
#include <proc_list.h>
#include <picking.h>
#include <ep-common.h>

#include	"printlabel.h"
#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"
#include	"linklist.h"
#include	"stampa.h"



/*
* 02-01-2002 rm+st : modificare per gestione EURO (usare campo ELCPZCOE * 100)
* esempio : 2345,45 EUR
*/
char *NotazioneConPunti(int nNum,char *pszString)
{
	int nLen,nPunti,nResto,nIndex,nActualDest,nActualOrig,nGap;
	char szNum[80];

	sprintf(szNum,"%d",nNum);
	nLen=strlen(szNum);
	nPunti=nLen/3;
	nResto=nLen%3;
	if (!nResto){
		nPunti--;
		nResto=3;
	}
	nActualDest=nLen+nPunti;
	nActualOrig=nLen-1;
	pszString[nActualDest--]='\0';
	for(nIndex=0;nIndex<nPunti;nIndex++){
		for(nGap=0;nGap<3;nGap++){
			pszString[nActualDest--]=szNum[nActualOrig--];
		}
		pszString[nActualDest--]='.';
	}
	for(nIndex=0;nIndex<nResto;nIndex++){
		pszString[nActualDest--]=szNum[nActualOrig--];
	}
	return(pszString);
}

/*
* 02-01-2002 rm+st : Per gestione euro
* esempio : 2345,45 EUR
*/
char *NotazioneConVirgola(int nNum,char *pszString)
{
	int nInteger,nDecimal;

	nInteger=nNum/100;
	nDecimal=nNum%100;

	sprintf(pszString,"%d,%02d",nInteger,nDecimal);

	return(pszString);
}


/*
* Questa funzione converte un numero nel corrispondente in lettere
*/
char *NumberToLetter(int nNum,char *pszString)
{
	int nLen,nIndex;
	char szNum[80];
	char cChar;

	sprintf(szNum,"%d",nNum);
	nLen=strlen(szNum);
	for(nIndex=0;nIndex<nLen;nIndex++){
		cChar=szNum[nIndex];
		switch (cChar){
			case '0': cChar='Z'; break;
			case '1': cChar='A'; break; 
			case '2': cChar='E'; break;
			case '3': cChar='G'; break;
			case '4': cChar='H'; break;
			case '5': cChar='M'; break;
			case '6': cChar='P'; break;
			case '7': cChar='D'; break;
			case '8': cChar='T'; break;
			case '9': cChar='K'; break;
		}				
		pszString[nIndex]=cChar;
	}
	pszString[nIndex]='\0';
	return(pszString);
}




void local_PrintFile(char *szFile,char *szPrinter,char *szTitle,BOOL bA2PS,int nWidth)
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
		//strcpy(szCommand,"lpr -r ");
		strcpy(szCommand,"lpr ");
	}
	
	/* stampante specificata */
	if(szPrinter && strlen(szPrinter)){
		sprintf(szBuffer," -P%s",szPrinter);
		strcat(szCommand,szBuffer);
	}

	sprintf(szBuffer," %s",szFile);
	strcat(szCommand,szBuffer);

#ifdef TRACE
	trace_out_vstr_date(1,"PrintFile : %s",szCommand);
#endif
	system(szCommand);
}

void PrintFileRaw(char *szPrintFile,char *szPrinterName)
{
	char szCommand[128];

	sprintf(szCommand,"lpr -P%s %s",szPrinterName,szPrintFile);
#ifdef TRACE
	trace_out_vstr_date(1,"PrintFileRaw : %s",szCommand);
#endif
	system(szCommand);
}
