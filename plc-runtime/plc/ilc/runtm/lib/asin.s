
//*
//* @(#) asin.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "ASIN".
//*/
//
///*
//(* Arcoseno di grandezze REAL *)
//
//function ASIN : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_ASIN
_f_ASIN:
x:
	.long	0

.text
	.globl	ASIN
ASIN:
	flds	x
	fld	%st(0)
	fmul	%st(0),%st(0)
	fld1
	fsubp
	fsqrt
	fpatan
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

