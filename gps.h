#ifndef __GPS_H__
#define __GPS_H__

#include "tsip.h"

/*
 *
 */

int gps_tsipToNmeaFormat(char *gpgga, char *gprmc, const tsip_t *pTsip, double dGeoid);

#endif /*__GPS_H__*/
