#include "defs.h"

extern struct input input;

int tcp_client (int key){

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
    
    if (key == 0){
        n=getaddrinfo(input.stream_id.ip,input.stream_id.port,&hints,&res);
        if(n!=0){perror("getaddrinfo()");exit(1);}
    }else{
        n=getaddrinfo(input.ipaddr,input.uport,&hints,&res);
        if(n!=0){perror("getaddrinfo()");exit(1);}}

    fd=socket(res->ai_family,res->ai_socktype,res->ai_protocol);
    if(fd==-1){perror("socket()");exit(1);}
    
    n=connect(fd,res->ai_addr,res->ai_addrlen);
    if(n==-1){perror("connect()");exit(1);}

    return fd;
}