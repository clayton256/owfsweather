#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void update_temp_data (int temp_fd) {

  char temp[6];
  float realtemp;
  int read_rc;

  assert(-1 != temp_fd);

  memset(temp, 0, 6);
  /* Advance the file pointer 6 to eliminate the leading garbage */
  lseek(temp_fd,0,SEEK_SET);

  read_rc = read(temp_fd,temp,5);
  if(0 > read_rc)
  {
    close(temp_fd);
    global_config.temp_fd = -1;
    return;
  }
  realtemp = atof(temp);

  app_debug(debug, "DS1820 Temp:\t%2.1f\n", realtemp);

  if (global_config.ignore_wsi_temp == 1)
  {
    /* Apply the same sanity checking with the DS1820 that you do with the WSI603 */
    if ((realtemp > (float)150) || (realtemp == (float)0.000))
    {
      app_debug(info, "Got garbage from the DS1820 sensor (%3.2f %s %d).  Rejecting\n", realtemp, temp, read_rc);
    } else
    {
      app_debug(debug, "DS1820 Temp:\t%2.1f\n",realtemp);
      /* Lock this with a mutex as well */
      pthread_mutex_lock(&global_data_mutex);
      global_data.temp = realtemp;
      global_data.updatetime = time(NULL);
      pthread_mutex_unlock(&global_data_mutex);
    }
  }
  return;
}


void update_temp2_data (int temp_fd) {

  char temp[6];
  float realtemp;
  int read_rc;

  assert(-1 != temp_fd);

  memset(temp, 0, 9);
  /* Advance the file pointer 6 to eliminate the leading garbage */
  lseek(temp_fd,0,SEEK_SET);

  read_rc = read(temp_fd,temp,5);
  if(0 > read_rc)
  {
    close(temp_fd);
    global_config.temp_fd = -1;
    return;
  }
  realtemp = atof(temp);

  app_debug(debug, "DS1820 Temp:\t%2.1f\n",realtemp);

  if (global_config.ignore_wsi_temp == 1) {

    /* Apply the same sanity checking with the DS1820 that you do with the WSI603 */
    if ((realtemp > (float)150) || (realtemp == (float)0.000))
    {
      app_debug(info, "Got garbage from the 2nd DS1820 sensor (%3.2f %s %d). Rejecting\n", realtemp, temp, read_rc);
    } else
    {
      app_debug(debug,"DS1820 Temp:\t%2.1f\n",realtemp);
      /* Lock this with a mutex as well */
      pthread_mutex_lock(&global_data_mutex);
      global_data.temp2 = realtemp;
      global_data.updatetime = time(NULL);
      pthread_mutex_unlock(&global_data_mutex);
    }
  }
  return;
}

