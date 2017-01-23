#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <bsd/string.h>

#define APP_DEBUG_FLAG
#include "owfsweather.h"

pthread_mutex_t global_data_mutex;

void print_usage(void);
int open_1wire_fd(char *, char *);
void update_wind_avg(void);

int main (int argc, char *argv[])
{
  struct app_data appdata;
  char config_file[256];
  int c;
  int cmdln_debug = -1;
  int cmdln_dont_fork = -1;

  pthread_t log_thread;
  pthread_t wu_thread;
  pthread_t jmc_thread;
#if WATCHDOG_SUPPORT
  pthread_t watchdog_thread;
#endif
#ifdef RRD_SUPPORT
  pthread_t rrd_thread;
#endif

  /* ws_mem_fd is the FD pointint to the 256 bits of RAM on the WS603 */
  /* ws_temp_fd points to the WS603 temperature probe */
  /* The rest are self-explanatory */
  /* You can add more as you see fit */

  int speed_ind;

  printf("\n");
  printf("OWFS-Based Weather Station Controller\n");
  printf("(c)2010 James Sharp <james@fivecats.org>\n");
  printf("\n");

  memset(&appdata, '\0', sizeof(struct app_data));
  global_config.debug = -1;
  global_config.ws_mem_fd = -1;
  global_config.ws_temp_fd = -1;
  global_config.temp_fd = -1;
  global_config.temp2_fd = -1;
  global_config.baro_fd = -1;
  global_config.humidity_fd = -1;
  global_config.rain_fd = -1;
  global_config.anem_fd = -1;
  global_config.vane_fd = -1;

  opterr = 0;
  while ((c = getopt (argc,argv,"c:dfh")) != -1)
    switch(c)
      {
      case 'c':
        if (strlcpy(config_file, optarg, sizeof(config_file)) >= sizeof(config_file))
        {
            app_debug(debug, "config file %s\n", config_file);
            exit(1);
        }
        app_debug(debug, "config file %s\n", config_file);
        break;
      case 'h':
        print_usage();
        exit(0);
        break;
      case 'f':
        cmdln_dont_fork = 1;
        app_debug(debug, "dont fork %d\n", global_config.dont_fork);
        break;
      case 'd':
        cmdln_debug++;
        app_debug(debug, "debug level %d\n", global_config.debug);
        break;
      default:
        break;
      }

  if (1)
  {
     strlcpy(config_file, "/home/clayton/Projects/owfsweather-code/owfsweather.conf", sizeof(config_file));
     cmdln_debug = 4;
     cmdln_dont_fork = 1;
  }

  if (FALSE == parse_config_file(config_file))
  {
      app_debug(critical, "Error parsong config file: %s)\n", config_file);
  }

  if(-1 != cmdln_dont_fork)
    global_config.dont_fork = cmdln_dont_fork;
  if(-1 != cmdln_debug)
    global_config.debug = cmdln_debug + 1;
  app_debug(info, "debug log levels %d %d)\n", cmdln_debug, cmdln_debug, cmdln_dont_fork);

  if (0 == global_config.dont_fork)
  {
      daemonize();
  }

  /* Open up the FDs pointing to the sensors on the 1-wire net */

  app_debug(info, "Opening 1-Wire device files (OWFS Root: %s)\n", global_config.owfsroot);

  if (NULL != global_config.wsi_serial && -1 == global_config.ws_mem_fd) {
    app_debug(debug, "Opening WS603 devices: %s\n", global_config.wsi_serial);
    global_config.ws_mem_fd = open_1wire_fd(global_config.wsi_serial,"memory");
    global_config.ws_temp_fd = open_1wire_fd(global_config.wsi_serial,"temperature");
  }

  global_config.rain_fd = open_rain_device(global_config.rain_serial);
  global_config.anem_fd = open_anem_device(global_config.anem_serial);

  if (NULL != global_config.anem_serial && -1 == global_config.anem_fd) {
    app_debug(debug, "Opening anem device: %s\n", global_config.anem_serial);
    global_config.anem_fd = open_1wire_fd(global_config.anem_serial,"counters.A");
  }

  if (NULL != global_config.vane_serial && -1 == global_config.vane_fd) {
    app_debug(debug, "Opening wind vane (direction) device: %s\n", global_config.vane_serial);
    global_config.vane_fd = open_1wire_fd(global_config.vane_serial,"volt.ALL");
  }

  if (NULL != global_config.temp_serial && -1 == global_config.temp_fd) {
    app_debug(debug, "Opening temperature device: %s\n", global_config.temp_serial);
    global_config.temp_fd = open_1wire_fd(global_config.temp_serial,"temperature");
  }

  if (NULL != global_config.temp2_serial && -1 == global_config.temp2_fd) {
    app_debug(debug, "Opening 2nd temperature device: %s\n", global_config.temp2_serial);
    global_config.temp2_fd = open_1wire_fd(global_config.temp2_serial,"temperature");
  }

  if (NULL != global_config.baro_serial && -1 == global_config.baro_fd) {
    app_debug(debug, "Opening barometer device: %s\n", global_config.baro_serial);
    //baro_fd = open_1wire_fd(global_config.baro_serial,"B1-R1-A/pressure");
    global_config.baro_fd = open_1wire_fd(global_config.baro_serial,"TAI8570/pressure");
  }

  if (NULL != global_config.humidity_serial && -1 == global_config.humidity_fd) {
    app_debug(debug, "Opening humidity device: %s\n", global_config.humidity_serial);
    global_config.humidity_fd = open_1wire_fd(global_config.humidity_serial,"HIH4000/humidity");
  }


  if (global_config.outfile != NULL) {
    pthread_create(&log_thread, NULL, &start_logging, &appdata);
  }

#ifdef RRD_SUPPORT
  if (global_config.rrd_file != NULL) {
    pthread_create(&rrd_thread, NULL, &start_rrd_logging, &appdata);
  }
#endif

  if (global_config.wu_username != NULL) {
    pthread_create(&wu_thread, NULL, &start_wu_update, &appdata);
  }

  if (global_config.jmc_username != NULL) {
    pthread_create(&jmc_thread, NULL, &start_jmc_update, &appdata);
  }
#if WATCHDOG_SUPPORT
  if (global_config.watchdog != NULL) {
    pthread_create(&watchdog_thread, NULL, &start_watchdog, &appdata);
  }
#endif
  //init_wsi(ws_mem_fd);

  //wsi_set_leds(ws_mem_fd,LED_HIGH,LED_OFF);


  /* TO DO: Start up WU update loop thread */
  /* TO DO: Start up logging thread */

  /* Main loop-de-loop */
  while(1)
  {
    if (NULL != global_config.wsi_serial)
    {
      if(-1 != global_config.ws_mem_fd && -1 != global_config.ws_temp_fd)
      {
        update_wsi_data(global_config.ws_mem_fd,global_config.ws_temp_fd);
        update_wind_avg();
      }
      else
      {
        app_debug(debug, "Re-opening WS603 devices: %s\n",global_config.wsi_serial);
        global_config.ws_mem_fd = open_1wire_fd(global_config.wsi_serial,"memory");
        global_config.ws_temp_fd = open_1wire_fd(global_config.wsi_serial,"temperature");
      }
    }

    if (NULL != global_config.temp_serial)
    {
      if (-1 != global_config.temp_fd)
        update_temp_data(global_config.temp_fd);
      else 
      {
        app_debug(debug, "Re-opening temperature device: %s\n",global_config.temp_serial);
        global_config.temp_fd = open_1wire_fd(global_config.temp_serial,"temperature");
      }
    }

    if (NULL != global_config.temp2_serial)
    {
      if(-1 != global_config.temp2_fd)
        update_temp2_data(global_config.temp2_fd);
      else
      {
        app_debug(debug, "Re-opening 2nd temperature device: %s\n",global_config.temp2_serial);
        global_config.temp2_fd = open_1wire_fd(global_config.temp2_serial,"temperature");
      }
    }

    if (NULL != global_config.baro_serial)
    {
      if(-1 != global_config.baro_fd)
        update_baro_data(global_config.baro_fd);
      else
      {
        app_debug(debug, "Re-opening barometer device: %s\n",global_config.baro_serial);
        global_config.baro_fd = open_1wire_fd(global_config.baro_serial,"TAI8570/pressure");
      }
   }

   if (NULL != global_config.humidity_serial)
   {
      if (-1 != global_config.humidity_fd)
        update_humidity_data(global_config.humidity_fd);
     else 
     {
       app_debug(debug, "Re-opening humidity device: %s\n",global_config.humidity_serial);
       global_config.humidity_fd = open_1wire_fd(global_config.humidity_serial,"HIH4000/humidity");
     }
   }

   if (NULL != global_config.rain_serial)
   {
     if (-1 != global_config.rain_fd)
       update_rain_data(global_config.rain_fd);
     else
     {
       app_debug(debug, "Re-opening rain device: %s\n",global_config.rain_serial);
       global_config.rain_fd = open_1wire_fd(global_config.rain_serial,"counters.A");
     }
  }

   if (NULL != global_config.vane_serial)
   {
     if (-1 != global_config.vane_fd)
     {
       update_vane_data(global_config.vane_fd);
       update_wind_avg();
     }
     else
     {
       app_debug(debug, "Re-opening wind vane (direction) device: %s\n",global_config.vane_serial);
       global_config.vane_fd = open_1wire_fd(global_config.vane_serial,"volt.ALL");
     }
   }

   if (NULL != global_config.anem_serial)
   {
     if (-1 != global_config.anem_fd)
     {
       update_anem_data(global_config.anem_fd);
       update_wind_avg();
     }
     else
     {
       app_debug(debug, "Re-opening anem device: %s\n",global_config.anem_serial);
       global_config.anem_fd = open_1wire_fd(global_config.anem_serial,"counters.A");
     }
   }

   if ((global_data.wind_speed > 10) && (speed_ind == 0)) {
     //wsi_set_leds(global_config.ws_mem_fd,LED_OFF,LED_HIGH);
     speed_ind = 1;
   }

   if ((global_data.wind_speed < 10) && (speed_ind == 1)) {
     //wsi_set_leds(global_config.ws_mem_fd,LED_HIGH,LED_OFF);
     speed_ind = 0;
   }

    sleep(1);
  }
}

void update_wind_avg(void) {

  static int count = 0;
  static float speed = 0.0;
  static int dir = 0;
  static time_t timer = 0;

  if ( ((time(NULL) - timer) > 120) && (count != 0) )
  {
    pthread_mutex_lock(&global_data_mutex);
    WINDAVG = speed/(float)count;
    WINDAVGDIR = dir/(int)count;
    pthread_mutex_unlock(&global_data_mutex);

    app_debug(debug, "MAIN:   Updating wind speed average to %2.2f\n", WINDAVG);
    app_debug(debug, "MAIN:   Updating wind direction average to %2.2f\n", WINDAVGDIR);

    count = 0;
    speed = 0;
    dir = 0;
    pthread_mutex_lock(&global_data_mutex);
    WINDGUST = 0.0;
    WINDGUSTDIR = 0;
    pthread_mutex_unlock(&global_data_mutex);

    timer = time(NULL);
  } else
  {
    count++;
    speed = speed + WINDSPD;
    dir = dir + WINDDIR;

    if (WINDSPD > WINDGUST)
    {
      pthread_mutex_lock(&global_data_mutex);
      WINDGUST = WINDSPD;
      WINDGUSTDIR = WINDDIR;
      pthread_mutex_unlock(&global_data_mutex);
      app_debug(debug, "MAIN:    Updated wind gust speed to %2.2f\n",WINDGUST);
      app_debug(debug, "MAIN:    Updated wind gust dir to %2.2f\n",WINDGUSTDIR);

    }
  }
  return;
}







void print_usage(void) {

  printf("owfsweather [-h] [-c configfile] [-d]\n");
  printf("      -h Help (this message)\n");
  printf("      -c <configfile> specify alternate config file\n");
  printf("         Default is \"owfsweather.conf\n\"");
  printf("      -d Enable debug\n");
  printf("      -f Don't fork (threads are still spawned)\n");
  printf("\n");
}


int open_1wire_fd(char *sn, char *subdev) {

  char *tmpfilename;
  int fd;

  tmpfilename = (char *)malloc(255);

  sprintf(tmpfilename,"%s/%s/%s",global_config.owfsroot,sn,subdev);
  app_debug(debug, "device open: %s\n",tmpfilename);

  fd = open(tmpfilename,O_RDWR);

  if (fd == -1) {
    app_debug(critical, "Failed to open device %s\n",tmpfilename);
    perror("Error: ");
    exit(0);
  } else {
    app_debug(critical, "Opened %s as fd %i\n",tmpfilename,fd);
  }
  free(tmpfilename);
  return fd;
}






