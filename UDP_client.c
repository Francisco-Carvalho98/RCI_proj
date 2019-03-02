#include "defs.h"

extern struct input input;

void udp_client (int key, char *message){
    
    struct addrinfo hints,*res;
    int fd,n, addrlen;
    struct sockaddr_in addr;
    int errcode;
    char host[NI_MAXHOST],service[NI_MAXSERV];//consts in <netdb.h>
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_DGRAM;
    hints.ai_flags=AI_CANONNAME;


    
    n=getaddrinfo(input.rs_id.adress,input.rs_id.port,&hints,&res);
    if(n!=0) exit(1);
    
    fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(fd==-1) exit(1);
    
    n=sendto(fd,message,strlen(message),0,res->ai_addr,res->ai_addrlen);
    if(n==-1) exit(1); 

    freeaddrinfo(res);

    addrlen=sizeof(addr);
    memset(message,0,strlen(message));

    if (key!=1){    
        n=recvfrom(fd,message,64,0,(struct sockaddr*)&addr,(unsigned int *)&addrlen);
        if(n==-1) exit(1);

        write(1,message,n);
    
        if((errcode=getnameinfo((struct sockaddr *)&addr,addrlen,host,sizeof host,service,sizeof service,0))!=0)
            fprintf(stderr,"error: getnameinfo: %s\n",gai_strerror(errcode));
        else printf("sent by [%s:%s]\n",host,service);
    }

    close(fd);
     
    return;
} 