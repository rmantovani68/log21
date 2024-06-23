	.file	"prova.plc"
	.text
ilc.compiled:
$BF70726F76612E706C63:
	.long	.L1,.L2,0x0
.L2:
	.long	0x0,0x41414141
	.data
	.globl	ING
ING:
	.byte	0x00
	.globl	USC
USC:
	.byte	0x00
	.globl	_CREATED
_CREATED:
	.long	0x66718b83
	.globl	_VERSION
_VERSION:
	.long	.LS1
	.text
	.globl	PROVA
PROVA:
$FNPROVA	=	8
$LE7$0:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	movl	8(%ebp),%esi
/LDN ING
$LE9$0:
.L4:
	movzbl	ING,%eax
	testl	%eax,%eax
	sete	%al
	nop
	nop
	nop
	nop
	nop
/ST ING
$LE10$1:
	nop
	nop
	nop
	nop
	nop
	movb	%al,ING
/
$LE11$1:
/
$LE12$1:
/
$LE13$1:
/
$LE14$1:
/
$LE15$1:
/
$LE16$1:
/END_PROGRAM
$LE17$1:
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
$LL18$1:
.L3:
	movl	-4(%ebp),%esi
	movl	%ebp,%esp
	popl	%ebp
	ret
.LS1:
	.byte	0x31,0x2e,0x30,0x2e,0x30,0x2e,0x30,0x0
	.align	4
.L1:
$EF70726F76612E706C63:
