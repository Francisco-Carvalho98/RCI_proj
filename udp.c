#include "defs.h"

void udp_client (int key, char *buffer, struct ipport ipport){
    
    struct addrinfo hints,*res;
    int fd,n, addrlen;
    struct sockaddr_in addr;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;
    
    if((n=getaddrinfo(ipport.ip,ipport.port,&hints,&res))!=0){perror("udp_client getaddrinfo()");exit(1);}
    
    if((fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1){perror("udp_client socket()");exit(1);}
    //if(input.debug)printf("cudp socket created %d\n", fd);
    
    if((n=sendto(fd,buffer,strlen(buffer),0,res->ai_addr,res->ai_addrlen))==-1){perror("udp_client sendto()");exit(1);}

    freeaddrinfo(res);

    addrlen=sizeof(addr);
    memset(buffer,'\0',BUFFER_SIZE);//buffer variable reused to save received buffer

    //condition when REMOVE is sent and no answer is expected. would get stuck on recvfrom
    if (key!=1)
        if((n=recvfrom(fd,buffer,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen))==-1)
        {perror("udp_client recvfrom()");exit(1);}
    
    printf("UDP --> %s", buffer);
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