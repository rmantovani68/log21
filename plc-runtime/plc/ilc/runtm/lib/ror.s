
//*
//* @(#) ror.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "ROR". Ruota i bit dell'accumulatore a destra di POS
//* posizioni.
//*/
//
///*
//(* Rotazione a destra di POS posizioni. *)
//
//function ROR : DWORD
//var_input
//	X : DWORD;
//	POS : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_ROR
_f_ROR:
x:
	.long	0
pos:
	.word	0

.text
	.globl	ROR
ROR:
	movl	x,%eax
	movw	pos,%cx
	ror	%cl,%eax
	ret

