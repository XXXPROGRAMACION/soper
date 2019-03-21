#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>

#define N_PROC 10
#define NUM_MAX 15
#define SEM_FICHERO "/sem_fichero"
#define SEM_HIJOS "/sem_hijos"
#define FICHERO "carrera.txt"

void manejador_SIGTERM(int sig) {
    while (wait(NULL) > 0);
    exit(EXIT_SUCCESS);
}

// LA CARRERA NO COMIENZA DE MANERA SINCRONIZADA

int main() {
    pid_t pid;
    int id, num, ganador, vuelta, i;
    sem_t *sem_fichero = NULL;
    sem_t *sem_hijos = NULL;
    FILE *file = NULL;
    int *vueltas;
    struct sigaction act;

    if ((sem_fichero = sem_open(SEM_FICHERO, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    if ((sem_hijos = sem_open(SEM_HIJOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    sem_post(sem_fichero);
    sem_post(sem_hijos);

    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    act.sa_handler = manejador_SIGTERM;
    if (sigaction(SIGTERM, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < N_PROC; i++) {
        pid = fork();
        if (pid < 0) {
            sem_unlink(SEM_FICHERO);
            sem_unlink(SEM_HIJOS);
            sem_close(sem_fichero);
            sem_close(sem_hijos);
            kill(0, SIGTERM);
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            id = i;
            break;
        }
    }

    if (pid == 0) {
        while (1) {
            sem_wait(sem_hijos);
            sem_wait(sem_fichero);

            file = fopen(FICHERO, "a");            
            if (file == NULL) {
                sem_unlink(SEM_FICHERO);
                sem_unlink(SEM_HIJOS);
                sem_close(sem_fichero);
                sem_close(sem_hijos);
                kill(0, SIGTERM);
            }

            fprintf(file, "%d\n", id);
            
            fclose(file);

            sem_post(sem_fichero);
            sem_post(sem_hijos);

            usleep(rand()%100001);
        }
    } else {
        vueltas = (int *) calloc(N_PROC, sizeof(int));
        ganador = -1;
        vuelta = 0;
        while (1) {
            sem_wait(sem_fichero);

            vuelta++;

            file = fopen(FICHERO, "r");
            if (file == NULL) {
                free(vueltas);
                sem_unlink(SEM_FICHERO);    
                sem_unlink(SEM_HIJOS);
                sem_close(sem_fichero);
                sem_close(sem_hijos);
                kill(0, SIGTERM);
            } 

            while (1) {
                fscanf(file, "%d", &num);
                if (feof(file)) break;
                vueltas[num]++;
                if (vueltas[num] == 20 && ganador == -1) {
                    ganador = num;
                }
            }

            freopen(FICHERO, "w", file);
            fclose(file);

            printf("\nVuelta %d:\n", vuelta);

            for (i = 0; i < N_PROC; i++) {
                printf("Proceso %d: %d vueltas\n", i, vueltas[i]);
            }

            if (ganador != -1) {
                printf("\nGanador: proceso %d\n", ganador);                
                free(vueltas);
                sem_unlink(SEM_FICHERO);    
                sem_unlink(SEM_HIJOS);
                sem_close(sem_fichero);
                sem_close(sem_hijos);
                kill(0, SIGTERM);
            }

            sem_post(sem_fichero);
            sleep(1);
        }
    }
}