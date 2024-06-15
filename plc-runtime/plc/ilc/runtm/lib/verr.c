
/*
* @(#) verr.c 1.0 Tue Feb 11 12:16:31 MET 1997
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
(***************************)
(* ERRORI (nuova gestione) *)
(***************************)
*/

/*
(* Emissione di un messaggio *)

function_block VERR
var_input
	CODE : dint;
	ENABLE : bool;
	RESET : bool;
end_var
var
	code_old : dint;
	enable_old : bool;
	reset_old : bool;
end_var
end_function_block
*/

struct plc_msg_t {
	long code __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));

	long code_old __attribute__((packed));
	char enable_old __attribute__((packed));
	char reset_old __attribute__((packed));
};

long VERR(register struct plc_msg_t * st)
{
register long c;

	c = st -> code;
	if (st -> reset && !st -> reset_old) {
		st -> enable_old = 0;
	}
	if (st -> enable) {
		if ((!st -> enable_old || c != st -> code_old) && c != 0)
			plcError((int)(c),"");
	}
	else {
		if (st -> enable_old && c != 0)
			plcDelerr(c);
	}

	st -> code_old = st -> code;
	st -> enable_old = st -> enable;
	st -> reset_old = st -> reset;

	return c;
}

