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
        else if (!strcasecmp(decoded->command, "POPRESP"));
        else if (!strcasecmp(decoded->command, "STREAMS")){node.udp.STREAMS = true;return;}
        else{printf("Unexpected error - udp\n");exit(EXIT_FAILURE);}

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
    if (!strcasecmp(decoded->command, "STREAMS")) printf("Streams:\nNo streams running\n");
    return;
}

void user_decoder (char * message){

    char command[10];
    char args[10];
    sscanf(message, "%s %s", command, args);
    if (!strcasecmp(command, "streams")) node.user.streams = true;
    else if (!strcasecmp(command, "exit")) node.user.exit_ = true;
    else if (!strcasecmp(command, "status")) node.user.status = true;
    else if (!strcasecmp(command, "tree")) node.user.tree = true;
    else if (!strcasecmp(command, "format")){
        if (!strcasecmp(args, "ascii")) input.format = true;
        else if (!strcasecmp(args, "hex")) input.format = false;
        else printf("Invalid command argument\n");}
    else if (!strcasecmp(command, "debug")){
        if (!strcasecmp(args, "on")) input.debug = true;
        else if (!strcasecmp(args, "off")) input.debug = false;
        else printf("Invalid command argument\n");}
    else if (!strcasecmp(command, "display")){
        if (!strcasecmp(args, "on")) input.display = true;
        else if (!strcasecmp(args, "off")) input.display = false;
        else printf("Invalid command argument\n");}
    else printf("Invalid command\n");
    
}

void ptp_encoder(char * command, char * data, int int1, int int2, struct ipport *ipport){
    char message[BUFFER_SIZE], *token;

         if (!strcasecmp(command, "DA")) sprintf(message, "DA %.4X\n%s", int1, data);
    else if (!strcasecmp(command, "NP")) sprintf(message, "NP %s:%s\n", input.ipaddr, input.tport);
    else if (!strcasecmp(command, "WE")) sprintf(message, "WE %s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
    else if (!strcasecmp(command, "RE")) sprintf(message, "RE %s:%s\n", new_fds[0].ipport.ip, new_fds[0].ipport.port);
    else if (!strcasecmp(command, "PQ")) sprintf(message, "PQ %s %d\n", data ,int1);
    else if (!strcasecmp(command, "PR")) sprintf(message, "PR %04X %s:%s %d\n", int2, input.ipaddr, input.tport, int1);
    else if (!strcasecmp(command, "TQ")) sprintf(message, "TQ %s:%s\n", ipport->ip, ipport->port);
    else if (!strcasecmp(command, "TR")){sprintf(message, "TR %s:%s %d\n", input.ipaddr, input.tport, input.tcpsessions);
        for (int i = 0; i < input.tcpsessions; i++){ 
            token = &message[0]; token += strlen(message);
            if (new_fds[i].fd != -1) sprintf(token, "%s:%s\n", new_fds[i].ipport.ip, new_fds[i].ipport.port);}
        strcat(message, "\n");
    } 
    else{printf("Unexpected error - ptp_encoder\n");exit(EXIT_FAILURE);}

    strcpy(data, message);
}

void ptp_decoder (char *message, struct message *decoded, int key){

    char command[10];
    char args[4][60];

    //catches DA, TR, SF, BS, TQ
    sscanf(message, "%s %s", command, args[0]);
         if (!strcasecmp(command, "DA")){node.ptp.DA = true;return;}
    else if (!strcasecmp(command, "TR")){node.ptp.TR = true;return;}
    else if (!strcasecmp(command, "SF")){node.ptp.SF = true;return;}
    else if (!strcasecmp(command, "BS")){node.ptp.BS = true;return;}

    //catches PR
    if(sscanf(message, "%s %s %s %s", command, args[0], args[1], args[2]) == 4){
        if (!strcasecmp(command, "PR")){node.ptp.PR = true; 
            sscanf(message, "%*s %hX %[^:]%*[:]%s %hd", &decoded->keys[0], decoded->address.ip, decoded->address.port, &decoded->keys[1]);
            return;}
        else{printf("Bad ptp message format\n%s\n", message);exit(1);}}

    //catches PQ
    if(sscanf(message, "%s %hX %hd", command, &decoded->keys[0], &decoded->keys[1]) == 3){
        if (!strcasecmp(command, "PQ")){node.ptp.PQ = true;return;}       
        else{printf("Bad ptp message format\n%s\n", message);exit(1);}} 
    
    //catches WE, NP, RE
    if(sscanf(message, "%s %s", command, args[0]) == 2){
        if(!strcasecmp(command, "WE")){node.ptp.WE = true;
                sscanf(args[0], "%[^:]%*[:]%[^:]%*[:]%s", args[1], args[2], args[3]);
                if (!strcasecmp(input.stream_id.name, args[1]) 
                &&  !strcasecmp(input.stream_id.ip, args[2]) 
                &&  !strcasecmp(input.stream_id.port, args[3])){
                    if(input.debug)printf("Connected to desired stream\n");
                    return;
                }else{printf("Connected to wrong stream, exiting...\n");exit(1);}
        }
        else if(!strcasecmp(command, "RE")){node.ptp.RE = true;
            sscanf(args[0], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
        }
        else if(!strcasecmp(command, "TQ")){node.ptp.TQ = true;
            sscanf(args[0], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
        }
        else if(!strcasecmp(command, "NP")){node.ptp.NP = true;
            sscanf(args[0], "%[^:]%*[:]%s", decoded->address.ip, decoded->address.port);
            Array_Addipport(new_fds, key, decoded->address);
            if(input.SF) write(key, "SF\n", 3);
        }
        else{printf("Bad ptp message format\n%s\n", message);exit(1);}}
}

int checkForMany(char *yetToProcess, char *readyToRead){

    int size = 0;
    char *token = NULL;
    
    if (!strncasecmp(yetToProcess, "DA", 2)){
        sscanf(yetToProcess, "%*s %X", &size);
        token = &yetToProcess[0]; token += 8+size;
        strncpy(readyToRead, yetToProcess, 8+size);
    }else if(!strncasecmp(yetToProcess, "TR", 2)){
        for (int i = 0; yetToProcess[i] != '\0'; i++)
            if(yetToProcess[i] == '\n' && yetToProcess[i+1] == '\n'){ 
                token = &yetToProcess[i+2];
                strncpy(readyToRead, yetToProcess, i+2);
                break;}
    }else 
        for (int i = 0; yetToProcess[i] != '\0'; i++)
            if(yetToProcess[i] == '\n'){
                token = &yetToProcess[i+1];
                strncpy(readyToRead, yetToProcess, i+1);
                break;}         
                
    strcpy(yetToProcess, token);
    if (token != NULL && !strcasecmp(yetToProcess, "\0")) return 0;
    else if (token == NULL){printf("This shouldnt happen -> checkForMany()\n");exit(1);}
    else return 1;
}