#ifndef _DEFS_H
#define _DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/select.h>
#include <stdbool.h>

struct udp_protocol{
    bool test;
};

struct user_commands{
    bool test;
};

struct ptp_protocol{
    bool test;
};

struct application{
    struct udp_protocol udp;
    struct user_commands user;
    struct ptp_protocol ptp;
};

struct stream{
    char name[63];
    char ip[63];
    char port[63];
};

struct ipport{
    char ip[63];
    char port[63];
};
 
//saves input args
struct input{
    struct stream stream_id; //stream data
    char ipaddr[63]; // self ip
    char tport[63]; //self tcp port (downlink server)
    char uport[63]; //self udp port (access server)
    struct ipport rs_id; //root server ip and port
    char tcpsessions[63]; //tcp session to provide
    char bestpops[63]; //number of access points to keep
    char tsecs[63]; //root update time
    bool display; 
    bool advanced;
    bool help;
};

struct message{
    char command[32];
    char args[2][64];
    struct ipport address; //used for tcp_client
};

struct access_point {
    struct ipport ipport;
    int key;
};

//inputHandler.c
void inputHandler (char **, int);
void display_help ();
void print_input();

//udp.c
void udp_client(int, char *, struct ipport);
int udp_server ();

//tcp.c
int tcp_client(struct ipport);
int tcp_server ();

//packetHandler.c
void udp_encoder (char *, char *, struct ipport *);
void udp_decoder (char *, struct message *);


//global vars declaration
struct access_point pop[10];
struct input input;

#endif