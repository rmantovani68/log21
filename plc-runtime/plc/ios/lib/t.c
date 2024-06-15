

#include <stdio.h>
#include <hwcni.h>

main()
{
int fd;
char *base;
char *nvram;
char *cpu[32];
struct ios_info_t iii;
int i;
	if (iosOpenGeneral(0,&iii,&fd) < 0) {
		exit(1);
	}
	 
	printf("Versione IOS : ");
	for (i = 0x225; i < 0x225+4; ++i)
		printf("%c",iii.base[i]);

	exit(0);
}

