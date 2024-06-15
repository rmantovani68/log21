int main(int argc, char **argv)
{
	int  pid;

	pid=fork();
	if(pid==0){
		execv("ps",0);
	}
}
