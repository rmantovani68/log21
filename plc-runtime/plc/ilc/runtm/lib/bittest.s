
//*
//* @(#) bittest.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "BITTEST". Controlla il valore del bit indicato
//* come parametro aggiuntivo.
//*/
//
///*
//(* Controllo del bit BITN nell'accumulatore (quantita` a 32 bit). *)
//
//function BITTEST : BOOL
//var_input
//	X : DWORD;
//	BITN : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_BITTEST
_f_BITTEST:
x:
	.long	0
bitn:
	.word	0

.text
	.globl	BITTEST
BITTEST:
	movzwl	bitn,%ebx
	xor	%eax,%eax
	bt	%ebx,x
	setc	%al
	ret

