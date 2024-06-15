
/*
* @(#) vmsgr.c 1.0 Thu Jul 13 20:00:42 MET DST 2000
*
*  Codifica dei blocchi funzionali CNi per emissione di messaggi
* codificati come reali. Funziona solo con versioni di "QUOTE" scritte
* dopo l'11/2/97.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 13/07/00 GG 1.0 Prima stesura.
*/

/*
(* Emissione di un messaggio con codice *)

function_block VMSGR
var_input
	CODE : dint;
	VALUE : real;
	ENABLE : bool;
	RESET : bool;
	UPDATE : bool;
	VALOK : bool;
end_var
var
	enable_old : bool;
	reset_old : bool;
	code_old : dint;
	value_old : real;
end_var
end_function_block
*/

struct plc_vmsgr_t {
	long code __attribute__((packed));
	float value __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));
	char update __attribute__((packed));
	char valok __attribute__((packed));

	long code_old __attribute__((packed));
	float value_old __attribute__((packed));
	char enable_old __attribute__((packed));
	char reset_old __attribute__((packed));
};

long VMSGR(register struct plc_vmsgr_t * st)
{
register long c;
float v;

	c = st -> code;
	if (st -> reset && !st -> reset_old) {
		st -> enable_old = 0;
	}
	if (st -> enable) {
		if (((((v = st -> value) != st -> value_old)
		      && st -> valok)
		     || !st -> enable_old
		     || c != st -> code_old)
		    && c != 0) {
		/* Convenzionalmente, i messaggi sottoposti al meccanismo
		 di associazione codice-stringa sono identificati da un testo
		 avente come primo carattere ^A, seguito dalla stringa di
		 estensione. In questo caso la stringa e` numerica. */
			if (st -> update)
				plcUpdateMessage((int)(c),"\001%f",v);
			else
				plcMessage((int)(c),"\001%f",v);
		}
	}
	else {
		if (st -> enable_old && c != 0)
			plcDelmess(c);
	}

	if (st -> valok)
		st -> value_old = st -> value;
	st -> code_old = st -> code;
	st -> enable_old = st -> enable;
	st -> reset_old = st -> reset;

	return c;
}

