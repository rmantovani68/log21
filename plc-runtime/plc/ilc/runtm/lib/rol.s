
//*
//* @(#) rol.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "ROL". Ruota i bit dell'accumulatore a sinistra di POS
//* posizioni.
//*/
//
///*
//(* Rotazione a sinistra di POS posizioni. *)
//
//function ROL : DWORD
//var_input
//	X : DWORD;
//	POS : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_ROL
_f_ROL:
x:
	.long	0
pos:
	.word	0

.text
	.globl	ROL
ROL:
	movl	x,%eax
	movw	pos,%cx
	rol	%cl,%eax
	ret

