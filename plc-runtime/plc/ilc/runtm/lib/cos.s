
//*
//* @(#) cos.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "COS".
//*/
//
///*
//(* Seno di grandezze REAL *)
//
//function COS : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_COS
_f_COS:
x:
	.long	0

.text
	.globl	COS
COS:
	flds	x
	fcos
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

