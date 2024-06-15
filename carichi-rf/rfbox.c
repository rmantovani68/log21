/*
* Modulo : rfbox.c
* --------------------
* Funzioni per la gestione dei box e di varia utilita'
*
* Progetto EMI Italiana SPA - Gestione Ordini
*
* Data creazione 19-06-2000
*
* Autore : Roberto Mantovani
*
* Copyright A&L srl 1996-2000
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
* function sane_exit()
*/
void sane_exit(int retval)
{
	endwin();
#ifdef TRACE
	trace_debug(TRUE, TRUE, "Quit");
#endif

	exit(retval);
}

/*
* SigSafeIntFunc()
*
* safe exit from software interrupt  (signal)
*/
void SigSafeIntFunc(int nSignal)
{
#ifdef TRACE
	trace_debug(TRUE, FALSE, "GOT SIGNAL : %d : ", nSignal);
	switch(nSignal){
		case SIGINT:  trace_debug(FALSE, TRUE, "SIGINT"); break;
		case SIGQUIT: trace_debug(FALSE, TRUE, "SIGQUIT"); break;
		case SIGILL:  trace_debug(FALSE, TRUE, "SIGILL"); break;
		case SIGKILL: trace_debug(FALSE, TRUE, "SIGKILL"); break;
		case SIGTERM: trace_debug(FALSE, TRUE, "SIGTERM"); break;
		case SIGHUP:  trace_debug(FALSE, TRUE, "SIGHUP"); break;
	}
#endif
	SafeIntFunc();
}

/*
* SafeIntFunc()
*
* safe exit from software interrupt 
*/
void SafeIntFunc()
{
	char szBuffer[128];

	/* chiudo la connessione con il database */
	//DisconnectDataBase();
	if(DBConnectionActive()){
		DBDisconnect();
	}

#ifdef TRACE
	trace_debug(TRUE, TRUE, "Stopped");
#endif
	/*
	* Avverto Main dello STOP
	*/
	sprintf(szBuffer,"%d",Macchina.nNumero);
	//SendProcessMessage(Cfg.nMainId,PROC_RF,PROGRAM_STOPPED,szBuffer);

	sane_exit(0);
}

void FlashMessage(char *szMessage)
{
	WINDOW *win;
	PANEL *pan;

	if ((win = newwin(SCREEN_HEIGHT-2, SCREEN_WIDTH-2,1,1)) != 0) {
		pan = new_panel(win);

		box(win, 0, 0);
		wmove(win, 1, 1);
		wprintw(win, szMessage);

		refresh();
		update_panels();
		doupdate();

	}
	sleep(1);
	delwin(win);
	del_panel(pan);
	refresh();


}

/*
* Funzione DisplayMessage()
*
* scrive la riga in basso
*/
void DisplayMessage(int nRow,char *szMessage,ep_bool_t bReverse)
{
	int     nLung;
	int		nPosX;
	char    szBuffer[100];

	nLung = strlen(szMessage);
	nPosX = (SCREEN_WIDTH - nLung) / 2;

	memset(szBuffer, ' ', SCREEN_WIDTH);	/* Carattere di fondo */
	memcpy(szBuffer+nPosX, szMessage,strlen(szMessage));
	szBuffer[SCREEN_WIDTH] = '\0';
	mvprintw(nRow,0,szBuffer);
	if(bReverse) mvchgat(nRow, 0, strlen(szBuffer), A_REVERSE, 1, NULL);
	refresh();

}

ep_bool_t LoginUser(void)
{
	char szBuffer[128];
	ep_bool_t bFine;

	FIELD *field[3];
	FORM  *my_form;
	int ch;
	int nCampo=1;

	Operatore.szIdOperatore[0]='\0';
	Operatore.szPasswordOperatore[0]='\0';

	clear();

	/* Initialize the fields */
	field[0] = new_field(1, 4, 4, 12, 0, 0); /* codice operatore */
	field[1] = new_field(1, 4, 6, 12, 0, 0); /* password */
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
	/* Field is filled up 		*/
	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"LOGIN OPERATORE",TRUE);

	mvprintw(4, 1, "OPERATORE :");
	mvprintw(6, 1, "PASSWORD  :");
	move(4,12);
	refresh();

	bFine=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();
#ifdef TRACE
		trace_debug(TRUE, TRUE, "Tasto Premuto : %d",ch);
#endif

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_INVIO:

				if(nCampo==1){
					/* enter su user */
					nCampo=2;
					form_driver(my_form, REQ_NEXT_FIELD);
					form_driver(my_form, REQ_END_LINE);
				} else {
					nCampo=1;
					form_driver(my_form, REQ_PREV_FIELD);
					form_driver(my_form, REQ_END_LINE);

					/* enter su password */
					strcpy(Operatore.szIdOperatore,field_buffer(field[0],0)); g_strstrip(Operatore.szIdOperatore);
					strcpy(Operatore.szPasswordOperatore,field_buffer(field[1],0)); g_strstrip(Operatore.szPasswordOperatore);

#ifdef TRACE
					trace_debug(TRUE, TRUE, "OP: %s",Operatore.szIdOperatore);
					trace_debug(TRUE, TRUE, "PW: %s",Operatore.szPasswordOperatore);
#endif
					if(CheckLogin(Operatore.szIdOperatore,Operatore.szPasswordOperatore)){
						DisplayMessage(SCREEN_HEIGHT,"LOGIN CORRETTO",TRUE);
						sleep(1);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
						bFine=TRUE;
					} else {
						DisplayMessage(SCREEN_HEIGHT,"ACCESSO NEGATO",TRUE);
						beep();
						FlashMessage("ACCESSO NEGATO");
						sleep(1);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
						set_field_buffer(field[0],0,"");
						set_field_buffer(field[1],0,"");
						refresh();
					}
				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

#ifdef DISABLED
			case KEY_DOWN:
				/* Go to next field */
				form_driver(my_form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(my_form, REQ_END_LINE);
			break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(my_form, REQ_PREV_FIELD);
				form_driver(my_form, REQ_END_LINE);
			break;
#endif
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	return (ch==KEY_INVIO);
}

int FormDistinta(void)
{
	char szBuffer[128];
	ep_bool_t bFine;
	ep_bool_t bOK;

	FIELD *field[3];
	FORM  *my_form;
	int ch;
	int nCampo=1;
	char szDIST[256];
	int nDist;

	szDIST[0]='\0';

	clear();

	/* Initialize the fields */
	field[0] = new_field(1, 6, 4, 2, 0, 0); /* Distinta */
	field[1] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
	set_field_type(field[0], TYPE_INTEGER,0,0,0);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"DISTINTA",TRUE);

	mvprintw(3, 1, "CODICE DISTINTA");
	move(4,2);
	refresh();

	bFine=FALSE;
	bOK=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_INVIO:

				if(nCampo==1){
					nCampo=1;
					form_driver(my_form, REQ_PREV_FIELD);
					form_driver(my_form, REQ_END_LINE);

					strcpy(szDIST,field_buffer(field[0],0)); g_strstrip(szDIST);

#ifdef TRACE
					trace_debug(TRUE, TRUE, "DIST: %s",szDIST);
#endif
					nDist=atoi(szDIST);
					if(DistintaEsistente(nDist)){
						bFine=TRUE;
						bOK=TRUE;
					} else {
						DisplayMessage(SCREEN_HEIGHT,"DISTINTA NON PRESENTE",TRUE);
						beep();
						//FlashMessage("DISTINTA NON PRESENTE");
					}


					refresh();
				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);

	if(bOK){
		return nDist;
	} else {
		return -1;
	}
}

int FormSetUdcInDistinta(int nDistinta)
{
	char szBuffer[128];
	ep_bool_t bFine;
	ep_bool_t bOK;

	FIELD *field[3];
	FORM  *my_form;
	int ch;
	int nCampo=1;
	char szUDC[256];
	char szDistBuffer[256];
	//int nDist;

	szUDC[0]='\0';

	clear();

	/* Initialize the fields */
	field[0] = new_field(1, 7, 3, 13, 0, 0); /* codice udc */
	field[1] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
	//set_field_type(field[0], TYPE_INTEGER,0,0,0);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"DISTINTA",TRUE);

	sprintf(szDistBuffer,"DISTINTA: %d",nDistinta);

	mvprintw(2, 1, szDistBuffer);
	mvprintw(3, 1, "CODICE UDC:");
	move(3,13);
	refresh();

	bFine=FALSE;
	bOK=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_INVIO:

				if(nCampo==1){
					nCampo=1;
					form_driver(my_form, REQ_PREV_FIELD);
					form_driver(my_form, REQ_END_LINE);

					strcpy(szUDC,field_buffer(field[0],0)); g_strstrip(szUDC);

#ifdef TRACE
					trace_debug(TRUE, TRUE, "UDC %s IN DISTINTA %d",szUDC,nDistinta);
#endif
					if (DistintaEsistente(nDistinta)){
						if (GetDistintaUdc(szUDC)==0){
							if(SetDistintaPerUdc(szUDC, nDistinta)){
								DisplayMessage(SCREEN_HEIGHT,"UDC IN DISTINTA",TRUE);
								sleep(1);
								DisplayMessage(SCREEN_HEIGHT,"",TRUE);
								set_field_buffer(field[0],0,"");
								refresh();

								form_driver(my_form, REQ_NEXT_FIELD);
								nCampo=1; 
								move(3,13);
							} else {
								DisplayMessage(SCREEN_HEIGHT,"UDC NON IN DISTINTA",TRUE);
								beep();
								//FlashMessage("UDC NON IN DISTINTA");
								//WarningMenu("UDC NON IN DISTINTA");
								sleep(3);
								DisplayMessage(SCREEN_HEIGHT,"",TRUE);
								set_field_buffer(field[0],0,"");
								refresh();

								form_driver(my_form, REQ_NEXT_FIELD);
								nCampo=1; 
								move(3,13);
							}

						} else {
							DisplayMessage(SCREEN_HEIGHT,"DISTINTA GIA' ASSEGNATA",TRUE);
							beep();
							//FlashMessage("DISTINTA GIA' ASSEGNATA");
							//WarningMenu("UDC NON IN DISTINTA");
							sleep(3);
							DisplayMessage(SCREEN_HEIGHT,"",TRUE);
							set_field_buffer(field[0],0,"");
							refresh();

							form_driver(my_form, REQ_NEXT_FIELD);
							nCampo=1; 
							move(3,13);
						}

						
					} else {
						DisplayMessage(SCREEN_HEIGHT,"DISTINTA INESISTENTE",TRUE);
						beep();
						//FlashMessage("DISTINTA INESISTENTE");
						//WarningMenu("UDC NON IN DISTINTA");
						sleep(3);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
						set_field_buffer(field[0],0,"");
						refresh();

						form_driver(my_form, REQ_NEXT_FIELD);
						nCampo=1; 
						move(3,13);
					}



					refresh();
				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);

	return 1;

}


ep_bool_t FormIngressoMerce(void)
{
	char szBuffer[128];
	ep_bool_t bFine;
	ep_bool_t bOK=TRUE;

	FIELD *field[3];
	FORM  *my_form;
	int ch;
	int nCampo=1;
	char szNMCED[256];
	char szCDPRO[256];
	char szQTPRO[256];
	char szDSTIT[256];

	szNMCED[0]='\0';
	szCDPRO[0]='\0';
	szQTPRO[0]='\0';
	szDSTIT[0]='\0';

	clear();

	/* Initialize the fields */
	/*
	* Altezza/larghezza/riga/colonna/offscreen rows/buffers
	*/
	//field[0] = new_field(1,  5, 2,12, 0, 0); /* Numero cedola */
	field[0] = new_field(1, CARICHI_PRODUCT_CODE_LENGTH, 3, 2, 0, 0); /* codice prodotto */
	field[1] = new_field(1,  6, 5,12, 0, 0); /* quantità */
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */

	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	//set_field_back(field[2], A_UNDERLINE); 
	//field_opts_off(field[2], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"INGRESSO",TRUE);

	//mvprintw(2, 1, "CEDOLA:");
	mvprintw(2, 1, "CODICE PRODOTTO");
	mvprintw(5, 1, "QUANTITA':");
	//move(2,12);
	move(3,2);
	refresh();

	bFine=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_F7:
			break;

			case KEY_INVIO:

				if(nCampo==1){
					/* enter su codprod */
					nCampo=2;
					form_driver(my_form, REQ_END_LINE);
					form_driver(my_form, REQ_NEXT_FIELD);
					DisplayMessage(SCREEN_HEIGHT-1,"",FALSE);
					move(5,12);
				} else if(nCampo==2){
					nCampo=1;
					form_driver(my_form, REQ_END_LINE);
					form_driver(my_form, REQ_FIRST_FIELD);

					/* enter su qtpro */
					strcpy(szCDPRO,field_buffer(field[0],0)); // // g_strstrip(szCDPRO);
					/* correzione check digiti ean 13 */
					szCDPRO[CARICHI_PRODUCT_CODE_LENGTH-1]=' ';
					strcpy(szQTPRO,field_buffer(field[1],0)); g_strstrip(szQTPRO);
					if (atoi(szQTPRO)<500000){
#ifdef TRACE
						trace_debug(TRUE, TRUE, "INGRESSO MERCE PRODOTTO: [%s] QUANTITA': %s",szCDPRO,szQTPRO);
#endif
						if( CodiceProdottoEsistente(szCDPRO)){
							if(MovimentaProdotto("INGRESSO",szCDPRO,atoi(szQTPRO),0,"",Macchina.szCodice,Operatore.szIdOperatore)){
								DisplayMessage(SCREEN_HEIGHT,"CARICO CORRETTO",TRUE);
								sleep(1);
								DisplayMessage(SCREEN_HEIGHT,"",TRUE);
								set_field_buffer(field[0],0,"");
								set_field_buffer(field[1],0,"");
								refresh();

								// form_driver(my_form, REQ_NEXT_FIELD);
								nCampo=1; /* codice prodotto */
								DisplayMessage(SCREEN_HEIGHT-1,"F7 - GENERA CODICE",FALSE);
								move(3,2);
							} else {
								DisplayMessage(SCREEN_HEIGHT,"CARICO ERRATO",TRUE);
								beep();
								//FlashMessage("CARICO ERRATO");
								sleep(2);
								DisplayMessage(SCREEN_HEIGHT,"",TRUE);
								set_field_buffer(field[0],0,"");
								set_field_buffer(field[1],0,"");
								move(3,2);
								refresh();
							}
						} else {
							DisplayMessage(SCREEN_HEIGHT,"PRODOTTO INESISTENTE",TRUE);
							beep();
							//FlashMessage("ERRORE IN INSERIMENTO A CATALOGO");
							sleep(2);
							DisplayMessage(SCREEN_HEIGHT,"",TRUE);
						}
					} else {
						DisplayMessage(SCREEN_HEIGHT,"QUANTITA' > 500000",TRUE);
						beep();
						//FlashMessage("ERRORE QUANTITA'");
						sleep(2);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
					}
				} else {
				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

#ifdef DISABLED
			case KEY_DOWN:
				/* Go to next field */
				form_driver(my_form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(my_form, REQ_END_LINE);
			break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(my_form, REQ_PREV_FIELD);
				form_driver(my_form, REQ_END_LINE);
			break;
#endif
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	return bOK;
}

ep_bool_t FormSpedizioneMerce(void)
{
	char szBuffer[128];
	ep_bool_t bFine;
	ep_bool_t bOK=TRUE;

	FIELD *field[3];
	FORM  *my_form;
	int ch;
	int nCampo=1;
	char szCDPRO[256];
	char szQTPRO[256];

	szCDPRO[0]='\0';
	szQTPRO[0]='\0';

	clear();

	/* Initialize the fields */
	//field[0] = new_field(1,  5, 2,12, 0, 0); /* cedola */
	field[0] = new_field(1, CARICHI_PRODUCT_CODE_LENGTH, 3, 2, 0, 0); /* codice prodotto */
	field[1] = new_field(1,  6, 5,12, 0, 0); /* quantità */
	field[2] = NULL;

	/* Set field options */
	set_field_back(field[0], A_UNDERLINE); 	/* Print a line for the option 	*/
	field_opts_off(field[0], O_AUTOSKIP);  	/* Don't go to next field when this */
	/* Field is filled up 		*/
	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"SPEDIZIONE",TRUE);

	//mvprintw(2, 1, "CEDOLA:");
	mvprintw(2, 1, "CODICE PRODOTTO");
	mvprintw(5, 1, "QUANTITA':");
	//move(2,12);
	move(3,2);
	refresh();

	bFine=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_INVIO:

				if(nCampo==1){
					/* enter su codprod */
					nCampo=2;
					form_driver(my_form, REQ_END_LINE);
					form_driver(my_form, REQ_NEXT_FIELD);
				} else {
					nCampo=1;
					form_driver(my_form, REQ_END_LINE);
					form_driver(my_form, REQ_FIRST_FIELD);

					/* enter su qtpro */
					strcpy(szCDPRO,field_buffer(field[0],0)); // g_strstrip(szCDPRO);
					/* correzione check digiti ean 13 */
					szCDPRO[CARICHI_PRODUCT_CODE_LENGTH-1]=' ';
					strcpy(szQTPRO,field_buffer(field[1],0)); g_strstrip(szQTPRO);

					if (atoi(szQTPRO)<500000){
#ifdef TRACE
						trace_debug(TRUE, TRUE, "SPEDIZIONE CDPRO: %s QTPRO: %s",szCDPRO,szQTPRO);
#endif
						if(MovimentaProdotto("SPEDIZIONE",szCDPRO,atoi(szQTPRO),0,"",Macchina.szCodice,Operatore.szIdOperatore)){
							DisplayMessage(SCREEN_HEIGHT,"SPEDIZIONE CORRETTA",TRUE);
							sleep(1);
							DisplayMessage(SCREEN_HEIGHT,"",TRUE);

							set_field_buffer(field[0],0,"");
							set_field_buffer(field[1],0,"");
							refresh();

							//form_driver(my_form, REQ_NEXT_FIELD);
							move(3,2);
							nCampo=1;
						} else {
							DisplayMessage(SCREEN_HEIGHT,"SPEDIZIONE ERRATA",TRUE);
							beep();
							//FlashMessage("SPEDIZIONE ERRATA");
							sleep(2);
							DisplayMessage(SCREEN_HEIGHT,"",TRUE);
							set_field_buffer(field[0],0,"");
							set_field_buffer(field[1],0,"");
							move(3,2);
							refresh();
						}
					} else {
						DisplayMessage(SCREEN_HEIGHT,"QUANTITA' > 500000",TRUE);
						beep();
						//FlashMessage("ERRORE QUANTITA'");
						sleep(2);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
					}
				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

#ifdef DISABLED
			case KEY_DOWN:
				/* Go to next field */
				form_driver(my_form, REQ_NEXT_FIELD);
				/* Go to the end of the present buffer */
				/* Leaves nicely at the last character */
				form_driver(my_form, REQ_END_LINE);
			break;
			case KEY_UP:
				/* Go to previous field */
				form_driver(my_form, REQ_PREV_FIELD);
				form_driver(my_form, REQ_END_LINE);
			break;
#endif
			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 

	return bOK;
}

ep_bool_t FormConteggioMerce()
{
	char szBuffer[128];
	ep_bool_t bFine;
	ep_bool_t bOK=TRUE;

	FIELD *field[4];
	FORM  *my_form;
	int ch;
	int nCampo=1;
	int nQTPRO;
	char szUDC[256];
	char szCDPRO[256];
	char szQTPRO[256];
	int nGiacenzaPresente=0;
	int nGiacenzaContata=0;
	int nGiacenzaDifferenza=0;

	szUDC[0]='\0';
	szCDPRO[0]='\0';
	szQTPRO[0]='\0';

	clear();

	/* Initialize the fields */
	field[0] = new_field(1, 7, 3, 2, 0, 0); /* codice udc */
	field[1] = new_field(1, CARICHI_PRODUCT_CODE_LENGTH, 6, 2, 0, 0); /* codice prodotto */
	field[2] = new_field(1, 6, 9, 13, 0, 0); /* quantità */
	field[3] = NULL;

	set_field_back(field[0], A_UNDERLINE);
	field_opts_off(field[0], O_AUTOSKIP); 

	set_field_back(field[1], A_UNDERLINE); 
	field_opts_off(field[1], O_AUTOSKIP);

	set_field_back(field[2], A_UNDERLINE);
	field_opts_off(field[2], O_AUTOSKIP); 

	/* Create the form and post it */
	my_form = new_form(field);
	post_form(my_form);
	refresh();

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"CONTEGGIO",TRUE);

	mvprintw(2, 2, "CODICE UDC:");
	mvprintw(5, 2, "CODICE PRODOTTO:");
	mvprintw(9, 2, "QUANTITA:");
	mvprintw(10, 2, "DIFFEREN:        ");
	move(3,2);
	refresh();

	bFine=FALSE;

	/* Loop through to get user requests */
	while(!bFine){

		ch = getch();

		switch(ch) {	
			case KEY_F9:
			case KEY_F8:
				bFine=TRUE;
			break;

			case KEY_INVIO:
				bOK=TRUE;
				if(nCampo==1){
					/* enter su udc */
					nCampo=2;
					form_driver(my_form, REQ_NEXT_FIELD);
					form_driver(my_form, REQ_END_LINE);
				} else if(nCampo==2){
					/* enter su codprod */
					form_driver(my_form, REQ_NEXT_FIELD);
					form_driver(my_form, REQ_END_LINE);
					strcpy(szCDPRO,field_buffer(field[1],0)); // g_strstrip(szCDPRO);
					/* correzione check digiti ean 13 */
					szCDPRO[CARICHI_PRODUCT_CODE_LENGTH-1]=' ';

					if (!CodiceProdottoEsistente(szCDPRO)) {
						DisplayMessage(SCREEN_HEIGHT,"PRODOTTO NON PRESENTE",TRUE);
						beep();
					}

					nGiacenzaPresente=GetGiacenzaProdotto(szCDPRO);
					nGiacenzaContata=GetContatoProdotto(szCDPRO);
					nGiacenzaDifferenza=nGiacenzaPresente-nGiacenzaContata;
					sprintf(szBuffer,"%d",nGiacenzaDifferenza);
					mvprintw(10, 13, szBuffer);
					nCampo=3;
					move(9,13);
				} else {
					nCampo=1;
					form_driver(my_form, REQ_PREV_FIELD);
					form_driver(my_form, REQ_END_LINE);
					form_driver(my_form, REQ_PREV_FIELD);
					form_driver(my_form, REQ_END_LINE);

					/* enter su qtpro */
					strcpy(szUDC,  field_buffer(field[0],0)); g_strstrip(szUDC);
					strcpy(szCDPRO,field_buffer(field[1],0)); // g_strstrip(szCDPRO);
					/* correzione check digiti ean 13 */
					szCDPRO[CARICHI_PRODUCT_CODE_LENGTH-1]=' ';
					strcpy(szQTPRO,field_buffer(field[2],0)); g_strstrip(szQTPRO);

					nQTPRO=atoi(szQTPRO);

					if (nQTPRO<500000){
#ifdef TRACE
						trace_debug(TRUE, TRUE, "UDC  : %s",szUDC);
						trace_debug(TRUE, TRUE, "CDPRO: %s",szCDPRO);
						trace_debug(TRUE, TRUE, "QTPRO: %s",szQTPRO);
#endif
						trace_debug(TRUE, TRUE, "QT: %d Conta: %d Ins: %d",nGiacenzaPresente,nGiacenzaContata,nQTPRO);

						if (!CodiceProdottoEsistente(szCDPRO)) {
							DisplayMessage(SCREEN_HEIGHT,"PRODOTTO NON PRESENTE",TRUE);
							beep();
						} else {
							if(MovimentaProdotto("CONTEGGIO",szCDPRO,nQTPRO,0,szUDC,Macchina.szCodice,Macchina.szCodice)){
							} else {
								bOK=FALSE;
							}

							if(bOK){
								DisplayMessage(SCREEN_HEIGHT,"CONTEGGIO CORRETTO",TRUE);
								sleep(1);
								DisplayMessage(SCREEN_HEIGHT,"",TRUE);
							} else {
								DisplayMessage(SCREEN_HEIGHT,"CONTEGGIO ERRATO",TRUE);
								beep();
								//FlashMessage("CONTEGGIO ERRATO");
								sleep(1);
							}
						}

						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
						set_field_buffer(field[0],0,"");
						set_field_buffer(field[1],0,"");
						set_field_buffer(field[2],0,"");
						mvprintw(10, 13, "       ");
						move(3,2);
						refresh();

					} else {
						DisplayMessage(SCREEN_HEIGHT,"QUANTITA' > 500000",TRUE);
						beep();
						//FlashMessage("ERRORE QUANTITA'");
						sleep(2);
						DisplayMessage(SCREEN_HEIGHT,"",TRUE);
					}

				}
			break;

			case KEY_LEFT:
				form_driver(my_form, REQ_LEFT_CHAR);
			break;
			case KEY_RIGHT:
				form_driver(my_form, REQ_RIGHT_CHAR);
			break;
			case KEY_HOME:
				form_driver(my_form, REQ_BEG_FIELD);
			break;
			case KEY_BACKSPACE:
				form_driver(my_form, REQ_DEL_PREV);
			break;

			default:
				/* If this is a normal character, it gets */
				/* Printed				  */	
				form_driver(my_form, ch);
			break;
		}
	}

	/* Un post form and free the memory */
	unpost_form(my_form);
	free_form(my_form);
	free_field(field[0]);
	free_field(field[1]); 
	free_field(field[2]); 

	return bOK;
}


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define CTRLD 	4

char *choices[] = {
                        "1-INGRESSO ",
                        "2-SPEDIZIONE",
                        "3-CONTEGGIO",
                        "4-UDC DISTINTA",
                        "5-FINE",
                  };

int MainMenu(void)
{
	ep_bool_t bFine;
	ITEM **my_items;
	int c;				
	MENU *my_menu;
	WINDOW *my_menu_win;
	int n_choices, i;
	ITEM *cur_item;
	int nItem;
	char szBuffer[128];
	
	clear();
	
	n_choices = ARRAY_SIZE(choices);
	my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

	for(i = 0; i < n_choices; ++i){
		// my_items[i] = new_item(choices[i], choices[i]);
		my_items[i] = new_item(choices[i], NULL);
	}
	my_items[n_choices] = (ITEM *)NULL;

	my_menu = new_menu((ITEM **)my_items);
	/* Create the window to be associated with the menu */
	my_menu_win = newwin(SCREEN_HEIGHT-1,SCREEN_WIDTH,1,0);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, SCREEN_HEIGHT-3, SCREEN_WIDTH-2, 1, 1));
	// set_menu_sub(my_menu, my_menu_win);

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	refresh();
	post_menu(my_menu);
	wrefresh(my_menu_win);

	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"MAIN MENU'",TRUE);
	refresh();

	bFine=FALSE;

	while(!bFine){
		c = getch();
		switch(c) {	
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
				nItem=c-'1';
				bFine=TRUE;
			break;
			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
			break;
			case KEY_INVIO:
				cur_item=current_item(my_menu);
				nItem=item_index(cur_item);
				bFine=TRUE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Selezionato Menu Item %d - Tasto %d",nItem,c);
				
#endif

			break;
		}
		wrefresh(my_menu_win);
	}

	unpost_menu(my_menu);
	free_item(my_items[0]);
	free_item(my_items[1]);
	free_item(my_items[2]);
	free_item(my_items[3]);
	free_item(my_items[4]);
	free_menu(my_menu);
	delwin(my_menu_win);

	return nItem;
}

char *warning_choices[] = {
                        "1-OK",
                  };


ep_bool_t WarningMenu(char *szMessage)
{
	ep_bool_t bFine;
	ITEM **my_items;
	int c;				
	MENU *my_menu;
	WINDOW *my_menu_win;
	int n_choices, i;
	ITEM *cur_item;
	int nItem;
	char szBuffer[128];
	
	//clear();
	
	n_choices = ARRAY_SIZE(warning_choices);
	my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

	for(i = 0; i < n_choices; ++i){
		// my_items[i] = new_item(choices[i], choices[i]);
		my_items[i] = new_item(warning_choices[i], NULL);
	}
	my_items[n_choices] = (ITEM *)NULL;

	my_menu = new_menu((ITEM **)my_items);


	/* Create the window to be associated with the menu */
	my_menu_win = newwin(SCREEN_HEIGHT-3,SCREEN_WIDTH,2,0);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, 2 , SCREEN_WIDTH-2, 1, 1));

	/* Set menu mark to the string " * " */
	set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	mvprintw(3, 2, szMessage);
	refresh();
	post_menu(my_menu);
	wrefresh(my_menu_win);


	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"USCITA",TRUE);
	refresh();

	bFine=FALSE;

	while(!bFine){
		c = getch();
		switch(c) {	
			case '1':
				nItem=c-'1';
				bFine=TRUE;
			break;
			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
			break;
			case KEY_INVIO:
				cur_item=current_item(my_menu);
				nItem=item_index(cur_item);
				bFine=TRUE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Selezionato Menu Item %d - Tasto %d",nItem,c);
				
#endif

			break;
		}
		wrefresh(my_menu_win);
	}	

	unpost_menu(my_menu);
	delwin(my_menu_win);
	free_item(my_items[0]);
	free_menu(my_menu);
	refresh();

	return nItem;
}




char *yes_no_choices[] = {
                        "1-SI",
                        "2-NO",
                  };


ep_bool_t YesNoMenu(char *szQuestion)
{
	ep_bool_t bFine;
	ITEM **my_items;
	int c;				
	MENU *my_menu;
	WINDOW *my_menu_win;
	int n_choices, i;
	ITEM *cur_item;
	int nItem;
	char szBuffer[128];
	
	clear();
	
	n_choices = ARRAY_SIZE(yes_no_choices);
	my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

	for(i = 0; i < n_choices; ++i){
		// my_items[i] = new_item(choices[i], choices[i]);
		my_items[i] = new_item(yes_no_choices[i], NULL);
	}
	my_items[n_choices] = (ITEM *)NULL;

	my_menu = new_menu((ITEM **)my_items);


	/* Create the window to be associated with the menu */
	my_menu_win = newwin(4,SCREEN_WIDTH,2,0);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, 2, SCREEN_WIDTH-2, 1, 1));

	/* Set menu mark to the string " * " */
	set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	refresh();
	post_menu(my_menu);
	wrefresh(my_menu_win);


	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"USCITA",TRUE);
	refresh();

	bFine=FALSE;

	while(!bFine){
		c = getch();
		switch(c) {	
			case '1':
			case '2':
				nItem=c-'1';
				bFine=TRUE;
			break;
			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
			break;
			case KEY_INVIO:
				cur_item=current_item(my_menu);
				nItem=item_index(cur_item);
				bFine=TRUE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Selezionato Menu Item %d - Tasto %d",nItem,c);
#endif

			break;
		}
		wrefresh(my_menu_win);
	}	

	unpost_menu(my_menu);
	delwin(my_menu_win);
	free_item(my_items[0]);
	free_item(my_items[1]);
	free_menu(my_menu);
	refresh();

	return nItem;
}


char *distinta_choices[] = {
                        "1-DISTINTA ESISTENTE",
                        "2-NUOVA DISTINTA",
                  };


int SetDistintaMenu(void)
{
	ep_bool_t bFine;
	ITEM **my_items;
	int c;				
	MENU *my_menu;
	WINDOW *my_menu_win;
	int n_choices, i;
	ITEM *cur_item;
	int nItem;
	char szBuffer[128];
	
	clear();
	
	n_choices = ARRAY_SIZE(distinta_choices);
	my_items = (ITEM **)calloc(n_choices + 1, sizeof(ITEM *));

	for(i = 0; i < n_choices; ++i){
		my_items[i] = new_item(distinta_choices[i], NULL);
	}
	my_items[n_choices] = (ITEM *)NULL;

	my_menu = new_menu((ITEM **)my_items);


	/* Create the window to be associated with the menu */
	my_menu_win = newwin(4,SCREEN_WIDTH,2,0);
	keypad(my_menu_win, TRUE);

	/* Set main window and sub window */
	set_menu_win(my_menu, my_menu_win);
	set_menu_sub(my_menu, derwin(my_menu_win, 2, SCREEN_WIDTH-2, 1, 1));

	/* Set menu mark to the string " * " */
	set_menu_mark(my_menu, " * ");

	/* Print a border around the main window and print a title */
	box(my_menu_win, 0, 0);
	refresh();
	post_menu(my_menu);
	wrefresh(my_menu_win);


	sprintf(szBuffer,"A&L - %-6.6s-%03d",Operatore.szIdOperatore,Macchina.nNumero);
	DisplayMessage(0,szBuffer,TRUE);
	DisplayMessage(SCREEN_HEIGHT,"SELEZIONE DISTINTA",TRUE);
	refresh();

	bFine=FALSE;

	while(!bFine){
		c = getch();
		switch(c) {	
			case '1':
			case '2':
				nItem=c-'1';
				bFine=TRUE;
			break;
			case KEY_DOWN:
				menu_driver(my_menu, REQ_DOWN_ITEM);
			break;
			case KEY_UP:
				menu_driver(my_menu, REQ_UP_ITEM);
			break;
			case KEY_INVIO:
				cur_item=current_item(my_menu);
				nItem=item_index(cur_item);
				bFine=TRUE;
#ifdef TRACE
				trace_debug(TRUE, TRUE, "Selezionato Menu Item %d - Tasto %d",nItem,c);
#endif

			break;
		}
		wrefresh(my_menu_win);
	}	

	unpost_menu(my_menu);
	delwin(my_menu_win);
	free_item(my_items[0]);
	free_item(my_items[1]);
	free_menu(my_menu);
	refresh();

	return nItem;
}

