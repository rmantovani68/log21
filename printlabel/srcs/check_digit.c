#include <stdio.h>

char *check_digit(char *szString)
{
	int nSum;
	int nIndex;
	int nCheckDigit;
	char szCheckDigit[128];

	/* 
	* calcolo il check digit secondo metodo qui documentato : 
	* riga GSIN : https://www.gs1.org/services/how-calculate-check-digit-manually 
	*/
	printf("lunghezza : %d\n", strlen(szString));

	nSum = 0;
	for(nIndex=0; nIndex < strlen(szString); nIndex++){
		if(nIndex % 2){
			/* cifre pari x1 */
			nSum += (szString[nIndex]-'0');
		} else {
			/* cifre dispari x3 */
			nSum += (szString[nIndex]-'0')*3;
		}
	}

	printf("primo passaggio : %d\n", nSum);
	/* check digit = complemento base 10 della unità : esempio 57 -> 3 */
	nCheckDigit = 10 - ( nSum % 10 );
	printf("secondo passaggio : %d\n", nCheckDigit);

	/* nel caso in cui l'operazione risulti 10 ...  esempio 50 -> 0 */
	nCheckDigit %= 10;
	printf("terzo passaggio : %d\n", nCheckDigit);

	sprintf(szCheckDigit,"%d",nCheckDigit);

	strcat(szString,szCheckDigit);


	return szString;
}

void main(int argc, char **argv)
{
	if(argc){
		printf("%s\n", check_digit(argv[1]));
	}
}
