
/*
* @(#) tp.c 1.1 Fri Feb  7 10:36:42 MET 1997
*
*  Blocco funzionale "TP" a norme IEC1131-3 (quasi. In realta`
* il blocco funzionale si chama qui "TPULSE", perche` "TP" e` un segnale
* di scambio...).
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 15/10/96 GG 1.0 Prima stesura.
* 07/02/97 GG 1.1 Micro-ottimizzazione.
*/

#include <limits.h>

long plcPeriod(void);
unsigned long plcTime();

/*
(* Generatore d'impulso (monostabile, piu` o meno). *)

function_block TPULSE (* Poveri noi ! Abbiamo gia`
                         un segnale di scambio TP ! *)
var_input
	IN : bool;
	PT : time;
end_var
var_output
	Q : bool;
	ET : time;
end_var
var
	sts : time;
end_var
end_function_block
*/

enum {
_TP_IN = 0,
_TP_PT,
_TP_Q = _TP_PT + sizeof(unsigned long),
_TP_ET,
_TP_sts = _TP_ET + sizeof(unsigned long),
};

int TPULSE(register char * st)
{
	if (st[_TP_Q]) {
		*(unsigned long *)(st + _TP_ET)
		  = plcTime() - *(unsigned long *)(st + _TP_sts);
	}

	if (*(unsigned long *)(st + _TP_ET)
	 >= *(unsigned long *)(st + _TP_PT)) {
		st[_TP_Q] = 0;
		if (! st[_TP_IN])
			*(unsigned long *)(st + _TP_ET) = 0;
		return 0;
	}

	if (st[_TP_Q]) {
		return 1;
	}

	if (st[_TP_IN]) {
		st[_TP_Q] = 1;
		*(unsigned long *)(st + _TP_sts) = plcTime();
		*(unsigned long *)(st + _TP_ET) = 0;
		return 1;
	}

	return 0;
}

