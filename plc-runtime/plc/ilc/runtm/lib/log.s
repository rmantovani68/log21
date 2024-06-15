
//*
//* @(#) log.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "LOG".
//*/
//
///*
//(* Logaritmo decimale di grandezze REAL *)
//
//function LOG : REAL
//var_input
//	X : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_LOG
_f_LOG:
x:
	.long	0

.text
	.globl	LOG
LOG:
	fldlg2
	flds	x
	fyl2x
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

