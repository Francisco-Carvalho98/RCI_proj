#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[80];
    struct message message; 
    int ctcp_fd; //tcp client socket

    inputHandler(argv, argc);
    udp_encoder("WHOISROOT", buffer); //builds WHOISROOT protocol message
    udp_client(0, buffer); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    if (strcmp(message.command, "URROOT") == 0){//node is root
        ctcp_fd = tcp_client(0); //connects to stream source
        //TODO 
        //initialize udp access server
        //initialize tcp stream server
    }else if (strcmp(message.command, "ROOTIS") == 0){//node isnt root
        strcpy(buffer, "POPREQ\n");//build message
        udp_client(0, buffer);//send it
        udp_decoder(buffer, &message);//decode received message

    }else exit(EXIT_FAILURE);

    fd_set rfds;
    int counter, n;
    while(1){
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(STDERR_FILENO,&rfds);

        counter=select(ctcp_fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}


        //checks for source stream
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,buffer,78))!=0){
                if(n==-1){ perror("read()");exit(1);}
                write(1, "Server: ", strlen("Server: "));
                write(1, buffer, n);printf("\n");}}

        //checks for user input
        if(FD_ISSET(STDERR_FILENO,&rfds)){
            fgets(buffer, 74, stdin);
            write(1, "User: ", 6);
            write(1, buffer, 74);}

    }
    return 0;
}

