#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
	int start = atoi(argv[1]);
	int i;
	for (i = 0; i < start; i++) {
		int child = fork();//only works for the initial fork and all child processes
		if (child == -1 ) return -1; //something went wrong
		else if (child > 0) { //parent process
			break;
		}
	}
	wait(NULL); //wait until all the child processes are done making forks
	printf("%d\n", i);
}

