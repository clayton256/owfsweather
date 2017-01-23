#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void * start_logging(void * data)
{
  struct app_data *appdata;
  int log_fd;
  int i;
  char *log_line;

  appdata = (struct app_data *)data;
  log_line = (char *)malloc(255);

  app_debug(quiet, "LOGFILE:  Trying to open outfile %s\n",appdata->config_data.outfile);
  log_fd = open(global_config.outfile,O_RDWR|O_CREAT,S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
  if (log_fd <= 0)
  {
    perror("LOGFILE:  Failed to open outfile");
    return NULL;
  }

  app_debug(debug, "LOGFILE:  Opened log file %s as FD %i\n", global_config.outfile, log_fd);

  sleep(5);  /* Pause the whole thing just a few seconds to get the first reading */

  while (1)
  {
    pthread_mutex_lock(&global_data_mutex);
    sprintf(log_line,"%lu,%.2f,%i,%.2f,%.2f,%i,%.2f,%i,%.2f,%i\n",
            UPDATETM,TEMP,WINDDIR,WINDSPD,BARO,HUMID,WINDAVG,WINDAVGDIR,WINDGUST,WINDGUSTDIR);
    pthread_mutex_unlock(&global_data_mutex);

    app_debug(debug, "LOGFILE:  Log Line:   %s\n",log_line);
    if (global_config.log_overwrite)
    {
      lseek(log_fd,0,SEEK_SET);
    }
    i = write(log_fd,log_line,strlen(log_line));

    if (i <= 0)
    {
      perror("LOGFILE:  Log writing error");
    }
    app_debug(debug, "LOGFILE:  Wrote %i bytes to outfile fd %i\n",i,log_fd);
    sleep(global_config.log_interval);
  }

  return NULL;
}

