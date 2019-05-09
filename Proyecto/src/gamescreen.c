/**
 * @file gamescreen.c
 * @brief Código fuente del modo pantalla
 */

#include <ncurses.h>

/**
 * Inicializa el modo pantalla en el terminal. Debe hacerse antes que cualquier otra función screen.
 */
void screen_init() {
	initscr();
	clear();
	noecho();
	cbreak();
	keypad(stdscr, FALSE);
	curs_set(0);
}

/**
 * Fija en pantalla un símbolo en la posición (fila, columna). Siendo (0, 0) la esquina superior
 * izquierda. El símbolo no se mostrará hasta el próximo screen_refresh().
 * @param row Fila a la que añadir el carácter especificado
 * @param col Columna a la que añadir el carácter especificado
 * @param symbol Símbolo a añadir
 */
void screen_addch(int row, int col, char symbol) {
	mvaddch(row, col, symbol);
}

/**
 * Refresca lo que muestra la pantalla. En principio, no hay que hacer refresh cada vez que se añade
 * un símbolo con screen_addch().
 */
void screen_refresh() {
	refresh();
}

/**
 * Finaliza el modo pantalla. Hay que hacerlo antes de finalizar el programa.
 */
void screen_end() {
	endwin();
}