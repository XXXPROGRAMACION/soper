/**
 * @file types.h
 * @brief Define los tipos y constantes de datos
 */

#ifndef TYPES_H
#define TYPES_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <semaphore.h>

#define N_EQUIPOS 4 							///< Número de equipos
#define N_NAVES 3 								///< Número de naves por equipo

#define BUFFER_SIZE 80 							///< Tamaño del buffer que se usa para leer de los pipes
#define TAM 256    								///< Tamaño máximo de los mensajes
#define MAX_MSG 10								///< Número máximo de mensajes en la cola

#define TURNO "turno" 							///< Mensaje que envía el simulador a los jefes para indicar que es un turno nuevo
#define FIN "fin"								///< Mensaje que envía el simulador a los jefes para que estos finalicen
#define MOVER_ALEATORIO "mover_aleatorio" 		///< Mensaje que envía el jefe a las naves para que estas se muevan
#define ATACAR "atacar" 						///< Mensaje que envía el jefe a las naves para que estas ataquen
#define DESTRUIR "destruir"						///< Mensaje que envía el jefe a las naves para que estas finalicen

#define NOMBRE_COLA "/cola_simulador"			///< Nombre de la cola que comunica a las naves con el simulador

#define SEM_EQUIPOS_LISTOS "sem_equipos_listos"	///< Nombre del semáforo que controla que todos los equipos estén listos
#define SEM_ALARMA "sem_alarma"					///< Nombre del semáforo que controla la alarma del simulador
#define SEM_MONITOR "sem_monitor"				///< Nombre del semáforo que controla la sincronización simulador-monitor

#define SHM "shm"								///< Nombre de la memoria compartida

extern char symbol_equipos[N_EQUIPOS]; 			///< Símbolos de los diferentes equipos en el mapa (mapa.c)

typedef struct {
	char info[TAM];
	int n_nave;
	int n_equipo;
	int x;
	int y;
} Mensaje; 	///< Estructura de los mensajes que envían las naves al simulador

/*** SCREEN ***/
#define MAPA_MAX_X 20 		  ///< Número de columnas del mapa
#define MAPA_MAX_Y 20 		  ///< Número de filas del mapa
#define SCREEN_REFRESH 10000  ///< Frequencia de refresco del mapa en el monitor
#define SYMB_VACIO '.' 		  ///< Símbolo para casilla vacia
#define SYMB_TOCADO '%' 	  ///< Símbolo para tocado
#define SYMB_DESTRUIDO 'X' 	  ///< Símbolo para destruido
#define SYMB_AGUA 'w' 		  ///< Símbolo para agua

/*** SIMULACION ***/
#define VIDA_MAX 10 		  ///< Vida inicial de una nave
#define ATAQUE_ALCANCE 20 	  ///< Distancia máxima de un ataque
#define ATAQUE_DANO 50 	      ///< Daño de un ataque
#define TURNO_SECS 2 		  ///< Segundos que dura un turno

/*** MAPA ***/
// Información de nave
typedef struct {
	int vida; 		///< Vida que le queda a la nave
	int pos_x; 		///< Columna en el mapa
	int pos_y; 		///< Fila en el mapa
	int equipo; 	///< Equipo de la nave
	int num_nave; 	///< Numero de la nave en el equipo
	bool viva; 		///< Si la nave está viva o ha sido destruida
} tipo_nave;

// Información de una casilla en el mapa
typedef struct {
	char simbolo; 	///< Símbolo que se mostrará en la pantalla para esta casilla
	int equipo; 	///< Si está vacia = -1. Si no, número de equipo de la nave que está en la casilla
	int num_nave; 	///< Número de nave en el equipo de la nave que está en la casilla
} tipo_casilla;

typedef struct {
	tipo_nave info_naves[N_EQUIPOS][N_NAVES];		///< Naves de cada equipo
	tipo_casilla casillas[MAPA_MAX_Y][MAPA_MAX_X];	///< Casillas que componen el mapa
	int num_naves[N_EQUIPOS];						///< Número de naves vivas en un equipo
} tipo_mapa;

#endif