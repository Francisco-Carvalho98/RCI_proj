#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

struct stream{
    char name[63];
    char ip[63];
    char port[63];
} stream;

struct rs{
    char adress[63];
    char port[63];
}rs;

struct input{
    struct stream stream_id;
    char ipaddr[63];
    char tport[63];
    char uport[63];
    struct rs rs_id;
    char tcpsessions[63];
    char bestpops[63];
    char tsecs[63];
}input;

int main ( int argc, char *argv[] )
{
    struct input;
    char buffer[256];
    char *token;

    write(1,"iamroot\t",8);
    scanf("%[^\n]", buffer);

    token = strtok(buffer,":");
    strcpy(input.stream_id.name,token);
    printf("%s\n",input.stream_id.name);

    token = strtok(NULL,":");
    strcpy(input.stream_id.ip,token);
    printf("%s\n",input.stream_id.ip);

    token = strtok(NULL," ");
    strcpy(input.stream_id.port,token);
    printf("%s\n",input.stream_id.port);
    
    token = strtok(NULL," ");
    token = strtok(NULL," ");
    strcpy(input.ipaddr,token);
    printf("%s\n",input.ipaddr); 

    strcpy(input.tport,"58000");
    strcpy(input.uport,"58000");
    strcpy(input.rs_id.adress,"193.136.138.142");
    strcpy(input.rs_id.port,"59000");
    strcpy(input.tcpsessions,"1");
    strcpy(input.bestpops,"1");
    strcpy(input.tsecs,"5");

    token = strtok(NULL," ");

    while (token != NULL)
    {

        if (strcmp(token,"-t") == 0)
        {
            token = strtok(NULL," ");
            strcpy(input.tport,token);
            printf("%s\n",input.tport); 
        }

        if (strcmp(token,"-u") == 0)
        {
            token = strtok(NULL," ");
            strcpy(input.uport,token);
            printf("%s\n",input.uport); 
        }

        if (strcmp(token,"-s") == 0)
        {
            token = strtok(NULL,":");
            strcpy(input.rs_id.adress,token);
            printf("%s\n",input.rs_id.adress);

            token = strtok(NULL," ");
            strcpy(input.rs_id.port,token);
            printf("%s\n",input.rs_id.port);
        }

        if (strcmp(token,"-p") == 0)
        {
            token = strtok(NULL," ");
            strcpy(input.tcpsessions,token);
            printf("%s\n",input.tcpsessions);
        }

        if (strcmp(token,"-n") == 0)
        {
            token = strtok(NULL," ");
            strcpy(input.bestpops,token);
            printf("%s\n",input.bestpops);
        }

        if (strcmp(token,"-x") == 0)
        {
            token = strtok(NULL," ");
            strcpy(input.tsecs,token);
            printf("%s\n",input.tsecs);
        }
        
        if (strcmp(token,"-b") == 0)
        {
           /* To be continued */
        }

        if (strcmp(token,"-d") == 0)
        {
           /* To be continued */
        }

        if (strcmp(token,"-h") == 0)
        {
           /* To be continued */
        }

        token= strtok(NULL," ");
    }

}