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
    Mensaje *m_recibido = NULL;
    mqd_t cola_lectura = NULL;
    
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = TAM
    };

    if (argc != 2) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t./c nombre_cola_lectura\n");
        return EXIT_FAILURE;
    }


    cola_lectura = mq_open(argv[1], O_CREAT | O_RDONLY);
    if (cola_lectura == (mqd_t)-1) {
        printf("Error abriendo la cola de lectura %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    mq_unlink(cola_lectura);

    m_recibido = (Mensaje *) malloc(sizeof(Mensaje));
 
    if (mq_receive(cola_lectura, m_recibido, sizeof(Mensaje), NULL) == -1) {
        printf("Error leyendo mensaje de la cola\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", m_recibido->info);

    mq_close(cola_lectura);

    return EXIT_SUCCESS;
}