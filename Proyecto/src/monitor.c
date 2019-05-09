#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>

#include "gamescreen.h"
#include "mapa.h"
#include "types.h"

void mapa_print(tipo_mapa *mapa);
void monitor_configurar_semaforos(sem_t **sem_monitor);
void monitor_configurar_memoria_compartida(int *shm, tipo_mapa **mapa);

int main() {
	sem_t *sem_monitor = NULL;
    int shm = -1;
    tipo_mapa *mapa = NULL;

	monitor_configurar_semaforos(&sem_monitor);
	monitor_configurar_memoria_compartida(&shm, &mapa);

	sem_wait(sem_monitor);

	screen_init();

	while (true) {
		mapa_print(mapa);
		usleep(SCREEN_REFRESH);
	}

	screen_end();

	exit(EXIT_SUCCESS);
}

void mapa_print(tipo_mapa *mapa) {
	int i, j;

	for (i = 0; i < MAPA_MAX_Y; i++) {
		for (j = 0; j < MAPA_MAX_X; j++) {
			tipo_casilla cas = mapa_get_casilla(mapa, i, j);
			screen_addch(i, j, cas.simbolo);
		}
	}

	screen_refresh();
}

void monitor_configurar_semaforos(sem_t** sem_monitor) {
    // Configura el semáforo del monitor
    if ((*sem_monitor = sem_open(SEM_MONITOR, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
}

void monitor_configurar_memoria_compartida(int *shm, tipo_mapa **mapa) {
    *shm = shm_open(SHM, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (*shm == -1) {
        printf("Error creando la memoria compartida.\n");
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(*shm, sizeof(tipo_mapa)) == -1) {
        fprintf(stderr, "Error cambiando el tamaño de la memoria compartida.\n");
        unlink(SHM);
        close(*shm);
        exit(EXIT_FAILURE);
    }

    *mapa = mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, MAP_SHARED, *shm, 0);
    if (*mapa == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        shm_unlink(SHM);
        close(*shm);
        exit(EXIT_FAILURE);
    }

    close(*shm);
}