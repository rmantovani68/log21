/*
*	File sockets.c
*
*	Gestione sockets
*
* rm 17-11-2000 : ported from xnc library (courtesy of Maio)
*                 files : ca_sock.c ca_sckm.c ca_sckm0.c ca_sckm1.c
*
*	------------------------------------------------------------------------
*
*	Per mettere a disposizione un servizio bisogna usare la xncScktSrvrOpen.
*	Per gestire a loop bisogna usare la xncScktSrvrGest se c'e' qualcosa da
*	ricevere dentro la funz(int *fd, clientdata, BOH) bisogna fare una
*	xncScktSrvrRecv.
*
*	Per attaccarsi ad un servizio si puo' usare la xncScktOpen.
*
*	------------------------------------------------------------------------
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
/*
#include <proto.h>
*/
#include <sys/utsname.h>

#include "sockets.h"

/*
*	Inizio CA_SOCK.C
*/
/*
 *	***************************************************************************
 *
 *	Funzione xncGetMyName
 *
 *	Ottiene il nome della macchina dal quale mi sto connettendo.
 *
 *	char	*pszMachineName	-	buffer di deposito del nome della macchina
 *
 *	Ritorna:
 *		TRUE	-	tutto ok
 *		FALSE	-	errore
 *
 *	***************************************************************************
 */
BOOL xncGetMyName(char *pszMachineName)
{
	/*
	 *  Setto il nome locale
	 */
	strcpy(pszMachineName, "127.0.0.1");

	return (TRUE);
}																/*  Fine xncGetMyName */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktIsReady
 *
 *	Controlla se sulla connessione ci sono dei bytes pendenti da leggere.
 *
 *	int		*pnFd			-	numero di file descriptor
 *
 *	Ritorna:
 *		TRUE	-	qualcosina da leggere
 *		FALSE	-	niente da leggere o errore
 *
 *	***************************************************************************
 */
BOOL xncScktIsReady(int *pnFd)
{
	int nMaxFd;
	int nReadyFd;
	fd_set Canali;
	struct timeval Timeout;


	if (pnFd == (int *) NULL) {
		return (FALSE);
	}

	if (*pnFd < 0) {
		/*
		 *  Se il canale e' chiuso non c'e' niente da leggere
		 */
		return (FALSE);
	}

	FD_ZERO(&Canali);
	FD_SET(*pnFd, &Canali);
	nMaxFd = (*pnFd) + 1;

	/*
	 *  Setto il timeout
	 */
	Timeout.tv_sec = 0;
	Timeout.tv_usec = 0;

	nReadyFd = select(nMaxFd, &Canali, NULL, NULL, &Timeout);
	if (nReadyFd <= 0) {
		/*
		 *  Vuoto
		 */
		return (FALSE);
	}

	/*
	 *  C'e' qualcosa da leggere
	 */
	return (TRUE);
}																/*  Fine xncScktIsReady */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktRecv
 *
 *	Riceve da una connessione (in ingresso) una stringa (modalita' client).
 *
 *	int		*pnFd			-	numero di file descriptor
 *	char	*pszMessage		-	buffer di deposito del messaggio
 *	int		nLenMessage		-	lunghezza del buffer di ricezione
 *	ULONG	ulTimeOut		-	micro secondi di timeout (0=infinito)
 *
 *	Ritorna:
 *		>	0	-	il numero di caratteri letti
 *		<=	0	-	errore
 *
 *	***************************************************************************
 */
int xncScktRecv(int *pnFd, char *pszMessage, int nLenMessage, ULONG ulTimeOut)
{
	int nRC;
	int nMaxFd;
	int nReadyFd;
	BOOL bRC;
	fd_set Canali;
	struct timeval Timeout;


	if (pnFd == (int *) NULL || pszMessage == NULL) {
		return (0);
	}

	if (*pnFd < 0) {
		return (0);
	}

	if (ulTimeOut > 0) {
		FD_ZERO(&Canali);
		FD_SET(*pnFd, &Canali);
		nMaxFd = (*pnFd) + 1;

		/*
		 *  Setto il timeout
		 */
		Timeout.tv_sec = (time_t) (ulTimeOut / 1000000);
		Timeout.tv_usec = (time_t) (ulTimeOut % 1000000);

		nReadyFd = select(nMaxFd, &Canali, NULL, NULL, &Timeout);
		if (nReadyFd <= 0) {
			/*
			 *  Nessuna risposta (==0) oppure errore (<0)
			 */
			return (0);
		}
	}

	/*
	 *  Leggo il messaggio
	 */
	while ((nRC = recv(*pnFd, pszMessage, nLenMessage, 0)) < 0) {
		/*
		 *  Errore in lettura
		 */
		bRC = xncScktClose(pnFd);

		return (0);
	}

	/*
	 *  nRC e' il numero di caratteri letti
	 */
	if (nRC >= 0)
		*(pszMessage + nRC) = BZERO;

	if (nRC == 0) {
		/*
		 *  Nessun carattere letto percio' file descriptor da chiudere
		 */
		bRC = xncScktClose(pnFd);

		return (0);
	}

	/*
	 *  E' arrivata una risposta
	 */

	return (nRC);
}																/*  Fine xncScktRecv */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSend
 *
 *	Spedisce alla connessione specificata, un messaggio (modalita' client).
 *
 *	int		*pnFd			-	numero di file descriptor
 *	char	*pszMessage		-	messaggio da spedire
 *	int		nLenMessage		-	lunghezza del buffer da spedire
 *
 *	Ritorna:
 *		TRUE	-	tutto ok
 *		FALSE	-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktSend(int *pnFd, char *pszMessage, int nLenMessage)
{
	BOOL bRC;


	if (pnFd == (int *) NULL || pszMessage == NULL || nLenMessage == 0) {
		return (FALSE);
	}

	if (*pnFd < 0) {
		return (FALSE);
	}

	/*
	 *  Send vera e propria del messaggio
	 */
	while (send(*pnFd, pszMessage, nLenMessage, 0) < 0) {
		/*
		 *  Se la scrittura fallisce per EINTR, basta riprovare
		 */
		/*
		 *  Fallita la scrittura
		 */
		bRC = xncScktClose(pnFd);
		return (FALSE);
	}

	return (TRUE);
}																/*  Fine xncScktSend */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktClose
 *
 *	Chiude la connessione con l'host in ingresso.
 *
 *	int		*pnFd			-	numero di file descriptor
 *
 *	Ritorna:
 *		TRUE	-	tutto ok
 *		FALSE	-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktClose(int *pnFd)
{
	int nRC;


	if (pnFd == (int *) NULL) {
		return (FALSE);
	}

	if (pnFd >= 0) {
		/*
		 *  Chiusura vera e propria del file descriptor
		 */
		nRC = close(*pnFd);
		if (nRC < 0) {
			*pnFd = ID_SOCK_NO_FD;
			return (FALSE);
		}
	}

	/*
	 *  Azzero il file descriptor
	 */
	*pnFd = ID_SOCK_NO_FD;

	return (TRUE);
}																/*  Fine xncScktClose */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktOpen
 *
 *	Apre una connessione con l'host in ingresso (modalita' client).
 *
 *	char	*pszHostName	-	nome della macchina cui connettersi
 *	int		*pnFd			-	numero di file descriptor
 *
 *	Ritorna:
 *		TRUE	-	tutto ok
 *		FALSE	-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktOpen(char *pszServicesName, char *pszServicesType, int nServicesNumber, char *pszHostName, int *pnFd)
{
	int nOn;
	int nRC;
	long addr;
	char szHostName[128];
	BOOL bRC;
	struct servent *Servizio;
	struct hostent *host;
	struct sockaddr_in Sin;


	if (pnFd == (int *) NULL) {
		return (FALSE);
	}

	if (pszHostName == NULL || strlen(pszHostName) <= 0) {
		/*
		 *  Puntatore NULL o vuoto, percio' cerco il mio stesso nome
		 */
		xncGetMyName(szHostName);
		pszHostName = szHostName;
	}

	if (*pnFd < 0) {
		/*
		 *  Bisogna aprire il socket
		 */

		*pnFd = socket(AF_INET, SOCK_STREAM, 0);
		if (*pnFd < 0) {
			/*
			 *  Errore in apertura socket
			 */
			return (FALSE);
		}

		nOn = 1;
		nRC = setsockopt(*pnFd, SOL_SOCKET, SO_KEEPALIVE, (char *) &nOn, sizeof(nOn)
			);
		if (nRC < 0) {
			/*
			 *  Errore poco importante
			 */
		}

		nOn = 1;
		nRC = setsockopt(*pnFd, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn)
			);
		if (nRC < 0) {
			/*
			 *  Errore poco importante
			 */
		}

		memset((char *) &Sin, 0, sizeof(Sin));

		Sin.sin_family = AF_INET;
		addr = inet_addr(pszHostName);
		if (addr == -1) {
			host = gethostbyname(pszHostName);
			if (host == NULL) {
				/*
				 *  Scrivi meglio il nome
				 */
				bRC = xncScktClose(pnFd);

				return (FALSE);
			}

			memcpy((char *) &Sin.sin_addr.s_addr, host->h_addr, host->h_length);
		} else {
			Sin.sin_addr.s_addr = addr;
		}

		/*
		 *  Ottengo il servizio
		 */
		Servizio = getservbyname(pszServicesName, pszServicesType);
		Sin.sin_port = (Servizio ? Servizio->s_port : htons(nServicesNumber));

		if (connect(*pnFd, (struct sockaddr *) &Sin, sizeof(Sin)
				) < 0) {
			bRC = xncScktClose(pnFd);

			return (FALSE);
		}
	}

	if (*pnFd < 0) {
		return (FALSE);
	}

	return (TRUE);
}																/*  Fine xncScktOpen */

/*
*	Fine CA_SOCK.C
*/
/*
*	Inizio CA_SCKM.C
*/

/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrGetMaxFD
 *
 *	Ottiene il numero di file descriptor maggiore fra quelli in gestione.
 *
 *	fd_set	Canali		-	array dei canali attivi
 *
 *	Ritorna:
 *		il numero di file descriptor memorizzati
 *
 *	***************************************************************************
 */
int xncScktSrvrGetMaxFD(fd_set Canali)
{
	int nMaxFd = -1;


	int nFd;

	for (nFd = 0; nFd < ID_MAX_FD; nFd++)
		if (FD_ISSET(nFd, &Canali))
			nMaxFd = nFd;

	return (nMaxFd);
}																/*  Fine xncScktSrvrGetMaxFD */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrRecv
 *
 *	Ricezione di un buffer dalla connessione in ingresso (modalita' server).
 *
 *	int		*pnFd		-	file descriptor della connessione
 *	char	*pszMessage	-	buffer per il messaggio da ricevere
 *	int		nLenMessage	-	lunghezza del buffer
 *	ULONG	ulTimeOut	-	micro secondi di timeout (0=infinito)
 *
 *	Ritorna:
 *		>	0	-	il numero di caratteri letti
 *		<=	0	-	errore
 *
 *	***************************************************************************
 */
int xncScktSrvrRecv(int *pnFd, char *pszMessage, int nLenMessage, ULONG ulTimeOut)
{
	return (xncScktRecv(pnFd, pszMessage, nLenMessage, ulTimeOut));
}																/*  Fine xncScktSrvrRecv */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrSend
 *
 *	Spedizione di un messaggio sulla connessione in ingresso (modalita' server)
 *
 *	int		*pnFd		-	file descriptor della connessione
 *	char	*pszMessage	-	messaggio da inviare
 *	int		nLenMessage	-	lunghezza del buffer
 *
 *	Ritorna:
 *		ID_ERR_NO	-	tutto ok
 *		<	0		-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktSrvrSend(int *pnFd, char *pszMessage, int nLenMessage)
{
	return (xncScktSend(pnFd, pszMessage, nLenMessage));
}																/*  Fine xncScktSrvrSend */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrLoop
 *
 *	Loop infinito di gestione dei canali di comunicazione per la modalita'
 *	server.
 *
 *	char		*pszServ...	-	nome del servizio tcp/ip
 *	char		*pszServ...	-	tipo di servizio (tcp)
 *	int			nService...	-	numero di servizio
 *	BOOL		(*ptFunz())	-	callback richiamata quando c'e' qualcosa
 *	void *	ClientData	-	client data della callback
 *
 *	Ritorna:
 *		ID_ERR_NO	-	tutto ok
 *		<	0		-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktSrvrLoop(char *pszServicesName, char *pszServicesType, int nServicesNumber, BOOL(*ptFunz) (), void *ClientData)
{
	PTSCKTSRVR ptScktSrvr;


	ptScktSrvr = xncScktSrvrOpen(pszServicesName, pszServicesType, nServicesNumber);
	if (ptScktSrvr == (PTSCKTSRVR) NULL)
		return (FALSE);

	while (TRUE) {
		/*
		 *  Gestione a LOOP (si ferma in attesa)
		 */
		xncScktSrvrGest(ptScktSrvr, ptFunz, ClientData, 0);
	}

	return (TRUE);
}																/*  Fine xncScktSrvrLoop */

/*	
* Fine CA_SCKM.C 
*/
/*	
* Inizio CA_SCKM0.C 
*/


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrOpen
 *
 *	Inizializza il servizio che deve gestire il server.
 *
 *	char		*pszServ...	-	nome del servizio tcp/ip
 *	char		*pszServ...	-	tipo di servizio (tcp)
 *	int			nService...	-	numero di servizio
 *
 *	Ritorna:
 *		il puntatore alla struttura contenitore
 *
 *	***************************************************************************
 */
PTSCKTSRVR xncScktSrvrOpen(char *pszServicesName, char *pszServicesType, int nServicesNumber)
{
	int nRC;
	int nOn = 1;
	PTSCKTSRVR ptScktSrvr;
	struct servent *servizio;
	struct sockaddr_in sin;


	ptScktSrvr = (PTSCKTSRVR) malloc(sizeof(SCKTSRVR));
	if (ptScktSrvr == (PTSCKTSRVR) NULL)
		return ((PTSCKTSRVR) NULL);

	ptScktSrvr->nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ptScktSrvr->nSocket < 0) {
		free((void *) ptScktSrvr);

		return ((PTSCKTSRVR) NULL);
	}

	/*
	 *  Setta la possibilita' di riutilizzare l'indirizzo in modo che non ci
	 *  siano delle pause
	 */
	nOn = 1;
	nRC = setsockopt(ptScktSrvr->nSocket, SOL_SOCKET, SO_REUSEADDR, (char *) &nOn, sizeof(nOn)
		);
	if (nRC < 0) {
		/*
		 *  Errore ma me ne frego perche' non troppo importante
		 */
	}
	nOn = 1;
	nRC = setsockopt(ptScktSrvr->nSocket, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn)
		);
	if (nRC < 0) {
		/*
		 *  Errore ma me ne frego perche' non troppo importante
		 */
	}

	memset(&sin, 0, sizeof(struct sockaddr_in));

	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	/*
	 *  Ottengo il servizio
	 */
	servizio = getservbyname(pszServicesName, pszServicesType);
	sin.sin_port = (USHORT) (servizio ? servizio->s_port : htons(nServicesNumber));

	/*
	 *  listen setta il numero di canali in attesa di collegamento
	 */
	if (bind(ptScktSrvr->nSocket, (struct sockaddr *) &sin, sizeof(sin)) < 0 || listen(ptScktSrvr->nSocket, 10) < 0) {
		close(ptScktSrvr->nSocket);

		free((void *) ptScktSrvr);

		return ((PTSCKTSRVR) NULL);
	}

	FD_ZERO(&ptScktSrvr->Canali);
	FD_SET(ptScktSrvr->nSocket, &ptScktSrvr->Canali);

	return (ptScktSrvr);
}																/*  Fine xncScktSrvrOpen */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrGest
 *
 *	Funzione da richiamare a loop per ottenere dai socket i messaggi pendenti.
 *
 *	PTSCKTSRVR	ptScktSrvr	-	puntatore alla struttura contenitore
 *	BOOL		(*ptFunz())	-	callback richiamata quando c'e' qualcosa
 *	void *	ClientData	-	client data della callback
 *	ULONG		ulTimeOut	-	micro secondi di timeout (0=infinito)
 *
 *	Ritorna:
 *		True	-	tutto ok
 *		False	-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktSrvrGest(PTSCKTSRVR ptScktSrvr, BOOL(*ptFunz) (), void *ClientData, ULONG ulTimeOut)
{
	int nRC;
	int nOn = 1;
	int fromsize = sizeof(struct sockaddr_in);
	int nFd;
	int nTmpFd;
	int nMaxFd;
	int nReadyFd;
	BOOL bRC;
	fd_set Risposte;
	struct timeval Timeout;
	struct sockaddr_in from;


	if (ptScktSrvr == (PTSCKTSRVR) NULL) {
		return (FALSE);
	}

	/*
	 *  Setto il timeout
	 */
	Timeout.tv_sec = (time_t) (ulTimeOut / 1000000);
	Timeout.tv_usec = (time_t) (ulTimeOut % 1000000);

	/*
	 *  Ottengo il numero massimo di file descriptor
	 */
	nMaxFd = max(ptScktSrvr->nSocket, xncScktSrvrGetMaxFD(ptScktSrvr->Canali)) + 1;

	memcpy((char *) &Risposte, (char *) &ptScktSrvr->Canali, sizeof(fd_set));
	nReadyFd = select(nMaxFd, &Risposte, NULL, NULL, (struct timeval *) (ulTimeOut > 0 ? &Timeout : NULL)
		);

	if (nReadyFd > 0 && FD_ISSET(ptScktSrvr->nSocket, &Risposte)) {
		/*
		 *  se nReadyFd<0  si e' verificato un errore
		 *  se nReadyFd==0 si e' verificato un timeout
		 */

		/*
		 *  Un file descriptor in meno
		 */
		--nReadyFd;

		nFd = accept(ptScktSrvr->nSocket, (struct sockaddr *) &from, &fromsize);
		if (nFd < 0) {
			/*
			 *  Imbarazzante fallimento della accept
			 */
			return (FALSE);
		}

		nOn = 1;
		nRC = setsockopt(nFd, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn)
			);
		if (nRC < 0) {
			/*
			 *  Errore ma me ne frego perche' non troppo importante
			 */
		}

		/*
		 *  Un nuovo cliente
		 */

		if (nFd >= ID_MAX_FD) {
			/*
			 *  Sottodimensionamento: piu' di ID_MAX_FD clienti
			 */
			close(nFd);

			return (FALSE);
		}

		/*
		 *  Gestisco il nuovo cliente
		 */
		FD_SET(nFd, &ptScktSrvr->Canali);
	}

	for (nFd = 0; (nReadyFd--) > 0; nFd++) {

		/*
		 *  Salto il socket
		 */
		if (nFd == ptScktSrvr->nSocket)
			++nFd;

		while (!(FD_ISSET(nFd, &Risposte))) {
			++nFd;

			/*
			 *  Salto il socket
			 */
			if (nFd == ptScktSrvr->nSocket)
				++nFd;

			if (nFd >= ID_MAX_FD) {
				/*
				 *  Sfiga! Non sarebbe mai dovuto accadere!
				 */
				printf("xncScktSrvrGest - Critical error (%d>=%d)\r\n", nFd, ID_MAX_FD);
			}
		}

		/*
		 *  Loop di gestione vera e propria della connessione
		 */
		if (ptFunz != NULL) {
			nTmpFd = nFd;
			bRC = ptFunz(&nTmpFd, ClientData, NULL);
			if (nTmpFd == ID_SOCK_NO_FD) {
				/*
				 *  Canale chiuso quindi un file descriptor in meno
				 */
				FD_CLR(nFd, &Risposte);
				FD_CLR(nFd, &ptScktSrvr->Canali);
			}
		}
	}

	return (TRUE);
}																/*  Fine xncScktSrvrGest */


/*	
* Fine CA_SCKM0.C 
*/
/*	
* Inizio CA_SCKM1.C 
*/


#ifdef ELIMINATO
/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrFdInput
 *
 *	PTSCKTS...	ptScktS...	-	puntatore alla struttura del file descriptor
 *
 *	Ritorna:
 *		niente
 *
 *	***************************************************************************
 */
void xncScktSrvrFdInput(PTSCKTSRVRFD ptScktSrvrFd, int *pnFd, XtInputId * InpId)
{
	BOOL bRC;
	PTSCKTSRVR ptScktSrvr;


	if (ptScktSrvrFd == (PTSCKTSRVRFD) NULL) {
		return;
	}

	ptScktSrvr = (PTSCKTSRVR) ptScktSrvrFd->ptFather;
	if (ptScktSrvr == (PTSCKTSRVR) NULL) {
		return;
	}

	/*
	 *  Loop di gestione vera e propria della connessione
	 */
	if (ptScktSrvr->ptFunz != NULL) {
		bRC = ptScktSrvr->ptFunz(&ptScktSrvrFd->nFd, ptScktSrvr->ClientData, NULL);
	}

	if (ptScktSrvrFd->nFd == ID_SOCK_NO_FD) {
		/*
		 *  Canale chiuso
		 */

		if (ptScktSrvrFd->FdInputId != (XtInputId) 0)
			XtRemoveInput(ptScktSrvrFd->FdInputId);
		ptScktSrvrFd->FdInputId = (XtInputId) 0;
	}
}																/*  Fine xncScktSrvrFdInput */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrSocketInput
 *
 *	PTSCKTSRVR	ptScktSrvr	-	puntatore alla struttura contenitore
 *
 *	Ritorna:
 *		niente
 *
 *	***************************************************************************
 */
void xncScktSrvrSocketInput(PTSCKTSRVR ptScktSrvr, int *pnFd, XtInputId * InpId)
{
	int nRC;
	int nFd;
	int nIndex;
	int nOn = 1;
	int nFound = -1;
	int fromsize = sizeof(struct sockaddr_in);
	PTSCKTSRVRFD ptScktSrvrFd;
	struct sockaddr_in from;


	if (ptScktSrvr == (PTSCKTSRVR) NULL) {
		return;
	}

	nFd = accept(ptScktSrvr->nSocket, (struct sockaddr *) &from, &fromsize);
	if (nFd < 0) {
		/*
		 *  Imbarazzante fallimento della accept
		 */
		return;
	}

	nOn = 1;
	nRC = setsockopt(nFd, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn));

	/*
	 *  Un nuovo cliente
	 */

	/*
	 *  Cerco un buco ...
	 */
	for (nIndex = 0; nIndex < ID_MAX_FD && nFound < 0; nIndex++)
		if (ptScktSrvr->ScktSrvrFd[nIndex].nFd == ID_SOCK_NO_FD)
			nFound = nIndex;

	if (nFound < 0) {
		/*
		 *  Grosso guaio ...
		 */
		printf("xncScktSrvrSocketInput - Critical error (%d>=%d)\r\n", nFd, ID_MAX_FD);
		return;
	}

	ptScktSrvrFd = &ptScktSrvr->ScktSrvrFd[nFound];

	/*
	 *  Memorizzo il file descriptor
	 */
	ptScktSrvrFd->nFd = nFd;

	if (ptScktSrvrFd->FdInputId == (XtInputId) 0) {
		/*
		 *  Attacco la funzione di attesa caratteri
		 */
		ptScktSrvrFd->FdInputId = XtAppAddInput(XtWidgetToApplicationContext(ptScktSrvr->hWidget), ptScktSrvrFd->nFd, (XtPointer) XtInputReadMask, (XtInputCallbackProc) xncScktSrvrFdInput, (XtPointer) ptScktSrvrFd);
	}
}																/*  Fine xncScktSrvrSocketInput */


/*
 *	***************************************************************************
 *
 *	Funzione xncScktSrvrAddInput
 *
 *	Setta la add input per la gestione del socket.
 *
 *	Widget		hWidget		-	widget di riferimento
 *	PTSCKTSRVR	ptScktSrvr	-	puntatore alla struttura contenitore
 *
 *	Ritorna:
 *		True	-	tutto ok
 *		False	-	errore
 *
 *	***************************************************************************
 */
BOOL xncScktSrvrAddInput(Widget hWidget, PTSCKTSRVR ptScktSrvr, BOOL(*ptFunz) (int *, void *, void *), void *ClientData)
{
	int nIndex;


	if (ptScktSrvr == (PTSCKTSRVR) NULL) {
		return (False);
	}

	/*
	 *  Loop di inizializzazione
	 */
	for (nIndex = 0; nIndex < ID_MAX_FD; nIndex++) {
		ptScktSrvr->ScktSrvrFd[nIndex].nFd = ID_SOCK_NO_FD;
		ptScktSrvr->ScktSrvrFd[nIndex].ptFather = (void *) ptScktSrvr;
		ptScktSrvr->ScktSrvrFd[nIndex].FdInputId = (XtInputId) 0;
	}


	ptScktSrvr->hWidget = hWidget;
	ptScktSrvr->ptFunz = ptFunz;
	ptScktSrvr->ClientData = ClientData;

	ptScktSrvr->SocketInputId = XtAppAddInput(XtWidgetToApplicationContext(ptScktSrvr->hWidget), ptScktSrvr->nSocket, (XtPointer) XtInputReadMask, (XtInputCallbackProc) xncScktSrvrSocketInput, (XtPointer) ptScktSrvr);

	return (True);
}																/*  Fine xncScktSrvrAddInput */
#endif


/*	
* Fine CA_SCKM1.C 
*/
