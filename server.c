#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
#include"become_daemon.h"
#include"InputOutput.h"

char pathToFile[500];
char pathToLogFile[500];
int port;

int main(int argc, char ** argv)
{
    GetArgumentsServer(argc, argv, pathToFile, &port, pathToLogFile);
    printf("%s %d %s\n", pathToFile, port, pathToLogFile);
    int pid = fork();
    // switch (pid)
    // {
    // case /* constant-expression */:
    //     /* code */
    //     break;
    
    // default:
    //     break;
    // }
    // if(pid == 0)
    // {
    //     int id = setsid();
    //     int mode = umask(0);
    //     printf("mode: %d, id: %d\n", mode, id);
    //     return 0;
    // }
    // else
    // {
    //     printf("asdasd: %d\n", pid);
    //     exit(EXIT_SUCCESS);
    // }
    becomeDaemon(0);
    
}

int becomeDaemon(int flags)  /* Returns 0 on success, -1 on error */
{
    int maxfd, fd;
    switch (fork()) 
    {
        /* Become background process */
        case -1: 
            return -1;
        case  0: 
            break;/* Child falls through; adopted by init */
        default: 
            _exit(EXIT_SUCCESS);/* parent terminates and shell prompt is back*/
    } 
    if (setsid() == -1)/* Become leader of new session, dissociate from tty */
        return -1; /* can still acquire a controlling terminal */
        switch (fork()) 
        {
            /* Ensure we are not session leader */
            case -1: 
                return -1;/* thanks to 2nd fork, there is no way of acquiring a tty */
            case 0: 
                break;
            default: 
                _exit(EXIT_SUCCESS);
        }
        if (!(flags & BD_NO_UMASK0))
            umask(0);/* Clear file mode creation mask */
        if (!(flags & BD_NO_CHDIR))
            chdir("/"); /* Change to root directory */
        if (!(flags & BD_NO_CLOSE_FILES)) 
        { /* Close all open files */
            maxfd = sysconf(_SC_OPEN_MAX);if (maxfd == -1)/* Limit is indeterminate... */
            maxfd = BD_MAX_CLOSE;/* so take a guess */
            for (fd = 0; fd < maxfd; fd++)
            close(fd);    
        }
        if (!(flags & BD_NO_REOPEN_STD_FDS)) 
        {
            close(STDIN_FILENO);/* Reopen standard fd's to /dev/null */
            fd = open("/dev/null", O_RDWR);
            if (fd != STDIN_FILENO) /* 'fd' should be 0 */
                return -1;
            if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
                return -1;
            if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
                return -1;
        }
        return 0;
}