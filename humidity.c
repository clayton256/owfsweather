#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void update_humidity_data (int humidity_fd) {

  char humidity[6];
  float realhumidity;
  int read_rc;

  /* Advance the file pointer 6 to eliminate the leading garbage */
  //lseek(humidity_fd,5,SEEK_SET);
  lseek(humidity_fd,0,SEEK_SET);

  read_rc = read(humidity_fd,humidity,5);
  if(0 > read_rc)
  {
    close(humidity_fd);
    global_config.humidity_fd = -1;
    return;
  }

  realhumidity = atof(humidity);
  realhumidity -= 5.0;

  app_debug(debug, "Humidity:\t%2.3f\n",realhumidity);

  /* Apply the sanity checking. */

  if ((realhumidity > (float)100) || (realhumidity < (float)0))
  {
    app_debug(info, "Got garbage from the humidity sensor.  Rejecting\n");
  } else
  {
    /* Lock this with a mutex as well */
    pthread_mutex_lock(&global_data_mutex);
    global_data.humidity = realhumidity;
    global_data.updatetime = time(NULL);
    pthread_mutex_unlock(&global_data_mutex);
  }
}
