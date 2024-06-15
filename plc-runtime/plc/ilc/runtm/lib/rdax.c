

/*
* @(#) rdax.c 1.0 Wed Jan  7 12:05:56 MET 1998
*
*  Funzione per la lettura della quota corrente di un asse.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 07/07/98 GG 1.0 Prima stesura.
*/

#include <stdio.h>
#include <plcapi.h>

struct rdax_t {
	long index __attribute__((packed));
} _f_RDAX = {
	0,
};

long RDAX(void)
{
union { float f; long l; } quota;

	quota.f = 0.0;
	if (CPAXSHM)
		read_parm(CPAXSHM,SAX_QUOTA_REALE,_f_RDAX.index,
		          -1,-1,(int *)&quota.f);
	return quota.l;
}

