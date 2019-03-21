#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

#define N_READ 3
#define SECS 3

#define SEM_LECTURA "/sem_lectura"
#define LECTORES "/lectores"
#define SEM_ESCRITURA "/sem_escritura"

void manejador_SIGINT(int sig) {
    sem_unlink(SEM_LECTURA);
    sem_unlink(LECTORES);
    sem_unlink(SEM_ESCRITURA);
    kill(0, SIGTERM);
    while (wait(NULL) > 0);
    exit(EXIT_SUCCESS);
}

void manejador_SIGTERM(int sig) {
    exit(EXIT_SUCCESS);
}

int main() {
    pid_t pid;
    sem_t *sem_lectura = NULL;
    sem_t *lectores = NULL;
    sem_t *sem_escritura = NULL;
    int n_lectores, i;
    struct sigaction act;

    if ((sem_lectura = sem_open(SEM_LECTURA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((lectores = sem_open(LECTORES, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((sem_escritura = sem_open(SEM_ESCRITURA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_post(sem_lectura);
    sem_post(sem_escritura);

    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    act.sa_handler = manejador_SIGINT;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    act.sa_handler = manejador_SIGTERM;
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N_READ; i++) {
        pid = fork();
        if (pid == -1) {
            printf("fork");
            sem_unlink(SEM_LECTURA);
            sem_unlink(LECTORES);
            sem_unlink(SEM_ESCRITURA);
            kill(0, SIGTERM);
            exit(EXIT_FAILURE);
        }
        if (pid == 0) break;
    }

    if (pid == 0) {
        pid = getpid();
        while (1) {
            sem_wait(sem_lectura);
            sem_post(lectores);
            sem_getvalue(lectores, &n_lectores);
            if (n_lectores == 1) {
                sem_wait(sem_escritura);
            }
            sem_post(sem_lectura);

            printf("R-INI %d\n", pid);
            sleep(1);
            printf("R-FIN %d\n", pid);

            sem_wait(sem_lectura);
            sem_wait(lectores);
            sem_getvalue(lectores, &n_lectores);
            if (n_lectores == 0) {
                sem_post(sem_escritura);
            }
            sem_post(sem_lectura);

            sleep(SECS);
        }        
    } else {
        pid = getpid();
        while (1) {
            sem_wait(sem_escritura);

            printf("W-INI %d\n", pid);
            sleep(1);
            printf("W-FIN %d\n", pid);

            sem_post(sem_escritura);

            sleep(SECS);
        }
    }
}