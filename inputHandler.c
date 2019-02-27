#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defs.h"

void inputHandler (struct input *input, char **argv, int argc){
    
    char *token;


    if (argc == 1) {printf("Not enough arguments");exit(0);}

    // DEFAULT DONE
    strcpy(input->tport,"58000");
    strcpy(input->uport,"58000");
    strcpy(input->rs_id.adress,"193.136.138.142");
    strcpy(input->rs_id.port,"59000");
    strcpy(input->tcpsessions,"1");
    strcpy(input->bestpops,"1");
    strcpy(input->tsecs,"5");
    input->display = on;
    input->advanced = off;
    input->help = off;


    // LOOKS FOR FLAGS
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],"-i") == 0) strcpy(input->ipaddr,argv[i+1]);   
        if (strcmp(argv[i],"-t") == 0) strcpy(input->tport,argv[i+1]); 
        if (strcmp(argv[i],"-u") == 0) strcpy(input->uport,argv[i+1]); 
        if (strcmp(argv[i],"-s") == 0){
            token = strtok(argv[i+1],":");
            strcpy(input->rs_id.adress,token);
            token = strtok(NULL," ");
            strcpy(input->rs_id.port,token);}
        if (strcmp(argv[i],"-p") == 0) strcpy(input->tcpsessions,argv[i+1]);
        if (strcmp(argv[i],"-n") == 0) strcpy(input->bestpops,argv[i+1]);
        if (strcmp(argv[i],"-x") == 0) strcpy(input->tsecs,argv[i+1]);
        if (strcmp(argv[i],"-b") == 0) input->display = off;
        if (strcmp(argv[i],"-d") == 0) input->advanced = on;
        if (strcmp(argv[i],"-h") == 0) input->help = on;
    } 


    if (input->help == on) exit(0);

    // STREAM NAME
    token = strtok(argv[1],":");
    strcpy(input->stream_id.name,token);

    // STREAM IP
    token = strtok(NULL,":");
    strcpy(input->stream_id.ip,token);

    // STREAM PORT
    token = strtok(NULL," ");
    strcpy(input->stream_id.port,token);


    printf("StreamID: %s:%s:%s\n", input->stream_id.name, input->stream_id.ip, input->stream_id.port);
    printf("Self IP, TCP and UDP ports: %s - %s - %s\n", input->ipaddr, input->tport, input->uport);
    printf("Root Server: %s:%s\n", input->rs_id.adress, input->rs_id.port);
    printf("Application variables: %s (sessions) - %s (bestp) - %s (tsecs)\n", input->tcpsessions, input->bestpops, input->tsecs);
    printf("Flags: %d %d %d\n", input->display, input->advanced, input->help);
    
    return;
}

void display_help (){
    return;
}