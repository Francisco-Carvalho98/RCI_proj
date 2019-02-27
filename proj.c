#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"


int main ( int argc, char *argv[] )
{
    struct input input;
    char buffer[64];

    system("clear");

    inputHandler(&input, argv, argc);

    printf("StreamID: %s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
    printf("Self IP, TCP and UDP ports: %s - %s - %s\n", input.ipaddr, input.tport, input.uport);
    printf("Root Server: %s:%s\n", input.rs_id.adress, input.rs_id.port);
    printf("Application variables: %s (sessions) - %s (bestp) - %s (tsecs)\n", input.tcpsessions, input.bestpops, input.tsecs);
    printf("Flags: %d %d %d\n", input.display, input.advanced, input.help);

    //perror("printf()");

    printf("iamroot> ");
    fgets(buffer, 10, stdin);
    printf("Echo: %s\n", buffer);

    return 0;
}