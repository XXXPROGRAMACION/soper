#ifndef JEFE_H
#define JEFE_H

#include "types.h"
#include "nave.h"

void jefe(tipo_mapa *mapa, int n_equipo, int fd_sim[2], sem_t *equipos_listos);

#endif