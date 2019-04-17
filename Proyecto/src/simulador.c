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

#include "mapa.h"

int main() {
	int ret = 0;
	int fd_sim[N_EQUIPOS][2];
	int fd_jefe[N_NAVES][2];
    int buffer[BUFFER_SIZE];
    int pipe_ret, pid, i, n_equipo, n_nave;
    Mensaje mensaje;
    mqd_t cola = (mqd_t)-1;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    for (i = 0; i < N_EQUIPOS; i++) {
        // Crea las tuberías
        pipe_ret = pipe(fd_sim[i]);
        if (pipe_ret == -1) {
            printf("Error creando los pipes entre el simulador y los jefes\n");
            kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
            exit(EXIT_FAILURE);
        }

        pid = fork();
        if (pid == -1) {
            printf("Error haciendo los fork para los jefes\n");
            kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
            exit(EXIT_FAILURE);
        }

        if (pid == 0) break;
        close(fd_sim[i][0]);
    }

    if (pid != 0) {
        /********************************************
         * Simulador
         ********************************************/
        // Abre la cola de mensajes
        cola = mq_open(NOMBRE_COLA, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &atributos);
        if (cola == (mqd_t)-1) {
            printf("Error abriendo la cola desde el simulador\n");
            kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
            exit(EXIT_FAILURE);
        }
    } else {
        /********************************************
         * Jefes
         ********************************************/
        n_equipo = i;
        close(fd_sim[n_equipo][1]);

        for (i = 0; i < N_NAVES; i++) {
            // Crea las tuberías
            pipe_ret = pipe(fd_jefe[i]);
            if (pipe_ret == -1) {
                printf("Error creando los pipes entre el jefe y las naves\n");
                kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
                exit(EXIT_FAILURE);
            }

            pid = fork();
            if (pid == 0) break;
            close(fd_jefe[i][0]);
        }

        if (pid != 0) {
            while(true) {
                read(fd_sim[n_equipo][0], buffer, sizeof(buffer));
                if (!strcmp(buffer, MENSAJE_TURNO_NUEVO)) {
                    //Turno nuevo
                }
            }
        } else {
            /********************************************
             * Naves
             ********************************************/
            n_nave = i;
            close(fd_sim[n_equipo][0]);
            close(fd_jefe[n_nave][1]);

            cola = mq_open(NOMBRE_COLA, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &atributos);
            if (cola == (mqd_t)-1) {
                printf("Error abriendo la cola desde la nave %d del equipo %d\n", n_nave, n_equipo);
                kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
                exit(EXIT_FAILURE);
            }
        }
    }

    exit(ret);
}