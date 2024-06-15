
//*
//* @(#) rs.s 1.0 Tue Oct 15 16:17:53 MET 1996
//*
//*  Blocco funzionale "RS" a norme IEC-1131/3.
//*/
//
///*
//(* Flip-Flop con "reset" dominante. *)
//
//function_block RS
//var_input
//	S : bool;
//	R1 : bool;
//end_var
//var_output
//	Q1 : bool;
//end_var
//
//	ld	S
//	or	Q1
//	andn	R1
//	st	Q1
//
//end_function_block
//*/

_RS_S = 0
_RS_R1 = 1
_RS_Q1 = 2

.text
	.globl	RS
RS:
	pushl	%esi
	movl	8(%esp),%esi
	xorl	%eax,%eax
	movb	_RS_S(%esi),%al
	orb	_RS_Q1(%esi),%al
	movb	_RS_R1(%esi),%bl
	xorb	$1,%bl
	andb	%bl,%al
	movb	%al,_RS_Q1(%esi)
	popl	%esi
	ret

