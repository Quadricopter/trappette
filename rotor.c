#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "rotor.h"


/*
 *
 */

void    rotor_init(rotor_t *rotor)
{
    assert(rotor);

    memset(rotor, 0, sizeof(rotor_t));
    rotor->nAzimuthMin   = ROTOR_AZ_MIN;
    rotor->nAzimuthMax   = ROTOR_AZ_MAX;
    rotor->nElevationMin = ROTOR_EL_MIN;
    rotor->nElevationMax = ROTOR_EL_MAX;

    rotor->nAzimuthInit   = -1;
    rotor->nElevationInit = -1;
}

int     rotor_check_params(rotor_t *rotor)
{
    int r = 0;

    assert(rotor);

    /*
     * Azimuth
     */

    if (rotor->nAzimuthMin < ROTOR_AZ_MIN) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.azimuth.min\" < %d (%d)\n", ROTOR_AZ_MIN, rotor->nAzimuthMin);
        rotor->nAzimuthMin = ROTOR_AZ_MIN;
        r++;
    }

    if (rotor->nAzimuthMax > ROTOR_AZ_MAX) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.azimuth.max\" > %d (%d)\n", ROTOR_AZ_MAX, rotor->nAzimuthMax);
        rotor->nAzimuthMax = ROTOR_AZ_MAX;
        r++;
    }

    if (rotor->nAzimuthMin >= rotor->nAzimuthMax ) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.azimuth.min\" > \"rotor.azimuth.max\"...\n");
        rotor->nAzimuthMin = ROTOR_AZ_MIN;
        rotor->nAzimuthMax = ROTOR_AZ_MAX;
        r++;
    }

    if ((rotor->nAzimuthInit < rotor->nAzimuthMin) ||
        (rotor->nAzimuthInit > rotor->nAzimuthMax)) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.azimuth.init\" out of range [%d,%d] (%d)\n",
                        rotor->nAzimuthMin, rotor->nAzimuthMax, rotor->nAzimuthInit);
        rotor->nAzimuthMax = -1;
        r++;
    }

    /*
     * Elevation
     */

    if (rotor->nElevationMin < ROTOR_EL_MIN) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.elevation.min\" < %d (%d)\n", ROTOR_EL_MIN, rotor->nElevationMin);
        rotor->nElevationMin = ROTOR_EL_MIN;
        r++;
    }

    if (rotor->nElevationMax > ROTOR_EL_MAX) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.elevation.max\" > %d (%d)\n", ROTOR_EL_MAX, rotor->nElevationMax);
        rotor->nElevationMax = ROTOR_EL_MAX;
        r++;
    }

    if (rotor->nElevationMin >= rotor->nElevationMax ) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.elevation.min\" > \"rotor.elevation.max\"...\n");
        rotor->nElevationMin = ROTOR_EL_MIN;
        rotor->nElevationMax = ROTOR_EL_MAX;
        r++;
    }

    if ((rotor->nElevationInit < rotor->nElevationMin) ||
        (rotor->nElevationInit > rotor->nElevationMax)) {

        fprintf(stderr, "[CONFIG] /!\\ Warning /!\\ \"rotor.elevation.init\" out of range [%d,%d] (%d)\n",
                        rotor->nElevationMin, rotor->nElevationMax, rotor->nElevationInit);
        rotor->nElevationMax = -1;
        r++;
    }

    return r;
}

int     rotor_open(rotor_t *rotor)
{
    assert(rotor);

    if (serial_init(&rotor->serial) != SERIAL_OK) {

        fprintf(stderr, "serial_init() failed\n");
        exit(EXIT_FAILURE);
    }
    if (serial_open(&rotor->serial, rotor->szPort, rotor->nBaud) != SERIAL_OK) {

        fprintf(stderr, "serial_open(%s, %d) failed\n", rotor->szPort, rotor->nBaud);
        exit(EXIT_FAILURE);
    }

    return 0;
}

int     rotor_release(rotor_t *rotor)
{
    assert(rotor);
    serial_close(&rotor->serial);
    if (rotor->szPort)
        free((char*)rotor->szPort);
    rotor->szPort = NULL;
fprintf(stderr, "rotor_release()\n");

    return 0;
}

int     rotor_move(rotor_t *rotor, uint16_t az, uint8_t el)
{
    char    buff[16];

    sprintf(buff, "W%03d %03d\n", az, el);
//    fprintf(stderr, "%s", buff);
    serial_write(&rotor->serial, buff, strnlen(buff, 16));

    return 0;
}
