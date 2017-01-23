


#ifndef OWFSINCLUDE_H
#define OWFSINCLUDE_H

#define TRUE  1
#define FALSE 0

enum log_level
{
    quiet,
    info,
    warning,
    critical,
    debug,
};

struct owfsconfig {
  char *owfsroot;
  char *wsi_serial;
  char *vane_serial;
  char *anem_serial;
  char *temp_serial;
  char *temp2_serial;
  char *baro_serial;
  char *rain_serial;
  char *humidity_serial;

  int ws_mem_fd;
  int ws_temp_fd;
  int temp_fd;
  int temp2_fd;
  int baro_fd;
  int humidity_fd;
  int rain_fd;
  int anem_fd;
  int vane_fd;

  int ignore_wsi_temp;
  int speed_thresh;

  int debug;
  int dont_fork;

  char *outfile;
  int log_interval;
  int log_overwrite;

  char *logfile;
  int log_level;

  char *wu_username;
  char *wu_password;
  char *jmc_username;
  char *jmc_password;
  int wu_rapid_fire;
  int wu_update_interval;
  char *rrd_file;
  int rrd_interval;
  time_t reset_tofd;
  char *reset_tofd_str;
  char *test_config;
  char *watchdog;
  int vane_calib;
  char *wind_calib;
  char *rain_calib;
} global_config;


struct owfsdata {
  time_t updatetime;
  int wind_dir;
  float wind_speed;
  float temp;
  float temp2;
  float rain;
  float rainrate;
  float barometer;
  int humidity;
  float wind_avg;
  int wind_avg_dir;
  float wind_gust;
  int wind_gust_dir;
} global_data;

struct app_data
{
    pthread_mutex_t data_mutex;
    struct owfsconfig config_data;
    struct owfsdata wx_data;
};


/* Configuration struct macros */
/* Coder laziness & control typos */

#define WSI_SERIAL global_config.wsi_serial
#define TEMP_SERIAL global_config.temp_serial
#define BARO_SERIAL global_config.baro_serial
#define ANEM_SERIAL global_config.anem_serial
#define VANE_SERIAL global_config.vane_serial
#define RAIN_SERIAL global_config.rain_serial
#define HUMID_SERIAL global_config.humidity_serial
#define WU_ID global_config.wu_username
#define WU_PWD global_config.wu_password
#define JMC_ID global_config.jmc_username
#define JMC_PWD global_config.jmc_password

/* Weather data struct macros */

#define UPDATETM global_data.updatetime
#define WINDDIR global_data.wind_dir
#define WINDSPD global_data.wind_speed
#define TEMP global_data.temp
#define TEMP2 global_data.temp2
#define BARO global_data.barometer
#define RAIN global_data.rain
#define RAINRATE global_data.rainrate
#define HUMID global_data.humidity
#define WINDAVG global_data.wind_avg
#define WINDAVGDIR global_data.wind_avg_dir
#define WINDGUST global_data.wind_gust
#define WINDGUSTDIR global_data.wind_gust_dir


#define MEMSIZE 256    /* WS603A has 256 bytes of memory mapped space */

#define WRITE_ADDR 0x80
#define READ_ADDR 0x88

#define READ_PARAM_CMD 0xA0
#define READ_DATA_CMD 0xA1
#define LED_CNTRL_CMD 0xA2

#define DATA_END_BYTE 0x1E
#define LED_END_BYTE 0x2E

#define LED_HIGH 0x07
#define LED_MED 0x03
#define LED_LOW 0x01
#define LED_OFF 0x00


/* Prototypes */
int parse_config_file(char *);
void app_debug(enum log_level, char *fmt, ...);
void update_wsi_data (int mem_fd, int temp_fd);
void update_humidity_data (int humidity_fd);
void update_temp_data (int temp_fd);
void update_temp2_data (int temp_fd);
void update_baro_data (int baro_fd);
void update_vane_data (int vane_fd);
void update_anem_data (int anem_fd);
void update_rain_data (int rain_fd);

void * start_logging(void *);
void * start_watchdog(void *);

void * start_rrd_logging(void *);

void * start_wu_update(void *);
void * start_jmc_update(void *);

void wsi_set_leds (int mem_fd, int blueLevel, int redLevel);

void daemonize(void);

int open_1wire_fd(char *sn, char *subdev);
int open_anem_device(char * device);
int open_rain_device(char * device);

/* End prototypes */

void app_debug_on(enum log_level lvl, char *fmt, ...);
void app_debug_off(enum log_level lvl, char *fmt, ...); 

#ifdef APP_DEBUG_FLAG
#define app_debug app_debug_on
#warning DEBUG FLAG is on
#else
#define app_debug app_debug_off
#endif

#endif
