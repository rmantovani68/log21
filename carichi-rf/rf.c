/*
* Modulo : rf.c
* ------------------
* Modulo per la gestione del terminale RF 
*
* Progetto DELTA Carichi/Scarichi
*
* Data creazione 19/06/2000
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

#include <fcntl.h>
#include <termio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <glib.h>

#include <pmx_msq.h>
#include <msq_lib.h>
#ifdef TRACE
	#include <trace.h>
#endif

#include <ep-common.h>
#include <picking.h>
#include <proc_list.h>
#include <ep-db.h>

#include <ncurses.h>
#include <panel.h>
#include <form.h>
#include <menu.h>

#include <dbfun-deltacarichi.h>

#include "rf.h"
#include "rfstruct.h"
#include "rfext.h"
#include "rffun.h"


/*
* Main function
* inizializzazione dati
* lettura dei dati di configurazione
*/
int main(int argc,char **argv)
{
#ifdef TRACE
	char szBufTrace[80];
#endif
	int nRC;
	ep_bool_t bFine;

	initscr();                    /* Start curses mode 		*/
	raw();                        /* Line buffering disabled	*/
	// start_color();
	keypad(stdscr, TRUE);         /* We get F1, F2 etc..		*/
	noecho();                     /* Don't echo() while we do getch */
    cbreak();

	/* 
	* modifico il livello di protezione di lettura / scrittura dei files
	*/
	umask(0);
	/*
	* Estrazione dei parametri dalla riga di comando
	*/
	if(argc>1){
		Macchina.nNumero=atoi(argv[1]);
	} else {
		Macchina.nNumero=0;
	}
	strcpy(Macchina.szNome, g_path_get_basename (argv[0]));
	sprintf(Macchina.szCodice,"%s_%02d", Macchina.szNome, Macchina.nNumero);
	/*
	* Leggo la configurazione
	*/
	//ReadConfiguration(TRUE);
	if(!ReadConfiguration(__configuration_file__,TRUE)){
        /* TODO */
	}
#ifdef TRACE
	/*
	* Apro il file trace 
	*/
	sprintf(szBufTrace,"%s/%s_%02d%s",Cfg.szPathTrace,Macchina.szNome,Macchina.nNumero,TRACE_FILE_SUFFIX);
	open_trace(Macchina.szNome, szBufTrace, TRACE_FILE_MAXSIZE);
	trace_debug(FALSE, TRUE, "Machine Name     : %s %2d",Macchina.szNome,Macchina.nNumero);
	trace_debug(TRUE, TRUE, "Started");
#endif
	/*
	* Definizione delle dimensioni dello schermo
	* I vari terminali hanno dimensioni diverse e bisogna tenerne conto
	* nelle varie applicazioni che si vogliono fare girare sopra
	*/
	if(!strcmp(Macchina.szNome,"rf")){
		/* versione con font 8x10 */
		// SCREEN_WIDTH=17;
		// SCREEN_HEIGHT=8;
		/* versione con font 6x8 */
		//SCREEN_WIDTH=26;
		// SCREEN_WIDTH=20;
		// SCREEN_HEIGHT=15;
		SCREEN_WIDTH=26;
		SCREEN_HEIGHT=16;
	} else if(!strcmp(Macchina.szNome,"rf960")){
		SCREEN_WIDTH=16;
		SCREEN_HEIGHT=8;
	} else if(!strcmp(Macchina.szNome,"dl9600")){
		SCREEN_WIDTH=15;
		SCREEN_HEIGHT=13;
	} else if(!strcmp(Macchina.szNome,"kyman")){
		SCREEN_WIDTH=26;
		SCREEN_HEIGHT=14;
	}

 
	/*
	* Gestione Database PostgreSQL
	*/
	if(!DBConnectUser(Cfg.szDBHost, Cfg.szDBPort, Cfg.szDBName, Cfg.szDBUser, Cfg.szDBPassword)){
#ifdef TRACE
		trace_debug(FALSE, TRUE, "Fallita connessione al database");
#endif
	}
	/*
	* intercetto le uscite irregolari dal programma
	*/
	signal(SIGINT,SigSafeIntFunc);
	signal(SIGQUIT,SigSafeIntFunc);
	signal(SIGILL,SigSafeIntFunc);
	signal(SIGKILL,SigSafeIntFunc);
	signal(SIGTERM,SigSafeIntFunc);
	signal(SIGHUP,SigSafeIntFunc);

	bFine=FALSE;

	if(LoginUser()){
		while(!bFine){
			nRC=MainMenu();
			switch(nRC){
				case 0: /* Ingresso Merce */
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Ingresso");
#endif
					FormIngressoMerce();
				break;
				case 1: /* scarico cedola */
#ifdef TRACE
					trace_debug(TRUE, TRUE, "Spedizione");
#endif
					FormSpedizioneMerce();
				break;
				case 2: /* conteggio */
				{
					FormConteggioMerce();
				}
				break;
				case 3: /* set distinta */
				{
					int nRCDist;
					int nDistinta;
					nRCDist=SetDistintaMenu();
					switch(nRCDist){
						case 0: 
							nDistinta=FormDistinta();
						break;
						case 1: 
							nDistinta=CreaDistinta();
						break;
					}
					FormSetUdcInDistinta(nDistinta);

				}
				break;
				case 4: /* fine  */
					if(0==YesNoMenu("USCITA")){
						bFine=TRUE;
					}
				break;
			}
		}
	}
	endwin();

	return 0;
}

/*
* ProcessKey()
*
* gestione tasti premuti
*/
void ProcessKey(int nKeyPressed)
{
	switch(nProgramStatus){

	}
}

