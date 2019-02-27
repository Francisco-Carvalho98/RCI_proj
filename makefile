CFLAGS= -Wall

iamroot: proj.o inputHandler.o
	gcc -o iamroot proj.o inputHandler.o $(CFLAGS)

proj.o: proj.c defs.h
	gcc $(CFLAGS) -c proj.c 

inputHandler.o: inputHandler.c defs.h
	gcc $(CFLAGS) -c inputHandler.c 

clean:
	rm -f *.o *.~ words *.gch
