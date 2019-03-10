#include "defs.h"

void udp_encoder (char *command, char *message, struct ipport *ipport){//WHOISROOT, REMOVE, DUMP, POPREQ
    //builds WHOISROOT message
    if (strcasecmp(command, "WHOISROOT") == 0){
        sprintf(message, "WHOISROOT %s:%s:%s %s:%s\n", input.stream_id.name
                                                     , input.stream_id.ip
                                                     , input.stream_id.port, input.ipaddr
                                                     , input.uport );}
    //builds REMOVE message
    if (strcasecmp(command, "REMOVE") == 0){
        sprintf(message, "REMOVE %s:%s:%s\n", input.stream_id.name
                                            , input.stream_id.ip
                                            , input.stream_id.port);}
    //builds POPRESP message
    if (!strcasecmp(command, "POPRESP")){
        sprintf(message, "POPRESP %s:%s:%s %s:%s\n", input.stream_id.name
                                                   , input.stream_id.ip
                                                   , input.stream_id.port
                                                   , ipport->ip, ipport->port);}

    return;
}

void udp_decoder (char *message, struct message *decoded){//URROOT, ROOTIS, STREAMS, ERROR, POPRESP

    char *token;
    //catches a ROOTIS, POPRESP
    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3){
        
        if (!strcasecmp(decoded->command, "ROOTIS")) node.udp.ROOTIS = true;
        else if (!strcasecmp(decoded->command, "POPRESP")) node.udp.POPRESP = true;
        else{printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}

        //splits address into ip and port
        token = strtok(decoded->args[1],":");
        strcpy(decoded->address.ip,token);
        token = strtok(NULL," ");
        strcpy(decoded->address.port,token);
        return;}

    //catches a URROOT, ERROR
    if (sscanf(message, "%s %s", decoded->command, decoded->args[0]) == 2){
        printf("%s\n", decoded->command);
        if (!strcasecmp(decoded->command, "URROOT")) node.udp.URROOT = true;
        else if (!strcasecmp(decoded->command, "ERROR")) node.udp.ERROR = true;
        else {printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}
        //splits address into ip and port
        token = strtok(decoded->args[0],":");//neglets stream name
        printf("token - %s\n", token);
        token = strtok(NULL,":");
        strcpy(decoded->address.ip,token);
        token = strtok(NULL," ");
        strcpy(decoded->address.port,token);
        printf("decoded: %s %s\n", decoded->address.ip, decoded->address.port);
        return;}
    
    //catches a POPREQ
    if (!strcasecmp(decoded->command, "POPREQ")) node.udp.POPREQ = true;
    //TODO process STREAM
    return;
}

void user_decode (char * command){

    sscanf(command, "%s %s", command, command);

    
}

void tcp_encoder(char * command, char * data, int size){
    char message[120];
    if (!strcasecmp(command, "DA")) 
        sprintf(message, "DA %.4X\n%s", size, data);

    printf("message: %s\n", message);
    printf("original data: %s\n", data);
    strcpy(data, message);
}