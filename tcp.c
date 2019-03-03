#include "defs.h"

extern struct input input;

int tcp_client (int key, struct ipport ipport){

    struct addrinfo hints,*res;
    int fd,n;
    

    //SIGPIPE protection
    struct sigaction act;
    memset(&act,0,sizeof act);
    act.sa_handler=SIG_IGN;
    if(sigaction(SIGPIPE,&act,NULL)==-1){perror("sigaction()");exit(1);}
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;//TCP socket
    hints.ai_flags=AI_NUMERICSERV;
    
    if (key == 0){//when root
        n=getaddrinfo(ipport.adress,ipport.port,&hints,&res);
        if(n!=0){perror("tcp_client getaddrinfo()");exit(1);}
    }else{//when not root
        n=getaddrinfo(ipport.adress,ipport.port,&hints,&res);
        if(n!=0){perror("tcp_client getaddrinfo()");exit(1);}}

    fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(fd==-1){perror("tcp_client socket()");exit(1);}
    
    n=connect(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1){perror("tcp_client connect()");exit(1);}

    return fd;
}

int tcp_server (){

    struct addrinfo hints,*res;
    int fd,n;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET;//IPv4
    hints.ai_socktype=SOCK_STREAM;//TCP socket 
    hints.ai_flags=AI_PASSIVE|AI_NUMERICSERV; 
    
    n=getaddrinfo(NULL,input.tport,&hints,&res);
    if(n!=0)/*error*/exit(1);
    
    if((fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1)/*error*/exit(1);
    printf("fd: %d\n", fd);
    
    if(bind(fd,res->ai_addr,res->ai_addrlen)==-1)/*error*/exit(1);
    if(listen(fd,5)==-1)/*error*/exit(1);

    return fd;
}