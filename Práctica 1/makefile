CC = gcc
CFLAGS = -Wall -pedantic
EXE = p1_exercise_3 p1_exercise_4 p1_exercise_5 p1_exercise_7 p1_exercise_9 p1_exercise_12

all: $(EXE)
	
$(EXE): %: %.c
	$(CC) $(CFLAGS) $< -o $@ -lpthread
	
clean: 
	rm $(EXE) *.o