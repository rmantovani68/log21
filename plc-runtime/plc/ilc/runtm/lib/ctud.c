

/*
* @(#) ctud.c 1.1 Wed May 20 20:58:18 MET DST 1998
*
*  Blocco funzionale "CTUD" a norme IEC1131-3.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
* 20/05/98 GG 1.1 Incredibile ! CD e CU erano invertiti !
*/

#include <limits.h>

long plcPeriod(void);
unsigned long plcTime();

/*
(* Up-down counter *)

function_block CTUD
var_input
	CU : bool;
	CD : bool;
	R : bool;
	L : bool; (* Vedi sopra. *)
	PV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var_output
	QU : bool;
	QD : bool;
	CV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var
	cu_old : bool;
	cd_old : bool;
end_var
end_function_block
*/

enum {
_CTUD_CU = 0,
_CTUD_CD,
_CTUD_R,
_CTUD_L,
_CTUD_PV,
_CTUD_QU = _CTUD_PV + sizeof(long),
_CTUD_QD,
_CTUD_CV,
_CTUD_CU_old = _CTUD_CV + sizeof(long),
_CTUD_CD_old
};

int CTUD(register char * st)
{
register char cu,cd;

	cu = (st[_CTUD_CU] && !st[_CTUD_CU_old]);
	st[_CTUD_CU_old] = st[_CTUD_CU];

	cd = (st[_CTUD_CD] && !st[_CTUD_CD_old]);
	st[_CTUD_CD_old] = st[_CTUD_CD];

	if (st[_CTUD_R])
		*(long *)(st + _CTUD_CV) = 0;
	else if (st[_CTUD_L])
		*(long *)(st + _CTUD_CV) = *(long *)(st + _CTUD_PV);
	else if (cu && *(long *)(st + _CTUD_CV) < LONG_MAX)
		++*(long *)(st + _CTUD_CV);
	else if (cd && *(long *)(st + _CTUD_CV) > LONG_MIN)
		--*(long *)(st + _CTUD_CV);
	st[_CTUD_QU] = *(long *)(st + _CTUD_CV) >= *(long *)(st + _CTUD_PV);
	return st[_CTUD_QD] = *(long *)(st + _CTUD_CV) <= 0;
}

