CC = gcc
CFLAGS = -Wall -pedantic
EXE = ejercicio2 ejercicio2_solved
EXE_E3 = ejercicio3_productor ejercicio3_consumidor
EXE_E4 = ejercicio4 a b c

all: $(EXE) $(EXE_E3) $(EXE_E4)
	
$(EXE): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -lpthread

$(EXE_E3): %: %.c cola_circular.h
	$(CC) $(CFLAGS) $< cola_circular.c -o $@ -lrt -lpthread

$(EXE_E4): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -lpthread
	
clean: 
	rm $(EXE)
	rm $(EXE_E3)
	rm $(EXE_E4)