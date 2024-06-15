
//*
//* @(#) abs.s 1.0 Thu Jun 19 08:01:09 MET DST 1997
//*
//*  Funzione "ABS".
//*/
//
///*
//(* VAlore assoluto di interi *)
//
//function ABS : DINT
//var_input
//	X : DINT;
//end_var
//end_function
//*/

.data
	.globl	_f_ABS
_f_ABS:
x:
	.long	0

.text
	.globl	ABS
ABS:
	movl	x,%eax
	testl	%eax,%eax
	jns	.L1
	negl	%eax
.L1:
	ret

