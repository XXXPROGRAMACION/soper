#include "nave.h"

void nave_configurar_cola(int n_equipo, int n_nave, mqd_t *cola);

void nave(tipo_mapa *mapa, int n_equipo, int n_nave, int fd_jefe[2], sem_t *sem_equipos_listos) {
    mqd_t cola = (mqd_t) -1;
    tipo_nave nave_aux, nave_actual;
    char buffer[BUFFER_SIZE];
    int direcciones[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
    int direccionInicial;
    int objetivoInicial;
    Mensaje mensaje;
    int i;

    close(fd_jefe[1]);

    nave_configurar_cola(n_equipo, n_nave, &cola);

    mensaje.n_equipo = n_equipo;
    mensaje.n_nave = n_nave;

    srand(time(NULL)+n_equipo+n_nave);

    while (true) {
        read(fd_jefe[0], buffer, BUFFER_SIZE);

        if (!strcmp(buffer, MOVER_ALEATORIO)) {
            strcpy(mensaje.info, MOVER_ALEATORIO);

            direccionInicial = rand()%8;
            for (i = 0; i < 8; i++) {
                mensaje.x = mapa_get_nave(mapa, n_equipo, n_nave).pos_x+direcciones[(direccionInicial+i)%8][0];
                mensaje.y = mapa_get_nave(mapa, n_equipo, n_nave).pos_y+direcciones[(direccionInicial+i)%8][1];

                if (mensaje.x < 0) continue;
                if (mensaje.x > MAPA_MAX_X-1) continue;
                if (mensaje.y < 0) continue;
                if (mensaje.y > MAPA_MAX_Y-1) continue; 

                if (mapa_is_casilla_vacia(mapa, mensaje.y, mensaje.x)) break;
            }

            if (i == 8) {
                mensaje.x = -1;
                mensaje.y = -1;
            }

            mq_send(cola, (char *) &mensaje, sizeof(mensaje), 1);
        } else if (!strcmp(buffer, ATACAR)) {
            strcpy(mensaje.info, ATACAR);

            objetivoInicial = rand()%(N_EQUIPOS*N_NAVES);

            nave_actual = mapa_get_nave(mapa, n_equipo, n_nave);
            for (i = 0; i < N_EQUIPOS*N_NAVES; i++) {
                nave_aux = mapa_get_nave(mapa, ((objetivoInicial+i)%(N_EQUIPOS*N_NAVES))/N_NAVES, (objetivoInicial+i)%N_NAVES);

                if (nave_aux.equipo == n_equipo) continue;
                if (nave_aux.viva == false) continue;
                if (mapa_get_distancia(mapa, nave_actual.pos_y, nave_actual.pos_x, nave_aux.pos_y, nave_aux.pos_x) > ATAQUE_ALCANCE) continue;

                mensaje.x = nave_aux.pos_x;
                mensaje.y = nave_aux.pos_y;

                break;
            }

            if (i == N_EQUIPOS*N_NAVES) {
                mensaje.x = -1;
                mensaje.y = -1;
            }

            mq_send(cola, (char *) &mensaje, sizeof(mensaje), 1);
        } else {
            //Destruir nave
            close(fd_jefe[0]);
            mq_close(cola);
            munmap(mapa, sizeof(mapa));
            exit(EXIT_SUCCESS);
        }
    }
    
    exit(EXIT_SUCCESS);
}

void nave_configurar_cola(int n_equipo, int n_nave, mqd_t *cola) {
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    *cola = mq_open(NOMBRE_COLA, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &atributos);
    if (*cola == (mqd_t) -1) {
        printf("Error abriendo la cola desde la nave %d del equipo %d\n", n_nave+1, n_equipo+1);
        kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
        exit(EXIT_FAILURE);
    }
}