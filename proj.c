#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[64];

    inputHandler(argv, argc);
    udp_client(0, udp_encoder("WHOISROOT", buffer));
    
    printf("iamroot> ");

    return 0;
}

