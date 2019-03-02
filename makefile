CFLAGS= -Wall

iamroot: proj.o inputHandler.o udp_client.o packetHandler.o tcp_client.o
	gcc -o iamroot proj.o inputHandler.o udp_client.o packetHandler.o tcp_client.o $(CFLAGS)

proj.o: proj.c defs.h
	gcc $(CFLAGS) -c proj.c 

inputHandler.o: inputHandler.c defs.h
	gcc $(CFLAGS) -c inputHandler.c 

packetHandler.o: packetHandler.c defs.h
	gcc $(CFLAGS) -c packetHandler.c 

udp_client.o: udp_client.c defs.h
	gcc $(CFLAGS) -c udp_client.c

tcp_client.o: tcp_client.c defs.h
	gcc $(CFLAGS) -c tcp_client.c


clean:
	rm -f *.o *.~ words *.gch
