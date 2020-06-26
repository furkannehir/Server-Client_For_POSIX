#include<unistd.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h> 
#include<netdb.h> 
#include<netinet/in.h> 
#include<fcntl.h>
#include<time.h>
#include"InputOutput.h"
#include"graph.h"
#define MAX 80
#define BACKLOG 50
#define INT_LEN 30


int CreateConnection();
void CreateAddrInfo(struct addrinfo * addr);
int GetSocketFileDescriptor();

char pathToFile[500];
char pathToLogFile[500];
int port, poolThread, maxPool;
char portString[500];
Graph graph;

int main(int argc, char ** argv)
{
    GetArgumentsServer(argc, argv, pathToFile, &port, portString, pathToLogFile, &poolThread, &maxPool);
    int pid = fork();
    switch (pid)
    {
        case 0:
            break;
        case -1:
            exit(EXIT_FAILURE);
        default:
            exit(EXIT_SUCCESS);
    }
    setsid();
    umask(0);
    //chdir("/");
    // CloseFile("stdin", STDIN_FILENO);
    // CloseFile("stdout", STDOUT_FILENO);
    // CloseFile("stderr", STDERR_FILENO);
    // int fdOut = OpenFile(pathToLogFile, WR);
    // if (fdOut != STDIN_FILENO) /* 'fd' should be 0 */
    //     return -1;
    // if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
    //     return -1;
    // if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
    //     return -1;
    printf("Loading graph...\n");
    int begin = clock();
    int size = GetMaxNodeID(pathToFile);
    CreateGraph(&graph, size);
    if(!FillTheGraph(&graph, pathToFile))
        return -1;
    int end = clock();
    double loadTime = end - begin;
    loadTime = loadTime / 1000000;
    printf("Graph loaded in %.2f seconds with %d nodes and %d edges.\n", loadTime, size, graph.edgeCount);
    PrintGraph(graph);
    //FOR PRINTING GRAPH
    // int i,j;
    // for(i = 0; i < graph.size; ++i)
    // {
    //     for(j = 0; j < graph.size; ++j)
    //         printf("%d ", graph.AdjList[i][j]);
    //     printf("\n");
    // }
    CreateConnection();
    DestroyGraph(&graph);
}


int CreateConnection()
{
    uint32_t seqNum;
    char reqLenStr[INT_LEN]; /* Length of requested sequence */
    char seqNumStr[INT_LEN]; /* Start of granted sequence */
    struct sockaddr_storage claddr;
    int cfd;
    socklen_t addrlen;
    int socketID = GetSocketFileDescriptor();
    printf("socketid: %d\n", socketID);
    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(socketID, (struct sockaddr *) &claddr, &addrlen);
    if (cfd == -1) {
        printf("Error in accept!\n");
    }
    else
    {
        printf("Connection established\n");
    }
    close(socketID); 
    return 0;
}

void CreateAddrInfo(struct addrinfo * addr)
{
    memset(addr, 0, sizeof(struct addrinfo));
    addr->ai_canonname = NULL;
    addr->ai_addr = NULL;
    addr->ai_next = NULL;
    addr->ai_socktype = SOCK_STREAM;
    addr->ai_family = AF_INET; /* Allows IPv4 or IPv6 */
    addr->ai_flags = AI_PASSIVE | AI_NUMERICSERV;
}

int GetSocketFileDescriptor()
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    int fileDesc;
    char addrStr[ADDRSTRLEN];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    CreateAddrInfo(&hints);
    if (getaddrinfo(NULL, portString, &hints, &result) != 0)
    {
        printf("Error in getaddrinfo\n");
        exit(EXIT_FAILURE);
    }
    int optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) 
    {
        fileDesc = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fileDesc == -1)
            continue;
        if (setsockopt(fileDesc, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
            {
                printf("Error in setsockopt\n");
                exit(EXIT_FAILURE);    
            }
        if (bind(fileDesc, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(fileDesc);
    }
    if (rp == NULL)
        printf("Could not bind socket to any address\n");
    if (listen(fileDesc, BACKLOG) == -1)
    {
        printf("listen");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    return fileDesc;
}