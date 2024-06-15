
/*
* @(#) msgup.c 1.0 Wed Jun 11 15:22:41 MET DST 1997
*
*  Codifica dei blocchi funzionali CNi per emissione di messaggi
* codificati come interi completati da un codice numerico e con la
* possibilita` di sovrascrittura del messaggio. Ci vuole quote 2.6.0.15. 
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 11/06/97 GG 1.0 Prima stesura.
*/

/*
(************)
(* MESSAGGI *)
(************)
*/

/*
(* Emissione di un messaggio *)

function_block MSGUPD
var_input
	CODE : dint;
	VALUE : dint;
end_var
var
	code_old : dint;
	value_old : dint;
end_var
end_function_block
*/

struct plc_simple_msgupd_t {
	long code __attribute__((packed));
	long value __attribute__((packed));

	long code_old __attribute__((packed));
	long value_old __attribute__((packed));
};

long MSGUPD(register struct plc_simple_msgupd_t * st)
{
register long c,v;

	c = st -> code;
	if (( (v = st -> value) != st -> value_old
	     || c != st -> code_old)
	    && c != 0) {
	/* Convenzionalmente, i messaggi sottoposti al meccanismo
	 di associazione codice-stringa sono identificati da un testo
	 avente come primo carattere ^A, seguito dalla stringa di
	 estensione. In questo caso la stringa e` numerica. */
		plcUpdateMessage((int)(c),"\001%d",v);
	}
	else {
		if (c == 0 && st -> code_old != 0)
			plcDelmess(st -> code_old);
	}

	st -> code_old = st -> code;
	st -> value_old = st -> value;

	return c;
}

