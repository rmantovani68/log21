
/*
* @(#) ton.c 1.1 Wed Feb  5 12:52:51 MET 1997
*
*  Blocco funzionale "TON" a norme IEC1131-3.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
* 05/02/97 GG 1.1 Esagerava di un ciclo.
*/

#include <limits.h>

long plcPeriod(void);
unsigned long plcTime();
#ifdef LONG_LONG_TIME
unsigned long plcTimeH();
#endif

/*
(* Salita ritardata. *)

function_block TON
var_input
	IN : bool;
	PT : time;
end_var
var_output
	Q : bool;
	ET : time;
end_var
var
	in_old : bool;
	sts : time;
#ifdef LONG_LONG_TIME
	sts_h : udint;
#endif
end_var
end_function_block
*/

enum {
_TON_IN = 0,
_TON_PT,
_TON_Q = _TON_PT + sizeof(unsigned long),
_TON_ET,
_TON_IN_old = _TON_ET + sizeof(unsigned long),
_TON_sts
#ifdef LONG_LONG_TIME
,
_TON_sts_h = _TON_sts + sizeof(unsigned long)
#endif
};

int TON(register char * st)
{
	if (st[_TON_IN]) {
		if (! st[_TON_IN_old]) {
			*(unsigned long *)(st + _TON_sts) = plcTime();
#ifdef LONG_LONG_TIME
			*(unsigned long *)(st + _TON_sts_h) = plcTimeH();
#endif
			st[_TON_IN_old] = 1;
		}
#ifdef LONG_LONG_TIME
		{
		unsigned long tl,th;

			tl = plcTime();
			th = plcTimeH() - *(unsigned long *)(st + _TON_sts_h);
			if (tl < *(unsigned long *)(st + _TON_sts))
				--th;
			tl -= *(unsigned long *)(st + _TON_sts);
			if (th || tl >= *(unsigned long *)(st + _TON_PT)) {
				*(unsigned long *)(st + _TON_ET)
				 = *(unsigned long *)(st + _TON_PT);
				return (st[_TON_Q] = 1);
			}
			else {
				*(unsigned long *)(st + _TON_ET) = tl;
				return (st[_TON_Q] = 0);
			}
		}
#else
		if (*(unsigned long *)(st + _TON_ET)
		 >= *(unsigned long *)(st + _TON_PT)) {
			return (st[_TON_Q] = 1);
		}
		else {
			*(unsigned long *)(st + _TON_ET)
			  = plcTime() - *(unsigned long *)(st + _TON_sts);

			if (*(unsigned long *)(st + _TON_ET)
			 >= *(unsigned long *)(st + _TON_PT)) {
				return (st[_TON_Q] = 1);
			}
			else {
				return (st[_TON_Q] = 0);
			}
		}
#endif
	}
	else {
		st[_TON_IN_old] = 0;
		*(unsigned long *)(st + _TON_ET) = 0;
		return (st[_TON_Q] = 0);
	}
}

