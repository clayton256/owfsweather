#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <rrd.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void create_rrd_file(void);

void start_rrd_logging(void *) {

  char *rrd_args[2];
  struct stat info;

  app_debug("RRD:  Starting RRD logging thread\n");

  if (stat(global_config.rrd_file, &info) == -1) {
    create_rrd_file();
  }


  sleep(8);  /* Pause the whole thing just a few seconds to get the first reading */

  rrd_args[0] = calloc(255,1);
  rrd_args[1] = (char *)NULL;


  while (1) {

    pthread_mutex_lock(&global_data_mutex);

    app_debug("RRD: Writing data\n");
    sprintf(rrd_args[0],"%i:%.2f:%.2f:%.2f:%i:%i:%.2f:%i",UPDATETM,TEMP,BARO,WINDAVG,WINDAVGDIR,HUMID,WINDGUST,WINDGUSTDIR);
    pthread_mutex_unlock(&global_data_mutex);

    // 1284784099:  84.53:    29.98:    1.66:   4:    1.46:     0.00:     30


    rrd_update_r(global_config.rrd_file,"temp:baro:windavg:winddir:humidity:windgustspd:windgustdir",1,(const char **)rrd_args);
    printf("%s\n",rrd_get_error());

    app_debug("RRD: Data writing complete (DATA BLOCK: %s\n",rrd_args[0]);



    sleep(global_config.rrd_interval);

  }

}

void create_rrd_file(void) {

  int argc = 8;

  char *argv[] = {
    "DS:temp:GAUGE:600:-50:150",
    "DS:baro:GAUGE:600:26:36",
    "DS:windavg:GAUGE:600:0:50",
    "DS:winddir:GAUGE:600:0:17",
    "DS:humidity:GAUGE:600:0:101",
    "DS:windgustspd:GAUGE:600:0:50",
    "DS:windgustdir:GAUGE:600:0:17",
    "RRA:LAST:0.5:1:86400",

    (char*)NULL
  };

  time_t start = time(NULL)-600;

  app_debug("RRD: Creating new RRD file %s\n",global_config.rrd_file);
  rrd_create_r(global_config.rrd_file, global_config.rrd_interval, start, argc, (const char **)argv);
  app_debug("RRD: Created new RRD file %s\n",global_config.rrd_file);

}
