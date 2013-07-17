#include <stdio.h>
#include <signal.h>

int toExitCtr = 0;

void sig_func(int sig)
{
	toExitCtr++;
 	printf("Caught signal SIGINT (%d)\n", toExitCtr);
}

int main()
{
 	signal(SIGINT, sig_func);
	
	for (;;) {
		printf("loop \n");
		sleep(1);

		if (3 <= toExitCtr) {
			printf("Exitting...\n"); 
			break;
		}
	}

	return 0;
}
