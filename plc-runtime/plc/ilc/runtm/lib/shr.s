
//*
//* @(#) shr.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "SHR". Sposta i bit dell'accumulatore a destra di POS
//* posizioni.
//*/
//
///*
//(* Shift a destra di POS posizioni. *)
//
//function SHR : DWORD
//var_input
//	X : DWORD;
//	POS : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_SHR
_f_SHR:
x:
	.long	0
pos:
	.word	0

.text
	.globl	SHR
SHR:
	movl	x,%eax
	movw	pos,%cx
	shr	%cl,%eax
	ret

