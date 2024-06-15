/*
* printfun.c
* 
* Easy Picking 4.0  : Stampa Etichette
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 2003-2021
*/
#include <glib.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
// #include <termio.h>
#include <sys/stat.h>

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

#include	"printstruct.h"
#include	"printext.h"
#include	"printfun.h"


extern int errno;
extern char __version__[];

/*
* void ReadConfiguration(void)
* lettura della configurazione
*/
ep_bool_t ReadConfiguration(char *szCfgFileName)
{
	char szCurrentDirectory[128];
	char szTmpBuffer[128];
	int nIndex;
	ep_bool_t bOK=TRUE;

	/*
	* leggo il file di configurazione dalla directory corrente
	*/
	getcwd(szCurrentDirectory,sizeof(szCurrentDirectory));
	strcpy(Cfg.szCfgFileName,szCurrentDirectory);
	strcat(Cfg.szCfgFileName,"/");
	strcat(Cfg.szCfgFileName,szCfgFileName);

	if(!FileExists(Cfg.szCfgFileName)){
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Read Configuration : file %s not exists",szCfgFileName);
#endif
		return FALSE;
	}

	/* CFGItems viene dichiarato e inizializzato in mainvar.c */
    ReadCfgItems(CFGItems,Cfg.szCfgFileName,0);


	return TRUE;
}

ep_bool_t StampaSpedizione(char *szKey,char *szPrinter)
{
	ep_bool_t bOK=TRUE;
	char szCommand[512];
	int rc;
	gchar *pszStdOutput=NULL;
	gchar *pszStdError=NULL;
	//GError *pGError;
	int nExitStatus;


	sprintf(szCommand,"sh -c 'cd %s;/usr/bin/xvfb-run -w 1 -a /usr/local/bin/ncreport.sh -f %s/spedizione.xml -q QPSQL -h %s -u roberto -p 3zin -d %s -c %s -par ordprog,%s -o pdf -of %s/SPEDIZIONE_%s.pdf'",
		Cfg.szPathData,Cfg.szPathData,Cfg.szDBHost,Cfg.szDBName,Cfg.szConnectionName,szKey,Cfg.szPathStampe,szKey);

#ifdef TRACE
	trace_debug(TRUE, TRUE, "NCReport : %s",szCommand);
#endif
	g_spawn_command_line_sync(szCommand, &pszStdOutput, &pszStdError, &nExitStatus, NULL);
	rc=WEXITSTATUS(nExitStatus);
	
	if(pszStdOutput && strlen(pszStdOutput)>512){
		pszStdOutput[512]='\0';
	}
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Comando eseguito : ritorno : %d - ExitStatus : %d out : ",rc,nExitStatus);

	if(pszStdOutput){
		trace_debug(FALSE, FALSE, "%s",pszStdOutput);
	}
#endif


	if(Cfg.nDebugLevel > 2){
		sprintf(szCommand,"lpr -P%s %s/SPEDIZIONE_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	} else {
		sprintf(szCommand,"lpr -P%s -r %s/SPEDIZIONE_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	}

	rc=system(szCommand);
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Ritorno di comando lpr : %d",rc);
#endif
	
	return bOK;
}

ep_bool_t StampaDistintaLinea(char *szKey,char *szPrinter)
{
	ep_bool_t bOK=TRUE;
	char szCommand[512];
	int rc;
	gchar *pszStdOutput=NULL;
	gchar *pszStdError=NULL;
	//GError *pGError;
	int nExitStatus;


	sprintf(szCommand,"sh -c 'cd %s;/usr/bin/xvfb-run -w 1 -a /usr/local/bin/ncreport.sh -f %s/distinta_linea.xml -q QPSQL -h %s -u roberto -p 3zin -d %s -c %s -par key_linea,%s -o pdf -of %s/DISTINTA_LINEA_%s.pdf'",
		Cfg.szPathData,Cfg.szPathData,Cfg.szDBHost,Cfg.szDBName,Cfg.szConnectionName,szKey,Cfg.szPathStampe,szKey);

#ifdef TRACE
	trace_debug(TRUE, TRUE, "NCReport : %s",szCommand);
#endif
	g_spawn_command_line_sync(szCommand, &pszStdOutput, &pszStdError, &nExitStatus, NULL);
	rc=WEXITSTATUS(nExitStatus);
	
	if(pszStdOutput && strlen(pszStdOutput)>512){
		pszStdOutput[512]='\0';
	}
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Comando eseguito : ritorno : %d - ExitStatus : %d out : ",rc,nExitStatus);

	if(pszStdOutput){
		trace_debug(FALSE, FALSE, "%s",pszStdOutput);
	}
#endif


	if(Cfg.nDebugLevel > 2){
		sprintf(szCommand,"lpr -P%s %s/DISTINTA_LINEA_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	} else {
		sprintf(szCommand,"lpr -P%s -r %s/DISTINTA_LINEA_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	}

	rc=system(szCommand);
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Ritorno di comando lpr : %d",rc);
#endif
	
	return bOK;
}


ep_bool_t StampaFrontespizioBancale(char *szKey,char *szPrinter)
{
	ep_bool_t bOK=TRUE;
	char szCommand[512];
	int rc;
	gchar *pszStdOutput=NULL;
	gchar *pszStdError=NULL;
	//GError *pGError;
	int nExitStatus;


	/* TODO controllare se bancale esiste */

	/* in ogni caso creo la XAB in pdf */
	sprintf(szCommand,"sh -c 'cd %s;/usr/bin/xvfb-run -w 1 -a /usr/local/bin/ncreport.sh -f %s/frontespizio_bancale.xml -q QPSQL -h %s -u roberto -p 3zin -d %s -c %s -par key_bancale,%s -o pdf -of %s/FRONTESPIZIO_BANCALE_%s.pdf'",
		Cfg.szPathData,Cfg.szPathData,Cfg.szDBHost,Cfg.szDBName,Cfg.szConnectionName,szKey,Cfg.szPathStampe,szKey);

#ifdef TRACE
	trace_debug(TRUE, TRUE, "NCReport : %s",szCommand);
#endif
	g_spawn_command_line_sync(szCommand, &pszStdOutput, &pszStdError, &nExitStatus, NULL);
	rc=WEXITSTATUS(nExitStatus);
	
	if(pszStdOutput && strlen(pszStdOutput)>512){
		pszStdOutput[512]='\0';
	}
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Comando eseguito : ritorno : %d - ExitStatus : %d out : ",rc,nExitStatus);

	if(pszStdOutput){
		trace_debug(FALSE, FALSE, "%s",pszStdOutput);
	}
#endif


	if(Cfg.nDebugLevel > 2){
		sprintf(szCommand,"lpr -P%s %s/FRONTESPIZIO_BANCALE_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	} else {
		sprintf(szCommand,"lpr -P%s -r %s/FRONTESPIZIO_BANCALE_%s.pdf",szPrinter,Cfg.szPathStampe,szKey);
	}

	rc=system(szCommand);
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Ritorno di comando lpr : %d",rc);
#endif
	
	return bOK;
}

/*
* SafeIntFunc()
*
* safe exit from software interrupt 
*/
void SafeIntFunc()
{
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Got Signal ! ");
#endif
	exit(0);
}
