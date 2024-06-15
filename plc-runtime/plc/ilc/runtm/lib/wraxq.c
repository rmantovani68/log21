

/*
* @(#) wraxq.c 1.0 Thu Mar  2 11:33:20 MET 2000
*
*  Funzione per la lettura stato di un asse.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 07/07/98 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>

struct wraxq_t {
	float val __attribute__((packed));
	long op __attribute__((packed));
	long index __attribute__((packed));
	long p1 __attribute__((packed));
	long p2 __attribute__((packed));
} _f_WRAXQ = {
	0.0,
	0,
	0,
	0,
	0,
};

long WRAXQ(void)
{
union { float f; double d; long l; } dato;

	if (CPAXSHM) {
	/* Questi sono codici pericolosi (e insensati). */
		if (_f_WRAXQ.op == SAX_INIT_ANALOGICA
		 || _f_WRAXQ.op == SAX_RELEASE_ANALOGICA) {
			return 0L;
		}
		if (_f_WRAXQ.op == SAX_CONTAMETRI) {
		/* Il contamentri e` espresso in double. */
			dato.d = (double)_f_WRAXQ.val;
		}
		else {
			dato.f = _f_WRAXQ.val;
		}
		if (write_parm(CPAXSHM,(int)_f_WRAXQ.op,_f_WRAXQ.index,
		          (int)_f_WRAXQ.p1,(int)_f_WRAXQ.p2,(int *)&dato)
		 == SAX_TUTTO_OK) {
			return 1L; /* Tutto OK */
		}
	}
	return 0L;
}

