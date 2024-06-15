
/*
* @(#) recv_string.c 1.1 Thu Sep 25 16:01:31 MET DST 1997
*
*  Codifica dei blocchi funzionali CNi per ricezione di una stringa
* su un canale asincrono. Ci vuole qplc >= 1.5.2.3 .
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 17/09/97 GG 1.0 Prima stesura.
* 25/09/97 GG 1.1 Rimaneggiato per semplificare il codice. Bastave sfruttare
*             l'inizializzazione a livello IL.
*/

#include <plcapi.h>

/*
(* Ricezione di una stringa. *)

function_block RECV_STRING
var_input
	CH : dint;
	LASTCHR : byte;
	ENABLE : bool;
	RESET : bool;
end_var
var_output
	DONE : bool;
	STR : string := '';
	NB_RECV : uint;
	ERR : bool;
end_var
var
	buffer : array[0..63] of byte;
	status : array[0..15] of byte;
end_var
end_function_block
*/

struct plc_recv_string_t {
	long ch __attribute__((packed));
	char lastchr __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));

	char done __attribute__((packed));
	char *str __attribute__((packed));
	unsigned short nb_recv __attribute__((packed));
	char err __attribute__((packed));

	char buffer[64] __attribute__((packed));
	stream_rxtx_t status __attribute__((packed));
};

long RECV_STRING(register struct plc_recv_string_t * st)
{
	if (st -> reset) {
		st -> str = "";
		st -> nb_recv
		 = st -> done
		 = st -> err
		 = st -> buffer[0]
		 = st -> status.status = 0;
	}
	else if (! st -> enable) {
		st -> str = st -> buffer;
	}
	else {
		if (st -> done) {
			st -> nb_recv
			 = st -> done
			 = st -> err
			 = st -> buffer[0]
			 = st -> status.status = 0;
		}
		st -> str = "";
		st -> status.message = &(st -> buffer[st -> nb_recv]);
		*(st -> status.message) = '\0';
		st -> status.msg_size = 1;
		st -> status.msg_curr_pos = 0;

		if (ioExtendedOp(st -> ch,
		                 DREXTOP_STREAM_OP_RECV,
		                 &(st -> status))
		 && ! (st -> status.status & DREXTOP_STREAM_F_ERRS)) {
			st -> err = 0;
			if (st -> status.status & DREXTOP_STREAM_F_DONE) {
				st -> status.message[1] = '\0';
				++(st -> nb_recv);
				if (st -> nb_recv >= sizeof(st -> buffer) - 1
				 || *(st -> status.message) == st -> lastchr) {
					st -> done = 1;
					st -> str = st -> buffer;
				}
			}
		}
		else {
			st -> err = 1;
			st -> done = 0;
		}
	}

	return st -> done;
}

