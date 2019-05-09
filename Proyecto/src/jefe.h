#ifndef JEFE_H
#define JEFE_H

#include "types.h"
#include "nave.h"

/** Función que emula el comportamiento de un jefe. Se ocupa de crear los procesos nave y de mandarles
 *  órdenes de acción en cada turno. Gestiona la comunicación entre el simulador y las naves.
 *  @param mapa Puntero al mapa sobre el que debe trabajar el jefe
 *  @param n_equipo Número de equipo del jefe
 *  @param fd_sim Pipe que le permite comunicarse con el simulador
 *  @param sem_equipos_listos Semáforo que permite al simulador saber cúando estan listos los jefes
 *  @see nave()
 *  @see simulador() 
 */
void jefe(tipo_mapa *mapa, int n_equipo, int fd_sim[2], sem_t *equipos_listos);

#endif