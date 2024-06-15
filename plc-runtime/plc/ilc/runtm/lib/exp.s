
//*
//* @(#) exp.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "EXP".
//*/
//
///*
//(* Logaritmo naturale di grandezze REAL *)
//
//function EXP : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_EXP
_f_EXP:
x:
	.long	0

.text
	.globl	EXP
EXP:
	pushl	%ecx
/ S[0] = S[0] * log2(e)
	fldl2e
	flds	x
	fmulp
/ S[1] = S[0]
	fld	%st(0)
/ S[2] = S[1]
	fld	%st(0)
/ S[2] = TRUNC(S[1])
	pushl	%eax
	fnstcw	(%esp)
	movl	(%esp),%eax
	movb	$0x0C,%ah
	pushl	%eax
	fldcw	(%esp)
	frndint
	popl	%eax
	fldcw	(%esp)
	addl	$4,%esp
/ S[0] = S[2]
	fst	%st(2)
/ S[1] = S[1] - S[2]
	fsubrp
/ S[1] = POW(2,S[1])
	f2xm1
	fld1
	faddp
/ S[0] = S[1] * POW(2,S[0]);
	fscale
	fstp	%st(1)
	pushl	%eax
	fstps	(%esp)
	popl	%eax

	popl	%ecx
	ret

