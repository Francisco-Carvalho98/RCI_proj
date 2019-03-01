CFLAGS= -Wall

iamroot: proj.o inputHandler.o UDP_client.o packetHandler.o
	gcc -o iamroot proj.o inputHandler.o UDP_client.o packetHandler.o $(CFLAGS)

proj.o: proj.c defs.h
	gcc $(CFLAGS) -c proj.c 

inputHandler.o: inputHandler.c defs.h
	gcc $(CFLAGS) -c inputHandler.c 

packetHandler.o: packetHandler.c defs.h
	gcc $(CFLAGS) -c packetHandler.c 

UDP_client.o: UDP_client.c defs.h
	gcc $(CFLAGS) -c UDP_client.c


clean:
	rm -f *.o *.~ words *.gch
