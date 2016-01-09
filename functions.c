#include "header.h"
read_file(char *file)
{
    FILE *fp;
    fp = fopen(file, "r");
    char * line = NULL;
    size_t len = 0;
    //ssize_t read;
    int i, read;

    if (fp==NULL)
    {
        perror ("ERROR: File does not exist");
    }
    else
    {
        int lineno=0;
        int num_chars = 0;
        while ((read = getline(&line, &len, fp)) != -1)
        {
            lineno++;
            if(lineno==2)
            {
                self.totalNeighbours = atoi(line);
            }
            if(lineno>=3 && lineno<=(3+MAX_SERVERS-1))
            {
                char* t;
                t = strtok(line," ");
                char *t1 = t;
                t = strtok('\0'," ");
                char *t2 = t;
                t = strtok('\0'," ");
                char *t3 = t;
                int id = atoi(t1);

                routingtable[id-1].serverID = id;
                routingtable[id-1].nextHop = 0;
                strcpy(routingtable[id-1].serverIP, t2);
                strcpy(routingtable[id-1].serverPort, t3);
                routingtable[id-1].serverCost = INF;
                routingtable[id-1].initCost = INF;
                routingtable[id-1].type = 2;
                routingtable[id-1].failedMsgs = 0;

                if(strcmp(self.myIP,t2)==0)
                {
                    routingtable[id-1].serverCost = 0;
                    routingtable[id-1].initCost = 0;
                    routingtable[id-1].type = 0;
                    routingtable[id-1].nextHop = id;
                    self.myID = id;
                    strcpy(self.myPort, t3);
                    self.totalNeighbours = 0;
                }
            }
            if(lineno>(3+MAX_SERVERS-1))
            {
                char* t;
                t = strtok(line," ");
                char *t1 = t;
                t = strtok('\0'," ");
                char *t2 = t;
                t = strtok('\0'," ");
                char *t3 = t;
                int id = atoi(t2);

                routingtable[id-1].serverCost = atoi(t3);
                routingtable[id-1].initCost = atoi(t3);
                routingtable[id-1].type = 1;
                routingtable[id-1].nextHop = id;
                self.totalNeighbours++;
            }
        }
    }
    fclose(fp);
    return(0);
}
update(char *from, char *to, char *cost, char *direction)
{
    if(self.myID==atoi(from))
    {
        int found = 0;
        if(routingtable[atoi(to)-1].type==1)
        {
            char buff2[10] = "update|-|";
            strncat(buff2,to,10);
            char buff3[5] = "~";
            strncat(buff2,buff3,15);
            strncat(buff2,from,15);
            strncat(buff2,buff3,15);
            strncat(buff2,cost,20);

            if(strcmp(cost,"inf")==0)
            {
                if(routingtable[atoi(to)-1].serverCost != INF){
                    routingtable[atoi(to)-1].serverCost = INF;
                    if(strcmp(direction,"f")==0){
                        if(sendto(masterSock_fd,buff2,MAX_DATA,0,(struct sockaddr *)&server_addr[atoi(to)-1],sizeof(server_addr[atoi(to)-1])))
                        {
                            //puts("msg sent");
                        }
                    }
                    int j;
                    for(j=0; j<MAX_SERVERS; j++){
                        if(routingtable[j].type == 2 && routingtable[j].nextHop == routingtable[atoi(to)-1].serverID){
                            routingtable[j].serverCost = INF;
                            routingtable[j].nextHop = 0;
                        }
                        else if(routingtable[j].type == 1 && routingtable[j].nextHop == routingtable[atoi(to)-1].serverID && routingtable[j].serverID != routingtable[atoi(to)-1].serverID){
                            routingtable[j].serverCost = routingtable[j].initCost;
                            routingtable[j].nextHop = routingtable[j].serverID;
                        }
                    }
                }
            }
            else if (atoi(cost)<INF)
            {
                if(routingtable[atoi(to)-1].serverCost != atoi(cost)){
                    routingtable[atoi(to)-1].serverCost = atoi(cost);
                    if(strcmp(direction,"f")==0){
                        if(sendto(masterSock_fd,buff2,MAX_DATA,0,(struct sockaddr *)&server_addr[atoi(to)-1],sizeof(server_addr[atoi(to)-1])))
                        {
                            //puts("msg sent");
                        }
                    }
                }
            }
            else
            {
                printf("update ERROR: Given cost value is unacceptable.\n");
                return 0;
            }
            found = 1;
            printf("update SUCCESS\n");
        }
        if(found==0)
        {
            printf("update ERROR: Server-2 ID is not correct or it is not a neighbor.\n");
        }
    }
    else
    {
        printf("update ERROR: Server-1 ID is not correct.\n");
    }
}
step()
{
    int i, count=0;
    char buff1[50];
    snprintf(buff1, 50, "%d", self.myID);
    strncat(buff1,"HH",50);
    char buff2[5] = "&&";
    char buff3[5] = "~";

    for(i=0; i<MAX_SERVERS; i++)
    {
        if(routingtable[i].type>0)
        {
            count++;
            char id[2];
            snprintf(id, 2, "%d", routingtable[i].serverID);
            char cost[4];
            snprintf(cost, 4, "%d", routingtable[i].serverCost);

            if(count>1){
                strncat(buff1,buff2,50);
            }
            strncat(buff1,id,50);
            strncat(buff1,buff3,50);
            strncat(buff1,cost,50);
        }
    }
    char nofield[2];
    snprintf(nofield, 2, "%d", count);
    char message[50];
    strcpy(message, "routing|-|");
    strncat(message,nofield,50);
    strncat(message,buff3,50);
    strncat(message,buff1,50);
    //puts(message);
    int error = 0;
    for(i=0; i<MAX_SERVERS; i++)
    {
        if(routingtable[i].type==1 && routingtable[i].serverCost<INF && self.myState==1)
        {
            if(sendto(masterSock_fd,message,MAX_DATA,0,(struct sockaddr *)&server_addr[i],sizeof(server_addr[i]))){

            }else{
                error++;
            }
        }
    }
    if(error==0){
        return 1;
    }else{
        return 0;
    }
}
read_dv(char *dist_vect[100])
{
    //puts(dist_vect);
    char* t;
    t = strtok(dist_vect,"HH");
    char *t1 = t;
    t = strtok('\0',"HH");
    char *t2 = t;

    char* sender;
    sender = strtok(t1,"~");
    char *nofield = sender;
    sender = strtok('\0',"~");
    char *senderid = sender;
    int senderCost = routingtable[atoi(senderid)-1].serverCost;
    printf("RECEIVED A MESSAGE FROM SERVER %d\n",atoi(senderid));

    int i; char *str = t2;
    if(atoi(nofield)>0)
    {
        char* tok;
        tok = strtok(str,"&&");
        char *tok1 = tok;

        char *servers[5];
        servers[0] = tok1;
        for(i=2; i<=atoi(nofield); i++)
        {
            tok = strtok('\0',"&&");
            char *tok2 = tok;
            servers[i-1] = tok2;
        }

        for(i=0; i<atoi(nofield); i++)
        {
            char* server;
            server = strtok(servers[i],"~");
            char *serverid = server;
            server = strtok('\0',"~");
            char *servercost = server;

            //Bellman-Ford Algo.
            int pathcost = senderCost + atoi(servercost);
            if((pathcost < routingtable[atoi(serverid)-1].serverCost))
            {
                routingtable[atoi(serverid)-1].serverCost = pathcost;
                routingtable[atoi(serverid)-1].nextHop = atoi(senderid);
            }
            if(pathcost > routingtable[atoi(serverid)-1].serverCost && routingtable[atoi(serverid)-1].nextHop == atoi(senderid))
            {
                if(senderCost==INF || routingtable[atoi(senderid)-1].type==3 || atoi(servercost)==INF){
                    if(routingtable[atoi(serverid)-1].type==2){
                        routingtable[atoi(serverid)-1].serverCost = INF;
                        routingtable[atoi(serverid)-1].nextHop = 0;
                    }
                    else if(routingtable[atoi(serverid)-1].type=1){
                        routingtable[atoi(serverid)-1].serverCost = routingtable[atoi(serverid)-1].initCost;
                        routingtable[atoi(serverid)-1].nextHop = routingtable[atoi(serverid)-1].serverID;
                    }
                    else{
                        routingtable[atoi(serverid)-1].serverCost = INF;
                        routingtable[atoi(serverid)-1].nextHop = 0;
                    }
                }
                else{
                    if(routingtable[atoi(serverid)-1].type=1 && pathcost > routingtable[atoi(serverid)-1].initCost){
                        routingtable[atoi(serverid)-1].serverCost = routingtable[atoi(serverid)-1].initCost;
                        routingtable[atoi(serverid)-1].nextHop = routingtable[atoi(serverid)-1].serverID;
                    }
                    else if(routingtable[atoi(serverid)-1].type=2){
                        routingtable[atoi(serverid)-1].serverCost = pathcost;
                        routingtable[atoi(serverid)-1].nextHop = atoi(senderid);
                    }
                    else{
                        routingtable[atoi(serverid)-1].serverCost = INF;
                        routingtable[atoi(serverid)-1].nextHop = 0;
                    }
                }
            }
        }
    }
}
display()
{
    printf("\n");
    int i;
    for(i=0; i<MAX_SERVERS; i++)
    {
        if(routingtable[i].serverCost<99)
        {
            printf("%d %d %d\n", routingtable[i].serverID, routingtable[i].nextHop, routingtable[i].serverCost);
        }
    }
    return 1;
}
/*
display()
{
    printf("\n");
    printf("%-10s %-10s %-10s  %-10s\n", "ID", "Next Hop","Cost", "Type");
    printf("\n");

    int i;
    for(i=0; i<MAX_SERVERS; i++)
    {
        if(routingtable[i].serverCost<99)
        {
            printf("%-10d %-10d %-10d  %-10d\n", routingtable[i].serverID, routingtable[i].nextHop, routingtable[i].serverCost, routingtable[i].type);
        }
    }
    return 1;
}
display_self()
{
    printf("\n");
    printf("%-4s %-10s %-20s  %-10s\n", "id:","Total N", "IP Address", "Port");
    printf("\n");
    printf("%-4d %-10d %-20s  %-10s\n", self.myID, self.totalNeighbours, self.myIP, self.myPort);
    printf("\n");
    return 1;
}
*/
