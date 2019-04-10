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
    FILE *f = NULL;
    mqd_t cola_1 = (mqd_t)-1;
    mqd_t cola_2 = (mqd_t)-1;
    pid_t pid;
    int i;
    struct mq_attr atributos = {
        .mq_flags = 0,
        .mq_maxmsg = MAX_MSG,
        .mq_curmsgs = 0,
        .mq_msgsize = sizeof(Mensaje)
    };

    if (argc != 4) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\t%s fichero nombre_cola_1 nombre_cola_2\n", argv[0]);
        return EXIT_FAILURE;
    }

    f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("Error al abrir el archivo \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }

    cola_1 = mq_open(argv[2], O_CREAT, S_IRUSR | S_IWUSR, &atributos);
    if (cola_1 == (mqd_t)-1) {
        printf("Error creando la primera cola \"%s\"\n", argv[2]);
        return EXIT_FAILURE;
    }

    cola_2 = mq_open(argv[3], O_CREAT, S_IRUSR | S_IWUSR, &atributos);
    if (cola_2 == (mqd_t)-1) {
        printf("Error creando la segunda cola \"%s\"\n", argv[3]);
        mq_unlink(argv[2]);
        mq_close(cola_1);
        return EXIT_FAILURE;
    }

    for (i = 0; i < 4; i++) {
        pid = fork();
        if (pid == 0) break;
    }

    if (i == 0) {
        sleep(4);
        execlp("./a", "./a", argv[1], argv[2], NULL);
        printf("Error en el exec en a\n");
        return EXIT_FAILURE;
    } else if (i == 1) {
        execlp("./b", "./b", argv[2], argv[3], NULL);
        printf("Error en el exec en b\n");
        return EXIT_FAILURE;
    } else if (i == 2) {
        while(1);
        execlp("./c", "./c", argv[3], NULL);
        printf("Error en el exec en c\n");
        return EXIT_FAILURE;
    } else if (i == 3) {
        /*while (wait(NULL) >= 0);
        printf("Cerrando colas...\n");
        mq_unlink(argv[2]);
        mq_unlink(argv[3]);
        mq_close(cola_1);
        mq_close(cola_2);*/
    }

    return EXIT_SUCCESS;
}