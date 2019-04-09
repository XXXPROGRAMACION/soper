#include "cola_circular.h"
#include <stdlib.h>

void cola_circular_inicializar(ColaCircular *cola_circular) {
    ColaCircular *cola_circular;
    int i;

    cola_circular->pos_ini = 0;
    cola_circular->pos_fin = 0;

    return cola_circular;
}

void cola_circular_insertar(ColaCircular *cola_circular, char caracacter) {

    if (cola_circular == NULL) return;
    if (cola_circular->caracteres == NULL) return;

    if ((cola_circular->pos_fin+1)%(TAM+1) == cola_circular->pos_ini) return;

    cola_circular->caracteres[cola_circular->pos_fin] = caracacter;
    cola_circular->pos_fin = (cola_circular->pos_fin+1)%(TAM+1);
} 

char cola_circular_extraer(ColaCircular *cola_circular) {
    char caracter;

    if (cola_circular == NULL) return 33; // Devolvemos escape en caso de error
    if (cola_circular->caracteres == NULL) return 33;

    if (cola_circular->pos_ini == cola_circular->pos_fin) return 33;

    caracter = cola_circular->caracteres[cola_circular->pos_ini];
    cola_circular->pos_ini = (cola_circular->pos_ini+1)%(TAM+1);

    return caracter;
}