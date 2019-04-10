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

int main(int argc, char **argv) {
    int i;
    Mensaje *m_recibido = NULL;
    Mensaje *m_cod = NULL;
    mqd_t cola_lectura = (mqd_t)-1;
    mqd_t cola_escritura = (mqd_t)-1;

    if (argc != 3) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t%s nombre_cola_lectura nombre_cola_escritura\n", argv[0]);
        return EXIT_FAILURE;
    }


    cola_lectura = mq_open(argv[1], O_RDONLY);
    if (cola_lectura == (mqd_t)-1) {
        printf("B: Error abriendo la cola de lectura %s\n", argv[2]);
        printf("Errno: %d\n", errno);
        return EXIT_FAILURE;
    }

    cola_escritura = mq_open(argv[2], O_WRONLY);
    if (cola_escritura == (mqd_t)-1) {
        printf("B: Error abriendo la cola de escritura %s\n", argv[2]);
        printf("Errno: %d\n", errno);
        mq_close(cola_lectura);
        return EXIT_FAILURE;
    }

    m_recibido = (Mensaje *) malloc(sizeof(Mensaje));
    do {        
        if (mq_receive(cola_lectura, (char *) m_recibido, sizeof(Mensaje), NULL) == -1) {
            printf("B: Error leyendo mensaje de la cola\n");
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            return EXIT_FAILURE;
        }

        printf("¡Mensaje recibido!: %s\n", m_recibido->info);

        m_cod = (Mensaje *) malloc(sizeof(Mensaje));
        m_cod->info = (char *) malloc(m_recibido->num_bytes*sizeof(char));
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
            printf("B: Error enviando mensaje a la cola\n");
            mq_close(cola_lectura);
            mq_close(cola_escritura);
            return EXIT_FAILURE;
        }
    } while (1);

    mq_close(cola_lectura);
    mq_close(cola_escritura);

    printf("B: Fin\n");
    return EXIT_SUCCESS;
}