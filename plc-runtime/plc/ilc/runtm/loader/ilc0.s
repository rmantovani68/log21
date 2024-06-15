
/*******************************************************/
/*  @(#) ilc0.s 2.2 Tue Oct 15 16:53:22 MET 1996       */
/*                                                     */
/* Codice di inizio eseguibile PLC.                    */
/* Deve essere il primo oggetto del link dei programmi */
/* compilati da "ilc".                                 */
/*                                                     */
/* 23/07/96 GG 1.0 Prima stesura.                      */
/* 08/10/96 GG 2.0 Incluso qui il codice contenete il  */
/*             nucleo del lanciatore del programma.    */
/*             Funziona solo con qplc 1.3.0.0 o piu`.  */
/* 10/10/96 GG 2.1 (4.0.0.1) Aggiornata la versione    */
/*             per modifiche apportate alla libreria.  */
/* 15/10/96 GG 2.2 Utilizzato il meccanismo consueto   */
/*             per la generazione della versione.      */
/*******************************************************/

	.text

	.global	module_name
module_name:
	.asciz	"U_CODE"

	.global	UserStart
UserStart:
	popl	%eax
	pushl	$il_module
	pushl	%eax
	jmp	IL_UserStart

	.global	UserStop
UserStop:
	jmp	IL_UserStop

	.global	UserMain
UserMain:
	jmp	IL_UserMain

	.global	il_module
il_module:
	.long	first_user_object
	.asciz	"IL_CODE"
	.align	4
first_user_object:
