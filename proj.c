#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "defs.h"


int main ( int argc, char *argv[] )
{
    struct input input;

    system("clear");

    inputHandler(&input, argv, argc);



    return 0;
}