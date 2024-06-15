

/*
* @(#) ctd.c 1.0 Tue Oct 15 16:06:11 MET 1996
*
*  Blocco funzionale "CTD" a norme IEC1131-3.
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
(* Down counter *)

function_block CTD
var_input
	CD : bool;
	L : bool;	(* Lo standard lo vorrebbe chiamare "LD".
			 Ma "LD" non e` una parola riservata ?
			 C'e` un discorso strano in merito nella
			 revisione alle norme del 17/7/96...  *)
	PV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var_output
	Q : bool;
	CV : dint; (* Lo standard parlerebbe di "int"... *)
end_var
var
	cd_old : bool;
end_var
end_function_block
*/

enum {
_CTD_CD = 0,
_CTD_L,
_CTD_PV,
_CTD_Q = _CTD_PV + sizeof(long),
_CTD_CV,
_CTD_CD_old = _CTD_CV + sizeof(long),
};

int CTD(register char * st)
{
register char cd;

	cd = (st[_CTD_CD] && !st[_CTD_CD_old]);
	st[_CTD_CD_old] = st[_CTD_CD];

	if (st[_CTD_L])
		*(long *)(st + _CTD_CV) = *(long *)(st + _CTD_PV);
	else if (cd && *(long *)(st + _CTD_CV) > LONG_MIN)
		--*(long *)(st + _CTD_CV);
	return st[_CTD_Q] = *(long *)(st + _CTD_CV) <= 0;
}

