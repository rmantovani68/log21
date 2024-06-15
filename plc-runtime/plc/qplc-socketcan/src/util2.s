
/*
/* @(#) util2.s 1.1 Fri Jul 28 16:58:38 MET DST 2000 */
/* */
/* Funzioni di utilita` in assembler. */
/* */
/* 15/01/97 GG 1.0 Prima stesura. */
/* 28/07/00 GG 1.1 Aggiustamenti per compilazione su Linux. */
/* */

/*
/* Funzione "util_bit_scan" */
/* ------------------------ */
/* */
/*  Questa funzione restituisce l'indice del primo bit acceso del parametro "x". */
/*  Se "x" vale 0, il risultato e` -1. */
/* */

/* int util_bit_scan(int x); */

	.globl	util_bit_scan

util_bit_scan:

	xorl	%eax,%eax
	decl	%eax
	movl	4(%esp),%edx
	bsfl	%edx,%eax
	ret

/*
/* Funzione "util_fpu_reset" */
/* ------------------------- */
/* */
/*  Questa funzione rappezza un buco di Lynx, che dimentica di */
/* riprogrammare la maschera della eccezioni del coprocessore dopo */
/* la ricezione del segnale SIGFPE. */
/* */

/* void util_fpu_reset(void); */

	.globl	util_fpu_reset

util_fpu_reset:
	finit
	xorl	%eax,%eax
	pushl	%eax
	fstcw	(%esp)
	popl	%eax
	andl	$0xFFF3,%eax
	pushl	%eax
	fldcw	(%esp)
	popl	%eax
	ret

/*
/* Funzione "util_get_pc" */
/* ---------------------- */
/* */
/*  Questa funzione restituisce il PC del punto di chiamata del chiamate. */
/*  "level" indica quanti livelli di chiamata si vogliono ignorare. */
/* */

/* unsigned long util_get_pc(int level); */

	.globl	util_get_pc

util_get_pc:
	pushl	%ebp
	movl	%esp,%ebp
	movl	8(%ebp),%edx
	incl	%edx
_util_get_pc_2:
	movl	4(%ebp),%eax
	movl	(%ebp),%ebp
	decl	%edx
	jnz	_util_get_pc_2
_util_get_pc_1:
	popl	%ebp
	ret

/*
/* Funzione "util_call_il" */
/* ---------------------- */
/* */
/*  Questa funzione serve a lanciare un programma o un blocco funzionale IL. */
/*  "prog" e` l'indirizzo del blocco, "statics" e` l'indirizzo della */
/* struttura contenente le variabili di stato ed i parametri del blocco. */
/* */

/* unsigned long util_call_il(void (*prog)(void *statics), void *statics); */

	.globl	util_call_il

util_call_il:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	pushl	%edi
	pushl	%ebx
	movl	12(%ebp),%eax
	pushl	%eax
	movl	8(%ebp),%eax
	call	*%eax
	addl	$4,%esp
	popl	%ebx
	popl	%edi
	popl	%esi
	popl	%ebp
	ret

/*
/* Funzione "util_null" */
/* -------------------- */
/* */
/* Non fa niente, ma lo fa in fretta. */
/* */

/* void util_null(...) */

	.globl	util_null
util_null:
	ret

/*
/* Funzione "util_outb" */
/* -------------------- */
/* */
/* Out ad una porta. Un classico. */
/* */
/* int util_outb(int val, int port); */

	.globl	util_outb
util_outb:
	movl	8(%esp),%edx
	movl	4(%esp),%eax
	outb	%al,%dx
	ret

/*
/* Funzione "util_outsb" */
/* -------------------- */
/* */
/* Out ad una porta. Un classico. */
/* */
/* int util_outb(unsigned char * p, int port); */

	.globl	util_outsb
util_outsb:
	movl	8(%esp),%edx
	movl	4(%esp),%eax
	pushl	%esi
	movl	%eax,%esi
	outsb
	popl	%esi
	ret

/*
/* Funzione "util_inb" */
/* -------------------- */
/* */
/* IN da una porta. Un classico. */
/* */
/* int util_inb(int port); */

	.globl	util_inb
util_inb:
	movl	4(%esp),%edx
	xorl	%eax,%eax
	inb	%dx,%al
	ret


	.globl	util_bitr

util_bitr:
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	btrw	%ax,(%edx)
	sbbl	%eax,%eax
	ret

	.globl	util_bits

util_bits:
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	btsw	%ax,(%edx)
	sbbl	%eax,%eax
	ret

	.extern	IL_UserStart_fixup
	.globl IL_UserStart
IL_UserStart:
	movl	8(%esp),%eax
	pushl	%eax
	movl	8(%esp),%eax
	pushl	%eax
	call	IL_UserStart_fixup
	addl	$8,%esp
	popl	%edx
	movl	%edx,(%esp)
	ret
