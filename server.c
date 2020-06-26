#include<unistd.h>
#include<pthread.h>
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
#define INT_LEN 3000

// TODO Threads
// TODO Thread Pool -> array + mutex
// TODO Cache -> Hashmap + mutex
// TODO Reader-Writer -r

int CreateConnection();
void CreateAddrInfo(struct addrinfo * addr);
int GetSocketFileDescriptor();
int CloseSTD();


char pathToFile[500];
char pathToLogFile[500];
int port, poolThread, maxPool;
char portString[500];
Graph graph;
int * threadPool;
int * emptyThreads;
pthread_mutex_t tPoolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tPoolLock = PTHREAD_COND_INITIALIZER;

int main(int argc, char ** argv)
{
    setbuf(stdout, NULL);
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
    int fdOut = CloseSTD();
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
    // PrintGraph(graph);
    CreateConnection();
    DestroyGraph(&graph);
    CloseFile("stdout", fdOut);
}


int CreateConnection()
{
    struct sockaddr_storage claddr;
    int cfd;
    socklen_t addrlen;
    char reqLenStr[INT_LEN];
    #define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    char addrStr[ADDRSTRLEN];
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];
    int socketID;
    socketID = GetSocketFileDescriptor();
    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(socketID, (struct sockaddr *) &claddr, &addrlen);
    if (cfd == -1) {
        printf("Error in accept!\n");
    }
    else
    {
        printf("Connection established\n");
    }
    if (getnameinfo((struct sockaddr *) &claddr, addrlen, host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
        snprintf(addrStr, ADDRSTRLEN, "(%s, %s)", host, service);
    else
        snprintf(addrStr, ADDRSTRLEN, "(?UNKNOWN?)");
    printf("Connection from %s\n", addrStr);
    ReadFile("pipe", cfd, reqLenStr, INT_LEN);
    printf("%s\n", reqLenStr);
    int start, destination;
    sscanf(reqLenStr, "%d %d", &start, &destination);
    Queue path;
    path = CreateQueue();
    BreadthFirstSearch(&graph, start, destination, &path);
    char pathString[5000];
    QueueString(path, pathString);
    WriteFile("pipe", cfd, pathString, strlen(pathString));
    close(cfd);
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

int CloseSTD()
{
    CloseFile("stdin", STDIN_FILENO);
    CloseFile("stdout", STDOUT_FILENO);
    CloseFile("stderr", STDERR_FILENO);
    int fdOut = OpenFile(pathToLogFile, WR);
    if (fdOut != STDIN_FILENO) /* 'fd' should be 0 */
        return -1;
    if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
        return -1; 
     if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
        return -1;
    return fdOut; 
}