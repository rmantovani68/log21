/* lplc039.c */
#include "lplc.h"

#define COEFF   20                /* 20 loops of user time out/20 */

static int plcLock(unsigned char * flag)
{
  register int rv;

  /* Si tratta di semafori finti. E per simularli ci vogliono DUE chiamate
 alle primitive di semaforo vere. O tempora !... */

  plcPSem (SEM_FLG);
  rv = *flag;
  if (!rv)
    *flag = 1;
  plcVSem (SEM_FLG);
  return rv;
}

static void plcUnlock(unsigned char * flag)
{
  plcPSem (SEM_FLG);
  *flag = 0;
  plcVSem (SEM_FLG);
}

/*
* Funzione "plcTell"
* ------------------
*
* Questa funzione realizza il colloquio coll'esecutore PLC Isagraf.
* Utilizza lo stesso protocollo del programma IsaTst.
* Invia il messaggio "msg" (MAX_QUESTION byte alpiu`) ed attende la risposta
* in "answ" per "time_out" millisecondi.
* Il valore di ritorno e` il numero di byte contenuti nella risposta,
* oppure 0 in caso di timeout.
*
* NOTE : Il messaggio e` formato da un byte di comando (i valori possibili
*  si trovano in "grsy0tst.h") seguito da eventuali parametri.
*   La risposta e` costituita da almeno tre byte :
*    0 : il codice di comando impostato nel messaggio trasmesso,
*    1,2 : un codice di errore, memorizzato come short.
*    byte successivi : eventuali dati aggiuntivi.
*/

int plcTell (unsigned char * msg, unsigned char * answ, long timeout)
{
int i,rv;

/* Il periodo di timeout e` scomposto in COEFF passi. */

  timeout = (timeout / COEFF) * 1000; /* Converte in microsecondi per usleep. */

/* Attende di avere accesso alla risorsa. */

  for (i = COEFF + 1; plcLock(&(SYST->client)) && --i; )
      usleep (timeout);
  if (! i)
    return 0;	/* ...sfiga... */

/* Copia il messaggio nell'apposita area del kernel plc. */

  SYST -> question_ker = (unsigned char *) MRTRANS(&(SYST -> buffer[0]));
  SYST -> answer_ker = (unsigned char *) MRTRANS(ANSW);
  for (i = 0; i < MAX_QUESTION; ++i)
    SYST -> buffer[i] = msg[i];
  SYST -> question_ready = 1;

/* Attende la risposta. */

  for (i = COEFF + 1; SYST -> question_ready && --i; )
      usleep(timeout);

/* C'e` una condizione critica ineliminabile : il Plc potrebbe eseguire il
 comando prima che il flag venga azzerato per timeout. La situzione si
 risolverebbe se fossero utilizzati due flag distinti. Ma non e` cosi`... */

  if (i) {

/* Copia la risposta nella destinazione. */

    rv = SYST -> answer_size;
    for (i = 0; i < rv; ++i)
      answ[i] = ANSW[i];
  }
  else {
    SYST -> question_ready = 0;
    rv = 0;	/* ...sfiga... */
  }

/* Libera la risorsa. */

  plcUnlock(&(SYST -> client));

  return rv;
}

