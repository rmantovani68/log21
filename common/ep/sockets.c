/**
* @file sockets.c
* Gestione sockets
*
* rm 17-11-2000 : ported from  library (courtesy of Maio)
*                 files : ca_sock.c ca_sckm.c ca_sckm0.c ca_sckm1.c
*
* ------------------------------------------------------------------------
* Per mettere a disposizione un servizio bisogna usare la ScktSrvrOpen.
* Per gestire a loop bisogna usare la ScktSrvrGest se c'e' qualcosa da
* ricevere dentro la funz(int *fd, clientdata, BOH) bisogna fare una
* ScktSrvrRecv.
*
* Per attaccarsi ad un servizio si puo' usare la ScktOpen.
* ------------------------------------------------------------------------
*/
#ifdef __linux__
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netdb.h>
#include <sys/utsname.h>
#endif
#include <time.h>
#include <sys/time.h>

#include <glib.h>
#include <ep-common.h>
#include "sockets.h"

/**
 * @brief get the machine name
 * 
 * @param pszMachineName 
 * @return ep_bool_t 
 */
ep_bool_t GetMyName(char *pszMachineName)
{
    /*
     *  Setto il nome locale
     */
    strcpy(pszMachineName, "127.0.0.1");

    return (TRUE);
}


/*
 *    ***************************************************************************
 *
 *    Funzione ScktIsReady
 *
 *    Controlla se sulla connessione ci sono dei bytes pendenti da leggere.
 *
 *    int        *pnFd            -    numero di file descriptor
 *
 *    Ritorna:
 *        TRUE    -    qualcosina da leggere
 *        FALSE    -    niente da leggere o errore
 *
 *    ***************************************************************************
 */


/**
 * @brief 
 * Controlla se sulla connessione ci sono dei bytes pendenti da leggere.
 * @param pnFd 
 * @return ep_bool_t 
 * TRUE  something to read
 * FALSE nothing to read
 */
ep_bool_t ScktIsReady(int *pnFd)
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
}

/**
 * @brief 
 * Riceve da una connessione (in ingresso) una stringa (modalita' client).
 * @param pnFd 
 * @param pszMessage 
 * @param nLenMessage 
 * @param ulTimeOut 
 * @return int 
 * >  0 il numero di caratteri letti
 * <= 0 errore
 */
int ScktRecv(int *pnFd, char *pszMessage, int nLenMessage, ULONG ulTimeOut)
{
    int nRC;
    int nMaxFd;
    int nReadyFd;
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
        ScktClose(pnFd);

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
        ScktClose(pnFd);

        return (0);
    }

    /*
     *  E' arrivata una risposta
     */

    return (nRC);
}

/**
 * @brief 
 * Spedisce alla connessione specificata, un messaggio (modalita' client).
 * @param pnFd numero di file descriptor
 * @param pszMessage messaggio da spedire
 * @param nLenMessage lunghezza del buffer da spedire
 * @return ep_bool_t 
 * TRUE  tutto ok
 * FALSE errore
 */
ep_bool_t ScktSend(int *pnFd, char *pszMessage, int nLenMessage)
{

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
        ScktClose(pnFd);
        return (FALSE);
    }

    return (TRUE);
}

/**
 * @brief 
 * Chiude la connessione con l'host in ingresso.
 * @param pnFd numero di file descriptor
 * @return ep_bool_t 
 * TRUE  tutto ok
 * FALSE errore
 */
ep_bool_t ScktClose(int *pnFd)
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
}

/**
 * @brief 
 * Apre una connessione con l'host in ingresso (modalita' client).
 * @param pszServicesName 
 * @param pszServicesType 
 * @param nServicesNumber 
 * @param pszHostName 
 * @param pnFd 
 * @return ep_bool_t 
 * TRUE  tutto ok
 * FALSE errore
 */
ep_bool_t ScktOpen(char *pszServicesName, char *pszServicesType, int nServicesNumber, char *pszHostName, int *pnFd)
{
    int nOn;
    int nRC;
    long addr;
    char szHostName[128];
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
        GetMyName(szHostName);
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
        nRC = setsockopt(*pnFd, SOL_SOCKET, SO_KEEPALIVE, (char *) &nOn, sizeof(nOn));
        if (nRC < 0) {
            /*
             *  Errore poco importante
             */
        }

        nOn = 1;
        nRC = setsockopt(*pnFd, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn));
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
                ScktClose(pnFd);

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

        if (connect(*pnFd, (struct sockaddr *) &Sin, sizeof(Sin)) < 0) {
            ScktClose(pnFd);

            return (FALSE);
        }
    }

    if (*pnFd < 0) {
        return (FALSE);
    }

    return (TRUE);
}

/**
 * @brief 
 * Ottiene il numero di file descriptor maggiore fra quelli in gestione.
 * @param Canali 
 * @return int 
 * il numero di file descriptor memorizzati
 */
int ScktSrvrGetMaxFD(fd_set Canali)
{
    int nMaxFd = -1;


    int nFd;

    for (nFd = 0; nFd < ID_MAX_FD; nFd++)
        if (FD_ISSET(nFd, &Canali))
            nMaxFd = nFd;

    return (nMaxFd);
}

/**
 * @brief 
 * Ricezione di un buffer dalla connessione in ingresso (modalita' server).
 * @param pnFd file descriptor della connessione
 * @param pszMessage buffer per il messaggio da ricevere
 * @param nLenMessage lunghezza del buffer
 * @param ulTimeOut micro secondi di timeout (0=infinito)
 * @return int 
 * >  0 il numero di caratteri letti
 * <= 0 errore
 */
int ScktSrvrRecv(int *pnFd, char *pszMessage, int nLenMessage, ULONG ulTimeOut)
{
    return (ScktRecv(pnFd, pszMessage, nLenMessage, ulTimeOut));
}

/**
 * @brief 
 * Spedizione di un messaggio sulla connessione in ingresso (modalita' server)
 * @param pnFd file descriptor della connessione
 * @param pszMessage messaggio da inviare
 * @param nLenMessage lunghezza del buffer
 * @return ep_bool_t 
 *  TRUE  tutto ok
 *  FALSE errore
 */
ep_bool_t ScktSrvrSend(int *pnFd, char *pszMessage, int nLenMessage)
{
    return (ScktSend(pnFd, pszMessage, nLenMessage));
}

/**
 * @brief Loop infinito di gestione dei canali di comunicazione per la modalità server.
 * 
 * @param pszServicesName nome del servizio tcp/ip
 * @param pszServicesType tipo di servizio (tcp)
 * @param nServicesNumber numero di servizio
 * @param ptFunz callback richiamata quando c'e' qualcosa
 * @param ClientData client data della callback
 * @return ep_bool_t 
 *  TRUE  tutto ok
 *  FALSE errore
 */
ep_bool_t ScktSrvrLoop(char *pszServicesName, char *pszServicesType, int nServicesNumber, ep_bool_t (*ptFunz) (), void *ClientData)
{
    PTSCKTSRVR ptScktSrvr;


    ptScktSrvr = ScktSrvrOpen(pszServicesName, pszServicesType, nServicesNumber);
    if (ptScktSrvr == (PTSCKTSRVR) NULL)
        return (FALSE);

    while (TRUE) {
        /*
         *  Gestione a LOOP (si ferma in attesa)
         */
        ScktSrvrGest(ptScktSrvr, ptFunz, ClientData, 0);
    }

    return (TRUE);
}


/*
 *    ***************************************************************************
 *
 *    Funzione ScktSrvrOpen
 *
 *    Inizializza il servizio che deve gestire il server.
 *
 *    char        *pszServ...    -    nome del servizio tcp/ip
 *    char        *pszServ...    -    tipo di servizio (tcp)
 *    int            nService...    -    numero di servizio
 *
 *    Ritorna:
 *        il puntatore alla struttura contenitore
 *
 *    ***************************************************************************
 */

/**
 * @brief Inizializza il servizio che deve gestire il server.
 * 
 * @param pszServicesName nome del servizio tcp/ip
 * @param pszServicesType tipo di servizio (tcp)
 * @param nServicesNumber numero di servizio
 * @return PTSCKTSRVR 
 * puntatore alla struttura contenitore
 */
PTSCKTSRVR ScktSrvrOpen(char *pszServicesName, char *pszServicesType, int nServicesNumber)
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
}

/**
 * @brief Funzione da richiamare a loop per ottenere dai socket i messaggi pendenti.
 * 
 * @param ptScktSrvr puntatore alla struttura contenitore
 * @param ptFunz callback richiamata quando c'e' qualcosa
 * @param ClientData client data della callback
 * @param ulTimeOut micro secondi di timeout (0=infinito)
 * @return ep_bool_t 
 * True  tutto ok
 * False errore
 */
ep_bool_t ScktSrvrGest(PTSCKTSRVR ptScktSrvr, ep_bool_t (*ptFunz) (), void *ClientData, ULONG ulTimeOut)
{
    int nRC;
    int nOn = 1;
    unsigned int fromsize = sizeof(struct sockaddr_in);
    int nFd;
    int nTmpFd;
    int nMaxFd;
    int nReadyFd;
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
    nMaxFd = max(ptScktSrvr->nSocket, ScktSrvrGetMaxFD(ptScktSrvr->Canali)) + 1;

    memcpy((char *) &Risposte, (char *) &ptScktSrvr->Canali, sizeof(fd_set));
    nReadyFd = select(nMaxFd, &Risposte, NULL, NULL, (struct timeval *) (ulTimeOut > 0 ? &Timeout : NULL));

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
        nRC = setsockopt(nFd, IPPROTO_TCP, TCP_NODELAY, (char *) &nOn, sizeof(nOn));
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
                printf("ScktSrvrGest - Critical error (%d>=%d)\r\n", nFd, ID_MAX_FD);
            }
        }

        /*
         *  Loop di gestione vera e propria della connessione
         */
        if (ptFunz != NULL) {
            nTmpFd = nFd;
            ptFunz(&nTmpFd, ClientData, NULL);
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
}

#endif