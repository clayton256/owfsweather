/*
UNIX Daemon Server Programming Sample Program
Levent Karakas <levent at mektup dot at> May 2001

To compile:	cc -o exampled examped.c
To run:		./exampled
To test daemon:	ps -ef|grep exampled (or ps -aux on BSD systems)
To test log:	tail -f /tmp/exampled.log
To test signal:	kill -HUP `cat /tmp/exampled.lock`
To terminate:	kill `cat /tmp/exampled.lock`
*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <syslog.h>

#include "owfsweather.h"

#define RUNNING_DIR "/"
#define LOCK_FILE   "/var/lock/owfswxd.lock"
#define LOG_FILE    "/var/log/owfswx.log"

void log_message(char *filename, char *message)
{
    FILE *logfile;
    logfile=fopen(filename,"a");
    if(!logfile) return;
    fprintf(logfile,"%s\n",message);
    fclose(logfile);
}

void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            log_message(LOG_FILE,"hangup signal caught!");
            log_message(LOG_FILE,"hangup signal caught!");
            break;
        case SIGTERM:
            log_message(LOG_FILE,"terminate signal caught!");
            exit(0);
            break;
    }
}

void daemonize(void)
{
    int i, lfp;
    char str[10];

    if (getppid() == 1)
        return; /* already a daemon */
    i = fork();
    if (i < 0)
    {
        printf("Fork Error");
        exit(EXIT_FAILURE); /* fork error */
    }
    if (i > 0)
        exit(EXIT_SUCCESS); /* parent exits */
    /* child (daemon) continues */
    setsid(); /* obtain a new process group */
    for (i = getdtablesize(); i >= 0; --i)
        close(i); /* close all descriptors */

    i = open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */

    umask(027); /* set newly created file permissions */

    chdir("/"); /* change running directory */

    lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
    if (lfp < 0)
    {
        app_debug(warning, "could not open lock file");
        exit(1); /* can not open */
    }
    if (lockf(lfp, F_TLOCK, 0) < 0)
    {
        app_debug(warning, "could not lock lock file");
        exit(0); /* can not lock */
    }
    /* first instance continues */
    sprintf(str, "%d\n", getpid());
    write(lfp, str, strlen(str)); /* record pid to lockfile */
    signal(SIGCHLD, SIG_IGN); /* ignore child */
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP, signal_handler); /* catch hangup signal */
    signal(SIGTERM, signal_handler); /* catch kill signal */

    return;
}

/* test function
int main(int argc, char ** argv)
{
    daemonize();
    while(1) sleep(1);
}
*/

