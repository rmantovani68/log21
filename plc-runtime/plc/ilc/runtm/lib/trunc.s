
//*
//* @(#) real_to_dint.s 1.0 Thu Jun 19 09:45:57 MET DST 1997
//*
//*  Funzione "TRUNC".
//*/
//
///*
//(* Conversione da REAL a DINT con troncamento. *)
//
//function TRUNC : DINT
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_TRUNC
_f_TRUNC:
x:
	.long	0

.text
	.globl	TRUNC
TRUNC:
	flds	x
	pushl	%eax
	fnstcw	(%esp)
	movl	(%esp),%eax
	movb	$0x0C,%ah
	pushl	%eax
	fldcw	(%esp)
	fistpl	(%esp)
	popl	%eax
	fldcw	(%esp)
	addl	$4,%esp
	ret

