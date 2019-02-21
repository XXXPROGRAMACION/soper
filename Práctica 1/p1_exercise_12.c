#include <stdio.h>
#include <pthread.h>

#define N 50

typedef struct {
    int entrada;
    long long salida;
} Argumentos;

void *potencia2(void *argumentos) {
    ((Argumentos *) argumentos)->salida = 1L << ((Argumentos *) argumentos)->entrada;

    pthread_exit(NULL);
}

int main() {
    pthread_t hilos[N];
    Argumentos argumentos[N];
    int i;

    for (i = 0; i < N; i++) {
        argumentos[i].entrada = i+1;
        argumentos[i].salida = -1;

        pthread_create(&hilos[i], NULL, potencia2, &argumentos[i]);
    }

    printf("Resultados:\n");
    for (i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
        
        printf("\t2^%d = %lld\n", argumentos[i].entrada, argumentos[i].salida);
    }

    return 0;
}