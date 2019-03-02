#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[80];
    struct message message;
    int fd;

    inputHandler(argv, argc);
    udp_encoder("WHOISROOT", buffer);
    udp_client(0, buffer);
    udp_decoder(buffer, &message);
    if (strcmp(message.command, "URROOT") == 0) fd = tcp_client(0);
    else fd = tcp_client(1);

    //printf("iamroot> ");

    fd_set rfds;
    int counter, n;
    while(1){
        FD_ZERO(&rfds);
        FD_SET(fd,&rfds);
        FD_SET(STDERR_FILENO,&rfds);

        counter=select(fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}

        if(FD_ISSET(fd,&rfds)){
            if((n=read(fd,buffer,78))!=0){
                if(n==-1){ perror("read()");exit(1);}
                write(1, "Server: ", strlen("Server: "));
                write(1, buffer, n);printf("\n");}}

        if(FD_ISSET(STDERR_FILENO,&rfds)){
            fgets(buffer, 74, stdin);
            write(1, "User: ", 6);
            write(1, buffer, 74);}

    }
    return 0;
}

