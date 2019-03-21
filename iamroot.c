#include "defs.h"

int main (int argc, char **argv)
{
    char buffer[BUFFER_SIZE], downlink_buffer[BUFFER_SIZE], *token;
    memset(buffer, '\0', BUFFER_SIZE);memset(downlink_buffer, '\0', BUFFER_SIZE);
    struct message message, downlink_message;
    int ctcp_fd, sudp_fd, stcp_fd;
    
    //takes care of program input args or sets to default if not specified
    inputHandler(argv, argc);

    //allocs client struct vector to the number of tcpsessions supported by node
    new_fds = (struct client*)calloc(input.tcpsessions, sizeof(struct client));
    for (int i = 0; i < input.tcpsessions; i++) new_fds[i].fd =-1;

    //allocs pop struct vector to the number of bestpops
    pop = (struct pop*)calloc(input.bestpops, sizeof(struct pop));
    for (int i = 0; i < input.bestpops; i++) pop[i].key =-1;

    udp_encoder("WHOISROOT", buffer, (struct ipport *)NULL); //builds WHOISROOT protocol message
    udp_client(0, buffer, input.rs_id); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    
    if (node.udp.URROOT){node.udp.URROOT = false;is_root=true;//APLICATION IS ROOT
  
        //adds pop to pop array
        strcpy(pop[0].ipport.ip, input.ipaddr);
        strcpy(pop[0].ipport.port, input.tport);
        pop[0].key = 0;

    }else if (node.udp.ROOTIS){node.udp.ROOTIS = false;is_root=false;//APLICATION IS NOT ROOT

        //build POPREQ message
        memset(buffer, '\0', strlen(buffer));
        strcpy(buffer, "POPREQ\n"); //printf("Sending: %s to %s:%s\n",buffer, message.address.adress, message.address.port);
        udp_client(0, buffer, message.address);//send it

        //decode POPRESP received message
        udp_decoder(buffer, &message); 

    }else{printf("Unexpected error - iamroot\n");exit(EXIT_FAILURE);}

    //connects to stream source
    ctcp_fd = tcp_client(message.address); //printf("Connected to stream source on socket: %d\n", ctcp_fd);

    //initializes udp access server
    sudp_fd = udp_server(); //printf("udp access server created on socket %d\n", sudp_fd);

    //initializes tcp downlink server
    stcp_fd = tcp_server(); //printf("tcp root downlink server created on socket %d\n", stcp_fd);

    //when the root leaves correctly, if the node that gets first to the root server
    //and becomes the new root, and only has room for 1 connection, but gets several
    //connection attempts, redirect doesnt work because the second connection attempt
    //arrives before the first connection's NP and therefor new_fds ipport field is 
    //still empty. quick fix: fill index 0 ipport with own ip and port
    strcpy(new_fds[0].ipport.ip, input.ipaddr);
    strcpy(new_fds[0].ipport.port, input.tport);

    fd_set rfds;
    int counter, n, addrlen, newfd=-1, maxfd, clients = 0, pop_tracker = 0;
    unsigned short query_num = -1;
    struct sockaddr_in addr;
    struct timeval timeout;

    time_t start = time(NULL);

    while(1){

        //BUILDS READ SET
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDIN_FILENO,&rfds);
        for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd!=-1) FD_SET(new_fds[i].fd, &rfds);

        //Clears buffers for the next cicle
        memset(buffer, '\0', BUFFER_SIZE);memset(downlink_buffer, '\0', BUFFER_SIZE);
        memset(&message, '\0', sizeof message);
        memset(&message, '\0', sizeof downlink_message);

        timeout.tv_sec = 2;
        timeout.tv_usec = 0;

        //GETS THE BIGGEST FD 
        if(!(maxfd=Array_Max(new_fds))) maxfd = stcp_fd;

        counter=select(maxfd+1,&rfds,(fd_set*)NULL,(fd_set*)NULL, &timeout);
        if(counter<0){perror("select()"); exit(1);}
        else if(counter == 0) printf("hmm ye\n");


        /*
        *
        * START OF READ SET CHECKS
        * 
        */

        //checks for uplink connection packets ------ TCP 
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,buffer,BUFFER_SIZE))!=0){if(n==-1){ perror("uplink - read()");exit(1);}
                if(is_root){if(input.debug)printf("Detected traffic from stream source\n");
                    ptp_encoder("DA", buffer, n);
                    send_downstream(&clients, buffer); 
                    input.SF = true;node.ptp.DA = true;
                }else ptp_decoder(buffer, &message, 0);
            }else{if(input.debug)printf("Uplink connection failure\n");
                close(ctcp_fd);ctcp_fd = -1;
                send_downstream(&clients, "BS\n");
                udp_encoder("WHOISROOT", buffer, (struct ipport *)NULL); //builds WHOISROOT protocol message
                udp_client(0, buffer, input.rs_id); //sends the built message
                udp_decoder(buffer, &message); //decodes received message
            }
        }

        //checks for downlink connection attempts ------- TCP
        else if(FD_ISSET(stcp_fd, &rfds)){
            addrlen=sizeof(addr);
            if((newfd=accept(stcp_fd,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){perror("downlink - accept()");exit(1);}

            if (clients < input.tcpsessions){if(input.debug)printf("Connection established on socket: %d\n", newfd);
                Array_Add(new_fds,newfd);  
                ptp_encoder("WE", buffer, 0);
                write(newfd, buffer, strlen(buffer));if(input.debug)printf("Sending Welcome message\n");
                clients++;}
            else{if(input.debug)printf("Cant accept more clients, redirecting...\n");
                ptp_encoder("RE", buffer, 0);
                write(newfd, buffer, strlen(buffer));
                close(newfd);}           
        }

        //check for udp access server requests -------- UDP
        else if(FD_ISSET(sudp_fd,&rfds)){printf("Detected traffic to upd access server\n");
            addrlen=sizeof(addr);
            if((n=recvfrom(sudp_fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){perror("recvfrom access server");exit(1);};      
            udp_decoder(buffer, &message);
        }

        //checks for user input ---- STDIN
        else if(FD_ISSET(STDIN_FILENO,&rfds)){
            fgets(buffer, BUFFER_SIZE, stdin);  
            user_decoder(buffer);
        }

        //check for downlink connection packets ----- TCP
        for (int i = 0; i < input.tcpsessions; i++)
            if (FD_ISSET(new_fds[i].fd, &rfds)){
                if((n=read(new_fds[i].fd,downlink_buffer,BUFFER_SIZE))!=0){
                    if(n==-1){perror("downlink - read()");exit(1);}
                    ptp_decoder(downlink_buffer, &downlink_message, new_fds[i].fd);
                }else{printf("Detected a connection failure, closing...\n");
                    close(new_fds[i].fd); 
                    clients--;
                    Array_Rem(new_fds, new_fds[i].fd);
                }
                break;
            }
        /*
        *
        * END OF THE READ SET CHECKS
        * 
        */

        /*
        *
        * START OF FLAG CHECKING. FLAG==TRUE MEANS THE MESSAGE ASSOCIATED WITH THE FLAG NAME WAS RECEIVED
        * 
        */ 

        /*
        *   PTP related flags
        * 
        */ 
        if (node.ptp.WE){node.ptp.WE = false;
            if(input.debug)printf("WE detected\n");
            ptp_encoder("NP", buffer, 0);
            write(ctcp_fd, buffer, strlen(buffer));
            //CHECKS TO SEE IF STREAM IS THE DESIRED ONE DONE IN ptp_decoder
        }

        if (node.ptp.BS){node.ptp.BS = false;
            if(input.debug)printf("BS detected\n");
            input.SF = false;
            send_downstream(&clients, "BS\n");
        }

        if (node.ptp.DA){node.ptp.DA = false;
            if(input.debug)printf("DA detected\n");
            token = &buffer[0]; token += 8; 
            if(input.display){ 
                if(input.format) printf("%s\n", token);
                else print_hex(token);}
            send_downstream(&clients, buffer);
        }

        if (node.ptp.NP){node.ptp.NP = false;
            if(input.debug)printf("NP detected\n");
            //DONE IN ptp_decoder FOR NOW
        }

        if (node.ptp.PQ){node.ptp.PQ = false;
            if(input.debug)printf("PQ detected --> %s\n", buffer);
            printf("hmm -> %d - %d\n", message.keys[0], message.keys[1]);
            //setsockopt
            //TODO
        }

        if (node.ptp.PR){node.ptp.PR = false;
            //TODO
        }

        if (node.ptp.RE){node.ptp.RE = false;
            if(input.debug)printf("RE detected\n%s", buffer);
            if(input.debug)printf("Closing ctcp: %d\n", ctcp_fd);
            close(ctcp_fd);
            ctcp_fd = tcp_client(message.address);
        }

        if (node.ptp.SF){node.ptp.SF = false;
            if(input.debug)printf("SF detected\n");
            input.SF = true;
            send_downstream(&clients, "SF\n");
        }

        if (node.ptp.TQ){node.ptp.TQ = false;
            //TODO
        }

        if (node.ptp.TR){node.ptp.TR = false;
            //TODO
        }




        /*
        *   user related flags
        * 
        */ 
        if (node.user.debug){node.user.debug = false;
             //DONE IN user_decode FOR NOW
        }

        if (node.user.display){node.user.display = false;
             //DONE IN user_decode FOR NOW
        }

        if (node.user.exit_){node.user.exit_ = false; 
            if(is_root){
                udp_encoder("REMOVE", buffer, (struct ipport *)NULL);
                udp_client(1, buffer, input.rs_id);
            }
            close(ctcp_fd);close(stcp_fd);close(sudp_fd);
            for (int i = 0; i < input.tcpsessions; i++) if (new_fds[i].fd != -1) close(new_fds[i].fd);
            free(new_fds);free(pop);
            printf("Exiting...\n");
            exit(0); 
        }

        if (node.user.format){node.user.format = false;
            //DONE IN user_decode FOR NOW
        }

        if (node.user.status){node.user.status = false;
            print_status(clients);
        }

        if (node.user.streams){node.user.streams = false;
            strcpy(buffer, "DUMP\n");
            udp_client(0, buffer, input.rs_id);
            udp_decoder(buffer, &message);
        }       

        if (node.user.tree){node.user.tree = false;
            //TODO
        }




        /*
        *   UDP related flags
        * 
        */ 
        if (node.udp.ERROR){
            //TODO
        }

        if (node.udp.POPREQ){node.udp.POPREQ = false; 
            printf("POPREQ detected\n");
            memset(buffer, '\0', strlen(buffer));
            udp_encoder("POPRESP", buffer, &(pop[pop_tracker].ipport));
            if((n=sendto(sudp_fd,buffer,strlen(buffer),0,(struct sockaddr*)&addr,addrlen))==-1){perror("udp_popreq - sendto\n");exit(1);}
            pop[pop_tracker].key++;//increments counter everytime a pop address is sent on a POPRESP

            //LOGIC TO KNOW WHEN TO CALL A POP QUERY
            if (pop[pop_tracker].key == 3){//arbitrary decision to send a POP QUERY if a pop has been used 3 times
                if(input.bestpops + clients - input.tcpsessions > 0){
                    memset(buffer, '\0', strlen(buffer));
                    sprintf(buffer, "%04X", ++query_num);
                    ptp_encoder("PQ", buffer, input.bestpops + clients - input.tcpsessions);
                    printf("buffer - %s\n", buffer);
                    send_downstream(&clients, buffer);}
            }

            if (pop_tracker+1 >= input.bestpops || pop[pop_tracker+1].key == -1) pop_tracker = 0; 
            else pop_tracker++;
        }

        if (node.udp.POPRESP){
            //TODO
        }

        if (node.udp.ROOTIS){node.udp.ROOTIS = false;
            strcpy(buffer, "POPREQ\n"); 
            udp_client(0, buffer, message.address);
            udp_decoder(buffer, &message); 
            ctcp_fd = tcp_client(message.address);if(input.debug)printf("Connected to point of presence\n");
        }

        if (node.udp.URROOT){node.udp.URROOT = false;is_root = true;
            ctcp_fd = tcp_client(message.address); 
            strcpy(pop[0].ipport.ip, input.ipaddr);
            strcpy(pop[0].ipport.port, input.tport);
            pop[0].key = 0;
            send_downstream(&clients, "SF\n");
        }

        /*
        *
        * END OF FLAG CHECKING
        * 
        */

       //TIMER FOR ROOT SERVER UPDATE
        if(is_root)
            if (time(NULL) - start >= input.tsecs){
                start = time(NULL);
                if(input.debug)printf("Elapsed 5 seconds\n");
                udp_encoder("WHOISROOT", buffer, (struct ipport *)NULL); //builds WHOISROOT protocol message
                udp_client(0, buffer, input.rs_id);
            }
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
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd == fd){vector[i].fd = -1; 
                                                                       memset(&vector[i].ipport, '\0', sizeof(struct ipport));
                                                                       return 0;}
    return -1;
}

int Array_Addipport (struct client *vector, int fd, struct ipport ipport){
    for(int i = 0; i < input.tcpsessions; i++)  
        if (vector[i].fd == fd){
            strcpy(vector[i].ipport.ip, ipport.ip);
            strcpy(vector[i].ipport.port, ipport.port);
            return 0;}
    return -1;
}

void send_downstream (int * clients, char * buffer){
    for (int i = 0; i < input.tcpsessions; i++)
        if (new_fds[i].fd != -1)
            if (write(new_fds[i].fd, buffer, strlen(buffer)) == -1){
                printf("Detected a connection failure, closing...\n");
                close(new_fds[i].fd); clients--;
                Array_Rem(new_fds, new_fds[i].fd);}
}

void print_status (int clients){
    printf("Connected to stream: %s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
    if (input.SF) printf("Stream is good\n");
    else printf("Stream broken\n");
    if (is_root){printf("I am root!\n");
        printf("UDP access server on: %s:%s\n", input.ipaddr, input.uport);}
    else{printf("I am groot!\n");printf("Uplink -- TODO\n");}
    printf("Access point on: %s:%s\n", input.ipaddr, input.tport);
    printf("Supported sessions: %d - Occupied: %d\n", input.tcpsessions, clients);
    printf("Connected pairs:\n");
    for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd != -1) printf("%s:%s\n", new_fds[i].ipport.ip, new_fds[i].ipport.port);
}

void print_hex(const char *s){
    while(*s) printf("%02X ", (unsigned int) *s++);
    printf("\n\n");
}