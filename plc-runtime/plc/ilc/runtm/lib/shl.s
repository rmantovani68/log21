
//*
//* @(#) shl.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "SHL". Sposta i bit dell'accumulatore a sinistra di POS
//* posizioni.
//*/
//
///*
//(* Shift a sinistra di POS posizioni. *)
//
//function SHL : DWORD
//var_input
//	X : DWORD;
//	POS : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_SHL
_f_SHL:
x:
	.long	0
pos:
	.word	0

.text
	.globl	SHL
SHL:
	movl	x,%eax
	movw	pos,%cx
	shl	%cl,%eax
	ret

