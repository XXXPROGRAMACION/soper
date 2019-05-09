#ifndef NAVE_H
#define NAVE_H

#include "types.h"
#include "mapa.h"

void nave(tipo_mapa *mapa, int n_equipo, int n_nave, int fd_jefe[2], sem_t *sem_equipos_listos);

#endif