
/*
* @(#) pass1.h 2.1 Fri Nov 29 11:36:28 MET 1996
* @(#) Rif. Guerrini
*
* --/--/-- 1.0 Prima stesura, sperimentale.
* 02/07/96 2.0 Eliminati i parametri di tutte le azioni.
* 29/11/96 GG 2.1 Modifiche per la lettura di costanti stringa.
*/

#ifndef _PASS1_H_

/* Contesto globale. Contiene tutti i simboli globali definiti nei
 sorgenti esaminati. */

extern context_t *global_context;

/* Contesto globale ristretto al file corrente. Contiene tutti i simboli
 globali definiti nel sorgenti corrente. Serve a generare l'header
 corrispondente al sorgente. */

extern context_t *curr_file_context;

/* Contesto locale. Contiene tutti i simboli locali
 definiti nel sorgenti corrente. Corrisponde allo scope delle variabili
 "static" del C. */

extern context_t *local_context;

/* Contesto "funzione". Contiene tutti i simboli locali
 definiti all'interno di un blocco procedurale. Corrisponde allo scope
 di funzione del C. */

extern context_t *fb_context;

/* Contesto "stringhe". E` utilizzato piu` che altro come serbatoio. */

extern context_t *string_context;


/* Inizializzatore a freddo dell'esecutore. */

void p1_init(void);

/* Inizializzatore a caldo. */

int p1_start(file_action_t *act);

#define _PASS1_H_

#endif

