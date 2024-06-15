/*
* @(#) hwcni.h 1.0 Wed May 10 14:42:57 WET 1995
*
* Funzioni di accesso ai dispositivi CNI.
*
* 10/05/95 GG 1.0 : Prima versione.
* 15/09/95 GG 1.1 : Aggiunta la funzione iosOpenGeneral ed i tipo associati.
*/

#ifndef _HWCNI_H

#include <ios.h>

/* Struttura utilizzata dalla funzione "iosOpenGeneral". */

struct ios_info_t {
	unsigned char *base;            /* Indirizzo dell'area condivisa. */
	int board_size;	                /* Dimensione dell'area condivisa. */
	unsigned char *cpu[MAX_IOS_CPU];/* Indirizzi dei controllori. */
	int cpu_size[MAX_IOS_CPU];      /* Dimensioni delle aree dei controllori */
	unsigned char *nvram;           /* Indirizzo della RAM non volatile. */
	int nvram_size;                 /* Dimensione della RAM non volatile. */
	int e2_size;                    /* Dimensione dell'E2 seriale. */
};

/*
* Funzione "iosOpen"
* ------------------
*
*  Apre il dispositivo IOS indicato, lo mappa in memoria, calcola i puntatori
* alle sue parti e, se richiesto, lo chiude.
*  "n" e` l'indice del dispositivo IOS (numero di scheda), "base" e`
* l'indirizzo di un puntatore che conterra` la base dell'area condivisa
* associata al dispositivo, "cpu" e` l'indirizzo
* di un vettore di puntatori (di almeno due elementi), che all'uscita
* conterra` gli indirizzi dei due controller (o NULL in caso di controller
* assente), "nvram" e` il puntatore ad un variabile che conterra` l'indirizzo
* della RAM non volatile (NULL se assente), infine "fd" e` il puntatore al
* file descriptor associato al dispositivo. Se e` posto a NULL, il file
* descriptor viene automaticamente chiuso all'uscita dalla funzione, altri-
* menti resta aperto in lettura e scrittura.
*  Tutti i parametri di tipo puntatore possono essere posti a NULL, se il
* dato corrispondente non interessa.
*  La funzione vale -1 in caso di errore, altrimenti 0.
*
* NOTE :
*  - La dimensione del vettore "cpu" deve essere almeno uguale a
* "MAX_IOS_CPU" (vedi "ios.h").
*  - In caso di errore, nessuna delle aree puntate e` alterata.
*/

int iosOpen(int n, unsigned char **base, unsigned char **cpu, unsigned char **nvram, int *fd);
/*
* Funzione "iosOpenGeneral"
* -------------------------
*
*  Apre il dispositivo IOS indicato, lo mappa in memoria, calcola i puntatori
* alle sue parti e, se richiesto, lo chiude. A differenza della "iosOpen",
* le informazioni sono registrate in una struttura apposita ("*buf"),
* che include anche campi contenenti le dimensioni delle varie componenti.
*  "n" e` l'indice del dispositivo IOS (numero di scheda), "buf" e` il
* puntatore alla struttura di tipo "ios_info_t" che ospitera` i dati, infine
* "fd" e` il puntatore al file descriptor associato al dispositivo.
* Se e` posto a NULL, il file descriptor viene automaticamente chiuso
* all'uscita dalla funzione, altrimenti resta aperto in lettura e scrittura.
*  La funzione vale -1 in caso di errore, altrimenti 0.
*/

int iosOpenGeneral(int n, struct ios_info_t *buf, int *fd);
/*
* Funzione "iosRestart"
* ---------------------
*
*  Questa funzione scrive la firma di restart sul controller di indirizzo
* dato.
*  Se l'indirizzo e` NULL, la funzione esce senza conseguenze.
*/

int iosRestart(unsigned char *p);

#define _HWCNI_H

#endif

