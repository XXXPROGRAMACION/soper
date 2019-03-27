CC = gcc
CFLAGS = -Wall -pedantic
EXE = ejercicio2 ejercicio3d ejercicio4 ejercicio6a ejercicio6b ejercicio8 ejercicio9

all: $(EXE)
	
$(EXE): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread
	
clean: 
	rm $(EXE)