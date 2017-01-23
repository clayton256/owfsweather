#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#define __USE_XOPEN
#include <time.h>
#include "confuse.h"
#include "owfsweather.h"




int parse_config_file (char *config_file) {

  int success = FALSE;
  //char * reset_tofd_str;

  //global_config = (struct owfsconfig *)malloc(sizeof(struct owfsconfig));

  /* Defaults */
  global_config.ignore_wsi_temp = 0;
  global_config.owfsroot = strdup("/1wire");
  global_config.speed_thresh = 5;
  global_config.wu_rapid_fire = 1;
  global_config.wu_update_interval = 5;
  global_config.log_overwrite = 0;
  global_config.rrd_interval = 120;

  cfg_opt_t opts[] = {
    CFG_SIMPLE_INT("debug", &global_config.debug),
    CFG_SIMPLE_STR("owfsroot", &global_config.owfsroot),
    CFG_SIMPLE_STR("wsi_serial", &global_config.wsi_serial),
    CFG_SIMPLE_STR("rain_serial", &global_config.rain_serial),
    CFG_SIMPLE_STR("anem_serial", &global_config.anem_serial),
    CFG_SIMPLE_STR("vane_serial", &global_config.vane_serial),
    CFG_SIMPLE_STR("temp_serial", &global_config.temp_serial),
    CFG_SIMPLE_STR("temp2_serial", &global_config.temp2_serial),
    CFG_SIMPLE_STR("baro_serial", &global_config.baro_serial),
    CFG_SIMPLE_STR("humidity_serial", &global_config.humidity_serial),
    CFG_SIMPLE_BOOL("ignore_wsi_temp", &global_config.ignore_wsi_temp),
    CFG_SIMPLE_STR("wu_username", &global_config.wu_username),
    CFG_SIMPLE_STR("wu_password", &global_config.wu_password),
    CFG_SIMPLE_STR("jmc_username", &global_config.jmc_username),
    CFG_SIMPLE_STR("jmc_password", &global_config.jmc_password),
    CFG_SIMPLE_INT("speed_threshold", &global_config.speed_thresh),
    CFG_SIMPLE_STR("outfile", &global_config.outfile),
    CFG_SIMPLE_INT("log_interval", &global_config.log_interval),
    CFG_SIMPLE_BOOL("log_overwrite", &global_config.log_overwrite),
    CFG_SIMPLE_STR("rrd_file", &global_config.rrd_file),
    CFG_SIMPLE_INT("rrd_log_interval", &global_config.rrd_interval),
    CFG_SIMPLE_BOOL("wu_rapid_fire", &global_config.wu_rapid_fire),
    CFG_SIMPLE_INT("wu_update_interval", &global_config.wu_update_interval),
    CFG_SIMPLE_STR("reset_time", &global_config.reset_tofd_str),
    CFG_SIMPLE_STR("test_config", &global_config.test_config),
    CFG_SIMPLE_STR("logfile", &global_config.logfile),
    CFG_SIMPLE_INT("log_level", &global_config.log_level),
    CFG_SIMPLE_STR("watchdog", &global_config.watchdog),
    CFG_SIMPLE_INT("vane_calibration", &global_config.vane_calib),
    CFG_SIMPLE_STR("wind_calibration", &global_config.wind_calib),
    CFG_SIMPLE_STR("rain_calibration", &global_config.rain_calib),

    CFG_END()
  };
  cfg_t *cfg;

  if ((config_file == NULL) || (strcmp(config_file,"") == 0))
    config_file = strdup("/etc/owfsweather.conf");

  app_debug(info, "Opening configuration file: %s\n", config_file);

  cfg = cfg_init(opts,0);
  if(NULL != cfg) 
  {
    if (CFG_SUCCESS != cfg_parse(cfg, config_file))
    {
        app_debug(info, "Error opening configuration file: %s\n", config_file);
        success = FALSE;
    }
    cfg_free(cfg);

    /* Display the results if debug == 1; */
    app_debug(debug, "\n");
    app_debug(debug, "Configuration settings:\n");
    app_debug(debug, "    OWFS Root: %s\n",global_config.owfsroot);
    app_debug(debug, "    WS603 Serial: %s (Ignoring onboard temp: %s)\n",global_config.wsi_serial, global_config.ignore_wsi_temp ? "Yes" : "No");
    app_debug(debug, "    Anem Sensor Serial: %s\n",global_config.anem_serial);
    app_debug(debug, "    Vane Sensor Serial: %s\n",global_config.vane_serial);
    app_debug(debug, "    Temp Sensor Serial: %s\n",global_config.temp_serial);
    app_debug(debug, "    Temp2 Sensor Serial: %s\n",global_config.temp2_serial);
    app_debug(debug, "    Barometer Serial: %s\n",global_config.baro_serial);
    app_debug(debug, "    Rain Serial: %s\n",global_config.rain_serial);
    app_debug(debug, "    Rain Reset : %s\n",global_config.reset_tofd);
    app_debug(debug, "    Rain Reset : %s\n",global_config.reset_tofd_str);
    app_debug(debug, "    Humidity Serial: %s\n",global_config.humidity_serial);
    app_debug(debug, "    WU Username: %s\n",global_config.wu_username);
    app_debug(debug, "    WU Password: %s\n",global_config.wu_password);
    app_debug(debug, "    JMC Username: %s\n",global_config.jmc_username);
    app_debug(debug, "    JMC Password: %s\n",global_config.jmc_password);
    app_debug(debug, "    Log File:  %s\n",global_config.outfile);
    app_debug(debug, "    Log Interval:  %i\n",global_config.log_interval);
    app_debug(debug, "    RRD Filename:  %s\n",global_config.rrd_file);
    app_debug(debug, "    RRD Logging:  %i\n",global_config.rrd_interval);
    app_debug(debug, "    Log File:  %s\n",global_config.logfile);
    app_debug(debug, "    Logging Level:  %i\n",global_config.log_level);
    app_debug(debug, "    Dont fork:  %i\n",global_config.dont_fork);
    app_debug(debug, "    Watchdog: %s\n", global_config.watchdog);
    app_debug(debug, "    vane_calib: %i\n", global_config.vane_calib);
    app_debug(debug, "    wind_calib: %s\n", global_config.wind_calib);
    app_debug(debug, "    rain_calib: %s\n", global_config.rain_calib);
  }
  else
  {
    app_debug(info, "Error allocating config structure");
    success = FALSE;
  }

  return success;
};
