
/*
* @(#) er.c 1.0 Tue Sep 10 02:08:09 MET DST 1996
*
*  Codifica dei blocchi funzionali CNi per emissione di errori
* o messaggi codificati come interi.
*
*  ATTENZIONE ! Questo codice dipende totalmente dal funzionamento
* del compilatore ILC !
*
* 10/09/96 GG 1.0 Prima stesura.
*/

/*
(**********)
(* ERRORI *)
(**********)
*/

/*
(* Emissione di un errore *)

function_block ERROR
var_input
	CODE : dint;
end_var
var
	code_old : dint;
end_var
end_function_block
*/

enum {
_ERROR_CODE,
_ERROR_CODE_old = _ERROR_CODE + sizeof(long),
};

long ERROR(register char * st)
{
register long err;

	err = *(long *)(st + _ERROR_CODE);
	if (err && err != *(long *)(st + _ERROR_CODE_old)) {
		plcError((int)(err),"");
	}
	*(long *)(st + _ERROR_CODE_old) = err;

	return err;
}

