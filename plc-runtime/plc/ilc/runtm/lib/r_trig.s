
//*
//* @(#) r_trig.s 1.0 Tue Oct 15 16:17:53 MET 1996
//*
//*  Blocco funzionale "R_TRIG" a norme IEC-1131/3.
//*/
//
//*
/(* Rising edge detector *)
//
//function_block R_TRIG
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
//	ld	CLK
//	andn	status__
//	st	Q
//	ld	CLK
//	st	status__
//
//end_function_block
//*/

_R_TRIG_CLK = 0
_R_TRIG_Q = 1
_R_TRIG_sts = 2

.text
	.globl	R_TRIG
R_TRIG:
	pushl	%esi
	movl	8(%esp),%esi
	xorl	%eax,%eax
	movb	_R_TRIG_sts(%esi),%al
	xorb	$1,%al
	movb	_R_TRIG_CLK(%esi),%bl
	movb	%bl,_R_TRIG_sts(%esi)
	andb	%bl,%al
	movb	%al,_R_TRIG_Q(%esi)
	popl	%esi
	ret

