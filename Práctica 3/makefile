CC = gcc
CFLAGS = -Wall -pedantic
EXE = ejercicio2 ejercicio2_solved a b c ejercicio4

all: $(EXE)
	
$(EXE): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lrt -lpthread
	
clean: 
	rm $(EXE)