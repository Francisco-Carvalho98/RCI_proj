#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[64];
    struct message message;

    inputHandler(argv, argc);
    udp_encoder("WHOISROOT", buffer);
    udp_client(0, buffer);
    udp_decoder(buffer, &message);
    printf("%s %s\n", message.command, message.args[0]);
    printf("iamroot> ");

    return 0;
}

