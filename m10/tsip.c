#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "tsip.h"
#include "checksum.h"


#define SEMI_2_DEG          (180.f / 2147483647)    /* 2^-31 semicircle to deg */

#define SECONDS_PER_DAY    86400
#define SECONDS_PER_HOUR    3600
#define TSIP_BASE_TIME 315964800   /* Timestamp for "January 6, 1980", UTC */
#define SECONDS_PER_WEEK    (SECONDS_PER_DAY*7)

/*
 *
 */

uint32_t    getu32(const uint8_t *data)
{
    return data[0]<<24 | data[1]<<16 | data[2]<<8 | data[3];
}

uint32_t    geti16(const uint8_t *data)
{
    return data[0]<<8 | data[1];
}


/*
 * TSIP time conversion
 *
 * 63530-10_Rev-B_Manual_Copernicus-II.pdf p58/254
 * Week #0: starting January 6, 1980
 * The seconds count begins at midnight each Sunday morning
 */

time_t  TSIP_generateTimestamp(uint32_t second, uint16_t week)
{
    time_t t;

    t = TSIP_BASE_TIME;
    t += week * SECONDS_PER_WEEK;
    t += second;

    return t;
}

/*
 * Fill TSIP strcut from 0x8F-20 string
 */

int TSIP_string2Struct(tsip_t *pTsip, const uint8_t *buff, uint8_t size)
{
    const uint8_t *pSuperPacket = NULL;
    uint32_t    time_ms;
    int8_t      utcOffset;
    uint16_t    week;
    int32_t     latitude;
    uint32_t    longitude;
    int32_t     altitude;
    double      dScale;
    int16_t     northVelocity, eastVelocity, upVelocity;

    if (!pTsip || !buff || (size != TSIP_PACKET_SIZE))
        return -1;

    memset(pTsip, 0, sizeof(tsip_t));

    /*
     * Compute Checksum
     */

    pTsip->bIsValidChecksum = isValidM10Checksum(buff, size);

    /*
     * SuperPacket "0x8F-20" excerpt
     * 63530-10_Rev-B_Manual_Copernicus-II.pdf p177
     */

    if (buff[1] == 0x9F && buff[2] == 0x20) {

        /*
         * Grab informations from SuperPacketString
         */

        pSuperPacket = &buff[2];

        time_ms = getu32((const uint8_t*)&pSuperPacket[8]);             // GPS Time,  UINT32, GPS Time in milliseconds
        latitude = (int32_t)getu32((const uint8_t*)&pSuperPacket[12]);  // Latitude,   INT32, WGS-84 latitude, 2^31 semicircle (-90 -> 90)
        longitude = getu32((const uint8_t*)&pSuperPacket[16]);          // Longitude, UINT32, WGS-84 longitude, 2^31 semicircle (0 -> 360)
        altitude = (int32_t) getu32((const uint8_t*)&pSuperPacket[20]); // Altitude above WGS-84 ellipsoid, mm
        utcOffset = pSuperPacket[29];                                   // UTC Offset, UINT8, Number of leap seconds between UTC and GPS time.
        week = geti16((const uint8_t*)&pSuperPacket[30]);               // Week,       INT16, GPS time of fix, weeks.

        /* Convert GPS Epoch to UNIX Epoch */
        pTsip->unixEpoch = TSIP_generateTimestamp(time_ms/1000-utcOffset + 1, week);  // p59/254

        eastVelocity  = geti16((const uint8_t*)&pSuperPacket[2]);
        northVelocity = geti16((const uint8_t*)&pSuperPacket[4]);
        upVelocity    = geti16((const uint8_t*)&pSuperPacket[6]);
        dScale = 0.005f;
        if (pSuperPacket[24] & 0x01)                                    // Velocity scale
            dScale = 0.020f;

        /*
         * Fill destination struct, The seconds count begins at the midnight which begins each Sunday morning
         */

        pTsip->dLatitude  = (double) latitude * SEMI_2_DEG;
        pTsip->dLongitude = (double) longitude * SEMI_2_DEG;
        if (pTsip->dLongitude > 180.f)
            pTsip->dLongitude -= 360.f;
        pTsip->dAltitude  = (double) altitude / 1000.f;

        pTsip->dNorthGroundSpeedMs = northVelocity;
        pTsip->dNorthGroundSpeedMs *= dScale;
        pTsip->dEastGroundSpeedMs = eastVelocity;
        pTsip->dEastGroundSpeedMs *= dScale;
        pTsip->dGroundSpeedMs = sqrt( pTsip->dNorthGroundSpeedMs * pTsip->dNorthGroundSpeedMs + pTsip->dEastGroundSpeedMs * pTsip->dEastGroundSpeedMs );
        pTsip->dClimbRateMs = upVelocity;
        pTsip->dClimbRateMs *= dScale;

        return 0;
    }

    /*
     * GPS status packet
     */

    if (buff[1] == 0x49) {

        //TODO
        return -1;
    }

    return -1;
}
