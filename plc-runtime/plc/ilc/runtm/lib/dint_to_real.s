
//*
//* @(#) dint_to_real.s 1.0 Thu Mar 13 16:49:20 MET 1997
//*
//*  Funzione "DINT_TO_REAL". Si tratta di un a "pezza d'emergenza" per
//* permettere di sommare - sottrarre - moltiplicare - dividere - convertire
//* Quantita` di tipo REAL da linguaggio IL. Ma non e` la soluzione giusta.
//*/
//
///*
//(* Conversione da DINT a REAL *)
//
//function DINT_TO_REAL : REAL
//var_input
//	X : DINT;
//end_var
//end_function
//*/

.data
	.globl	_f_DINT_TO_REAL
_f_DINT_TO_REAL:
x:
	.long	0

.text
	.globl	DINT_TO_REAL
DINT_TO_REAL:
	fildl	x
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

