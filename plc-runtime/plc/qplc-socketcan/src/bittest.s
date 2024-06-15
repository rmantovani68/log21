

/*
 VENGONO USATE per settare resettare bit degli I/O mappati in memoria 
---------------------------------------------------------------------
int bitr(char *byte, int bit);		reset
int bits(char *byte, int bit);		set
int bitc(char *byte, int bit);		complemento

	Queste tre funzioni scritte in assembler permettono di settare
	resettare,complementare bit di dati condivisi dal loop e dalla
	scansione senza pericolo di creare copie incoerenti. 

	N.B. Usare bit da 0 a 7 (potrebbe variare da 0 a 15 ma non la
	si usa in questo modo )
	AVVERTENZA IMPORTANTE
	L'indirizzo del dato da manipolare non puo' essere l'ultimo
	della ram allocata al processo perche' l'istruzione assembler
	carica anche il byte successivo e  il sistema darebbe MEMORY FAULT. 
*/

	.globl	bitr

bitr:
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	btrw	%ax,(%edx)
	sbbl	%eax,%eax
	ret

	.globl	bits

bits:
	movl	4(%esp),%edx
	movl	8(%esp),%eax
	btsw	%ax,(%edx)
	sbbl	%eax,%eax
	ret

