

	.text

	.globl	abilita_IM
abilita_IM:
	xorl	%eax,%eax
	pushl	%eax
	fstcw	(%esp)
	popl	%eax
	andl	$0xFFFE,%eax
	pushl	%eax
	fldcw	(%esp)
	popl	%eax
	ret

