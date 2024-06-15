	.file	"test.msd"
	.text
ilc.compiled:
$BF746573742E6D7364:
	.long	.L1,.L2,NONAME_CONTROL,0x0
.L2:
	.long	IN1
	.byte	0x1,0xe
	.asciz	"IOS.0.0.1.0.8"
	.long	IN2
	.byte	0x1,0xe
	.asciz	"IOS.0.0.1.0.9"
	.long	IN3
	.byte	0x1,0xf
	.asciz	"IOS.0.0.1.0.11"
	.long	IN4
	.byte	0x1,0xf
	.asciz	"IOS.0.0.1.0.12"
	.long	IN5
	.byte	0x1,0xf
	.asciz	"IOS.0.0.1.0.13"
	.long	TEST
	.byte	0x1,0xf
	.asciz	"IOS.0.0.1.0.48"
	.long	OUT2
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.48"
	.long	OUT3
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.49"
	.long	OUT4
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.50"
	.long	OUT5
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.51"
	.long	OUT6
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.52"
	.long	USCI
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.53"
	.long	USCI2
	.byte	0x2,0xf
	.asciz	"IOS.0.0.1.0.54"
	.long	0x0,0x41414141
.L3:
	.asciz	"NONAME_CONTROL"
	.data
	.globl	IN1
IN1:
	.byte	0x00
	.globl	IN2
IN2:
	.byte	0x00
	.globl	IN3
IN3:
	.byte	0x00
	.globl	IN4
IN4:
	.byte	0x00
	.globl	IN5
IN5:
	.byte	0x00
	.globl	TEST
TEST:
	.byte	0x00
	.globl	OUT2
OUT2:
	.byte	0x00
	.globl	OUT3
OUT3:
	.byte	0x00
	.globl	OUT4
OUT4:
	.byte	0x00
	.globl	OUT5
OUT5:
	.byte	0x00
	.globl	OUT6
OUT6:
	.byte	0x00
	.globl	USCI
USCI:
	.byte	0x00
	.globl	USCI2
USCI2:
	.byte	0x00
	.text
.L4:
	.asciz	"UNICA"
.L5:
	.asciz	"SOLA_CPU_PRESENTE"
	.data
G:
	.text
.L6:
	.long	G,NONAME
	.long	0x0,0x41414141
.L7:
	.long	.L6,0x0
.L8:
	.long	.L4,.L5,.L7
.L9:
	.long	.L8,0x0
NONAME_CONTROL:
	.long	.L3,.L9
	.align	4
.L1:
$EF746573742E6D7364:
