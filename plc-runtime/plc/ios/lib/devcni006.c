/* devcni006.c */
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

int iosOpenGeneral(int n, struct ios_info_t *buf, int *fd)
{
int i,file;
char *addr;
struct iosboard info;

	if (_iosOpen(n,&file,&info) < 0)
		return -1;
	if (_iosMap(file,fd,&addr,0,0) < 0)
		return -1;

	buf -> base = (unsigned char *) addr;
	buf -> board_size = IOS_SHMSIZE;

	for (i = 0; i < MAX_IOS_CPU; ++i) {
		buf -> cpu[i] = info.cpu_size[i] ?
		                (unsigned char *) (addr + info.cpu_off[i]) :
		                (unsigned char *) 0;
		buf -> cpu_size[i] = info.cpu_size[i];
	}

	buf -> nvram = info.nvram_size ?
	               (unsigned char *)(addr + info.nvram_off) :
	               (unsigned char *) 0;
	buf -> nvram_size = info.nvram_size;

	buf -> e2_size = info.e2_size;

	return 0;
}

