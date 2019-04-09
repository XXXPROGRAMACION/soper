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
    int fd_shm; 
    char caracter;

    fd_shm = shm_open(MEM, O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_shm == -1) {
        printf("Error creando la memoria compartida.\n");
        return 1;
    }

    cola_circular = mmap(NULL, sizeof(ColaCircular), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (cola_circular == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        close(fd_shm);
        return 1;
    }

    cola_circular_inicializar(cola_circular);

    if ((sem = sem_open(SEM, O_CREAT | O_EXCL)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }

    if ((items = sem_open(ITEMS, O_CREAT | O_EXCL)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }

    if ((slots = sem_open(SLOTS, O_CREAT | O_EXCL)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(items);

        sem_wait(sem);

        caracter = cola_circular_extraer(cola_circular);
        if (caracter == '\0') {
            sem_close(sem);
            sem_close(items);
            sem_close(slots);
            close(fd_shm);
            exit(EXIT_SUCCESS);
        }
        printf("%c", caracter);

        sem_post(sem);
    }
}