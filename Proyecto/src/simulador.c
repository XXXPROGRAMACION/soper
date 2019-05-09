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
void manejador_SIGALRM(int pid);
void manejador_SIGTERM(int pid);

sem_t *sem_alarma = NULL;

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
    tipo_nave nave_aux;
    tipo_casilla casilla_aux;
    char buffer[BUFFER_SIZE];
    int n_naves_restantes, n_naves_destruidas;
    int naves_equipo[N_EQUIPOS];
    int ganador;
    int i;

    printf("Simulador: configurando recursos compartidos\n");

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
        naves_equipo[i] = N_NAVES;
    }

    sem_post(sem_monitor);

    srand(time(NULL));
    n_naves_restantes = N_EQUIPOS*N_NAVES;
    
    while (true) {
        mapa_restore(mapa);

        printf("------------------------------------------------\n");

        ganador = -1;
        for (i = 0; i < N_EQUIPOS; i++) {
            if (naves_equipo[i] > 0) {
                if (ganador == -1) {
                    ganador = i;
                } else {
                    break;
                }
            }
        }

        if (i == N_EQUIPOS) {
            if (ganador != -1) {
                printf("GANA EL EQUIPO %c!\n", symbol_equipos[ganador]);
            } else {
                printf("TODOS PIERDEN!\n");
            }

            strcpy(buffer, FIN);
            for (i = 0; i < N_EQUIPOS; i++) {
                write(fd_sim[i][1], buffer, sizeof(buffer));
            }

            while (wait(NULL) > 0);

            sem_close(sem_equipos_listos);
            sem_close(sem_monitor);
            mq_close(cola);
            close(shm);
            munmap(mapa, sizeof(mapa));
            for (i = 0; i < N_EQUIPOS; i++) {
                close(fd_sim[i][1]);                
            }

            mq_unlink(NOMBRE_COLA);
            sem_unlink(SEM_MONITOR);
            shm_unlink(SHM);

            exit(EXIT_SUCCESS);
        }

        printf("Simulador: empieza el turno (prueba)\n");        

        strcpy(buffer, TURNO);
        for (i = 0; i < N_EQUIPOS; i++) {
            write(fd_sim[i][1], buffer, sizeof(buffer));
        }

        alarm(TURNO_SECS);

        for (i = 0; i < N_EQUIPOS; i++) {
            sem_wait(sem_equipos_listos);
        }

        printf("Simulador: todos los equipos han hecho sus movimientos\n");

        n_naves_destruidas = 0;
        for (i = 0; i < n_naves_restantes*2; i++) {
            mq_receive(cola, (char *) &mensaje, sizeof(mensaje), NULL);
        
            nave_aux = mapa_get_nave(mapa, mensaje.n_equipo, mensaje.n_nave);

            if (!nave_aux.viva) continue;

            if (!strcmp(mensaje.info, MOVER_ALEATORIO)) {
                if (mensaje.x >= 0 && mensaje.y >= 0) {
                    if (mapa_is_casilla_vacia(mapa, mensaje.y, mensaje.x)) {
                        mapa_clean_casilla(mapa, nave_aux.pos_y, nave_aux.pos_x);
                        nave_aux.pos_x = mensaje.x;
                        nave_aux.pos_y = mensaje.y;
                        mapa_set_nave(mapa, nave_aux);

                        printf("ACCION MOVER  [%c%d] Objetivo (%02d, %02d)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y);
                    } else {
                        printf("ACCION MOVER  [%c%d] Objetivo (%02d, %02d) - Cancelado, casilla ocupada\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y);
                    }
                } else {
                    printf("ACCION MOVER  [%c%d] Sin objetivo\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave);
                }  
            } else if (!strcmp(mensaje.info, ATACAR)) {
                if (mensaje.x >= 0 && mensaje.y >= 0) {
                    mapa_send_misil(mapa, nave_aux.pos_y, nave_aux.pos_x, mensaje.y, mensaje.x);

                    if (!mapa_is_casilla_vacia(mapa, mensaje.y, mensaje.x)) {
                        casilla_aux = mapa_get_casilla(mapa, mensaje.y, mensaje.x);
                        nave_aux = mapa_get_nave(mapa, casilla_aux.equipo, casilla_aux.num_nave);
                        nave_aux.vida -= ATAQUE_DANO;
                        if (nave_aux.vida <= 0) {
                            nave_aux.viva = false;
                            n_naves_destruidas++;
                            naves_equipo[nave_aux.equipo]--;
                            sprintf(buffer, "%s %d", DESTRUIR, nave_aux.num_nave);
                            write(fd_sim[nave_aux.equipo][1], buffer, sizeof(buffer));
                        }
                        mapa_set_nave(mapa, nave_aux);

                        if (nave_aux.viva) {
                            printf("ACCION ATACAR [%c%d] Objetivo (%02d, %02d) - %c%d impactada (vida = %d)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y, symbol_equipos[nave_aux.equipo], nave_aux.num_nave, nave_aux.vida);
                        } else {
                            printf("ACCION ATACAR [%c%d] Objetivo (%02d, %02d) - %c%d impactada (destruida)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y, symbol_equipos[nave_aux.equipo], nave_aux.num_nave);
                        }
                    } else {
                        printf("ACCION ATACAR [%c%d] Objetivo (%02d, %02d)\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave, mensaje.x, mensaje.y);
                    }
                } else {
                    printf("ACCION ATACAR [%c%d] Sin objetivo\n", symbol_equipos[mensaje.n_equipo], mensaje.n_nave);
                }                
            }

            usleep(100000);
        }

        n_naves_restantes -= n_naves_destruidas;

        sem_wait(sem_alarma);
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

    // Configura el semáforo de la alarma
    if ((sem_alarma = sem_open(SEM_ALARMA, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 0)) == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    sem_unlink(SEM_ALARMA);
}

void simulador_configurar_manejador() {
    struct sigaction act;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    
    // Crea el manejador de SIGINT
    act.sa_handler = manejador_SIGINT;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Crea el manejador de SIGINT
    act.sa_handler = manejador_SIGALRM;
    if (sigaction(SIGALRM, &act, NULL) < 0) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Crea el manejador de SIGTERM
    act.sa_handler = manejador_SIGTERM;
    if (sigaction(SIGTERM, &act, NULL) < 0) {
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
    tipo_nave nave;

    nave.vida = VIDA_MAX;
    nave.viva = true;

    // EQUIPO 1
    nave.equipo = 0;
    mapa_set_num_naves(mapa, 0, N_NAVES);
    nave.num_nave = 0;
    nave.pos_x = 1;
    nave.pos_y = 1;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 1;
    nave.pos_x = 3;
    nave.pos_y = 1;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 2;
    nave.pos_x = 1;
    nave.pos_y = 3;
    mapa_set_nave(mapa, nave);
    
    // EQUIPO 2
    nave.equipo = 1;
    mapa_set_num_naves(mapa, 1, N_NAVES);
    nave.num_nave = 0;
    nave.pos_x = MAPA_MAX_X-2;
    nave.pos_y = 1;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 1;
    nave.pos_x = MAPA_MAX_X-4;
    nave.pos_y = 1;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 2;
    nave.pos_x = MAPA_MAX_X-2;
    nave.pos_y = 3;
    mapa_set_nave(mapa, nave);
    
    // EQUIPO 3
    nave.equipo = 2;
    mapa_set_num_naves(mapa, 2, N_NAVES);
    nave.num_nave = 0;
    nave.pos_x = 1;
    nave.pos_y = MAPA_MAX_Y-2;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 1;
    nave.pos_x = 3;
    nave.pos_y = MAPA_MAX_Y-2;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 2;
    nave.pos_x = 1;
    nave.pos_y = MAPA_MAX_Y-4;
    mapa_set_nave(mapa, nave);
    
    // EQUIPO 4
    nave.equipo = 3;
    mapa_set_num_naves(mapa, 3, N_NAVES);
    nave.num_nave = 0;
    nave.pos_x = MAPA_MAX_X-2;
    nave.pos_y = MAPA_MAX_Y-2;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 1;
    nave.pos_x = MAPA_MAX_X-4;
    nave.pos_y = MAPA_MAX_Y-2;
    mapa_set_nave(mapa, nave);
    nave.num_nave = 2;
    nave.pos_x = MAPA_MAX_X-2;
    nave.pos_y = MAPA_MAX_Y-4;
    mapa_set_nave(mapa, nave);
}

void manejador_SIGINT(int pid) {
    printf("Simulador: se termina la ejecucion\n");
    mq_unlink(NOMBRE_COLA);
    sem_unlink(SEM_MONITOR);
    shm_unlink(SHM);
    kill(0, SIGKILL);
}

void manejador_SIGALRM(int pid) {
    sem_post(sem_alarma);
}

void manejador_SIGTERM(int pid) {
    exit(EXIT_SUCCESS);
}