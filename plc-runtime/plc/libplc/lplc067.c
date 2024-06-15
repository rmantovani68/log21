/* lplc067.c */
#include "lplc.h"

/*
* Questa funzione e` usata internamente per copiare dati dall buffer circolare
* del debugger.
*/

void plcDebugMemCopy(char *dest, unsigned long src, unsigned long n)
{
unsigned long n1,n2;

	if (src + n <= plc_get_event_status.size) {
		n1 = n;
		n2 = 0;
	}
	else {
		n1 = plc_get_event_status.size - src;
		n2 = n - n1;
	}
	if (n1)
		memcpy(dest,plc_get_event_status.deb_buffer + src,(int)n1);
	if (n2)
		memcpy(dest + n1,plc_get_event_status.deb_buffer,(int)n2);
}

