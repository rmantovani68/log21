#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _DOC
function checkLuhn(string purportedCC) 
{
    int sum := integer(purportedCC[length(purportedCC)-1])
    int nDigits := length(purportedCC)
    int parity := nDigits modulus 2
    for i from 0 to nDigits - 2 {
        int digit := integer(purportedCC[i])
        if i modulus 2 = parity
            digit := digit × 2
        if digit > 9
            digit := digit - 9
        sum := sum + digit
    }
    return (sum modulus 10) = 0
}
#endif

int checkLuhn(char *purportedCC) 
{
	int sum = (purportedCC[strlen(purportedCC)-1]);
	int nDigits = strlen(purportedCC);
	int parity = nDigits % 2;

	for(int i=0; i < nDigits - 2; i++)
	{
		int digit = purportedCC[i]-'0';
		if (i % 2 == parity)
			digit = digit * 2;
		if (digit > 9)
			digit = digit - 9 ;
		sum = sum + digit;
	}
	return (sum % 10);
}

int checkdigit_modulo_10(char *a)
{
	int i,len,mul,sum;
	int m10;

	len = strlen(a);

	mul = 2; sum=0;

	for(i=len-1;i>=0;i--) {
		if( ((a[i]-'0')*mul) >= 10 )
			sum += (((a[i]-'0')*mul ) / 10) +(((a[i]-'0')*mul ) % 10 );
		else
			sum += (a[i]-'0')*mul;

		if(mul==2) 
			mul=1;
		else 
			mul=2;
	}

	m10 = sum % 10;

	if(m10) 
		m10 = 10 - m10;

	return m10;
}

int main(int argc, char **argv)
{
	char string[128];

	strcpy(string,"123456789");            printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"87436856346");          printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"985743956743");         printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"32463651237");          printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"0345985389098");        printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"2365423542537865423");  printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"92836538653468");       printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"098361763246875436");   printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"329564864236328689");   printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));
	strcpy(string,"437658365836534868");   printf("checkdigit : [%s] = %d\n", string, checkdigit_modulo_10(string));

	return 0;
}
