
/*
* @(#) recv_bytes.c 1.1 Thu Sep 25 15:59:24 MET DST 1997
*
*  Codifica dei blocchi funzionali CNi per ricezione di un insieme
* di byte su un canale asincrono. Ci vuole qplc >= 1.5.2.3 .
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 17/09/97 GG 1.0 Prima stesura.
* 25/09/97 GG 1.1 Rimaneggiato per semplificare il codice. Bastave sfruttare
*             l'inizializzazione a livello IL. E aggiunto l'ingresso ENABLE.
*/

#include <plcapi.h>

/*
(* Ricezione di NB byte *)

function_block RECV_BYTES
var_input
	CH : dint;
	NB : uint;
	ENABLE : bool;
	RESET : bool;
end_var
var_output
	BUFFER : array[0..63] of byte;
	NB_RECV : uint;
	DONE : bool;
	ERR : bool;
end_var
var
	status : array[0..15] of byte;
end_var
end_function_block
*/

struct plc_recv_bytes_t {
	long ch __attribute__((packed));
	unsigned short nb __attribute__((packed));
	char enable __attribute__((packed));
	char reset __attribute__((packed));

	char buffer[64] __attribute__((packed));
	unsigned short nb_recv __attribute__((packed));
	char done __attribute__((packed));
	char err __attribute__((packed));

	stream_rxtx_t status __attribute__((packed));
};

long RECV_BYTES(register struct plc_recv_bytes_t * st)
{
	if (st -> reset) {
		st -> status.msg_curr_pos
		 = st -> nb_recv
		 = st -> done
		 = st -> err
		 = st -> status.status = 0;
	}
	else if	(st -> nb > sizeof(st -> buffer)) {
		st -> err = 1;
		st -> done = 0;
	}
	else if (st -> enable) {
		if (st -> done) {
			st -> status.msg_curr_pos
			 = st -> nb_recv
			 = st -> done
			 = st -> err
			 = st -> status.status = 0;
		}

		st -> status.message = st -> buffer;
		st -> status.msg_size = st -> nb;

		if (ioExtendedOp(st -> ch,
				 DREXTOP_STREAM_OP_RECV,
				 &(st -> status))
		 && ! (st -> status.status & DREXTOP_STREAM_F_ERRS)) {
			st -> err = 0;
			st -> done = (st -> status.status
				      & DREXTOP_STREAM_F_DONE) != 0;
		}
		else {
			st -> err = 1;
			st -> done = 0;
		}

		st -> nb_recv = st -> status.msg_curr_pos;
	}

	return st -> done;
}

