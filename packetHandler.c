#include "defs.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern struct input input;

char * udp_encoder (char *command, char *message){

    if (strcmp(command, "WHOISROOT") == 0){
        sprintf(message, "WHOISROOT %s:%s:%s %s:%s\n", input.stream_id.name
                                                     , input.stream_id.ip
                                                     , input.stream_id.port, input.ipaddr
                                                     , input.tport );}

    if (strcmp(command, "REMOVE") == 0){
        sprintf(message, "REMOVE %s:%s:%s\n", input.stream_id.name
                                            , input.stream_id.ip
                                            , input.stream_id.port);}

    return message;
}

void udp_decorder (char *message){


}