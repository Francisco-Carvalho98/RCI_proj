#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[80];
    struct message message; 
    int ctcp_fd, sudp_fd, stcp_fd; //tcp client socket

    inputHandler(argv, argc);
    udp_encoder("WHOISROOT", buffer); //builds WHOISROOT protocol message
    udp_client(0, buffer, input.rs_id); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    
    if (strcmp(message.command, "URROOT") == 0){//APLICATION IS ROOT

        //connects to stream source
        ctcp_fd = tcp_client(0, message.address);printf("Connected to stream source on socket: %d\n", ctcp_fd);

        //initializes udp access server
        sudp_fd = udp_server();printf("udp access server created on socket %d\n", sudp_fd);

        //initializes tcp downlink server
        stcp_fd = tcp_server();printf("tcp downlink server created on socket %d\n", stcp_fd);


    }else if (strcmp(message.command, "ROOTIS") == 0){//APLICATION IS NOT ROOT
        strcpy(buffer, "POPREQ\n");//build message
        printf("Sending: %s to %s:%s\n",buffer, message.address.adress, message.address.port);
        udp_client(0, buffer, message.address);//send it
        udp_decoder(buffer, &message);//decode received message
        printf("Message decoded\n");exit(0);

    }else exit(EXIT_FAILURE);

    fd_set rfds;
    int counter, n, addrlen;
    struct sockaddr_in addr;

    while(1){
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(STDERR_FILENO,&rfds);

        counter=select(sudp_fd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}


        //checks for source stream
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,buffer,78))!=0){
                if(n==-1){ perror("read()");exit(1);}
                printf("Detected traffic from stream source\n");
                /*write(1, "Server: ", strlen("Server: "));
                write(1, buffer, n);printf("\n");*/}}

        //checks for user input
        if(FD_ISSET(STDERR_FILENO,&rfds)){
            fgets(buffer, 74, stdin);
            write(1, "User: ", 6);
            write(1, buffer, 74);}

        //check for udp access server requests
        if(FD_ISSET(sudp_fd,&rfds)){
            printf("Detected traffic to upd access server\n");
            addrlen=sizeof(addr);
            n=recvfrom(sudp_fd,buffer,128,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen);
            if(n==-1)/*error*/exit(1);
            
            n=sendto(sudp_fd,buffer,n,0,(struct sockaddr*)&addr,addrlen);
            if(n==-1)/*error*/exit(1);
        }
    }
    return 0;
}

