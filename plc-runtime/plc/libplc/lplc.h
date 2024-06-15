/* lplc.h */
/*
* @(#) libplc.c 2.6 Wed Jan  7 11:15:33 MET 1998
*
* Primitive di colloquio col PLC Isagraf.
*
* --/--/-- 1.0. Prima stasura.
* 20/06/95 1.1. Aggiunta la funzione "plcLdStatus".
* 14/07/95 1.2. Corretto un errore che poteva provocare il crash nella funzione
*          "dbFindIsaVar".
* 13/09/95 1.3. (Versione per Solaris) Aggiunta la funzione "plcDbChanged".
* 18/09/95 1.4. Unificato il sorgente per Solaris e LynxOS.
* 04/12/95 1.5. Aggiunte funzioni per lettura byte arbitrari e statistica.
* 30/01/96 1.6. Aggiunta la funzione "plcModProp", per ottenere i dati
*          rilevanti su di un modulo caricato dinamicamente.
* 31/01/96 1.7. Aggiunti comandi di controllo della statistica.
* 02/02/96 1.8. Aggiunte le funzioni di collegamento non bloccante al Plc :
*          "plcTryAttach(0)", "plcReady()". Sono da utilizzare nell'ordine
*          dato. Il successo della seconda autorizza il chiamante ad utilizzare
*          la "plcLinkStatics()".
* 13/02/96 1.9. La funzionalita` delle primitive aggiunte nella versione
*          precedente e` stata raccolta nella nuova funzione "plcTryOpen()".
*          Sono poi state aggiunte funzioni per la lettura multipla di aree
*          e la validazione di indirizzi.
* 21/03/96 1.10. Corretta la funzione "plcStClear"...che non andava.
* 10/06/96 1.11. Aggiunta la funzione "plcTraceAddr", che permette di
*          specificare un indirizzo qualunque per il trace.
* 22/06/96 1.12. Aggiunta la funzione "plcSafeCopyVar", per la copia di
*          variabili in maniera sicura, cioe` senza schianti, qualsiasi cosa
*          succeda...o quasi.
*          Aggiunta la funzione "dbQPLC", che dice se si sta usando qplc.
*          Aggiunta la funzione "dbFindQplcVar" che strova una variabile
*          condivisa di QPLC scomponendo un nome in stile isagraf in
*          parte base ed indici.
*          Aggiunta la funzione "dbGetQplcVarDescr", che trova indirizzo
*          e descrittore di una variabile di QPLC.
* 25/06/96 1.13. Corrette le immancabili sfighe aggiunte.
* 31/07/96 1.14. Corretto un buco nella ricerca dei nomi in stile QPLC :
*          si esplorava erroneamente la lista globale anziche` la lista
*          di hash. Inoltre e` stato corretta l'inversione delle stringhe
*          "TRUE" e "FALSE" nelle proprieta` dei nomi di qplc.
* 28/11/96 GG 1.15 Aggiunta la gestione delle variabili in RAM non volatile.
*             Tale gestione e` limitata alle funzioni che utilizzano la
*             primitive "dbGetQplcVarDescr", e cioe` "dbGetVar" e
*             "dbGetVarAddr". Non funzionano invece le varie "plcGetVar..."
*             "plcSerVar...", perche` la modifica richiesta comporterebbe
*             un rallentamento sensibile, senza che vi sia, al momento,
*             un reale bisogno della nuova prestazione. Bisogna inoltre
*             ricordare che queste funzioni sono in via di obsolescenza,
*             anche se il loro completo abbandono richiedera` molto tempo.
*             La gestione della RAM non volatile non e` a totale carico della
*             libreria, perche` la modialita` di aggancio al dispositivo
*             puo` variare parecchio da un'applicazione all'altra. Si e`
*             percio` deciso che il puntatore alla zona di RAM non volatile
*             del PLC (cioe` l'area nominata "NVRAM_LBL_PLC480" in "nvram.h"),
*             deve essere ottenuto dall'applicazione stessa, e comunicato
*             alla libreria con la nuova funzione "dbRegisterNvram". Se
*             questo non viene fatto, l'indirizzo associato a variabili
*             in RAM non volatile varra` NULL.
* 09/12/96 GG 1.16 Corretta un'imperfezione in plcModProp. La funzione tentava
*             di copiare alcuni byte dopo la fine del nome del modulo, con
*             schianti se il nome del modulo si trova nei pressi di indirizzi
*             inaccessibili.
* 11/12/96 GG 2.0 Modificata la dimensione del campo "size" nella struttura
*             "plcvar_t", per poter rappresentare oggetti di grandi dimensioni.
*             A causa di questa modifica, SI E` PERSA LA COMPATIBILITA` con
*             il passato. I programmi che volessero utilizzare la nuova
*             versione della libreria DEVONO essere ricompilati interamente,
*             non si possono cioe` mescolare oggetti compilati con la versione
*             precedente di questo file con oggetti compilati con questa.
* 16/12/96 GG 2.1 Aggiunte funzioni per il debug in tempo reale di QPLC :
*             plcCreateWatchList, plcDeleteWatchList, plcAddBreak,
*             plcEnableWatchList, plcDisableWatchList.
* 17/12/96 GG 2.2 Aggiunte le funzioni (qplc >= 1.3.3.8) plcClearAllWatches,
*             plcResetDebugEvents, plcLastDebugEventList, plcGetDebugEvent.
* 18/12/96 GG 2.3 Corretto uno schianto in plcLastDebugEventList.
* 03/07/97 GG 2.4 Aumentata la dimensione di default della shared memory,
*             in vista dell'abbandono definitivo del file "isarc".
*             Aggiunte le funzioni "plcLoadDebugHistory" e
*             "plcIndexDebugEventList", che servono a caricare ed esplorare
*             l'intero insieme degli eventi immagazzinati da qplc.
*             "plcDebugMemCopy" e` stata resa pubblica.
* 09/07/97 GG 2.5 Aggiunto un parametro a "plcLoadDebugHistory".
* 07/01/98 GG 2.6 Eliminato il controllo sul tipo per le variabili da
*             tracciare.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>

#include <grsy0def.h>
#include <grsy0ker.h>
#include <grsy0tst.h>

#include <hsort.h>
#include <port.h>

#define _USE_PLC_FUNC 1
#include <plcdb.h>
#include "plclink.h"
#include "plclink_p.h"

/*
#define strcmp _dbST_strcmp
#define NEW_strcmp
*/

#define PLC_TIMEOUT 500	/* Il kernel plc ha 500 ms di tempo per rispondere. */

