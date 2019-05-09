/** @file nave.h
 *  @brief Recoge la funcionalidad de una nave
 */

#ifndef NAVE_H
#define NAVE_H

#include "types.h"
#include "mapa.h"

/** Emula el comportamiento de una nave, "obedeciendo" las órdenes de su jefe e informando de sus
 *  acciones al simulador. 
 *  @param mapa Mapa sobre el que se situa la nave
 *  @param n_equipo Número de equipo de la nave
 *  @param n_nave Número de la nave
 *  @param fd_jefe Pipe que conecta la nave con su jefe 
 *  @see jefe()
 *  @see simulador()
 */ 
void nave(tipo_mapa *mapa, int n_equipo, int n_nave, int fd_jefe[2]);

#endif