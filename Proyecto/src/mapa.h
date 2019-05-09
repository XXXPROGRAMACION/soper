/**
 * @file mapa.h
 * @brief Cabecera del mapa
 */

#ifndef SRC_MAPA_H_
#define SRC_MAPA_H_

#include "types.h"

/**
 * Reinicia la casilla especificada.
 * @param mapa Mapa de la simulación
 * @param pos_y Coordenada y de la casilla
 * @param pos_x Coordenada x de la casilla
 * @return Devuelve 0
 */
int mapa_clean_casilla(tipo_mapa *mapa, int pos_y, int pos_x);

/**
 * Obtiene la casilla especificada.
 * @param mapa Mapa de la simulación
 * @param pos_y Coordenada y de la casilla
 * @param pos_x Coordenada x de la casilla
 * @return La casilla especificada.
 */
tipo_casilla mapa_get_casilla(tipo_mapa *mapa, int posy, int posx);

/**
 * Obtiene la distancia de las coordenadas de origen a las coordenadas objetivo.
 * @param mapa Mapa de la simulación
 * @param ori_y Coordenada y origen
 * @param ori_x Coordenada x origen
 * @param target_y Coordenada y objetivo
 * @param targer_x Coordenada x objetivo
 * @return Valor máximo entre la distancia en el eje X y la distancia en el eje Y
 */
int mapa_get_distancia(tipo_mapa *mapa, int ori_y, int ori_x, int target_y, int target_x);

/**
 * Devuelve la nave especificada del equipo indicado.
 * @param mapa Mapa de la simulación
 * @param equipo Equipo de la nave a devolver
 * @param num_nave Número de la nave a devolver
 * @return Nave especificada del equipo indicado.
 */
tipo_nave mapa_get_nave(tipo_mapa *mapa, int equipo, int num_nave);

/**
 * Devuelve el número de naves restantes del equipo indicado.
 * @param mapa Mapa de la simulación
 * @param equipo Equipo del que se desean conocer las naves restantes
 * @return Naves restantes del equipo indicado.
 */
int mapa_get_num_naves(tipo_mapa *mapa, int equipo);

/**
 * Devuelve el símbolo de la casilla especificada.
 * @param mapa Mapa de la simulación
 * @param pos_y Coordenada y de la casilla
 * @param pos_x Coordenada x de la casilla
 * @return Símbolo de la casilla especificada.
 */
char mapa_get_symbol(tipo_mapa *mapa, int pos_y, int pos_x);

/**
 * Indica si la casilla especificada está vacía.
 * @param mapa Mapa de la simulación
 * @param pos_y Coordenada y de la casilla
 * @param pos_x Coordenada x de la casilla
 * @return Si la casilla especificada está vacía o no.
 */
bool mapa_is_casilla_vacia(tipo_mapa *mapa, int pos_y, int pos_x);

/**
 * Restaura los símbolos de las casillas del mapa, conservando solo las naves.
 * @param mapa Mapa de la simulación
 */
void mapa_restore(tipo_mapa *mapa);

/**
 * Establece el símbolo de la casilla especificada.
 * @param mapa Mapa de la simulación
 * @param pos_y Coordenada y de la casilla
 * @param pos_x Coordenada x de la casilla
 * @param symbol Nuevo símbolo de la casilla especificada.
 */
void mapa_send_misil(tipo_mapa *mapa, int origen_y, int origen_x, int target_y, int target_x);

/**
 * Añade una nueva nave al mapa de la simulación.
 * @param mapa Mapa de la simulación
 * @param nave Nueva nave que añadir al mapa
 * @return Devuelve 0
 */
int mapa_set_nave(tipo_mapa *mapa, tipo_nave nave);

/**
 * Establece el número de naves restantes de un equipo.
 * @param mapa Mapa de la simulación
 * @param equipo Equipo al que se establecerá el nuevo valor de naves restantes
 * @param num_naves Naves restantes del equipo especificado
 */
void mapa_set_num_naves(tipo_mapa *mapa, int equipo, int num_naves);

/**
 * Imprime por pantalla la animación de un misil dirigido desde las coordenadas de origen a las de
 * destino.
 * @param mapa Mapa de la simulación
 * @param ori_y Coordenada y origen
 * @param ori_x Coordenada x origen
 * @param target_y Coordenada y objetivo
 * @param targer_x Coordenada x objetivo
 */
void mapa_set_symbol(tipo_mapa *mapa, int pos_y, int pos_x, char symbol);

#endif /* SRC_MAPA_H_ */