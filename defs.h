#ifndef _DEFS_H
#define _DEFS_H


enum flag {off, on};

struct stream{
    char name[63];
    char ip[63];
    char port[63];
};

struct rs{
    char adress[63];
    char port[63];
};
 
struct input{
    struct stream stream_id; //stream data
    char ipaddr[63]; // self ip
    char tport[63]; //self tcp port (downlink server)
    char uport[63]; //self udp port (access server)
    struct rs rs_id; //root server ip and port
    char tcpsessions[63]; //tcp session to provide
    char bestpops[63]; //number of access points to keep
    char tsecs[63]; //root update time
    enum flag display; 
    enum flag advanced;
    enum flag help;
};

struct message{
    char command[32];
    char args[32][10];
};

//inputHandler.c
void inputHandler (char **, int);
void display_help ();

//UDP_client.c
void udp_client(int, char *);

//packetHandler.c
void udp_encoder (char *, char *);
void udp_decoder (char *, struct message *);


#endif