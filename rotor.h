#ifndef __ROTOR_H__
#define __ROTOR_H__

#include <stdint.h>
#include "serial.h"

/*
 *
 */

#define ROTOR_AZ_MIN    0
#define ROTOR_AZ_MAX    360
#define ROTOR_EL_MIN    0
#define ROTOR_EL_MAX    90

/*
 *
 */

typedef struct  {

    /* Serial port */
    const char *szPort; 
    uint32_t    nBaud;
    serial_t    serial;

    /* Range */
    int16_t    nAzimuthMin;
    int16_t    nAzimuthMax;
    int8_t     nElevationMin;
    int8_t     nElevationMax;

    /* Init position */
    int16_t    nAzimuthInit;
    int8_t     nElevationInit;
} rotor_t;

/*
 *
 */

void    rotor_init(rotor_t*);
int     rotor_open(rotor_t*);
int     rotor_move(rotor_t*, uint16_t az, uint8_t el);
int     rotor_release(rotor_t*);

int     rotor_check_params(rotor_t*);

#endif /*__ROTOR_H__*/
