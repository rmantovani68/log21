
//*
//* @(#) fabs.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "ABS_REAL".
//*/
//
///*
//(* Valore assoluto di grandezze REAL *)
//
//function ABS_REAL : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_ABS_REAL
_f_ABS_REAL:
x:
	.long	0

.text
	.globl	ABS_REAL
ABS_REAL:
	flds	x
	fabs
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

