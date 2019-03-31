#include "defs.h"

void udp_client (int key, char *buffer, struct ipport ipport){
    
    struct addrinfo hints,*res;
    int fd,n, addrlen, counter=0;
    struct sockaddr_in addr;
    fd_set set;
    struct timeval timeout;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((n=getaddrinfo(ipport.ip,ipport.port,&hints,&res))!=0){perror("udp_client getaddrinfo()");exit(1);}
    
    if((fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1){perror("udp_client socket()");exit(1);}
    //if(input.debug)printf("cudp socket created %d\n", fd);
    
    if((n=sendto(fd,buffer,strlen(buffer),0,res->ai_addr,res->ai_addrlen))==-1){perror("udp_client sendto()");exit(1);}

    //condition when REMOVE is sent and no answer is expected. would get stuck on recvfrom
    while (key!=1){

        FD_ZERO(&set);
        FD_SET(fd, &set);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        n=select(fd+1, &set, (fd_set *)NULL, (fd_set *)NULL, &timeout);
        if(n<0){perror("select()"); exit(1);}
        if(counter >= 3){if(input.debug)printf("UDP timeout\n");exit(1);}

        if(n == 0){
            if(input.debug)printf("Retrying...\n");
            if((n=sendto(fd,buffer,strlen(buffer),0,res->ai_addr,res->ai_addrlen))==-1){
                perror("udp_client sendto()");exit(1);}
            counter++;continue;}
        
        addrlen=sizeof(addr);
        memset(buffer,'\0',BUFFER_SIZE);//buffer variable reused to save received buffer
        if((n=recvfrom(fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1){
            perror("udp_client recvfrom()");exit(1);}
        break;
    }
    
    //printf("UDP --> %s", buffer);
    freeaddrinfo(res);
    close(fd);//if(input.debug)printf("cudp socket closing %d\n", fd);

    return;
} 

int udp_server (){

    struct addrinfo hints,*res;
    int fd,n;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;//UDP socket
    hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV;
    
    n=getaddrinfo(NULL,input.uport,&hints,&res);
    if(n!=0){perror("udp_server getaddrinfo()");exit(1);}

    fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(fd==-1){perror("udp_server socket()");exit(1);}

    n=bind(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1){perror("udp_server bind()");exit(1);}

    freeaddrinfo(res);
    //if(input.debug)printf("sudp socket created - %d\n", fd);
    return fd;
}