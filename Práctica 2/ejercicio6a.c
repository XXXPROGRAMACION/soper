#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5
#define SECS 40


int main(void) {
    pid_t pid;
    int counter;
    sigset_t set1, set2, oldset;

    sigemptyset(&set1);
    sigaddset(&set1, SIGUSR1);
    sigaddset(&set1, SIGUSR2);
    sigaddset(&set1, SIGALRM);

    sigemptyset(&set2);
    sigaddset(&set2, SIGUSR1);
    sigaddset(&set2, SIGALRM);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        if (alarm(SECS))
            fprintf(stderr, "Existe una alarma previa establecida\n");
        while (1) {
            if (sigprocmask(SIG_BLOCK, &set1, &oldset) < 0) {
                perror("sigprocmask");
                exit(EXIT_FAILURE);
            }
            for (counter = 0; counter < N_ITER; counter++) {
                printf("%d\n", counter);
                sleep(1);
            }
            if (sigprocmask(SIG_UNBLOCK, &set2, &oldset) < 0) {
                perror("sigprocmask");
                exit(EXIT_FAILURE);
            }
            sleep(3);
        }
    }
    while(wait(NULL) > 0);
}