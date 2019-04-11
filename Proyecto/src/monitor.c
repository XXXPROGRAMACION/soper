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
#include <stdbool.h>
#include <unistd.h>

#include "simulador.h"
#include "gamescreen.h"
#include "mapa.h"

void mapa_print(tipo_mapa *mapa) {
	int i, j;

	for (i = 0; i < MAPA_MAX_Y; i++) {
		for (j = 0; j < MAPA_MAX_X; j++) {
			tipo_casilla cas = mapa_get_casilla(mapa, i, j);
			screen_addch(i, j, cas.simbolo);
		}
	}
	screen_refresh();
}

int main() {
	screen_init();

	screen_end();

	exit(EXIT_SUCCESS);
}