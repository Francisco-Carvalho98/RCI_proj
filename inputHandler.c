#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

void inputHandler (char **argv, int argc){
    
    char *token;
    char buffer[BUFFER_SIZE];
    struct message message;

    // DEFAULT DONE
    strcpy(input.ipaddr, "\n");
    strcpy(input.tport,"58000");
    strcpy(input.uport,"58000");
    strcpy(input.rs_id.ip,"193.136.138.142");
    strcpy(input.rs_id.port,"59000");
    input.tcpsessions = 1;
    input.bestpops = 1;
    input.tsecs = 5;
    input.display = true;
    input.debug = true;
    input.help = false;
    input.SF = false;

    if (argc == 1) {
        display_help();
        strcpy(buffer, "DUMP\n");
        udp_client(0, buffer, input.rs_id);
        udp_decoder(buffer, &message);
        exit(0);}

    // LOOKS FOR FLAGS
    for (int i = 1; i < argc; i++){
        if (strcasecmp(argv[i],"-i") == 0) strcpy(input.ipaddr,argv[++i]);   
        if (strcasecmp(argv[i],"-t") == 0) strcpy(input.tport,argv[++i]); 
        if (strcasecmp(argv[i],"-u") == 0) strcpy(input.uport,argv[++i]); 
        if (strcasecmp(argv[i],"-s") == 0){
            token = strtok(argv[++i],":");
            strcpy(input.rs_id.ip,token);
            token = strtok(NULL," ");
            strcpy(input.rs_id.port,token);}
        if (strcasecmp(argv[i],"-p") == 0) input.tcpsessions = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-n") == 0) input.bestpops = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-x") == 0) input.tsecs = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-b") == 0) input.display = false;
        if (strcasecmp(argv[i],"-d") == 0) input.debug = true;
        if (strcasecmp(argv[i],"-h") == 0) input.help = true;
    } 


    if (input.help == true){display_help(); exit(0);}
    if (strcasecmp(input.ipaddr, "\n") == 0){printf("Must specify application IP\n");display_help();exit(0);}

    // STREAM NAME
    token = strtok(argv[1],":");
    strcpy(input.stream_id.name,token);
    // STREAM IP
    token = strtok(NULL,":");
    strcpy(input.stream_id.ip,token);

    // STREAM PORT
    token = strtok(NULL," ");
    strcpy(input.stream_id.port,token);

    return;
}

void display_help (){

    printf("./iamroot [<streamID>] [-i <ipaddr>] [-t <tport>] [-u <uport>]\n"); 
    printf("\t\t       [-s <rsaddr>[:<rsport>]]\n");
    printf("\t\t       [-p <tcpsessions>]\n");
    printf("\t\t       [-n <bestpops>] [-x <tsecs>]\n");
    printf("\t\t       [-b] [-d] [-h]\n\n\n");
    return; 
}

void print_input(){
    printf("StreamID: %s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
    printf("Self IP, TCP and UDP ports: %s - %s - %s\n", input.ipaddr, input.tport, input.uport);
    printf("Root Server: %s:%s\n", input.rs_id.ip, input.rs_id.port);
    printf("Application variables: %d (sessions) - %d (bestp) - %d (tsecs)\n", input.tcpsessions, input.bestpops, input.tsecs);
    printf("Flags: %d %d %d\n", input.display, input.debug, input.help);
    return;
}