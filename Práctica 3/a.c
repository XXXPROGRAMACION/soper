#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
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
    int fd, leido;
    struct stat s;
    char *f = NULL;
    Mensaje m;
    mqd_t cola = (mqd_t)-1;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    if (argc != 3) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t%s fichero nombre_cola_escritura\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        printf("Error abriendo el archivo %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    fstat(fd, &s);

    f = (char *) mmap(NULL, s.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (f == NULL) {
        printf("Error mapeando el archivo %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    cola = mq_open(argv[2], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &atributos);
    if (cola == (mqd_t)-1) {
        printf("A: Error abriendo la cola de escritura %s\n", argv[2]);
        printf("Errno: %d\n", errno);
        munmap(f, s.st_size*sizeof(char));
        exit(EXIT_FAILURE);
    }

    leido = 0;

    do {
        if (leido+TAM < s.st_size) {
            memcpy(&m.info, &f[leido], TAM);
            leido += TAM;
            m.num_bytes = TAM;
        } else {
            memcpy(&m.info, &f[leido], s.st_size-leido);
            m.num_bytes = s.st_size-leido;
        }

        if (mq_send(cola, (char *) &m, sizeof(Mensaje), 1) == -1) {
            printf("A: Error enviando mensaje a la cola\n");
            printf("Errno: %d\n", errno);
            munmap(f, s.st_size*sizeof(char));
            mq_close(cola);
            exit(EXIT_FAILURE);
        }
    } while (m.num_bytes == TAM);

    munmap(f, s.st_size*sizeof(char));
    mq_close(cola);

    return EXIT_SUCCESS;
}