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
    int mem, error; 
    char caracter;

    mem = open(MEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (mem == -1) {
        printf("Error creando la memoria compartida.\n");
        exit(EXIT_FAILURE);
    }

    error = ftruncate(mem, sizeof(ColaCircular));
    if (error == -1) {
        fprintf(stderr, "Error cambiando el tamaño de la memoria compartida.\n");
        unlink(MEM);
        close(mem);
        exit(EXIT_FAILURE);
    }

    cola_circular = mmap(NULL, sizeof(ColaCircular), PROT_READ | PROT_WRITE, MAP_SHARED, mem, 0);
    if (cola_circular == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        unlink(MEM);
        close(mem);
        exit(EXIT_FAILURE);
    }
    close(mem);

    cola_circular_inicializar(cola_circular);

    if ((sem = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
        perror("sem_open");
        unlink(MEM);
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    if ((items = sem_open(ITEMS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(SEM);
        sem_close(sem);
        unlink(MEM);
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    if ((slots = sem_open(SLOTS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        sem_unlink(ITEMS);
        sem_close(items);
        sem_unlink(SEM);
        sem_close(sem);
        unlink(MEM);
        munmap(cola_circular, sizeof(ColaCircular));
        exit(EXIT_FAILURE);
    }

    while (1) {
        sem_wait(slots); // Comprueba que haya algún hueco libre

        sem_wait(sem); // Inicio de la zona crítica

        scanf("%c", &caracter);
        if (feof(stdin)) { // Si la entrada finaliza se inserta un 0 y se termina
            cola_circular_insertar(cola_circular, '\0');
            sem_post(items);
            sem_post(sem); // Final de la zona crítica en caso de terminar

            sem_unlink(SLOTS);
            sem_close(slots);
            sem_unlink(ITEMS);
            sem_close(items);
            sem_unlink(SEM);
            sem_close(sem);
            unlink(MEM);
            munmap(cola_circular, sizeof(ColaCircular));
            exit(EXIT_SUCCESS);
        }
        cola_circular_insertar(cola_circular, caracter); // Inserta un elemento
        sem_post(items); // Indica que hay un elemento más para consumir

        sem_post(sem); // Final de la zona crítica
    }
}