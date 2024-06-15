/* lplc014.c */
#include "lplc.h"

/*
* Funzione "dbRegisterNvram"
* --------------------------
*
*  Questa funzione registra come puntatore alla base della RAM non volatile
* del PLC l'indirizzo dato. Tale indirizzo deve essere ottenuto utilizzando
* la funzione "AttachNvram" (in "libdevcni.a") con "NVRAM_LBL_PLC480" come
* primo parametro (le definizioni utili si trovano in "nvram.h"). Dopo
* la chiamata di questa funzione, le "dbGet..." potranno dare risultati
* corretti anche se applicate a variabili in RAM non volatile.
*/

#if _NO_PROTO
void dbRegisterNvram(nvram) void * nvram;
#else
void dbRegisterNvram(void * nvram)
#endif
{
	_plclib_nvram = nvram;
}

