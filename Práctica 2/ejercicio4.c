#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define N_PROC 5

void manejador_padre(int sig) {
    printf("¡Todos los participantes listos!\n");
    fflush(stdout);
}

void manejador_gestor(int sig) {
    printf("Participante listo\n");
    fflush(stdout);
}

void manejador_participante(int sig) {
    printf("Proceso %d: ¡He capturado la señal!\n", getpid());
    fflush(stdout);
    exit(EXIT_SUCCESS);
}

void manejador_SIGUSR1(int sig) {

}

int main() {
    int pid;
    int i;
    struct sigaction act;
    
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    pid = fork();

    if (pid == 0) { //Gestor
        act.sa_handler = manejador_gestor;
        if (sigaction(SIGUSR2, &act, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }        

        for (i = 0; i < N_PROC; i++) {
            pid = fork();
            if (pid == 0) { //Participante
                act.sa_handler = manejador_participante;
                if (sigaction(SIGUSR1, &act, NULL) < 0) {
                    perror("sigaction");
                    exit(EXIT_FAILURE);
                }
                kill(getppid(), SIGUSR2);
                pause();
                exit(EXIT_FAILURE);
            } else { //Gestor                
                pause();
            }
        }

        act.sa_handler = manejador_SIGUSR1;
        if (sigaction(SIGUSR1, &act, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        kill(getppid(), SIGUSR2);

        pause();

        while (wait(NULL) > 0);

        exit(EXIT_SUCCESS);
    } else {
        act.sa_handler = manejador_padre;
        if (sigaction(SIGUSR2, &act, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        act.sa_handler = manejador_SIGUSR1;
        if (sigaction(SIGUSR1, &act, NULL) < 0) {
            perror("sigaction");
            exit(EXIT_FAILURE);
        }

        pause();

        sleep(1);
        printf("Empieza en... ");
        fflush(stdout);
        sleep(1);
        printf("3... ");
        fflush(stdout);
        sleep(1);
        printf("2... ");
        fflush(stdout);
        sleep(1);
        printf("1... \n");
        fflush(stdout);
        sleep(1);

        kill(0, SIGUSR1);

        wait(NULL);
    }

    return 0;
}