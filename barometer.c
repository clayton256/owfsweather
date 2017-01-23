#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void update_baro_data (int baro_fd) {

  char pressure[6];
  float realpressure;
  int read_rc;

  /* Advance the file pointer 6 to eliminate the leading garbage */
  //lseek(baro_fd,5,SEEK_SET);
  lseek(baro_fd,0,SEEK_SET);

  read_rc = read(baro_fd,pressure,5);
  if(0 > read_rc)
  {
    close(baro_fd);
    global_config.baro_fd = -1;
    return;
  }
  
  realpressure = atof(pressure) * 0.02953;

  app_debug(debug, "Baro Press:\t%2.3f\n", realpressure);

  /* Apply the sanity checking.  Less than 26inHg */
  /* and either the sensor is misreading or you're in a Cat5+ hurricane */
  /* More than 33inHg and you've moved to Venus */
  if ((realpressure > (float)33) || (realpressure < (float)26))
  {
    app_debug(info, "Got garbage from the pressure sensor.  Rejecting\n");
  } else
  {
    /* Lock this with a mutex as well */
    pthread_mutex_lock(&global_data_mutex);
    global_data.barometer = realpressure;
    global_data.updatetime = time(NULL);
    pthread_mutex_unlock(&global_data_mutex);
  }

  return;
}
