#include "header.h"
buildSocket(char *topology_file, char *update_interval)
{
    int i, j, msgBytes, sel;
    char data[MAX_DATA], command[MAX_DATA];
    int data_len, addr_size, packet_no=0;

    //create UDP socket
    if((masterSock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == ERROR)
    {
        perror("ERROR: Creating Socket");
        exit(ERROR);
    }

    struct hostent *he;
    char hostname[128];
    gethostname(hostname, sizeof(hostname));
    he = gethostbyname(hostname);
    strcpy(self.myIP, inet_ntoa(*(struct in_addr*)he->h_addr));
    self.myState = 1;
    read_file(topology_file);

    for(i=0; i<MAX_SERVERS; i++)
    {
        if(routingtable[i].type<2){
            server_addr[i].sin_family = AF_INET;
            server_addr[i].sin_port = htons(atoi(routingtable[i].serverPort));
            server_addr[i].sin_addr.s_addr = inet_addr(routingtable[i].serverIP);
            memset(server_addr[i].sin_zero, '\0', sizeof server_addr[i].sin_zero);

            if(routingtable[i].type==0){
                if((bind(masterSock_fd, (struct sockaddr *)&server_addr[i], sizeof(server_addr[i]))) == ERROR)
                {
                    perror("ERROR: Bind");
                    exit(ERROR);
                }
            }
        }
    }
    step();
    while(1)
    {
        FD_ZERO(&read_fdset);
        FD_SET(0, &read_fdset);
        FD_SET(masterSock_fd, &read_fdset);

        interval.tv_sec = atoi(update_interval);
        sel = select(masterSock_fd+1, &read_fdset, NULL, NULL, &interval);
        if (sel < 0){
            perror("ERROR: Select");
        }
        else if(sel == 0){
            step();
        }
        else{
            if(FD_ISSET(masterSock_fd, &read_fdset))
            {
                addr_size = sizeof ( struct sockaddr_in );
                msgBytes = recvfrom(masterSock_fd,data,MAX_DATA,0,(struct sockaddr *)&peer_addr,&addr_size);

                if(msgBytes==0){
                    perror("ERROR: Connection lost.");
                    break;
                }
                else if(msgBytes==-1){
                    perror("ERROR: Recv msg");
                    break;
                }
                else{
                    data[msgBytes] = '\0';
                    char* t;
                    t = strtok(data,"|-|");
                    char *t1 = t;
                    t = strtok('\0',"|-|");
                    char *t2 = t;

                    if(strcmp(t1,"update")==0)
                    {
                        char* tok;
                        tok = strtok(t2,"~");
                        char *tok1 = tok;
                        tok = strtok('\0',"~");
                        char *tok2 = tok;
                        tok = strtok('\0',"~");
                        char *tok3 = tok;

                        update(tok1, tok2, tok3, "b");
                    }
                    if(strcmp(t1,"routing")==0)
                    {
                        packet_no++;
                        read_dv(t2);
                    }
                    if(strcmp(t1,"disable")==0 || strcmp("crash",t1)==0)
                    {
                        if(routingtable[atoi(t2)-1].type==1){
                            routingtable[atoi(t2)-1].serverCost = INF;
                            routingtable[atoi(t2)-1].type = 3;
                            routingtable[atoi(t2)-1].nextHop = 0;

                            for(j=0; j<MAX_SERVERS; j++){
                                if(routingtable[j].type == 2 && routingtable[j].nextHop == routingtable[atoi(t2)-1].serverID){
                                    routingtable[j].serverCost = INF;
                                    routingtable[j].nextHop = 0;
                                }
                                else if(routingtable[j].type == 1 && routingtable[j].nextHop == routingtable[atoi(t2)-1].serverID && routingtable[j].serverID != routingtable[atoi(t2)-1].serverID){
                                    routingtable[j].serverCost = routingtable[j].initCost;
                                    routingtable[j].nextHop = routingtable[j].serverID;
                                }
                            }
                            if(strcmp("crash",t1)==0){
                                printf("Server %s crashed.\n",t2);
                            }else{
                                printf("Server %s disabled.\n",t2);
                            }
                            step();
                        }
                    }
                }
            }

            if(FD_ISSET(0, &read_fdset))
            {
                gets(command);
                char* t;
                t = strtok(command," ");
                char *t1 = t;
                t = strtok('\0'," ");
                char *t2 = t;
                t = strtok('\0'," ");
                char *t3 = t;
                t = strtok('\0'," ");
                char *t4 = t;

                if(self.myState == 1){
                    if(strcmp("display",command)==0)
                    {
                        printf("display SUCCESS\n");
                        display();
                    }
                    else if(strcmp("update",t1)==0)
                    {
                        update(t2, t3, t4, "f");
                    }
                    else if(strcmp("disable",t1)==0)
                    {
                        if(routingtable[atoi(t2)-1].type==1){
                            routingtable[atoi(t2)-1].serverCost = INF;
                            routingtable[atoi(t2)-1].type = 3;
                            routingtable[atoi(t2)-1].nextHop = 0;

                            for(j=0; j<MAX_SERVERS; j++){
                                if(routingtable[j].type == 2 && routingtable[j].nextHop == routingtable[atoi(t2)-1].serverID){
                                    routingtable[j].serverCost = INF;
                                    routingtable[j].nextHop = 0;
                                }
                                else if(routingtable[j].type == 1 && routingtable[j].nextHop == routingtable[atoi(t2)-1].serverID && routingtable[j].serverID != routingtable[atoi(t2)-1].serverID){
                                    routingtable[j].serverCost = routingtable[j].initCost;
                                    routingtable[j].nextHop = routingtable[j].serverID;
                                }
                            }
                            char buff2[15] = "disable|-|";
                            char selfID[2];
                            snprintf(selfID, 2, "%d", self.myID);
                            strncat(buff2,selfID,20);
                            sendto(masterSock_fd,buff2,MAX_DATA,0,(struct sockaddr *)&server_addr[atoi(t2)-1],sizeof(server_addr[atoi(t2)-1]));
                            printf("disable SUCCESS\n");
                            step();
                        }
                    }
                    else if(strcmp("crash",command)==0)
                    {
                        self.myState = 0;
                        for(i=0; i<MAX_SERVERS; i++){
                            if(routingtable[i].type>0){
                                routingtable[i].serverCost = INF;
                                routingtable[i].type = 3;
                                routingtable[i].nextHop = 0;

                                char buff2[15] = "crash|-|";
                                char selfID[2];
                                snprintf(selfID, 2, "%d", self.myID);
                                strncat(buff2,selfID,20);
                                sendto(masterSock_fd,buff2,MAX_DATA,0,(struct sockaddr *)&server_addr[i],sizeof(server_addr[i]));
                            }
                        }
                        step();
                    }
                    else if(strcmp("step",command)==0)
                    {
                        if(step()){
                            printf("step SUCCESS\n");
                        }else{
                            printf("step ERROR: Message was not sent to one or more server(s).\n");
                        }
                    }
                    else if(strcmp("packets",command)==0)
                    {
                        printf("packets SUCCESS\n");
                        printf("Number of packets received since last invocation = %d\n",packet_no);
                        packet_no = 0;
                    }
                    else{
                        printf("ERROR: Command not found.\n");
                    }
                }
                else{
                    printf("ERROR: This server is crashed.\n");
                }
            }
        }
    }
}
