
//*
//* @(#) real_to_dint.s 1.1 Thu Jun 19 09:54:41 MET DST 1997
//*
//*  Funzione "REAL_TO_DINT". Si tratta di un a "pezza d'emergenza" per
//* permettere di sommare - sottrarre - moltiplicare - dividere - convertire
//* Quantita` di tipo REAL da linguaggio IL. Ma non e` la soluzione giusta.
//*
//* 13/03/97 GG 1.0 Prima stesura.
//* 19/06/97 GG 1.1 Corretta la programmazione della modalita` di
//*             arrotondamento, per allinearla alle norme.
//*/
//
///*
//(* Conversione da DINT a REAL *)
//
//function REAL_TO_DINT : DINT
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_REAL_TO_DINT
_f_REAL_TO_DINT:
x:
	.long	0

.text
	.globl	REAL_TO_DINT
REAL_TO_DINT:
	flds	x
	pushl	%eax
	fnstcw	(%esp)
	movl	(%esp),%eax
	movb	$0x00,%ah
	pushl	%eax
	fldcw	(%esp)
	fistpl	(%esp)
	popl	%eax
	fldcw	(%esp)
	addl	$4,%esp
	ret

