#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "tsip.h"
#include "checksum.h"
#include "endian_util.h"

#define SEMI_2_DEG          (180.f / 0x7FFFFFFF)    /* 2^-31 semicircle to deg */

#define SECONDS_PER_DAY    86400
#define SECONDS_PER_HOUR    3600
#define TSIP_BASE_TIME 315964800   /* Timestamp for "January 6, 1980", UTC */
#define SECONDS_PER_WEEK    (SECONDS_PER_DAY*7)


/*
 * TSIP time conversion
 *
 * 63530-10_Rev-B_Manual_Copernicus-II.pdf p58/254
 * Week #0: starting January 6, 1980
 * The seconds count begins at midnight each Sunday morning
 */

time_t  TSIP_generateTimestamp_9F(uint32_t second, uint16_t week)
{
    time_t t;

    t = TSIP_BASE_TIME;
    t += week * SECONDS_PER_WEEK;
    t += second;

    return t;
}

time_t  TSIP_generateTimestamp_AF(uint32_t time24, uint32_t date24)
{
    time_t t;
    uint8_t hour, min, sec, day, month, year;
    struct tm tmp;

    memset(&tmp, 0, sizeof(struct tm));

//    fprintf(stdout, "[time] t:%d d:%d\n", time24, date24);

    hour = time24 / 10000;
    time24 -= hour * 10000;
    min = time24 / 100;
    sec = time24 - (min*100);

    day = date24 / 10000;
    date24 -= day * 10000;
    month = date24 / 100;
    year = date24 - (month*100);

//    fprintf(stdout, "[time] %02d:%02d:%02d d:%02d:%02d:%02d\n", hour, min, sec, day, month, year);

    tmp.tm_hour = hour;
    tmp.tm_min = min;
    tmp.tm_sec = sec;
    tmp.tm_mday = day;
    tmp.tm_mon = month - 1;
    tmp.tm_year = year + 100;

    t = timegm(&tmp);
//    fprintf(stdout, "[time] %ld\n", t);

    return t;
}

/*
 * Fill TSIP strcut from 0x8F-20 string
 */

int TSIP_stream2Struct(decoded_position_t *pPositon, const uint8_t *pStream, uint8_t size)
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

    if (!pPositon || !pStream || (size != TSIP_PACKET_SIZE))
        return -1;

    memset(pPositon, 0, sizeof(decoded_position_t));

    /*
     * Compute Checksum
     */

    pPositon->bIsValidChecksum = isValidM10Checksum(pStream, size);

    /*
     * M10 - Copernicus
     */

    if (!memcmp(pStream, "\x64\x9F\x20", 3)) {

        /*
         * Grab raw data from SuperPacketString
     	 * SuperPacket "0x8F-20" excerpt
         * 63530-10_Rev-B_Manual_Copernicus-II.pdf p177
         */

        pSuperPacket = &pStream[2];

        eastVelocity  = (int16_t) getu16_be((const uint8_t*)&pSuperPacket[2]);
        northVelocity = (int16_t) getu16_be((const uint8_t*)&pSuperPacket[4]);
        upVelocity    = (int16_t) getu16_be((const uint8_t*)&pSuperPacket[6]);
        time_ms = getu32_be((const uint8_t*)&pSuperPacket[8]);             // GPS Time,  UINT32, GPS Time in milliseconds
        latitude = (int32_t) getu32_be((const uint8_t*)&pSuperPacket[12]); // Latitude,   INT32, WGS-84 latitude, 2^31 semicircle (-90 -> 90)
        longitude = getu32_be((const uint8_t*)&pSuperPacket[16]);          // Longitude, UINT32, WGS-84 longitude, 2^31 semicircle (0 -> 360)
        altitude = (int32_t) getu32_be((const uint8_t*)&pSuperPacket[20]); // Altitude above WGS-84 ellipsoid, mm
        utcOffset = pSuperPacket[29];                                      // UTC Offset, UINT8, Number of leap seconds between UTC and GPS time.
        week = getu16_be((const uint8_t*)&pSuperPacket[30]);               // Week,       INT16, GPS time of fix, weeks.

        dScale = 0.005f;
        if (pSuperPacket[24] & 0x01)                                       // Velocity scale
            dScale = 0.020f;

        /*
         * Fill destination struct, The seconds count begins at the midnight which begins each Sunday morning
         */

        pPositon->unixEpoch = TSIP_generateTimestamp_9F(time_ms/1000-utcOffset + 1, week);  // p59/254

        pPositon->dLatitude  = (double) latitude * SEMI_2_DEG;
        pPositon->dLongitude = (double) longitude * SEMI_2_DEG;
        if (pPositon->dLongitude > 180.f)
            pPositon->dLongitude -= 360.f;
        pPositon->dAltitude  = (double) altitude / 1000.f;

        pPositon->dNorthGroundSpeedMs = northVelocity;
        pPositon->dNorthGroundSpeedMs *= dScale;
        pPositon->dEastGroundSpeedMs = eastVelocity;
        pPositon->dEastGroundSpeedMs *= dScale;
        pPositon->dGroundSpeedMs = sqrt( pPositon->dNorthGroundSpeedMs * pPositon->dNorthGroundSpeedMs + pPositon->dEastGroundSpeedMs * pPositon->dEastGroundSpeedMs );
        pPositon->dVerticalSpeedMs = upVelocity;
        pPositon->dVerticalSpeedMs *= dScale;

        return 0;
    }

    /*
     * GPS status packet
     */

    if (!memcmp(pStream, "\x64\x49", 2)) {

        //TODO
        return -1;
    }

    /*
     * M10 - Global Top Firefly
     */

    if (!memcmp(pStream, "\x64\xAF\x02", 3)) {

        /*
         * Grab raw values from stream
         */

        latitude = (int32_t)getu32_be((const uint8_t*)&pStream[4]);
        longitude = getu32_be((const uint8_t*)&pStream[8]);
        altitude = (int32_t) getu24_be((const uint8_t*)&pStream[12]);

        eastVelocity  = (int16_t)getu16_be((const uint8_t*)&pStream[15]);
        northVelocity = (int16_t)getu16_be((const uint8_t*)&pStream[17]);
        upVelocity    = (int16_t)getu16_be((const uint8_t*)&pStream[19]);
//        fprintf(stdout, "[Velocity] N:%d  E:%d  U:%d\n", northVelocity, eastVelocity, upVelocity);

        int32_t t = (int32_t)getu24_be((const uint8_t*)&pStream[21]);
        int32_t d = (int32_t)getu24_be((const uint8_t*)&pStream[24]);

        /*
         * Fill destinatino struct
         */

        pPositon->unixEpoch = TSIP_generateTimestamp_AF(t, d);

        pPositon->dLatitude  = (double) latitude / 1000000.f;
        pPositon->dLongitude = (double) longitude / 1000000.f;
        if (pPositon->dLongitude > 180.f)
            pPositon->dLongitude -= 360.f;
        pPositon->dAltitude  = (double) altitude / 100.f;

        pPositon->dNorthGroundSpeedMs = northVelocity / 100.f;
        pPositon->dEastGroundSpeedMs = eastVelocity /100.f;
        pPositon->dGroundSpeedMs = sqrt( pPositon->dNorthGroundSpeedMs * pPositon->dNorthGroundSpeedMs + pPositon->dEastGroundSpeedMs * pPositon->dEastGroundSpeedMs );
        pPositon->dVerticalSpeedMs = upVelocity / 100.f;

        return 0;
    }

    return -1;
}
