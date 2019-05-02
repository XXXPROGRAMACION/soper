#include "jefe.h"
#include "mapa.h"
#include "types.h"

void simulador();
void simulador_crear_jefes(int fd_sim[N_EQUIPOS][2], sem_t *equipos_listos);
void simulador_configurar_semaforo(sem_t **equipos_listos);
void simulador_configurar_manejador();
void simulador_configurar_cola(mqd_t *cola);
void manejador_SIGINT(int pid);

int main() {
    simulador();
}

void simulador() {
    int fd_sim[N_EQUIPOS][2];
    sem_t *equipos_listos = NULL;
    mqd_t cola = -1;
    Mensaje mensaje;
    char buffer[BUFFER_SIZE];
    int i, n_naves_restantes;
    n_naves_restantes = N_EQUIPOS*N_NAVES;

    simulador_configurar_semaforo(&equipos_listos);

    printf("Simulador: iniciando la partida\n");

    simulador_crear_jefes(fd_sim, equipos_listos);

    printf("Simulador: creados todos los jefes\n");

    simulador_configurar_manejador();
    simulador_configurar_cola(&cola);
    
    for (i = 0; i < N_EQUIPOS; i++) {
        sem_wait(equipos_listos);
    }

    strcpy(buffer, MENSAJE_TURNO_NUEVO);
    while (true) {
        printf("Simulador: empieza el turno (prueba)\n");
        
        for (i = 0; i < N_EQUIPOS; i++) {
            write(fd_sim[i][1], buffer, sizeof(buffer));
        }

        sleep(TURNO_SECS);

        for (i = 0; i < N_EQUIPOS; i++) {
            sem_wait(equipos_listos);
        }

        printf("Simulador: todos los equipos listos\n");

        for (i = 0; i < n_naves_restantes; i++) {
            mq_receive(cola, (char *) &mensaje, sizeof(mensaje), 0);
            printf("Simulador: mensaje recibido: Nave %d del equipo %d \"%s\"\n", mensaje.n_nave, mensaje.n_equipo, mensaje.info);
        }
    }

    exit(EXIT_SUCCESS);
}

void simulador_crear_jefes(int fd_sim[N_EQUIPOS][2], sem_t *equipos_listos) {
    int pipe_ret, pid, i;

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

        if (pid == 0) jefe(i, fd_sim[i], equipos_listos);
        close(fd_sim[i][0]);
    }
}

void simulador_configurar_semaforo(sem_t **equipos_listos) {
    // Configura el semáforo de equipos listos
    if ((*equipos_listos = sem_open(EQUIPOS_LISTOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    sem_unlink(EQUIPOS_LISTOS);
}

void simulador_configurar_manejador() {
    struct sigaction act;
    
    // Crea el manejador de SIGINT
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    act.sa_handler = manejador_SIGINT;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

void simulador_configurar_cola(mqd_t *cola) {
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };
    
    // Abre la cola de mensajes
    *cola = mq_open(NOMBRE_COLA, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &atributos);
    if (*cola == -1) {
        printf("Error abriendo la cola desde el simulador\n");
        kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
        exit(EXIT_FAILURE);
    }
}

void manejador_SIGINT(int pid) {
    printf("Simulador: se termina la ejecucion\n");
    kill(0, SIGKILL);
}