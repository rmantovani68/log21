
//*
//* @(#) ln.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "LN".
//*/
//
///*
//(* Logaritmo naturale di grandezze REAL *)
//
//function LN : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_LN
_f_LN:
x:
	.long	0

.text
	.globl	LN
LN:
	fldln2
	flds	x
	fyl2x
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

