#include "defs.h"

int main (int argc, char **argv)
{
    //VARIABLES DECLARATION
    char buffer[BUFFER_SIZE], D_buffer[BUFFER_SIZE],T_buffer[BUFFER_SIZE], *token;
    char pre_buffer[BUFFER_SIZE], pre_D_buffer[BUFFER_SIZE];
    memset(pre_buffer, 0, BUFFER_SIZE);memset(pre_D_buffer, 0, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);memset(D_buffer, 0, BUFFER_SIZE);memset(T_buffer, 0, BUFFER_SIZE);
    struct message message, D_message;
    memset(&message, '\0', sizeof message);memset(&D_message, '\0', sizeof message);
    int ctcp_fd=-1, sudp_fd=-1, stcp_fd=-1;
    fd_set rfds;
    int counter=0, n, addrlen, newfd=-1, maxfd=-1, clients = 0, bestpops = 0, yetToReadU = 0, yetToReadD = 0, loop = 0;
    unsigned short query_num = -1;
    struct sockaddr_in addr;
    struct timeval timeout;
    bool Pquery_active = false, Tquery_active = false;
    struct tree Tvec[512];int Tvec_C=0;
    memset(Tvec, 0, 64*sizeof(struct tree));
    time_t start = time(NULL), PQ_time = time(NULL), TQ_time = time(NULL);
    
    //takes care of program input args or sets to default if not specified
    inputHandler(argv, argc);

    //allocs client struct vector to the number of tcpsessions supported by node
    if ((new_fds = (struct client*)calloc(input.tcpsessions, sizeof(struct client))) == NULL){printf("Couldn't allocate memory\n");exit(1);}
    for (int i = 0; i < input.tcpsessions; i++) new_fds[i].fd =-1;

    //allocs pop struct vector to the number of bestpops
    if ((pop = (struct pop*)calloc(input.bestpops, sizeof(struct pop))) == NULL){printf("Couldn't allocate memory\n");exit(1);}
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

    while(1){

        //BUILDS READ SET
        FD_ZERO(&rfds);
        FD_SET(ctcp_fd,&rfds);
        FD_SET(sudp_fd,&rfds);
        FD_SET(stcp_fd, &rfds);
        FD_SET(STDIN_FILENO,&rfds);
        for (int i = 0; i < input.tcpsessions; i++) if(new_fds[i].fd!=-1) FD_SET(new_fds[i].fd, &rfds);

        //TIMEOUT UPDATE
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        //GETS THE BIGGEST FD 
        maxfd = Array_Max(new_fds); 
        maxfd = ctcp_fd > maxfd ? ctcp_fd : maxfd;
        maxfd = maxfd > sudp_fd ? maxfd : sudp_fd;

        //DOESNT RUN SELECT IN THE FIRST ITERATION OR IF BUFFERS HAD MORE THAN 1 MESSAGE
        //AND STILL HAVE YET TO PROCESS THE REST BEFORE RECEIVING ANYTHING ELSE
        //IN THIS CASE WE WANT RFDS TO BE EMPTY AS TO NOT TO ENTER IN THE FD_ISSET'S
        if (ctcp_fd != -1 && !yetToReadU && !yetToReadD) counter=select(maxfd+1,&rfds, NULL, NULL, &timeout);
        else FD_ZERO(&rfds);

        if (yetToReadU){//UPLINK BUFFER STILL HAS MESSAGE(S) TO PROCESS
            yetToReadU = checkForMany(pre_buffer, buffer);
            ptp_decoder(buffer, &message, 0);
        }
        if (yetToReadD){//DOWNLINK BUFFER STILL HAS MESSAGE(S) TO PROCESS
            yetToReadD = checkForMany(pre_D_buffer, D_buffer);
            ptp_decoder(D_buffer, &D_message, 0);
        }
        
        if(counter<0){perror("select()"); exit(1);}

        //IF THE PROGRAM COMES HERE 6 CONSECUTIVE TIMES, THAT MEANS THE VARIABLE WASNT SET TO 0 WHEN
        //SOMETHING IS DETECTED FROM THE UPLINK CONNECTION, WHICH LIKELY MEANS A LOOP HAS BEEN
        //FORMED WITH A NODE LEAVING THE TREE RECENTLY. TO PREVENT FURTHER LOOPS, THE PROGRAM
        //DISCONNECTS FROM EVERY OTHER NODE AND TRIES TO REJOIN JUST LIKE IN THE BEGINNING
        else if(counter == 0){
            if(input.debug && ctcp_fd != -1){loop++;
                if (loop > 6){loop = 0;close(ctcp_fd);clients = 0;
                    for (int i = 0; i < input.tcpsessions; i++) 
                        if (new_fds[i].fd != -1){close(new_fds[i].fd);Array_Rem(new_fds, new_fds[i].fd);}
                    strcpy(new_fds[0].ipport.ip, input.ipaddr);
                    strcpy(new_fds[0].ipport.port, input.tport);
                    udp_encoder("WHOISROOT", buffer, NULL);
                    udp_client(0, buffer, input.rs_id); 
                    udp_decoder(buffer, &message); 
                }
            }
        }

        /*
        *
        * START OF READ SET CHECKS
        * 
        */

        //checks for uplink connection packets ------ TCP 
        if(FD_ISSET(ctcp_fd,&rfds)){
            if((n=read(ctcp_fd,pre_buffer,BUFFER_SIZE))!=0){if(n==-1){perror("uplink - read()");exit(1);}
                loop = 0;//VARIABLE USED TO CATCH LOOPS
                if(is_root){node.ptp.DA = true;//ENCAPSULATES DATA FROM STREAM SOURCE IN A DA PACKAGE
                    strcpy(buffer, pre_buffer);
                    ptp_encoder("DA", buffer, n, 0, NULL);
                    //LOGIC TO KNOW IF STREAM IS FLOWING OR BROKEN. SENDS SF FOR THE VERY SPECIAL CASE OF
                    //A NODE CONNECTING RIGHT AFTER THE TREE HAD LAUNCHED, AND INPUT.SF HAD NOT YET BEEN SET TO TRUE
                    if (!input.SF){input.SF = true;if(clients!=0)send_downstream(&clients, "SF\n");}
                }else{
                    yetToReadU = checkForMany(pre_buffer, buffer);//FUNCTION CALLED TO CHECK IF BUFFER BRINGS MORE THAN 1 MESSAGE
                    ptp_decoder(buffer, &message, 0);//FUNCTIONS CALLED TO DECODED RECEIVED MESSAGE
                }
            //IN CASE OF AN UPLINK CONNECTION FAILURE, A BS IS SENT DOWNSTREAM ONLY IF THE STREAM WAS
            //GOOD BEFORE THE FAILURE, AS TO PREVENT A BS ENDLESS STREAM IN A CASE OF A LOOP, AND THE NODE TRIES TO RECONNECT
            }else{close(ctcp_fd);ctcp_fd = -1;
                if (input.SF) if(clients!=0)send_downstream(&clients, "BS\n");
                input.SF = false;
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
            //CHECKS WHETHER APP CAN TAKE OR NOT ANYMORE CONNECTIONS
            if (clients < input.tcpsessions){
                Array_Add(new_fds,newfd);  
                ptp_encoder("WE", buffer, 0, 0, NULL);
                if(input.debug)printf("Sending: %s", buffer);
                write(newfd, buffer, strlen(buffer));//if(input.debug)printf("Sending Welcome message\n");
                clients++;}
            else{//IF NOT, REDIRECTS BY DEFAULT TO THE FIRST NON EMPTY CLIENT IN THE CLIENT INFO ARRAY
                ptp_encoder("RE", buffer, 0, 0, NULL);
                if(input.debug)printf("Sending: %s", buffer);
                write(newfd, buffer, strlen(buffer));
                close(newfd);}           
        }

        //check for udp access server requests -------- UDP
        else if(FD_ISSET(sudp_fd,&rfds)){
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
                    ptp_decoder(D_buffer, &D_message, new_fds[i].fd);
                }else{close(new_fds[i].fd);clients--;
                    Array_Rem(new_fds, new_fds[i].fd);
                    if (clients == 0){
                        strcpy(new_fds[0].ipport.ip, input.ipaddr);
                        strcpy(new_fds[0].ipport.port, input.tport);}
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
            if(input.debug)printf("WE detected:\n%s", buffer);
            ptp_encoder("NP", buffer, 0, 0, NULL);
            if(input.debug)printf("Sending: %s", buffer);
            write(ctcp_fd, buffer, strlen(buffer));
            //CHECKS TO SEE IF STREAM IS THE DESIRED ONE DONE IN ptp_decoder
        }

        if (node.ptp.BS){node.ptp.BS = false;input.SF = false;
            if(input.debug)printf("BS detected:\n");
            if(input.debug && clients != 0)printf("Sending: BS\n");
            if(clients!=0)send_downstream(&clients, "BS\n");
        }

        if (node.ptp.DA){node.ptp.DA = false;
            //if(input.debug)printf("DA detected:\n");         
            if(input.display){ 
                token = &buffer[0]; token += 8;
                if(input.format) printf("%s\n", token);
                else print_hex(token);}
            if(clients!=0)send_downstream(&clients, buffer);
        }

        if (node.ptp.NP){node.ptp.NP = false;
            if(input.debug)printf("NP detected:\n%s", D_buffer);
            //DONE IN ptp_decoder FOR NOW
        }

        if (node.ptp.PQ){node.ptp.PQ = false;
            if(input.debug)printf("PQ detected:\n%s", buffer);
            query_num = message.keys[0];//SAVES QUERY_NUM TO KNOW WHETHER OR NOT TO REJECT FUTURE PR
            bestpops = message.keys[1];
            //IF APP CAN ACCEPT CLIENTS, SENDS PR MESSAGE
            if (clients < input.tcpsessions){bestpops--;
                memset(buffer, '\0', strlen(buffer));
                ptp_encoder("PR", buffer, input.tcpsessions - clients, query_num, NULL);
                if(input.debug)printf("Sending: %s", buffer);
                write(ctcp_fd, buffer, strlen(buffer));
            }      
            if (bestpops > 0){
                memset(buffer, '\0', strlen(buffer));
                sprintf(buffer, "%04X", query_num);
                ptp_encoder("PQ", buffer, bestpops, 0, NULL);
                if(input.debug && clients!=0)printf("Sending: %s", buffer);
                if(clients!=0)send_downstream(&clients, buffer);
            }
        }

        if (node.ptp.PR){node.ptp.PR = false;
            if(input.debug)printf("PR detected:\n%s", D_buffer);
            if (is_root){
                if (Pquery_active){//LOGIG TO KNOW WHETHER TO SAVE OR NOT A PR RECEIVED, AND TO TERMINATE CURRENT QUERY
                    if (query_num == D_message.keys[0] && bestpops > 0){
                        strcpy(pop[input.bestpops - bestpops].ipport.ip, D_message.address.ip);
                        strcpy(pop[input.bestpops - bestpops].ipport.port, D_message.address.port);
                        pop[input.bestpops - bestpops].key = 0;bestpops--;
                        if (bestpops == 0) Pquery_active = false;}
                }
            }else{
                if (query_num == D_message.keys[0] && bestpops > 0){bestpops--;
                    if(input.debug)printf("Sending: %s", D_buffer);
                    write(ctcp_fd, D_buffer, strlen(D_buffer));}
            }
        }

        if (node.ptp.RE){node.ptp.RE = false;
            if(input.debug){printf("RE detected:\n%s", buffer);}
            close(ctcp_fd);ctcp_fd = tcp_client(message.address);
            strcpy(input.uplink.ip, message.address.ip);strcpy(input.uplink.port, message.address.port);
        }

        if (node.ptp.SF){node.ptp.SF = false;
            if(input.debug)printf("SF detected:\n%s", buffer);
            input.SF = true;
            if(input.debug && clients != 0)printf("Sending: SF\n");
            if(clients!=0)send_downstream(&clients, "SF\n");
        }

        if (node.ptp.TQ){node.ptp.TQ = false;
            if(input.debug)printf("TQ detected:\n%s", buffer);
            //IF TQ IS TO SELF, SENDS TR ELSE SENDS DOWNSTREAM
            if (!strcasecmp(message.address.ip, input.ipaddr) && !strcasecmp(message.address.port, input.tport)){
                memset(buffer, 0, BUFFER_SIZE);
                ptp_encoder("TR", buffer, 0, 0, NULL);
                if(input.debug)printf("Sending: %s", buffer);
                write(ctcp_fd, buffer, strlen(buffer));
            }else{
                if(input.debug && clients!=0)printf("Sending: %s", buffer);
                if(clients!=0)send_downstream(&clients, buffer);  
            }      
        }

        //RESETS TQ TIMEOUT, PROCESSES A TR, SAVES INFORMATION TO TVEC FOR PRINTING LATER
        //SENDS TQ TO EVENTUAL NODES THAT COME IN TR'S
        if (node.ptp.TR){node.ptp.TR = false;TQ_time=time(NULL);      
            if (input.debug)printf("TR detected:\n%s", D_buffer);
            if (Tquery_active){     
                sscanf(D_buffer, "%*s %[^:]%*[:]%s %d",Tvec[Tvec_C].self.ip, Tvec[Tvec_C].self.port, &Tvec[Tvec_C].tcpsessions);
                Tvec[Tvec_C].ipport = (struct ipport*)calloc(Tvec[Tvec_C].tcpsessions, sizeof(struct ipport));
                token = strtok(D_buffer, "\n");//skips the first line
                token = strtok(NULL, "\n");//gets the second line or NULL if non existing
                for (int i = 0; token != NULL; i++){
                    sscanf(token, "%[^:]%*[:]%s", Tvec[Tvec_C].ipport[i].ip, Tvec[Tvec_C].ipport[i].port);
                    memset(D_buffer, 0, strlen(D_buffer));
                    ptp_encoder("TQ", D_buffer, 0, 0, &Tvec[Tvec_C].ipport[i]);
                    if(input.debug && clients!=0)printf("Sending: %s", D_buffer);
                    if(clients!=0)send_downstream(&clients, D_buffer);
                    token = strtok(NULL, "\n");
                }Tvec_C++;
            }else{
                if(input.debug)printf("Sending: %s", D_buffer);
                write(ctcp_fd, D_buffer, strlen(D_buffer));
            }
        }




        /*
        *   user related flags
        * 
        */
        if (node.user.exit_){node.user.exit_ = false; 
            if(is_root){
                close(sudp_fd);
                udp_encoder("REMOVE", buffer, NULL);
                udp_client(1, buffer, input.rs_id);
            }
            close(ctcp_fd);close(stcp_fd);
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
            if(input.debug)printf("Sending: DUMP\n");
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
                        if(input.debug)printf("Sending: %s", buffer);
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
            if(input.debug)printf("STREAMS detected:\n");
            token = &buffer[0]; token +=8;//SKIPS THE HEADER
            printf("Streams:\n%s", token);
        }

        if (node.udp.POPREQ){node.udp.POPREQ = false; 
            if(input.debug)printf("POPREQ detected:\n%s", buffer);
            memset(buffer, '\0', strlen(buffer));
            while(1){//REPLIES WITH A RANDOM NON EMPTY POP
                srand(time(0));
                if (pop[rand() % input.bestpops].key != -1){ 
                    udp_encoder("POPRESP", buffer, &(pop[0].ipport));
                    break;
                }else continue;
            }
            if(input.debug)printf("Sending: %s", buffer);
            if((n=sendto(sudp_fd,buffer,strlen(buffer),0,(struct sockaddr*)&addr,addrlen))==-1){perror("udp_popreq - sendto\n");exit(1);}
        }

        if (node.udp.ROOTIS){node.udp.ROOTIS = false;
            if(input.debug)printf("ROOTIS detected:\n%s", buffer);
            strcpy(buffer, "POPREQ\n");counter = 0;
            strcpy(D_message.address.ip, message.address.ip);
            strcpy(D_message.address.port, message.address.port); 
            strcpy(T_buffer, buffer);
            //TRIES A FEW TIMES BEFORE GIVING UP. USEFUL IN THE CASE OF RECEIVING A BAD 
            //TCP ADDRESS, USUALLY WHEN THE TREE REPLIES WITH A NODE ADDRESS THAT HAS SINCE
            //LEFT THE TREE. ROOT WILL EVENTUALLY RUN NEW POP QUERY THAT SHOULD THEN YIELD
            // A GOOD TCP ADDRESS FOR THIS NODE TO CONNECT TO
            while(1){
                strcpy(buffer, T_buffer);
                if(input.debug)printf("Sending: %s", buffer);
                udp_client(0, buffer, D_message.address);
                udp_decoder(buffer, &message); 
                ctcp_fd = tcp_client(message.address);
                strcpy(input.uplink.ip, message.address.ip);strcpy(input.uplink.port, message.address.port);
                if (ctcp_fd != -1) break;
                if (counter > 4) exit(1);
                else counter++;
                memset(&message, 0, sizeof(message));memset(buffer, 0, BUFFER_SIZE);
            }
        }

        if (node.udp.URROOT){node.udp.URROOT = false;is_root = true;
            if(input.debug)printf("URROOT detected:\n%s", buffer);
            ctcp_fd = tcp_client(message.address);
            sudp_fd = udp_server();
            strcpy(pop[0].ipport.ip, input.ipaddr);
            strcpy(pop[0].ipport.port, input.tport);
            pop[0].key = 0;
            if(input.debug && clients != 0)printf("Sending: SF\n");
            if(clients!=0)send_downstream(&clients, "SF\n");
        }

        if (node.udp.POPRESP){node.udp.POPRESP = false;
            if(input.debug)printf("POPRESP detected:\n%s", buffer);
        }

        /*
        *
        * END OF FLAG CHECKING
        * 
        */

       //TIMER FOR ROOT SERVER UPDATE AND PERIODIC POP_QUERY
        if(is_root){
            if (time(NULL) - start >= input.tsecs){
                start = time(NULL);//RESETS TIMER FOR NEXT UPDATE
                udp_encoder("WHOISROOT", buffer, NULL);
                udp_client(1, buffer, input.rs_id);
                memset(buffer, 0, BUFFER_SIZE);//NEGLETS ANSWER
                
            if (!Pquery_active && !Tquery_active){//CALLS POP_QUERY IF BOTH QUERIES ARE INNACTIVE
                memset(pop, 0, input.bestpops*sizeof(struct pop));//CLEARS CURRENT POP ARRAY
                for (int i = 0; i < input.bestpops; i++) pop[i].key = -1;
                //FILLS ONE POSITION IN CASE A POPREQ IS RECEIVED BEFORE A PR
                strcpy(pop[0].ipport.ip, input.ipaddr);strcpy(pop[0].ipport.port, input.tport);pop[0].key = 0;
                //CHECKS IF NODE CAN STILL HOLD MORE CONNECTIONS
                bestpops = (clients < input.tcpsessions) ? input.bestpops - 1 : input.bestpops;
                if (bestpops > 0){
                    memset(buffer, '\0', strlen(buffer));
                    sprintf(buffer, "%04X", ++query_num);//USES IN/OUT VAR TO CARRY HEX VERSION OF QUERY_NUM
                    ptp_encoder("PQ", buffer, bestpops, 0, NULL); 
                    Pquery_active = true;PQ_time = time(NULL);
                    if(input.debug && clients!=0)printf("Sending: %s", buffer);
                    if(clients!=0)send_downstream(&clients, buffer);}
                }
            }
            if (Pquery_active) if (time(NULL) - PQ_time >= 2){Pquery_active = false;if(input.debug)printf("PQ timeout\n");}
        }
        
        /* PRINTS TREE IF 2 SECONDS HAVE PASSED SINCE LAST RECEIVED PR */
        if (Tquery_active) if (time(NULL) - TQ_time >= 2){Tquery_active = false;
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

        //Clears stuff for the next cicle
        memset(&message, 0, sizeof message);
        memset(&D_message, 0, sizeof message);
        memset(buffer, 0, BUFFER_SIZE);memset(D_buffer, 0, BUFFER_SIZE);memset(T_buffer, 0, BUFFER_SIZE);
        if(yetToReadD || yetToReadU) continue;
        memset(pre_buffer, 0, BUFFER_SIZE);memset(pre_D_buffer, 0, BUFFER_SIZE);
    }
    return 0;
}

/**
 * 
 * outputs integer of the biggest fd, 0 if empty
 * 
 **/
int Array_Max (struct client *vector){
    int max = 0;
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd > max) max = vector[i].fd;
    return max;  
}

/**
 * 
 * Adds fd to the first empty spot in vector
 * 
 * 
 **/
int Array_Add (struct client *vector, int fd){
    for(int i = 0; i < input.tcpsessions; i++) if (vector[i].fd == -1){vector[i].fd = fd; return 0;}
    return -1;
}

/**
 * 
 * 
 * Removes client with matching fd
 * 
 * */
int Array_Rem (struct client *vector, int fd){
    for(int i = 0; i < input.tcpsessions; i++) 
        if (vector[i].fd == fd){vector[i].fd = -1; 
            memset(&vector[i].ipport, '\0', sizeof(struct ipport));
            return 0;}
    return -1;
}


/**
 * 
 * Adds ipport to matching fd in vector
 * 
 * 
 * */
int Array_Addipport (struct client *vector, int fd, struct ipport ipport){
    for(int i = 0; i < input.tcpsessions; i++)  
        if (vector[i].fd == fd){
            strcpy(vector[i].ipport.ip, ipport.ip);
            strcpy(vector[i].ipport.port, ipport.port);
            return 0;}
    return -1;
}

/**
 * 
 * 
 * Sends buffer to every client connected, closes associated connection in case of error
 * 
 * */
void send_downstream (int * clients, char * buffer){
    for (int i = 0; i < input.tcpsessions; i++)
        if (new_fds[i].fd != -1 && write(new_fds[i].fd, buffer, strlen(buffer)) == -1){
                close(new_fds[i].fd); clients--;
                Array_Rem(new_fds, new_fds[i].fd);}
}

/**
 * 
 * 
 * Handles the display of information for command status 
 * 
 * 
 * */
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

/**
 * 
 * 
 * Handles the display of information when format is set to HEX
 * 
 * */
void print_hex(const char *s){
    while(*s) printf("%02X ", (unsigned int) *s++);
    printf("\n\n");
}