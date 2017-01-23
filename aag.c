#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
//#define APP_DEBUG_FLAG
#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

int dirpointcalib[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};

void calibrate_vane(int north)
{
   for(int i=0; i<16; i++)
   {
       dirpointcalib[i] -= north;
       if(dirpointcalib[i] < 0)
       {
           dirpointcalib[i] += 16;
       }
   }     
}


int open_anem_device(char * device)
{
  int fd = -1;
  if (NULL != global_config.anem_serial && -1 == global_config.anem_fd)
  {
    app_debug(debug, "Opening anem device: %s\n", global_config.anem_serial);
    fd = open_1wire_fd(global_config.anem_serial,"counters.A");
  }
  calibrate_vane(global_config.vane_calib);
  return fd;
}


float normalize_voltage(float voltage)
{
  if (0.0f == voltage)
  {
    return 0.0f;
  }
  if (voltage <= 1.0f )
  {
    return 0.0f;
  }
  if ((voltage > 1.0f) && (voltage <= 2.7f))
  {
    return 2.5;
  }
  if ((voltage > 2.7f) && (voltage <= 4.0f))
  {
    return 3.3f;
  }
  if (voltage > 4.0f)
  {
    return 5.0f;
  }
  return 0.0f;
}


void update_vane_data (int vane_fd) {

  char voltsstr[53];
  float volts[4];
  int i;
  int dirpoint=-1;

  memset(voltsstr, 0, 53);
  lseek(vane_fd,0,SEEK_SET);
  read(vane_fd,voltsstr,52);

  sscanf(voltsstr, " %f, %f, %f, %f", &volts[0], &volts[1], &volts[2], &volts[3]);
  for(i=0; i<4; i++)
  {
    volts[i] = normalize_voltage(volts[i]);
  }

  app_debug(debug, "Vane Varr:\t%1.2f, %1.2f, %1.2f, %1.2f\n", volts[0], volts[1], volts[2], volts[3]);
  if ((volts[0]==5.0f)&&(volts[1]==5.0f)&&(volts[2]==2.5f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[0];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==3.3f)&&(volts[2]==3.3f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[1];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==2.5f)&&(volts[2]==5.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[2];
  }
  else if ((volts[0]==3.3f)&&(volts[1]==3.3f)&&(volts[2]==5.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[3];
  }
  else if ((volts[0]==2.5f)&&(volts[1]==5.0f)&&(volts[2]==5.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[4];
  }
  else if ((volts[0]==2.5f)&&(volts[1]==5.0f)&&(volts[2]==5.0f)&&(volts[3]==0.0f))
  {
    dirpoint = dirpointcalib[5];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==5.0f)&&(volts[2]==5.0f)&&(volts[3]==0.0f))
  {
    dirpoint = dirpointcalib[6];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==5.0f)&&(volts[2]==0.0f)&&(volts[3]==0.0f))
  {
    dirpoint = dirpointcalib[7];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==5.0f)&&(volts[2]==0.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[8];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==0.0f)&&(volts[2]==0.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[9];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==0.0f)&&(volts[2]==5.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[10];
  }
  else if ((volts[0]==0.0f)&&(volts[1]==0.0f)&&(volts[2]==5.0f)&&(volts[3]==5.0f))
  {
    dirpoint = dirpointcalib[11];
  }
  else if ((volts[0]==0.0)&&(volts[1]==5.0)&&(volts[2]==5.0)&&(volts[3]==5.0))
  {
    dirpoint = dirpointcalib[12];
  }
  else if ((volts[0]==0.0)&&(volts[1]==5.0)&&(volts[2]==5.0)&&(volts[3]==2.5))
  {
    dirpoint = dirpointcalib[13];
  }
  else if ((volts[0]==5.0)&&(volts[1]==5.0)&&(volts[2]==5.0)&&(volts[3]==2.5))
  {
    dirpoint = dirpointcalib[14];
  }
  else if ((volts[0]==5.0f)&&(volts[1]==5.0f)&&(volts[2]==3.3f)&&(volts[3]==3.3f))
  {
    dirpoint = dirpointcalib[15];
  }
  else
  {
    app_debug(debug, "Vane Vstr:\t%s\n", voltsstr);
    app_debug(debug, "Vane: Unknown Direction Reading %d %1.2f %1.2f %1.2f %1.2f\n", 
					dirpoint, volts[0], volts[1], volts[2], volts[3]);
  }
  if (dirpoint == -1)
  {
    app_debug(debug, "Vane Vstr:\t%s\n", voltsstr);
    app_debug(debug, "Vane: Unknown Direction Reading %d %1.2f %1.2f %1.2f %1.2f\n", 
					dirpoint, volts[0], volts[1], volts[2], volts[3]);
  }
  else
  {
    app_debug(debug, "Vane Dir:\t%d\n", dirpoint);
    /* Lock this with a mutex as well */
    pthread_mutex_lock(&global_data_mutex);
    global_data.wind_dir = dirpoint;
    global_data.updatetime = time(NULL);
    pthread_mutex_unlock(&global_data_mutex);
  }

  return;
}


static time_t prevTime = 0;
static float  prevAnem = 0.0f;

void update_anem_data (int anem_fd) {

  char anem[7];
  float realanem, anemdelta;
  float wind_speed = 0.0f;
  time_t now = time(NULL);
  float delta_T;

  /* Advance the file pointer 6 to eliminate the leading garbage */
  memset(anem, 0, 7);
  lseek(anem_fd,0,SEEK_SET);
  read(anem_fd,anem,5);
  //lseek(anem_fd,0,SEEK_SET);
  app_debug(debug, "Anem str:\t%s\n", anem);
  realanem = atof(anem);
  anemdelta = realanem - prevAnem;
  app_debug(debug, "Anem Speed:\t%4.2f\n", realanem);
  app_debug(debug, "Prev Anem :\t%4.2f\n", prevAnem);
  app_debug(debug, "Anem delta:\t%4.2f\n", anemdelta);
  prevAnem = realanem;
  if (anemdelta < 0.0f)
  {
    app_debug(warning, "WIND: Unknown Speed Reading %1.2f\n", realanem);
  }
  else
  {
    char *end;
    delta_T = (float)(now - prevTime);
    app_debug(debug, "Wind Speed:\tnow %lu prev %lu delta_T %1.2f\n", now, prevTime, delta_T);
    anemdelta /= 2.0f;
    anemdelta /= delta_T;
    wind_speed = anemdelta*2.453f; // if ($speed_uom eq "mph");
    wind_speed *= strtof(global_config.wind_calib, &end);
    //wind_speed = realanem*3.9477f; // if ($speed_uom eq "kph");
    //wind_speed = realanem*1.096f; // if ($speed_uom eq "mps");
    //wind_speed = realanem*2.130f; // if ($speed_uom eq "kt");
    //we now have the speed, not just rotate/sec
    app_debug(debug, "Wind Speed:\t%1.2f\n", wind_speed);
    /* Lock this with a mutex as well */
    pthread_mutex_lock(&global_data_mutex);
    global_data.wind_speed = wind_speed;
    global_data.updatetime = time(NULL);
    pthread_mutex_unlock(&global_data_mutex);
    prevTime = now;
  }

  return;
}

/*

def aag_winddir(key, path, last_data, ts):
    """Calculate wind direction for AAG TAI8515 V3 wind instrument.
    Contributed by Howard Walter, based on oww C implementation."""
    w = ow.owfs_get("%s%s" % (path, "/volt.ALL"))
    wd = w.split(',')
    wd = [float(x) for x in wd]
    mx = max(x for x in wd)
    wd = [x/mx for x in wd]
    if wd[0] < 0.26:
        if wd[1] < 0.505:
            wdir = 11
        else:
            if wd[3] < 0.755:
                wdir = 13
            else:
                wdir = 12
    else:
        if wd[1] < 0.26:
            if wd[2] < 0.505:
                wdir = 9
            else:
                wdir = 10
        else:
            if wd[2] < 0.26:
                if wd[3] < 0.505:
                    wdir = 7
                else:
                    wdir = 8
            else:
                if wd[3] < 0.26:
                    if wd[0] < 0.755:
                        wdir = 5
                    else:
                        wdir = 6
                else:
                    if wd[3] < 0.84:
                        if wd[2] < 0.84:
                            wdir = 15
                        else:
                            wdir = 14
                    else:
                        if wd[0] < 0.845:
                            if wd[1] < 0.845:
                                wdir = 3
                            else:
                                wdir = 4
                        else:
                            if wd[1] > 0.84:
                                wdir = 0
                            else:
                                if wd[2] > 0.845:
                                    wdir = 2
                                else:
                                    wdir = 1
    return 22.5 * wdir

*/
