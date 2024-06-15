/*
* @(#) libdevcni.c 1.6 Sun May 28 05:06:14 PM CEST 2023
*
* ??/??/?? GG 1.0, 1.1 versioni preliminari.
* ??/??/?? GG 1.2 Prima versione distribuita.
* 15/09/95 GG 1.3 Aggiunta la funzione iosOpenGeneral.
* 25/10/95 GG 1.4 Non documentata !
* 22/11/96 GG 1.5 Modificata la mmap per permettere la gestione di offset
*             e lunghezza. Richiede (solo per completezza) una
*             modifica a kutils nel kernel.
* 28-05-23 RM 1.6 eliminate le ref a ios 
*/

#include <errno.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <ios.h>
#include <hwcni.h>
#include <nvram.h>

int iosOpen(int n, unsigned char **base, unsigned char **cpu, unsigned char **nvram, int *fd)
{
	return -1;
}

int iosOpenGeneral(int n, struct ios_info_t *buf, int *fd)
{
	return -1;
}

void iosRestart(unsigned char *p)
{
}

int AttachNvram(char *name, nvram_t *area, void *base, int ios)
{
	return -1;
}

