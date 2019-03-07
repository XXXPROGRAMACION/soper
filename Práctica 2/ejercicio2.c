#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int pid;
    int i;

    for (i = 0; i < 4; i++) {
        pid = fork();

        if (pid == 0) {
            pid = getpid();
            printf("Soy el proceso hijo %d\n", pid);
            sleep(30);
            printf("Soy el proceso %d y ya me toca terminar\n", pid);
            exit(EXIT_SUCCESS);
        } else {
            sleep(5);
            kill(pid, SIGTERM);
        }
    }

    while (wait(NULL) > 0);

    return 0;
}