#include "jefe.h"
#include "mapa.h"
#include "types.h"

void simulador();
void simulador_crear_jefes(tipo_mapa *mapa, int fd_sim[N_EQUIPOS][2], sem_t *sem_equipos_listos);
void simulador_configurar_semaforos(sem_t **sem_monitor, sem_t **sem_equipos_listos);
void simulador_configurar_manejador();
void simulador_configurar_cola(mqd_t *cola);
void simulador_configurar_memoria_compartida(int *shm, tipo_mapa **mapa);
void simulador_configurar_mapa(tipo_mapa *mapa);
void simulador_configurar_naves(tipo_mapa *mapa);
void manejador_SIGINT(int pid);

int main() {
    simulador();
}

void simulador() {
    int fd_sim[N_EQUIPOS][2];
    sem_t *sem_equipos_listos = NULL;
    sem_t *sem_monitor = NULL;
    mqd_t cola = (mqd_t) -1;
    int shm = -1;
    tipo_mapa *mapa = NULL;
    Mensaje mensaje;
    char buffer[BUFFER_SIZE];
    int i, n_naves_restantes;
    n_naves_restantes = N_EQUIPOS*N_NAVES;

    printf("Simulador: configurando recursos compartidos\n");

    srand(time(NULL));
    simulador_configurar_semaforos(&sem_monitor, &sem_equipos_listos);
    simulador_configurar_manejador();
    simulador_configurar_cola(&cola);
    simulador_configurar_memoria_compartida(&shm, &mapa);
    simulador_configurar_mapa(mapa);
    simulador_configurar_naves(mapa);

    printf("Simulador: iniciando la partida\n");

    simulador_crear_jefes(mapa, fd_sim, sem_equipos_listos);

    printf("Simulador: creados todos los jefes\n");
    
    for (i = 0; i < N_EQUIPOS; i++) {
        sem_wait(sem_equipos_listos);
    }

    sem_post(sem_monitor);

    strcpy(buffer, MENSAJE_TURNO_NUEVO);
    while (true) {
        printf("------------------------------------------------\n");
        printf("Simulador: empieza el turno (prueba)\n");
        
        for (i = 0; i < N_EQUIPOS; i++) {
            write(fd_sim[i][1], buffer, sizeof(buffer));
        }

        sleep(TURNO_SECS);

        for (i = 0; i < N_EQUIPOS; i++) {
            sem_wait(sem_equipos_listos);
        }

        printf("Simulador: todos los equipos han hecho sus movimientos\n");

        for (i = 0; i < n_naves_restantes*2; i++) {
            mq_receive(cola, (char *) &mensaje, sizeof(mensaje), NULL);
            if (strcmp(mensaje.info, MENSAJE_MOVER_NAVE)) {
                printf("ACCION MOVER  [%c%d] Objetivo (%d, %d)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y);
            } else if (strcmp(mensaje.info, MENSAJE_ATACAR_NAVE)) {
                printf("ACCION ATACAR [%c%d] Objetivo (%d, %d)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y);
            }
            usleep(100000);
        }
    }

    mq_close(cola);
    mq_unlink(NOMBRE_COLA);

    exit(EXIT_SUCCESS);
}

void simulador_crear_jefes(tipo_mapa *mapa, int fd_sim[N_EQUIPOS][2], sem_t *sem_equipos_listos) {
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

        if (pid == 0) jefe(mapa, i, fd_sim[i], sem_equipos_listos);
        close(fd_sim[i][0]);
    }
}

void simulador_configurar_semaforos(sem_t** sem_monitor, sem_t **sem_equipos_listos) {
    // Configura el semáforo del monitor
    if ((*sem_monitor = sem_open(SEM_MONITOR, O_CREAT, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // Configura el semáforo de equipos listos
    if ((*sem_equipos_listos = sem_open(SEM_EQUIPOS_LISTOS, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }    
    sem_unlink(SEM_EQUIPOS_LISTOS);
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
    if (*cola == (mqd_t) -1) {
        printf("Error abriendo la cola desde el simulador\n");
        kill(0, SIGKILL);   //Quizá deberíamos mandar otra señal y recogerla correctamente
        exit(EXIT_FAILURE);
    }
}

void simulador_configurar_memoria_compartida(int *shm, tipo_mapa **mapa) {
    *shm = shm_open(SHM, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (*shm == -1) {
        printf("Error creando la memoria compartida.\n");
        kill(0, SIGKILL);
        exit(EXIT_FAILURE);
    }
    
    if (ftruncate(*shm, sizeof(tipo_mapa)) == -1) {
        fprintf(stderr, "Error cambiando el tamaño de la memoria compartida.\n");
        unlink(SHM);
        close(*shm);
        kill(0, SIGKILL);
        exit(EXIT_FAILURE);
    }

    *mapa = mmap(NULL, sizeof(tipo_mapa), PROT_READ | PROT_WRITE, MAP_SHARED, *shm, 0);
    if (*mapa == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        shm_unlink(SHM);
        close(*shm);
        kill(0, SIGKILL);
        exit(EXIT_FAILURE);
    }

    close(*shm);
}

void simulador_configurar_mapa(tipo_mapa *mapa) {
    int i, j;

    for (i = 0; i < MAPA_MAX_Y; i++) {
        for (j = 0; j < MAPA_MAX_X; j++) {
            mapa_clean_casilla(mapa, i, j);
        }
    }
}

void simulador_configurar_naves(tipo_mapa *mapa) {
    int i, j;

    for (i = 0; i < N_EQUIPOS; i++) {
        for (j = 0; j < N_NAVES; j++) {
            mapa->info_naves[i][j].equipo = i;
            mapa->info_naves[i][j].num_nave = j;
            mapa->info_naves[i][j].vida = VIDA_MAX;
            mapa->info_naves[i][j].viva = true;
        }
    }

    mapa->info_naves[0][0].pos_x = 0;
    mapa->info_naves[0][0].pos_y = 0;
    mapa->info_naves[0][1].pos_x = 1;
    mapa->info_naves[0][1].pos_y = 0;
    mapa->info_naves[0][2].pos_x = 0;
    mapa->info_naves[0][2].pos_y = 1;

    mapa->info_naves[1][0].pos_x = MAPA_MAX_X-1;
    mapa->info_naves[1][0].pos_y = 0;
    mapa->info_naves[1][1].pos_x = MAPA_MAX_X-2;
    mapa->info_naves[1][1].pos_y = 0;
    mapa->info_naves[1][2].pos_x = MAPA_MAX_X-1;
    mapa->info_naves[1][2].pos_y = 1;

    mapa->info_naves[2][0].pos_x = 0;
    mapa->info_naves[2][0].pos_y = MAPA_MAX_Y-1;
    mapa->info_naves[2][1].pos_x = 1;
    mapa->info_naves[2][1].pos_y = MAPA_MAX_Y-1;
    mapa->info_naves[2][2].pos_x = 0;
    mapa->info_naves[2][2].pos_y = MAPA_MAX_Y-2;

    mapa->info_naves[3][0].pos_x = MAPA_MAX_X-1;
    mapa->info_naves[3][0].pos_y = MAPA_MAX_Y-1;
    mapa->info_naves[3][1].pos_x = MAPA_MAX_X-2;
    mapa->info_naves[3][1].pos_y = MAPA_MAX_Y-1;
    mapa->info_naves[3][2].pos_x = MAPA_MAX_X-1;
    mapa->info_naves[3][2].pos_y = MAPA_MAX_Y-2;

    for (i = 0; i < N_EQUIPOS; i++) {
        mapa_set_num_naves(mapa, i, N_NAVES);
        for (j = 0; j < N_NAVES; j++) {
            mapa_set_nave(mapa, mapa->info_naves[i][j]);
        }
    }
}

void manejador_SIGINT(int pid) {
    printf("Simulador: se termina la ejecucion\n");
    mq_unlink(NOMBRE_COLA);
    sem_unlink(SEM_MONITOR);
    shm_unlink(SHM);
    kill(0, SIGKILL);
}