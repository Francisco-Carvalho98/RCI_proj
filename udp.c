#include "defs.h"

void udp_client (int key, char *message, struct ipport ipport){
    
    struct addrinfo hints,*res;
    int fd,n, addrlen;
    struct sockaddr_in addr;
    //int errcode;
    //char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;
    
    n=getaddrinfo(ipport.ip,ipport.port,&hints,&res);
    if(n!=0){perror("udp_client getaddrinfo()");exit(1);}
    
    fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(fd==-1){perror("udp_client socket()");exit(1);}
    
    n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
    if(n==-1){perror("udp_client sendto()");exit(1);}

    freeaddrinfo(res);

    addrlen=sizeof(addr);
    memset(message,'\0',BUFFER_SIZE);//message variable reused to save received message

    if (key!=1){//condition when REMOVE is sent and no answer is expected. would get stuck on recvfrom
        printf("Waiting for a reply from server\n");
        n=recvfrom(fd,message,BUFFER_SIZE,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen);
        if(n==-1){perror("udp_client recvfrom()");exit(1);}
        //printf("Message received:\n");
        //write(1,message,n);
    
        /*if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
            fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
        else printf("sent by [%s:%s]\n",host,service);*/
    }

    if(input.debug)printf("cudp socket created %d\n", fd);
    if(input.debug)printf("cudp socket closing %d\n", fd);
    close(fd);
     
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

    if(input.debug)printf("sudp socket created - %d\n", fd);
    return fd;
}