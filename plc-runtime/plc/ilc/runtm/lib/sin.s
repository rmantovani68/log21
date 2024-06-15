
//*
//* @(#) sin.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "SIN".
//*/
//
///*
//(* Seno di grandezze REAL *)
//
//function SIN : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_SIN
_f_SIN:
x:
	.long	0

.text
	.globl	SIN
SIN:
	flds	x
	fsin
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

