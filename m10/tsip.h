#ifndef __TSIP_H__
#define __TSIP_H__

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#define TSIP_PACKET_SIZE    101

/*
 *
 */

typedef struct  {

    time_t      unixEpoch;

    double      dLatitude;
    double      dLongitude;
    double      dAltitude;

    double      dNorthGroundSpeedMs;
    double      dEastGroundSpeedMs;
    double      dGroundSpeedMs;
    double      dClimbRateMs;

    bool        bIsValidChecksum;
}   tsip_t; 

/*
 *
 */

int TSIP_string2Struct(tsip_t *pTsip, const uint8_t *buff, uint8_t size);
int TSIP_getTime(uint8_t *hour, uint8_t *min, uint8_t *sec, const tsip_t *pTsip);
void    TSIP_timeToString(char *dst, uint32_t seconds);

#endif /*__TSIP_H__*/
