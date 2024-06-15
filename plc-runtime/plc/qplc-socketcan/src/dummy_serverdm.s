/
/ @(#) dummy_serverdm.s Wed Mar 18 12:07:55 MET 1998
/
/ Questo file serve a "buttar via" alcune funzioni inutili contenute
/ nella libreria del server dei dati macchina.
/

	.text
	.globl	sdmScktSendAndWaitString
sdmScktSendAndWaitString:
	.globl	sdmScktNewTx
sdmScktNewTx:
	.globl	sdmScktSendAndWaitRc
sdmScktSendAndWaitRc:
	.globl	sdmScktGetAndSetArr
sdmScktGetAndSetArr:
	.globl	sdmScktGetAndSetData
sdmScktGetAndSetData:
	.globl	sdmScktGetConfInfo
sdmScktGetConfInfo:
	.globl	sdmScktGetDataInfo
sdmScktGetDataInfo:
	.globl	sdmScktSimplyRc
sdmScktSimplyRc:
	.globl	sdmScktGetSelected
sdmScktGetSelected:
	.globl	sdmScktFullRc
sdmScktFullRc:
	.globl	ScktMdata
ScktMdata:
	.globl	sdmScktRecv
sdmScktRecv:
	.globl	sdmScktSend
sdmScktSend:
	.globl	sdmScktClose
sdmScktClose:
	.globl	sdmScktOpen
sdmScktOpen:
	.globl	sdmCheckScktCmd
sdmCheckScktCmd:

	xorl	%eax,%eax
	ret

