#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MEM "mem"
#define SEM "sem"
#define NAME_MAX 128

typedef struct {
    int previous_id;
    int id;
    char name[NAME_MAX]; 
} ClientInfo;

ClientInfo *info = NULL;
sem_t *sem = NULL;

void manejador_SIGUSR1(int pid) {
    if (info != NULL) {
        printf("\nId previa: %d\n", info->previous_id);
        printf("Id actual: %d\n", info->id);
        printf("Nombre: %s\n\n", info->name);
    }

    sem_post(sem); // Fin de la zona crítica
}

int main(int argc, char **argv) {
    pid_t pid;
    struct sigaction act;       
    int fd_shm, error; 
    int n, i;

    if (argc != 2) {
        printf("Número de argumentos inválido.\n");
        exit(EXIT_FAILURE);
    }

    n = atoi(argv[1]);
    if (n <= 0) {
        printf("Número de procesos inválido.\n");
        exit(EXIT_FAILURE);
    }

    fd_shm = shm_open(MEM, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd_shm == -1) {
        printf("Error creando la memoria compartida.\n");
        exit(EXIT_FAILURE);
    }
    shm_unlink(MEM);

    error =  ftruncate(fd_shm, sizeof(ClientInfo));
    if (error == -1) {
        fprintf(stderr, "Error cambiando el tamaño de la memoria compartida.\n");
        exit(EXIT_FAILURE);
    }

    info = mmap(NULL, sizeof(ClientInfo), PROT_READ | PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (info == NULL) {
        printf("Error enlazando la memoria compartida.\n");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }

    if ((sem = sem_open(SEM, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1)) == SEM_FAILED) {
        perror("sem_open");
        close(fd_shm);
        exit(EXIT_FAILURE);
    }
    sem_unlink(SEM);

    info->previous_id = -1;
    info->id = 0;

    for (i = 0; i < n; i++) {
        pid = fork();
        if (pid == 0) break;
    }

    if (pid != 0) { //Padre
        act.sa_handler = manejador_SIGUSR1;
        sigemptyset(&(act.sa_mask));
        act.sa_flags = 0;

        if (sigaction(SIGUSR1, &act, NULL) < 0) {
            perror("sigaction");            
            close(fd_shm);
            sem_close(sem);
            exit(EXIT_FAILURE);
        }

        while (wait(NULL) >= 0 || errno == EINTR);

        close(fd_shm);
        sem_close(sem);
        exit(EXIT_SUCCESS);
    } else { //Hijos
        srand(getpid());
        sleep(1+(double)rand()/RAND_MAX*10);

        sem_wait(sem); // Inicio de la zona crítica

        info->previous_id++;
        printf("¡Da de alta a un cliente!\n");
        printf("Introduce su nombre: ");
        scanf("%s", info->name);
        info->id++;

        kill(getppid(), SIGUSR1); // Se notifica al padre

        close(fd_shm);
        sem_close(sem);
        exit(EXIT_SUCCESS);
    }
}