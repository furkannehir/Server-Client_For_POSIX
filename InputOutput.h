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

void GetArgumentsServer(int argc, char ** argv, char * pathToFile, int * port, char * portString, char * pathToLogFile, int * poolThread, int * maxPool, int * prio);
void GetArgumentsClient(int argc,char ** argv, char * IP, int * port, char * portString, int * sourceNode, int * destNode);
int StringToInt(char* strToInt, int size);
int OpenFile(char* filename, char* mode);
void CloseFile(char * filename, int fileDescription);
int LockFile(int fileDescription);
int UnlockFile(int fileDescription);
int StringCompare(char* first, char* second);
int ReadFile(char* filename, int fileDescription, void* buffer, int count);
int ReadOneLine(char * filename, int fileDescription, char * buffer);
char * GetTimestamp(char * str);

/*End of declaration*/

void GetArgumentsServer(int argc, char ** argv, char * pathToFile, int * port, char * portString, char * pathToLogFile, int * poolThread, int * maxPool, int * prio)
{
    if(argc > 13)
    {
        printf("Too many arguments maybe...\n");
        printf("You should've called like that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 0\n");
        printf("Or that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24");
        exit(-1);
    }
    if(argc < 11)
    {
        printf("Too little arguments maybe...\n");
        printf("You should've called like that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 0\n");
        printf("Or that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24");
        exit(-1);
    }
    int opt;
    if(argc == 13)
    {
        while((opt = getopt(argc, argv, "i:p:o:s:x:r:")) != -1)  
        {  
            switch(opt)  
            {  
                case 'i':
                    strcpy(pathToFile, optarg);
                    break;
                case 'p':
                    *port = StringToInt(optarg, strlen(optarg));
                    strcpy(portString, optarg);
                    break;
                case 'o':
                    strcpy(pathToLogFile, optarg);
                    break;
                case 's':
                    *poolThread = StringToInt(optarg, strlen(optarg));
                    break;
                case 'x':
                    *maxPool = StringToInt(optarg, strlen(optarg));
                    break;
                case 'r':
                    *prio = StringToInt(optarg, strlen(optarg));
                    break;
                default:
                    printf("Wrong argument!\n");
                    printf("You should've called like that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 0\n");
                    exit(-1);
            }
        }
    }
    else if(argc == 11)
    {
        *prio = 1;
        while((opt = getopt(argc, argv, "i:p:o:s:x:")) != -1)  
        {  
            switch(opt)  
            {  
                case 'i':
                    strcpy(pathToFile, optarg);
                    break;
                case 'p':
                    *port = StringToInt(optarg, strlen(optarg));
                    strcpy(portString, optarg);
                    break;
                case 'o':
                    strcpy(pathToLogFile, optarg);
                    break;
                case 's':
                    *poolThread = StringToInt(optarg, strlen(optarg));
                    break;
                case 'x':
                    *maxPool = StringToInt(optarg, strlen(optarg));
                    break;
                default:
                    printf("Wrong argument!\n");
                    printf("You should've called like that: ./server -i pathToFile -p PORT -o pathToLogFile -s 4 -x 24 -r 0\n");
                    exit(-1);
            }
        }
    }
}

void GetArgumentsClient(int argc,char ** argv, char * IP, int * port, char * portString, int * sourceNode, int * destNode)
{
    if(argc > 9)
    {
        printf("Too many arguments maybe...\n");
        printf("You should've called like that: ./program -i pathToFile -p PORT -o pathToLogFile\n");
        exit(-1);
    }
    if(argc < 9)
    {
        printf("Too little arguments maybe...\n");
        printf("You should've called like that: ./program -i pathToFile -p PORT -o pathToLogFile\n");
        exit(-1);
    }
    int opt;
    while((opt = getopt(argc, argv, "a:p:s:d:")) != -1)  
    {  
        switch(opt)  
        {  
            case 'a':
                strcpy(IP, optarg);
                break;
            case 'p':
                *port = StringToInt(optarg, strlen(optarg));
                strcpy(portString, optarg);
                break;
            case 's':
                *sourceNode = StringToInt(optarg, strlen(optarg));
                break;
            case 'd':
                *destNode = StringToInt(optarg, strlen(optarg));
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

int LockFile(int fileDescription)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    int num = fcntl(fileDescription, F_SETLK, &lock);
    printf("lock : %d\n", num);
    if(num > 0)
    {
        printf("ERROR LOCKING THIS FILE!: %s\n", strerror(errno));
        return 0;
    }
    return 1;
}

int UnlockFile(int fileDescription)
{
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_UNLCK;
    if(-1 == fcntl(fileDescription, F_SETLKW, &lock))
    {
        printf("ERROR LOCKING THIS FILE!\n%s", strerror(errno));
        return 0;
    }
    return 1;
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

int ReadOneLine(char * filename, int fileDescription, char * buffer)
{
    int i = 0;
    char temp;
    if(0 == ReadFile(filename, fileDescription, &temp, 1))
    {
        return 0;
    }
    buffer[i] = temp;
    ++i;
    while(temp != '\n' && ReadFile(filename, fileDescription, &temp, 1))
    {
        buffer[i] = temp;
        ++i;
    }
    buffer[i] = '\0';
    return i;
}

char * GetTimestamp(char * str)
{
    time_t timer;
    char temp[500];
    time(&timer);
    // printf("%s", ctime(&timer));
    strcpy(temp, ctime(&timer));
    int k = 0;
    for(int i = 0; i < strlen(temp); ++i)
    {
        if(temp[i] != '\n'){
            str[k] = temp[i];
            ++k;
        }
    }
    str[k] = '\0';
    return str;
}
#endif