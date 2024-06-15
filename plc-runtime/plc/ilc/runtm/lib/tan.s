
//*
//* @(#) tan.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "TAN".
//*/
//
///*
//(* Seno di grandezze REAL *)
//
//function TAN : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_TAN
_f_TAN:
x:
	.long	0

.text
	.globl	TAN
TAN:
	flds	x
	fsincos
	fdivrp
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

