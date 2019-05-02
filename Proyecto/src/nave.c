#include "nave.h"

void nave_configurar_cola(int n_equipo, int n_nave, mqd_t *cola);

void nave(int n_equipo, int n_nave, int fd_jefe[2]) {
    mqd_t cola = -1;
    char buffer[BUFFER_SIZE];
    Mensaje mensaje;
    // Mensaje mensaje;

    close(fd_jefe[1]);

    nave_configurar_cola(n_equipo, n_nave, &cola);


    mensaje.n_equipo = n_equipo;
    mensaje.n_nave = n_nave;

    while (true) {
        read(fd_jefe[0], buffer, BUFFER_SIZE);
        if (!strcmp(buffer, MENSAJE_MOVER_NAVE)) {
            printf("Nave %d del equipo %d: me ordenan moverme\n", n_nave+1, n_equipo+1);
            strcpy(mensaje.info, MENSAJE_MOVER_NAVE);
            mq_send(cola, (char *) &mensaje, sizeof(mensaje), 0);
        } else if (!strcmp(buffer, MENSAJE_ATACAR_NAVE)) {
            printf("Nave %d del equipo %d: me ordenan atacar\n", n_nave+1, n_equipo+1);
            strcpy(mensaje.info, MENSAJE_ATACAR_NAVE);
            mq_send(cola, (char *) &mensaje, sizeof(mensaje), 0);
        } else {
            printf("Nave %d del equipo %d: no entiendo a mi jefe...\n", n_nave+1, n_equipo+1);
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
    if (*cola == -1) {
        printf("Error abriendo la cola desde la nave %d del equipo %d\n", n_nave+1, n_equipo+1);
        kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
        exit(EXIT_FAILURE);
    }
}