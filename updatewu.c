#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <strings.h>
#include <string.h>

#include <curl/curl.h>
#include <time.h>

#include "owfsweather.h"

#define UPDATEURL "http://weatherstation.wunderground.com/weatherstation/updateweatherstation.php?ID=%s&PASSWORD=%s&dateutc=%s&winddir=%s&windspeedmph=%f&tempf=%f&action=updateraw"

#define RFUPDATEURL "http://rtupdate.wunderground.com/weatherstation/updateweatherstation.php?ID=%s&PASSWORD=%s&dateutc=%s&winddir=%s&windspeedmph=%f&tempf=%f&action=updateraw&realtime=1&rtfreq=%i"

#define DATEFORMAT "%Y-%m-%d %H:%M:%S"




extern pthread_mutex_t global_data_mutex;

void * start_wu_update(void * data)
{
  char *log_line;
  char *wind_dir[17] = {"NONE","0","22","45","67","90","112","135","157","180","202","225","247","270","292","315","337"};
  float tempf;
  float wind_speed;
  char utcdatestr[24];

  CURL *curl;
  CURLcode res;

  char *escaped_url = NULL;

  struct tm *utctime;

  log_line = (char *)malloc(255);

  utctime = (struct tm *)malloc(sizeof(struct tm));

  app_debug(info, "WUUPDATE:  Beginning WU update thread\n");

  sleep(5);  /* Pause the whole thing just a few seconds to get the first reading */

  while (1) {

    bzero(utctime,sizeof(struct tm));
    curl = curl_easy_init();

    pthread_mutex_lock(&global_data_mutex);

    tempf = ((float)(1.8)*TEMP)+(float)32;

    wind_speed = WINDSPD * (float)2.24;

    utctime = gmtime(&global_data.updatetime);
    strftime(utcdatestr,24,DATEFORMAT,utctime);

    if (global_config.wu_rapid_fire){
      sprintf(log_line,RFUPDATEURL,WU_ID,WU_PWD,curl_easy_escape(curl,utcdatestr,0),wind_dir[WINDDIR],wind_speed,tempf,global_config.wu_update_interval);
    }
    if (!global_config.wu_rapid_fire)
    {
      sprintf(log_line,UPDATEURL,WU_ID,WU_PWD,curl_easy_escape(curl,utcdatestr,0),wind_dir[WINDDIR],wind_speed,tempf);
    }

    pthread_mutex_unlock(&global_data_mutex);

    curl_easy_setopt(curl, CURLOPT_URL, log_line);
    app_debug(debug, "WUUPDATE:  URL Line:   %s\n",curl_easy_unescape(curl,log_line,0,NULL));
    app_debug(debug, "WUUPDATE:  URL Line:   %s\n", log_line);
    res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        app_debug(debug, "libcurl: (%d) %s\n", res, curl_easy_strerror(res));
    }
    curl_easy_cleanup(curl);

    if (!global_config.wu_rapid_fire)
    {
      sleep(global_config.wu_update_interval * 60); /* Non-rapidfire is in min */
    } else {
      sleep(global_config.wu_update_interval);
    }

    curl_free(escaped_url);
  }
  return NULL;
}

