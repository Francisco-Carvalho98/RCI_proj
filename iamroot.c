#include "defs.h"

int main (int argc, char **argv)
{
    char buffer[BUFFER_SIZE], D_buffer[BUFFER_SIZE],T_buffer[BUFFER_SIZE], *token;
    char pre_buffer[BUFFER_SIZE], pre_D_buffer[BUFFER_SIZE];
    memset(pre_buffer, 0, BUFFER_SIZE);memset(pre_D_buffer, 0, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);memset(D_buffer, 0, BUFFER_SIZE);memset(T_buffer, 0, BUFFER_SIZE);
    struct message message, D_message;
    memset(&message, '\0', sizeof message);memset(&D_message, '\0', sizeof message);
    int ctcp_fd=-1, sudp_fd=-1, stcp_fd=-1;
    
    //takes care of program input args or sets to default if not specified
    inputHandler(argv, argc);

    //allocs client struct vector to the number of tcpsessions supported by node
    if ((new_fds = (struct client*)calloc(input.tcpsessions, sizeof(struct client))) == NULL){printf("Couldnt allocate memory\n");exit(1);}
    for (int i = 0; i < input.tcpsessions; i++) new_fds[i].fd =-1;

    //allocs pop struct vector to the number of bestpops
    if ((pop = (struct pop*)calloc(input.bestpops, sizeof(struct pop))) == NULL){printf("Couldnt allocate memory\n");exit(1);}
    for (int i = 0; i < input.bestpops; i++) pop[i].key =-1;

    udp_encoder("WHOISROOT", buffer, NULL); //builds WHOISROOT protocol message
    udp_client(0, buffer, input.rs_id); //sends the built message
    udp_decoder(buffer, &message); //decodes received message
    
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
    int counter, n, addrlen, newfd=-1, maxfd=-1, clients = 0, bestpops = 0, yetToReadU = 0, yetToReadD = 0;
    unsigned short query_num = -1;
    struct sockaddr_in addr;
    struct timeval timeout;
    bool Pquery_active = false, Tquery_active = false;
    struct tree Tvec[64];int Tvec_C=0;
    memset(Tvec, 0, 64*sizeof(struct tree));
    time_t start = time(NULL), PQ_time = time(NULL), TQ_time = time(NULL);

    while(1){

        //BUILDS READ SET
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDIN_FILENO,&rfds);
        for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd!=-1) FD_SET(new_fds[i].fd, &rfds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        //GETS THE BIGGEST FD 
        if(!(maxfd=Array_Max(new_fds))) maxfd = ctcp_fd > sudp_fd ? ctcp_fd : sudp_fd;
        maxfd = maxfd > sudp_fd ? maxfd : sudp_fd;

        if (ctcp_fd != -1 && !yetToReadU && !yetToReadD) counter=select(maxfd+1,&rfds, NULL, NULL, &timeout);
        else FD_ZERO(&rfds);

        if(yetToReadU) {if(input.debug)printf("There was something left to read...\n");
            yetToReadU = checkForMany(pre_buffer, buffer);
            printf("yTrU -> %d\n", yetToReadU);
            ptp_decoder(buffer, &message, 0);
        }
        if(yetToReadD) {if(input.debug)printf("There was even more left to read...\n");
            yetToReadD = checkForMany(pre_D_buffer, D_buffer);
            printf("yTrD -> %d\n", yetToReadD);
            ptp_decoder(D_buffer, &D_message, 0);
        }
        
        if(counter<0){perror("select()"); exit(1);}
        else if(counter == 0){
            if(input.debug && ctcp_fd != -1)printf("select timeout\n");
        }

        /*
        *
        * START OF READ SET CHECKS
        * 
        */

        //checks for uplink connection packets ------ TCP 
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,pre_buffer,BUFFER_SIZE))!=0){if(n==-1){perror("uplink - read()");exit(1);}
                if(is_root){node.ptp.DA = true;
                    strcpy(buffer, pre_buffer);
                    ptp_encoder("DA", buffer, n, 0, NULL);
                    if (!input.SF){input.SF = true;send_downstream(&clients, "SF\n");}
                }else{
                    yetToReadU = checkForMany(pre_buffer, buffer);
                    printf("yTrU -> %d\n", yetToReadU);
                    ptp_decoder(buffer, &message, 0);
                }
            }else{if(input.debug)printf("Uplink connection failure\n");
                close(ctcp_fd);ctcp_fd = -1;
                if (input.SF) send_downstream(&clients, "BS\n");
                udp_encoder("WHOISROOT", buffer, NULL); //builds WHOISROOT protocol message
                udp_client(0, buffer, input.rs_id); //sends the built message
                udp_decoder(buffer, &message); //decodes received message
            }
        }

        //checks for downlink connection attempts ------- TCP
        else if(FD_ISSET(stcp_fd, &rfds)){
            addrlen=sizeof(addr);
            if((newfd=accept(stcp_fd,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){
                perror("downlink - accept()");exit(1);}
            if (clients < input.tcpsessions){if(input.debug)printf("Connection established on socket: %d\n", newfd);
                Array_Add(new_fds,newfd);  
                ptp_encoder("WE", buffer, 0, 0, NULL);
                write(newfd, buffer, strlen(buffer));//if(input.debug)printf("Sending Welcome message\n");
                clients++;}
            else{if(input.debug)printf("Cant accept more clients, redirecting...\n");
                ptp_encoder("RE", buffer, 0, 0, NULL);
                write(newfd, buffer, strlen(buffer));
                close(newfd);}           
        }

        //check for udp access server requests -------- UDP
        else if(FD_ISSET(sudp_fd,&rfds)){if(input.debug)printf("Detected traffic to upd access server\n");
            addrlen=sizeof(addr);
            if((n=recvfrom(sudp_fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){
                perror("recvfrom access server");exit(1);};      
            udp_decoder(buffer, &message);
        }

        //checks for user input ---- STDIN
        else if(FD_ISSET(STDIN_FILENO,&rfds)){fgets(buffer, BUFFER_SIZE, stdin);user_decoder(buffer);}

        //check for downlink connection packets ----- TCP
        for (int i = 0; i < input.tcpsessions; i++)
            if (FD_ISSET(new_fds[i].fd, &rfds)){
                if((n=read(new_fds[i].fd,pre_D_buffer,BUFFER_SIZE))!=0){
                    if(n==-1){perror("downlink - read()");exit(1);}
                    yetToReadD = checkForMany(pre_D_buffer, D_buffer);
                    printf("yTrD -> %d\n", yetToReadD);
                    ptp_decoder(D_buffer, &D_message, new_fds[i].fd);
                }else{if(input.debug)printf("Detected a connection failure, closing...\n");
                    close(new_fds[i].fd);clients--;
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
            if(input.debug)printf("WE detected\n%s", buffer);
            /*token = strchr(buffer, '\n');token++; 
            if (!strncasecmp(token, "SF", 2)) node.ptp.SF = true;*/
            ptp_encoder("NP", buffer, 0, 0, NULL);
            write(ctcp_fd, buffer, strlen(buffer));
            //CHECKS TO SEE IF STREAM IS THE DESIRED ONE DONE IN ptp_decoder
        }

        if (node.ptp.BS){node.ptp.BS = false;input.SF = false;
            if(input.debug)printf("BS detected\n");
            send_downstream(&clients, "BS\n");
        }

        if (node.ptp.DA){node.ptp.DA = false;
            if(input.debug)printf("DA detected\n");         
            if(input.display){ 
                token = &buffer[0]; token += 8;
                if(input.format) printf("%s\n", token);
                else print_hex(token);}
            send_downstream(&clients, buffer);
        }

        if (node.ptp.NP){node.ptp.NP = false;
            if(input.debug)printf("NP detected\n");
            //DONE IN ptp_decoder FOR NOW
        }

        if (node.ptp.PQ){node.ptp.PQ = false;
            if(input.debug)printf("PQ detected\n%s", buffer);
            query_num = message.keys[0];
            bestpops = message.keys[1];
            if (clients < input.tcpsessions){bestpops--;
                memset(buffer, '\0', strlen(buffer));
                ptp_encoder("PR", buffer, input.tcpsessions - clients, query_num, NULL);
                write(ctcp_fd, buffer, strlen(buffer));
            }      
            if (bestpops > 0){
                memset(buffer, '\0', strlen(buffer));
                sprintf(buffer, "%04X", query_num);
                ptp_encoder("PQ", buffer, bestpops, 0, NULL);
                send_downstream(&clients, buffer);
            }
        }

        if (node.ptp.PR){node.ptp.PR = false;
            if(input.debug)printf("PR detected\n%s", D_buffer);
            if (is_root){
                if (Pquery_active){if(input.debug)printf("POP accepted\n");
                    strcpy(pop[input.bestpops - bestpops].ipport.ip, D_message.address.ip);
                    strcpy(pop[input.bestpops - bestpops].ipport.port, D_message.address.port);
                    pop[input.bestpops - bestpops].key = 0;bestpops--;
                    if (bestpops == 0) Pquery_active = false;
                }else if(input.debug)printf("POP not needed\n");
            }else{
                if (query_num == D_message.keys[0] && bestpops > 0){bestpops--;
                    if(input.debug)printf("POP accepted\n");
                    printf("Sending.. %s\n", buffer);
                    write(ctcp_fd, D_buffer, strlen(D_buffer));
                }else if(input.debug)printf("POP not needed\n");
            }
        }

        if (node.ptp.RE){node.ptp.RE = false;
            if(input.debug){printf("RE detected\n%s", buffer);printf("Closing ctcp: %d\n", ctcp_fd);}
            close(ctcp_fd);ctcp_fd = tcp_client(message.address);
            strcpy(input.uplink.ip, message.address.ip);strcpy(input.uplink.port, message.address.port);
        }

        if (node.ptp.SF){node.ptp.SF = false;
            if(input.debug)printf("SF detected\n");
            input.SF = true;
            send_downstream(&clients, "SF\n");
        }

        if (node.ptp.TQ){node.ptp.TQ = false;
            if(input.debug)printf("TQ detected\n%s", buffer);
            if (!strcasecmp(message.address.ip, input.ipaddr) && !strcasecmp(message.address.port, input.tport)){
                memset(buffer, 0, BUFFER_SIZE);
                ptp_encoder("TR", buffer, 0, 0, NULL);
                write(ctcp_fd, buffer, strlen(buffer));
            }else send_downstream(&clients, buffer);        
        }

        if (node.ptp.TR){node.ptp.TR = false;TQ_time=time(NULL);      
            if (input.debug)printf("TR detected\n%s", D_buffer);
            if (Tquery_active){     
                sscanf(D_buffer, "%*s %[^:]%*[:]%s %d",Tvec[Tvec_C].self.ip, Tvec[Tvec_C].self.port, &Tvec[Tvec_C].tcpsessions);
                Tvec[Tvec_C].ipport = (struct ipport*)calloc(Tvec[Tvec_C].tcpsessions, sizeof(struct ipport));
                token = strtok(D_buffer, "\n");//skips the first line
                token = strtok(NULL, "\n");//gets the second line or NULL if non existing
                for (int i = 0; token != NULL; i++){
                    sscanf(token, "%[^:]%*[:]%s", Tvec[Tvec_C].ipport[i].ip, Tvec[Tvec_C].ipport[i].port);
                    memset(D_buffer, 0, strlen(D_buffer));
                    ptp_encoder("TQ", D_buffer, 0, 0, &Tvec[Tvec_C].ipport[i]);
                    send_downstream(&clients, D_buffer);
                    token = strtok(NULL, "\n");
                }Tvec_C++;
            }else write(ctcp_fd, D_buffer, strlen(D_buffer));
        }




        /*
        *   user related flags
        * 
        */
        if (node.user.exit_){node.user.exit_ = false; 
            if(is_root){
                udp_encoder("REMOVE", buffer, NULL);
                udp_client(1, buffer, input.rs_id);
            }
            close(ctcp_fd);close(stcp_fd);close(sudp_fd);
            for (int i = 0; i < input.tcpsessions; i++) if (new_fds[i].fd != -1) close(new_fds[i].fd);
            free(new_fds);free(pop);
            printf("Exiting...\n");
            exit(0); 
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
            if (!Tquery_active){
                TQ_time = time(NULL);
                Tquery_active = true;
                for (int i = 0; i < input.tcpsessions; i++){
                    if (new_fds[i].fd != -1){
                        memset(buffer, '\0', strlen(buffer));
                        ptp_encoder("TQ", buffer, 0, 0, &new_fds[i].ipport);
                        write(new_fds[i].fd, buffer, strlen(buffer));
                    }
                }
            }else printf("Tree query already underway...\n");
        }




        /*
        *   UDP related flags
        * 
        */ 
        if (node.udp.STREAMS){node.udp.STREAMS = false;
            if(input.debug)printf("STREAMS detected\n");
            token = &buffer[0]; token +=8;
            printf("Streams:\n%s", token);
        }

        if (node.udp.POPREQ){node.udp.POPREQ = false; 
            if(input.debug)printf("POPREQ detected\n");
            memset(buffer, '\0', strlen(buffer));
            udp_encoder("POPRESP", buffer, &(pop[0].ipport));
            if((n=sendto(sudp_fd,buffer,strlen(buffer),0,(struct sockaddr*)&addr,addrlen))==-1){perror("udp_popreq - sendto\n");exit(1);}
        }

        if (node.udp.ROOTIS){node.udp.ROOTIS = false;
            if(input.debug)printf("ROOTIS detected\n");
            strcpy(buffer, "POPREQ\n");counter = 0;
            strcpy(D_message.address.ip, message.address.ip);
            strcpy(D_message.address.port, message.address.port); 
            strcpy(T_buffer, buffer);
            while(1){
                strcpy(buffer, T_buffer);
                udp_client(0, buffer, D_message.address);
                udp_decoder(buffer, &message); 
                ctcp_fd = tcp_client(message.address);
                strcpy(input.uplink.ip, message.address.ip);strcpy(input.uplink.port, message.address.port);
                if (ctcp_fd != -1) break;
                if (counter > 4) exit(1);
                else counter++;
                //sleep(1);
                memset(&message, 0, sizeof(message));memset(buffer, 0, BUFFER_SIZE);
            }
            if(input.debug)printf("Connected to point of presence\n");
        }

        if (node.udp.URROOT){node.udp.URROOT = false;is_root = true;
            if(input.debug)printf("URROOT detected\n");
            ctcp_fd = tcp_client(message.address);
            sudp_fd = udp_server();printf("udp access server created on socket %d\n", sudp_fd); 
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

       //TIMER FOR ROOT SERVER UPDATE AND POP_QUERY TIMEOUT
        if(is_root){
            if (time(NULL) - start >= input.tsecs){
                start = time(NULL);
                if(input.debug)printf("Root server update\n");
                udp_encoder("WHOISROOT", buffer, NULL); //builds WHOISROOT protocol message
                udp_client(0, buffer, input.rs_id);
                
                if (!Pquery_active && !Tquery_active){
                if (clients < input.tcpsessions){
                    bestpops = input.bestpops - 1;
                    strcpy(pop[0].ipport.ip, input.ipaddr);
                    strcpy(pop[0].ipport.port, input.tport);
                    pop[0].key = 0;
                }else bestpops = input.bestpops;
                if (bestpops > 0){
                    memset(buffer, '\0', strlen(buffer));
                    sprintf(buffer, "%04X", ++query_num);
                    ptp_encoder("PQ", buffer, bestpops, 0, NULL); 
                    Pquery_active = true;PQ_time = time(NULL);
                    send_downstream(&clients, buffer);}
                }
            }
            if (Pquery_active) if (time(NULL) - PQ_time >= 2){Pquery_active = false;if(input.debug)printf("PQ timeout\n");}
        }
            
        if (Tquery_active) if (time(NULL) - TQ_time >= 2){
            Tquery_active = false;if(input.debug)printf("TQ timeout\n");
            printf("\n%s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
            printf("%s:%s (%d", input.ipaddr, input.tport, input.tcpsessions);
            for (int i = 0; i < input.tcpsessions; i++)if (new_fds[i].fd != -1) 
                printf(" %s:%s", new_fds[i].ipport.ip, new_fds[i].ipport.port);
            printf(")\n");
            for (int i = 0; i < 64; i++){
                if (Tvec[i].tcpsessions > 0){
                    printf("%s:%s (%d", Tvec[i].self.ip, Tvec[i].self.port, Tvec[i].tcpsessions);
                    for (int j = 0; j < Tvec[i].tcpsessions; j++)
                        if (strcasecmp(Tvec[i].ipport[j].ip, "\0") && strcasecmp(Tvec[i].ipport[j].port, "\0"))
                            printf(" %s:%s", Tvec[i].ipport[j].ip, Tvec[i].ipport[j].port);
                    printf(")\n");free(Tvec[i].ipport);Tvec[i].tcpsessions = 0;
                    memset(&Tvec[i].self, '\0', sizeof (struct ipport));}
            }printf("\n");Tvec_C=0;
        }

        memset(&message, 0, sizeof message);
        memset(&D_message, 0, sizeof message);
        memset(buffer, 0, BUFFER_SIZE);memset(D_buffer, 0, BUFFER_SIZE);memset(T_buffer, 0, BUFFER_SIZE);
        if(yetToReadD || yetToReadU) continue;
        //Clears stuff for the next cicle
        memset(pre_buffer, 0, BUFFER_SIZE);memset(pre_D_buffer, 0, BUFFER_SIZE);
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
    for(int i = 0; i < input.tcpsessions; i++) 
        if (vector[i].fd == fd){vector[i].fd = -1; 
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
        if (new_fds[i].fd != -1 && write(new_fds[i].fd, buffer, strlen(buffer)) == -1){
                if(input.debug)printf("Detected a connection failure, closing...\n");
                close(new_fds[i].fd); clients--;
                Array_Rem(new_fds, new_fds[i].fd);}
}

void print_status (int clients){
    printf("Connected to stream: %s:%s:%s\n", input.stream_id.name, input.stream_id.ip, input.stream_id.port);
    if (input.SF) printf("Stream is good\n");
    else printf("Stream broken\n");
    if (is_root){printf("I am root!\n");
        printf("UDP access server on: %s:%s\n", input.ipaddr, input.uport);}
    else{printf("I am groot!\n");printf("Uplink node: %s:%s\n", input.uplink.ip, input.uplink.port);}
    printf("Access point on: %s:%s\n", input.ipaddr, input.tport);
    printf("Supported sessions: %d - Occupied: %d\n", input.tcpsessions, clients);
    printf("Connected pairs:\n");
    for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd != -1) printf("%s:%s\n", new_fds[i].ipport.ip, new_fds[i].ipport.port);
}

void print_hex(const char *s){
    while(*s) printf("%02X ", (unsigned int) *s++);
    printf("\n\n");
}