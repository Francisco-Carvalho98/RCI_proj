#include "defs.h"



int main (int argc, char **argv)
{
    char buffer[BUFFER_SIZE];
    memset(&pop, 0, sizeof pop);
    struct message message; 
    int ctcp_fd, sudp_fd, stcp_fd; //tcp client socket
    bool is_root;
    

    inputHandler(argv, argc);//print_input();
    new_fds = (struct client*)calloc(input.tcpsessions, sizeof(struct client));
    for (int i = 0; i < input.tcpsessions; i++) new_fds[i].fd =-1;

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

        //adds point of presence to pop array
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
    int clients = 0;
    //struct timeval timeout;
    //time_t start = clock();
    

    while(1){

        //BUILDS READ SET
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDIN_FILENO,&rfds);
        for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd!=-1) FD_SET(new_fds[i].fd, &rfds);

        //Clears buffers for the next cicle
        memset(buffer, '\0', strlen(buffer));
        memset(&message, '\0', sizeof message);

        //GETS THE BIGGEST FD 
        if(!(maxfd=Array_Max(new_fds))) maxfd = stcp_fd; 

        counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL, (struct timeval *)NULL);
        if(counter<=0){perror("select()"); exit(1);}

        /*
        *
        * START OF THE READ SET CHECKS
        * 
        */

        //checks for uplink connection packets ------ TCP 
        if(FD_ISSET(ctcp_fd,&rfds) ){
            if((n=read(ctcp_fd,buffer,BUFFER_SIZE))!=0){
                if(n==-1){ perror("read()");exit(1);}
                if(is_root){printf("Detected traffic from stream source\n");
                    ptp_encoder("DA", buffer, n);
                }else{
                    printf("Detected traffic from uplink connection\n");
                    printf("%s\n", buffer);
                }
                //sends stream downstream
                for (int i = 0; i < input.tcpsessions; i++)
                    if (new_fds[i].fd != -1)
                        if (write(new_fds[i].fd, buffer, BUFFER_SIZE) == -1){
                            close(new_fds[i].fd); 
                            Array_Rem(new_fds, new_fds[i].fd);}}}
        
        //checks for downlink connection attempts ------- TCP
        if(FD_ISSET(stcp_fd, &rfds) ){
            addrlen=sizeof(addr);
            if((newfd=accept(stcp_fd,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){perror("accept()");exit(1);}

            if (clients < input.tcpsessions){
                node.ptp.WE = true;
                clients++;}
            else node.ptp.RE = true;     
        }

        //check for downlink connection packets ----- TCP
        for (int i = 0; i < input.tcpsessions; i++)
            if (FD_ISSET(new_fds[i].fd, &rfds) ){
                if((n=read(newfd,buffer,BUFFER_SIZE))!=0){
                    if(n==-1){perror("read()");exit(1);}/* ... */
                    //write buffer in afd
                    write(1, "echo: ", 6);
                    write(1, buffer, n);}
            }

        //checks for user input ---- STDIN
        if(FD_ISSET(STDIN_FILENO,&rfds) ){
            fgets(buffer, BUFFER_SIZE, stdin);  
            user_decoder(buffer);
        }

        //check for udp access server requests -------- UDP
        if(FD_ISSET(sudp_fd,&rfds) ){

            printf("Detected traffic to upd access server\n");
            addrlen=sizeof(addr);
            n=recvfrom(sudp_fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen);
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
        /*
        *
        * END OF THE READ SET CHECKS
        * 
        */

        /*
        *
        * START OF FLAG CHECKING
        * 
        */ 

        //PTP related flags handling
        if (node.ptp.WE){node.ptp.WE = false;
            Array_Add(new_fds,newfd);
            printf("Connection established on socket: %d\n", newfd);
            printf("Sending Welcome message\n");
        }

        if (node.ptp.BS){node.ptp.BS = false;
            //TODO
        }

        if (node.ptp.DA){node.ptp.DA = false;
            //TODO
        }

        if (node.ptp.NP){node.ptp.NP = false;
            //TODO
        }

        if (node.ptp.PQ){node.ptp.PQ = false;
            //TODO
        }

        if (node.ptp.PR){node.ptp.PR = false;
            //TODO
        }

        if (node.ptp.RE){node.ptp.RE = false;
            //TODO
        }
        if (node.ptp.SF){node.ptp.SF = false;
            //TODO
        }

        if (node.ptp.TQ){node.ptp.TQ = false;
            //TODO
        }   
        if (node.ptp.TR){node.ptp.TR = false;
            //TODO
        }

        //USER related flags handling
        if (node.user.debug){node.user.debug = false;
            //TODO
        }

        if (node.user.display){node.user.display = false;
            //TODO
        }

        if (node.user.exit_){node.user.exit_ = false;
            free(new_fds);
            close(ctcp_fd);
            exit(0);
        }

        if (node.user.format){node.user.format = false;
            //TODO
        }

        if (node.user.status){node.user.status = false;
            //TODO
        }

        if (node.user.streams){node.user.streams = false;
            strcpy(buffer, "DUMP\n");
            udp_client(0, buffer, input.rs_id);
            udp_decoder(buffer, &message);
        }       

        if (node.user.tree){node.user.tree = false;
            //TODO
        }


        //UDP related flags
        if (node.udp.DUMP){
            
        }  
        if (node.udp.ERROR){

        } 
        if (node.udp.POPREQ){

        }  
        if (node.udp.POPRESP){

        }  
        if (node.udp.REMOVE){

        }  
        if (node.udp.ROOTIS){

        }  
        if (node.udp.STREAMS){

        }  
        if (node.udp.URROOT){

        }  
        if (node.udp.WHOISROOT){

        }
        /*
        *
        * END OF FLAG CHECKING
        * 
        */


    }
    return 0;
}

int Array_Max (struct client *vector){
    int max = 0;
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd > max) max = vector[i].fd;
    return max;  
}

int Array_Add (struct client *vector, int fd){
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd == -1){vector[i].fd = fd; return 0;}
    return -1;
}

int Array_Rem (struct client *vector, int fd){
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd == fd){vector[i].fd = -1; return 0;}
    return -1;
}

int Array_Addipport (struct client *vector, int fd, char *ip, char *port){
    for(int i = 0; i < input.tcpsessions; i++)  
        if (vector[i].fd == fd){
            strcpy(vector[i].ipport.ip, ip);
            strcpy(vector[i].ipport.ip, port);
            return 0;}
    return -1;
}