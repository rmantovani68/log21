
//*
//* @(#) f_trig.s 1.0 Tue Oct 15 16:17:53 MET 1996
//*
//*  Blocco funzionale "F_TRIG" a norme IEC-1131/3.
/*/

//*
//(* Falling edge detector *)
//
//function_block F_TRIG
//var_input
//	CLK : bool;
//end_var
//var_output
//	Q : bool;
//end_var
//var
//	status__ : bool;
//end_var
//
//	ldn	CLK
//	and	status__
//	st	Q
//	ld	CLK
//	st	status__
//
//end_function_block
//*/

_F_TRIG_CLK = 0
_F_TRIG_Q = 1
_F_TRIG_sts = 2

	.text
	.globl	F_TRIG
F_TRIG:
	pushl	%esi
	movl	8(%esp),%esi
	xorl	%eax,%eax
	movb	_F_TRIG_sts(%esi),%bl
	movb	_F_TRIG_CLK(%esi),%al
	movb	%al,_F_TRIG_sts(%esi)
	xorb	$1,%al
	andb	%bl,%al
	movb	%al,_F_TRIG_Q(%esi)
	popl	%esi
	ret

