#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/socket.h>
#include<sys/types.h> 
#include<netdb.h> 
#include<netinet/in.h> 
#include<fcntl.h>
#include<sys/file.h>
#include<time.h>
#include"InputOutput.h"
#include"graph.h"
#include"requestQueue.h"
#include"hashmap.h"
#include<sys/signal.h>
#define MAX 80
#define BACKLOG 50
#define INT_LEN 300
#define MYFILENAME "My Perfect File For Provide Duplicates"

void * CreateConnection(void * opt);
void CreateAddrInfo(struct addrinfo * addr);
int GetSocketFileDescriptor();
int CloseSTD();
void * BreadthFirstSearchRequestReaderPrio(void * opt);
void * BreadthFirstSearchRequestWriterPrio(void * opt);
void * BreadthFirstSearchRequestNoPrio(void * opt);
double SystemLoad(int * emptyThreads, int size);
int FindAnEmptyThread(int * emptyThreads, int size);
void * SystemTrackThread(void * opt);
void handler(int signo);


char pathToFile[500];
char pathToLogFile[500];
int port, threadPoolSize, maxPoolSize, fdOut, prio, myFD;
char portString[500];
Graph graph;
hashmap cache;
int readerCount = 0;
pthread_mutex_t readMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t tPoolMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t theresSpaceMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t * emptyThreadMutex;
pthread_mutex_t * signalMutex; 
pthread_cond_t * signalLock;
pthread_cond_t * emptyThreadLock;
pthread_cond_t theresSpaceLock = PTHREAD_COND_INITIALIZER;
pthread_t mainThreadID, systemTrackID;
pthread_t * threadPool;
int * emptyThreads;
Queue requests;
char * pathString;

int main(int argc, char ** argv)
{
    setbuf(stdout, NULL);
    myFD = open(MYFILENAME, O_CREAT | O_EXCL);
    if(myFD == -1)
    {
        printf("I can't be duplicated!\n");
        exit(EXIT_FAILURE);
    }
    GetArgumentsServer(argc, argv, pathToFile, &port, portString, pathToLogFile, &threadPoolSize, &maxPoolSize, &prio);
    printf("Executing with parameters:\n-i %s\n-p %s\n-o %s\n-s %d\n-x %d\n", pathToLogFile, portString, pathToLogFile, threadPoolSize, maxPoolSize);
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
    pid = fork();
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
    //printf("pid: %d\n", getpid());
    chdir("/");
    fdOut = CloseSTD();
    char timestamp[500];
    printf("%s: Loading graph...\n", GetTimestamp(timestamp));
    int begin = clock();
    int size = GetMaxNodeID(pathToFile);
    CreateGraph(&graph, size);
    cache = CreateHashmap();
    if(!FillTheGraph(&graph, pathToFile))
        return -1;
    int end = clock();
    double loadTime = end - begin;
    loadTime = loadTime / 1000000;
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    printf("%s: Graph loaded in %.2f seconds with %d nodes and %d edges.\n", GetTimestamp(timestamp), loadTime, size, graph.edgeCount);
    // PrintGraph(graph);
    requests = CreateQueue();
    emptyThreads = (int*)calloc(maxPoolSize, sizeof(int));
    emptyThreadMutex = (pthread_mutex_t*)calloc(maxPoolSize, sizeof(pthread_mutex_t));
    emptyThreadLock = (pthread_cond_t*)calloc(maxPoolSize, sizeof(pthread_cond_t));
    signalMutex = (pthread_mutex_t*)calloc(maxPoolSize, sizeof(pthread_mutex_t));
    signalLock = (pthread_cond_t*)calloc(maxPoolSize, sizeof(pthread_cond_t));
    int i, arr[maxPoolSize];
    for(i = 0; i < maxPoolSize; ++i)
    {
        pthread_mutex_init(&emptyThreadMutex[i], NULL);
        pthread_cond_init(&emptyThreadLock[i], NULL);
        pthread_mutex_init(&signalMutex[i], NULL);
        pthread_cond_init(&signalLock[i], NULL);
    }
    threadPool = (pthread_t*)calloc(maxPoolSize, sizeof(pthread_t));
    for(i = 0; i <maxPoolSize; ++i)
        arr[i] = i; 
    pthread_create(&mainThreadID, NULL, CreateConnection, NULL);
    pthread_create(&systemTrackID, NULL, SystemTrackThread, NULL);
    for(i = 0; i < threadPoolSize; ++i)
    {
        if(prio == 0)
            pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestReaderPrio, &arr[i]);
        if(prio == 1)
            pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestWriterPrio, &arr[i]);
        if(prio == 2)
            pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestNoPrio, &arr[i]);
    }
    printf("%s: A pool of %d threads has been created\n", GetTimestamp(timestamp), threadPoolSize);
    pthread_join(mainThreadID, NULL);
    for(i = 0; i < threadPoolSize; ++i)
    {
        pthread_join(threadPool[i], NULL);
    }
    pthread_join(systemTrackID, NULL);
    free(emptyThreads);
    free(threadPool);
    for(i = 0; i < maxPoolSize; ++i)
    {
        pthread_mutex_destroy(&emptyThreadMutex[i]);
        pthread_cond_destroy(&emptyThreadLock[i]);
        pthread_mutex_destroy(&signalMutex[i]);
        pthread_cond_destroy(&signalLock[i]);
    }
// emptyThreadMutex
    free(emptyThreadMutex);
// emptyThreadLock 
    free(emptyThreadLock);
// signalMutex
    free(signalMutex);
// signalLock
    free(signalLock);
    DestroyGraph(&graph);
    DestroyHashmap(&cache);
    CloseFile("stdout", fdOut);
    CloseFile("lock", myFD);
    unlink(MYFILENAME);
}


void * CreateConnection(void * opt)
{
    struct sockaddr_storage claddr;
    int cfd;
    socklen_t addrlen;
    #define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    int socketID, t_id;
    socketID = GetSocketFileDescriptor();
    for(;;)
    {
        addrlen = sizeof(struct sockaddr_storage);
        cfd = accept(socketID, (struct sockaddr *) &claddr, &addrlen);
        pthread_mutex_lock(&tPoolMutex);
        t_id = FindAnEmptyThread(emptyThreads, threadPoolSize);
        emptyThreads[t_id] = cfd;
        pthread_mutex_lock(&emptyThreadMutex[t_id]);
        pthread_cond_signal(&emptyThreadLock[t_id]);
        pthread_mutex_unlock(&emptyThreadMutex[t_id]);
        pthread_mutex_unlock(&tPoolMutex);
    }
    close(socketID);
    return NULL;
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
        printf("Error on listen: %s\n", strerror(errno));
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

void * BreadthFirstSearchRequestReaderPrio(void * opt)
{
    int * t_idAddr = (int*)opt;
    int t_id = *t_idAddr;
    int cfd;
    Queue path = CreateQueue();
    char timestamp[500], reqLenStr[INT_LEN];
    hashNode hNode;
    while(1)
    {
        printf("%s: Thread #%d: waiting for connection\n", GetTimestamp(timestamp), t_id);
        pthread_mutex_lock(&emptyThreadMutex[t_id]);
        //SIGNAL STUFF
        pthread_mutex_lock(&signalMutex[t_id]);
        pthread_cond_signal(&signalLock[t_id]);
        pthread_mutex_unlock(&signalMutex[t_id]);
        pthread_mutex_unlock(&tPoolMutex);
        //ENDSIGNALSTUFF
        pthread_cond_wait(&emptyThreadLock[t_id], &emptyThreadMutex[t_id]);
        pthread_mutex_lock(&tPoolMutex);
        cfd = emptyThreads[t_id];
        pthread_mutex_unlock(&tPoolMutex);
        pthread_mutex_unlock(&emptyThreadMutex[t_id]);
        if(cfd > 0)
        {
            printf("%s: A connection has been delegated to thread id #%d, system load %.2f%%\n", GetTimestamp(timestamp), t_id, SystemLoad(emptyThreads, threadPoolSize));
            ReadFile("pipe", cfd, reqLenStr, INT_LEN);
            int start, destination;
            sscanf(reqLenStr, "%d %d", &start, &destination);
            pthread_mutex_lock(&readMutex);
            hNode = GetItemHashmap(&cache, reqLenStr);
            pthread_mutex_unlock(&readMutex);
            if(strcmp(hNode.key, ""))
            {
                WriteFile("pipe", cfd, hNode.value, strlen(hNode.value));
            }
            else
            {
                path = CreateQueue();
                BreadthFirstSearch(&graph, start, destination, &path);
                pathString = (char*)calloc(5000, sizeof(char));
                QueueString(path, pathString);
                pthread_mutex_lock(&readMutex);
                pthread_mutex_unlock(&readMutex);
                InsertHashmap(&cache, reqLenStr, pathString);
                WriteFile("pipe", cfd, pathString, strlen(pathString));
                DestroyQueue(&path);
                free(pathString);
            }
            close(cfd);
        }
        pthread_mutex_lock(&tPoolMutex);
        double load = SystemLoad(emptyThreads, threadPoolSize);
        pthread_mutex_unlock(&tPoolMutex);
        if(load == 100.0)
        {
            pthread_mutex_lock(&theresSpaceMutex);
            pthread_cond_signal(&theresSpaceLock);
            pthread_mutex_unlock(&theresSpaceMutex);
        }
        pthread_mutex_lock(&tPoolMutex);
        emptyThreads[t_id] = 0;
    }
    return NULL;
}

void * BreadthFirstSearchRequestWriterPrio(void * opt)
{
    int * t_idAddr = (int*)opt;
    int t_id = *t_idAddr;
    int cfd;
    Queue path = CreateQueue();
    char timestamp[500], reqLenStr[INT_LEN];
    hashNode hNode;
    while(1)
    {
        printf("%s: Thread #%d: waiting for connection\n", GetTimestamp(timestamp), t_id);
        pthread_mutex_lock(&emptyThreadMutex[t_id]);
        //SIGNAL STUFF
        pthread_mutex_lock(&signalMutex[t_id]);
        pthread_cond_signal(&signalLock[t_id]);
        pthread_mutex_unlock(&signalMutex[t_id]);
        pthread_mutex_unlock(&tPoolMutex);
        //ENDSIGNALSTUFF
        pthread_cond_wait(&emptyThreadLock[t_id], &emptyThreadMutex[t_id]);
        pthread_mutex_lock(&tPoolMutex);
        cfd = emptyThreads[t_id];
        pthread_mutex_unlock(&tPoolMutex);
        pthread_mutex_unlock(&emptyThreadMutex[t_id]);
        if(cfd > 0)
        {
            printf("%s: A connection has been delegated to thread id #%d, system load %.2f%%\n", GetTimestamp(timestamp), t_id, SystemLoad(emptyThreads, threadPoolSize));
            ReadFile("pipe", cfd, reqLenStr, INT_LEN);
            int start, destination;
            sscanf(reqLenStr, "%d %d", &start, &destination);
            pthread_mutex_lock(&writeMutex);
            pthread_mutex_unlock(&writeMutex);
            hNode = GetItemHashmap(&cache, reqLenStr);
            if(strcmp(hNode.key, ""))
            {
                WriteFile("pipe", cfd, hNode.value, strlen(hNode.value));
            }
            else
            {
                path = CreateQueue();
                BreadthFirstSearch(&graph, start, destination, &path);
                pathString = (char*)calloc(5000, sizeof(char));
                QueueString(path, pathString);
                pthread_mutex_lock(&writeMutex);
                InsertHashmap(&cache, reqLenStr, pathString);
                pthread_mutex_unlock(&writeMutex);
                WriteFile("pipe", cfd, pathString, strlen(pathString));
                DestroyQueue(&path);
                free(pathString);
            }
            close(cfd);
        }
        pthread_mutex_lock(&tPoolMutex);
        double load = SystemLoad(emptyThreads, threadPoolSize);
        pthread_mutex_unlock(&tPoolMutex);
        if(load == 100.0)
        {
            pthread_mutex_lock(&theresSpaceMutex);
            pthread_cond_signal(&theresSpaceLock);
            pthread_mutex_unlock(&theresSpaceMutex);
        }
        pthread_mutex_lock(&tPoolMutex);
        emptyThreads[t_id] = 0;
        pthread_mutex_unlock(&tPoolMutex);
    }
    return NULL;
}

void * BreadthFirstSearchRequestNoPrio(void * opt)
{
    int * t_idAddr = (int*)opt;
    int t_id = *t_idAddr;
    int cfd;
    Queue path = CreateQueue();
    char timestamp[500], reqLenStr[INT_LEN];
    hashNode hNode;
    while(1)
    {
        printf("%s: Thread #%d: waiting for connection\n", GetTimestamp(timestamp), t_id);
        pthread_mutex_lock(&emptyThreadMutex[t_id]);
        //SIGNAL STUFF
        pthread_mutex_lock(&signalMutex[t_id]);
        pthread_cond_signal(&signalLock[t_id]);
        pthread_mutex_unlock(&signalMutex[t_id]);
        pthread_mutex_unlock(&tPoolMutex);
        //ENDSIGNALSTUFF
        pthread_cond_wait(&emptyThreadLock[t_id], &emptyThreadMutex[t_id]);
        pthread_mutex_lock(&tPoolMutex);
        cfd = emptyThreads[t_id];
        pthread_mutex_unlock(&tPoolMutex);
        pthread_mutex_unlock(&emptyThreadMutex[t_id]);
        if(cfd > 0)
        {
            printf("%s: A connection has been delegated to thread id #%d, system load %.2f%%\n", GetTimestamp(timestamp), t_id, SystemLoad(emptyThreads, threadPoolSize));
            ReadFile("pipe", cfd, reqLenStr, INT_LEN);
            int start, destination;
            sscanf(reqLenStr, "%d %d", &start, &destination);
            hNode = GetItemHashmap(&cache, reqLenStr);
            if(strcmp(hNode.key, ""))
            {
                WriteFile("pipe", cfd, hNode.value, strlen(hNode.value));
            }
            else
            {
                path = CreateQueue();
                BreadthFirstSearch(&graph, start, destination, &path);
                pathString = (char*)calloc(5000, sizeof(char));
                QueueString(path, pathString);
                InsertHashmap(&cache, reqLenStr, pathString);
                WriteFile("pipe", cfd, pathString, strlen(pathString));
                DestroyQueue(&path);
                free(pathString);
            }
            close(cfd);
        }
        pthread_mutex_lock(&tPoolMutex);
        double load = SystemLoad(emptyThreads, threadPoolSize);
        pthread_mutex_unlock(&tPoolMutex);
        if(load == 100.0)
        {
            pthread_mutex_lock(&theresSpaceMutex);
            pthread_cond_signal(&theresSpaceLock);
            pthread_mutex_unlock(&theresSpaceMutex);
        }
        pthread_mutex_lock(&tPoolMutex);
        emptyThreads[t_id] = 0;
        pthread_mutex_unlock(&tPoolMutex);
    }
    return NULL;
}


double SystemLoad(int * emptyThreads, int size)
{
    int i, busy = 0;
    for(i = 0; i < size; ++i)
    {
        if(emptyThreads[i])
            ++busy;
    }
    double load = (double)busy;
    load = load/size*100;
    return load;
}

int FindAnEmptyThread(int * emptyThreads, int size)
{
    int i;
    double load = SystemLoad(emptyThreads, threadPoolSize);
    if(load == 100.0)
    {
            pthread_mutex_lock(&theresSpaceMutex);
            pthread_cond_wait(&theresSpaceLock, &theresSpaceMutex);
            pthread_mutex_unlock(&theresSpaceMutex);
    }
    for(i = 0; i < size; ++i)
    {
        if(emptyThreads[i] == 0)
            return i;
    }
    return -1;
}

void * SystemTrackThread(void * opt)
{
    int i, threadPoolSizeAug;
    double load;
    int arr[maxPoolSize];
    char timestamp[500];
    for(i = 0; i < maxPoolSize; ++i)
        arr[i] = i;
    for(;;)
    {
        pthread_mutex_lock(&tPoolMutex);
        load = SystemLoad(emptyThreads, threadPoolSize);
        pthread_mutex_unlock(&tPoolMutex);
        if(load >= 75.0 && threadPoolSize < maxPoolSize)
        {
            threadPoolSizeAug = threadPoolSize * 1.25;
            if(threadPoolSizeAug > maxPoolSize)
                threadPoolSizeAug = maxPoolSize;
            for(i = threadPoolSize; i < threadPoolSizeAug; ++i)
            {
                if(prio == 0)
                    pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestReaderPrio, &arr[i]);
                if(prio == 1)
                    pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestWriterPrio, &arr[i]);
                if(prio == 2)
                    pthread_create(&threadPool[i], NULL, BreadthFirstSearchRequestNoPrio, &arr[i]);
            }
            threadPoolSize = threadPoolSizeAug;
            printf("%s: System load %.2f%%, pool extended to %d threads\n", GetTimestamp(timestamp),load, threadPoolSize);
        }
    }
}

void handler(int signo)
{
    char timestamp[500];
    switch (signo)
    {
        case SIGINT:
            pthread_cancel(mainThreadID);
            pthread_cancel(systemTrackID);
            int i;
            for(i = 0; i < threadPoolSize; ++i)
            {
                pthread_mutex_lock(&signalMutex[i]);
                if(emptyThreads[i])
                {
                    pthread_cond_wait(&signalLock[i], &signalMutex[i]);
                }
                pthread_cancel(threadPool[i]);
                pthread_mutex_unlock(&signalMutex[i]);
            }
        // emptyThreads
            free(emptyThreads);
            free(threadPool);
            for(i = 0; i < maxPoolSize; ++i)
            {
                pthread_mutex_destroy(&emptyThreadMutex[i]);
                pthread_cond_destroy(&emptyThreadLock[i]);
                pthread_mutex_destroy(&signalMutex[i]);
                pthread_cond_destroy(&signalLock[i]);
            }
        // emptyThreadMutex
            free(emptyThreadMutex);
        // emptyThreadLock 
            free(emptyThreadLock);
        // signalMutex
            free(signalMutex);
        // signalLock
            free(signalLock);
            DestroyGraph(&graph);
            DestroyHashmap(&cache);
            printf("%s: Now I can rest.\n", GetTimestamp(timestamp));
            CloseFile("stdout", fdOut);
            CloseFile("lock", myFD);
            unlink(MYFILENAME);
            exit(EXIT_FAILURE);
    }
}