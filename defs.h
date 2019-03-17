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
#include <time.h>

#define BUFFER_SIZE 512

struct udp_protocol{
    bool ROOTIS;
    bool URROOT;
    bool WHOISROOT;
    bool REMOVE;
    bool DUMP;
    bool STREAMS;
    bool ERROR;
    bool POPRESP;
    bool POPREQ;
};

struct user_commands{
    bool streams;
    bool status;
    bool display;
    bool format;
    bool debug;
    bool tree;
    bool exit_;
};

struct ptp_protocol{
    bool WE;
    bool NP;
    bool RE;
    bool SF;
    bool BS;
    bool DA;
    bool PQ;
    bool PR;
    bool TQ;
    bool TR;
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
    unsigned int tcpsessions; //tcp session to provide
    unsigned int bestpops; //number of access points to keep
    unsigned int tsecs; //root update time
    bool display; 
    bool debug;
    bool help;
    bool SF;
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

struct client {
    struct ipport ipport;
    int fd;
};

//iamroot.c
int Array_Max (struct client*);
int Array_Add (struct client*, int);
int Array_Rem (struct client*, int);
int Array_Addipport (struct client *, int, struct ipport);
void send_downstream (int *, char *);
void print_status(int);

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
void ptp_encoder (char*, char*, int n);
void ptp_decoder (char*, struct message *, int);
void user_decoder (char*);

//global vars declaration
struct access_point pop[10];
struct input input;
struct application node;
struct client *new_fds;
bool is_root;

#endif