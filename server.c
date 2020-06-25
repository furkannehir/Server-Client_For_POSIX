#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"become_daemon.h"
#include"InputOutput.h"
#include"graph.h"

char pathToFile[500];
char pathToLogFile[500];
int port;

int main(int argc, char ** argv)
{
    GetArgumentsServer(argc, argv, pathToFile, &port, pathToLogFile);
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
    char str[500];
    int size = GetMaxNodeID(pathToFile);
    printf("%d\n", size);
    Graph graph;
    CreateGraph(&graph, size);
    FillTheGraph(&graph, pathToFile);
    int i,j;
    for(i = 0; i < graph.size; ++i)
    {
        for(j = 0; j < graph.size; ++j)
            printf("%d ", graph.AdjList[i][j]);
        printf("\n");
    }
    WriteFile("output", fdOut, str, strlen(str));
    return 0;
}