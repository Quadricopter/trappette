#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "gps.h"
#include "wgs84.h"

#define NMEA_EOL    "\r\n"

/*
 * Source: https://fr.wikipedia.org/wiki/NMEA_0183
 * $GPGGA,064036.289,4836.5375,N,00740.9373,E,1,04,3.2,200.2,M,,,,0000*0E
 * 
 * $GPGGA       : Type de trame
 * 064036.289   : Trame envoyée à 06h 40m 36,289s (heure UTC)
 * 4836.5375,N  : Latitude 48,608958° Nord = 48°36'32.25" Nord
 * 00740.9373,E : Longitude 7,682288° Est = 7°40'56.238" Est
 * 1            : Type de positionnement (le 1 est un positionnement GPS)
 * 04           : Nombre de satellites utilisés pour calculer les coordonnées
 * 3.2          : Précision horizontale ou HDOP (Horizontal dilution of precision)
 * 200.2,M      : Altitude 200,2, en mètres
 * ,,,,,0000    : D'autres informations peuvent être inscrites dans ces champs
 * *0E          : Somme de contrôle de parité, un simple XOR sur les caractères entre $ et *
*/

int gps_degToDegMin(int *deg, int *min, int *dec, double degrees)
{
    double  dMin;

    if (degrees < 0)
        degrees *= -1.f;

    /* Degrees */
    *deg = degrees;

    /* Minutes */
    dMin = (degrees - *deg) * 100.f;    
    dMin = 60.f * dMin / 100.f;
    *min = dMin;

    /* Minutes decimal */
    dMin = dMin - *min;
    dMin *= 10000.f;
    *dec = dMin;

    return 0;
}

uint8_t gps_computeNemaChecksum(const char *nmea)
{
    uint8_t checksum;
    char *p;

    checksum = 0x00;
    p = (char*) nmea;
    if (*p == '$')
        p++;
    while (*p && *p!='*' && *p!='\r' && *p!='\n') {

        checksum ^= *p;
        p++;
    }

    return checksum;
}

int gps_tsipToNmeaFormat(char *gpgga, char *gprmc, const decoded_position_t *pPositon, double dGeoid)
{
    int degLat, minLat, decLat;
    int degLon, minLon, decLon;
    double  dTrueCourse;
    char chkBuff[8];
    uint8_t checksum;
    struct  tm *ptm;

    if (!pPositon)
        return -1;

    gps_degToDegMin(&degLat, &minLat, &decLat, pPositon->dLatitude);
    gps_degToDegMin(&degLon, &minLon, &decLon, pPositon->dLongitude);
//    printf("%f%c %f%c  ->  ", pPositon->dLatitude, pPositon->dLatitude>0.f?'N':'S', pPositon->dLongitude, pPositon->dLongitude>0.f?'E':'O');
//    printf("%02d°%02d.%04d%c %03d°%02d.%04d%c\n",   degLat, minLat, decLat, pPositon->dLatitude>0.f?'N':'S',
//                                                    degLon, minLon, decLon, pPositon->dLongitude>0.f?'E':'W');

    ptm = gmtime(&pPositon->unixEpoch);

    if (gpgga) {

        sprintf(gpgga, "$GPGGA,%02d%02d%02d.000,%02d%02d.%04d,%c,%03d%02d.%04d,%c,1,4,3.2,%.1f,M,%.1f,M,,",
                    ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
                    degLat, minLat, decLat, pPositon->dLatitude>0.f?'N':'S',
                    degLon, minLon, decLon, pPositon->dLongitude>0.f?'E':'W',
                    pPositon->dAltitude-dGeoid, dGeoid);
        checksum = gps_computeNemaChecksum(gpgga);
        sprintf(chkBuff, "*%02X%s", checksum, NMEA_EOL);
        strcat(gpgga, chkBuff);
    }

    if (gprmc) {

        /*
         * Time for GPRMC
         */

        while (ptm->tm_year >= 100)
            ptm->tm_year -= 100;

        /*
         * Compute course from North and East velicity
         */

        dTrueCourse = 0.f;
//        fprintf(stderr, "NorthVel: %f, EastVel: %f\n", pPositon->dNorthGroundSpeedMs, pPositon->dEastGroundSpeedMs);
        dTrueCourse = radToDeg( atan2(pPositon->dEastGroundSpeedMs, pPositon->dNorthGroundSpeedMs) );
        while (dTrueCourse < 0.f)
            dTrueCourse += 360.f;

        sprintf(gprmc, "$GPRMC,%02d%02d%02d.000,A,%02d%02d.%04d,%c,%03d%02d.%04d,%c,%.1f,%.1f,%02d%02d%02d,,,A",
                    ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
                    degLat, minLat, decLat, pPositon->dLatitude>0.f?'N':'S',
                    degLon, minLon, decLon, pPositon->dLongitude>0.f?'E':'W',
                    pPositon->dGroundSpeedMs/0.5144444f,   // m/s to knots
                    dTrueCourse,
                    ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year );
        checksum = gps_computeNemaChecksum(gprmc);
        sprintf(chkBuff, "*%02X%s", checksum, NMEA_EOL);
        strcat(gprmc, chkBuff);
    }

    return 0;
}
