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
    char buffer;
    FILE *f = NULL;
    Mensaje *m = NULL;
    mqd_t cola = (mqd_t)-1;

    if (argc != 3) {
        printf("Número de argumentos inválido. Uso: \n");
        printf("\ta fichero nombre_cola_escritura\n");
        return EXIT_FAILURE;
    }

    f = fopen(argv[1], "rb");
    if (f == NULL) {
        printf("Error abriendo el archivo %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    cola = mq_open(argv[2], O_WRONLY);
    if (cola == (mqd_t)-1) {
        printf("Error abriendo la cola de escritura %s\n", argv[2]);
        fclose(f);
        return EXIT_FAILURE;
    }
    mq_unlink(argv[2]);

    do {
        m = (Mensaje *) malloc(TAM*sizeof(char));
        m->info = (char *) malloc(TAM*sizeof(char));
        i = 0;
        while (!feof(f) && i < TAM) {
            fscanf(f, "%c", &buffer);
            m->info[i] = buffer;
            i++;
        }

        m->num_bytes = i+1;

        if (mq_send(cola, (char *) &m, sizeof(m), 1) == -1) {
            printf("Error enviando mensaje a la cola\n");
            fclose(f);
            mq_close(cola);
            return EXIT_FAILURE;
        }
    } while (!feof(f));

    fclose(f);
    mq_close(cola);

    return EXIT_SUCCESS;
}