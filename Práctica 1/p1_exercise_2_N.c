#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h> 

int main() {
    int pid;
    pid = fork();

    if (pid != 0) {
        wait(NULL);
        printf("Padre\n");
    } else {
        printf("Hijo\n");
    }
    return 0;
}