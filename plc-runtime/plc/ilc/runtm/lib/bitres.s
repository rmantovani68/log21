
//*
//* @(#) bitres.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "BITSET". Porta a 0 il bit dell'accumulatore indicato
//* come parametro aggiuntivo.
//*/
//
///*
//(* Spegnimento del bit BITN nell'accumulatore (quantita` a 32 bit). *)
//
//function BITRES : DWORD
//var_input
//	X : DWORD;
//	BITN : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_BITRES
_f_BITRES:
x:
	.long	0
bitn:
	.word	0

.text
	.globl	BITRES
BITRES:
	movl	x,%eax
	movzwl	bitn,%ebx
	btr	%ebx,%eax
	ret

