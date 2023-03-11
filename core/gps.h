#ifndef __GPS_H__
#define __GPS_H__

#include "trappette_sdk.h"

/*
 *
 */

int gps_tsipToNmeaFormat(char *gpgga, char *gprmc, const decoded_position_t *pPositon, double dGeoid);

#endif /*__GPS_H__*/
