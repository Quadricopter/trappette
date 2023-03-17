#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdio.h>
#include <stdbool.h>
#include "serial.h"
#include "watchdog.h"
#include "rotor.h"

#define DEFAULT_CONFIG_FILE     ".trappette.cfg"

/*
 * Return values
 */

enum {

    CONFIG_SUCCESS = 0,
    CONFIG_FILE_NOT_FOUND
};

/*
 *
 */

typedef struct {

    /* Dump Offsets/Settings */
    bool    bShowUTC;
    double  dEllipsoid;
    int     headerRepeat;

    /* Google Earth - kml output */
    char    *szKmlFileName;
    FILE    *pKmlFile;

    /* Tracking */
    int     qraAvailable;
    double  myLat;
    double  myLon;
    double  myAlt;

    /* NMEA Output (file) */
    char        *szNmeaFileName;

    /* NMEA Output (usart) */
    int         enableGpsOut;
    serial_t    serialGpsOut;
    char        *szGpsOutPort;
    int         nGpsOutBaud;

    /* Rotor */
    int         enableRotor;
    rotor_t     rotor;

    /* Watchdog */
    watchdog_t  watchdog;
}   config_t;


/*
 *
 */

int Config_init(config_t *pConfig);
int Config_loadFromFile(config_t *pConfig, const char *szFileName);
int Config_clean(config_t *pConfig);
int Config_overwriteQRA(config_t *pConfig, const char *optarg);

/*
 * Return config_t pointer, provided to Config_init()
 */

const config_t* Config_getConfig(void);


#endif /*__CONFIG_H__*/
