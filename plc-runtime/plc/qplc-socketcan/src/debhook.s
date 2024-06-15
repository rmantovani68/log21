/**************************************************/
/* @(#) debhook.s 1.0 Tue Dec 3 12:34:51 MET 1996 */
/*                                                */
/* Primitive a basso livello per il debugger in   */
/* tempo reale. Le funzoni qui raccolte interven- */
/* gono direttamente sul codice macchina del pro- */
/* gramma in esecuzione, e vi si "agganciano" per */
/* registrare lo stato della CPU.                 */
/*                                                */
/* 03/12/96 GG 1.0 Prima versione.                */
/**************************************************/

	.text

/**************************************************/
/* Funzione "brk_hook"                            */
/*                                                */
/* Questa funzione e` chiamata dal punto del pro- */
/* gramma nel quale si vuole analizzare lo stato  */
/* di esecuzione.                                 */
/**************************************************/

	.globl	brk_hook
brk_hook:
	pushal
	pushfl
	movl	%esp,%eax
	pushl	%eax
	call	brkCore
	addl	$4,%esp
	popfl
	popal
	ret

