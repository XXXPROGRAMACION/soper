/*
 * Ejemplo de codigo que genera un numero aleatorio y lo muestra por pantalla
*/
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>

int main(int argc, char *argv[]) {
    int fd1[2], fd2[2];
    int pid1, pid2;
    int rand_aux;

    srand(time(NULL));

    if (pipe(fd1) == -1) {
        printf("Error en el pipe.\n");
        exit(EXIT_FAILURE);
    }

    pid1 = fork();
    if (pid1 == -1) {
        printf("Error en el fork.\n");
        exit(EXIT_FAILURE);
    }

    if (pid1 != 0) {
        if (pipe(fd2) == -1) {
            printf("Error en el pipe.\n");
            exit(EXIT_FAILURE);
        }

        pid2 = fork();
        if (pid2 == -1) {
            printf("Error en el fork.\n");
            wait(NULL);
            exit(EXIT_FAILURE);
        }
    }

    if (pid1 == 0) {
        close(fd1[0]);
        rand_aux = rand();
        write(fd1[1], &rand_aux, sizeof(int));
        printf("HIJO 1: numero aleatorio %d\n", rand_aux);
        exit(EXIT_SUCCESS);
    } else if (pid2 == 0) {
        close(fd1[0]);
        close(fd1[1]);
        close(fd2[1]);
        read(fd2[0], &rand_aux, sizeof(int));
        printf("HIJO 2: el numero aleatorio recibido es: %d\n", rand_aux);
        exit(EXIT_SUCCESS);
    } else {
        close(fd1[1]);
        close(fd2[0]);
        read(fd1[0], &rand_aux, sizeof(int));
        write(fd2[1], &rand_aux, sizeof(int));
        while (wait(NULL) > 0);
    }

    exit(EXIT_SUCCESS);
}
