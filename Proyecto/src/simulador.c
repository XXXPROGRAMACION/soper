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
#include <semaphore.h>

#include "mapa.h"

#define E_LISTOS "e_listos"

void manejador_SIGINT(int pid) {

}

int main() {
	int fd_sim[N_EQUIPOS][2];
	int fd_jefe[N_NAVES][2];
    char buffer[BUFFER_SIZE];
    sem_t *e_listos;
    struct sigaction act;
    int pipe_ret, pid, n_equipo, n_nave, aux, i, j;
    // Mensaje mensaje;
    mqd_t cola = -1;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };
    
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;

    act.sa_handler = manejador_SIGINT;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    if ((e_listos = sem_open(E_LISTOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_unlink(E_LISTOS);

    printf("Simulador: iniciando la partida\n");

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
        printf("Simulador: creados todos los jefes\n");

        // Abre la cola de mensajes
        cola = mq_open(NOMBRE_COLA, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &atributos);
        if (cola == -1) {
            printf("Error abriendo la cola desde el simulador\n");
            kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
            exit(EXIT_FAILURE);
        }

        sem_getvalue(e_listos, &aux);
        printf("Simulador: valor sem %d\n", aux);
        while (aux != N_EQUIPOS) {
            pause();
            sem_getvalue(e_listos, &aux);
        }

        // PRUEBA
        strcpy(buffer, MENSAJE_TURNO_NUEVO);
        for (i = 0; i < 3; i++) {
            printf("Sistema: empieza el turno %d (prueba)\n", i+1);
            
            for (j = 0; j < N_EQUIPOS; j++) {
                write(fd_sim[j][1], buffer, sizeof(buffer));
            }

            sleep(3);
        }

        printf("Simulador: todos los equipos listos\n");
        kill(0, SIGKILL); //Medida temporal
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
            printf("Jefe %d: equipo %d listo\n", n_equipo+1, n_equipo+1);
            sem_post(e_listos);
            kill(getppid(), SIGINT);

            while(true) {
                read(fd_sim[n_equipo][0], buffer, sizeof(buffer));
                if (!strcmp(buffer, MENSAJE_TURNO_NUEVO)) {
                    //Turno nuevo
                    printf("Jefe %d: recibido inicio de turno\n", n_equipo+1); // PRUEBA
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
            if (cola == -1) {
                printf("Error abriendo la cola desde la nave %d del equipo %d\n", n_nave, n_equipo);
                kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
                exit(EXIT_FAILURE);
            }
        }
    }
}