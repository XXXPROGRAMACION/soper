#include "jefe.h"

void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2], sem_t *sem_equipos_listos, pid_t pids_naves[N_NAVES]);

void jefe(tipo_mapa *mapa, int n_equipo, int fd_sim[2], sem_t *sem_equipos_listos) {
    pid_t pids_naves[N_NAVES];
	int fd_jefe[N_NAVES][2];
    char buffer[BUFFER_SIZE];
    int i, j, num_nave_destruir;

    close(fd_sim[1]);

    jefe_crear_naves(mapa, n_equipo, fd_jefe, sem_equipos_listos, pids_naves);
    
    sem_post(sem_equipos_listos);

    srand(time(NULL)+n_equipo);

    while (true) {
        read(fd_sim[0], buffer, sizeof(buffer));

        if (!strcmp(buffer, TURNO)) {
            //Turno nuevo
            for (i = 0; i < N_NAVES; i++) {
                if (!mapa_get_nave(mapa, n_equipo, i).viva) continue; 

                for (j = 0; j < 2; j++) {
                    if (rand()%2 == 0) {
                        strcpy(buffer, MOVER_ALEATORIO);
                    } else {
                        strcpy(buffer, ATACAR);
                    }
                    
                    write(fd_jefe[i][1], buffer, sizeof(buffer));
                }
            }

            sem_post(sem_equipos_listos);
        } else if (!strcmp(buffer, FIN)) {
            close(fd_sim[0]);
            sem_close(sem_equipos_listos);
            for (i = 0; i < N_NAVES; i++) {
                close(fd_jefe[i][1]);
                kill(pids_naves[i], SIGTERM);
            }

            while (wait(NULL) > 0);

            exit(EXIT_SUCCESS);
        } else {
            //Destruir nave
            sscanf(buffer, "%*s %d", &num_nave_destruir);
            strcpy(buffer, DESTRUIR);
            write(fd_jefe[num_nave_destruir][1], buffer, sizeof(buffer));
        }
    }

    exit(EXIT_SUCCESS);
}

void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2], sem_t *sem_equipos_listos, pid_t pids_naves[N_NAVES]) {
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
        if (pid == 0) nave(mapa, n_equipo, i, fd_jefe[i], sem_equipos_listos);

        pids_naves[i] = pid;

        close(fd_jefe[i][0]);
    }
}