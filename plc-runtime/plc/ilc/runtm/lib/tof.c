
/*
* @(#) tof.c 1.1 Thu Feb  6 19:12:48 MET 1997
*
*  Blocco funzionale "TOF" a norme IEC1131-3.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
* 06/02/97 GG 1.1 Esagerava di un ciclo.
*/

#include <limits.h>

long plcPeriod(void);
unsigned long plcTime();
#ifdef LONG_LONG_TIME
unsigned long plcTimeH();
#endif

/*
(* Discesa ritardata. *)

function_block TOF
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
_TOF_IN = 0,
_TOF_PT,
_TOF_Q = _TOF_PT + sizeof(unsigned long),
_TOF_ET,
_TOF_IN_old = _TOF_ET + sizeof(unsigned long),
_TOF_sts
#ifdef LONG_LONG_TIME
,
_TOF_sts_h = _TOF_sts + sizeof(unsigned long)
#endif
};

int TOF(register char * st)
{
	if (! st[_TOF_IN]) {
		if (st[_TOF_IN_old]) {
			*(unsigned long *)(st + _TOF_sts) = plcTime();
#ifdef LONG_LONG_TIME
			*(unsigned long *)(st + _TOF_sts_h) = plcTimeH();
#endif
			st[_TOF_IN_old] = 0;
		}
#ifdef LONG_LONG_TIME
		{
		unsigned long tl,th;

			tl = plcTime();
			th = plcTimeH() - *(unsigned long *)(st + _TOF_sts_h);
			if (tl < *(unsigned long *)(st + _TOF_sts))
				--th;
			tl -= *(unsigned long *)(st + _TOF_sts);
			if (th || tl >= *(unsigned long *)(st + _TOF_PT)) {
				*(unsigned long *)(st + _TOF_ET)
				 = *(unsigned long *)(st + _TOF_PT);
				return (st[_TOF_Q] = 0);
			}
			else {
				*(unsigned long *)(st + _TOF_ET) = tl;
				return (st[_TOF_Q] = 1);
			}
		}
#else
		if (*(unsigned long *)(st + _TOF_ET)
		 >= *(unsigned long *)(st + _TOF_PT)) {
			return (st[_TOF_Q] = 0);
		}
		else if (st[_TOF_Q]) {
			*(unsigned long *)(st + _TOF_ET)
			  = plcTime() - *(unsigned long *)(st + _TOF_sts);
			if (*(unsigned long *)(st + _TOF_ET)
			 >= *(unsigned long *)(st + _TOF_PT)) {
				return (st[_TOF_Q] = 0);
			}
			return 1;
		}
		else
			return 0;
#endif
	}
	else {
		st[_TOF_IN_old] = 1;
		*(unsigned long *)(st + _TOF_ET) = 0;
		return (st[_TOF_Q] = 1);
	}
}

