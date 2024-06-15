
//*
//* @(#) acos.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "ACOS".
//*/
//
///*
//(* Arcocoseno di grandezze REAL *)
//
//function ACOS : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_ACOS
_f_ACOS:
x:
	.long	0

.text
	.globl	ACOS
ACOS:
	flds	x
	fld	%st(0)
	fmul	%st(0),%st(0)
	fld1
	fsubp
	fsqrt
	fxch	%st(1)
	fpatan
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

