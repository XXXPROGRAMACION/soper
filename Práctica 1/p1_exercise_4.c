#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_PROG 3

int main() {
    pid_t pid;
    int i;

    for (i = 0; i < NUM_PROG; i++) {
        pid = fork();
        if (pid != 0) break;
    }

    wait(NULL);

    if (i == 0) printf("Soy el padre\n");
    else printf("Soy el hijo %d\n", i);

    return 0;
}