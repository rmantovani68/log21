
/*******************************************************/
/*  @(#) ilcn.s 1.0 Tue Jul 23 09:44:02 MET DST 1996   */
/*                                                     */
/* Codice di fine eseguibile PLC.                      */
/* Deve essere l'ultimo oggetto del link dei programmi */
/* compilati da "ilc". Eventuali librerie compilate da */
/* "ilc" dovranno apparire prima di questo file. Per   */
/* contro, librerie od oggetti non compilati con "ilc" */
/* DOVRANNO comparire dopo.                            */
/*******************************************************/

	.text

end_of_ilc_space:
	.long	0
	.long	0
	.long	0
	.long	0
