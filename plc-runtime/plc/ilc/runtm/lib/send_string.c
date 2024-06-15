
/*
* @(#) send_string.c 1.2 Tue Oct  7 09:26:59 MET 1997
*
*  Codifica dei blocchi funzionali CNi per trasmissione di una stringa
* su un canale asincrono. Ci vuole qplc >= 1.5.2.3 .
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 17/09/97 GG 1.0 Prima stesura.
* 25/09/97 GG 1.1 Rimaneggiato per semplificare il codice. Bastave sfruttare
*             l'inizializzazione a livello IL.
* 07/10/97 GG 1.2 Modificata la gestione del puntatore NULL, che provocava
*             il blocco permanente del trasmettitore.
*/

#include <plcapi.h>

/*
(* Trasmissione di una stringa. *)

function_block SEND_STRING
var_input
	CH : dint;
	STR : string;
	ENABLE : bool;
	RESET : bool;
end_var
var_output
	NB_SENT : uint;
	DONE : bool := TRUE;
	ERR : bool;
end_var
var
	enable_old : bool;
	status : array[0..15] of byte;
end_var
end_function_block
*/

struct plc_send_string_t {
	long ch __attribute__((packed));
	char *str __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));

	unsigned short nb_sent __attribute__((packed));
	char done __attribute__((packed));
	char err __attribute__((packed));

	char enable_old __attribute__((packed));
	stream_rxtx_t status __attribute__((packed));
};

static void SEND_STRING_core(register struct plc_send_string_t * st)
{
	if (! st -> str) {
		st -> err = 0;
		st -> done = 1;
	}
	else if (ioExtendedOp(st -> ch,
	                 DREXTOP_STREAM_OP_SEND,
	                 &(st -> status))
	 && ! (st -> status.status & DREXTOP_STREAM_F_ERRS)) {
		st -> err = 0;
		st -> done = (st -> status.status
			        & DREXTOP_STREAM_F_DONE) != 0;
	}
	else {
		st -> err = 1;
		st -> done = 1;
	}
	st -> nb_sent = st -> status.msg_curr_pos;
}

long SEND_STRING(register struct plc_send_string_t * st)
{
	if (st -> reset) {
		st -> enable_old = 0;
		st -> nb_sent = 0;
		st -> str = "";
		st -> done = 1;
		st -> err = 0;
	}
	else if (st -> enable) {
		if (! st -> enable_old) {
			if (st -> done) {
				st -> nb_sent = st -> status.msg_curr_pos = 0;
				st -> status.message = st -> str;
				st -> status.msg_size = strlen(st -> str);
				SEND_STRING_core(st);
			}
			else {
				st -> err = 1;
			}
		}
		else if (! st -> done) {
			SEND_STRING_core(st);
		}
	}

	st -> enable_old = st -> enable;

	return st -> done;
}

