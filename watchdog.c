#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define __USE_XOPEN
#include <time.h>
#include <assert.h>

#include "owfsweather.h"

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

void * start_watchdog(void * data)
{
  struct app_data *appdata;
  int wd_fd;

  appdata = (struct app_data *)data;

  app_debug(quiet, "LOGFILE:  Trying to open watchdog%s\n",appdata->config_data.watchdog);
  wd_fd = open(global_config.watchdog, O_RDWR);
  if (wd_fd <= 0)
  {
    app_debug(quiet, "LOGFILE:  Failed to open watchdog");
    return NULL;
  }

  app_debug(debug, "LOGFILE:  Opened watchdog as FD %i\n", wd_fd);

  sleep(5);  /* Pause the whole thing just a few seconds to get the first reading */

  while (1)
  {
    app_debug(debug, "LOGFILE: write to watchdog\n");
    write(wd_fd, "w", 1);
    app_debug(debug, "LOGFILE: Wrote to watchdog\n");
    sleep(50);
  }
 
  write(wd_fd, "V", 1);
  close(wd_fd);
  return NULL ;
}

