#include "jefe.h"

void jefe_crear_naves(int n_equipo, int fd_jefe[N_NAVES][2]);

void jefe(int n_equipo, int fd_sim[2], sem_t *equipos_listos) {
	int fd_jefe[N_NAVES][2];
    char buffer[BUFFER_SIZE];

    close(fd_sim[1]);

    jefe_crear_naves(n_equipo, fd_jefe);

    printf("Jefe %d: equipo %d listo\n", n_equipo+1, n_equipo+1);
    sem_post(equipos_listos);
    printf("Despues de post\n");

    while (true) {
        read(fd_sim[0], buffer, sizeof(buffer));
        if (!strcmp(buffer, MENSAJE_TURNO_NUEVO)) {
            //Turno nuevo
            printf("Jefe %d: recibido inicio de turno\n", n_equipo+1); // PRUEBA
            sem_post(equipos_listos);
        }
    }

    exit(EXIT_SUCCESS);
}

void jefe_crear_naves(int n_equipo, int fd_jefe[N_NAVES][2]) {
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
        if (pid == 0) nave(n_equipo, i, fd_jefe[i]);
        close(fd_jefe[i][0]);
    }
}