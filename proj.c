#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


enum flag {on, off};

struct stream{
    char name[63];
    char ip[63];
    char port[63];
};

struct rs{
    char adress[63];
    char port[63];
};
 
struct input{
    struct stream stream_id;
    char ipaddr[63];
    char tport[63];
    char uport[63];
    struct rs rs_id;
    char tcpsessions[63];
    char bestpops[63];
    char tsecs[63];
    enum flag display;
    enum flag advanced;
    enum flag help;
};

int main ( int argc, char *argv[] )
{
    struct input input;
    char *token;
    int cont = 4;

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

    // IPADDR
    strcpy(input.ipaddr,argv[3]);
    printf("%s\n",input.ipaddr);

    // DEFAULT
    strcpy(input.tport,"58000");
    strcpy(input.uport,"58000");
    strcpy(input.rs_id.adress,"193.136.138.142");
    strcpy(input.rs_id.port,"59000");
    strcpy(input.tcpsessions,"1");
    strcpy(input.bestpops,"1");
    strcpy(input.tsecs,"5");

    // MORE ARGUMENTS ?
    while (cont < argc )
    {
        if (strcmp(argv[cont],"-t") == 0)
        {
            strcpy(input.tport,argv[cont + 1]);
            printf("%s\n",input.tport); 
        }

        if (strcmp(argv[cont],"-u") == 0)
        {
            strcpy(input.uport,argv[cont + 1]);
            printf("%s\n",input.uport); 
        }

        if (strcmp(argv[cont],"-s") == 0)
        {
            token = strtok(argv[cont+1],":");
            strcpy(input.rs_id.adress,token);
            printf("%s\n",input.rs_id.adress);

            token = strtok(NULL," ");
            strcpy(input.rs_id.port,token);
            printf("%s\n",input.rs_id.port);
        }

        if (strcmp(argv[cont],"-p") == 0)
        {
            strcpy(input.tcpsessions,argv[cont +1]);
            printf("%s\n",input.tcpsessions);
        }

        if (strcmp(argv[cont],"-n") == 0)
        {
            strcpy(input.bestpops,argv[cont +1]);
            printf("%s\n",input.bestpops);
        }

        if (strcmp(argv[cont],"-x") == 0)
        {
            strcpy(input.tsecs,argv[cont +1]);
            printf("%s\n",input.tsecs);
        }
        
        if (strcmp(argv[cont],"-b") == 0)
        {
           /* To be continued */
        }

        if (strcmp(argv[cont],"-d") == 0)
        {
           /* To be continued */
        }

        if (strcmp(argv[cont],"-h") == 0)
        {
           /* To be continued */
        }

        cont++;
    } 

 return 0;
}