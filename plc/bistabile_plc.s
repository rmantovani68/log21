	.file	"bistabile.plc"
	.text
ilc.compiled:
$BF626973746162696C652E706C63:
	.long	.L1,.L2,0x0
.L2:
	.long	0x0,0x41414141
	.data
	.globl	CICCIA
CICCIA:
	.byte	0x00
	.globl	CICCIA1
CICCIA1:
	.byte	0x00
	.globl	CICCIAN
CICCIAN:
	.space	0x1,0x0
	.space	0x4,0x0
	.space	0x1,0x0
	.space	0x4,0x0
	.space	0x1,0x0
	.space	0x4,0x0
	.globl	CICCIAF
CICCIAF:
	.space	0x1,0x0
	.space	0x4,0x0
	.space	0x1,0x0
	.space	0x4,0x0
	.space	0x1,0x0
	.space	0x4,0x0
	.globl	_CREATED
_CREATED:
	.long	0x666f424a
	.globl	_VERSION
_VERSION:
	.long	.LS1
	.text
	.globl	BISTABILE
BISTABILE:
$FNBISTABILE	=	10
$LE9$0:
	pushl	%ebp
	movl	%esp,%ebp
	pushl	%esi
	movl	8(%ebp),%esi
/	LDN	CICCIA
$LE18$0:
.L4:
	movzbl	CICCIA,%eax
	testl	%eax,%eax
	sete	%al
	nop
	nop
	nop
	nop
	nop
/	ST	CICCIAN.IN
$LE19$1:
	nop
	nop
	nop
	nop
	nop
	movb	%al,CICCIAN+0x0
/	LD	T#500MS
$LE20$1:
	movl	$0x1f4,%eax
/	ST	CICCIAN.PT
$LE21$1:
	movl	%eax,CICCIAN+0x1
/	CAL	CICCIAN	 
$LE22$1:
	pushl	$CICCIAN
	call	TON
	addl	$0x4,%esp
/	LD	CICCIAN.Q
$LE23$1:
.L5:
	movzbl	CICCIAN+0x5,%eax
/	ST	CICCIA1
$LE24$1:
	movb	%al,CICCIA1
/	
$LE25$1:
/	
$LE26$1:
/	LD	CICCIA1
$LE27$1:
.L6:
	movzbl	CICCIA1,%eax
/	ST	CICCIAF.IN
$LE28$1:
	movb	%al,CICCIAF+0x0
/	LD	T#500MS
$LE29$1:
	movl	$0x1f4,%eax
/	ST	CICCIAF.PT
$LE30$1:
	movl	%eax,CICCIAF+0x1
/	CAL	CICCIAF	 
$LE31$1:
	pushl	$CICCIAF
	call	TOF
	addl	$0x4,%esp
/	LD	CICCIAF.Q
$LE32$1:
.L7:
	movzbl	CICCIAF+0x5,%eax
/	ST	CICCIA
$LE33$1:
	movb	%al,CICCIA
/
$LE34$1:
/END_PROGRAM
$LE35$1:
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
$LL36$1:
.L3:
	movl	-4(%ebp),%esi
	movl	%ebp,%esp
	popl	%ebp
	ret
.LS1:
	.byte	0x31,0x2e,0x30,0x2e,0x30,0x2e,0x30,0x0
	.align	4
.L1:
$EF626973746162696C652E706C63:
