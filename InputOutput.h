#ifndef InputAndOutput
#define InputAndOutput
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<math.h>
#include<string.h>
#include<stdarg.h> 
#include<sys/stat.h>

int PATH_SIZE = 500;

char READ[10] = "READ";
char WRITE[10] = "WRITE";
char WR[10] = "WR";

void GetArgumentsServer(int argc, char ** argv, char * pathToFile, int * port, char * pathToLogFile);
int StringToInt(char* strToInt, int size);
int OpenFile(char* filename, char* mode);
void CloseFile(char * filename, int fileDescription);
void LockFile(int fileDescription);
void UnlockFile(int fileDescription);
int StringCompare(char* first, char* second);
int ReadFile(char* filename, int fileDescription, void* buffer, int count);

/*End of declaration*/

void GetArgumentsServer(int argc, char ** argv, char * pathToFile, int * port, char * pathToLogFile)
{
    if(argc > 7)
    {
        printf("Too many arguments maybe...\n");
        printf("You should've called like that: ./program -i pathToFile -p PORT -o pathToLogFile\n");
        exit(-1);
    }
    if(argc < 7)
    {
        printf("Too little arguments maybe...\n");
        printf("You should've called like that: ./program -i pathToFile -p PORT -o pathToLogFile\n");
        exit(-1);
    }
    int opt;
    while((opt = getopt(argc, argv, "i:p:o:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'i':
                strcpy(pathToFile, optarg);
                break;
            case 'p':
                *port = StringToInt(optarg, strlen(optarg));
                break;
            case 'o':
                strcpy(pathToLogFile, optarg);
                break;
            default:
                printf("Wrong argument!\n");
                exit(-1);
        }
    }
}

int StringToInt(char* strToInt, int size)
{
    int i, result = 0;
    for(i = 0; i < size; ++i)
    {
        result = result * 10;
        if(strToInt[i] == '0')
        {
            result += 0;
        }
        if(strToInt[i] == '1')
        {
            result += 1;
        }
        if(strToInt[i] == '2')
        {
            result += 2;
        }
        if(strToInt[i] == '3')
        {
            result += 3;
        }
        if(strToInt[i] == '4')
        {
            result += 4;
        }
        if(strToInt[i] == '5')
        {
            result += 5;
        }
        if(strToInt[i] == '6')
        {
            result += 6;
        }
        if(strToInt[i] == '7')
        {
            result += 7;
        }
        if(strToInt[i] == '8')
        {
            result += 8;
        }
        if(strToInt[i] == '9')
        {
            result += 9;
        }
    }
    return result;
}

int OpenFile(char* filename, char* mode)
{
    int f;
    if(StringCompare(mode, READ))
    {
        f = open(filename, O_RDONLY | O_SYNC, 0666);
    }
    else if(StringCompare(mode, WRITE))
    {
        f = open(filename, O_WRONLY | O_SYNC | O_CREAT, 0666);
    }
    else if(StringCompare(mode, WR))
    {
        f = open(filename, O_RDWR | O_SYNC | O_CREAT, 0666);
    }
    if(f == -1)
    {
        printf("Cannot open this file: %s\nERROR %s\n", filename,strerror(errno));
        exit(-1);
    }
    return f;
}

void CloseFile(char * filename, int fileDescription)
{
    int f;
    f = close(fileDescription);
    if(f == -1)
    {
        printf("Cannot close this file: %s\nERROR %s\n", filename, strerror(errno));
        exit(-1);
    }
}

void LockFile(int fileDescription)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    if(-1 == fcntl(fileDescription, F_SETLKW, &lock))
    {
        printf("ERROR LOCKING THIS FILE!\n%s", strerror(errno));
        fflush(stdout);
    }
}

void UnlockFile(int fileDescription)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    if(-1 == fcntl(fileDescription, F_SETLKW, &lock))
    {
        printf("ERROR LOCKING THIS FILE!\n%s", strerror(errno));
        fflush(stdout);
    }
}

int StringCompare(char* first, char* second)
{
    int i = 0;
    while (first[i] == second[i]) 
    {
        if (first[i] == '\0' || second[i] == '\0')
            break;
        i++;
    }
    if (first[i] == '\0' && second[i] == '\0')
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int ReadFile(char* filename, int fileDescription, void* buffer, int count)
{
    int result;
    result = read(fileDescription, buffer, count);
    if(result == -1)
    {
        printf("Something went wrong while reading this file: %s\nERROR: %s\n", filename, strerror(errno));
        fflush(stdout);
        exit(-1);
    }
    return result;
}

int WriteFile(char* filename, int fileDescription, void* buffer, int count)
{
    int result;
    result = write(fileDescription, buffer, count);
    if(result == -1)
    {
        printf("Something went wrong while writing this file: %s\nERROR: %s\n", filename, strerror(errno));
        fflush(stdout);
        exit(-1);
    }
    return result;
}
#endif