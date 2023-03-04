#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"
#include "reader.h"
#include "wordtab.h"

#include "kml.h"

#define ASYNC_READ_BUFFER_SIZE  128
#define CONFIG_BUFFER_SIZE      128
#define DEFAULT_GPS_BAUD        4800

static  config_t    *gl_pConfig;

/*
 * Private
 */

int async_read_config_cb(char *dst, int size, void *data)
{
    FILE *f = (FILE*)data;

    return fread(dst, 1, size, f);
}

int trim_config_line(char *szLine)
{
    int n;
    char    tmp[CONFIG_BUFFER_SIZE];

    // Remove EOL
    for (n = strlen(szLine)-1; n >= 0; n--)
        if ((szLine[n] == '\n') || (szLine[n] == '\r'))
            szLine[n] = 0;

    // Remove space and tab
    for (n = 0; n < strlen(szLine); n++)
        if (szLine[n])
            while ((szLine[n] == ' ') || (szLine[n] == '\t')) {

                strcpy(tmp, &szLine[n+1]);
                strcpy(&szLine[n], tmp);
            }

    return 0; 
}

int lowercase_config_line(char *szLine)
{
    int n;

    // Lower-case everything
    for (n = 0; n < strlen(szLine); n++) {
        if (isupper(szLine[n]))
            szLine[n] = tolower(szLine[n]);
    }

    return 0;
}

int process_config_line(config_t *pConfig, const char *szLine)
{
    wordtab_t   *wt;

    /*
     * Split line
     */

    wt = StrToWordTab(szLine, '=');
    if (!wt || !wt[1]) {

        // Incomplete line, '=' char is missing.
        fprintf(stderr, "[CONFIG] Warning, \"%s\" ignored, bad syntax...\n", szLine);
        return -1;
    }

    lowercase_config_line(wt[0]);

    /*
     * Supported config line?
     */

    if (!strncmp(wt[0], "qra.latitude", 12)) {

        pConfig->myLat = atof(wt[1]);
//        fprintf(stderr, "latitude found: %f\n", pConfig->myLat);
        pConfig->qraAvailable--;
    }

    if (!strncmp(wt[0], "qra.longitude", 13)) {

        pConfig->myLon = atof(wt[1]);
//        fprintf(stderr, "longitude found: %f\n", pConfig->myLon);
        pConfig->qraAvailable--;
    }

    if (!strncmp(wt[0], "qra.altitude", 12)) {

        pConfig->myAlt = atof(wt[1]);
//        fprintf(stderr, "altitude found: %f\n", pConfig->myLon);
        pConfig->qraAvailable--;
    }

    if (!strncmp(wt[0], "gps.out.port", 12)) {

        pConfig->szGpsOutPort = strdup(wt[1]);
//        fprintf(stderr, "gps.out.port found: [%s]\n", pConfig->szGpsOutPort);
    }

    if (!strncmp(wt[0], "gps.out.baud", 12)) {

        pConfig->nGpsOutBaud = atoi(wt[1]);
//        fprintf(stderr, "gps.out.baud: %d\n", pConfig->nGpsOutBaud);
    }

    /*
     * Offsets
     */

    if (!strncmp(wt[0], "time.offset", 11)) {

/***** TODO ******
        tzset();
        fprintf(stdout, "tzname:   [%s] [%s]\n", tzname[0], tzname[1]);
        fprintf(stdout, "timezone: %ld\n", timezone);
        fprintf(stdout, "daylight: %d\n", daylight);
****************/

        pConfig->timeOffset = atoi(wt[1]);
//        fprintf(stderr, "time.offset: %d\n", pConfig->timeOffset);
    }

    if (!strncmp(wt[0], "earth.ellipsoid", 9)) {

        pConfig->dEllipsoid = atof(wt[1]);
    }

    if (!strncmp(wt[0], "header.repeat", 9)) {

        pConfig->headerRepeat = atoi(wt[1]);
        if (pConfig->headerRepeat < 0)
            pConfig->headerRepeat = 0;
    }

    /*
     * Rotor conf lines
     */

    if (!strncmp(wt[0], "rotor.port", 10)) {

        pConfig->rotor.szPort = strdup(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.baud", 10)) {

        pConfig->rotor.nBaud = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.azimuth.init", 18)) {

        pConfig->rotor.nAzimuthInit = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.elevation.init", 20)) {

        pConfig->rotor.nElevationInit = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.azimuth.min", 17)) {

        pConfig->rotor.nAzimuthMin = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.azimuth.max", 17)) {

        pConfig->rotor.nAzimuthMax = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.elevation.min", 19)) {

        pConfig->rotor.nElevationMin = atoi(wt[1]);
    }

    if (!strncmp(wt[0], "rotor.elevation.max", 19)) {

        pConfig->rotor.nElevationMax = atoi(wt[1]);
    }

    DestroyWordTab(wt);

    return 0;
}

/*
 * Public
 */

int Config_init(config_t *pConfig)
{
    if (!pConfig)
        return -1;

    memset(pConfig, 0, sizeof(config_t));
    gl_pConfig = pConfig;

    pConfig->nGpsOutBaud = DEFAULT_GPS_BAUD;
    rotor_init(&pConfig->rotor);

    return 0;
}

int Config_loadFromFile(config_t *pConfig, const char *szFileName)
{
    FILE        *f;
    reader_t    reader;
    char        buff[CONFIG_BUFFER_SIZE];
    int         size;

    /*
     * File available?
     */

    f = fopen(szFileName, "r");
    if (!f) {

//        fprintf(stderr, "[CONFIG] Config file not found [%s]\n", szFileName);
        return CONFIG_FILE_NOT_FOUND;
    }
//    fprintf(stderr, "[CONFIG] %s found\n", szFileName);
    reader_init(&reader, async_read_config_cb, ASYNC_READ_BUFFER_SIZE, (void*)f);

    /*
     * Browse file
     */

    while ((size = reader_getNextLine(&reader, buff, CONFIG_BUFFER_SIZE, '\n'))) {

        trim_config_line(buff);
        if (strlen(buff) && buff[0]!='#')
            process_config_line(pConfig, (const char*)buff);
    }

    if (pConfig->qraAvailable <= -2) {

        pConfig->qraAvailable = 1;
        fprintf(stderr, "[CONFIG] QRA: %f %f\n", pConfig->myLat, pConfig->myLon);
    }
    else if (pConfig->qraAvailable < 0) {
        fprintf(stderr, "[CONFIG] Warning, QRA incomplete.\n");
        pConfig->qraAvailable = 0;
    }
    else if (pConfig->qraAvailable == 0) {
        fprintf(stderr, "[CONFIG] Warning, QRA not found.\n");
        pConfig->qraAvailable = 0;
    }

    /*
     * Release
     */

    reader_release(&reader);
    fclose(f);

    return CONFIG_SUCCESS;
}

int Config_overwriteQRA(config_t *pConfig, const char *optarg)
{
    wordtab_t   *wt;
    int wordCount;

    if (!pConfig || !optarg)
        return -1;

    wt = StrToWordTab(optarg, ':');
    wordCount = GetWTWordNumber(wt);

    if ((wordCount < 2) || (wordCount > 3)) {

        fprintf(stderr, "[CONFIG] Overwrite QRA: /!\\ Syntax error /!\\ (%s)\n", optarg);
    }
    else {

        pConfig->myLat = atof( wt[0] );
        pConfig->myLon = atof( wt[1] );
        if (wordCount == 3)
            pConfig->myAlt = atof( wt[2] );
    
        fprintf(stderr, "[CONFIG] QRA overwritten: %f %f %.0fm\n", pConfig->myLat, pConfig->myLon, pConfig->myAlt);
        pConfig->qraAvailable = 1;
    }

    DestroyWordTab(wt);

    return 0;
}

int Config_clean(config_t *pConfig)
{
    if (!pConfig)
        return -1;

    if (pConfig->pKmlFile) {

        Kml_writeTail(pConfig->pKmlFile);
        fclose(pConfig->pKmlFile);
        pConfig->pKmlFile = NULL;
    }
    if (pConfig->szKmlFileName) {
        free(pConfig->szKmlFileName);
        pConfig->szKmlFileName = NULL;
    }
    if (pConfig->szNmeaFileName) {

        free(pConfig->szNmeaFileName);
        pConfig->szNmeaFileName = NULL;
    }
    if (pConfig->szGpsOutPort) {
        free(pConfig->szGpsOutPort);
        pConfig->szGpsOutPort = NULL;
    }
    if (pConfig->rotor.szPort) {
        rotor_release(&pConfig->rotor);
    }

    return 0;
}

const config_t* Config_getConfig(void)
{
    return gl_pConfig;
}
