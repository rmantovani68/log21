/*
* @(#) libh.h 1.2 Tue Dec 17 16:21:23 MET 1996
*
* Questo file contiene definizioni necessarie per la libreria
* di accesso al Plc.
*
* 06/12/96 GG Prima versione.
* 12/12/96 GG Aggiunti i comandi per la definizione di breakpoint.
* 17/12/96 GG Aggiunto il comando R_CLEARALL_WATCH.
*/

#ifndef _LIBH_H_

/*
* Codici di comando riconosciuti dal debugger "vecchio stile".
*/

#define R_TRACE_START 160	/* Lancia il trace. */
#define R_TRACE_STOP 161	/* Interrompe il trace. */
#define R_TRACE_STATUS 162	/* Riporta lo stato del trace. */
#define R_TRACE_LOAD 163	/* Copia una parte del buffer di trace. */
#define R_TRACE_ADD 164		/* Aggiunge una variabile alla lista. */
#define R_TRACE_CLEAR 165	/* Vuota la lista di trace. */
#define R_TRACE_SIGNAL 166	/* Segnala il completamento di una copia. */
#define R_TRACE_SETBUF 167	/* Imposta la dimensione dei suttobuffer. */

#define R_DYNLD_LOAD 170	/* Carica dinamicamente un modulo oggetto. */
#define R_DYNLD_UNLOAD 171	/* Scarica dinamicamente un modulo oggetto. */
#define R_DYNLD_STATUS 172	/* Stato del caricamento dinamico. */
#define R_DYNLD_PROP 175	/* Caratteristiche di un modulo. */

#define R_KERMEM_COPY 173	/* Copia blocco di memoria PLC. */
#define R_KERMEM_MCHK 179	/* Convalida indirizzi per multiletture. */
#define R_KERMEM_MCOPY 180	/* Esegue una lettura multipla. */

#define R_GET_TCYSTAT 174	/* Statistica del tempo di ciclo (12 byte). */
#define R_IF_ENSTAT 176		/* Stato della statistica. */
#define R_START_STAT 177	/* Abilitazione-disabilitazione statistica. */
#define R_CLEAR_STAT 181	/* Riazzeramento della statistica. */

#define R_GET_EXENAME 178	/* Fornisce il path completo dell'eseguibile. */

#define R_CREATE_WATCH 182	/* Crea una lista di watch (ottiene l'ID). */
#define R_DELETE_WATCH 183	/* Cancella una lista di watch dato l'ID. */
#define R_ADD_BREAK 184		/* Aggiunge un punto di break al watch. */
#define R_ENABLE_WATCH 185	/* Abilita la lista di watch dato l'ID. */
#define R_DISABLE_WATCH 186	/* Disabilita la lista di watch dato l'ID. */
#define R_CLEARALL_WATCH 187	/* Reinizializza il sistema di watch. */

#define _LIBH_H_

#endif


