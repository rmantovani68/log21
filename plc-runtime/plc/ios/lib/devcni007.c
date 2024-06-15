/* devcni007.c */
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
* Funzione "iosRestart"
* ---------------------
*
*  Questa funzione scrive la firma di restart sul controller di indirizzo
* dato.
*  Se l'indirizzo e` NULL, la funzione esce senza conseguenze.
*/

static unsigned char magic[] = IOS_MAGIC;

int iosRestart(unsigned char *p)
{
int i;

	if (p) {
		p += IOS_PASSWD;
		for (i = 0; i < sizeof(magic); ++i)
			p[i] = magic[i];
	}
}

