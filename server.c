#include<unistd.h>
#include"InputOutput.h"

char pathToFile[500];
char pathToLogFile[500];
int port;

int main(int argc, char ** argv)
{
    GetArgumentsServer(argc, argv, pathToFile, &port, pathToLogFile);
    printf("%s %d %s\n", pathToFile, port, pathToLogFile);
}