#ifndef HEADER_H_INCLUDED
#define HEADER_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/unistd.h>
#include <time.h>

#define ERROR -1
#define MAX_DATA 1024
#define MIN_PORT 1024
#define MAX_SERVERS 5
#define INF 99

struct serverdetails
{
    int serverID;
    int serverCost;
    int nextHop; //SelfID if neighbor
    int initCost; //at program start
    int type; //0:self 1:neighbor 2:other 3:disabled/crashed
    int failedMsgs;
    char serverIP[MAX_DATA];
    char serverPort[4];
}routingtable[MAX_SERVERS];

struct mydetails
{
    int totalNeighbours;
    int myID;
    int myState; //1:active 0:crash
    char myIP[MAX_DATA];
    char myPort[4];
}self;

struct sockaddr_in server_addr[MAX_SERVERS], peer_addr;
int masterSock_fd;
fd_set read_fdset; //set of socket descriptors
struct timeval interval;
#endif // HEADER_H_INCLUDED



