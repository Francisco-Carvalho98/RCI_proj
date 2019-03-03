CFLAGS= -Wall

iamroot: proj.o inputHandler.o udp.o packetHandler.o tcp.o
	gcc -o iamroot proj.o inputHandler.o udp.o packetHandler.o tcp.o $(CFLAGS)

proj.o: proj.c defs.h
	gcc $(CFLAGS) -c proj.c 

inputHandler.o: inputHandler.c defs.h
	gcc $(CFLAGS) -c inputHandler.c 

packetHandler.o: packetHandler.c defs.h
	gcc $(CFLAGS) -c packetHandler.c 

udp.o: udp.c defs.h
	gcc $(CFLAGS) -c udp.c

tcp.o: tcp.c defs.h
	gcc $(CFLAGS) -c tcp.c


clean:
	rm -f *.o *.~ words *.gch
