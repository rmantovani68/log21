	.file	"prova.ilc"
	.text
ilc.compiled:
$BF70726F76612E696C63:
	.long	.L1,.L2,CONF_1,0x0
.L2:
	.long	IN1
	.byte	0x1,0xe
	.asciz	"IOS.0.0.3.0.0"
	.long	OUT1
	.byte	0x2,0xf
	.asciz	"IOS.0.0.3.0.24"
	.long	0x0,0x41414141
	.data
	.globl	_VERSION
_VERSION:
	.long	.LS1
	.globl	_CREATED
_CREATED:
	.long	0x55385651
	.text
.L3:
	.asciz	"CONF_1"
.L4:
	.asciz	"SOLA_ED_UNICA"
.L5:
	.asciz	"CPU1"
	.data
PROVA:
	.text
.L6:
	.long	PROVA,P
	.long	0x0,0x41414141
.L7:
	.long	.L6,0x0
.L8:
	.long	.L4,.L5,.L7
.L9:
	.long	.L8,0x0
CONF_1:
	.long	.L3,.L9
	.data
	.globl	IN1
IN1:
	.space	0x1,0x0
	.globl	OUT1
OUT1:
	.space	0x1,0x0
	.text
	.globl	P
P:
$FNP	=	19
$LN19$0:
$LE18$0:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	movl	8(%ebp),%esi
/	LD	IN1
$LN20$0:
$LE19$0:
.L11:
	movzbl	IN1,%eax
	nop
	nop
	nop
	nop
	nop
/	ST	OUT1
$LN21$1:
$LE20$1:
	nop
	nop
	nop
	nop
	nop
	movb	%al,OUT1
/END_PROGRAM
$LN22$1:
$LE21$1:
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
$LL22$1:
.L10:
	movl	-4(%ebp),%esi
	movl	%ebp,%esp
	popl	%ebp
	ret
.LS1:
	.byte	0x70,0x72,0x6f,0x76,0x61,0x20,0x49,0x4c
	.byte	0x20,0x31,0x2e,0x33,0x20,0x54,0x68,0x75
	.byte	0x20,0x4a,0x75,0x6e,0x20,0x31,0x32,0x20
	.byte	0x31,0x31,0x3a,0x34,0x33,0x3a,0x31,0x37
	.byte	0x20,0x4d,0x45,0x54,0x20,0x44,0x53,0x54
	.byte	0x20,0x31,0x39,0x39,0x37,0x0
	.align	4
.L1:
$EF70726F76612E696C63:
