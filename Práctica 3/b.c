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
    char info[TAM+1];
    int num_bytes;
} Mensaje;

int main(int argc, char **argv) {
    int i;
    Mensaje m_recibido;
    Mensaje m_cod;
    mqd_t cola_lectura = (mqd_t)-1;
    mqd_t cola_escritura = (mqd_t)-1;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    if (argc != 3) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t%s nombre_cola_lectura nombre_cola_escritura\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    cola_lectura = mq_open(argv[1], O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &atributos);
    if (cola_lectura == (mqd_t)-1) {
        printf("B: Error abriendo la cola de lectura %s\n", argv[2]);
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    cola_escritura = mq_open(argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &atributos);
    if (cola_escritura == (mqd_t)-1) {
        printf("B: Error abriendo la cola de escritura %s\n", argv[2]);
        printf("Errno: %d\n", errno);
        mq_close(cola_lectura);
        exit(EXIT_FAILURE);
    }

    do {        
        if (mq_receive(cola_lectura, (char *) &m_recibido, sizeof(Mensaje), NULL) == -1) {
            printf("B: Error leyendo mensaje de la cola\n");
            printf("Errno: %d\n", errno);
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            exit(EXIT_FAILURE);
        }

        m_cod.num_bytes = m_recibido.num_bytes;

        i = 0;
        while (i < m_recibido.num_bytes) {
            if ('a' <= m_recibido.info[i] && m_recibido.info[i] < 'z') {
                m_cod.info[i] = m_recibido.info[i]+1;
            } else if (m_recibido.info[i] == 'z') {
                m_cod.info[i] = 'a';
            } else {
                m_cod.info[i] = m_recibido.info[i];
            }
            i++;
        }

        if (mq_send(cola_escritura, (char *) &m_cod, sizeof(Mensaje), 1) == -1) {
            printf("B: Error enviando mensaje a la cola\n");
            printf("Errno: %d\n", errno);
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            exit(EXIT_FAILURE);
        }
    } while (m_recibido.num_bytes == TAM);

    mq_close(cola_lectura);
    mq_close(cola_escritura);

    exit(EXIT_SUCCESS);
}