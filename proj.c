#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "struct.h"

int main ( int argc, char *argv[] )
{
    struct input input;
    char *token;

    system("clear");
    //inputHandler();
    
    // DEFAULT DONE
    strcpy(input.tport,"58000");
    strcpy(input.uport,"58000");
    strcpy(input.rs_id.adress,"193.136.138.142");
    strcpy(input.rs_id.port,"59000");
    strcpy(input.tcpsessions,"1");
    strcpy(input.bestpops,"1");
    strcpy(input.tsecs,"5");
    input.display = on;
    input.advanced = off;
    input.help = off;


    // LOOKS FOR FLAGS
    for (int i = 1; i < argc; i++){
        if (strcmp(argv[i],"-i") == 0){  
            // IPADDR
            strcpy(input.ipaddr,argv[i+1]);
            printf("%s\n",input.ipaddr);
        }

        if (strcmp(argv[i],"-t") == 0){
            strcpy(input.tport,argv[i + 1]);
            printf("%s\n",input.tport); 
        }

        if (strcmp(argv[i],"-u") == 0){
            strcpy(input.uport,argv[i + 1]);
            printf("%s\n",input.uport); 
        }

        if (strcmp(argv[i],"-s") == 0){
            token = strtok(argv[i+1],":");
            strcpy(input.rs_id.adress,token);
            printf("%s\n",input.rs_id.adress);

            token = strtok(NULL," ");
            strcpy(input.rs_id.port,token);
            printf("%s\n",input.rs_id.port);
        }

        if (strcmp(argv[i],"-p") == 0){
            strcpy(input.tcpsessions,argv[i +1]);
            printf("%s\n",input.tcpsessions);
        }

        if (strcmp(argv[i],"-n") == 0){
            strcpy(input.bestpops,argv[i +1]);
            printf("%s\n",input.bestpops);
        }

        if (strcmp(argv[i],"-x") == 0){
            strcpy(input.tsecs,argv[i +1]);
            printf("%s\n",input.tsecs);
        }
        
        if (strcmp(argv[i],"-b") == 0){
           input.display = off;
        }

        if (strcmp(argv[i],"-d") == 0){
           input.advanced = on;
        }

        if (strcmp(argv[i],"-h") == 0){
            input.help = on;
           
        }
    } 


    if (input.help == on) exit(0);

    // STREAM NAME
    token = strtok(argv[1],":");
    strcpy(input.stream_id.name,token);
    printf("%s\n",input.stream_id.name);

    // STREAM IP
    token = strtok(NULL,":");
    strcpy(input.stream_id.ip,token);
    printf("%s\n",input.stream_id.ip);

    // STREAM PORT
    token = strtok(NULL," ");
    strcpy(input.stream_id.port,token);
    printf("%s\n",input.stream_id.port);

    printf("saiu bem\n");
    return 0;
}