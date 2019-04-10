#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mqueue.h>

#define TAM 2048    //Tamaño del trozo a escribir (en bytes)
#define MAX_MSG 10

typedef struct {
    char *info;
    int num_bytes;
} Mensaje;

int main(int argc, char *argv) {
    int i;
    char buffer;
    Mensaje *m_recibido = NULL;
    Mensaje *m_cod = NULL;
    mqd_t cola_lectura = NULL;
    mqd_t cola_escritura = NULL;
    
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = TAM
    };

    if (argc != 3) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t./b nombre_cola_lectura nombre_cola_escritura\n");
        return EXIT_FAILURE;
    }


    cola_lectura = mq_open(argv[1], O_CREAT | O_RDONLY);
    if (cola_lectura == (mqd_t)-1) {
        printf("Error abriendo la cola de lectura %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    mq_unlink(cola_lectura);

    cola_escritura = mq_open(argv[1], O_CREAT | O_WRONLY);
    if (cola_escritura == (mqd_t)-1) {
        printf("Error abriendo la cola de escritura %s\n", argv[2]);
        mq_close(cola_lectura);
        return EXIT_FAILURE;
    }
    mq_unlink(cola_escritura);

    m_recibido = (Mensaje *) malloc(sizeof(Mensaje));
    do {        
        if (mq_receive(cola_lectura, m_recibido, sizeof(Mensaje), NULL) == -1) {
            printf("Error leyendo mensaje de la cola\n");
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            return EXIT_FAILURE;
        }

        m_cod = (Mensaje *) malloc(sizeof(Mensaje));
        m_cod->info = (Mensaje *) malloc(m_recibido->num_bytes*sizeof(char));
        m_cod->num_bytes = m_recibido->num_bytes;

        i = 0;
        while (i < m_recibido->num_bytes) {
            if ('a' <= m_recibido->info[i] && m_recibido->info[i] < 'z') {
                m_cod->info[i] = m_recibido->info[i]+1;
            } else if (m_recibido->info[i] == 'z') {
                m_cod->info[i] = 'a';
            }
        }

        if (mq_send(cola_escritura, (char *) &m_cod, sizeof(m_cod), 1) == -1) {
            printf("Error enviando mensaje a la cola\n");
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            return EXIT_FAILURE;
        }
    } while (1);

    mq_close(cola_lectura);
    mq_close(cola_escritura);

    return EXIT_SUCCESS;
}