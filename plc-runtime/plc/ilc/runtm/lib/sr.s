
//*
//* @(#) sr.s 1.0 Tue Oct 15 16:18:01 MET 1996
//*
//*  Blocco funzionale "SR" a norme IEC-1131/3.
//*/
//
///*
//(* Flip-Flop con "set" dominante. *)
//
//function_block SR
//var_input
//	S1 : bool;
//	R : bool;
//end_var
//var_output
//	Q1 : bool;
//end_var
//
//	ldn	R
//	and	Q1
//	or	S1
//	st	Q1
//
//end_function_block
//*/

_SR_S1 = 0
_SR_R = 1
_SR_Q1 = 2

.text
	.globl	SR
SR:
	pushl	%esi
	movl	8(%esp),%esi
	xorl	%eax,%eax
	movb	_SR_R(%esi),%al
	xorb	$1,%al
	andb	_SR_Q1(%esi),%al
	orb	_SR_S1(%esi),%al
	movb	%al,_SR_Q1(%esi)
	popl	%esi
	ret

