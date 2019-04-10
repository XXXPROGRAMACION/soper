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
    Mensaje m_recibido;
    mqd_t cola_lectura = (mqd_t)-1;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    if (argc != 2) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t%s nombre_cola_lectura\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    cola_lectura = mq_open(argv[1], O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &atributos);
    if (cola_lectura == (mqd_t)-1) {
        printf("C: Error abriendo la cola de lectura %s\n", argv[1]);
        printf("Errno: %d\n", errno);
        exit(EXIT_FAILURE);
    }
 
    do {
        if (mq_receive(cola_lectura, (char *) &m_recibido, sizeof(Mensaje), NULL) == -1) {
            printf("C: Error leyendo mensaje de la cola\n");
            printf("Errno: %d\n", errno);
            mq_close(cola_lectura);
            exit(EXIT_FAILURE);
        }
        if (m_recibido.num_bytes == 1) break;

        m_recibido.info[m_recibido.num_bytes] = '\0';

        printf("%s", m_recibido.info);
    } while (m_recibido.num_bytes == TAM);

    printf("\n");

    mq_close(cola_lectura);

    return EXIT_SUCCESS;
}