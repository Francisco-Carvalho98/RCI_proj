#include "defs.h"

struct input input;

int main (int argc, char **argv)
{

    //system("clear");
    char buffer[80];
    struct message message; 
    int ctcp_fd, sudp_fd, stcp_fd; //tcp client socket
    enum flag is_root;

    inputHandler(argv, argc);
    udp_encoder("WHOISROOT", buffer); //builds WHOISROOT protocol message
    udp_client(0, buffer, input.rs_id); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    
    if (strcmp(message.command, "URROOT") == 0){is_root=true;//APLICATION IS ROOT

        //connects to stream source
        ctcp_fd = tcp_client(message.address); //printf("Connected to stream source on socket: %d\n", ctcp_fd);

        //initializes udp access server
        sudp_fd = udp_server(); //printf("udp access server created on socket %d\n", sudp_fd);

        //initializes tcp downlink server
        stcp_fd = tcp_server(); //printf("tcp root downlink server created on socket %d\n", stcp_fd);


    }else if (strcmp(message.command, "ROOTIS") == 0){is_root=false;//APLICATION IS NOT ROOT

        //build POPREQ message
        strcpy(buffer, "POPREQ\n"); //printf("Sending: %s to %s:%s\n",buffer, message.address.adress, message.address.port);
        //send it
        udp_client(0, buffer, message.address);
        //decode POPRESP received message
        udp_decoder(buffer, &message); printf("Message decoded: %s %s\n", message.address.ip, message.address.ip);//exit(0);
        //connects to tree entry point
        ctcp_fd = tcp_client(message.address); printf("Connected to tree entry point\n");
        stcp_fd = tcp_server(); printf("tcp node downlink server created on socket %d\n", stcp_fd);
        sudp_fd = -1;

    }else exit(EXIT_FAILURE);

    fd_set rfds;
    int counter, n, addrlen;
    struct sockaddr_in addr;

    while(1){
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDERR_FILENO,&rfds);

        counter=select(6,&rfds,(fd_set*)NULL,(fd_set*)NULL,(struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}

        //checks for source stream
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,buffer,78))!=0){
                if(n==-1){ perror("read()");exit(1);}
                if(is_root)printf("Detected traffic from stream source\n");
                else printf("Detected traffic from uplink connection\n");
                /*write(1, "Server: ", strlen("Server: "));
                write(1, buffer, n);printf("\n");*/}}
        
        if(FD_ISSET(stcp_fd, &rfds)){
            
        }

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

