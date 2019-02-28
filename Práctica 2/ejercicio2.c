#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int pid;
    int i;
    struct sigaction act;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;


    for (i = 0; i < 4; i++) {
        pid = fork();

        if (pid != 0) {
            pause()
        }
    }
}