#include "defs.h"



int main (int argc, char **argv)
{
    char buffer[80];
    memset(&pop, 0, sizeof pop);
    struct message message; 
    int ctcp_fd, sudp_fd, stcp_fd; //tcp client socket
    bool is_root;
    int *new_fds;
    

    inputHandler(argv, argc);//print_input();
    new_fds = (int*)malloc(input.tcpsessions*sizeof(int));
    memset(new_fds, -1, input.tcpsessions*sizeof(int));

    udp_encoder("WHOISROOT", buffer, (struct ipport *)NULL); //builds WHOISROOT protocol message
    udp_client(0, buffer, input.rs_id); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    
    if (node.udp.URROOT){node.udp.ROOTIS = false;is_root=true;//APLICATION IS ROOT

        //connects to stream source
        ctcp_fd = tcp_client(message.address); //printf("Connected to stream source on socket: %d\n", ctcp_fd);

        //initializes udp access server
        sudp_fd = udp_server(); //printf("udp access server created on socket %d\n", sudp_fd);

        //initializes tcp downlink server
        stcp_fd = tcp_server(); //printf("tcp root downlink server created on socket %d\n", stcp_fd);

        strcpy(pop[0].ipport.ip, input.ipaddr);
        strcpy(pop[0].ipport.port, input.tport);


    }else if (node.udp.ROOTIS){node.udp.ROOTIS = false;is_root=false;//APLICATION IS NOT ROOT

        //build POPREQ message
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer, "POPREQ\n"); //printf("Sending: %s to %s:%s\n",buffer, message.address.adress, message.address.port);

        //send it
        udp_client(0, buffer, message.address);

        //memset(&message, '\0', sizeof message);
        //decode POPRESP received message
        udp_decoder(buffer, &message); printf("Message decoded: %s %s\n", message.address.ip, message.address.port);//exit(0);
        
        //connects to tree entry point
        ctcp_fd = tcp_client(message.address); printf("Connected to tree entry point\n");
        stcp_fd = tcp_server(); printf("tcp node downlink server created on socket %d\n", stcp_fd);
        sudp_fd = -1;

    }else{printf("Unexpected error - iamroot\n");exit(EXIT_FAILURE);}

    fd_set rfds;
    int counter, n, addrlen, newfd=-1, maxfd;
    struct sockaddr_in addr;
    //struct timeval timeout;
    //time_t start = clock();
    

    while(1){

        //BUILDS READ SET
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDIN_FILENO,&rfds);
        for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i]!=-1) FD_SET(new_fds[i], &rfds);


        memset(buffer, '\0', strlen(buffer));
        memset(&message, '\0', sizeof message);

        //GETS THE BIGGEST FD 
        if(!(maxfd=Array_Max(new_fds, input.tcpsessions))) maxfd = stcp_fd; 
        printf("maxfd - %d\n", maxfd);

        counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL, (struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}

        //checks for uplink connection packets ------ TCP 
        if(FD_ISSET(ctcp_fd,&rfds) ){
            if((n=read(ctcp_fd,buffer,128))!=0){
                if(n==-1){ perror("read()");exit(1);}
                if(is_root){printf("Detected traffic from stream source\n");
                    tcp_encoder("DA", buffer, n);
                }else{
                    printf("Detected traffic from uplink connection\n");
                    write(1, "Redirected:\n", strlen("Redirected:\n"));
                    write(1, buffer, n);
                    printf("\n");
                }
                //sends stream downstream
                for (int i = 0; i < input.tcpsessions; i++)
                    if (new_fds[i] != -1)
                        if (write(new_fds[i], buffer, n) == -1){
                            close(new_fds[i]); 
                            Array_Rem(new_fds, input.tcpsessions, new_fds[i]);}}}
        
        //checks for downlink connection attempts ------- TCP
        if(FD_ISSET(stcp_fd, &rfds) ){
            addrlen=sizeof(addr);
            if((newfd=accept(stcp_fd,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){perror("accept()");exit(1);}
            Array_Add(new_fds, input.tcpsessions, newfd);
            printf("Connection established on socket: %d\n", newfd);
        }

        //check for downlink connection packets ----- TCP
        for (int i = 0; i < input.tcpsessions; i++)
            if (FD_ISSET(new_fds[i], &rfds) ){
                if((n=read(newfd,buffer,128))!=0){
                    if(n==-1){perror("read()");exit(1);}/* ... */
                    //write buffer in afd
                    write(1, "echo: ", 6);
                    write(1, buffer, n);}
            }

        //checks for user input ---- STDIN
        if(FD_ISSET(STDIN_FILENO,&rfds) ){
            fgets(buffer, 74, stdin);
            write(1, "User: ", 6);
            write(1, buffer, 74);}

        //check for udp access server requests -------- UDP
        if(FD_ISSET(sudp_fd,&rfds) ){

            printf("Detected traffic to upd access server\n");
            addrlen=sizeof(addr);
            n=recvfrom(sudp_fd,buffer,128,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen);
            if(n==-1)/*error*/exit(1);

            udp_decoder(buffer, &message);
            memset(buffer, '\0', strlen(buffer));
            if (node.udp.POPREQ){node.udp.POPREQ = false; 
                printf("POPREQ detected\n");
                udp_encoder("POPRESP", buffer, &(pop[0].ipport));
            }

            n=sendto(sudp_fd,buffer,strlen(buffer),0,(struct sockaddr*)&addr,addrlen);
            if(n==-1)/*error*/exit(1);
        }






    }
    return 0;
}

int Array_Max (int *vector, int size){
    int max = 0;
    for(int i = 0; i < size; i++) if (vector[i] > max) max = vector[i];
    return max;  
}

int Array_Add (int *vector, int size, int fd){
    for(int i = 0; i < size; i++) if (vector[i] == -1){vector[i] = fd; return 0;}
    return -1;
}

int Array_Rem (int *vector, int size, int fd){
    for(int i = 0; i < size; i++) if (vector[i] == fd){vector[i] = -1; return 0;}
    return -1;
}