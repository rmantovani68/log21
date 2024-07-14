	.file	"bistabile.msd"
	.text
ilc.compiled:
$BF626973746162696C652E6D7364:
	.long	.L1,.L2,BISTABILE_CONTROL,0x0
.L2:
	.long	0x0,0x41414141
.L3:
	.asciz	"BISTABILE_CONTROL"
.L4:
	.asciz	"UNICA"
.L5:
	.asciz	"SOLA_CPU_PRESENTE"
	.data
G:
	.text
.L6:
	.long	G,BISTABILE
	.long	0x0,0x41414141
.L7:
	.long	.L6,0x0
.L8:
	.long	.L4,.L5,.L7
.L9:
	.long	.L8,0x0
BISTABILE_CONTROL:
	.long	.L3,.L9
	.align	4
.L1:
$EF626973746162696C652E6D7364:
