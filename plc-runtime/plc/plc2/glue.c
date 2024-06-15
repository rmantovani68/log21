
int invXerror(void *v,...)
{
	return 0;
}
int _dummy_XcgLiteClueAddWidget(void *v, ...)
{
	return 0;
}

int initXerror(void)
{
	return 0;
}
int initStdout(void)
{
	return 0;
}
int initStderr(void)
{
	return 0;
}

void util_fatal(void)
{
	printf("Fatal error!\n");
	exit(1);
}
void xncMovePointerTo(void *v)
{
}
void invVersione(char *s1,int n, char *s2)
{
	printf("%s (%d) VERSION %s\n",s1,n,s2);
}
char *GetMessageString(void *v, char *s)
{
	return s;
}

