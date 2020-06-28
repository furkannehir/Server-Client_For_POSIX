
#include<unistd.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h> 
#include<netdb.h> 
#include<netinet/in.h> 
#include<fcntl.h>
#include<time.h>
#include<stdio.h>
#include"InputOutput.h"

char IP[500];
int port, sourceNode, destNode;
char portString[500];

int main(int argc, char ** argv) 
{ 
    GetArgumentsClient(argc, argv, IP, &port, portString, &sourceNode, &destNode);
    int cfd, pid;
    char path[5000];
    char sourceDestString[100];
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_INET; /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    pid = getpid();
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    double begin = tp.tv_nsec/1000000000.0;
    begin += tp.tv_sec;
    char timestamp[500];
    printf("%s: Client (%d) connecting to %s:%s\n", GetTimestamp(timestamp), pid, IP, portString);
    if (getaddrinfo(IP, portString, &hints, &result) != 0)
    {
        printf("Error on getaddrinfo\n");
        exit(EXIT_FAILURE);
    }
    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {
        cfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (cfd == -1)
            continue;
        if (connect(cfd, rp->ai_addr, rp->ai_addrlen) != -1)
        {
            break;
        }
        close(cfd);
    }
    if (rp == NULL)
        printf("%s: Could not connect socket to any address\n", GetTimestamp(timestamp));
    freeaddrinfo(result);
    sprintf(sourceDestString, "%d %d", sourceNode, destNode);
    WriteFile("pipe", cfd, sourceDestString, 100);
    printf("%s: Client (%d) connected and requesting a path from node %d to %d\n", GetTimestamp(timestamp), pid, sourceNode, destNode);
    ReadOneLine("pipe", cfd, path);
    CloseFile("pipe",cfd);
    clock_gettime(CLOCK_REALTIME, &tp);
    double end = tp.tv_nsec/1000000000.0;
    end += tp.tv_sec;
    double loadTime = end - begin;
    printf("%s: Server’s response to (%d): %s, arrived in %.3f seconds.\n", GetTimestamp(timestamp), pid, path, loadTime);
    return 0;
}
