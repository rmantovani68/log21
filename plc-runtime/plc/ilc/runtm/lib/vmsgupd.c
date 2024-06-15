
/*
* @(#) vmsgupd.c 1.1 Tue Aug 26 13:01:53 MET DST 1997
*
*  Codifica dei blocchi funzionali CNi per emissione di messaggi
* codificati come interi completati da un codice numerico e con la
* possibilita` di sovrascrittura del messaggio. Ci vuole quote 2.6.0.15. 
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 11/06/97 GG 1.0 Prima stesura.
* 26/08/97 GG 1.1 Corretto... un commento.
*/

/*
(************)
(* MESSAGGI *)
(************)
*/

/*
(* Emissione di un messaggio *)

function_block VMSGUPD
var_input
	CODE : dint;
	VALUE : dint;
	ENABLE : bool;
	RESET : bool;
end_var
var
	enable_old : bool;
	reset_old : bool;
	code_old : dint;
	value_old : dint;
end_var
end_function_block
*/

struct plc_vmsgupd_t {
	long code __attribute__((packed));
	long value __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));

	char enable_old __attribute__((packed));
	char reset_old __attribute__((packed));
	long code_old __attribute__((packed));
	long value_old __attribute__((packed));
};

long VMSGUPD(register struct plc_vmsgupd_t * st)
{
register long c,v;

	c = st -> code;
	if (st -> reset && !st -> reset_old) {
		st -> enable_old = 0;
	}
	if (st -> enable) {
		if (((v = st -> value) != st -> value_old
		     || !st -> enable_old
		     || c != st -> code_old)
		    && c != 0) {
		/* Convenzionalmente, i messaggi sottoposti al meccanismo
		 di associazione codice-stringa sono identificati da un testo
		 avente come primo carattere ^A, seguito dalla stringa di
		 estensione. In questo caso la stringa e` numerica. */
			plcUpdateMessage((int)(c),"\001%d",v);
		}
	}
	else {
		if (st -> enable_old && c != 0)
			plcDelmess(c);
	}

	st -> code_old = st -> code;
	st -> value_old = st -> value;
	st -> enable_old = st -> enable;
	st -> reset_old = st -> reset;

	return c;
}

