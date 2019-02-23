#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_PROC 3

int main(void)
{
	pid_t pid;
	int i;
	for(i = 0; i < NUM_PROC; i++)
	{
		pid = fork();
		if(pid <  0)
		{
			printf("Error al emplear fork\n");
			exit(EXIT_FAILURE);
		}
		else if(pid == 0)
		{
			printf("HIJO:\n\tMi pid: %d\n\tEl de mi padre: %d\n", getpid(), getppid());
			exit(EXIT_SUCCESS);
		}
		else if(pid > 0)
		{
			wait(NULL);
			printf("PADRE %d\n", i);
		}
	}
	wait(NULL);
	exit(EXIT_SUCCESS);
}

