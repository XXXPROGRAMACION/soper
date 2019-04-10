#include "cola_circular.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SEM "sem"
#define ITEMS "items"
#define SLOTS "slots"

#define MEM "mem"

int main() {
    ColaCircular *cola_circular;
    sem_t *sem, *items, *slots;
    int fd_shm, error; 
    char caracter;

    fd_shm = shm_open(MEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd_shm == -1) {
        printf("Error creando la memoria compartida.\n");
        return 1;
    }
    shm_unlink(MEM);

    error = ftruncate(fd_shm, sizeof(ColaCircular));
    if (error == -1) {
        fprintf(stderr, "Error cambiando el tamaño de la memoria compartida.\n");
        shm_unlink(MEM);
        return EXIT_FAILURE;
    }

    cola_circular = mmap(NULL, sizeof(ColaCircular), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (cola_circular == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        close(fd_shm);
        return 1;
    }

    cola_circular_inicializar(cola_circular);

    if ((sem = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }
    sem_unlink(SEM);

    if ((items = sem_open(ITEMS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }
    sem_unlink(ITEMS);

    if ((slots = sem_open(SLOTS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, TAM)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }
    sem_unlink(SLOTS);

    while (1) {
        sem_wait(slots);

        sem_wait(sem);

        scanf("%c", &caracter);        
        if (feof(stdin)) {
            cola_circular_insertar(cola_circular, '\0');
            sem_post(items);
            printf("-->Fin\n");

            sem_close(sem);
            sem_close(items);
            sem_close(slots);
            close(fd_shm);
            exit(EXIT_SUCCESS);
        }
        cola_circular_insertar(cola_circular, caracter);
        printf("-->%c\n", caracter);
        sem_post(items);

        sem_post(sem);
    }
}