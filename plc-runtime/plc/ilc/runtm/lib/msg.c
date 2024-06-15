
/*
* @(#) msg.c 1.0 Tue Feb 11 12:16:31 MET 1997
*
*  Codifica dei blocchi funzionali CNi per emissione di messaggi
* codificati come interi. Funziona solo con versioni di "QUOTE" scrtiite
* dopo l'11/2/97.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 11/02/97 GG 1.0 Prima stesura.
*/

/*
(************)
(* MESSAGGI *)
(************)
*/

/*
(* Emissione di un messaggio *)

function_block MESSAGE
var_input
	CODE : dint;
end_var
var
	code_old : dint;
end_var
end_function_block
*/

struct plc_simple_msg_t {
	long code __attribute__((packed));

	long code_old __attribute__((packed));
};

long MESSAGE(register struct plc_simple_msg_t * st)
{
register long c;

	c = st -> code;
	if (c != st -> code_old && c != 0) {
	/* Convenzionalmente, i messaggi sottoposti al meccanismo
	 di associazione codice-stringa sono identificati da un testo
	 contenente il solo carattere ^A. */
		plcMessage((int)(c),"\001");
	}
	else {
		if (c == 0 && st -> code_old != 0)
			plcDelmess(st -> code_old);
	}

	st -> code_old = st -> code;

	return c;
}

