
/*******************************************************/
/*  @(#) ilc0mix.s 1.0 Fri Nov 29 17:34:22 MET 1996    */
/*                                                     */
/* Codice di inizio eseguibile PLC (versione "mix",    */
/* cioe` adatta a produrre moduli user.oo che lancino  */
/* sia il programma IL, sia quello in stile 480).      */
/* Deve essere il secondo nel link dei programmi       */
/* compilati da "ilc". Il primo oggetto deve essere    */
/* "plc480mix.o" (vedere ~/qplc/src).                  */
/*                                                     */
/* 29/11/96 GG 1.0 Prima stesura.                      */
/*******************************************************/

	.text

	.global	module_name
module_name:
	.asciz	"U_CODE"

	.global	MIX_UserStart
MIX_UserStart:
	popl	%eax
	pushl	$il_module
	pushl	%eax
	jmp	IL_UserStart

	.global	MIX_UserStop
MIX_UserStop:
	jmp	IL_UserStop

	.global	MIX_UserMain
MIX_UserMain:
	jmp	IL_UserMain

	.global	il_module
il_module:
	.long	first_user_object
	.asciz	"IL_CODE"
	.align	4
first_user_object:
