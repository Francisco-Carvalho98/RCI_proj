#include "defs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern struct input input;

void udp_encoder (char *command, char *message){

    if (strcmp(command, "WHOISROOT") == 0){
        sprintf(message, "WHOISROOT %s:%s:%s %s:%s\n", input.stream_id.name
                                                     , input.stream_id.ip
                                                     , input.stream_id.port, input.ipaddr
                                                     , input.tport );}

    if (strcmp(command, "REMOVE") == 0){
        sprintf(message, "REMOVE %s:%s:%s\n", input.stream_id.name
                                            , input.stream_id.ip
                                            , input.stream_id.port);}

    return;
}

void udp_decoder (char *message, struct message *decoded){
    
    int i;
    if ((i = sscanf(message, "%s %s", decoded->command, decoded->args[0])) == 2) return;
        
    printf("%d\n", i);

    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3) return;
}