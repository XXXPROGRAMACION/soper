#include "mapa.h"

char symbol_equipos[N_EQUIPOS] = {'A', 'B', 'C'};

int mapa_clean_casilla(tipo_mapa *mapa, int pos_y, int pos_x) {
	mapa->casillas[pos_y][pos_x].equipo = -1;
	mapa->casillas[pos_y][pos_x].num_nave = -1;
	mapa->casillas[pos_y][pos_x].simbolo = SYMB_VACIO;
	return 0;
}

tipo_casilla mapa_get_casilla(tipo_mapa *mapa, int pos_y, int pos_x) {
	return mapa->casillas[pos_y][pos_x];
}

int mapa_get_distancia(tipo_mapa *mapa, int ori_y, int ori_x, int target_y, int target_x) {
	int dist_x, dist_y;

	dist_x = abs(target_x-ori_x);
	dist_y = abs(target_y-ori_y);
	return (dist_x > dist_y) ? dist_x : dist_y;
}

tipo_nave mapa_get_nave(tipo_mapa *mapa, int equipo, int num_nave) {
	return mapa->info_naves[equipo][num_nave];
}

int mapa_get_num_naves(tipo_mapa *mapa, int equipo) {
	return mapa->num_naves[equipo];
}

char mapa_get_symbol(tipo_mapa *mapa, int pos_y, int pos_x) {
	return mapa->casillas[pos_y][pos_x].simbolo;
}

bool mapa_is_casilla_vacia(tipo_mapa *mapa, int pos_y, int pos_x) {
	return (mapa->casillas[pos_y][pos_x].equipo < 0);
}

void mapa_restore(tipo_mapa *mapa) {
	int i, j;

	for (i = 0; i < MAPA_MAX_Y; i++) {
		for (j = 0; j < MAPA_MAX_X; j++) {
			tipo_casilla cas = mapa_get_casilla(mapa, i, j);
			if (cas.equipo < 0) {
				mapa_set_symbol(mapa, i, j, SYMB_VACIO);
			} else {
				mapa_set_symbol(mapa, i, j, symbol_equipos[cas.equipo]);
			}
		}
	}
}

void mapa_set_symbol(tipo_mapa *mapa, int pos_y, int pos_x, char symbol) {
	mapa->casillas[pos_y][pos_x].simbolo = symbol;
}

int mapa_set_nave(tipo_mapa *mapa, tipo_nave nave) {
	if (nave.equipo >= N_EQUIPOS) return -1;
	if (nave.num_nave >= N_NAVES) return -1;
	mapa->info_naves[nave.equipo][nave.num_nave] = nave;
	if (nave.viva) {
		mapa->casillas[nave.pos_y][nave.pos_x].equipo = nave.equipo;
		mapa->casillas[nave.pos_y][nave.pos_x].num_nave = nave.num_nave;
		mapa->casillas[nave.pos_y][nave.pos_x].simbolo = symbol_equipos[nave.equipo];
	} else {
		mapa_clean_casilla(mapa,nave.pos_y, nave.pos_x);
	}
	return 0;
}

void mapa_set_num_naves(tipo_mapa *mapa, int equipo, int num_naves) {
	mapa->num_naves[equipo] = num_naves;
}

void mapa_send_misil(tipo_mapa *mapa, int origen_y, int origen_x, int target_y, int target_x) {
	int p_x = origen_x;
	int p_y = origen_y;
	int t_x = target_x;
	int t_y = target_y;
	char p_s = mapa_get_symbol(mapa, p_y, p_x);
	int next_x, next_y;
	char nexts;

	int run = t_x-origen_x;
	int rise = t_y-origen_y;
	float m = ((float) rise)/((float) run);
	float b = origen_y-(m*origen_x);
	int inc = (origen_x < t_x) ? 1 : -1;

	for (next_x = origen_x; ((origen_x < t_x) && (next_x <= t_x)) || (( origen_x > t_x) && (next_x >= t_x)); next_x += inc) {
		// solve for y
		float y = (m * next_x) + b;

		// round to nearest int
		next_y = (y > 0.0) ? floor(y+0.5) : ceil(y-0.5);

		if ((next_y < 0) || (next_y >= MAPA_MAX_Y)) continue;

		nexts = mapa_get_symbol(mapa, next_y, next_x);
		mapa_set_symbol(mapa, next_y, next_x, '*');
		mapa_set_symbol(mapa, p_y, p_x, p_s);
		usleep(50000);
		p_x = next_x;
		p_y = next_y;
		p_s = nexts;
	}

	mapa_set_symbol(mapa, p_y, p_x, p_s);
}