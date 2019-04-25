TARGET = ../target

CC = gcc
CFLAGS = -g -Wall -pthread
LDLIBS = -lrt -lncurses

BOLD=\e[1m
NC=\e[0m

all: simulador #monitor

clean: 
	rm -r -f $(TARGET)

simulador:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) simulador.c mapa.c jefe.c nave.c -o $(TARGET)/simulador -lrt -lm
	
monitor:
	mkdir -p $(TARGET)
	$(CC) $(CFLAGS) gamescreen.c mapa.c monitor.c -o $(TARGET)/monitor -lrt -lncurses -lm