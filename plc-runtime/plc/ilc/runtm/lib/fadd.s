
//*
//* @(#) fadd.s 1.2 Wed Jan 28 11:58:54 MET 1998
//*
//*  Funzione "ADD_REAL". Si tratta di un a "pezza d'emergenza" per
//* permettere di sommare - sottrarre - moltiplicare - dividere - convertire
//* Quantita` di tipo REAL da linguaggio IL. Ma non e` la soluzione giusta.
//*
//* 13/03/97 GG 1.0 Prima stesura.
//* 19/06/97 GG 1.1 Allineato con le indicazioni IEC riguardanti
//*             l'overloading di funzioni. Il vecchi nome sara` mantenuto
//*             per un po', per compatibilita`.
//* 28/01/98 GG 1.2 Aggiunte definizioni globali per compatibilita`.
//*/
//
///*
//(* Somma di valori REAL *)
//
//function ADD_REAL : REAL
//var_input
//	X : REAL;
//	Y : REAL;
//end_var
//end_function
//*/

.data
	.globl	_f_ADD_REAL
_f_ADD_REAL:
	.globl	_f_FADD
_f_FADD:
x:
	.long	0
y:
	.long	0

.text
	.globl	ADD_REAL
ADD_REAL:
	.globl	FADD
FADD:
	flds	x
	fadds	y
	pushl	%eax
	fstps	(%esp)
	popl	%eax
	ret

