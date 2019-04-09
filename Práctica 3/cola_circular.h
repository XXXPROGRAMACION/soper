#ifndef COLA_CIRCULAR
#define COLA_CIRCULAR

#define TAM 10

typedef struct {
    char caracteres[TAM+1];
    int pos_ini;
    int pos_fin;
    int tam;
} ColaCircular;

void cola_circular_inicializar(ColaCircular *cola_circular);
void cola_circular_insertar(ColaCircular *cola_circular, char caracacter);
char cola_circular_extraer(ColaCircular *cola_circular);

#endif