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

void udp_decoder (char *message, struct message *decoded){//URROOT, ROOTIS, STREAMS, ERROR, POPRESP, POPREQ

    char *token;
    //catches a ROOTIS, POPRESP and non empty STREAM
    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3){
        if (!strcasecmp(decoded->command, "ROOTIS")) node.udp.ROOTIS = true;
        else if (!strcasecmp(decoded->command, "POPRESP")) node.udp.POPRESP = true;
        else if (!strcasecmp(decoded->command, "STREAMS")){
            message += 8;//skips STREAMS\n
            printf("Streams:\n");
            printf("%s", message);
            return;
        }
        else{printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}

        //splits address into ip and port
        token = strtok(decoded->args[1],":");
        strcpy(decoded->address.ip,token);
        token = strtok(NULL," ");
        strcpy(decoded->address.port,token);
        return;}

    //catches a URROOT, ERROR
    if (sscanf(message, "%s %s", decoded->command, decoded->args[0]) == 2){
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
    if (!strcasecmp(decoded->command, "POPREQ")){node.udp.POPREQ = true;return;}

    //catches empty stream
    printf("Streams:\nNo streams running\n");
    return;
}

void user_decoder (char * message){

    char command[10];
    char args[10];
    sscanf(message, "%s %s", command, args);
    if (!strcasecmp(command, "streams")) node.user.streams = true;
    else if (!strcasecmp(command, "debug")) node.user.debug = true;
    else if (!strcasecmp(command, "display")) node.user.display = true;
    else if (!strcasecmp(command, "exit")) node.user.exit_ = true;
    else if (!strcasecmp(command, "format")) node.user.format = true;
    else if (!strcasecmp(command, "status")) node.user.status = true;
    else if (!strcasecmp(command, "tree")) node.user.tree = true;
    else{printf("Unexpected error - user_decoder\n"); exit(EXIT_FAILURE);}
    
}

void ptp_encoder(char * command, char * data, int size){
    char message[BUFFER_SIZE];
    if (!strcasecmp(command, "DA")) sprintf(message, "DA %.4X\n%s", size, data);
    else if (!strcasecmp(command, "NP")) sprintf(message, "NP %s:%s\n", input.ipaddr, input.tport);
    else if (!strcasecmp(command, "WE")) sprintf(message, "WE %s:%s:%s\n", input.stream_id.name
                                                                         , input.stream_id.ip
                                                                         , input.stream_id.port);
    else if (!strcasecmp(command, "RE")) sprintf(message, "RE %s:%s\n", new_fds[0].ipport.ip
                                                                      , new_fds[0].ipport.port);
    else{printf("Unexpected error - ptp_encoder\n");exit(EXIT_FAILURE);}

    strcpy(data, message);
}

void ptp_decoder (char *message){

    char command[10];
    char args[4][60];

    //catches DA, TR
    sscanf(message, "%s %s", command, args[0]);
    if (!strcasecmp(command, "DA")) node.ptp.DA = true;
    else if (!strcasecmp(command, "TR")) node.ptp.TR = true;

    //catches PR
    if(sscanf(message, "%s %s %s %s", command, args[0]) == 4);

    //catches PQ
    if(sscanf(message, "%s %s %s", command, args[0]) == 3);
    
    //catches WE, NP, RE, TQ
    if(sscanf(message, "%s %s", command, args[0]) == 2);

    

    

    


    //TODO SF, BS
}