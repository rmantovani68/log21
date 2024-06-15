
/*
* Esempio d'uso del driver IOS.
*
* Questo programma visualizza il contenuto del primo 256 byte
* della scheda IOS 0, e fa lampeggiare le uscite 0-7 del modulo seriale
* 0 collegato.
*
* Si suppone che esista il dispositivo "/dev/ios0", leggibile e
* scrivibile da tutti.
* Si suppone inoltre che il modulo remoto possieda uno o tre gruppi
* di uscite.
*/

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

main()
{
int fd;
char * addr;
int i,j,n;

/* Apre il dispositivo. */

	fd = open("/dev/ios0",O_RDWR);
	if (fd < 0) {
		perror("ios0");
		exit(1);
	}

/* Acquisisce l'accesso alla memoria della scheda IOS. */

	addr = (char *) mmap((void *)0, 0x4000, PROT_READ | PROT_WRITE,
				MAP_SHARED, fd, 0);
	if (addr == (char *) -1) {
		perror("mmap");
		exit(1);
	}

/* Si puo` chiudere il dispositivo. La memoria della IOS restera`
 comunque visibile al processo. */

	close(fd);

/* Si visualizzano i primi 256 byte della scheda. */

	for (i = 0; i < 256; i += 16) {
		printf("0x%04x  ",i);
		for (j = 0; j < 16; ++j)
			printf(" %02x",(unsigned char)addr[i+j+0x200]);
		printf("\n");
	}

/* Ciclo infinito che fa lampeggiare le uscite 0-7. */

	n = 1;

	for (;;) {
		addr[0] = n;
		addr[1] = n;
		addr[2] = n;
		n <<= 1;
		if (n == 256)
			n = 1;
		usleep(300000);
	}
	
}

