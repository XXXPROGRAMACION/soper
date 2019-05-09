#include "jefe.h"

/** Crea las naves del jefe y las conexiones entre las naves y el jefe.
 *  @param mapa Puntero al mapa sobre el que trabajan el jefe y las naves
 *  @param n_equipo Número de equipo del jefe y las naves
 *  @param fd_jefe Pipes que conectarán al jefe con sus naves
 *  @param pids_naves En esta estructura se almacenarán los PIDs de las naves que se creen
 */
void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2], pid_t pids_naves[N_NAVES]);

/** Emula el comportamiento de un jefe. Se ocupa de crear los procesos nave y de mandarles
 *  órdenes de acción en cada turno. Gestiona la comunicación entre el simulador y las naves.
 *  @param mapa Puntero al mapa sobre el que debe trabajar el jefe
 *  @param n_equipo Número de equipo del jefe
 *  @param fd_sim Pipe que le permite comunicarse con el simulador
 *  @param sem_equipos_listos Semáforo que permite al simulador saber cúando estan listos los jefes
 *  @see nave()
 *  @see simulador() 
 */
void jefe(tipo_mapa *mapa, int n_equipo, int fd_sim[2], sem_t *sem_equipos_listos) {
    pid_t pids_naves[N_NAVES];
	int fd_jefe[N_NAVES][2];
    char buffer[BUFFER_SIZE];
    int i, j, num_nave_destruir;

    close(fd_sim[1]);

    // Creamos las naves
    jefe_crear_naves(mapa, n_equipo, fd_jefe, pids_naves);
    
    sem_post(sem_equipos_listos);

    srand(time(NULL)+n_equipo);

    while (true) {
        read(fd_sim[0], buffer, sizeof(buffer));

        if (!strcmp(buffer, TURNO)) {
            // Turno nuevo
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
            // Fin de la ejecución
            close(fd_sim[0]);
            sem_close(sem_equipos_listos);
            munmap(mapa, sizeof(mapa));

            // Terminamos a todas las naves
            for (i = 0; i < N_NAVES; i++) {
                close(fd_jefe[i][1]);
                kill(pids_naves[i], SIGTERM);
            }

            while (wait(NULL) > 0);

            exit(EXIT_SUCCESS);
        } else {
            // Destruir nave
            sscanf(buffer, "%*s %d", &num_nave_destruir);
            strcpy(buffer, DESTRUIR);
            write(fd_jefe[num_nave_destruir][1], buffer, sizeof(buffer));
        }
    }

    exit(EXIT_SUCCESS);
}

void jefe_crear_naves(tipo_mapa *mapa, int n_equipo, int fd_jefe[N_NAVES][2], pid_t pids_naves[N_NAVES]) {
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

        pids_naves[i] = pid;

        close(fd_jefe[i][0]);
    }
}