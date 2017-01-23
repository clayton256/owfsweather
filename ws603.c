#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "owfsweather.h"

extern pthread_mutex_t global_data_mutex;

void init_wsi(int mem_fd) {

  char memory[MEMSIZE];
  int i;

  read(mem_fd,memory,MEMSIZE);

  memory[0x80] = READ_DATA_CMD;
  memory[0x81] = READ_DATA_CMD;  /* Checksum for this is easy */
  memory[0x82] = DATA_END_BYTE;

  lseek(mem_fd,0,SEEK_SET);

  i = write(mem_fd,memory,MEMSIZE);

  app_debug(debug, "init_wsi:  Wrote %i bytes to device\n",i);

}



void update_wsi_data (int mem_fd, int temp_fd) {

  unsigned char memory[MEMSIZE];
  char temp[6];



  int dir;
  float speed;
  float realtemp;

  /* rewind the file pointers back to 0, just to be certain */

  lseek(mem_fd,0,SEEK_SET);
  //lseek(temp_fd,6,SEEK_SET);

  read(mem_fd,memory,MEMSIZE);
  read(temp_fd,temp,6);

  /* parse out the blob from the memory cell */


  speed = (float)memory[0x89] * 2.453 * 1.069 * 1000 / 3600;  /* Returns m/s */
  dir = (int)memory[0x8a];
  realtemp = atof(temp);

  app_debug(debug, "\nBEGIN WS603 READ\n");



  if ((speed == (float)255) || (dir > 16))
    {
      app_debug(info, "Got garbage from the wind system.  Rejecting\n");
    } else {
      app_debug(debug, "Wind Speed: %2.1f m/s\n",speed);
      app_debug(debug, "Wind Dir:   %i\n",dir);
      app_debug(debug, "Light:     %2.1f\n",(float)memory[0x8c]);
      pthread_mutex_lock(&global_data_mutex);
      global_data.wind_dir = dir;
      global_data.wind_speed = speed;
      global_data.updatetime = time(NULL);
      pthread_mutex_unlock(&global_data_mutex);

    }

  if (global_config.ignore_wsi_temp == 0) {

    /* Sanity check here.  Over 150F or exactly 0.000F means bogus data */
    /* If it is over 150F, you've got bigger problems than bad data */
    /* And it is unlikely the temp sensor is *exactly* 0.000F */

    if ((realtemp > (long)150) || (realtemp == (long)0.000))
      {
        app_debug(info, "WSI:  Got garbage from the WSI603 temp sensor.  Rejecting\n");
      } else {
        app_debug(debug, "Temp:       %2.1f\n",realtemp);
        /* Lock this with a mutex as well */
        pthread_mutex_lock(&global_data_mutex);
        global_data.temp = realtemp;
        global_data.updatetime = time(NULL);
        pthread_mutex_unlock(&global_data_mutex);
    }
  }

  app_debug(debug, "END WSI603 READ\n\n");

}


void wsi_set_leds (int mem_fd, int blueLevel, int redLevel) {

  char memory[MEMSIZE];

  lseek(mem_fd,0,SEEK_SET);

  read(mem_fd,memory,MEMSIZE);

  /* Twiddle the bits to twiddle the LEDS */

  app_debug(debug, "LED Control:   Blue %i   Red: %i\n",blueLevel, redLevel);

  memory[0x80] = LED_CNTRL_CMD;
  memory[0x81] = 0x00;
  memory[0x82] = (0x80 | (redLevel << 4) | blueLevel);
  memory[0x83] = 0x00;
  memory[0x84] = 0x00;
  memory[0x85] = memory[0x80] + memory[0x81] + memory[0x82] + memory[0x83] + memory[0x84];  /* Checksum */
  memory[0x86] = LED_END_BYTE;


  lseek(mem_fd,0,SEEK_SET);  /* Rewind before write */
  write(mem_fd,memory,MEMSIZE);
  lseek(mem_fd,0,SEEK_SET);  /* Rewind before write */
  write(mem_fd,memory,MEMSIZE);

  init_wsi(mem_fd);  /* reset back to data gathering mode */

}


