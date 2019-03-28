#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#define MEM "mem"

typedef struct {
    int previous_id; //!< Id of the previous client.
    int id; //!< Id of the current client.
    char name[NAME_MAX]; //!< Name of the client.
} ClientInfo;

int main(int argc, char **argv) {
    int n, i;
    pid_t pid;
    ClientInfo *info = NULL;

    if (argc != 2) {
        printf("Número de argumentos inválido.\n");
        return 1;
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        printf("Número de procesos inválido.\n");
        return 1;
    }

    int fd_shm = shm_open(MEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd_shm == -1) {
        printf("Error creando la memoria compartida.\n");
        return 1;
    }

    info = nmap(NULL, sizeof(ClientInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (info == NULL) {
        shm_unlink(fd_shm);
        printf("Error creando la memoria compartida.\n");
        return 1;
    }


    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) break;
    }
}