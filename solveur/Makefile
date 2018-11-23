TEST = gcc -Wall -g3
FAST = gcc -O3 -Wno-unused-result
COMPIL = $(FAST)
OBJ = parser.o structures.o prefiltrage.o tri.o heuristique.o backtracking.o forwardchecking.o


solver: main.c $(OBJ)
	$(COMPIL) -o $@ $^

%.o: %.c %.h
	$(COMPIL) -c $<

clean:
	rm -f solver *.o
