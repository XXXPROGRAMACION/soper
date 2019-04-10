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

#define MEM "mem"

#define SEM "sem"
#define ITEMS "items"
#define SLOTS "slots"

int main() {
    ColaCircular *cola_circular;
    sem_t *sem, *items, *slots;
    int mem, i; 
    char caracter;

    mem = open(MEM, O_RDWR, 0);
    if (mem == -1) {
        printf("Error creando la memoria compartida.\n");
        exit(EXIT_FAILURE);
    }

    cola_circular = mmap(NULL, sizeof(ColaCircular), PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
    if (cola_circular == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        close(mem);
        exit(EXIT_FAILURE);
    }
    close(mem);

    cola_circular_inicializar(cola_circular);

    if ((sem = sem_open(SEM, 0)) == SEM_FAILED) {
        perror("sem_open");
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    if ((items = sem_open(ITEMS, 0)) == SEM_FAILED) {
        perror("sem_open");
        sem_close(sem);
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    if ((slots = sem_open(SLOTS, 0)) == SEM_FAILED) {
        perror("sem_open");
        sem_close(items);
        sem_close(sem);
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < TAM; i++) sem_post(slots);

    while (1) {
        sem_wait(items);

        sem_wait(sem);

        caracter = cola_circular_extraer(cola_circular);
        if (caracter == '\0') {
            printf("\n");

            sem_close(slots);
            sem_close(items);
            sem_close(sem);
            munmap(cola_circular, sizeof(ColaCircular));
            exit(EXIT_SUCCESS);
        }
        printf("%c", caracter);
        sem_post(slots);

        sem_post(sem);
    }
}