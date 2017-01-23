#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>

#include "owfsweather.h"

static FILE *logfile = NULL;


void app_debug_on(enum log_level lvl, char *fmt, ...)
{
  va_list ap;
//  char lmsg[1024];
//  time_t current_time;
//  char* c_time_string;

//  memset(lmsg, '\0', 1024);

  /* Obtain current time. */
//  current_time = time(NULL);

//  if (current_time != ((time_t)-1))
//  {
    /* Convert to local time format. */
//    c_time_string = ctime(&current_time);

//    if (c_time_string != NULL)
//    {
//      sprintf(lmsg,"%s: ", c_time_string);
//      strl = strlen(lmsg);
//      va_start(ap,fmt);
//      vfprintf(lmsg+strl,fmt,ap);
//      fflush(logfile);
//      va_end(ap);
//    }
//  }

  if (lvl <= global_config.debug)
  {
      if (1 == global_config.dont_fork)
      {
          logfile = stderr;
      }
      else
      {
          if(NULL == global_config.logfile)
              return;
          logfile = fopen(global_config.logfile, "a");
      }
      if(NULL != logfile)
      {
          va_start(ap,fmt);
          vfprintf(logfile,fmt,ap);
          fflush(logfile);
          va_end(ap);
      }
      else
      {
          fprintf(stderr, "logfile null %s\n", global_config.logfile);
      }
      if (1 == global_config.dont_fork)
      {
          logfile = NULL;
      }
      else
      {
          if (NULL != logfile)
          {
              fclose(logfile);
          }
      }
  }
  return;
}


void app_debug_off(enum log_level lvl, char *fmt, ...) {return;}

