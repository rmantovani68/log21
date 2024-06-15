/* devcni005.c */
#include "devcni.h"

#include <stdio.h>
#include <fcntl.h>
#ifdef Lynx
#include <mmap.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <ios.h>
#include <hwcni.h>

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

int iosOpen(int n, unsigned char **base, unsigned char **cpu, unsigned char **nvram, int *fd)
{
int i,file;
char *addr;
struct iosboard info;

	if (_iosOpen(n,&file,&info) < 0)
		return -1;
	if (_iosMap(file,fd,&addr,0,0) < 0)
		return -1;

	if (base)
		*base = (unsigned char *) addr;

	if (cpu) {
		for (i = 0; i < MAX_IOS_CPU; ++i) {
			cpu[i] = info.cpu_size[i] ?
						(unsigned char *) (addr + info.cpu_off[i]) :
						(unsigned char *) 0;
		}
	}

	if (nvram) {
		*nvram = info.nvram_size ?
					(unsigned char *)(addr + info.nvram_off) :
					(unsigned char *) 0;
	}

	return 0;
}

