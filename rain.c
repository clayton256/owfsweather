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
#define APP_DEBUG_FLAG
#include "owfsweather.h"
/*
 * For stations which supply a rain rate, Cumulus uses that. For stations which don't supply a rain rate (e.g. Fine Offset and La Crosse), Cumulus simply takes the rain total from the last five minutes and calculates a rate based on that; e.g. a single tip of 0.3mm in 5 minutes is a rate of 3.6mm/hr. When data from the station's logger is used, a similar calculation is performed, but the interval used is the logger interval rather than five minutes.
 *
The reason that you are seeing such rates is because it is projected from one minute to the next and does not consider the data from the past hour. This is then the instantaneous rate of change, not the hourly rate. The instantaneous rate would assume that the change seen from one minute to the next would continue for the next hour at the rate from 0 to .4mm. I think you are also multiplying by 60 which is not correct. (.4 * 3600) = 1440. I think this would really be .4 * 60 = 24 mm/hr if that same rate continued for an entire hour (always at the same .4mm per minute for an entire hour).
To determine an hourly rainfall rate, the change in time is 60 minutes, the change in total from one hour ago to the time the calculation is made is the "distance" in the formula:
distance (mm) = rate (mm/hr)  * time (hr)
rate = distance / time
rain rate = ((total now - total one hour ago)) / 60 mins.
rate = (.4mm - 0) / 60
rate = 0.0066 mm/hr, which rounds to 0.01 mm/hr.
 *
Very light rain	precipitation rate is < 0.25 mm/hour
Light rain	precipitation rate is between 0.25mm/hour and 1.0mm/hour
Moderate rain	precipitation rate is between 1.0 mm/hour and 4.0 mm/hour
Heavy rain	recipitation rate is between 4.0 mm/hour and 16.0 mm/hour
Very heavy rain   	precipitation rate is between 16.0 mm/hour and 50 mm/hour
Extreme rain	recipitation rate is > 50.0 mm/hour
 */
extern pthread_mutex_t global_data_mutex;

static int prevRainA = 0;
static int prevRainB = 0;
static int prevTime = 0;
//static time_t prevReset = 0;
static time_t reset_time = 0;

int open_rain_device(char * device_str)
{
  int fd = -1;
  assert(-1 == global_config.rain_fd);
  if (NULL != global_config.rain_serial && -1 == global_config.rain_fd) {
    app_debug(debug, "Opening rain device: %s\n" ,global_config.rain_serial);
    fd = open_1wire_fd(global_config.rain_serial,"counters.ALL");
    app_debug(debug, "rain_fd: %d\n" ,fd);
  }
  return fd;
}


void update_rain_data (int rain_fd)
{
  char counters[18];
  int counterA;
  int counterB;
  int realcounters;
  struct tm currtm;
  time_t now;
  int tm_hour, tm_min;
  int read_rc;

  assert(-1 != rain_fd);

  memset(counters, 0, 18);

  now = time(NULL);

  /* The first time through, set the reference count */
  if (0 == prevRainA || 0 == prevRainB)
  {
    app_debug(debug, "Rain: first time\n");
    // segfaults: strptime(global_config.reset_tofd_str, 6, "%H:%M", &temp);
    sscanf(global_config.reset_tofd_str, "%2d:%2d", &tm_hour, &tm_min);
    currtm = *localtime(&now);
    currtm.tm_hour = tm_hour;
    currtm.tm_min = tm_min;
    currtm.tm_sec = 0;
    reset_time = mktime(&currtm);

    /* Advance the file pointer to eliminate the leading garbage */
    lseek(rain_fd, 0, SEEK_SET);
    read_rc = read(rain_fd, counters, 17);
    if(-1 == read_rc)
    {
      app_debug(debug, "Rain:first time read error %d\n", read_rc);
      close(rain_fd);
      global_config.rain_fd = -1;
      return;
    }
    app_debug(debug, "Rain counters: %s\n", counters);
    sscanf(counters, "%d,%d", &counterA, &counterB);
    app_debug(debug, "Rain counterA: %d\n", counterA);
    app_debug(debug, "Rain counterB: %d\n", counterB);
    prevRainA = counterA;
    prevRainB = counterB;
    //prevRain = atoi(counters);
    prevTime = now;
  }

  /* Advance the file pointer 6 to eliminate the leading garbage */
  lseek(rain_fd, 0, SEEK_SET);
  read_rc = read(rain_fd, counters, 17);
  if(-1 == read_rc)
  {
    app_debug(debug, "Rain:read error %d\n", read_rc);
    close(rain_fd);
    global_config.rain_fd = -1;
    return;
  }
  //prevRain = atoi(counters);
  sscanf(counters, "%d,%d", &counterA, &counterB);
  app_debug(debug, "Rain counterA: %d\n", counterA);
  app_debug(debug, "Rain counterB: %d\n", counterB);
  realcounters = (counterA - prevRainA) + (counterB - prevRainB);
  app_debug(debug, "Rain realcounters %d\n", realcounters);

  int diff = difftime(reset_time, now);
  //printf("diff:   %d\n", diff);
  if(diff < 0)
  {
    app_debug(debug, "Rain: reset time\n");
    reset_time += 86400;
    //prevRain = atoi(counters);
    prevRainA = counterA;
    prevRainB = counterB;
    prevTime = now;
  }

  app_debug(debug, "Rain Counter:\t%d\n", realcounters);

  /* Apply the sanity checking. */
  if ((realcounters > 999) || (realcounters < 0))
  {
    app_debug(info, "Rain: Got garbage from the rain counter sensor.  Rejecting\n");
  } else {
    char *end;
    /* Lock this with a mutex as well */
    pthread_mutex_lock(&global_data_mutex);
    global_data.rain = (float)realcounters;
    global_data.rain *= 0.01f;
    global_data.rain *= strtof(global_config.rain_calib, &end);
    app_debug(debug, "Rain Calibration:\t%1.2f\n", strtof(global_config.rain_calib, &end));
    int part_of_hour = (int)(60/(now - prevTime + 1));
    global_data.rainrate = global_data.rain * part_of_hour;
    global_data.updatetime = time(NULL);
    app_debug(debug, "Rain:\t\t%1.2f\nRate:\t\t%1.2f\n", global_data.rain, global_data.rainrate);
    pthread_mutex_unlock(&global_data_mutex);
    prevTime = now;
  }

  return;
}
