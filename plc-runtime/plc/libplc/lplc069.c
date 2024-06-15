/* lplc069.c */
#include "lplc.h"

/*
* Funzione "plcGetDebugEvent"
* ---------------------------
*
*  Questa funzione legge il prossimo evento di debug dal buffer, e lo
* copia in un'area allocata internamente dalla libreria.
*  Restituisce il puntatore all'evento letto, oppure NULL.
*/

brk_event * plcGetDebugEvent(void)
{
brk_event *ev;
unsigned char *p;

	if (! plc_get_event_status.buffer)
		return (brk_event *) NULL;

	if (plc_get_event_status.read_pos >= plc_get_event_status.event_end)
		return (brk_event *) NULL;

	ev = (brk_event *) (plc_get_event_status.buffer
	                    + plc_get_event_status.read_pos);
	plc_get_event_status.read_pos += ev -> be_size;

	return ev;
}

