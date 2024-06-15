
//*
//* @(#) bitset.s 1.0 Wed Feb 12 08:42:48 MET 1997
//*
//*  Funzione "BITSET". Porta ad 1 il bit dell'accumulatore indicato
//* come parametro aggiuntivo
//*/
//
///*
//(* Accensione del bit BITN nell'accumulatore (quantita` a 32 bit). *)
//
//function BITSET : DWORD
//var_input
//	X : DWORD;
//	BITN : UINT;
//end_var
//end_function
//*/

.data
	.globl	_f_BITSET
_f_BITSET:
x:
	.long	0
bitn:
	.word	0

.text
	.globl	BITSET
BITSET:
	movl	x,%eax
	movzwl	bitn,%ebx
	bts	%ebx,%eax
	ret

