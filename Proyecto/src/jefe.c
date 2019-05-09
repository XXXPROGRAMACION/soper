#include "jefe.h"

void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2]);

void jefe(tipo_mapa *mapa, int n_equipo, int fd_sim[2], sem_t *sem_equipos_listos) {
	int fd_jefe[N_NAVES][2];
    char buffer[BUFFER_SIZE];
    int i, j;

    close(fd_sim[1]);

    jefe_crear_naves(mapa, n_equipo, fd_jefe);

    printf("Jefe %d: equipo %d listo\n", n_equipo+1, n_equipo+1);
    sem_post(sem_equipos_listos);

    while (true) {
        read(fd_sim[0], buffer, sizeof(buffer));
        if (!strcmp(buffer, MENSAJE_TURNO_NUEVO)) {
            //Turno nuevo
            sem_post(sem_equipos_listos);
            for (i = 0; i < N_NAVES; i++) {
                for (j = 0; j < 2; j++) {
                    if (rand()%2 == 0) {
                        strcpy(buffer, MENSAJE_MOVER_NAVE);
                    } else {
                        strcpy(buffer, MENSAJE_ATACAR_NAVE);
                    }
                    
                    write(fd_jefe[i][1], buffer, sizeof(buffer));
                }
            }
        }
    }

    exit(EXIT_SUCCESS);
}

void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2]) {
    int pipe_ret, pid, i;

    for (i = 0; i < N_NAVES; i++) {
        // Crea las tuberías
        pipe_ret = pipe(fd_jefe[i]);
        if (pipe_ret == -1) {
            printf("Error creando los pipes entre el jefe y las naves\n");
            kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid == 0) nave(mapa, n_equipo, i, fd_jefe[i]);
        close(fd_jefe[i][0]);
    }
}