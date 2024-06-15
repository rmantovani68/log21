	.file	"test.plc"
	.text
ilc.compiled:
$BF746573742E706C63:
	.long	.L1,.L2,0x0
.L2:
	.long	0x0,0x41414141
	.data
	.globl	UNO
UNO:
	.short	0x0000
	.globl	V1
V1:
	.byte	0x00
	.globl	V2
V2:
	.byte	0x00
	.globl	V3
V3:
	.byte	0x00
	.globl	V4
V4:
	.byte	0x00
	.globl	INTEST
INTEST:
	.byte	0x00
	.globl	_CREATED
_CREATED:
	.long	0x4239d449
	.globl	_VERSION
_VERSION:
	.long	.LS1
	.text
	.globl	NONAME
NONAME:
$FNNONAME	=	12
$LE11$0:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	movl	8(%ebp),%esi
/	LD	TEST
$LE12$0:
.L4:
	movzbl	TEST,%eax
	nop
	nop
	nop
	nop
	nop
/	ST 	INTEST
$LE13$1:
	nop
	nop
	nop
	nop
	nop
	movb	%al,INTEST
/
$LE14$1:
/	LD	UNO
$LE15$1:
.L5:
	movzwl	UNO,%eax
/	ADD	1
$LE16$1:
	incl	%eax
/	ST	UNO
$LE17$1:
	movw	%ax,UNO
/	EQ	20
$LE18$1:
	cmpl	$0x14,%eax
	sete	%al
	movzbl	%al,%eax
/	JMPCN	AVANTI
$LE19$1:
	nop
	nop
	nop
	nop
	nop
$LJ20$1:
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
	je	.L6
/	LD	0
$LE20$1:
	movl	$0x0,%eax
	nop
	nop
	nop
	nop
	nop
/	ST	UNO
$LE21$1:
	nop
	nop
	nop
	nop
	nop
	movw	%ax,UNO
/	
$LE22$1:
/	LD	USCI
$LE23$1:
.L7:
	movzbl	USCI,%eax
/	NOT
$LE24$1:
	testl	%eax,%eax
	sete	%al
/	ST	USCI
$LE25$1:
	movb	%al,USCI
/	ST	OUT3
$LE26$1:
	movb	%al,OUT3
/	STN	OUT4
$LE27$1:
	movl	%eax,%ebx
	testl	%ebx,%ebx
	sete	%bl
	movb	%bl,OUT4
/	STN	OUT5
$LE28$1:
	movl	%eax,%ebx
	testl	%ebx,%ebx
	sete	%bl
	movb	%bl,OUT5
/	
$LE29$1:
/	LD	IN1
$LE30$1:
.L8:
	movzbl	IN1,%eax
/	ST	USCI2
$LE31$1:
	movb	%al,USCI2
/	LD	IN2
$LE32$1:
.L9:
	movzbl	IN2,%eax
/	
$LE33$1:
/AVANTI:
$LE34$1:
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
$LL35$1:
.L6:
/
$LE35$1:
/	LD	IN1
$LE36$1:
.L10:
	movzbl	IN1,%eax
	nop
	nop
	nop
	nop
	nop
/	ST	V1
$LE37$1:
	nop
	nop
	nop
	nop
	nop
	movb	%al,V1
/	LD	IN2
$LE38$1:
.L11:
	movzbl	IN2,%eax
/	ST	V2
$LE39$1:
	movb	%al,V2
/	
$LE40$1:
/	LD	IN3
$LE41$1:
.L12:
	movzbl	IN3,%eax
/	ST	V3
$LE42$1:
	movb	%al,V3
/	OR	IN4
$LE43$1:
.L13:
	orb	IN4,%al
$VA44$0011$0$IN4	=	.L13
/	ST	V4
$LE44$1:
	movb	%al,V4
/	ST	OUT2
$LE45$1:
	movb	%al,OUT2
/	
$LE46$1:
/	LD	IN5
$LE47$1:
.L14:
	movzbl	IN5,%eax
/	ST	OUT6
$LE48$1:
	movb	%al,OUT6
/	
$LE49$1:
/END_PROGRAM
$LE50$1:
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
$LL51$1:
.L3:
	movl	-4(%ebp),%esi
	movl	%ebp,%esp
	popl	%ebp
	ret
.LS1:
	.byte	0x31,0x2e,0x30,0x2e,0x30,0x2e,0x30,0x0
	.align	4
.L1:
$EF746573742E706C63:
