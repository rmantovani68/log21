
/*
* @(#) ctu.c 1.0 Tue Oct 15 16:06:11 MET 1996
*
*  Blocco funzionale "CTU" a norme IEC1131-3.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
*/

#include <limits.h>

long plcPeriod(void);
unsigned long plcTime();

/*
(* Up counter *)

function_block CTU
var_input
	CU : bool;
	R : bool;
	PV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var_output
	Q : bool;
	CV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var
	cu_old : bool;
end_var
end_function_block
*/

enum {
_CTU_CU = 0,
_CTU_R,
_CTU_PV,
_CTU_Q = _CTU_PV + sizeof(long),
_CTU_CV,
_CTU_CU_old = _CTU_CV + sizeof(long),
};

int CTU(register char * st)
{
register char cu;

	cu = (st[_CTU_CU] && !st[_CTU_CU_old]);
	st[_CTU_CU_old] = st[_CTU_CU];

	if (st[_CTU_R])
		*(long *)(st + _CTU_CV) = 0;
	else if (cu && *(long *)(st + _CTU_CV) < LONG_MAX)
		++*(long *)(st + _CTU_CV);
	return st[_CTU_Q] = *(long *)(st + _CTU_CV) >= *(long *)(st + _CTU_PV);
}

