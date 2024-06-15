
//*
//* @(#) sqrt.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "SQRT".
//*/
//
///*
//(* Radice quadrata di grandezze REAL *)
//
//function SQRT : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_SQRT
_f_SQRT:
x:
	.long	0

.text
	.globl	SQRT
SQRT:
	flds	x
	fsqrt
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

