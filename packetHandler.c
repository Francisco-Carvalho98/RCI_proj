#include "defs.h"

extern struct input input;

void udp_encoder (char *command, char *message){//WHOISROOT, REMOVE, DUMP, POPREQ
    //builds WHOISROOT message
    if (strcmp(command, "WHOISROOT") == 0){
        sprintf(message, "WHOISROOT %s:%s:%s %s:%s\n", input.stream_id.name
                                                     , input.stream_id.ip
                                                     , input.stream_id.port, input.ipaddr
                                                     , input.tport );}
    //builds REMOVE message
    if (strcmp(command, "REMOVE") == 0){
        sprintf(message, "REMOVE %s:%s:%s\n", input.stream_id.name
                                            , input.stream_id.ip
                                            , input.stream_id.port);}

    return;
}

void udp_decoder (char *message, struct message *decoded){//URROOT, ROOTIS, STREAMS, ERROR, POPRESP

    //catches a ROOTIS
    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3){
        //TODO build message.address
        return;}

    //catches a URROOT, POPRESP
    if (sscanf(message, "%s %s", decoded->command, decoded->args[0]) == 2){
        //TODO build message.address
        return;}
    
    //TODO process STREAMS, ERROR
    return;
}