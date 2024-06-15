	.file	"prova2.ilc"
	.text
ilc.compiled:
$BF70726F7661322E696C63:
	.long	.L1,.L2,CONF_2,0x0
.L2:
	.long	0x0,0x41414141
.L3:
	.asciz	"CONF_2"
.L4:
	.asciz	"SOLA_ED_UNICA"
.L5:
	.asciz	"CPU1"
	.data
PROVA_2:
	.text
.L6:
	.long	PROVA_2,P_2
	.long	0x0,0x41414141
.L7:
	.long	.L6,0x0
.L8:
	.long	.L4,.L5,.L7
.L9:
	.long	.L8,0x0
CONF_2:
	.long	.L3,.L9
	.globl	P_2
P_2:
$FNP_2	=	9
$LN9$0:
$LE8$0:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	movl	8(%ebp),%esi
/	LD	TASTI[0]
$LN10$0:
$LE9$0:
.L11:
	movl	pKeyTab,%eax
	movl	%eax,%edi
.L12:
	movl	21483(%eax),%eax
.L13:
	movl	pchMem,%ebx
	movl	%ebx,%ecx
.L14:
	movzbl	(%eax,%ebx),%eax
	nop
	nop
	nop
	nop
	nop
/	JMPCN	NNNNN
$LN11$1:
$LE10$1:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
$LJ11$1:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	testl	%eax,%eax
	je	.L15
/ 
$LN12$1:
$LE11$1:
/
$LN13$1:
$LE12$1:
/
$LN14$1:
$LE13$1:
/NNNNN:
$LN15$1:
$LE14$1:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
$LL15$1:
.L15:
/
$LN16$1:
$LE15$1:
/END_PROGRAM
$LN17$1:
$LE16$1:
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
$LL17$1:
.L10:
	movl	-4(%ebp),%esi
	movl	%ebp,%esp
	popl	%ebp
	ret
	.align	4
.L1:
$EF70726F7661322E696C63:
