#include "defs.h"

void inputHandler (char **argv, int argc){
    
    char buffer[BUFFER_SIZE];
    struct message message;

    // fILLS DEFAULT VALUES
    strcpy(input.ipaddr, "\n");
    strcpy(input.tport,"58000");
    strcpy(input.uport,"58000");
    strcpy(input.rs_id.ip,"193.136.138.142");
    strcpy(input.rs_id.port,"59000");
    input.tcpsessions = 1;
    input.bestpops = 1;
    input.tsecs = 5;
    input.display = true;
    input.debug = false;
    input.help = false;
    input.SF = false;
    input.format = true;

    if (argc == 1) {//if no flags detected
        display_help();
        print_input();
        strcpy(buffer, "DUMP\n");
        udp_client(0, buffer, input.rs_id);
        udp_decoder(buffer, &message);
        char *token = &buffer[0] + 8;
        printf("Streams:\n%s\n", token);
        exit(0);}

    // LOOKS FOR FLAGS
    for (int i = 1; i < argc; i++){
        if (strcasecmp(argv[i],"-i") == 0) strcpy(input.ipaddr,argv[++i]);   
        if (strcasecmp(argv[i],"-t") == 0) strcpy(input.tport,argv[++i]); 
        if (strcasecmp(argv[i],"-u") == 0) strcpy(input.uport,argv[++i]); 
        if (strcasecmp(argv[i],"-s") == 0) sscanf(argv[++i], "%[^:]%*[:]%s", input.rs_id.ip, input.rs_id.port);
        if (strcasecmp(argv[i],"-p") == 0) input.tcpsessions = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-n") == 0) input.bestpops = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-x") == 0) input.tsecs = atoi(argv[++i]);
        if (strcasecmp(argv[i],"-b") == 0) input.display = false;
        if (strcasecmp(argv[i],"-d") == 0) input.debug = true;
        if (strcasecmp(argv[i],"-h") == 0) input.help = true;
    } 


    if (input.help == true){display_help(); exit(0);}

    //checks for the only mandatory flag content
    if (strcasecmp(input.ipaddr, "\n") == 0){printf("Must specify application IP\n");display_help();exit(0);}

    if (sscanf(argv[1], "%[^:]%*[:]%[^:]%*[:]%s", input.stream_id.name, input.stream_id.ip, input.stream_id.port) != 3){
        printf("You must provide a stream ID\n");
        display_help();exit(0);
    }
    return;
}

void display_help (){
    printf("./iamroot [<streamID>] [-i <ipaddr>] [-t <tport>] [-u <uport>]\n"); 
    printf("\t\t       [-s <rsaddr>[:<rsport>]]\n");
    printf("\t\t       [-p <tcpsessions>]\n");
    printf("\t\t       [-n <bestpops>] [-x <tsecs>]\n");
    printf("\t\t       [-b] [-d] [-h]\n\n\n");
    return; 
}

void print_input(){
    printf("DEFAULT VALUES\n");
    printf("TCP and UDP ports: %s - %s\n", input.tport, input.uport);
    printf("Root Server: %s:%s\n", input.rs_id.ip, input.rs_id.port);
    printf("Application variables: %d (sessions) - %d (bestp) - %d (tsecs)\n", input.tcpsessions, input.bestpops, input.tsecs);
    printf("Flags: %d %d %d\n\n\n", input.display, input.debug, input.help);
    return;
}