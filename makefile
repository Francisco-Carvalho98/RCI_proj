CFLAGS= -Wall -g

iamroot: iamroot.o inputHandler.o udp.o messageHandler.o tcp.o
	gcc -o iamroot iamroot.o inputHandler.o udp.o messageHandler.o tcp.o $(CFLAGS)

iamroot.o: iamroot.c defs.h
	gcc $(CFLAGS) -c iamroot.c 

inputHandler.o: inputHandler.c defs.h
	gcc $(CFLAGS) -c inputHandler.c 

messageHandler.o: messageHandler.c defs.h
	gcc $(CFLAGS) -c messageHandler.c 

udp.o: udp.c defs.h
	gcc $(CFLAGS) -c udp.c

tcp.o: tcp.c defs.h
	gcc $(CFLAGS) -c tcp.c


clean:
	rm -f *.o *.~ words *.gch
