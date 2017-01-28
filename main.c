/*
 * Michael Leyssenne
 * PSK Demodulation for M10 Weather Balloon
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#ifdef ENABLE_WATCHDOG
#include "watchdog.h"
#endif

#include "kml.h"
#include "wgs84.h"
#include "config.h"
#include "gps.h"
#include "version.h"
#include "m10.h"

#define GNUPLOT_SEP     '\t'

#define EXPECTED_BIT_RATE       48000
#define EXPECTED_BIT_PER_SAMPLE 16
#define SAMPLE_TO_READ      1024

//#define DEBUG_OUTPUT

/*
 * -----------------------
 */

void usage(char *prgname)
{   
    fprintf(stderr, "Usage: %s [-k kmlfile] [-n nemafile] [-gpsout] [-hex] [-v]\n", prgname);
#ifdef ENABLE_WATCHDOG
    fprintf(stderr, "                   [-a seconds] [-t seconds]\n");
#endif
    fprintf(stderr, "                   [-q lat:lon[:alt]]\n");
    fprintf(stderr, " -k: specifie an .kml output file\n");
    fprintf(stderr, " -n: specifie an .nema gps output file\n");
    fprintf(stderr, " -gpsout: NEMA output on serial port\n");
    fprintf(stderr, " -hex: hexadecimal dump\n");
    fprintf(stderr, " -v: verbose\n");
#ifdef ENABLE_WATCHDOG
    fprintf(stderr, " -a: abort time ( nothing received at all )\n");
    fprintf(stderr, " -t: time out ( after last received position )\n");
#endif
    fprintf(stderr, " -q: overwrite QRA position\n");
}

void sigkillhandler(int i) // Ctrl+C or Timer
{
    config_t    *pConfig;
#ifdef ENABLE_WATCHDOG
    time_t      t;
    struct tm   *pTm;
#endif

//    fprintf(stderr, "%s(%d)\n", __FUNCTION__, i);

    pConfig = (config_t*) Config_getConfig();

#ifdef ENABLE_WATCHDOG

    if (i == SIGALRM) {

        time(&t);
        t += pConfig->timeOffset;
        pTm = gmtime(&t);
        fprintf(stderr, "# Watchdog [%02d:%02d:%02d]", pTm->tm_hour, pTm->tm_min, pTm->tm_sec);
        if (pConfig->timeOffset == 0)
            fprintf(stderr, " UTC");
        fprintf(stderr, "\n");
        Watchdog_delete(&pConfig->watchdog);
    }

#endif

    if (pConfig && pConfig->pKmlFile) {

        Kml_writeTail(pConfig->pKmlFile);
        fclose(pConfig->pKmlFile);
        pConfig->pKmlFile = NULL;
    }

    // TODO: fix dirty...
    exit(EXIT_SUCCESS);
}


void    printHeader(void)
{
    printf("   TIME   |   LAT   |   LONG   | H SPEED | ALT  | V SPEED |  AZ  | ELE |  DIST\n");
}

/*
 * -----------------------
 * New full trame callback
 * -----------------------
 */

int tsip_dump_cb(const tsip_t *tsip, void *data)
{
    config_t    *pConfig = (config_t*)data;
    double      azimuth, elevation, distance;
    wgs84_t     myPos, tsipPos;
    char        gga[128];
    char        rmc[128];
    FILE        *nmeaFILE;
    time_t      myLocalEpoch;
    char        myLocaTimeString[32];
    struct tm   *ptm;
    static  int idx = 0;

//    fprintf(stdout, "%s(%p, %p)\n", __FUNCTION__, tsip, data);

    assert(pConfig);

#ifdef ENABLE_WATCHDOG

    /*
     * Kick the dog ;-)
     */

    Watchdog_kick(&pConfig->watchdog);

#endif

    /*
     * Header repeat
     */

    if (pConfig->headerRepeat) {

        if (idx && !(idx%pConfig->headerRepeat))
            printHeader();
    }
    idx++;

    /*
     * Console dump
     */

    myLocalEpoch = tsip->unixEpoch + pConfig->timeOffset;
    ptm = gmtime(&myLocalEpoch);
    sprintf(myLocaTimeString, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    printf("[%s] %8.5f%c %9.5f%c %5.1fkm/h %5.0fm % 6.2fm/s",   myLocaTimeString,
                                                            tsip->dLatitude>=0.f?tsip->dLatitude:-tsip->dLatitude,
                                                            tsip->dLatitude>=0.f?'N':'S',
                                                            tsip->dLongitude>=0.f?tsip->dLongitude:-tsip->dLongitude,
                                                            tsip->dLongitude>=0.f?'E':'W',
                                                            tsip->dGroundSpeedMs * 3.6f, // To km/h
                                                            tsip->dAltitude - pConfig->dEllipsoid,
                                                            tsip->dClimbRateMs);

    if (tsip->bIsValidChecksum == false) {

        printf("\t/!\\\n");
        return 0;
    }

    if (pConfig->qraAvailable == 1) {

        /*
         * Compute azimuth and distance...
         */

        wgs84_setCoordinate(&myPos, pConfig->myLat, pConfig->myLon, 0.f);
        wgs84_setCoordinate(&tsipPos, tsip->dLatitude, tsip->dLongitude, tsip->dAltitude - pConfig->dEllipsoid);
        wgs84_computeHeadingAndDistance(&myPos, &tsipPos, &azimuth, &distance);

        /*
         * ... Compute elevation ( naive calculation: earth is flat =) ) ...
         */

        elevation = radToDeg( atan2(tsip->dAltitude - pConfig->dEllipsoid - pConfig->myAlt, distance*1000.f) );
        if (elevation < 0.f)
            elevation = 0.f;

        /*
         * ... and dump it!
         */

        printf(" %5.1f° %4.1f° ", azimuth, elevation);
        if (distance < 100.f)
            printf("%6.3fkm", distance);
        else
            printf("%6.1fkm", distance);
    }
    printf("\n");

    /*
     * Write kml
     */


    if (pConfig->szKmlFileName && tsip->bIsValidChecksum == true) {

        /* First time? */
        if (pConfig->pKmlFile == NULL) {

            /* Open kml output file */
            pConfig->pKmlFile = fopen(pConfig->szKmlFileName, "w");
            if (!pConfig->pKmlFile) {

                fprintf(stderr, "Can't open %s kml file\n", pConfig->szKmlFileName);
                exit(EXIT_FAILURE);
            }

            Kml_writeHeader(pConfig->pKmlFile);
        }

        Kml_writeEntry(pConfig->pKmlFile, tsip);
    }

    /*
     * GPS Output
     */
    
    if ((pConfig->enableGpsOut || pConfig->szNmeaFileName) && (tsip->bIsValidChecksum == true)) {

        gps_tsipToNmeaFormat(gga, rmc, tsip, pConfig->dEllipsoid);

        if (pConfig->szNmeaFileName) {

            nmeaFILE = fopen(pConfig->szNmeaFileName, "a");
            if (nmeaFILE) {

//                fprintf(stderr, "%s", gga);
//                fprintf(stderr, "%s", rmc);
                fprintf(nmeaFILE, "%s", gga);
                fprintf(nmeaFILE, "%s", rmc);
                fclose(nmeaFILE);
            }
        }

        if (pConfig->enableGpsOut) {

            if (serial_write(&pConfig->serialGpsOut, gga, strlen(gga)) != strlen(gga)) {

                fprintf(stderr, "serial_write(gga) failed\n");
                exit(EXIT_FAILURE);
            }
            if (serial_write(&pConfig->serialGpsOut, rmc, strlen(rmc)) != strlen(rmc)) {

                fprintf(stderr, "serial_write(rmc) failed\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    /*
     * Rotor
     */

    if (pConfig->enableRotor == 1 && pConfig->rotor.szPort != NULL &&
        pConfig->qraAvailable == 1 && tsip->bIsValidChecksum == true) {

        rotor_move(&pConfig->rotor, azimuth+.5f, elevation+.5f);
    }

    return 0;
}

int stream_dump_cb(const uint8_t *stream, uint16_t size, void *data)
{
    uint16_t    n;

    /*
     * Hexadecimal dump
     */

    for (n = 0; n < size; n++) {

        if (n == 2)
            fprintf(stdout, " ");
        fprintf(stdout, "%02X", stream[n]);
        if (n == 34)
            fprintf(stdout, " ");
    }
    fprintf(stdout, " (%d)\n", size);

    return 0;
}

/*
 * -----------------------
 */

unsigned int    getSecondsFromParamString(const char *szString)
{
    unsigned int    seconds;
    char    str[32];
    int n, mul;

    strncpy(str, szString, 32);
    for (seconds = 0, n = 0, mul = 1; n < strlen(str); n++) {

/*        if (str[n] == ' ') {

            str[n] = 0;
            break;
        } */

        if (isalpha(str[n])) {

            if (str[n] == 's' || str[n] == 'S')
                mul = 1;
            if (str[n] == 'm' || str[n] == 'M')
                mul = 60;
            if (str[n] == 'h' || str[n] == 'H')
                mul = 3600;
            if (str[n] == 'd' || str[n] == 'D')
                mul = 86400;
            str[n] = 0;
            break;
        }
    }

    seconds = atoi(str);
    seconds *= mul;

    return seconds;
}

/*
 * -----------------------
 */

int main(int ac, char *av[])
{
    m10_t       m10ctx;
    config_t    config;
    int16_t     samples[SAMPLE_TO_READ];
    int         stop, samplesRead, opt;
#ifdef ENABLE_WATCHDOG
    int         nAbortSecond, nTimeoutSecond;
#endif

#ifndef ENABLE_WATCHDOG
    fprintf(stderr, "# Trappette v%d.%d.%d\n", TRAPPETTE_VERSION_MAJOR, TRAPPETTE_VERSION_MINOR, TRAPPETTE_VERSION_REVISION);
#else
    fprintf(stderr, "# Trappette v%d.%d.%d [Watchdog]\n", TRAPPETTE_VERSION_MAJOR, TRAPPETTE_VERSION_MINOR, TRAPPETTE_VERSION_REVISION);
#endif /*ENABLE_WATCHDOG*/

    M10_init(&m10ctx);

    /*
     * Init, load configuration file
     */

    Config_init(&config);
    if (Config_loadFromFile(&config, "." DEFAULT_CONFIG_FILE) != CONFIG_SUCCESS) {

        if (Config_loadFromFile(&config, DEFAULT_CONFIG_FILE) != CONFIG_SUCCESS) {

            fprintf(stderr, "[CONFIG] Config file not found [%s]\n", DEFAULT_CONFIG_FILE);
        }
    }

#ifdef ENABLE_WATCHDOG
    Watchdog_init(&config.watchdog, sigkillhandler); 
    nAbortSecond = 0;
    nTimeoutSecond = 0;
#endif

    /*
     * Check parameters
     */

#ifdef ENABLE_WATCHDOG
    while ((opt = getopt(ac, av, "q:h:i:k:n:g:vr:a:t:")) != -1) {
#else
    while ((opt = getopt(ac, av, "q:h:i:k:n:g:vr:")) != -1) {
#endif
        switch (opt) {

            case 'k':
                config.szKmlFileName = strdup(optarg);
                break;
            case 'h':
                if (!strncmp(optarg, "ex", 2)) {
                    M10_setStreamCallback(&m10ctx, stream_dump_cb, NULL);
                }
                else {
                    usage(av[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'n':
                config.szNmeaFileName = strdup(optarg);
                break;
            case 'g':
                if (!strncmp(optarg, "psout", 5)) {
                    config.enableGpsOut = 1;
                }
                break;
            case 'r':
                if (!strncmp(optarg, "otor", 4)) {
                    config.enableRotor = 1;
                }
                break;
            case 'v':
                M10_setVerboseLevel(&m10ctx, 1);
                break;
#ifdef ENABLE_WATCHDOG
            case 'a':
                nAbortSecond = getSecondsFromParamString(optarg);
                break;
            case 't':
                nTimeoutSecond = getSecondsFromParamString(optarg);
                break;
#endif
            case 'q':
                Config_overwriteQRA(&config, optarg);
                break;
            default: /* '?' */
                usage(av[0]);
                exit(EXIT_FAILURE);
        }
    }

    fprintf(stderr, "# Read %dHz/%dbits on stdin\n", EXPECTED_BIT_RATE, EXPECTED_BIT_PER_SAMPLE);

    /*
     * Ctrl+C handler
     */

    if (signal(SIGINT, sigkillhandler) == SIG_ERR) {
    
        fprintf(stderr, "signal failed...\n");
        exit(EXIT_FAILURE);
    }

    /*
     * GPS output (serial)
     */

    if (config.enableGpsOut == 1) {

        if (!config.szGpsOutPort) {

            fprintf(stderr, "ERROR gps.out.port undefined\n");
            exit(EXIT_FAILURE);
        }

        fprintf(stderr, "[CONFIG] GPSOUT: %s ( %d bauds )\n", config.szGpsOutPort, config.nGpsOutBaud);
        if (serial_init(&config.serialGpsOut) != SERIAL_OK) {

            fprintf(stderr, "serial_init() failed\n");
            exit(EXIT_FAILURE);
        }

        if (serial_open(&config.serialGpsOut, config.szGpsOutPort, config.nGpsOutBaud) != SERIAL_OK) {

            fprintf(stderr, "serial_open(%s, %d) failed\n", config.szGpsOutPort, config.nGpsOutBaud);
            exit(EXIT_FAILURE);
        }
    }

    /*
     * Rotor
     */

    if (config.enableRotor && config.rotor.szPort) {

        rotor_check_params(&config.rotor);

        if (config.qraAvailable) {

            rotor_open(&config.rotor);
            rotor_move(&config.rotor, config.rotor.nAzimuthInit, config.rotor.nElevationInit);
        }
        else {

            fprintf(stderr, "Can't use rotor, QRA not set...\n");
            config.enableRotor = 0;
        }
    }

#ifdef ENABLE_WATCHDOG

    /*
     * Abort/Timeout timer
     */

    Watchdog_set(&config.watchdog, nAbortSecond, nTimeoutSecond);

#endif

    /*
     * Almost infinite loop
     */

    M10_setTsipCallback(&m10ctx, tsip_dump_cb, (void*)&config);

    printHeader();

    stop = 0;
    while (stop == 0) {

        /*
         * Read samples from stdin
         */

        samplesRead = read(0, &samples, SAMPLE_TO_READ*sizeof(int16_t)) / 2;

        if (samplesRead <= 0) {

#ifdef DEBUG_OUTPUT
            fprintf(stderr, "\nnothing more to read (%d)\n", samplesRead);
#endif
            stop = 1;
        }

        /*
         * Go!
         */

        M10_process16bit48k(&m10ctx, samples, samplesRead);
    }

    /*
     * Flush PSK buffer
     */

    M10_release(&m10ctx);

    /*
     * Clean...
     */

#ifdef ENABLE_WATCHDOG
    Watchdog_delete(&config.watchdog);
#endif

    Config_clean(&config);

//    printf("Max alt: %.0fm\n", demod.dMaxAltitude);

    return EXIT_SUCCESS;
}