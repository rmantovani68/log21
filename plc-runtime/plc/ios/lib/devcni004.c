/* devcni004.c */
#include "devcni.h"

#include <stdio.h>
#include <fcntl.h>
#ifdef Lynx
#include <mmap.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <hwcni.h>


int _iosMap(int file, int *fd, char **pa, int len, int off)
{
char *addr;
char name[20];

	if (len == 0)
	  	len = IOS_SHMSIZE;
	addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, file, off);
	if (addr == (char *) -1) {
		close(file);
		return -1;
	}

	*pa = addr;

	if (fd)
		*fd = file;
	else
		close(file);

	return 0;
}

