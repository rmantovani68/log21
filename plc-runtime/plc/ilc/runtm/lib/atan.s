
//*
//* @(#) atan.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "ATAN".
//*/
//
///*
//(* Arcocoseno di grandezze REAL *)
//
//function ATAN : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_ATAN
_f_ATAN:
x:
	.long	0

.text
	.globl	ATAN
ATAN:
	flds	x
	fld1
	fpatan
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

