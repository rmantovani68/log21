/*
* iosver.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>

#ifdef HAS_IOS_CNI
	#include <hwcni.h>
#endif

#ifdef DOCUMENTAZIONE
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

#ifdef _NO_PROTO
int iosOpen();
#else
int iosOpen(int n,
			unsigned char **base,
			unsigned char **cpu,
			unsigned char **nvram,
			int *fd);
#endif

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

#ifdef _NO_PROTO
int iosOpenGeneral();
#else
int iosOpenGeneral(int n,
			struct ios_info_t *buf,
			int *fd);
#endif

#endif	/* DOCUMENTAZIONE */

int main(int argc, char **argv) 
{
#ifdef HAS_IOS_CNI
	struct ios_info_t IOSInfo;
	int fdIOS=0xff;	/* NON NULL VALUE */
	int nRC;
	int i;

	nRC=iosOpenGeneral(0,&IOSInfo,&fdIOS);
	printf("Versione IOS : ");
	for (i = 0x225; i < 0x225+4; ++i)
		printf("%c",IOSInfo.base[i]);
#else
	printf("IOS non supportata\n");
#endif
	printf("\n");
	

	exit(0);
}

