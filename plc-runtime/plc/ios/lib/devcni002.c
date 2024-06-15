/* devcni002.c */
#include "devcni.h"

#ifdef Lynx
#include <mmap.h>

/*
* Funzione "munmap"
* -----------------
*
*  Questa funzione sgancia lo spazio virtuale del processo chiamante
* dall'area condivisa "addr" ottenuta con "mmap". La funzione vale sempre
* 0, perche` le primitive utilizzate non segnalano errori se "addr" non
* contiene un valore accettabile.
*  Il parametro "len" e` ignorato.
*/

int munmap(char * addr, unsigned int len)
{
	smem_create("",addr,len,SM_DETACH);
	return 0;
}

#endif /* Lynx */
