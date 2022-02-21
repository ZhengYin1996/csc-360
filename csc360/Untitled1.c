#include <unistd.h>            
#include <string.h>            
#include <ctype.h>             
#include <stdio.h>             
#include <stdlib.h>            
#include <sys/types.h>              
#include <signal.h>            
#include <stdlib.h>
int main(void)
{
	int i =3;
	pid_t pid= fork();
	if(pid!=0)
	{
		i++;
		pid = fork();
		i+=3;
	}
	else{
		i+=4;
		pid = fork();
		i+=5;
	}
	printf("%d\n", i);
	return 1;
}
