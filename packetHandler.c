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

    char *token;
    //catches a ROOTIS
    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3){
        //splits address into ip and port
        token = strtok(decoded->args[1],":");
        strcpy(decoded->address.ip,token);
        token = strtok(NULL," ");
        strcpy(decoded->address.port,token);
        return;}

    //catches a URROOT, POPRESP
    if (sscanf(message, "%s %s", decoded->command, decoded->args[0]) == 2){
        //splits address into ip and port
        token = strtok(decoded->args[0],":");//neglets stream name
        token = strtok(NULL,":");
        strcpy(decoded->address.ip,token);
        token = strtok(NULL," ");
        strcpy(decoded->address.port,token);
        printf("decoded: %s %s\n", decoded->address.ip, decoded->address.port);
        return;}
    
    //TODO process STREAMS, ERROR
    return;
}