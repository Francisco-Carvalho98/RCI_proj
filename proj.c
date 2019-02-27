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

    printf("iamroot> ");
    fgets(buffer, 10, stdin);
    printf("Echo: %s\n", buffer);

    return 0;
}