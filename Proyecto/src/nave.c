#include "nave.h"

void nave_configurar_cola(int n_equipo, int n_nave, mqd_t *cola);

void nave(tipo_mapa *mapa, int n_equipo, int n_nave, int fd_jefe[2]) {
    mqd_t cola = (mqd_t) -1;
    char buffer[BUFFER_SIZE];
    int direcciones[8][2] = {{0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}};
    int direccionInicial;
    Mensaje mensaje;
    int i;

    close(fd_jefe[1]);

    nave_configurar_cola(n_equipo, n_nave, &cola);

    mensaje.n_equipo = n_equipo;
    mensaje.n_nave = n_nave;

    while (true) {
        read(fd_jefe[0], buffer, BUFFER_SIZE);
        if (!strcmp(buffer, MENSAJE_MOVER_NAVE)) {
            strcpy(mensaje.info, MENSAJE_MOVER_NAVE);

            direccionInicial = rand()%8;
            for (i = 0; i < 8; i++) {
                mensaje.x = mapa->info_naves[n_equipo][n_nave].pos_x+direcciones[(direccionInicial+i)%8][0];
                mensaje.y = mapa->info_naves[n_equipo][n_nave].pos_y+direcciones[(direccionInicial+i)%8][1];

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
        } else if (!strcmp(buffer, MENSAJE_ATACAR_NAVE)) {
            strcpy(mensaje.info, MENSAJE_ATACAR_NAVE);
            mensaje.x = -1;
            mensaje.y = -1;
            mq_send(cola, (char *) &mensaje, sizeof(mensaje), 1);
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