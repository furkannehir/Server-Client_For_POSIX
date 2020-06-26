
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
    int cfd;
    ssize_t numRead;
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_INET; /* Allows IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
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
            break;
        close(cfd);
    }
    if (rp == NULL)
        printf("Could not connect socket to any address\n");
    freeaddrinfo(result);
    return 0;
}
