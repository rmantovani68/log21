/* devcni003.c */
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


int _iosOpen(int n, int *fd, struct iosboard *inf)
{
int file;
char name[20];

	sprintf(name,"/dev/ios%d",n);

	file = open(name,O_RDWR);
	if (file < 0)
		return -1;

	if (ioctl(file, IOSGET, inf) < 0) {
		close(file);
		return -1;
	}

	*fd = file;

	return 0;
}

