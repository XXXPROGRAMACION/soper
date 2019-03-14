#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define N_ITER 5
#define SECS 40

void manejador(int sig) {
    printf("Soy %d y he recibido la señal SIGTERM\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

int main(void) {
    pid_t pid;
    int counter;
    struct sigaction act;

    act.sa_handler = manejador;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        if (sigaction(SIGTERM, &act, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }
        while (1) {
            for (counter = 0; counter < N_ITER; counter++) {
                printf("%d\n", counter);
                sleep(1);
            }
            sleep(3);
        }
    } else {
        sleep(SECS);
        kill(pid, SIGTERM);
        wait(NULL);
    }
}