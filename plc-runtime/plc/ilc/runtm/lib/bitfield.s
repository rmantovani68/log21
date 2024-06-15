
//*
//* @(#) bitfield.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "BITFIELD x,y". Estrae il campo di bit di posizione x e lunghezza
//* y dall'accumulatore.
//*/
//
///*
//(* Estrazione del campo di bit di posizione POS e lunghezza LEN *)
//(* dall'accumulatore (quantita` a 32 bit). *)
//
//function BITFIELD : DWORD
//var_input
//	X : DWORD;
//	POS : UINT;
//	LEN : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_BITFIELD
_f_BITFIELD:
x:
	.long	0
pos:
	.word	0
len:
	.word	0

.text
	.globl	BITFIELD
BITFIELD:
	movl	x,%eax
	movw	pos,%cx
	shr	%cl,%eax
	movw	len,%cx
	movl	$1,%ebx
	shl	%cl,%ebx
	decl	%ebx
	andl	%ebx,%eax
	ret

