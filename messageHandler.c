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

    //catches a ROOTIS, POPRESP and non empty STREAM
    if (sscanf(message, "%s %s %s", decoded->command, decoded->args[0], decoded->args[1]) == 3){
        if (!strcasecmp(decoded->command, "ROOTIS")) node.udp.ROOTIS = true;
        else if (!strcasecmp(decoded->command, "POPRESP")) node.udp.POPRESP = true;
        else if (!strcasecmp(decoded->command, "STREAMS")){
            message += 8;//skips "STREAMS\n"
            printf("Streams:\n");
            printf("%s", message);
            return;
        }else{printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}
        //splits address into ip and port
        sscanf(decoded->args[1], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
        return;}

    //catches a URROOT, ERROR
    if (sscanf(message, "%s %s", decoded->command, decoded->args[0]) == 2){
        if (!strcasecmp(decoded->command, "URROOT")) node.udp.URROOT = true;
        else if (!strcasecmp(decoded->command, "ERROR")){node.udp.ERROR = true;return;}
        else {printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}
        //splits address into ip and port, ignores stream name
        sscanf(decoded->args[0], "%*[^:]%*[:]%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
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
    else if (!strcasecmp(command, "debug")){
        if (!strcasecmp(args, "on")) input.debug = true;
        else if (!strcasecmp(args, "off")) input.debug = false;
        else printf("Invalid command argument\n");}
    else if (!strcasecmp(command, "display")){
        if (!strcasecmp(args, "on")) input.display = true;
        else if (!strcasecmp(args, "off")) input.display = false;
        else printf("Invalid command argument\n");}
    else if (!strcasecmp(command, "exit")) node.user.exit_ = true;
    else if (!strcasecmp(command, "format")){
        if (!strcasecmp(args, "ascii")) input.format = true;
        else if (!strcasecmp(args, "hex")) input.format = false;
        else printf("Invalid command argument\n");}
    else if (!strcasecmp(command, "status")) node.user.status = true;
    else if (!strcasecmp(command, "tree")) node.user.tree = true;
    else printf("Invalid command\n");
    
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
    else if (!strcasecmp(command, "PQ")) sprintf(message, "PQ %s %d\n",data ,size);
    else{printf("Unexpected error - ptp_encoder\n");exit(EXIT_FAILURE);}

    strcpy(data, message);
}

void ptp_decoder (char *message, struct message *decoded, int key){

    char command[10];
    char args[4][60];

    //catches DA, TR
    sscanf(message, "%s %s", command, args[0]);
    if (!strcasecmp(command, "DA")){node.ptp.DA = true;return;}
    else if (!strcasecmp(command, "TR")){node.ptp.TR = true;return;}
    else if(!strcasecmp(command, "SF")){node.ptp.SF = true;return;}
    else if(!strcasecmp(command, "BS")){node.ptp.BS = true;return;}
    //else{printf("Unexpected error ptp_decoder\n");exit(EXIT_FAILURE);}

    //catches PR
    if(sscanf(message, "%s %s %s %s", command, args[0], args[1], args[2]) == 4);

    //catches PQ
    if(sscanf(message, "%s %s %s", command, decoded->args[0], decoded->args[1]) == 3){
        if (!strcasecmp(command, "PQ")){node.ptp.PQ = true;return;}       
        else{printf("uh\n"); printf("Bad ptp message format %s\n", command);exit(1);}} 
    
    //catches WE, NP, RE, TQ
    if(sscanf(message, "%s %s", command, args[0]) == 2){
        if(!strcasecmp(command, "WE")){node.ptp.WE = true;
                sscanf(args[0], "%[^:]%*[:]%[^:]%*[:]%s", args[1], args[2], args[3]);
                if (!strcasecmp(input.stream_id.name, args[1]) && !strcasecmp(input.stream_id.ip, args[2]) 
                                                               && !strcasecmp(input.stream_id.port, args[3])){
                    if(input.debug)printf("Connected to desired stream\n");
                    return;
                }else{printf("Connected to wrong stream, exiting...\n");exit(1);}
        }
        else if(!strcasecmp(command, "RE")){node.ptp.RE = true;
            sscanf(args[0], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
        }
        else if(!strcasecmp(command, "NP")){node.ptp.NP = true;
            sscanf(args[0], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
            Array_Addipport(new_fds, key, decoded->address);
            if(input.SF) write(key, "SF\n", 3);
            else write(key, "BS\n", 3);
        }else{printf("Bad ptp message format %s\n", command);exit(1);}}
}